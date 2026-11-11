"""Build a normals-only mesh derivative and one authored cabin fabric material.

Run through the installed Unreal Python commandlet after the normal recovery
processor and the editor-module build. No original asset or map is saved.
"""
from pathlib import Path
import hashlib
import json
import unreal as u

root = Path(u.Paths.project_dir()).resolve()
report_dir = root / 'Reports/IntrepidCabin20261111'
report_dir.mkdir(parents=True, exist_ok=True)
source = root / 'Content/Lunar/Imported/ApolloLander/ApolloLander.uasset'
source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
mesh_receipt = report_dir / 'mesh-build.json'
material_only = '-cabinmaterialonly' in u.SystemLibrary.get_command_line().lower()
if not material_only:
    if mesh_receipt.exists():
        raise RuntimeError('Keep existing build receipts; archive mesh-build.json before another invocation')
    world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
    u.SystemLibrary.execute_console_command(world, 'LunarRescue.BuildIntrepidCabin')
if not mesh_receipt.is_file():
    raise RuntimeError('Editor mesh builder did not write a fresh receipt; inspect the commandlet log')
receipt = json.loads(mesh_receipt.read_text(encoding='utf-8-sig'))
if receipt.get('passed') is not True:
    raise RuntimeError('Mesh normal recovery failed its geometry/attribute invariants')
derived_file = root / 'Content/Lunar/Apollo12/IntrepidVisual/SM_IntrepidCabin.uasset'
if not derived_file.is_file():
    raise RuntimeError('Validated derivative must exist before creating its material')
derived_sha = hashlib.sha256(derived_file.read_bytes()).hexdigest()

lib, edit = u.EditorAssetLibrary, u.MaterialEditingLibrary
path = '/Game/Lunar/Apollo12/IntrepidVisual/M_IntrepidCabin'
material = lib.load_asset(path) if lib.does_asset_exist(path) else u.AssetToolsHelpers.get_asset_tools().create_asset(
    'M_IntrepidCabin', '/Game/Lunar/Apollo12/IntrepidVisual', u.Material, u.MaterialFactoryNew())
if not material or material.get_path_name().split('.')[0] != path:
    raise RuntimeError('Invalid dedicated cabin material destination')
edit.delete_all_material_expressions(material)
material.set_editor_property('tangent_space_normal', False)
material.set_editor_property('shading_model', u.MaterialShadingModel.MSM_DEFAULT_LIT)
material.set_editor_property('two_sided', True)

def node(cls, **properties):
    value = edit.create_material_expression(material, cls)
    if not value:
        raise RuntimeError('Material expression creation failed')
    for key, item in properties.items():
        value.set_editor_property(key, item)
    return value

def output(value, prop):
    if not edit.connect_material_property(value, '', prop):
        raise RuntimeError('Unconnected material output')

def constant(value):
    return node(u.MaterialExpressionConstant, r=value)

position = node(u.MaterialExpressionPreSkinnedPosition)
normal = node(u.MaterialExpressionPreSkinnedNormal)
p = node(u.MaterialExpressionVertexInterpolator)
n = node(u.MaterialExpressionVertexInterpolator)
if not edit.connect_material_expressions(position, '', p, 'VS') or not edit.connect_material_expressions(normal, '', n, 'VS'):
    raise RuntimeError('Source-local position/normal require explicit vertex-to-pixel interpolation')
tint = node(u.MaterialExpressionVectorParameter, parameter_name='CabinTint', default_value=u.LinearColor(.498039216, .498039216, .498039216, 1))
custom = node(u.MaterialExpressionCustom, output_type=u.CustomMaterialOutputType.CMOT_FLOAT3, code='''
float3 a=abs(N);
float2 uv=(a.z>max(a.x,a.y))?P.xy:((a.x>a.y)?P.yz:P.xz);
float footprint=max(length(ddx(uv)),length(ddy(uv)));
float2 edge=min(frac(uv/80.0),1-frac(uv/80.0))*80.0;
float seam=1-smoothstep(.35,.85+footprint,min(edge.x,edge.y));
float fold=sin(uv.x*.12+sin(uv.y*.07))*sin(uv.y*.16)*.012;
float weave=sin(uv.x*125.66)*sin(uv.y*125.66)*.003*(1-saturate(footprint*20));
return Tint*(1-seam*.025+fold+weave);
''')
pins=[]
for name in ('P','N','Tint'):
    pin=u.CustomInput(); pin.set_editor_property('input_name',name); pins.append(pin)
custom.set_editor_property('inputs',pins)
for name,value in (('P',p),('N',n),('Tint',tint)):
    if not edit.connect_material_expressions(value,'',custom,name):
        raise RuntimeError('Unconnected cabin shader input')
output(custom,u.MaterialProperty.MP_BASE_COLOR)
output(node(u.MaterialExpressionVertexNormalWS),u.MaterialProperty.MP_NORMAL)
output(constant(.78),u.MaterialProperty.MP_ROUGHNESS)
output(constant(.18),u.MaterialProperty.MP_SPECULAR)
output(constant(0),u.MaterialProperty.MP_METALLIC)
edit.recompile_material(material)
if not lib.save_loaded_asset(material,only_if_is_dirty=False):
    raise RuntimeError('Could not save the dedicated cabin material')
if hashlib.sha256(source.read_bytes()).hexdigest()!=source_sha:
    raise RuntimeError('Original lander asset changed during the build')
if hashlib.sha256(derived_file.read_bytes()).hexdigest()!=derived_sha:
    raise RuntimeError('Material construction must not rewrite the validated mesh')
result={'passed':True,'material':material.get_path_name(),'mesh_receipt':'mesh-build.json',
        'source_uasset_sha256':source_sha,'derived_uasset_sha256':derived_sha,'material_only':material_only,'maps_saved':False,'original_assets_saved':False,
        'measured_terrain_changed':False,'role':'Retained GLB-authored cabin normals plus project-authored fabric shading; not measured optical properties or new NASA terrain data.',
        'coefficients':{'roughness':.78,'specular':.18,'metallic':0,'seam_color_strength':.025,'two_source_neutral_tints':[.498039216,.678431392]}}
(report_dir/'material-build.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
u.log('INTREPID_CABIN_ASSETS_READY original mesh and maps unchanged')
