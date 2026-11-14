"""Import a new static visual only; do not load or regenerate mission maps."""
from pathlib import Path
import os
import unreal as u
root=Path(u.Paths.project_dir()).resolve();folder='/Game/Lunar/Robots/Perseverance20261114'
if u.EditorAssetLibrary.list_assets(folder,recursive=True):
 if os.environ.get('LUNAR_NOMAD_CORRECT_ATLAS')!='1':raise RuntimeError('Refusing to replace an existing NOMAD visual import')
 # Explicit repair of this batch's rejected oversized atlas import only. The
 # caller backs up its exact new directory first; no owner assets/maps touched.
 if not u.EditorAssetLibrary.delete_directory(folder+'/SM_NomadReference'):raise RuntimeError('Could not retire the rejected atlas import')
task=u.AssetImportTask();task.filename=str(root/'SourceArt/Perseverance20261114/Derived/SM_NomadReference.glb');task.destination_path=folder;task.automated=True;task.replace_existing=False;task.save=True
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
subsystem=u.get_editor_subsystem(u.StaticMeshEditorSubsystem) or u.get_default_object(u.StaticMeshEditorSubsystem)
meshes=[]
for path in u.EditorAssetLibrary.list_assets(folder,recursive=True):
 asset=u.EditorAssetLibrary.load_asset(path)
 if not isinstance(asset,u.StaticMesh):continue
 meshes.append(path);nanite=asset.get_editor_property('nanite_settings');nanite.enabled=False;asset.set_editor_property('nanite_settings',nanite)
 options=u.StaticMeshReductionOptions();options.auto_compute_lod_screen_size=False;levels=[]
 for fraction,screen in [(1.,1.),(.4,.6),(.15,.25),(.04,.08)]:
  setting=u.StaticMeshReductionSettings();setting.percent_triangles=fraction;setting.screen_size=screen;levels.append(setting)
 options.reduction_settings=levels
 if subsystem.set_lods(asset,options)<1:raise RuntimeError('NOMAD reference LOD creation failed')
 u.EditorAssetLibrary.save_loaded_asset(asset,only_if_is_dirty=False)
if len(meshes)!=1:raise RuntimeError('Expected one merged NOMAD static mesh: '+str(meshes))
(root/'Reports/Feedback20261113/nomad-import.json').write_text(__import__('json').dumps({'meshes':meshes,'maps_regenerated':False},indent=2))
u.log('NOMAD_REFERENCE_IMPORT_COMPLETE '+str(meshes))
