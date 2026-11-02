"""Fourteen staged runtime equipment views; no map/asset saves or game-stage writes.

Run in UnrealEditor -game with -EquipmentPythonReview. This uses the existing
pawn CameraComponent because deferred runtime spawning is not exposed by the
public Python GameplayStatics API. Epic API references and launch instructions
are in Tools/review_equipment.md. The script is not a playtest or benchmark.
"""
import datetime
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import time
import traceback
import unreal

DETAIL_TAG = "Lunar.EquipmentDetail"
SHOT_NAMES = (
    "surveyor-overview", "surveyor-camera", "surveyor-scoop", "surveyor-cable",
    "alsep-seismometer", "alsep-central", "intrepid-wide", "intrepid-side-handles",
    "selene-power-socket", "selene-hdu-contact", "selene-rover-wheel-axes", "selene-solar-underside",
    "ares-outpost", "ares-mesa-talus",
)
EXPECTED = {
    "ApolloLander": ((-325, -324.960337108427, 0), (325, 324.960337108427, 506.941769932379)),
    "HabitatMain": ((-625, -436.580595861489, 0), (625, 436.580595861489, 431.163416819958)),
    "HabitatUtility": ((-350, -152.749812418051, 0), (350, 152.749812418051, 400.311307117443)),
    "Lunokhod": ((-103.576640771552, -170, 0), (103.576640771552, 170, 237.555946193295)),
    "PowerCell": ((-32.031891687987, -32.5, 0), (32.031891687987, 32.5, 79.921261744598)),
    "ScienceTerminal": ((-70.781591787131, -70.781588421, 0), (70.781591787131, 70.781588421, 230)),
    "SolarPanel": ((-230, -92.450279556088, 0), (230, 92.450279556088, 181.286919599347)),
}

def xyz(v):
    return [float(v.x), float(v.y), float(v.z)]

def vec(v):
    return unreal.Vector(*v)

def add(a, b):
    return unreal.Vector(a.x + b[0], a.y + b[1], a.z + b[2])

def distance(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(xyz(a), xyz(b))))

def argument(name, default=None):
    pattern = r'(?:^|\s)-' + re.escape(name) + r'=(?:"([^"]+)"|([^\s]+))'
    match = re.search(pattern, unreal.SystemLibrary.get_command_line(), re.IGNORECASE)
    return (match.group(1) or match.group(2)) if match else default

def mesh_of(component):
    return component.get_editor_property("static_mesh")

def primary(actor):
    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        if not isinstance(component, unreal.InstancedStaticMeshComponent) and mesh_of(component):
            return component
    raise RuntimeError("Missing original static mesh: " + actor.get_path_name())

def source_point(component, point):
    return unreal.MathLibrary.transform_location(component.get_world_transform(), vec(point))

class EquipmentReview:
    def __init__(self):
        command = unreal.SystemLibrary.get_command_line()
        if not re.search(r"(?:^|\s)-EquipmentPythonReview(?=\s|$)", command, re.IGNORECASE):
            raise RuntimeError("Explicit -EquipmentPythonReview isolation flag is required")
        self.width = int(argument("EquipmentReviewWidth", "1280"))
        self.height = int(argument("EquipmentReviewHeight", "720"))
        if not (640 <= self.width <= 3840 and 480 <= self.height <= 2160):
            raise RuntimeError("Review dimensions must be between 640x480 and 3840x2160")
        folder = argument("EquipmentReviewOutput")
        if not folder:
            raise RuntimeError("Provide a fresh -EquipmentReviewOutput directory")
        self.folder = Path(folder).resolve()
        project = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())).resolve()
        reports = (project / "Reports").resolve()
        if reports not in self.folder.parents:
            raise RuntimeError("Review output must be a new child directory of the current project Reports")
        if self.folder.exists():
            raise RuntimeError("Refusing to reuse an existing review directory")
        self.folder.mkdir(parents=True, exist_ok=False)
        raw_shots = argument("EquipmentReviewShots", "")
        self.requested_shots = raw_shots.split(",") if raw_shots else []
        if len(set(self.requested_shots)) != len(self.requested_shots) or any(n not in SHOT_NAMES for n in self.requested_shots):
            raise RuntimeError("EquipmentReviewShots must contain distinct exact names from the documented shot plan")
        self.started = time.monotonic()
        self.world = self.game = self.controller = self.pawn = self.camera = None
        self.frames = []
        self.bounds = []
        self.shots = []
        self.actors = []
        self.detail = {}
        self.hidden_widgets = []
        self.index = 0
        self.phase = "startup"
        self.deadline = self.started + 45
        self.handle = unreal.register_slate_post_tick_callback(self.tick)
        unreal.log("EQUIPMENT_PYTHON_REVIEW registered; awaiting actual game world")

    def worlds(self):
        # Public ObjectIterator is also useful in -game, where the editor
        # subsystem's PIE-specific game world can legitimately be absent.
        return list(unreal.ObjectIterator(unreal.World))

    def ready(self):
        for world in self.worlds():
            name = unreal.GameplayStatics.get_current_level_name(world, True)
            if name not in ("Apollo12", "MoonBase", "MarsBase"):
                continue
            game = unreal.GameplayStatics.get_game_mode(world)
            controller = unreal.GameplayStatics.get_player_controller(world, 0)
            pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
            if game and controller and pawn:
                self.world, self.game, self.controller, self.pawn = world, game, controller, pawn
                self.map_name = name
                if game.actor_has_tag("Lunar.EquipmentDetailBuilt"):
                    return True
        return False

    def tagged(self, tag):
        matches = [a for a in self.actors if a.actor_has_tag(tag)]
        if len(matches) != 1:
            raise RuntimeError("Expected one actor tagged %s, found %d" % (tag, len(matches)))
        return matches[0]

    def model(self, name):
        for actor in self.actors:
            if actor == self.pawn:
                continue  # Imported scene axes, not intentionally expanded POV bounds.
            for component in actor.get_components_by_class(unreal.StaticMeshComponent):
                if isinstance(component, unreal.InstancedStaticMeshComponent):
                    continue
                mesh = mesh_of(component)
                if mesh and mesh.get_name() == name:
                    return actor, component
        raise RuntimeError("Missing retained mesh " + name)

    def subject(self, name, actor, eye_offset, target_offset=(0, 0, 0)):
        origin = actor.get_actor_location()
        self.shots.append({"name": name, "actor": actor, "eye": add(origin, eye_offset),
                           "target": add(origin, target_offset), "space": "runtime actor offset"})

    def model_shot(self, name, model_name, eye_source, target_source):
        actor, component = self.model(model_name)
        self.shots.append({"name": name, "actor": actor, "eye": source_point(component, eye_source),
                           "target": source_point(component, target_source), "space": "retained mesh source cm transformed through actual component"})

    def terrain_height(self, point):
        terrain_name = {"Apollo12": "SM_Apollo12Terrain", "MoonBase": "SM_LunarTerrain", "MarsBase": "SM_MarsTerrain"}[self.map_name]
        _, terrain = self.model(terrain_name)
        hit = terrain.line_trace_component(add(point, (0, 0, 20000)), add(point, (0, 0, -20000)), True, False, False)
        if hit is None:
            raise RuntimeError("No actual terrain contact for exterior review camera")
        location, normal, bone, out_hit = hit
        return float(location.z)

    def solar_shot(self):
        actor, panel = self.model("SolarPanel")
        matches = []
        for component in actor.get_components_by_class(unreal.InstancedStaticMeshComponent):
            if not component.component_has_tag(DETAIL_TAG):
                continue
            mesh = mesh_of(component)
            if not mesh or mesh.get_name() != "Cube":
                continue
            for index in range(component.get_instance_count()):
                transform = component.get_instance_transform(index, True)
                if transform is None:
                    raise RuntimeError("Missing actual solar junction instance transform")
                scale = transform.scale3d
                if max(abs(abs(a)-b) for a,b in zip(xyz(scale), (.23,.17,.11))) < .001:
                    matches.append((component, index, transform))
        if len(matches) != 1:
            raise RuntimeError("Expected exactly one authored solar junction Cube instance, found %d" % len(matches))
        component, index, transform = matches[0]
        target = transform.translation
        normal = transform.transform_direction(vec((0,0,1)))
        side = transform.transform_direction(vec((1,0,0)))
        selected = None
        attempts = []
        original_collision = panel.get_collision_enabled()
        try:
            # This retained panel is normally NoCollision. A false trace in
            # that mode is not a visibility proof. Enable queries only in the
            # transient review world, then always restore the exact old mode.
            panel.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
            for sign in (1,-1):
                eye = add(target, (normal.x*sign*350 + side.x*95,
                                   normal.y*sign*350 + side.y*95,
                                   normal.z*sign*350 + side.z*95 + 35))
                ground = self.terrain_height(eye)
                eye.z = max(eye.z, ground+130)
                blocked = panel.line_trace_component(eye, target, True, False, False) is not None
                attempts.append({"normal_sign":sign,"eye_cm":xyz(eye),"ground_z_cm":ground,"original_panel_occludes_target":blocked})
                if not blocked:
                    selected = (eye, ground, sign)
                    break
        finally:
            panel.set_collision_enabled(original_collision)
        if panel.get_collision_enabled() != original_collision:
            raise RuntimeError("Transient panel collision mode was not restored")
        if selected is None:
            raise RuntimeError("Original solar panel blocks both actual-instance normal views: " + str(attempts))
        eye, ground, sign = selected
        self.shots.append({"name":"selene-solar-underside","actor":actor,"eye":eye,"target":target,
                           "space":"actual authored Cube instance center and transformed normal, panel-only line-of-sight proof",
                           "target_component":component.get_path_name(),"target_instance":index,
                           "target_instance_world_cm":xyz(target),"camera_ground_z_cm":ground,
                           "target_normal_world":xyz(normal),"normal_side_selected":sign,
                           "target_visibility_trace_attempts":attempts,"panel_original_collision_mode":str(original_collision),
                           "panel_collision_restored":True})

    def mesa_shot(self):
        actor, mesa = self.model("SM_MarsMesa")
        center, extent = actor.get_actor_bounds(False, False)
        talus = []
        for owner in self.actors:
            if not owner.actor_has_tag("Lunar.EquipmentSurface"):
                continue
            for component in owner.get_components_by_class(unreal.InstancedStaticMeshComponent):
                if not component.component_has_tag(DETAIL_TAG):
                    continue
                for index in range(component.get_instance_count()):
                    transform = component.get_instance_transform(index, True)
                    if transform is None:
                        raise RuntimeError("Missing actual surface instance transform")
                    mesh = mesh_of(component)
                    if not mesh or not mesh.get_name().startswith("SM_Basalt_"):
                        continue
                    bounds = mesh.get_bounding_box()
                    low, high = bounds.min, bounds.max
                    scale = transform.scale3d
                    dimensions = [abs((b-a)*c) for a,b,c in zip(xyz(low),xyz(high),xyz(scale))]
                    if max(abs(a-b) for a,b in zip(dimensions,(70,45,25))) > .05:
                        continue  # Actual bounded dimensions, not assumed 100 cm primitives.
                    midpoint = vec([(a+b)*.5 for a,b in zip(xyz(low),xyz(high))])
                    point = transform.transform_location(midpoint)
                    talus.append((math.hypot(point.x-center.x, point.y-center.y), point, component, index))
        if not talus:
            raise RuntimeError("No actual authored talus instances available for visual review")
        _, point, component, index = min(talus, key=lambda t: t[0])
        dx, dy = point.x-center.x, point.y-center.y
        length = math.hypot(dx, dy)
        if length < 1:
            raise RuntimeError("Cannot derive outward mesa view from actual talus")
        eye = add(point, (dx/length*1800, dy/length*1800, 400))
        eye.z = max(eye.z, self.terrain_height(eye)+350)
        target = add(point, (0, 0, 35))
        self.shots.append({"name": "ares-mesa-talus", "actor": actor, "eye": eye, "target": target,
                           "space": "actual talus instance, exterior mesa direction, terrain-cleared camera",
                           "target_component": component.get_path_name(), "target_instance": index,
                           "target_instance_world_cm": xyz(point), "camera_ground_z_cm": self.terrain_height(eye)})

    def inspect_bounds(self):
        seen = set()
        count = total = 0
        for actor in self.actors:
            if actor == self.pawn:
                continue  # Imported scene axes, not intentionally expanded POV bounds.
            for component in actor.get_components_by_class(unreal.StaticMeshComponent):
                if isinstance(component, unreal.InstancedStaticMeshComponent):
                    if component.component_has_tag(DETAIL_TAG):
                        count += 1
                        total += component.get_instance_count()
                    continue
                mesh = mesh_of(component)
                if not mesh or mesh.get_name() not in EXPECTED or mesh.get_name() in seen:
                    continue
                name = mesh.get_name()
                seen.add(name)
                low, high = component.get_local_bounds()
                expected = EXPECTED[name]
                actual = (xyz(low), xyz(high))
                error = max(abs(a - b) for aa, bb in zip(actual, expected) for a, b in zip(aa, bb))
                self.bounds.append({"mesh": name, "actor": actor.get_path_name(),
                                    "component": component.get_path_name(), "actual_local_min_max_cm": actual,
                                    "retained_obj_expected_min_max_cm": expected, "maximum_axis_error_cm": error,
                                    "axes_match_within_0_1_cm": error <= .1})
        self.detail = {"components": count, "instances": total, "cap_components": 128, "cap_instances": 1400}
        if count <= 0 or total <= 0 or count > 128 or total > 1400:
            raise RuntimeError("Missing or over-budget actual equipment geometry: " + str(self.detail))
        bad = [b["mesh"] for b in self.bounds if not b["axes_match_within_0_1_cm"]]
        if bad:
            raise RuntimeError("Imported mesh bounds/axes differ from retained OBJ: " + ", ".join(bad))

    def setup(self):
        self.actors = list(unreal.GameplayStatics.get_all_actors_of_class(self.world, unreal.Actor))
        self.inspect_bounds()
        self.camera = self.pawn.get_component_by_class(unreal.CameraComponent)
        movement = self.pawn.get_component_by_class(unreal.CharacterMovementComponent)
        if not self.camera or not movement:
            raise RuntimeError("Playable pawn lacks its existing camera or CharacterMovementComponent")
        self.game.set_actor_tick_enabled(False)
        movement.stop_movement_immediately()
        movement.disable_movement()
        movement.set_component_tick_enabled(False)
        self.pawn.set_actor_tick_enabled(False)
        self.pawn.set_actor_hidden_in_game(True)
        self.pawn.set_actor_enable_collision(False)
        for axis in ("yaw", "pitch", "roll"):
            self.pawn.set_editor_property("use_controller_rotation_" + axis, False)
        self.camera.set_editor_property("absolute_rotation", True)
        self.camera.set_editor_property("use_pawn_control_rotation", False)
        self.camera.set_field_of_view(85.0)
        self.controller.set_view_target_with_blend(self.pawn, 0.0)
        # UWidgetBlueprintLibrary declares ScriptName="WidgetLibrary" in UE
        # 5.8; its C++ class name is intentionally not a Python export. Use the
        # documented current-world + viewport-only filter, never editor widgets.
        widget_library = getattr(unreal, "WidgetLibrary", None)
        user_widget = getattr(unreal, "UserWidget", None)
        if widget_library is None or user_widget is None:
            raise RuntimeError("Required UMG Python exports WidgetLibrary/UserWidget are unavailable")
        widgets = list(widget_library.get_all_widgets_of_class(self.world, user_widget, True))
        self.hidden_widgets = [widget.get_path_name() for widget in widgets]
        for widget in widgets:
            widget.remove_from_parent()
        if widget_library.get_all_widgets_of_class(self.world, user_widget, True):
            raise RuntimeError("Current-world viewport widgets remain after transient review removal")
        # Set the documented HUD property explicitly: a toggle could re-enable
        # a previously hidden Canvas. This changes only the transient game HUD.
        hud = self.controller.get_hud()
        if not hud:
            raise RuntimeError("Playable controller has no runtime HUD")
        hud.set_editor_property("show_hud", False)
        if self.map_name == "Apollo12":
            self.subject("surveyor-overview", self.tagged("Surveyor3"), (-720, -430, 208), (0, 0, 50))
            self.subject("surveyor-camera", self.tagged("surveyor.camera"), (-180, -55, 85), (-8, 0, 12))
            self.subject("surveyor-scoop", self.tagged("surveyor.scoop"), (-170, -85, 95), (0, 0, 5))
            self.subject("surveyor-cable", self.tagged("surveyor.cable"), (-170, -85, 85), (0, 4.5, 0))
            self.subject("alsep-seismometer", self.tagged("alsep.seismometer"), (-180, -160, 90), (0, 0, -25))
            self.subject("alsep-central", self.tagged("alsep.central"), (-260, -200, 120), (-10, 0, 5))
            self.model_shot("intrepid-wide", "ApolloLander", (-950, -760, 480), (0, 0, 220))
            self.model_shot("intrepid-side-handles", "ApolloLander", (-390, -180, 220), (-152.36, 0, 147.5))
        elif self.map_name == "MoonBase":
            self.subject("selene-power-socket", self.tagged("Power"), (-240, -120, 110), (-70, 0, -10))
            self.model_shot("selene-hdu-contact", "HabitatMain", (-790, 330, 240), (-293.7382, 150, 120))
            self.model_shot("selene-rover-wheel-axes", "Lunokhod", (-530, -380, 380), (0, -20, 95))
            if not self.requested_shots or "selene-solar-underside" in self.requested_shots:
                self.solar_shot()
        else:
            self.subject("ares-outpost", self.tagged("Power"), (-890, -2900, 370), (810, -600, 90))
            if not self.requested_shots or "ares-mesa-talus" in self.requested_shots:
                self.mesa_shot()
        expected_count = {"Apollo12": 8, "MoonBase": 4, "MarsBase": 2}[self.map_name]
        if not self.requested_shots and len(self.shots) != expected_count:
            raise RuntimeError("Unexpected shot-plan count")
        if self.requested_shots:
            self.shots = [shot for shot in self.shots if shot["name"] in self.requested_shots]
            if not self.shots:
                raise RuntimeError("Requested shots do not belong to this mission world")
        self.begin_shot()

    def begin_shot(self):
        if self.index == len(self.shots):
            self.finish(True)
            return
        shot = self.shots[self.index]
        path = self.folder / (shot["name"] + ".png")
        if path.exists():
            raise RuntimeError("Refusing to overwrite an old PNG: " + str(path))
        self.file = path
        rotation = unreal.MathLibrary.find_look_at_rotation(shot["eye"], shot["target"])
        offset = self.camera.get_world_location() - self.pawn.get_actor_location()
        self.pawn.set_actor_location(shot["eye"] - offset, False, True)
        self.controller.set_control_rotation(rotation)
        self.camera.set_world_location_and_rotation(shot["eye"], rotation, False, True)
        self.camera.set_field_of_view(85.0)
        self.phase = "settle"
        self.deadline = time.monotonic() + 1.4
        unreal.log("EQUIPMENT_PYTHON_REVIEW pose " + shot["name"])

    def request(self):
        self.request_wall = time.time()
        command = 'HighResShot filename="%s" %dx%d' % (self.file.as_posix(), self.width, self.height)
        unreal.SystemLibrary.execute_console_command(self.world, command, self.controller)
        self.phase = "capture"
        self.deadline = time.monotonic() + 25

    def capture(self):
        if not self.file.exists():
            return False
        data = self.file.read_bytes()
        if len(data) < 1024 or not data.startswith(b"\x89PNG\r\n\x1a\n") or not data.endswith(b"\x00\x00\x00\x00IEND\xaeB`\x82"):
            return False
        width, height = struct.unpack(">II", data[16:24])
        if (width, height) != (self.width, self.height) or self.file.stat().st_mtime < self.request_wall - 1:
            raise RuntimeError("Invalid dimensions or stale captured PNG")
        shot = self.shots[self.index]
        actual_eye, actual_rotation = self.controller.get_player_view_point()
        if distance(actual_eye, shot["eye"]) > 1:
            raise RuntimeError("Actual player camera differs from requested eye by more than 1 cm")
        expected_rotation = unreal.MathLibrary.find_look_at_rotation(actual_eye, shot["target"])
        rotation_error = max(abs((a - b + 180) % 360 - 180) for a, b in ((actual_rotation.yaw, expected_rotation.yaw), (actual_rotation.pitch, expected_rotation.pitch)))
        if rotation_error > .5:
            raise RuntimeError("Actual camera aim differs from requested target by more than 0.5 degrees")
        origin, extent = shot["actor"].get_actor_bounds(False, False)
        self.frames.append({"name": shot["name"], "file": self.file.name, "width": width, "height": height,
                            "sha256": hashlib.sha256(data).hexdigest(), "actor": shot["actor"].get_path_name(),
                            "requested_eye_cm": xyz(shot["eye"]), "target_cm": xyz(shot["target"]),
                            "actual_eye_cm": xyz(actual_eye), "actual_rotation_deg": [actual_rotation.pitch, actual_rotation.yaw, actual_rotation.roll],
                            "fov_degrees": float(self.camera.get_editor_property("field_of_view")), "aim_error_degrees": rotation_error, "coordinate_space": shot["space"],
                            "actor_bounds_world_center_extent_cm": [xyz(origin), xyz(extent)], "equipment_counts": self.detail,
                            "target_instance": shot.get("target_instance"), "target_component": shot.get("target_component"),
                            "target_instance_world_cm": shot.get("target_instance_world_cm"), "camera_ground_z_cm": shot.get("camera_ground_z_cm"),
                            "target_normal_world":shot.get("target_normal_world"), "normal_side_selected":shot.get("normal_side_selected"),
                            "target_visibility_trace_attempts":shot.get("target_visibility_trace_attempts"),
                            "panel_original_collision_mode":shot.get("panel_original_collision_mode"), "panel_collision_restored":shot.get("panel_collision_restored"),
                            "subject_equipment_batches": [{"component": c.get_path_name(), "instances": c.get_instance_count()}
                                for c in shot["actor"].get_components_by_class(unreal.InstancedStaticMeshComponent) if c.component_has_tag(DETAIL_TAG)]})
        self.index += 1
        self.begin_shot()
        return True

    def finish(self, success, error=None):
        if self.phase == "finished":
            return
        self.phase = "finished"
        unreal.unregister_slate_post_tick_callback(self.handle)
        result = {"synthetic": True, "human_playtest": False, "performance_measurement": False,
                  "success": bool(success), "error": error, "camera_mode": "existing_game_pawn_camera_runtime_only",
                  "map": getattr(self, "map_name", None), "completed_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  "description": "Fourteen planned equipment presentation views across three separate game worlds; no Stage access, map/asset saves, real mission completion, user photographs or benchmark claims.",
                  "dimensions": [self.width, self.height], "equipment_counts": self.detail,
                  "requested_shot_names": self.requested_shots, "complete_default_plan": not bool(self.requested_shots),
                  "removed_current_world_viewport_widgets": self.hidden_widgets,
                  "retained_model_bounds_checks": self.bounds, "frames": self.frames}
        (self.folder / "equipment-manifest.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
        unreal.log(("EQUIPMENT_PYTHON_REVIEW COMPLETE " if success else "EQUIPMENT_PYTHON_REVIEW FAILED ") + str(self.folder))
        if error:
            unreal.log_error(error)
        if self.world and self.controller:
            unreal.SystemLibrary.quit_game(self.world, self.controller, unreal.QuitPreference.QUIT, False)
        else:
            unreal.SystemLibrary.execute_console_command(None, "quit")

    def tick(self, delta):
        try:
            now = time.monotonic()
            if self.phase == "startup":
                if self.ready():
                    self.setup()
                elif now > self.deadline:
                    raise RuntimeError("No fully constructed mission game world within 45 seconds")
            elif self.phase == "settle" and now >= self.deadline:
                self.request()
            elif self.phase == "capture":
                if not self.capture() and now > self.deadline:
                    raise RuntimeError("Screenshot did not complete within 25 seconds: " + str(self.file))
        except Exception:
            self.finish(False, traceback.format_exc())

_EQUIPMENT_PYTHON_REVIEW = EquipmentReview()
