"""Optional local reference-art importer. Public builds do not require these files."""
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir())
for name,filename in [('T_AstroKid','AstroKidReference.jpg'),('T_InventoryReference','InventoryReference.jpg')]:
    source=root/'SourceArt/Startup20261114'/filename
    if not source.is_file():
        u.log('OPTIONAL_REFERENCE_SKIPPED '+filename+'; public text presentation remains available')
        continue
    asset='/Game/Lunar/Frontend/Startup20261114/'+name
    if not u.EditorAssetLibrary.does_asset_exist(asset):
        task=u.AssetImportTask();task.filename=str(source);task.destination_path='/Game/Lunar/Frontend/Startup20261114';task.destination_name=name;task.automated=task.save=True;task.replace_existing=False
        u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture=u.load_asset(asset)
    assert isinstance(texture,u.Texture2D)
    texture.set_editor_property('lod_group',u.TextureGroup.TEXTUREGROUP_UI);texture.set_editor_property('never_stream',True)
    u.EditorAssetLibrary.save_loaded_asset(texture)
u.log('STARTUP_FEEDBACK_IMPORT_OK optional references only; map_writes=0')
