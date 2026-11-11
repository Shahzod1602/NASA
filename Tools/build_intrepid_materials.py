"""Build only four new authored presentation materials; never load/save a map.

The retained regolith bitmap is an AI-authored art source, not a lunar photograph.
No displacement, elevation import, source texture rewrite or suit rewrite occurs.
Run with Unreal's supported Python commandlet after preserving project state.
"""
import hashlib
import json
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir()).resolve()
folder = '/Game/Lunar/Apollo12/IntrepidVisual'
lib = u.EditorAssetLibrary
edit = u.MaterialEditingLibrary
assets = u.AssetToolsHelpers.get_asset_tools()
texture = lib.load_asset('/Game/Lunar/Apollo12/Realism/T_RegolithAuthored')
if not texture:
    raise RuntimeError('Retained authored regolith texture must be present; no placeholder substitution')


def node(material, cls, **props):
    result = edit.create_material_expression(material, cls)
    if not result:
        raise RuntimeError('Cannot create material expression ' + cls.__name__)
    for key, value in props.items():
        result.set_editor_property(key, value)
    return result


def scalar(material, value):
    return node(material, u.MaterialExpressionConstant, r=value)


def parameter(material, name, value):
    return node(material, u.MaterialExpressionScalarParameter, parameter_name=name, default_value=value)


def custom(material, code, inputs, scalar_output=False):
    kind = u.CustomMaterialOutputType.CMOT_FLOAT1 if scalar_output else u.CustomMaterialOutputType.CMOT_FLOAT3
    result = node(material, u.MaterialExpressionCustom, code=code, output_type=kind)
    pins = []
    for name, _ in inputs:
        pin = u.CustomInput()
        pin.set_editor_property('input_name', name)
        pins.append(pin)
    result.set_editor_property('inputs', pins)
    for name, value in inputs:
        if not edit.connect_material_expressions(value, '', result, name):
            raise RuntimeError('Unconnected custom material input: ' + name)
    return result


def output(material, expression, prop):
    if not edit.connect_material_property(expression, '', prop):
        raise RuntimeError('Unconnected material output: ' + str(prop))


def make(name):
    path = folder + '/' + name
    result = lib.load_asset(path) if lib.does_asset_exist(path) else assets.create_asset(name, folder, u.Material, u.MaterialFactoryNew())
    if not result or result.get_path_name().split('.')[0] != path:
        raise RuntimeError('Material destination escaped the dedicated art folder')
    edit.delete_all_material_expressions(result)
    result.set_editor_property('shading_model', u.MaterialShadingModel.MSM_DEFAULT_LIT)
    result.set_editor_property('tangent_space_normal', False)
    return result


def save(material, instanced=False):
    if instanced:
        edit.set_material_usage(material, u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    edit.recompile_material(material)
    if not lib.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError('Cannot save generated material: ' + material.get_path_name())


noise = '''struct Surface {
 float h(float2 p){return frac(sin(dot(p,float2(127.1,311.7)))*43758.5453);}
 float n(float2 p){float2 i=floor(p),f=frac(p);f=f*f*(3-2*f);
 return lerp(lerp(h(i),h(i+float2(1,0)),f.x),lerp(h(i+float2(0,1)),h(i+1),f.x),f.y);}
 float grain(float2 p){float w=max(length(ddx(p)),length(ddy(p)));return lerp(n(p),.5,saturate(w-.25));}
 float foil(float3 p){return n(p.xy*.15+p.z*.07)*.6+n(p.yz*.28+p.x*.03)*.4;}
};Surface s;
'''
mask = '''float edge=max(abs(P.x-Center.r),abs(P.y-Center.g));
float mask=1-smoothstep(max(0,HalfExtent-EdgeFade),max(1,HalfExtent),edge);
'''

ground = make('M_IntrepidRegolith')
p = node(ground, u.MaterialExpressionWorldPosition)
n = node(ground, u.MaterialExpressionVertexNormalWS)
camera = node(ground, u.MaterialExpressionCameraPositionWS)
tex = node(ground, u.MaterialExpressionTextureObject, texture=texture)
center = node(ground, u.MaterialExpressionVectorParameter, parameter_name='SliceCenter', default_value=u.LinearColor(0, 0, 0, 0))
half = parameter(ground, 'HalfExtentCM', 2500.0)
fade = parameter(ground, 'EdgeFadeCM', 350.0)
common = [('P', p), ('Tex', tex), ('Center', center), ('HalfExtent', half), ('EdgeFade', fade), ('Camera', camera)]
albedo = noise + mask + '''
float3 detail=Texture2DSample(Tex,TexSampler,P.xy/150).rgb;
float2 rotated=float2(dot(P.xy,float2(.8,-.6)),dot(P.xy,float2(.6,.8)));
float3 broad=Texture2DSample(Tex,TexSampler,rotated/530+.37).rgb;
float3 original=lerp(detail,broad,.22)*(.72+s.n(P.xy*.0018)*.16);
if(mask<=0)return original;
float near=1-smoothstep(220,1100,distance(Camera,P));
float3 fine=Texture2DSample(Tex,TexSampler,rotated/48+.173).rgb;
float grain=s.grain(P.xy*.65);
float macro=s.n(rotated*.0013);
float3 local=lerp(lerp(detail,broad,.32),fine,.28*near);
local*=.79+macro*.11;
local*=1+(grain-.5)*.065*near;
return lerp(original,local,mask);
'''
normal = noise + mask + '''
float2 uv=P.xy/150;
float h=Texture2DSample(Tex,TexSampler,uv).r;
float dx=Texture2DSample(Tex,TexSampler,uv+float2(.002,0)).r-h;
float dy=Texture2DSample(Tex,TexSampler,uv+float2(0,.002)).r-h;
float3 originalB=float3(-dx,-dy,0)*2.3;
float3 original=normalize(N+originalB-N*dot(N,originalB));
if(mask<=0)return original;
float near=1-smoothstep(180,900,distance(Camera,P));
float2 localUV=P.xy/48+.173;
float fine=Texture2DSample(Tex,TexSampler,localUV).r;
float fx=Texture2DSample(Tex,TexSampler,localUV+float2(.003,0)).r-fine;
float fy=Texture2DSample(Tex,TexSampler,localUV+float2(0,.003)).r-fine;
float grain=s.grain(P.xy*.65);
float gx=s.grain((P.xy+float2(.28,0))*.65)-grain;
float gy=s.grain((P.xy+float2(0,.28))*.65)-grain;
float3 b=originalB+float3(-fx,-fy,0)*1.15*near+float3(-gx,-gy,0)*.15*near;
float3 local=normalize(N+b-N*dot(N,b));
return normalize(lerp(original,local,mask));
'''
output(ground, custom(ground, albedo, common), u.MaterialProperty.MP_BASE_COLOR)
output(ground, custom(ground, normal, common + [('N', n)]), u.MaterialProperty.MP_NORMAL)
output(ground, scalar(ground, .94), u.MaterialProperty.MP_ROUGHNESS)
output(ground, scalar(ground, .16), u.MaterialProperty.MP_SPECULAR)
save(ground)

rock = make('M_IntrepidRock')
p = node(rock, u.MaterialExpressionWorldPosition)
n = node(rock, u.MaterialExpressionVertexNormalWS)
tex = node(rock, u.MaterialExpressionTextureObject, texture=texture)
output(rock, custom(rock, noise + '''
float3 weights=pow(abs(N),4);weights/=max(.001,weights.x+weights.y+weights.z);
float3 yz=Texture2DSample(Tex,TexSampler,P.yz/28+.31).rgb;
float3 zx=Texture2DSample(Tex,TexSampler,P.zx/28+.17).rgb;
float3 xy=Texture2DSample(Tex,TexSampler,P.xy/28+.53).rgb;
float3 texel=yz*weights.x+zx*weights.y+xy*weights.z;
return lerp(float3(.068,.067,.064),texel*.47,.48)*(.88+s.n(P.xy*.037+P.z*.014)*.22);
''', [('P', p), ('N', n), ('Tex', tex)]), u.MaterialProperty.MP_BASE_COLOR)
output(rock, custom(rock, noise + '''
float h=s.n(P.xy*.16+P.z*.11);
float3 b=-float3(s.n((P.xy+float2(.35,0))*.16+P.z*.11)-h,
 s.n((P.xy+float2(0,.35))*.16+P.z*.11)-h,s.n(P.xy*.16+(P.z+.35)*.11)-h)*.42;
return normalize(N+b-N*dot(N,b));
''', [('P', p), ('N', n)]), u.MaterialProperty.MP_NORMAL)
output(rock, scalar(rock, .91), u.MaterialProperty.MP_ROUGHNESS)
output(rock, scalar(rock, .13), u.MaterialProperty.MP_SPECULAR)
save(rock, instanced=True)

foil = make('M_IntrepidFoil')
p = node(foil, u.MaterialExpressionWorldPosition)
n = node(foil, u.MaterialExpressionVertexNormalWS)
output(foil, custom(foil, noise + '''
float crumple=s.foil(P);
return float3(.58,.34,.10)*(.85+crumple*.15);
''', [('P', p)]), u.MaterialProperty.MP_BASE_COLOR)
output(foil, custom(foil, noise + '''
float h=s.foil(P);
float3 b=-float3(s.foil(P+float3(.45,0,0))-h,s.foil(P+float3(0,.45,0))-h,s.foil(P+float3(0,0,.45))-h)*1.8;
return normalize(N+b-N*dot(N,b));
''', [('P', p), ('N', n)]), u.MaterialProperty.MP_NORMAL)
output(foil, scalar(foil, .62), u.MaterialProperty.MP_METALLIC)
output(foil, custom(foil, noise + 'return .48+s.foil(P)*.14;', [('P', p)], scalar_output=True), u.MaterialProperty.MP_ROUGHNESS)
save(foil, instanced=True)

metal = make('M_IntrepidMetal')
p = node(metal, u.MaterialExpressionWorldPosition)
n = node(metal, u.MaterialExpressionVertexNormalWS)
output(metal, custom(metal, noise + 'return float3(.27,.285,.29)*(.93+s.n(P.xy*.18+P.z*.025)*.07);', [('P', p)]), u.MaterialProperty.MP_BASE_COLOR)
output(metal, custom(metal, noise + '''
float fade=1-saturate(max(length(ddx(P.xy*.8)),length(ddy(P.xy*.8))));
float3 b=float3(sin(P.y*12.4)*.018*fade,0,0);
return normalize(N+b-N*dot(N,b));
''', [('P', p), ('N', n)]), u.MaterialProperty.MP_NORMAL)
output(metal, scalar(metal, .78), u.MaterialProperty.MP_METALLIC)
output(metal, scalar(metal, .42), u.MaterialProperty.MP_ROUGHNESS)
save(metal, instanced=True)

report_dir = root / 'Reports/IntrepidVisual20261111'
report_dir.mkdir(parents=True, exist_ok=True)
report = {
    'kind': 'authored_material_build', 'created_materials': [m.get_path_name() for m in (ground, rock, foil, metal)],
    'retained_texture': texture.get_path_name(),
    'retained_texture_source_sha256': hashlib.sha256((root / 'SourceArt/Apollo12/Realism/T_RegolithAuthored.png').read_bytes()).hexdigest(),
    'source_role': 'AI-authored regolith bitmap retained unchanged; new shaders authored in code. No measured microgeometry.',
    'scope': '50 m square material mask; ground outside preserves original shader formula; no displacement.',
    'maps_loaded_or_saved': False, 'original_materials_or_textures_saved': False,
}
(report_dir / 'material-build.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8', newline='\n')
u.log('INTREPID_VISUAL_MATERIALS_READY four new materials; no map or original asset writes')
