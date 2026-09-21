import unreal as u
from pathlib import Path
for p in (Path(u.Paths.project_dir())/'SourceArt/Mars').glob('Radio_*.wav'):
 t=u.AssetImportTask();t.filename=str(p);t.destination_path='/Game/Mars/Audio';t.automated=True;t.replace_existing=True;t.save=True;u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
