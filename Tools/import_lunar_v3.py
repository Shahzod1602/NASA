import unreal as u
from pathlib import Path
import json
ROOT=Path(u.Paths.project_dir()).resolve()
ART=ROOT/'SourceArt/LunarV3'
lib=u.EditorAssetLibrary
assets=u.AssetToolsHelpers.get_asset_tools()
report={}
u.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.OBJ 0')
u.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
for path in sorted(ART.glob('*.obj')):
    if '-LunarStarsOnly' in u.SystemLibrary.get_command_line() and not path.stem.startswith('SM_Stars'):continue
    opts=u.FbxImportUI()
    opts.automated_import_should_detect_type=False
    opts.import_materials=False;opts.import_textures=False
    opts.import_as_skeletal=False;opts.import_mesh=True
    opts.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
    data=opts.static_mesh_import_data
    data.set_editor_property('convert_scene',True)
    data.set_editor_property('convert_scene_unit',False)
    data.set_editor_property('normal_import_method',u.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
    data.combine_meshes=True;data.auto_generate_collision=not path.stem.startswith('SM_Stars');data.generate_lightmap_u_vs=False
    task=u.AssetImportTask();task.filename=str(path);task.destination_path='/Game/Lunar/V3/Meshes';task.destination_name=path.stem
    task.automated=True;task.replace_existing=True;task.replace_existing_settings=True;task.save=True;task.factory=u.FbxFactory();task.options=opts
    assets.import_asset_tasks([task])
    mesh=lib.load_asset('/Game/Lunar/V3/Meshes/'+path.stem)
    if not mesh:raise RuntimeError('Failed to import '+str(path))
    body=mesh.get_editor_property('body_setup')
    body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    body.set_editor_property('double_sided_geometry',True)
    lib.save_loaded_asset(mesh,only_if_is_dirty=False)
    b=mesh.get_bounds()
    report[path.stem]={'origin':str(b.origin),'extent':str(b.box_extent),'asset':mesh.get_path_name()}
for path in [ART/'T_Earth_Clouds.jpg']:
    if not path.exists():continue
    task=u.AssetImportTask();task.filename=str(path);task.destination_path='/Game/Lunar/V3/Textures';task.destination_name=path.stem
    task.automated=True;task.replace_existing=True;task.save=True
    assets.import_asset_tasks([task])
    tex=lib.load_asset('/Game/Lunar/V3/Textures/'+path.stem)
    if not tex:raise RuntimeError('Failed texture '+str(path))
    tex.set_editor_property('max_texture_size',4096)
    if path.stem=='T_Earth_Clouds':
        tex.set_editor_property('never_stream',True)
        tex.set_editor_property('mip_gen_settings',u.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    if path.suffix=='.exr':
        tex.set_editor_property('srgb',False)
        tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_HDR)
        tex.set_editor_property('mip_gen_settings',u.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    lib.save_loaded_asset(tex,only_if_is_dirty=False)
    report[path.stem]=tex.get_class().get_name()
(ROOT/'Reports/v3-import.json').write_text(json.dumps(report,indent=2))
u.log('LUNAR_V3_IMPORT_READY '+str(report))
