"""Idempotent station lighting and signal actors for the MoonBase level."""
import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir()).resolve()
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
lib=u.EditorAssetLibrary
tools=u.AssetToolsHelpers.get_asset_tools()
matlib=u.MaterialEditingLibrary
levels.load_level('/Game/Lunar/Maps/MoonBase')
for a in list(actors.get_all_level_actors()):
    if a.get_actor_label().startswith('PowerSeq_'):actors.destroy_actor(a)

def emissive(name,color):
    folder='/Game/Lunar/PowerSequence'
    m=lib.load_asset(folder+'/'+name) if lib.does_asset_exist(folder+'/'+name) else tools.create_asset(name,folder,u.Material,u.MaterialFactoryNew())
    matlib.delete_all_material_expressions(m)
    m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
    c=matlib.create_material_expression(m,u.MaterialExpressionConstant3Vector)
    c.constant=u.LinearColor(*color)
    matlib.connect_material_property(c,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    matlib.recompile_material(m);lib.save_loaded_asset(m)
    return m
blue=emissive('M_StationOnline',(.12,2.2,3.5))
red=emissive('M_StationEmergency',(3.3,.06,.025))
sphere=lib.load_asset('/Engine/BasicShapes/Sphere')
def bulb(name,pos,material,tag):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*pos))
    a.set_actor_label('PowerSeq_'+name)
    a.set_editor_property('tags',[tag])
    c=a.static_mesh_component;c.set_static_mesh(sphere);c.set_material(0,material)
    c.set_collision_profile_name('NoCollision');a.set_actor_enable_collision(False);c.set_cast_shadow(False)
    a.set_actor_scale3d(u.Vector(.22,.22,.22))
    return a
def lamp(name,pos,color,tag):
    a=actors.spawn_actor_from_class(u.PointLight,u.Vector(*pos))
    a.set_actor_label('PowerSeq_'+name);a.set_editor_property('tags',[tag])
    c=a.light_component;c.set_mobility(u.ComponentMobility.MOVABLE)
    c.set_light_color(u.LinearColor(*color));c.set_intensity(0)
    c.set_attenuation_radius(950);c.set_cast_shadows(False)
    return a
for i,y in enumerate((-1360,-1120,-880,-640)):
    p=(515,y,565)
    bulb(f'roof indicator {i}',p,blue,f'PowerBulb{i}')
    lamp(f'roof wash {i}',(p[0]-55,p[1],p[2]-18),(.12,.65,1),f'PowerLight{i}')
bulb('emergency signal',(505,-1550,550),red,'PowerWarningBulb')
lamp('emergency red wash',(465,-1550,545),(1,.035,.016),'PowerWarningLight')
for a in actors.get_all_level_actors():
    if a.get_actor_label()=='Relay dish':
        a.static_mesh_component.set_mobility(u.ComponentMobility.MOVABLE)
        if 'PowerAntenna' not in [str(t) for t in a.get_editor_property('tags')]:
            a.set_editor_property('tags',list(a.get_editor_property('tags'))+['PowerAntenna'])
    if a.get_actor_label()=='Terminal screen':
        if 'PowerScreen' not in [str(t) for t in a.get_editor_property('tags')]:
            a.set_editor_property('tags',list(a.get_editor_property('tags'))+['PowerScreen'])
levels.save_current_level()
for wav in (root/'SourceArt/Immersion').glob('Power*.wav'):
    task=u.AssetImportTask();task.filename=str(wav)
    task.destination_path='/Game/Lunar/Immersion/Audio'
    task.automated=True;task.replace_existing=True;task.save=True
    tools.import_asset_tasks([task])
u.log('POWER_SEQUENCE_SCENE_READY')
