import json
from pathlib import Path
import unreal as u

root=Path(u.Paths.project_dir())
library=u.EditorAssetLibrary
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')
before=list(actors.get_all_level_actors())
bylabel={}
for actor in before:
    bylabel.setdefault(actor.get_actor_label(),[]).append(actor)

def mesh_asset(name):
    path='/Game/Lunar/Imported/'+name+'/'+name
    obj=library.load_asset(path)
    if not obj or not isinstance(obj,u.StaticMesh): raise RuntimeError('Missing imported mesh '+path)
    return obj

for name in ('ApolloLander','HabitatMain','HabitatUtility','Lunokhod','PowerCell','ScienceTerminal','SolarPanel'):
    mesh_asset(name)

remove_labels={
    'Habitat insulated shell','Habitat top','Orange identification band',
    'Window seal','Window glass','Door frame','Door',
    'Rover chassis','Rover wheel','Rover solar panel',
    'Lander body','Lander upper','Lander strut','Lander foot',
    'Array support','Solar panel','Solar cell divider'
}
removed=0
for actor in before:
    if actor.get_actor_label() in remove_labels or actor.get_actor_label().startswith('Imported_'):
        actors.destroy_actor(actor)
        removed+=1

def replace_target(label,asset,location):
    matches=bylabel.get(label,[])
    if len(matches)!=1: raise RuntimeError('Expected one '+label+': '+str(len(matches)))
    actor=matches[0]
    actor.static_mesh_component.set_static_mesh(mesh_asset(asset))
    actor.set_actor_scale3d(u.Vector(1,1,1))
    actor.set_actor_location(u.Vector(*location),False,False)
    actor.static_mesh_component.set_collision_profile_name('BlockAll')
    actor.set_actor_enable_collision(True)
    return actor

replace_target('Spare power module','PowerCell',(3960,-500,22))
replace_target('Science terminal','ScienceTerminal',(-80,-1500,18))

def place(name,location,rotation=(0,0,0),collision=True):
    actor=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*location),u.Rotator(*rotation))
    actor.set_actor_label('Imported_'+name)
    actor.static_mesh_component.set_static_mesh(mesh_asset(name))
    actor.set_actor_scale3d(u.Vector(1,1,1))
    actor.static_mesh_component.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    actor.set_actor_enable_collision(collision)
    return actor

place('HabitatMain',(1050,-1000,140))
place('HabitatUtility',(1770,-1000,140))
place('ApolloLander',(-1900,-300,0))
place('Lunokhod',(4300,-250,0),(0,15,0))
place('SolarPanel',(1500,-2700,145),(0,0,12),False)
place('SolarPanel',(1500,-3700,145),(0,0,12),False)

tags=('Module','Power','Data','Home')
allactors=list(actors.get_all_level_actors())
tag_counts={tag:sum(tag in [str(t) for t in a.get_editor_property('tags')] for a in allactors) for tag in tags}
if any(n!=1 for n in tag_counts.values()): raise RuntimeError('Mission tag mismatch '+str(tag_counts))
levels.save_current_level()
report={'removed_prototypes':removed,'placed_models':['HabitatMain','HabitatUtility','ApolloLander','Lunokhod','SolarPanel x2'],'replaced_targets':['Spare power module','Science terminal'],'tag_counts':tag_counts,'actor_count':len(allactors)}
(root/'Reports/imported-models.json').write_text(json.dumps(report,indent=2))
u.log('LUNAR_MODELS_PLACED '+str(report))
