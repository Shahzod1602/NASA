"""Import new credited derivatives; preserve original missions and vehicle code."""
import json, os
from pathlib import Path
import unreal as u

root=Path(u.Paths.project_dir()).resolve()
manifest=json.loads((root/'SourceArt/MuseumRover20261113/adaptation.json').read_text())
destination='/Game/Lunar/Vehicles/Museum20261113'
subsystem=u.get_editor_subsystem(u.StaticMeshEditorSubsystem) or u.get_default_object(u.StaticMeshEditorSubsystem)
wheel_material=None
for part in manifest['parts']:
 name=part['name']
 existing=u.EditorAssetLibrary.list_assets(destination+'/'+name,recursive=True)
 if existing and os.environ.get('LUNAR_ROVER_RESUME')!='1':raise RuntimeError('Refusing to overwrite existing new rover assets: '+name)
 if not existing:
  task=u.AssetImportTask();task.filename=str(root/'SourceArt/MuseumRover20261113/Derived'/(name+'.glb'));task.destination_path=destination;task.automated=True;task.replace_existing=False;task.save=True
  u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 meshes=[]
 for path in u.EditorAssetLibrary.list_assets(destination+'/'+name,recursive=True):
  asset=u.EditorAssetLibrary.load_asset(path)
  if not isinstance(asset,u.StaticMesh):continue
  meshes.append(path)
  nanite=asset.get_editor_property('nanite_settings');nanite.enabled=False;asset.set_editor_property('nanite_settings',nanite)
  if asset.get_name()!=name:raise RuntimeError('Unexpected mesh in derivative folder: '+path)
  reduction=u.StaticMeshReductionOptions();reduction.auto_compute_lod_screen_size=False
  levels=[]
  for percentage,screen in ((1.,1.),(.4,.6),(.15,.25),(.05,.08)):
   setting=u.StaticMeshReductionSettings();setting.percent_triangles=percentage;setting.screen_size=screen;levels.append(setting)
  reduction.reduction_settings=levels
  if subsystem.set_lods(asset,reduction)<1:raise RuntimeError('Static LOD generation failed: '+path)
  if 'Wheel' in name:
   if wheel_material is None:wheel_material=asset.get_material(0)
   else:
    for index in range(len(asset.static_materials)):asset.set_material(index,wheel_material)
  u.EditorAssetLibrary.save_loaded_asset(asset,only_if_is_dirty=False)
 if len(meshes)!=1:raise RuntimeError('Expected one new rover mesh: '+str(meshes))
 u.log('MUSEUM_ROVER_IMPORTED '+str(meshes))
u.log('MUSEUM_ROVER_IMPORT_COMPLETE')
