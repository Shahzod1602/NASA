"""Reimport authored radio WAVs without rebuilding tools, materials or maps."""
import unreal as u
from pathlib import Path

root = Path(u.Paths.project_dir()).resolve()
assets = u.AssetToolsHelpers.get_asset_tools()
for key in ("StartMessage", "CheckoutDone", "SamplesDone"):
    source = root / "SourceArt/Apollo12/Radio" / (key + ".wav")
    if not source.is_file():
        raise RuntimeError("Missing narration: " + str(source))
    task = u.AssetImportTask()
    task.filename = str(source)
    task.destination_path = "/Game/Lunar/Apollo12/Radio"
    task.automated = True
    task.replace_existing = True
    task.save = True
    assets.import_asset_tasks([task])
    if not task.imported_object_paths:
        raise RuntimeError("Radio import failed: " + key)
u.log("APOLLO_RADIO_BINDING_NEUTRAL_VERIFIED")
