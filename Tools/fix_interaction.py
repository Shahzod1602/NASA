import unreal as u
from pathlib import Path
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')
actors=u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
rows=[]
for a in actors:
    comp=a.get_component_by_class(u.StaticMeshComponent)
    if comp and comp.get_collision_enabled()==u.CollisionEnabled.NO_COLLISION:
        comp.set_collision_profile_name('NoCollision')
        a.set_actor_enable_collision(False)
    if a.get_name()=='StaticMeshActor_529':
        rows.append('BLOCKER: '+a.get_actor_label()+' '+str(a.get_actor_location()))
    if a.get_actor_label() in ['Power socket','Terminal screen','Return console display','Module beacon']:
        c=a.get_component_by_class(u.StaticMeshComponent)
        rows.append(a.get_actor_label()+' prior='+str(c.get_collision_enabled()))
        c.set_collision_profile_name('NoCollision')
        c.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
        a.set_actor_enable_collision(False)
    if a.get_actor_label()=='Low lunar sun':
        a.light_component.set_editor_property('forward_shading_priority',1)
levels.save_current_level()
(Path(u.Paths.project_dir())/'Reports/interaction-fix.txt').write_text('\n'.join(rows))
