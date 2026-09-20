import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir()).resolve()
assets=u.AssetToolsHelpers.get_asset_tools();lib=u.EditorAssetLibrary;ed=u.MaterialEditingLibrary
for f in (root/'SourceArt/Immersion').glob('*.wav'):
    if '-LunarMaterialsOnly' in u.SystemLibrary.get_command_line():break
    task=u.AssetImportTask();task.filename=str(f);task.destination_path='/Game/Lunar/Immersion/Audio';task.automated=True;task.replace_existing=True;task.save=True
    assets.import_asset_tasks([task])
    sound=lib.load_asset('/Game/Lunar/Immersion/Audio/'+f.stem)
    sound.set_editor_property('looping',f.stem=='SuitBreath');lib.save_loaded_asset(sound)
def mat(name,color,rough):
    path='/Game/Lunar/Immersion/Materials'
    m=lib.load_asset(path+'/'+name) if lib.does_asset_exist(path+'/'+name) else assets.create_asset(name,path,u.Material,u.MaterialFactoryNew())
    ed.delete_all_material_expressions(m)
    c=ed.create_material_expression(m,u.MaterialExpressionConstant3Vector);c.constant=u.LinearColor(*color)
    ed.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
    r=ed.create_material_expression(m,u.MaterialExpressionConstant);r.r=rough;ed.connect_material_property(r,'',u.MaterialProperty.MP_ROUGHNESS)
    return m
for name,color in [('M_SuitCloth',(.62,.60,.54)),('M_SuitJoint',(.028,.033,.038)),('M_SuitTrim',(.85,.22,.025))]:
    m=mat(name,color,.9);ed.recompile_material(m);lib.save_loaded_asset(m)
m=mat('M_Bootprint',(.023,.021,.019),1)
m.set_editor_property('material_domain',u.MaterialDomain.MD_DEFERRED_DECAL)
m.set_editor_property('blend_mode',u.BlendMode.BLEND_TRANSLUCENT)
uv=ed.create_material_expression(m,u.MaterialExpressionTextureCoordinate)
c=ed.create_material_expression(m,u.MaterialExpressionCustom)
c.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT1)
c.set_editor_property('code','float2 p=(UV.yx-.5)*2; float width=.73-.14*exp(-pow((p.y+.15)*4,2)); float body=(1-smoothstep(width-.08,width,abs(p.x)))*(1-smoothstep(.72,.92,abs(p.y))); float tread=smoothstep(.42,.6,abs(sin(p.y*22))); return body*(.06+tread*.84);')
i=u.CustomInput();i.set_editor_property('input_name','UV');c.set_editor_property('inputs',[i]);ed.connect_material_expressions(uv,'',c,'UV');ed.connect_material_property(c,'',u.MaterialProperty.MP_OPACITY)
ed.recompile_material(m);lib.save_loaded_asset(m)
u.log('IMMERSION_ASSETS_READY')
