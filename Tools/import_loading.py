"""Import authored loading artwork only; never touch maps or scientific terrain."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir())
names = ("Moon", "Mars")
for name in names:
    source = root / "SourceArt" / "Frontend" / f"Loading{name}.png"
    if not source.is_file():
        raise RuntimeError(f"Missing loading artwork: {source}")

tasks = []
for name in names:
    task = unreal.AssetImportTask()
    task.filename = str(root / "SourceArt" / "Frontend" / f"Loading{name}.png")
    task.destination_path = "/Game/Lunar/Frontend"
    task.destination_name = f"T_Loading{name}"
    task.automated = True
    task.replace_existing = False
    task.save = True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for name in names:
    path = f"/Game/Lunar/Frontend/T_Loading{name}"
    texture = unreal.load_asset(path)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError(f"Missing imported loading texture: {path}")
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("never_stream", True)
    unreal.EditorAssetLibrary.save_asset(path)
print("LOADING_IMPORT_OK: two loading textures; no map writes")
