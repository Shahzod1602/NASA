"""Import the new derivative only; never overwrite existing A7L assets/maps."""
from pathlib import Path
import os
import unreal as u

root=Path(u.Paths.project_dir()).resolve()
for name in ('SK_ExpeditionSuit','SK_ExpeditionHands'):
    destination='/Game/Lunar/Suit/Expedition20261113'
    task=u.AssetImportTask();task.filename=str(root/'SourceArt/ExpeditionSuit20261113/Derived'/(name+'.glb'))
    reimport=os.environ.get('LUNAR_SUIT_REIMPORT')=='1'
    task.destination_path=destination;task.automated=True;task.replace_existing=reimport;task.save=True
    existing=u.EditorAssetLibrary.list_assets(destination+'/'+name,recursive=True)
    if existing and not reimport:raise RuntimeError('Refusing to replace existing expedition import: '+name)
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths:raise RuntimeError('Expedition import produced no assets: '+name)
    meshes=[]
    for path in u.EditorAssetLibrary.list_assets(destination+'/'+name,recursive=True):
        asset=u.EditorAssetLibrary.load_asset(path)
        if isinstance(asset,u.SkeletalMesh):
            meshes.append(path)
            subsystem=u.get_editor_subsystem(u.SkeletalMeshEditorSubsystem)
            if not subsystem.regenerate_lod(asset,3,True,False):raise RuntimeError('LOD generation failed: '+path)
            u.EditorAssetLibrary.save_loaded_asset(asset,only_if_is_dirty=False)
    if len(meshes)!=1:raise RuntimeError('Expected one derivative skeletal mesh: '+str(meshes))
    u.log('EXPEDITION_IMPORTED '+str(meshes))
# Both view/body skins use the same source surfaces and material instances.
body=u.EditorAssetLibrary.load_asset(destination+'/SK_ExpeditionSuit/SkeletalMeshes/SK_ExpeditionSuit')
hands=u.EditorAssetLibrary.load_asset(destination+'/SK_ExpeditionHands/SkeletalMeshes/SK_ExpeditionHands')
materials={str(s.get_editor_property('material_slot_name')):s.get_editor_property('material_interface') for s in body.get_editor_property('materials')}
slots=hands.get_editor_property('materials')
for slot in slots:slot.set_editor_property('material_interface',materials[str(slot.get_editor_property('material_slot_name'))])
hands.set_editor_property('materials',slots);u.EditorAssetLibrary.save_loaded_asset(hands,only_if_is_dirty=False)
u.log('EXPEDITION_IMPORT_COMPLETE')
