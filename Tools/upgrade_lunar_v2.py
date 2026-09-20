import unreal as u
import json,math,random
from pathlib import Path
ROOT=Path(u.Paths.project_dir()).resolve()
lib=u.EditorAssetLibrary;edit=u.MaterialEditingLibrary
assets=u.AssetToolsHelpers.get_asset_tools()
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')

def material(name,unlit=False,two=False):
    path='/Game/Lunar/V2/Materials/'+name
    m=lib.load_asset(path) if lib.does_asset_exist(path) else assets.create_asset(name,'/Game/Lunar/V2/Materials',u.Material,u.MaterialFactoryNew())
    edit.delete_all_material_expressions(m)
    m.set_editor_property('two_sided',two)
    if unlit:m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
    return m
def node(m,cls,**props):
    n=edit.create_material_expression(m,cls)
    for k,v in props.items():n.set_editor_property(k,v)
    return n
def scalar(m,value):return node(m,u.MaterialExpressionConstant,r=value)
def vector(m,value):return node(m,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(*value))
def out(m,n,prop):edit.connect_material_property(n,'',prop)
def custom(m,code,inputs,kind=u.CustomMaterialOutputType.CMOT_FLOAT3):
    n=node(m,u.MaterialExpressionCustom,code=code,output_type=kind)
    names=[]
    for name,v in inputs:
        i=u.CustomInput();i.set_editor_property('input_name',name);names.append(i)
    n.set_editor_property('inputs',names)
    for name,v in inputs:edit.connect_material_expressions(v,'',n,name)
    return n
def save(m):edit.recompile_material(m);lib.save_loaded_asset(m,only_if_is_dirty=False)

noise='''struct Grain {
 float h(float3 p) { return frac(sin(dot(p,float3(127.1,311.7,74.7)))*43758.5453); }
 float n(float3 p) { float3 i=floor(p),f=frac(p);f=f*f*(3-2*f);
 return lerp(lerp(lerp(h(i),h(i+float3(1,0,0)),f.x),lerp(h(i+float3(0,1,0)),h(i+float3(1,1,0)),f.x),f.y),
 lerp(lerp(h(i+float3(0,0,1)),h(i+float3(1,0,1)),f.x),lerp(h(i+float3(0,1,1)),h(i+float3(1,1,1)),f.x),f.y),f.z); }
 float f(float3 p) { return n(p)*.55+n(p*2.13+7.1)*.28+n(p*4.21+13.7)*.17; }
}; Grain g;
'''
reg=material('M_LunarRegolith')
reg.set_editor_property('tangent_space_normal',False)
pos=node(reg,u.MaterialExpressionWorldPosition)
base=custom(reg,noise+'float c=.55+g.f(P*.055)*.27+g.f(P*.004)*.25+g.f(P*.00037)*.23; return float3(.16,.158,.153)*c;', [('P',pos)])
out(reg,base,u.MaterialProperty.MP_BASE_COLOR)
normal=custom(reg,noise+'float h=g.f(P*.085); float dx=g.f((P+float3(1,0,0))*.085)-h; float dy=g.f((P+float3(0,1,0))*.085)-h; return normalize(N+float3(-dx,-dy,0)*2.2);',[('P',pos),('N',node(reg,u.MaterialExpressionVertexNormalWS))])
out(reg,normal,u.MaterialProperty.MP_NORMAL)
out(reg,scalar(reg,.97),u.MaterialProperty.MP_ROUGHNESS)
out(reg,scalar(reg,.12),u.MaterialProperty.MP_SPECULAR)
save(reg)

earth=material('M_EarthPhotographic',True)
tex=node(earth,u.MaterialExpressionTextureSample,texture=lib.load_asset('/Game/Lunar/V2/Textures/T_Earth_Clouds'))
color=custom(earth,'float d=saturate(dot(normalize(N),normalize(float3(-.85,-.35,.55)))); return T*(.015+d*1.8);',[('T',tex),('N',node(earth,u.MaterialExpressionVertexNormalWS))])
out(earth,color,u.MaterialProperty.MP_EMISSIVE_COLOR)
out(earth,node(earth,u.MaterialExpressionCameraPositionWS),u.MaterialProperty.MP_WORLD_POSITION_OFFSET);save(earth)
atmo=material('M_EarthLimb',True)
atmo.set_editor_property('blend_mode',u.BlendMode.BLEND_ADDITIVE)
fresnel=node(atmo,u.MaterialExpressionFresnel,exponent=4.0,base_reflect_fraction=0.0)
blue=vector(atmo,(.018,.09,.21))
out(atmo,blue,u.MaterialProperty.MP_EMISSIVE_COLOR);out(atmo,fresnel,u.MaterialProperty.MP_OPACITY)
out(atmo,node(atmo,u.MaterialExpressionCameraPositionWS),u.MaterialProperty.MP_WORLD_POSITION_OFFSET);save(atmo)

star_mats={}
for name,tint in [('Blue',(.65,.8,1)),('White',(.94,.95,1)),('Warm',(1,.82,.58))]:
    m=material('M_Stars'+name,True,True)
    m.set_editor_property('blend_mode',u.BlendMode.BLEND_MASKED)
    m.set_editor_property('opacity_mask_clip_value',.1)
    uv=node(m,u.MaterialExpressionTextureCoordinate)
    disc=custom(m,'return saturate(1-length(UV-float2(.5,.5))*2);',[('UV',uv)],u.CustomMaterialOutputType.CMOT_FLOAT1)
    out(m,disc,u.MaterialProperty.MP_OPACITY_MASK)
    glow=custom(m,'return Tint*pow(D,1.2)*8.0;',[('Tint',vector(m,tint)),('D',disc)])
    out(m,glow,u.MaterialProperty.MP_EMISSIVE_COLOR)
    out(m,node(m,u.MaterialExpressionCameraPositionWS),u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    save(m);star_mats[name]=m

planetmat=material('M_PlanetPoint',True)
out(planetmat,vector(planetmat,(3,2.5,1.8)),u.MaterialProperty.MP_EMISSIVE_COLOR)
out(planetmat,node(planetmat,u.MaterialExpressionCameraPositionWS),u.MaterialProperty.MP_WORLD_POSITION_OFFSET);save(planetmat)

# Replace only generated environment actors; mission props retain their transforms and tags.
prefixes=['Lunar surface','Crater dark basin','Crater regolith rim','Regolith boulder','Distant lunar ridge','Earth over horizon','Distant star','V2_']
removed=0
for a in list(actors.get_all_level_actors()):
    label=a.get_actor_label()
    if any(label.startswith(p) for p in prefixes):actors.destroy_actor(a);removed+=1
    elif label=='Low lunar sun':
        a.set_actor_rotation(u.Rotator(-19,-35,0),False)
        a.light_component.set_intensity(3.8)
        a.light_component.set_light_color(u.LinearColor(1,.98,.94))
        a.light_component.set_editor_property('forward_shading_priority',1)
        a.light_component.set_editor_property('light_source_angle',.53)
    elif label=='Soft readability fill':a.light_component.set_intensity(.10)
    elif label=='Lander lamp':a.light_component.set_intensity(90)
    elif label=='Module beacon light':a.light_component.set_intensity(50)

def mesh(name,asset,p=(0,0,0),scale=(1,1,1),mat=None,collision=True,rot=(0,0,0)):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*p),u.Rotator(*rot));a.set_actor_label('V2_'+name)
    c=a.static_mesh_component;c.set_static_mesh(lib.load_asset(asset));a.set_actor_scale3d(u.Vector(*scale))
    if mat:c.set_material(0,mat)
    c.set_collision_profile_name('BlockAll' if collision else 'NoCollision');a.set_actor_enable_collision(collision)
    if not collision:c.set_cast_shadow(False)
    return a

terrain=mesh('Continuous crater terrain','/Game/Lunar/V2/Meshes/SM_LunarTerrain',mat=reg)
horizon=mesh('Distant crater highlands','/Game/Lunar/V2/Meshes/SM_LunarHorizon',mat=reg,collision=False)
# Keep distant horizon beneath the local playable basin.
horizon.set_actor_location(u.Vector(0,0,-1500),False,False)

data=json.loads((ROOT/'SourceArt/LunarV2/geometry.json').read_text())
craters=data['craters']
def noisev(x,y):
    return math.sin(x*.00071+math.sin(y*.00048)*2.1)*math.cos(y*.00057+x*.00012)*.48+math.sin(x*.0019-y*.0011)*math.sin(y*.0023+x*.0008)*.28+math.sin(x*.0049+y*.0043)*math.cos(y*.0061-x*.0037)*.15+math.sin(x*.019-y*.013)*math.cos(y*.023+x*.016)*.09
def ground(x,y):
    z=noisev(x,y)*75+noisev(x*.23,y*.23)*150
    for cx,cy,r,depth in craters:
        d=math.hypot(x-cx,y-cy)/r
        z+=-depth*max(0,1-d*d)**1.5+depth*.37*math.exp(-((d-1.03)/.17)**2)
    dx=max(-3200-x,x-5000,0);dy=max(-4200-y,y-650,0)
    blend=min(1,math.hypot(dx,dy)/1400);blend=blend*blend*(3-2*blend)
    return z*blend
rng=random.Random(1972)
for i in range(550):
    x,y=rng.uniform(-24000,24000),rng.uniform(-24000,24000)
    if -3400<x<5400 and -4400<y<1100:continue
    size=rng.uniform(.25,2.0)
    a=mesh('Basalt fragment',f'/Game/Lunar/V2/Meshes/SM_Basalt_{i%6}',(x,y,ground(x,y)+size*12),(size,size*.85,size),reg,size>.8,(rng.uniform(-25,25),rng.uniform(0,360),rng.uniform(-20,20)))
    a.static_mesh_component.set_editor_property('ld_max_draw_distance',24000)
for i in range(110):
    x,y=rng.uniform(-3000,5000),rng.uniform(-2500,700)
    size=rng.uniform(.045,.18)
    mesh('Fine surface debris',f'/Game/Lunar/V2/Meshes/SM_Basalt_{i%6}',(x,y,1),(size,size,size*.6),reg,False,(0,rng.uniform(0,360),0))

# Catalog stars: one combined mesh per color, three draw calls instead of thousands of actors.
for group in star_mats:
    mesh('Catalog stars '+group,'/Game/Lunar/V2/Meshes/SM_Stars_'+group,mat=star_mats[group],collision=False,rot=(15,35,0))

# Approximately two-degree Earth disc with real cloud/continent imagery.
dist=900000;az=math.radians(18);el=math.radians(27)
ep=(dist*math.cos(el)*math.cos(az),dist*math.cos(el)*math.sin(az),dist*math.sin(el))
diameter=2*dist*math.tan(math.radians(1));scale=diameter/100
mesh('Earth','/Engine/BasicShapes/Sphere',ep,(scale,scale,scale),earth,False,(0,120,12))
mesh('Earth atmosphere','/Engine/BasicShapes/Sphere',ep,(scale*1.014,)*3,atmo,False)
for name,az,el,sz in [('Venus',-30,18,10),('Jupiter',75,39,7),('Mars',125,24,5)]:
    az,el=map(math.radians,(az,el));d=850000
    p=(d*math.cos(el)*math.cos(az),d*math.cos(el)*math.sin(az),d*math.sin(el))
    mesh(name,'/Engine/BasicShapes/Sphere',p,(sz,sz,sz),planetmat,False)

u.get_editor_subsystem(u.UnrealEditorSubsystem).set_level_viewport_camera_info(u.Vector(-2700,900,350),u.Rotator(12,12,0))
levels.save_current_level()
lib.save_directory('/Game/Lunar/V2',only_if_is_dirty=False,recursive=True)
report={'removed_prototype_actors':removed,'actors':len(actors.get_all_level_actors()),'catalog_stars':5044,'terrain_triangles':320000,'earth_angular_diameter_degrees':2,'terrain':'procedural, not surveyed NASA terrain','planets':'artist-placed distant points; no ephemeris simulation','visibility':'star brightness enhanced for game readability'}
(ROOT/'Reports/lunar-v2.json').write_text(json.dumps(report,indent=2))
u.log('LUNAR_V2_READY '+str(report))
