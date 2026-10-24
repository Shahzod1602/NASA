"""Import authored suit geometry and create reusable cloth/rubber/metal materials."""
import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir()).resolve();out='/Game/Lunar/Suit'
lib=u.EditorAssetLibrary;assets=u.AssetToolsHelpers.get_asset_tools();ed=u.MaterialEditingLibrary
colors={'Cloth':(.52,.54,.51),'Rubber':(.055,.065,.07),'Metal':(.4,.44,.46),'Blue':(.035,.12,.28),'Red':(.42,.025,.015),'Gold':(.63,.36,.065),'Patch':(.065,.10,.14)}
mats={}
for name,col in colors.items():
    path=out+'/Materials/M_'+name
    m=lib.load_asset(path) if lib.does_asset_exist(path) else assets.create_asset('M_'+name,out+'/Materials',u.Material,u.MaterialFactoryNew())
    ed.delete_all_material_expressions(m)
    c=ed.create_material_expression(m,u.MaterialExpressionConstant3Vector);c.constant=u.LinearColor(*col);ed.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
    rough=ed.create_material_expression(m,u.MaterialExpressionConstant);rough.r=.82 if name=='Cloth' else .58 if name in ('Rubber','Patch') else .24
    ed.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    if name in ('Metal','Gold','Blue'):
        metal=ed.create_material_expression(m,u.MaterialExpressionConstant);metal.r=.85;ed.connect_material_property(metal,'',u.MaterialProperty.MP_METALLIC)
    if name=='Cloth':
        uv=ed.create_material_expression(m,u.MaterialExpressionTextureCoordinate)
        weave=ed.create_material_expression(m,u.MaterialExpressionCustom);weave.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT3)
        weave.set_editor_property('code','float2 p=UV*180; return normalize(float3(.025*sin(p.x*6.283),.025*sin(p.y*6.283),1));')
        i=u.CustomInput();i.set_editor_property('input_name','UV');weave.set_editor_property('inputs',[i]);ed.connect_material_expressions(uv,'',weave,'UV');ed.connect_material_property(weave,'',u.MaterialProperty.MP_NORMAL)
    ed.recompile_material(m);lib.save_loaded_asset(m);mats[name]=m
u.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.OBJ 0')
for p in (root/'SourceArt/Suit').glob('*.obj'):
    opts=u.FbxImportUI();opts.automated_import_should_detect_type=False;opts.import_materials=False;opts.import_textures=False;opts.import_as_skeletal=False;opts.import_mesh=True;opts.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
    data=opts.static_mesh_import_data;data.combine_meshes=True;data.auto_generate_collision=False;data.generate_lightmap_u_vs=False;data.set_editor_property('convert_scene',True);data.set_editor_property('convert_scene_unit',False)
    data.set_editor_property('normal_import_method',u.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
    task=u.AssetImportTask();task.filename=str(p);task.destination_path=out+'/Meshes';task.automated=True;task.replace_existing=True;task.save=True;task.factory=u.FbxFactory();task.options=opts;assets.import_asset_tasks([task])
    mesh=lib.load_asset(out+'/Meshes/'+p.stem)
    if not mesh:raise RuntimeError('Missing imported mesh '+p.stem)
    for n,slot in enumerate(mesh.get_editor_property('static_materials')):
        key=str(slot.material_slot_name);mesh.set_material(n,mats.get(key,mats['Cloth']))
    lib.save_loaded_asset(mesh,only_if_is_dirty=False)
    u.log('SUIT_MESH '+p.stem+' '+str(mesh.get_bounds()))
lib.save_directory(out,recursive=True);u.log('SUIT_ASSETS_READY')
