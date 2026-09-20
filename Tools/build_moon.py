import unreal as u
import math, random, json
from pathlib import Path

ROOT=Path(u.Paths.project_dir()).resolve()
lib=u.EditorAssetLibrary
assets=u.AssetToolsHelpers.get_asset_tools()
edit=u.MaterialEditingLibrary
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
rng=random.Random(1969)

def node(m, cls, **props):
    n=edit.create_material_expression(m,cls)
    for k,v in props.items(): n.set_editor_property(k,v)
    return n

def mat(name,color,metal=0,emission=0,rock=False):
    m=assets.create_asset(name,'/Game/Lunar/Materials',u.Material,u.MaterialFactoryNew())
    c=node(m,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(*color))
    edit.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
    rough=node(m,u.MaterialExpressionConstant,r=.92 if rock else .55)
    edit.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    met=node(m,u.MaterialExpressionConstant,r=metal)
    edit.connect_material_property(met,'',u.MaterialProperty.MP_METALLIC)
    if emission:
        e=node(m,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(*[v*emission for v in color]))
        edit.connect_material_property(e,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    if rock:
        pos=node(m,u.MaterialExpressionWorldPosition)
        custom=node(m,u.MaterialExpressionCustom,code='float n=frac(sin(dot(floor(P*0.19),float3(12.9898,78.233,39.425)))*43758.5453); float b=sin(P.x*0.013+sin(P.y*0.017))*sin(P.y*0.011+P.z*0.03); return float3(0.23,0.24,0.255)*(0.65+n*0.42+b*0.16);',output_type=u.CustomMaterialOutputType.CMOT_FLOAT3)
        inp=u.CustomInput(); inp.set_editor_property('input_name','P'); custom.set_editor_property('inputs',[inp])
        edit.connect_material_expressions(pos,'',custom,'P')
        edit.connect_material_property(custom,'',u.MaterialProperty.MP_BASE_COLOR)
    edit.recompile_material(m)
    lib.save_loaded_asset(m)
    return m

mats={
 'dust':mat('M_Regolith',(.25,.26,.28),rock=True),
 'white':mat('M_Ceramic',(.65,.69,.72),.25),
 'dark':mat('M_Graphite',(.026,.043,.059),.6),
 'gold':mat('M_ThermalFoil',(.6,.3,.06),.65),
 'cyan':mat('M_Signal',(.06,.65,.8),.1,2),
 'orange':mat('M_Warning',(.9,.22,.025),.2,.4),
 'blue':mat('M_Solar',(.025,.06,.16),.65),
 'earth':mat('M_Earth',(.045,.24,.46),.1,.6),
 'stars':mat('M_Stars',(.65,.73,.9),0,3),
}
levels.new_level('/Game/Lunar/Maps/MoonBase')
meshes={s:lib.load_asset('/Engine/BasicShapes/'+s) for s in ['Cube','Sphere','Cylinder','Cone','Plane']}
def shape(name,kind,p,scale,material,rot=(0,0,0),collision=True,tag=None):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*p),u.Rotator(*rot))
    a.set_actor_label(name)
    c=a.static_mesh_component
    c.set_static_mesh(meshes[kind]); c.set_material(0,mats[material])
    a.set_actor_scale3d(u.Vector(*scale))
    c.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    c.set_collision_enabled(u.CollisionEnabled.QUERY_AND_PHYSICS if collision else u.CollisionEnabled.NO_COLLISION)
    a.set_actor_enable_collision(collision)
    if tag: a.set_editor_property('tags',[tag])
    return a
def cube(name,p,s,m,**kw): return shape(name,'Cube',p,s,m,**kw)
def text(name,value,p,size=65,color=(180,224,237,255),rot=(0,180,0)):
    a=actors.spawn_actor_from_class(u.TextRenderActor,u.Vector(*p),u.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]));a.set_actor_label(name)
    c=a.get_component_by_class(u.TextRenderComponent)
    c.set_text(value); c.set_world_size(size); c.set_text_render_color(u.Color(*color))
    c.set_horizontal_alignment(u.HorizTextAligment.EHTA_CENTER)
    return a
def light(name,p,color,intensity,tag=None):
    a=actors.spawn_actor_from_class(u.PointLight,u.Vector(*p)); a.set_actor_label(name)
    c=a.light_component; c.set_mobility(u.ComponentMobility.MOVABLE);c.set_intensity(intensity)
    c.set_light_color(u.LinearColor(*color)); c.set_attenuation_radius(1400);c.set_cast_shadows(False)
    if tag: a.set_editor_property('tags',[tag])
    return a

# Broad traversable regolith, rough boulders and crater rims.
cube('Lunar surface',(0,0,-80),(500,500,1.6),'dust')
for cx,cy,rad in [(4200,1100,900),(-2800,4300,1200),(7000,-4500,1700),(-7500,-5200,2100),(10500,6800,2500)]:
    shape('Crater dark basin','Cylinder',(cx,cy,3),(rad/50,rad/50,.03),'dark',collision=False)
    for i in range(44):
        t=i*math.tau/44
        shape('Crater regolith rim','Sphere',(cx+math.cos(t)*rad,cy+math.sin(t)*rad,-25),(3.5,3.0,rng.uniform(1.7,2.5)),'dust',rot=(rng.uniform(-12,12),math.degrees(t),0))
for i in range(240):
    x,y=rng.uniform(-19000,19000),rng.uniform(-19000,19000)
    # Keep landing, station, and direct mission routes unobstructed.
    if -1700<x<5600 and -2200<y<900: continue
    s=rng.uniform(.5,3.5)
    shape('Regolith boulder','Sphere',(x,y,-s*12),(s*1.4,s,s*.75),'dust',rot=(rng.uniform(-30,30),rng.uniform(0,360),rng.uniform(-25,25)))
for i in range(48):
    t=i*math.tau/48;rad=rng.uniform(22000,26000)
    shape('Distant lunar ridge','Sphere',(math.cos(t)*rad,math.sin(t)*rad,-400),(rng.uniform(35,75),rng.uniform(28,50),rng.uniform(15,40)),'dust',collision=False)

# SELENE station: raised sealed habitat, external power and science panels.
cube('Station foundation',(800,-1000,70),(15,11,1.4),'dark')
cube('Habitat insulated shell',(1050,-1000,390),(10,8,5),'white')
cube('Habitat top',(1050,-1000,655),(10.6,8.6,.35),'dark')
cube('Orange identification band',(535,-1000,500),(.12,8,.45),'orange')
for y in [-1300,-1100,-900,-700]:
    cube('Window seal',(543,y,395),(.1,1.6,1.5),'dark')
    cube('Window glass',(535,y,400),(.12,1.3,1.15),'blue')
cube('Door frame',(1050,-582,295),(2.6,.25,3.7),'dark')
cube('Door',(1050,-560,295),(2.1,.2,3.3),'gold')
for x in [580,1500]:
    for y in [-1340,-660]: cube('Station feet',(x,y,0),(1,1,2),'gold')
text('SELENE branding','S E L E N E',(528,-1000,577),63)
text('Station number','RESEARCH OUTPOST / 01',(525,-1000,205),25)
power=cube('Power port',(-80,-1000,90),(1.0,1.0,1.8),'dark',tag='Power')
cube('Power socket',(-135,-1000,112),(.08,.65,.65),'orange',collision=False)
text('Power label','POWER / E',(-142,-1000,195),23)
data=cube('Science terminal',(-80,-1500,95),(1,1.2,1.9),'white',tag='Data')
cube('Terminal screen',(-134,-1500,125),(.08,.93,.75),'cyan',collision=False)
text('Data label','SCIENCE / E',(-142,-1500,212),22)
for y in [-1000,-1500]: light('Station status light',(-230,y,220),(.1,.85,1),0,'StationLight')
for y in [-2700,-3700]:
    cube('Array support',(1500,y,160),(.4,.4,3.2),'white')
    cube('Solar panel',(1500,y,350),(8,6,.12),'blue',rot=(0,0,12))
    for i in range(9): cube('Solar cell divider',(1100+i*100,y,358),(.035,6,.035),'white',collision=False)
shape('Antenna mast','Cylinder',(1400,-1000,1050),(.13,.13,8),'white')
shape('Relay dish','Sphere',(1400,-1000,1450),(3.1,3.1,.5),'white',rot=(20,0,0))

# Rescue module at a rover wreck, before the nearest crater rim.
cube('Rover chassis',(4300,-250,75),(3.4,2.3,1),'gold',rot=(0,15,8))
for x in [4170,4430]:
    for y in [-390,-110]: shape('Rover wheel','Cylinder',(x,y,45),(1.0,1.0,.45),'dark',rot=(0,0,90))
cube('Rover solar panel',(4300,-250,162),(3.7,2.5,.1),'blue',rot=(0,15,8))
module=cube('Spare power module',(3960,-500,65),(.75,.65,.95),'gold',tag='Module')
cube('Module beacon',(3960,-500,120),(.14,.14,.18),'cyan',collision=False)
light('Module beacon light',(3960,-500,170),(.1,.7,1),400)
text('Rover ID','SURVEY / 03',(4100,-550,240),36)

# Lander at the starting area; four legs and a small science cargo bay.
shape('Lander body','Cylinder',(-1900,-300,250),(4.3,4.3,3.2),'gold')
shape('Lander upper','Cone',(-1900,-300,490),(4.3,4.3,1.8),'white')
for dx in [-260,260]:
    for dy in [-260,260]:
        cube('Lander strut',(-1900+dx,-300+dy,100),(.25,.25,2.0),'white',rot=(0,0,15 if dy<0 else -15))
        shape('Lander foot','Cylinder',(-1900+dx,-300+dy,12),(.9,.9,.22),'dark')
home=cube('Lander return console',(-2250,-300,95),(.7,1.1,1.9),'dark',tag='Home')
cube('Return console display',(-2290,-300,140),(.06,.8,.5),'cyan',collision=False)
text('Lander label','ASCENT / 01',(-2130,-300,345),35)
light('Lander lamp',(-2400,-300,270),(.15,.65,1),1200)

# Navigation stakes lead between landing zone and the stranded rover.
for i in range(12):
    x=-1200+i*440
    shape('Route marker pole','Cylinder',(x,350,55),(.065,.065,1.1),'white',collision=False)
    cube('Route marker cap',(x,350,115),(.23,.23,.1),'cyan',collision=False)

# Black sky, distant Earth and a restrained fixed star field.
shape('Earth over horizon','Sphere',(24000,35000,24000),(48,48,48),'earth',collision=False)
for i in range(170):
    t=rng.uniform(0,math.tau);z=rng.uniform(.13,.98);r=math.sqrt(1-z*z);d=90000
    s=rng.uniform(.25,.8)
    shape('Distant star','Sphere',(math.cos(t)*r*d,math.sin(t)*r*d,z*d),(s,s,s),'stars',collision=False)
sun=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,5000),u.Rotator(-24,-38,0))
sun.set_actor_label('Low lunar sun');sun.light_component.set_mobility(u.ComponentMobility.MOVABLE);sun.light_component.set_intensity(3.2)
sun.light_component.set_editor_property('forward_shading_priority',1)
sun.light_component.set_light_color(u.LinearColor(1,.93,.83))
fill=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,4000),u.Rotator(-65,140,0))
fill.set_actor_label('Soft readability fill');fill.light_component.set_mobility(u.ComponentMobility.MOVABLE);fill.light_component.set_intensity(.36);fill.light_component.set_cast_shadows(False)
fill.light_component.set_light_color(u.LinearColor(.48,.61,.82))
start=actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-2450,300,100),u.Rotator(0,-8,0));start.set_actor_label('Lunar arrival')

# Editable Blueprint children expose movement and mission tuning in the editor.
for name,parent in [('BP_Astronaut','LunarCharacter'),('BP_LunarGameMode','LunarGameMode')]:
    factory=u.BlueprintFactory();factory.set_editor_property('parent_class',u.load_class(None,'/Script/LunarRescue.'+parent))
    bp=assets.create_asset(name,'/Game/Lunar/Blueprints',u.Blueprint,factory)
    lib.save_loaded_asset(bp)
gm=u.load_class(None,'/Game/Lunar/Blueprints/BP_LunarGameMode.BP_LunarGameMode_C')
cdo=u.get_default_object(gm)
cdo.set_editor_property('default_pawn_class',u.load_class(None,'/Game/Lunar/Blueprints/BP_Astronaut.BP_Astronaut_C'))
lib.save_asset('/Game/Lunar/Blueprints/BP_LunarGameMode')
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode',gm)
u.get_editor_subsystem(u.UnrealEditorSubsystem).set_level_viewport_camera_info(u.Vector(-3300,2500,1400),u.Rotator(-17,-29,0))
levels.save_current_level()
lib.save_directory('/Game/Lunar',only_if_is_dirty=False,recursive=True)
report={'map':'/Game/Lunar/Maps/MoonBase','actors':len(actors.get_all_level_actors()),'mission_tags':['Module','Power','Data','Home'],'blueprints':['BP_Astronaut','BP_LunarGameMode'],'gravity_cm_s2':-162,'oxygen_seconds':300,'terrain':'fictional, no NASA elevation dataset used'}
(ROOT/'Reports/map-build.json').write_text(json.dumps(report,indent=2))
u.log('LUNAR_MAP_READY '+str(report))
