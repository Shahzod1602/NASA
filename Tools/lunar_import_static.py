import json
from pathlib import Path

import unreal as u

root = Path(r'C:\Users\shaxz\lunar_staging')
manifest = json.loads((root / 'manifest.json').read_text())
if '-LunarImportProbe' in u.SystemLibrary.get_command_line():
    names = ['PowerCell']
else:
    names = list(manifest)

u.SystemLibrary.execute_console_command(None, 'Interchange.FeatureFlags.Import.OBJ 0')
u.SystemLibrary.execute_console_command(None, 'Interchange.FeatureFlags.Import.FBX 0')
tool = u.AssetToolsHelpers.get_asset_tools()
library = u.EditorAssetLibrary
report = {}
for name in names:
    source = root / name / (name + '.obj')
    opts = u.FbxImportUI()
    opts.automated_import_should_detect_type = False
    opts.import_as_skeletal = False
    opts.import_mesh = True
    opts.import_materials = True
    opts.import_textures = True
    opts.mesh_type_to_import = u.FBXImportType.FBXIT_STATIC_MESH
    data = opts.static_mesh_import_data
    data.set_editor_property('convert_scene', False)
    data.set_editor_property('convert_scene_unit', False)
    data.combine_meshes = True
    data.auto_generate_collision = False
    data.generate_lightmap_u_vs = False
    task = u.AssetImportTask()
    task.filename = str(source)
    task.destination_path = '/Game/Lunar/Imported/' + name
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    task.factory = u.FbxFactory()
    task.options = opts
    tool.import_asset_tasks([task])
    mesh = library.load_asset('/Game/Lunar/Imported/' + name + '/' + name)
    if not mesh or not isinstance(mesh, u.StaticMesh):
        raise RuntimeError('No static mesh imported: ' + name + ' ' + str(task.imported_object_paths))
    body = mesh.get_editor_property('body_setup')
    body.set_editor_property('collision_trace_flag', u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    library.save_loaded_asset(mesh, only_if_is_dirty=False)
    report[name] = {
        'asset': mesh.get_path_name(),
        'bounds': str(mesh.get_bounds().box_extent),
        'materials': [str(x.material_interface) for x in mesh.get_editor_property('static_materials')],
    }
    u.log('LUNAR_STATIC_IMPORTED ' + name + ' ' + str(report[name]))
(root / 'import_report.json').write_text(json.dumps(report, indent=2))
