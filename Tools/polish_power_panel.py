"""Polish the existing power port without moving mission targets or rebuilding the map."""
import json
import shutil
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir())
backup = root / 'Backups' / 'BeforeAuditFixes' / 'MoonBase.umap'
backup.parent.mkdir(parents=True, exist_ok=True)
if not backup.exists():
    shutil.copy2(root / 'Content/Lunar/Maps/MoonBase.umap', backup)
levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')
all_actors = list(actors.get_all_level_actors())
by_label = {a.get_actor_label(): a for a in all_actors}
for label in ('Power port', 'Power socket', 'Power label'):
    if label not in by_label:
        raise RuntimeError('Missing original actor: ' + label)
for a in all_actors:
    if a.get_actor_label().startswith('AuditPanel_'):
        actors.destroy_actor(a)
lib = u.EditorAssetLibrary
graphite = lib.load_asset('/Game/Lunar/Materials/M_Graphite')
ceramic = lib.load_asset('/Game/Lunar/Materials/M_Ceramic')
signal = lib.load_asset('/Game/Lunar/Materials/M_Signal')
foil = lib.load_asset('/Game/Lunar/Materials/M_ThermalFoil')
cube = lib.load_asset('/Engine/BasicShapes/Cube')
if not all((graphite, ceramic, signal, foil, cube)):
    raise RuntimeError('Panel materials unavailable')
port = by_label['Power port']
port.set_actor_scale3d(u.Vector(.65, .85, 1.15))
socket = by_label['Power socket']
socket.set_actor_location(u.Vector(-116, -1000, 120), False, False)
socket.set_actor_scale3d(u.Vector(.035, .61, .40))
socket.static_mesh_component.set_material(0, graphite)
label = by_label['Power label']
label.set_actor_location(u.Vector(-120, -1000, 138), False, False)
text = label.get_component_by_class(u.TextRenderComponent)
text.set_text('SELENE / PWR-01')
text.set_world_size(5)

def detail(name, pos, scale, material):
    a = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector(*pos))
    a.set_actor_label('AuditPanel_' + name)
    c = a.static_mesh_component
    c.set_static_mesh(cube)
    c.set_material(0, material)
    c.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    a.set_actor_enable_collision(False)
    a.set_actor_scale3d(u.Vector(*scale))

for y in (-1040, -960):
    detail('edge', (-114, y, 90), (.035, .025, 1.10), ceramic)
for z in (36, 144):
    detail('rail', (-114, -1000, z), (.035, .82, .025), ceramic)
for z in (111, 118, 125, 132):
    detail('vent', (-119, -1012, z), (.018, .28, .012), ceramic)
detail('status', (-119, -981, 130), (.02, .075, .025), signal)
detail('socket', (-119, -972, 116), (.028, .08, .08), foil)
levels.save_current_level()
(root / 'Reports/panel-polish.json').write_text(json.dumps({
    'map_saved': True, 'backup': str(backup),
    'mission_target_location_unchanged': str(port.get_actor_location()),
    'panel_scale': str(port.get_actor_scale3d()),
}, indent=2))
u.log('LUNAR_PANEL_POLISH_COMPLETE')
