import json
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir())
levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
rows = []
for actor in actors.get_all_level_actors():
    component = actor.get_component_by_class(u.TextRenderComponent)
    if not component:
        continue
    p = actor.get_actor_location()
    r = actor.get_actor_rotation()
    rows.append({
        'label': actor.get_actor_label(),
        'location': [p.x, p.y, p.z],
        'rotation': [r.pitch, r.yaw, r.roll],
        'text': str(component.get_editor_property('text')),
    })
target = root / 'Reports' / 'world-text-before.json'
target.write_text(json.dumps(rows, indent=2))
u.log('WORLD_TEXT_INSPECT ' + str(target))
