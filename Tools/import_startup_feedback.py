"""Import team-authorized demo illustrations without modifying any map."""
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir())
items=[
 ('T_AstroKid','Startup20261114/AstroKidReference.jpg','Startup20261114'),
 ('T_InventoryReference','Startup20261114/InventoryReference.jpg','Startup20261114'),
 ('T_PreflightCabin','AuthorizedMedia20261115/PreflightCabin.png','Demo20261115'),
 ('T_PreflightReference','AuthorizedMedia20261115/PreflightReference.jpg','Demo20261115'),
 ('T_Acknowledgements','AuthorizedMedia20261115/IntroAcknowledgements.jpg','Demo20261115'),
]
for name,filename,folder in items:
    source=root/'SourceArt'/filename
    if not source.is_file():
        u.log('OPTIONAL_REFERENCE_SKIPPED '+filename+'; public text presentation remains available')
        continue
    destination='/Game/Lunar/Frontend/'+folder
    asset=destination+'/'+name
    if not u.EditorAssetLibrary.does_asset_exist(asset):
        task=u.AssetImportTask();task.filename=str(source);task.destination_path=destination;task.destination_name=name;task.automated=task.save=True;task.replace_existing=False
        u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture=u.load_asset(asset)
    assert isinstance(texture,u.Texture2D)
    texture.set_editor_property('lod_group',u.TextureGroup.TEXTUREGROUP_UI);texture.set_editor_property('never_stream',True)
    u.EditorAssetLibrary.save_loaded_asset(texture)
u.log('STARTUP_FEEDBACK_IMPORT_OK optional references only; map_writes=0')
