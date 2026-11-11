"""Safety and evidence validation shared by the Intrepid PowerShell wrappers.

This module never starts Unreal. Original saved/config/photo files are backed up
before a run and restored byte-for-byte with their original modification times.
Changed maps/project config are reported, never overwritten with an old backup.
"""
import argparse
import csv
import hashlib
import json
import math
import os
import re
import shutil
import stat
import statistics
import struct
import subprocess
import zlib
import sys
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path

FAMILY = Path("Reports/IntrepidVisual20261111")
STAGED = Path("Dist/Apollo12/Windows/LunarRescue")
TERRAIN = "/Game/Lunar/Apollo12/Meshes/SM_Apollo12Terrain.SM_Apollo12Terrain"
CAMERAS = ("intrepid-wide", "fp-ground", "ladder-foot", "outside-ground")
CABIN_CAMERAS = ("cabin-front", "cabin-side")
CABIN_SOURCE = "/Game/Lunar/Imported/ApolloLander/ApolloLander.ApolloLander"
CABIN_DERIVED = "/Game/Lunar/Apollo12/IntrepidVisual/SM_IntrepidCabin.SM_IntrepidCabin"
CABIN_MATERIAL = "/Game/Lunar/Apollo12/IntrepidVisual/M_IntrepidCabin.M_IntrepidCabin"
CABIN_SLOTS = {"initialShadingGr_001": .498039216, "blinn3SG_001": .678431392}


def sha(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def guarded(root, path):
    """Reject traversal, junctions and symlinks, including existing ancestors."""
    root = Path(os.path.abspath(root))
    path = Path(os.path.abspath(path))
    if not path.is_relative_to(root):
        raise ValueError(f"Path escaped its permitted root: {path}")
    for ancestor in (path, *path.parents):
        if ancestor.exists() or ancestor.is_symlink():
            info = ancestor.lstat()
            if ancestor.is_symlink() or getattr(info, "st_file_attributes", 0) & getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400):
                raise ValueError(f"Refusing reparse-point path: {ancestor}")
    return path


def files_below(root, folder):
    folder = guarded(root, folder)
    if not folder.exists():
        return
    for current, directories, files in os.walk(folder, followlinks=False):
        for name in directories:
            guarded(root, Path(current) / name)
        for name in files:
            yield guarded(root, Path(current) / name)


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n", encoding="utf-8", newline="\n")


def protected(root):
    result = {}
    for saved in (root / "Saved", root / STAGED / "Saved"):
        for path in files_below(root, saved):
            parts = {p.casefold() for p in path.relative_to(saved).parts[:-1]}
            if path.suffix.lower() in (".ini", ".sav") or parts.intersection(("config", "savegames", "photos")):
                result[path] = "player_saved_config"
    for path in files_below(root, root / "Config"):
        result[path] = "project_config"
    for path in files_below(root, root / "Content"):
        if path.suffix.lower() == ".umap":
            result[path] = "map"
    return result


def snapshot(root, output):
    root = guarded(root, root)
    output = guarded(root, output)
    if output.exists():
        raise ValueError(f"Snapshot output already exists: {output}")
    output.mkdir(parents=True)
    rows = []
    for path, category in sorted(protected(root).items()):
        relative = path.relative_to(root).as_posix()
        backup = guarded(root, output / "files" / relative)
        backup.parent.mkdir(parents=True, exist_ok=True)
        info = path.stat()
        if category == "map" and path.read_bytes()[:64].startswith(b"version https://git-lfs.github.com/spec/"):
            raise ValueError(f"Map is an unresolved LFS pointer: {relative}")
        original_sha = sha(path)
        shutil.copy2(path, backup)
        if sha(backup) != original_sha or sha(path) != original_sha or path.stat().st_mtime_ns != info.st_mtime_ns:
            raise ValueError(f"Protected file changed while taking snapshot: {relative}")
        rows.append(dict(path=relative, category=category, size=info.st_size,
                         sha256=original_sha, mtime_ns=info.st_mtime_ns,
                         backup=backup.relative_to(root).as_posix()))
    manifest = dict(schema_version=1, root=str(root), created_utc=datetime.now(timezone.utc).isoformat(),
                    counts=dict(Counter(r["category"] for r in rows)), files=rows)
    write_json(output / "manifest.json", manifest)
    return manifest


def restore(root, manifest_path, output):
    root = guarded(root, root)
    manifest_path = guarded(root, manifest_path)
    manifest = json.loads(manifest_path.read_text(encoding="utf-8-sig"))
    if Path(manifest["root"]) != root:
        raise ValueError("Snapshot root does not match this checkout")
    changes, errors = [], []
    known = set()
    for row in manifest["files"]:
        path = guarded(root, root / row["path"])
        backup = guarded(root, root / row["backup"])
        known.add(row["path"])
        if sha(backup) != row["sha256"]:
            errors.append("Backup hash mismatch: " + row["path"])
            continue
        same = path.is_file() and sha(path) == row["sha256"]
        if not same and row["category"] != "player_saved_config":
            errors.append("Protected map/project config changed; not overwritten: " + row["path"])
            continue
        changed = dict(path=row["path"], restored_bytes=False, restored_mtime=False)
        if not same:
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(backup, path)
            changed["restored_bytes"] = True
        if path.stat().st_mtime_ns != row["mtime_ns"]:
            os.utime(path, ns=(path.stat().st_atime_ns, row["mtime_ns"]))
            changed["restored_mtime"] = True
        if changed["restored_bytes"] or changed["restored_mtime"]:
            changes.append(changed)
        if sha(path) != row["sha256"] or path.stat().st_mtime_ns != row["mtime_ns"]:
            errors.append("Restoration verification failed: " + row["path"])
    generated = []
    for path, category in sorted(protected(root).items()):
        relative = path.relative_to(root).as_posix()
        if relative not in known:
            generated.append(dict(path=relative, category=category, sha256=sha(path)))
            # Generated engine INIs are retained and recorded; never delete unknown files.
            if category != "player_saved_config" or path.suffix.lower() != ".ini" or {p.casefold() for p in path.parts}.intersection(("savegames", "photos")):
                errors.append("Unexpected new protected file retained for review: " + relative)
    result = dict(schema_version=1, all_original_bytes_and_mtimes_match=not errors,
                  snapshot_sha256=sha(manifest_path), counts=manifest["counts"],
                  changes=changes, generated_files_retained=generated, errors=errors)
    write_json(guarded(root, output), result)
    if errors:
        raise ValueError("; ".join(errors))
    return result


def fingerprint(root, executable, module=None):
    root = guarded(root, root)
    paths = set()
    for folder, extensions in (("Source", {".cpp", ".h", ".cs"}), ("Config", {".ini"}),
                               ("Content/Science", None), ("Content/Lunar/Apollo12/IntrepidVisual", None)):
        for path in files_below(root, root / folder):
            if extensions is None or path.suffix.lower() in extensions:
                paths.add(path)
    for folder in ("SourceArt",):
        for path in files_below(root, root / folder):
            if any("intrepid" in p.casefold() for p in path.relative_to(root / folder).parts):
                paths.add(path)
    for name in ("LunarRescue.uproject", "Content/Lunar/Apollo12/Meshes/SM_Apollo12Terrain.uasset",
                 "Tools/build_intrepid_materials.py", "Tools/build_intrepid_cabin.py", "Tools/prepare_intrepid_cabin.py", "Tools/verify_intrepid_visual.ps1",
                 "Tools/profile_intrepid_slice.ps1", "Tools/IntrepidValidationCommon.ps1",
                 "Tools/intrepid_validation.py", "Tools/summarize_profile.py"):
        path = guarded(root, root / name)
        if path.is_file():
            paths.add(path)
    rows = [dict(path=p.relative_to(root).as_posix(), bytes=p.stat().st_size, sha256=sha(p)) for p in sorted(paths)]
    canonical = "\n".join(r["path"] + ":" + r["sha256"] for r in rows)
    exe = Path(executable)
    # UE executable may reside outside the project; it is read-only, never restored.
    executable_info = dict(path=str(exe), sha256=sha(exe), bytes=exe.stat().st_size)
    module_info = None
    if module:
        path = guarded(root, module)
        module_info = dict(path=path.relative_to(root).as_posix(), sha256=sha(path), bytes=path.stat().st_size)
    return dict(schema_version=1, input_sha256=hashlib.sha256(canonical.encode()).hexdigest(),
                input_files=rows, executable=executable_info, module=module_info)


def finite(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def vector(value, keys):
    return isinstance(value, dict) and all(finite(value.get(k)) for k in keys)


def child_file(folder, name):
    require(isinstance(name, str) and Path(name).name == name and "/" not in name and "\\" not in name,
            "Driver artifact must be a plain filename")
    path = guarded(folder, folder / name)
    require(path.is_file(), "Missing driver artifact: " + name)
    return path


def png_size(path):
    raw = path.read_bytes()
    require(raw[:8] == b"\x89PNG\r\n\x1a\n", "Invalid PNG signature: " + path.name)
    offset, size, image_data, ended = 8, None, False, False
    while offset + 12 <= len(raw):
        length = struct.unpack(">I", raw[offset:offset + 4])[0]
        kind = raw[offset + 4:offset + 8]
        end = offset + 12 + length
        require(end <= len(raw), "Truncated PNG chunk: " + path.name)
        payload = raw[offset + 8:offset + 8 + length]
        crc = struct.unpack(">I", raw[offset + 8 + length:end])[0]
        require(zlib.crc32(kind + payload) & 0xffffffff == crc, "PNG chunk CRC mismatch: " + path.name)
        if offset == 8:
            require(kind == b"IHDR" and length == 13, "PNG must begin with IHDR")
            size = struct.unpack(">II", payload[:8])
        if kind == b"IDAT":
            image_data = True
        if kind == b"IEND":
            require(length == 0 and end == len(raw), "Invalid PNG termination")
            ended = True
            break
        offset = end
    require(size and image_data and ended, "Incomplete PNG: " + path.name)
    return size


def focus_box_fits(camera):
    """Independently project the audited box using Unreal's X-forward rotator axes."""
    pitch, yaw, roll = (math.radians(camera["rotation"][key]) for key in ("pitch", "yaw", "roll"))
    cp, sp, cy, sy, cr, sr = math.cos(pitch), math.sin(pitch), math.cos(yaw), math.sin(yaw), math.cos(roll), math.sin(roll)
    axes = ((cp * cy, cp * sy, sp),
            (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp),
            (-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp))
    tan = math.tan(math.radians(camera["fov"] / 2))
    for corner in camera["focus_bounds_corners_world"]:
        delta = [corner[k] - camera["eye"][k] for k in ("x", "y", "z")]
        x, y, z = [sum(a * b for a, b in zip(delta, axis)) for axis in axes]
        if x <= 1 or abs(y) > x * tan * .920001 or abs(z) > x * tan / camera["aspect_ratio"] * .920001:
            return False
    return True


def validate(folder, kind, baseline, rendered, width, height, frames, started, log, cabin=False, cabin_baseline=False, cabin_review=False):
    require(not cabin_review or (cabin and kind == "review"), "Cabin close-camera mode is review-only")
    folder = guarded(folder, folder)
    report = child_file(folder, "checks.txt").read_text(encoding="utf-8-sig")
    passes = len(re.findall(r"(?m)^PASS ", report))
    require(passes > 0 and not re.search(r"(?m)^FAIL ", report), "Driver has no passing checks or contains FAIL")
    require(len(re.findall(r"(?m)^RESULT:", report)) == 1 and len(re.findall(r"(?m)^RESULT: 0 failures\r?$", report)) == 1,
            "Driver must report exactly one RESULT: 0 failures")
    data = json.loads(child_file(folder, "manifest.json").read_text(encoding="utf-8-sig"))
    require(data.get("schema_version") == 1 and data.get("kind") == kind, "Unexpected manifest schema/kind")
    for key, expected in (("passed", True), ("baseline", baseline), ("synthetic", True),
                          ("human_playtest", False), ("performance_measurement", kind == "profile"),
                          ("render_capable", rendered), ("stage_static_visual", kind != "test"),
                          ("field_operations_enabled", True)):
        require(data.get(key) is expected, f"Unexpected manifest {key}: {data.get(key)!r}")
    for key, expected in (("cabin_scope", cabin), ("cabin_baseline", cabin_baseline), ("cabin_review", cabin_review)):
        require(data.get(key, False) is expected, f"Unexpected manifest {key}")
    if cabin:
        require(data.get("cabin_mesh") == (CABIN_SOURCE if cabin_baseline else CABIN_DERIVED)
                and data.get("cabin_source_mesh") == CABIN_SOURCE, "Wrong cabin baseline/feature/source mesh")
        source_bounds, derived_bounds = data.get("cabin_source_bounds"), data.get("cabin_derived_bounds")
        for bounds in (source_bounds, derived_bounds):
            require(isinstance(bounds, dict) and vector(bounds.get("origin"), ("x", "y", "z"))
                    and vector(bounds.get("box_extent"), ("x", "y", "z"))
                    and all(bounds["box_extent"][k] > 0 for k in ("x", "y", "z")), "Invalid cabin source/derived bounds")
        require(all(abs(source_bounds[field][k] - derived_bounds[field][k]) <= .001
                    for field in ("origin", "box_extent") for k in ("x", "y", "z")), "Cabin derivative changed source bounds")
        require(len(data.get("cabin_pad_world", [])) == 4
                and all(vector(v, ("x", "y", "z")) for v in data["cabin_pad_world"]), "Cabin four pad anchors missing")
        materials = data.get("cabin_effective_materials", [])
        require(len(materials) > 2 and [r.get("index") for r in materials] == list(range(len(materials))), "Cabin material inventory missing")
        approved = [r for r in materials if r.get("approved_shared_white_slot") is True]
        require({r.get("slot") for r in approved} == set(CABIN_SLOTS) and len(approved) == 2, "Cabin shared white slots differ from source audit")
        for material in materials:
            white = material.get("slot") in CABIN_SLOTS
            expected = white and not cabin_baseline
            require(material.get("cabin_parent") is expected and material.get("effective_material") not in (None, "missing"), "Cabin material override leaked beyond approved slots")
            if expected:
                require(material.get("effective_parent") == CABIN_MATERIAL
                        and all(finite(material.get("tint_" + k)) and abs(material["tint_" + k] - CABIN_SLOTS[material["slot"]]) < .00001 for k in ("r", "g", "b")), "Cabin material parent/source neutral factors mismatch")
        require(isinstance(data.get("cabin_normals_validation"), str) and "Not inspected at runtime" in data["cabin_normals_validation"], "Runtime must distinguish normal-source validation")
    require(data.get("pass_assertions") == passes and data.get("fail_assertions") == 0, "Manifest/report assertion counts disagree")
    require(data.get("terrain_asset") == TERRAIN, "Expected the retained imported NASA terrain asset")
    grid = data.get("height_grid", [])
    expected_xy = [(x * 600, y * 600) for y in range(-4, 5) for x in range(-4, 5)] + [(-3000, -3000), (3000, -3000), (3000, 3000), (-3000, 3000)]
    require(len(grid) == len(expected_xy), "Expected 81 local and 4 outside terrain samples")
    for point, (x, y) in zip(grid, expected_xy):
        require(vector(point, ("x", "y", "z")) and abs(point["x"] - x) < .01 and abs(point["y"] - y) < .01,
                "Terrain coordinates/units/order are invalid")
        normal = point.get("normal")
        require(vector(normal, ("x", "y", "z")) and abs(sum(normal[k] ** 2 for k in ("x", "y", "z")) - 1) < .01,
                "Invalid terrain normal")
    for name, field in (("height-grid.csv", "height_grid_sha1"), ("outside-instances.txt", "outside_instances_sha1")):
        text = child_file(folder, name).read_bytes().decode("utf-8-sig")
        require(hashlib.sha1(text.encode("utf-8")).hexdigest() == data.get(field), "Driver collision/inventory SHA1 mismatch: " + name)
    require(isinstance(data.get("outside_instance_count"), int) and data["outside_instance_count"] > 0,
            "Outside source instance inventory is missing")
    if rendered:
        require((data.get("width"), data.get("height")) == (width, height), "Rendered viewport did not match requested size")
    if kind != "test":
        cameras = data.get("cameras", [])
        names = CABIN_CAMERAS if cabin_review else CAMERAS
        fov = 60 if cabin_review else 85
        require(len(cameras) == len(names), "Unexpected fixed camera count")
        for index, camera in enumerate(cameras):
            require(camera.get("name") == names[index] and vector(camera.get("eye"), ("x", "y", "z"))
                    and vector(camera.get("rotation"), ("pitch", "yaw", "roll")) and camera.get("fov") == fov,
                    "Invalid fixed camera definition")
            if cabin_review:
                require(vector(camera.get("target"), ("x", "y", "z")) and camera.get("focus_box_in_frustum") is True
                        and finite(camera.get("fit_pullback")) and 1 <= camera["fit_pullback"] <= 2
                        and math.isclose(camera.get("aspect_ratio", 0), width / height, rel_tol=1e-9)
                        and len(camera.get("focus_bounds_corners_world", [])) == 8
                        and all(vector(v, ("x", "y", "z")) for v in camera["focus_bounds_corners_world"]),
                        "Cabin focus-box/FOV/frustum metadata invalid")
                require(focus_box_fits(camera), "Audited cabin box does not independently fit the reported frustum")
    if kind == "review":
        shots = data.get("frames", [])
        require(len(shots) == len(names) * 2, "Expected each fixed camera at both qualities")
        for index, shot in enumerate(shots):
            camera, quality = index % len(names), index // len(names)
            name = names[camera] + f"-q{quality}"
            require(shot.get("name") == name and shot.get("path") == name + ".png"
                    and shot.get("camera_index") == camera and shot.get("quality") == quality
                    and shot.get("actual_quality") == quality, "Review image order/camera/actual scalability mismatch")
            require(shot.get("eye") == data["cameras"][camera]["eye"]
                    and shot.get("rotation") == data["cameras"][camera]["rotation"] and shot.get("fov") == fov,
                    "Review pose does not match fixed camera")
            require(png_size(child_file(folder, shot["path"])) == (width, height), "PNG dimensions differ from viewport")
    if kind == "profile":
        require(data.get("captured_frames") == frames and data.get("requested_profile_frames") == frames
                and data.get("settle_frames") == 300, "CSV frame/settle contract mismatch")
        phases = data.get("camera_phases", [])
        require(len(phases) == 4, "Expected four camera phases")
        for index, phase in enumerate(phases):
            require(phase.get("camera_index") == index and phase.get("first_frame") == index * frames // 4
                    and phase.get("last_frame") == (index + 1) * frames // 4 - 1,
                    "CSV camera phase boundaries are not the requested equal quarters")
            require(phase.get("eye") == data["cameras"][index]["eye"]
                    and phase.get("rotation") == data["cameras"][index]["rotation"], "CSV phase pose mismatch")
        require(data.get("csv_file") == "intrepid-profile.csv", "Unexpected CSV filename")
        child_file(folder, data["csv_file"])
    log_text = Path(log).read_text(encoding="utf-8-sig", errors="replace")
    require(f"INTREPID_VISUAL_{kind.upper()} COMPLETE" in log_text, "Driver completion marker absent")
    require(not re.search(r"Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|(?m:^.*Log[^\r\n:]*:\s*Error:)", log_text),
            "Engine reported an error; inspect fresh log")
    threshold = datetime.fromisoformat(started).timestamp()
    inventory = []
    for path in files_below(folder, folder):
        require(path.stat().st_mtime >= threshold, "Driver artifact predates this process: " + path.name)
        inventory.append(dict(path=path.relative_to(folder).as_posix(), bytes=path.stat().st_size, sha256=sha(path)))
    return dict(passed=True, kind=kind, assertions=passes, artifacts=inventory, manifest_sha256=sha(folder / "manifest.json"))


def summarize_csv(csv_path, frames, fps_cap, output):
    csv.field_size_limit(20 * 1024 * 1024)
    with csv_path.open(encoding="utf-8-sig", newline="") as handle:
        reader = csv.reader(handle)
        header = next(reader)
        index = header.index("FrameTime")
        valid, nonnumeric, invalid = [], 0, 0
        for row in reader:
            try:
                value = float(row[index])
            except (ValueError, IndexError):
                nonnumeric += 1
                continue  # Same header/footer handling as summarize_profile.py.
            if not math.isfinite(value) or value <= 0:
                invalid += 1
            else:
                valid.append(row)
    require(invalid == 0 and len(valid) == frames, f"CSV expected {frames} positive finite frames; valid={len(valid)}, invalid={invalid}")
    command = [sys.executable, str(Path(__file__).with_name("summarize_profile.py")), str(csv_path),
               "--warmup", "0", "--expected-frames", str(frames), "--fps-cap", str(fps_cap), "--output", str(output)]
    subprocess.run(command, check=True, capture_output=True, text=True, encoding="utf-8")
    data = json.loads(output.read_text())
    counters = {}
    for name in ("GameThreadTime", "RenderThreadTime", "GPUTime", "GPU/SlateUI", "GPUMem/LocalUsedMB"):
        if name not in header:
            counters[name] = dict(present=False, rows=frames, finite_rows=0, positive_rows=0, mean_including_zero=None)
            continue
        column = header.index(name)
        values = []
        for row in valid:
            try:
                value = float(row[column])
                if math.isfinite(value):
                    values.append(value)
            except (ValueError, IndexError):
                pass
        positive = sum(v > 0 for v in values)
        counters[name] = dict(present=True, rows=frames, finite_rows=len(values), positive_rows=positive,
                              mean_including_zero=statistics.mean(values) if values and positive else None)
    data.update(workload="Four fixed cameras; frozen mission/pawn; production parked support actors enabled; normal engine delta",
                settling_frames_before_csv=300, csv_nonnumeric_header_footer_rows=nonnumeric,
                csv_nonpositive_or_nonfinite_frame_rows=invalid, optional_counters=counters,
                limitations="One rendered offscreen four-camera run at Medium with normal delta time, VSync off and the recorded FPS cap. The 300 settling frames occur before capture and no CSV frames are discarded. Baseline is an opt-out counterfactual in this same binary, not an earlier executable. Not occupied driving, a human playtest, universal FPS, animation-smoothness proof, true peak memory or VRAM. Zero/unavailable GPU counters do not establish GPU measurement.")
    write_json(output, data)
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("snapshot", "restore", "fingerprint", "validate", "csv"))
    parser.add_argument("--root", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--module", type=Path)
    parser.add_argument("--folder", type=Path)
    parser.add_argument("--kind", choices=("test", "review", "profile"))
    parser.add_argument("--baseline", action="store_true")
    parser.add_argument("--cabin", action="store_true")
    parser.add_argument("--cabin-baseline", action="store_true")
    parser.add_argument("--cabin-review", action="store_true")
    parser.add_argument("--rendered", action="store_true")
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--frames", type=int, default=6000)
    parser.add_argument("--fps-cap", type=int, default=60)
    parser.add_argument("--started")
    parser.add_argument("--log", type=Path)
    args = parser.parse_args()
    if args.action == "snapshot":
        data = snapshot(args.root, args.output)
    elif args.action == "restore":
        data = restore(args.root, args.manifest, args.output)
    elif args.action == "fingerprint":
        data = fingerprint(args.root, args.exe, args.module)
        write_json(guarded(args.root, args.output), data)
    elif args.action == "validate":
        data = validate(args.folder, args.kind, args.baseline, args.rendered, args.width, args.height,
                        args.frames, args.started, args.log, args.cabin, args.cabin_baseline, args.cabin_review)
        write_json(guarded(args.root, args.output), data)
    else:
        data = summarize_csv(args.folder / "intrepid-profile.csv", args.frames, args.fps_cap, guarded(args.root, args.output))
        if args.cabin:
            data.update(cabin_scope=True, cabin_baseline=args.cabin_baseline, intrepid_visual_baseline=args.baseline,
                        comparison_scope="Cabin-only opt-out in the same final binary; the independent visual-baseline flag controls the prior Nov11 layer. Four original camera phases remain unchanged.")
            data["limitations"] += " CabinBaseline controls only the derivative/shared white finish; VisualBaseline independently controls the earlier art layer."
            write_json(args.output, data)
    print(json.dumps(dict(action=args.action, passed=True, output=str(args.output))))


if __name__ == "__main__":
    main()
