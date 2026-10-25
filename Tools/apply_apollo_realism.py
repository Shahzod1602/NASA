"""Reproducible authored materials and lighting; never modifies NASA source data."""
import json
from pathlib import Path
import unreal as u

root=Path(u.Paths.project_dir()).resolve()
lib=u.EditorAssetLibrary;edit=u.MaterialEditingLibrary;assets=u.AssetToolsHelpers.get_asset_tools()
folder='/Game/Lunar/Apollo12/Realism'
texture_path=root/'SourceArt/Apollo12/Realism/T_RegolithAuthored.png'
task=u.AssetImportTask();task.filename=str(texture_path);task.destination_path=folder
task.automated=True;task.replace_existing=True;task.save=True;assets.import_asset_tasks([task])
regolith_texture=lib.load_asset(folder+'/T_RegolithAuthored')
if not regolith_texture:raise RuntimeError('Missing authored regolith texture')
regolith_texture.set_editor_property('max_texture_size',2048);regolith_texture.set_editor_property('srgb',True)
regolith_texture.set_editor_property('power_of_two_mode',u.TexturePowerOfTwoSetting.STRETCH_TO_POWER_OF_TWO)
lib.save_loaded_asset(regolith_texture)
def node(m,cls,**props):
    n=edit.create_material_expression(m,cls)
    for key,value in props.items():n.set_editor_property(key,value)
    return n
def scalar(m,x):return node(m,u.MaterialExpressionConstant,r=x)
def vec(m,x):return node(m,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(*x))
def output(m,n,prop):edit.connect_material_property(n,'',prop)
def custom(m,code,inputs,kind=u.CustomMaterialOutputType.CMOT_FLOAT3):
    n=node(m,u.MaterialExpressionCustom,code=code,output_type=kind)
    pins=[]
    for name,value,pin in inputs:
        i=u.CustomInput();i.set_editor_property('input_name',name);pins.append(i)
    n.set_editor_property('inputs',pins)
    for name,value,pin in inputs:
        if not edit.connect_material_expressions(value,pin,n,name):raise RuntimeError('Disconnected material input '+name+' from '+value.get_class().get_name()+' output '+repr(pin))
    return n
def material(path):
    parent,name=path.rsplit('/',1)
    m=lib.load_asset(path) if lib.does_asset_exist(path) else assets.create_asset(name,parent,u.Material,u.MaterialFactoryNew())
    edit.delete_all_material_expressions(m)
    m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_DEFAULT_LIT)
    m.set_editor_property('tangent_space_normal',True)
    return m
def save(m):
    edit.recompile_material(m);lib.save_loaded_asset(m,only_if_is_dirty=False)

# Continuous two-dimensional noise with pixel-footprint filtering. Tiny grains
# converge to their mean at distance, rather than becoming bright pixel speckle.
noise='''struct Surface {
 float h(float2 p){return frac(sin(dot(p,float2(127.1,311.7)))*43758.5453);}
 float n(float2 p){float2 i=floor(p),f=frac(p);f=f*f*(3-2*f);
 return lerp(lerp(h(i),h(i+float2(1,0)),f.x),lerp(h(i+float2(0,1)),h(i+1),f.x),f.y);}
 float grain(float2 p){float w=max(length(ddx(p)),length(ddy(p)));return lerp(n(p),.5,saturate(w-.35));}
 float height(float2 p){return n(p*.028)+n(p*.12)*.14;}
 float foil(float3 p){return n(p.xy*.15+p.z*.07)*.6+n(p.yz*.28+p.x*.03)*.4;}
};Surface s;
'''

ground=material(folder+'/M_RegolithSurface');ground.set_editor_property('tangent_space_normal',False)
p=node(ground,u.MaterialExpressionWorldPosition);n=node(ground,u.MaterialExpressionVertexNormalWS)
tex=node(ground,u.MaterialExpressionTextureObject,texture=regolith_texture)
output(ground,custom(ground,noise+'''float3 detail=Texture2DSample(Tex,TexSampler,P.xy/150).rgb;
float2 rotated=float2(dot(P.xy,float2(.8,-.6)),dot(P.xy,float2(.6,.8)));
float3 broad=Texture2DSample(Tex,TexSampler,rotated/530+.37).rgb;
return lerp(detail,broad,.22)*(.72+s.n(P.xy*.0018)*.16);''',[('P',p,''),('Tex',tex,'')]),u.MaterialProperty.MP_BASE_COLOR)
output(ground,custom(ground,'''float2 uv=P.xy/150;
float h=Texture2DSample(Tex,TexSampler,uv).r;
float dx=Texture2DSample(Tex,TexSampler,uv+float2(.002,0)).r-h;
float dy=Texture2DSample(Tex,TexSampler,uv+float2(0,.002)).r-h;
float3 b=float3(-dx,-dy,0)*2.3;return normalize(N+b-N*dot(N,b));''',[('P',p,''),('N',n,''),('Tex',tex,'')]),u.MaterialProperty.MP_NORMAL)
output(ground,scalar(ground,.94),u.MaterialProperty.MP_ROUGHNESS)
output(ground,scalar(ground,.16),u.MaterialProperty.MP_SPECULAR);save(ground)

rock=material(folder+'/M_RegolithRock');p=node(rock,u.MaterialExpressionWorldPosition)
output(rock,custom(rock,noise+'return float3(.073,.071,.067)*(.65+s.n(P.xy*.17)*.55);',[('P',p,'')]),u.MaterialProperty.MP_BASE_COLOR)
output(rock,scalar(rock,.92),u.MaterialProperty.MP_ROUGHNESS);output(rock,scalar(rock,.14),u.MaterialProperty.MP_SPECULAR);save(rock)
edit.set_material_usage(rock,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES);save(rock)

foil=material(folder+'/M_LunarFoil');foil.set_editor_property('tangent_space_normal',False)
p=node(foil,u.MaterialExpressionWorldPosition);n=node(foil,u.MaterialExpressionVertexNormalWS)
output(foil,custom(foil,noise+'float c=.65+s.n((P.xy+P.z*.37)*.13)*.35;return float3(.52,.255,.058)*c;',[('P',p,'')]),u.MaterialProperty.MP_BASE_COLOR)
output(foil,custom(foil,noise+'''float h=s.foil(P);
float3 b=-float3(s.foil(P+float3(.5,0,0))-h,s.foil(P+float3(0,.5,0))-h,s.foil(P+float3(0,0,.5))-h)*2.5;
return normalize(N+b-N*dot(N,b));''',[('P',p,''),('N',n,'')]),u.MaterialProperty.MP_NORMAL)
output(foil,scalar(foil,.88),u.MaterialProperty.MP_METALLIC)
output(foil,custom(foil,noise+'return .42+s.n((P.xy+P.z)*.12)*.16;',[('P',p,'')],u.CustomMaterialOutputType.CMOT_FLOAT1),u.MaterialProperty.MP_ROUGHNESS);save(foil)

for name,color,metal,rough in [('M_EquipmentPaint',(.39,.405,.38),.12,.66),('M_EquipmentDark',(.018,.022,.025),.1,.76)]:
    m=material(folder+'/'+name);p=node(m,u.MaterialExpressionWorldPosition)
    output(m,custom(m,noise+'return Tint*(.87+s.n(P.xy*.19+P.z*.11)*.13);',[('P',p,''),('Tint',vec(m,color),'')]),u.MaterialProperty.MP_BASE_COLOR)
    output(m,scalar(m,metal),u.MaterialProperty.MP_METALLIC);output(m,scalar(m,rough),u.MaterialProperty.MP_ROUGHNESS);save(m)

for name,color,metal,rough in [('Cloth',(.32,.33,.305),0,.93),('Rubber',(.017,.022,.027),0,.82),('Metal',(.36,.39,.40),.92,.34),('Blue',(.026,.075,.13),.55,.4),('Red',(.3,.018,.011),.12,.6),('Patch',(.018,.025,.029),0,.92),('Gold',(.55,.28,.055),1,.16)]:
    m=material('/Game/Lunar/Suit/Materials/M_'+name)
    uv=node(m,u.MaterialExpressionTextureCoordinate)
    if name=='Cloth':
        pre_skin=node(m,u.MaterialExpressionPreSkinnedPosition)
        bind=node(m,u.MaterialExpressionVertexInterpolator)
        if not edit.connect_material_expressions(pre_skin,'',bind,'VS'):
            raise RuntimeError('Bind-space position failed to connect to the vertex interpolator')
        output(m,custom(m,noise+'''float d=saturate((-Bind.z-10)/95)*(.65+s.n(UV*31)*.35);
float edge=min(frac(UV.x),1-frac(UV.x));float seam=1-smoothstep(.004,.014,edge);
float shade=.87+s.n(UV*16)*.13-seam*.16;
return lerp(Tint*shade,float3(.065,.062,.052),d*.9);''',[('UV',uv,''),('Bind',bind,''),('Tint',vec(m,color),'')]),u.MaterialProperty.MP_BASE_COLOR)
        output(m,custom(m,'''float weaveFade=1-saturate(max(length(ddx(UV*160)),length(ddy(UV*160))));
float2 weave=sin(UV*1005.3)*.028*weaveFade;
float2 fold=float2(cos(UV.x*31+sin(UV.y*13)*2)*.08,cos(UV.y*47+sin(UV.x*19))*.11);
return normalize(float3(weave+fold,1));''',[('UV',uv,'')]),u.MaterialProperty.MP_NORMAL)
    else:output(m,vec(m,color),u.MaterialProperty.MP_BASE_COLOR)
    if name=='Gold':
        # Authored lunar hemisphere approximation, not a captured scene reflection.
        reflection=node(m,u.MaterialExpressionReflectionVectorWS)
        output(m,custom(m,'''float soil=smoothstep(.06,-.15,R.z);
float horizon=exp(-abs(R.z+.035)*55);return float3(.115,.067,.018)*soil+float3(.055,.04,.018)*horizon;''',[('R',reflection,'')]),u.MaterialProperty.MP_EMISSIVE_COLOR)
    output(m,scalar(m,metal),u.MaterialProperty.MP_METALLIC);output(m,scalar(m,rough),u.MaterialProperty.MP_ROUGHNESS)
    edit.set_material_usage(m,u.MaterialUsage.MATUSAGE_SKELETAL_MESH);save(m)

u.EditorLoadingAndSavingUtils.load_map('/Game/Lunar/Maps/Apollo12')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
lander_slots=[]
for actor in actors.get_all_level_actors():
    label=actor.get_actor_label()
    if label.startswith('NASA LROC Apollo 12 terrain'):actor.static_mesh_component.set_material(0,ground)
    elif label=='Readability fill':actor.light_component.set_intensity(.19)
    elif label=='Readability bounce':actor.light_component.set_intensity(.14)
    elif label.startswith('Low lunar sunlight'):
        c=actor.light_component;c.set_intensity(3.7)
        c.set_editor_property('contact_shadow_length',.025);c.set_editor_property('shadow_bias',.35)
    elif label.startswith('Intrepid / Apollo lunar module'):
        mesh=actor.static_mesh_component.get_editor_property('static_mesh')
        for i,slot in enumerate(mesh.get_editor_property('static_materials')):
            key=str(slot.material_slot_name);mat=slot.material_interface
            descriptor=(key+' '+(mat.get_name() if mat else '')).lower()
            # Numeric imported names are mapped from the original model's MTL
            # diffuse colors (gold/yellow), not inferred from mission geometry.
            gold=key in ('blinn10SG_001','blinn9SG_001','blinn1SG_002') or any(word in descriptor for word in ('gold','foil','thermal','yellow'))
            if gold:actor.static_mesh_component.set_material(i,foil)
            lander_slots.append({'slot':key,'material':mat.get_path_name() if mat else '', 'foil_replaced':gold})
    elif label=='Authored small basalt detail':actor.static_mesh_component.set_material(0,rock)
volume=next((a for a in actors.get_all_level_actors() if a.get_actor_label()=='Apollo contact shading'),None)
if volume is None:volume=actors.spawn_actor_from_class(u.PostProcessVolume,u.Vector(0,0,0),u.Rotator());volume.set_actor_label('Apollo contact shading')
volume.set_editor_property('unbound',True)
settings=volume.get_editor_property('settings')
settings.set_editor_property('override_ambient_occlusion_intensity',True);settings.set_editor_property('ambient_occlusion_intensity',.7)
settings.set_editor_property('override_ambient_occlusion_radius',True);settings.set_editor_property('ambient_occlusion_radius',55)
volume.set_editor_property('settings',settings)
u.EditorLoadingAndSavingUtils.save_current_level()
lib.save_directory(folder)
(root/'Reports/realism-material-build.json').write_text(json.dumps({'lander_slots':lander_slots,'materials':list(lib.list_assets(folder))},indent=2))
u.log('APOLLO_REALISM_MATERIALS_READY')
