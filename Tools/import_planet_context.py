"""Create an offline UI sphere shader and import credited global colour maps."""
from pathlib import Path
import os
import unreal as u

root=Path(u.Paths.project_dir()).resolve();folder='/Game/Lunar/UI/PlanetContext20261114';lib=u.EditorAssetLibrary;edit=u.MaterialEditingLibrary
for name,filename in [('T_MoonContext','lroc_color_2k.jpg'),('T_MarsContext','Mars.jpg')]:
 if not lib.does_asset_exist(folder+'/'+name):
  task=u.AssetImportTask();task.filename=str(root/'SourceArt/PlanetContext20261114/Raw'/filename);task.destination_path=folder;task.destination_name=name;task.automated=True;task.replace_existing=False;task.save=True;u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 if not lib.does_asset_exist(folder+'/'+name):raise RuntimeError('Planet texture missing: '+name)
path=folder+'/M_PlanetContext'
if lib.does_asset_exist(path):
 if os.environ.get('LUNAR_CONTEXT_REPAIR')!='1':raise RuntimeError('Refusing to overwrite a planet context material')
 material=lib.load_asset(path);edit.delete_all_material_expressions(material)
else:material=u.AssetToolsHelpers.get_asset_tools().create_asset('M_PlanetContext',folder,u.Material,u.MaterialFactoryNew())
material.set_editor_property('material_domain',u.MaterialDomain.MD_UI);material.set_editor_property('blend_mode',u.BlendMode.BLEND_TRANSLUCENT)
uv=edit.create_material_expression(material,u.MaterialExpressionTextureCoordinate)
texture=edit.create_material_expression(material,u.MaterialExpressionTextureObjectParameter);texture.set_editor_property('parameter_name','Planet');texture.set_editor_property('texture',lib.load_asset(folder+'/T_MoonContext'))
parameters={}
for name,default in [('Yaw',0.),('Pitch',0.),('Zoom',1.),('Aspect',1.)]:
 node=edit.create_material_expression(material,u.MaterialExpressionScalarParameter);node.set_editor_property('parameter_name',name);node.set_editor_property('default_value',default);parameters[name]=node
code='''float2 p=(UV-0.5)*float2(max(Aspect,0.1),1)*2.2/max(Zoom,0.5);
float r=dot(p,p); if(r>=1) return float4(0,0,0,0);
float3 visible=float3(sqrt(max(0,1-r)),p.x,-p.y); float3 n=visible;
float y=Yaw*0.01745329252, a=Pitch*0.01745329252;
n=float3(n.x*cos(a)+n.z*sin(a),n.y,n.z*cos(a)-n.x*sin(a));
n=float3(n.x*cos(y)-n.y*sin(y),n.x*sin(y)+n.y*cos(y),n.z);
float2 t=float2(frac(atan2(n.y,n.x)/6.28318530718+0.5),acos(clamp(n.z,-1,1))/3.14159265359);
float3 color=Texture2DSample(Planet,PlanetSampler,t).rgb;
float light=0.18+0.82*max(0,dot(visible,normalize(float3(0.6,-0.4,0.7))));
return float4(color*light,saturate((1-r)*120));'''
custom=edit.create_material_expression(material,u.MaterialExpressionCustom);custom.set_editor_property('code',code);custom.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT4)
pins=[]
for name in ['UV','Planet','Yaw','Pitch','Zoom','Aspect']:
 pin=u.CustomInput();pin.set_editor_property('input_name',name);pins.append(pin)
custom.set_editor_property('inputs',pins)
for name,node in dict(UV=uv,Planet=texture,**parameters).items():
 if not edit.connect_material_expressions(node,'',custom,name):raise RuntimeError('Planet shader input missing: '+name)
rgb=edit.create_material_expression(material,u.MaterialExpressionComponentMask);rgb.set_editor_property('r',True);rgb.set_editor_property('g',True);rgb.set_editor_property('b',True);rgb.set_editor_property('a',False)
alpha=edit.create_material_expression(material,u.MaterialExpressionComponentMask);alpha.set_editor_property('r',False);alpha.set_editor_property('g',False);alpha.set_editor_property('b',False);alpha.set_editor_property('a',True)
if not edit.connect_material_expressions(custom,'',rgb,''):raise RuntimeError('Planet RGB mask connection failed')
if not edit.connect_material_expressions(custom,'',alpha,''):raise RuntimeError('Planet alpha mask connection failed')
if not edit.connect_material_property(rgb,'',u.MaterialProperty.MP_EMISSIVE_COLOR):raise RuntimeError('Planet colour output connection failed')
if not edit.connect_material_property(alpha,'',u.MaterialProperty.MP_OPACITY):raise RuntimeError('Planet opacity output connection failed')
edit.recompile_material(material);lib.save_loaded_asset(material,only_if_is_dirty=False)
u.log('PLANET_CONTEXT_IMPORTED / offline colour context, not a terrain or geolocation product')
