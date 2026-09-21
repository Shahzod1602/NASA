"""Build the authored Mars canyon and mission selection map; never edits MoonBase."""
import unreal as u
import math, random
from pathlib import Path
ROOT=Path(u.Paths.project_dir()).resolve();ART=ROOT/'SourceArt/Mars';ART.mkdir(parents=True,exist_ok=True)
lib=u.EditorAssetLibrary;assets=u.AssetToolsHelpers.get_asset_tools();edit=u.MaterialEditingLibrary
actors=u.get_editor_subsystem(u.EditorActorSubsystem);levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
def height(x,y):
    edge=max(abs(y)-5500,0)/2800
    return (min(edge,1)**2)*(1200+700*math.sin(x*.0003)**2)+max(abs(x)-26000,0)*.07
path=ART/'SM_MarsTerrain.obj';n=161;extent=48000
with path.open('w') as f:
    f.write('o SM_MarsTerrain\n')
    for j in range(n):
        y=-extent+2*extent*j/(n-1)
        for i in range(n):
            x=-extent+2*extent*i/(n-1);f.write(f'v {x} {-y} {height(x,y)}\n')
    f.write('s 1\n')
    for j in range(n-1):
        for i in range(n-1):
            a=j*n+i+1;b=a+1;c=a+n;d=c+1;f.write(f'f {a} {d} {b}\nf {a} {c} {d}\n')
# Faceted, eroded mesa instead of a basic cylinder silhouette.
mesa_path=ART/'SM_MarsMesa.obj';segments=17
with mesa_path.open('w') as f:
    f.write('o SM_MarsMesa\n')
    for ring,(z,radius) in enumerate([(0,1.15),(18,1.02),(55,.82),(88,.72),(100,.65)]):
        for i in range(segments):
            a=i*math.tau/segments;r=50*radius*(1+.16*math.sin(i*2.7)+.09*math.cos(i*4.1+ring*.6))
            f.write(f'v {r*math.cos(a):.3f} {-r*math.sin(a):.3f} {z+math.sin(i*3.4)*2:.3f}\n')
    for ring in range(4):
        for i in range(segments):
            a=ring*segments+i+1;b=ring*segments+(i+1)%segments+1;c=a+segments;d=b+segments
            f.write(f'f {a} {c} {b}\nf {b} {c} {d}\n')
    f.write('v 0 0 100\n')
    for i in range(segments):f.write(f'f 86 {69+(i+1)%segments} {69+i}\n')
u.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.OBJ 0')
opts=u.FbxImportUI();opts.automated_import_should_detect_type=False;opts.import_materials=False;opts.import_textures=False;opts.import_as_skeletal=False;opts.import_mesh=True;opts.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=True;opts.static_mesh_import_data.generate_lightmap_u_vs=False
for path in (ART/'SM_MarsTerrain.obj',mesa_path):
    job=u.AssetImportTask();job.filename=str(path);job.destination_path='/Game/Mars/Meshes';job.destination_name=path.stem;job.automated=True;job.replace_existing=True;job.save=True;job.factory=u.FbxFactory();job.options=opts;assets.import_asset_tasks([job])
terrain=lib.load_asset('/Game/Mars/Meshes/SM_MarsTerrain');body=terrain.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);body.set_editor_property('double_sided_geometry',True);lib.save_loaded_asset(terrain)
def material(name,color,code=None,sky=False):
    dest='/Game/Mars/Materials';m=lib.load_asset(dest+'/'+name) if lib.does_asset_exist(dest+'/'+name) else None
    if m:return m
    m=assets.create_asset(name,dest,u.Material,u.MaterialFactoryNew())
    if sky:m.set_editor_property('two_sided',True);m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
    if code:
        pos=edit.create_material_expression(m,u.MaterialExpressionWorldPosition)
        c=edit.create_material_expression(m,u.MaterialExpressionCustom);c.set_editor_property('code',code);c.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT3)
        inp=u.CustomInput();inp.set_editor_property('input_name','P');c.set_editor_property('inputs',[inp]);edit.connect_material_expressions(pos,'',c,'P')
    else:
        c=edit.create_material_expression(m,u.MaterialExpressionConstant3Vector);c.set_editor_property('constant',u.LinearColor(*color))
    edit.connect_material_property(c,'',u.MaterialProperty.MP_EMISSIVE_COLOR if sky else u.MaterialProperty.MP_BASE_COLOR)
    r=edit.create_material_expression(m,u.MaterialExpressionConstant);r.set_editor_property('r',.94);edit.connect_material_property(r,'',u.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(m);lib.save_loaded_asset(m);return m
dust=material('M_MarsRegolith',(.32,.10,.035),'float n=frac(sin(dot(floor(P*.22),float3(12.989,78.23,39.4)))*43758.5);float dune=sin(P.x*.006+sin(P.y*.008)*2);return float3(.32,.105,.039)*(.8+n*.3+dune*.12);')
cliff=material('M_MarsCliff',(.23,.07,.025),'float band=sin(P.z*.024+sin(P.x*.0007)*2);return lerp(float3(.16,.048,.018),float3(.38,.15,.065),band*.35+.5);')
sky=material('M_MarsSky',(.2,.12,.085),'float a=saturate(P.z/90000);return lerp(float3(.39,.23,.14),float3(.075,.10,.14),a);',True)
white=lib.load_asset('/Game/Lunar/Materials/M_Ceramic');dark=lib.load_asset('/Game/Lunar/Materials/M_Graphite');cyan=lib.load_asset('/Game/Lunar/Materials/M_Signal');gold=lib.load_asset('/Game/Lunar/Materials/M_ThermalFoil');red=lib.load_asset('/Game/Lunar/Materials/M_Warning');blue=lib.load_asset('/Game/Lunar/Materials/M_Solar')
def mesh(name,asset,p,scale=(1,1,1),mat=None,tag=None,collision=True,rot=(0,0,0)):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*p),u.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]));a.set_actor_label(name);c=a.static_mesh_component
    c.set_static_mesh(lib.load_asset(asset) if isinstance(asset,str) else asset)
    if mat:c.set_material(0,mat)
    a.set_actor_scale3d(u.Vector(*scale));c.set_collision_profile_name('BlockAll' if collision else 'NoCollision');a.set_actor_enable_collision(collision)
    if tag:a.tags=[tag]
    return a
def shape(name,kind,p,scale,mat,**kw):return mesh(name,'/Engine/BasicShapes/'+kind,p,scale,mat,**kw)
def imported(name,p,scale=(1,1,1),tag=None):return mesh(name,'/Game/Lunar/Imported/'+name+'/'+name,p,scale,tag=tag,collision=False)
def point(name,p,tag,color=(.1,.6,1)):
    a=actors.spawn_actor_from_class(u.PointLight,u.Vector(*p));a.set_actor_label(name);a.tags=[tag];c=a.light_component;c.set_mobility(u.ComponentMobility.MOVABLE);c.set_intensity(0);c.set_attenuation_radius(650);c.set_cast_shadows(False);c.set_light_color(u.LinearColor(*color));return a
def setup():
    w=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world();ws=w.get_world_settings();ws.set_editor_property('default_game_mode',u.load_class(None,'/Game/Lunar/Blueprints/BP_LunarGameMode.BP_LunarGameMode_C'))
def fresh(path):
    assert path in ('/Game/Mars/Maps/MarsBase','/Game/Rescue/Maps/MissionSelect')
    if lib.does_asset_exist(path):
        if not levels.load_level(path):raise RuntimeError('Cannot load '+path)
        for a in actors.get_all_level_actors():
            if isinstance(a,(u.StaticMeshActor,u.Light,u.PlayerStart,u.TextRenderActor)):actors.destroy_actor(a)
    elif not levels.new_level(path):raise RuntimeError('Cannot create '+path)
fresh('/Game/Mars/Maps/MarsBase');setup()
mesh('Mars canyon terrain',terrain,(0,0,0),mat=dust)
sky_actor=shape('Atmospheric sky','Sphere',(0,0,0),(2200,2200,2200),sky,collision=False);sky_actor.static_mesh_component.set_cast_shadow(False)
sun=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,8000),u.Rotator(pitch=-28,yaw=-35,roll=0));sun.light_component.set_mobility(u.ComponentMobility.MOVABLE);sun.light_component.set_editor_property('forward_shading_priority',1);sun.light_component.set_intensity(3.3);sun.light_component.set_light_color(u.LinearColor(1,.80,.63))
fill=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,8000),u.Rotator(pitch=-65,yaw=145,roll=0));fill.light_component.set_mobility(u.ComponentMobility.MOVABLE);fill.light_component.set_intensity(.65);fill.light_component.set_cast_shadows(False);fill.light_component.set_light_color(u.LinearColor(.65,.72,1))
actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-7350,900,95),u.Rotator(0,0,0))
rng=random.Random(2035)
for i in range(230):
    x=rng.uniform(-30000,30000);y=rng.uniform(-19000,19000)
    if -10000<x<21000 and -5400<y<2800:continue
    sc=rng.uniform(.7,3.5);mesh('Weathered canyon rock',f'/Game/Lunar/V3/Meshes/SM_Basalt_{i%6}',(x,y,height(x,y)),(sc,sc,sc*.8),cliff,rot=(0,rng.uniform(0,360),0))
for i in range(25):
    x=-32000+i*2800;y=(8500 if i%2 else -8500)+rng.uniform(-1000,1000)
    mesh('Layered mesa','/Game/Mars/Meshes/SM_MarsMesa',(x,y,min(height(x+dx,y+dy) for dx in (-2400,0,2400) for dy in (-2400,0,2400))-350),(rng.uniform(20,34),rng.uniform(17,26),rng.uniform(16,30)),cliff,rot=(0,rng.uniform(0,360),0))
# ARES outpost is assembled independently of SELENE.
shape('Habitat deck','Cube',(650,-3600,40),(14,18,.8),dark)
imported('HabitatMain',(850,-3500,100),(.8,.8,.8));imported('HabitatUtility',(1700,-3300,80),(.65,.65,.65))
for i in range(4):imported('SolarPanel',(1400+i*650,-5100,30),(.8,.8,.8))
shape('External power cabinet','Cube',(-160,-3000,80),(1.5,1.2,1.6),white,tag='Power')
imported('ScienceTerminal',(-240,-4500,95),tag='Data')
shape('Powered science screen','Cube',(-290,-4500,175),(.06,.7,.45),cyan,tag='PowerScreen',collision=False)
imported('Lunokhod',(12650,-1400,0));imported('PowerCell',(11880,-1500,0),tag='Module')
# Tall cylindrical ascent lander with three feet and an accessible return beacon.
shape('ARES ascent cabin','Cylinder',(-6450,-900,430),(4.2,4.2,6),white)
shape('ARES ascent nose','Cone',(-6450,-900,825),(4.2,4.2,2.2),gold)
for a in (0,120,240):
    t=math.radians(a);shape('Landing foot','Cube',(-6450+math.cos(t)*300,-900+math.sin(t)*300,35),(1.5,1.5,.7),dark)
shape('Return beacon','Cylinder',(-6750,-900,95),(.2,.2,1.9),cyan,tag='Home',collision=False)
shape('Power antenna mast','Cylinder',(900,-2500,350),(.12,.12,7),white)
shape('Power antenna dish','Sphere',(900,-2500,700),(2.5,2.5,.35),white,tag='PowerAntenna',collision=False)
for i in range(4):
    p=(200+i*350,-2850,400);shape('Power indicator','Sphere',p,(.22,.22,.22),cyan,tag='PowerBulb'+str(i),collision=False);point('Power light',p,'PowerLight'+str(i))
shape('Warning bulb','Sphere',(-160,-3000,190),(.18,.18,.18),red,tag='PowerWarningBulb',collision=False);point('Warning lamp',(-250,-3000,200),'PowerWarningLight',(1,.1,.01));point('Station lamp',(550,-3800,400),'StationLight')
imported('ScienceTerminal',(2400,1200,95),tag='Relay')
shape('Orbital relay mast','Cylinder',(2900,1200,420),(.15,.15,8.4),white)
shape('Orbital relay dish','Sphere',(2900,1200,850),(4,4,.4),gold,tag='RelayDish',collision=False,rot=(15,-40,0))
for name,value,p in [('ARES sign','ARES / 02',(50,-3200,290)),('Relay sign','ORBITAL RELAY',(2600,1200,260))]:
    a=actors.spawn_actor_from_class(u.TextRenderActor,u.Vector(*p),u.Rotator(pitch=0,yaw=180,roll=0));a.set_actor_label(name);c=a.get_component_by_class(u.TextRenderComponent);c.set_text(value);c.set_world_size(50)
levels.save_current_level()
fresh('/Game/Rescue/Maps/MissionSelect');setup();actors.spawn_actor_from_class(u.PlayerStart,u.Vector(0,0,90));shape('Menu floor','Cube',(0,0,-50),(10,10,1),dark);levels.save_current_level()
lib.save_directory('/Game/Mars',recursive=True);u.log('MARS_BUILD_COMPLETE')
