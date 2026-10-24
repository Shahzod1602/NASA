import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir()).resolve()
for name in ('SK_A7L','SK_A7L_Hands'):
    task=u.AssetImportTask();task.filename=str(root/'SourceArt/Suit/Rig'/(name+'.glb'))
    task.destination_path='/Game/Lunar/Suit/Rig';task.automated=True;task.replace_existing=True;task.save=True
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    u.log('RIG_IMPORT '+str(task.imported_object_paths))
for path in u.EditorAssetLibrary.list_assets('/Game/Lunar/Suit/Rig',recursive=True):
    asset=u.EditorAssetLibrary.load_asset(path)
    if isinstance(asset,u.SkeletalMesh):
        slots=asset.get_editor_property('materials')
        for i,slot in enumerate(slots):
            key=str(slot.material_slot_name)
            mat=u.EditorAssetLibrary.load_asset('/Game/Lunar/Suit/Materials/M_'+key)
            if mat:slot.material_interface=mat
        asset.set_editor_property('materials',slots)
        u.EditorAssetLibrary.save_loaded_asset(asset)
        u.log('RIG_READY '+path+' '+str(asset.get_editor_property('skeleton')))
u.EditorAssetLibrary.save_directory('/Game/Lunar/Suit/Rig')
