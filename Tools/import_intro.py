"""Import the supplied intro artwork only; no map or mesh writes."""
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
path="/Game/Lunar/Frontend/T_IntroReference"
if not unreal.EditorAssetLibrary.does_asset_exist(path):
 task=unreal.AssetImportTask()
 task.filename=str(root/"SourceArt/Frontend/IntroReference.jpg")
 task.destination_path="/Game/Lunar/Frontend"
 task.destination_name="T_IntroReference"
 task.automated=True
 task.replace_existing=False
 task.save=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture=unreal.load_asset(path)
if not isinstance(texture,unreal.Texture2D):raise RuntimeError("Intro texture import failed")
texture.set_editor_property("lod_group",unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property("never_stream",True)
unreal.EditorAssetLibrary.save_asset(path)
print("NASA_INTRO_IMPORT_OK: supplied artwork, no map writes")
