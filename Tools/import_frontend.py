"""Import authored menu artwork only; never touch maps or scientific terrain."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir())
tasks = []
for name in ("Moon", "Mars"):
    task = unreal.AssetImportTask()
    task.filename = str(root / "SourceArt" / "Frontend" / f"Menu{name}.png")
    task.destination_path = "/Game/Lunar/Frontend"
    task.destination_name = f"T_Menu{name}"
    task.automated = True
    task.replace_existing = False
    task.save = True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for name in ("Moon", "Mars"):
    path = f"/Game/Lunar/Frontend/T_Menu{name}"
    texture = unreal.load_asset(path)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Missing imported texture: {path}")
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("never_stream", True)
    unreal.EditorAssetLibrary.save_asset(path)
print("FRONTEND_IMPORT_OK: two menu textures; no map writes")
