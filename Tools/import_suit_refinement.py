"""Update only authored garment meshes; preserve existing materials and maps."""
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir()).resolve()
lib = u.EditorAssetLibrary
assets = u.AssetToolsHelpers.get_asset_tools()
u.SystemLibrary.execute_console_command(None, 'Interchange.FeatureFlags.Import.OBJ 0')
for name in ('Forearm', 'Cuff', 'Palm', 'Finger', 'UpperArm', 'Thigh', 'Shin'):
    stem = 'SM_Suit' + name
    opts = u.FbxImportUI()
    opts.automated_import_should_detect_type = False
    opts.import_materials = False
    opts.import_textures = False
    opts.import_as_skeletal = False
    opts.import_mesh = True
    opts.mesh_type_to_import = u.FBXImportType.FBXIT_STATIC_MESH
    data = opts.static_mesh_import_data
    data.combine_meshes = True
    data.auto_generate_collision = False
    data.generate_lightmap_u_vs = False
    data.set_editor_property('convert_scene', True)
    data.set_editor_property('convert_scene_unit', False)
    data.set_editor_property('normal_import_method', u.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
    task = u.AssetImportTask()
    task.filename = str(root / 'SourceArt/Suit' / (stem + '.obj'))
    task.destination_path = '/Game/Lunar/Suit/Meshes'
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.factory = u.FbxFactory()
    task.options = opts
    assets.import_asset_tasks([task])
    mesh = lib.load_asset(task.destination_path + '/' + stem)
    if not mesh:
        raise RuntimeError('Missing suit mesh ' + stem)
    for i, slot in enumerate(mesh.get_editor_property('static_materials')):
        material = lib.load_asset('/Game/Lunar/Suit/Materials/M_' + str(slot.material_slot_name))
        if not material:
            raise RuntimeError('Unknown garment material')
        mesh.set_material(i, material)
    lib.save_loaded_asset(mesh, only_if_is_dirty=False)

script = root / 'Tools/import_suit_rig.py'
exec(compile(script.read_text(encoding='utf-8'), str(script), 'exec'), {'__file__': str(script)})
sub = u.get_editor_subsystem(u.SkeletalMeshEditorSubsystem)
for name in ('SK_A7L', 'SK_A7L_Hands'):
    mesh = lib.load_asset('/Game/Lunar/Suit/Rig/' + name + '/SkeletalMeshes/' + name)
    if not mesh or not sub.regenerate_lod(mesh, 3, True, False):
        raise RuntimeError('Suit LOD generation failed: ' + name)
    lib.save_loaded_asset(mesh, only_if_is_dirty=False)
script = root / 'Tools/bind_realism_materials.py'
exec(compile(script.read_text(encoding='utf-8'), str(script), 'exec'), {'__file__': str(script)})
u.log('SUIT_REFINEMENT_IMPORTED')
