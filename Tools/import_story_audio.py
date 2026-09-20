import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir())
for f in (root/'SourceArt/Immersion').glob('Radio_Story*.wav'):
    task=u.AssetImportTask()
    task.filename=str(f)
    task.destination_path='/Game/Lunar/Immersion/Audio'
    task.automated=True
    task.replace_existing=True
    task.save=True
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths: raise RuntimeError('Failed audio import '+f.name)
u.log('STORY_AUDIO_IMPORTED')
