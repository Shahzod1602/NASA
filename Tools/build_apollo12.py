"""Import prepared NASA terrain and build a separate Apollo12 map in Unreal."""
import unreal as u
import json, math, random
from pathlib import Path
ROOT=Path(u.Paths.project_dir()).resolve(); ART=ROOT/'SourceArt/Apollo12'
lib=u.EditorAssetLibrary; assets=u.AssetToolsHelpers.get_asset_tools()
actors=u.get_editor_subsystem(u.EditorActorSubsystem); levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
data=json.loads((ART/'manifest.json').read_text()); hf=json.loads((ART/'heightfield.json').read_text())
u.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.OBJ 0')
opts=u.FbxImportUI(); opts.automated_import_should_detect_type=False
opts.import_materials=False; opts.import_textures=False; opts.import_as_skeletal=False; opts.import_mesh=True
opts.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
d=opts.static_mesh_import_data; d.combine_meshes=True; d.auto_generate_collision=False
d.generate_lightmap_u_vs=False; d.set_editor_property('convert_scene',True); d.set_editor_property('convert_scene_unit',False)
d.set_editor_property('normal_import_method',u.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
task=u.AssetImportTask(); task.filename=str(ART/'SM_Apollo12Terrain.obj'); task.destination_path='/Game/Lunar/Apollo12/Meshes'
task.destination_name='SM_Apollo12Terrain'; task.automated=True; task.replace_existing=True; task.save=True
task.factory=u.FbxFactory(); task.options=opts; assets.import_asset_tasks([task])
terrain=lib.load_asset('/Game/Lunar/Apollo12/Meshes/SM_Apollo12Terrain')
if not terrain:raise RuntimeError('Terrain import failed')
body=terrain.get_editor_property('body_setup'); body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
body.set_editor_property('double_sided_geometry',True); lib.save_loaded_asset(terrain,only_if_is_dirty=False)
task=u.AssetImportTask(); task.filename=str(ART/'T_Apollo12_Orbital.png'); task.destination_path='/Game/Lunar/Apollo12/Textures'
task.destination_name='T_Apollo12_Orbital';task.automated=True;task.replace_existing=True;task.save=True;assets.import_asset_tasks([task])
tex=lib.load_asset('/Game/Lunar/Apollo12/Textures/T_Apollo12_Orbital');tex.set_editor_property('never_stream',True)
lib.save_loaded_asset(tex,only_if_is_dirty=False)
dest='/Game/Lunar/Maps/Apollo12'
if lib.does_asset_exist(dest):
    if not levels.load_level(dest):raise RuntimeError('Could not load Apollo12')
    for a in list(actors.get_all_level_actors()):
        if isinstance(a,(u.StaticMeshActor,u.Light,u.PlayerStart,u.TextRenderActor)):actors.destroy_actor(a)
elif not levels.new_level(dest):raise RuntimeError('Could not create Apollo12')
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode',u.load_class(None,'/Game/Lunar/Blueprints/BP_LunarGameMode.BP_LunarGameMode_C'))
reg=lib.load_asset('/Game/Lunar/V3/Materials/M_LunarRegolith')
def mesh(name,asset,p,scale=(1,1,1),mat=None,collision=False,rot=(0,0,0)):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*p),u.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]))
    a.set_actor_label(name);c=a.static_mesh_component
    c.set_static_mesh(lib.load_asset(asset) if isinstance(asset,str) else asset)
    if mat:c.set_material(0,mat)
    a.set_actor_scale3d(u.Vector(*scale));c.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    a.set_actor_enable_collision(collision);return a
mesh('NASA LROC Apollo 12 terrain / 2m grid',terrain,(0,0,0),mat=reg,collision=True)
# Existing model is a visual approximation; exact work sites come from the mapping dataset.
lander=mesh('Intrepid / Apollo lunar module','/Game/Lunar/Imported/ApolloLander/ApolloLander',(0,0,0))
def ground(x,y):
    fx=(x-hf['x0'])/200;fy=(y-hf['y0'])/200;i=int(fx);j=int(fy);u1=fx-i;v=fy-j
    z=hf['z'];a,b,c,d=z[j][i],z[j][i+1],z[j+1][i],z[j+1][i+1]
    return a*(1-u1)+b*(u1-v)+d*v if u1>=v else a*(1-v)+c*(v-u1)+d*u1
start=data['sites']['check']['xyz'];actors.spawn_actor_from_class(u.PlayerStart,u.Vector(start[0]-300,start[1]-100,ground(start[0]-300,start[1]-100)+100),u.Rotator(pitch=0,yaw=20,roll=0))
sun=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,9000),u.Rotator(pitch=-15,yaw=180,roll=0))
sun.set_actor_label('Low lunar sunlight / approximate mission lighting');c=sun.light_component;c.set_mobility(u.ComponentMobility.MOVABLE)
c.set_intensity(4.2);c.set_light_color(u.LinearColor(1,.98,.94));c.set_editor_property('forward_shading_priority',1);c.set_editor_property('light_source_angle',.53)
c.set_editor_property('dynamic_shadow_distance_movable_light',35000)
fill=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,9000),u.Rotator(pitch=-65,yaw=0,roll=0))
fill.set_actor_label('Readability fill');fill.light_component.set_mobility(u.ComponentMobility.MOVABLE)
fill.light_component.set_intensity(.35);fill.light_component.set_cast_shadows(False)
bounce=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,9000),u.Rotator(pitch=-35,yaw=180,roll=0))
bounce.set_actor_label('Readability bounce');bounce.light_component.set_mobility(u.ComponentMobility.MOVABLE)
bounce.light_component.set_intensity(.25);bounce.light_component.set_cast_shadows(False)
# Approximate sub-Earth geometry at the landing latitude/longitude; not a 1969 ephemeris.
dist=900000;az=math.radians(-7);el=math.radians(66)
p=(dist*math.cos(el)*math.cos(az),dist*math.cos(el)*math.sin(az),dist*math.sin(el))
diam=2*dist*math.tan(math.radians(.95))/100
earth=mesh('Earth / photographic visual approximation','/Engine/BasicShapes/Sphere',p,(diam,)*3,lib.load_asset('/Game/Lunar/V3/Materials/M_EarthPhotographic'),rot=(0,120,12))
earth.static_mesh_component.set_cast_shadow(False)
# Cosmetic detail is separate from the untouched 2m collision surface.
rng=random.Random(19691120);rockmat=lib.load_asset('/Game/Lunar/V3/Materials/M_FracturedBasalt')
for i in range(650):
    x=rng.uniform(-48000,32000);y=rng.uniform(-26000,37000)
    if abs(x)<700 and abs(y)<700:continue
    scale=rng.uniform(.08,.75)
    a=mesh('Authored small basalt detail',f'/Game/Lunar/V3/Meshes/SM_Basalt_{i%6}',(x,y,ground(x,y)),(scale,scale*.85,scale*.65),rockmat,rot=(0,rng.uniform(0,360),0))
    a.static_mesh_component.set_editor_property('ld_max_draw_distance',15000)
levels.save_current_level();lib.save_directory('/Game/Lunar/Apollo12',recursive=True)
b=terrain.get_bounds(); lb=lander.static_mesh_component.get_editor_property('static_mesh').get_bounds()
(ROOT/'Reports/apollo12-map-build.json').write_text(json.dumps(dict(terrain_bounds=str(b),lander_bounds=str(lb),actors=len(actors.get_all_level_actors()),triangles=data['triangles']),indent=2))
u.log('APOLLO12_MAP_READY')
