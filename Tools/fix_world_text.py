import json
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir())
levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
expected = {
    'SELENE branding',
    'Station number',
    'Power label',
    'Data label',
    'Rover ID',
    'Lander label',
}
fixed = []
for actor in actors.get_all_level_actors():
    label = actor.get_actor_label()
    if label not in expected:
        continue
    if not actor.get_component_by_class(u.TextRenderComponent):
        raise RuntimeError(f'{label} is no longer a text actor')
    before = actor.get_actor_rotation()
    actor.set_actor_rotation(u.Rotator(pitch=0.0, yaw=180.0, roll=0.0), False)
    after = actor.get_actor_rotation()
    if abs(after.pitch) > 0.01 or abs(abs(after.yaw) - 180.0) > 0.01 or abs(after.roll) > 0.01:
        raise RuntimeError(f'{label} rotation did not apply: {after}')
    fixed.append({'label': label, 'before': [before.pitch, before.yaw, before.roll], 'after': [after.pitch, after.yaw, after.roll]})

if {item['label'] for item in fixed} != expected:
    raise RuntimeError('Missing text actors: ' + str(expected - {item['label'] for item in fixed}))
levels.save_current_level()
(root / 'Reports' / 'world-text-fix.json').write_text(json.dumps(fixed, indent=2))
u.log('WORLD_TEXT_FIXED ' + str(len(fixed)))
