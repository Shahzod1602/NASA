"""Persist only instancing usage on four existing authored presentation materials."""
import json
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve()
paths=['/Game/Lunar/Apollo12/Realism/M_EquipmentDark','/Game/Lunar/Apollo12/Realism/M_EquipmentPaint','/Game/Lunar/Apollo12/Realism/M_LunarFoil','/Game/Lunar/Suit/Materials/M_Metal']
usage=u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES
results=[]
for path in paths:
    material=u.EditorAssetLibrary.load_asset(path)
    if not isinstance(material,u.Material): raise RuntimeError('Expected existing base material: '+path)
    before=u.MaterialEditingLibrary.has_material_usage(material,usage)
    if not before:
        u.MaterialEditingLibrary.set_base_material_usage(material,usage,True)
        u.MaterialEditingLibrary.recompile_material(material)
        if not u.EditorAssetLibrary.save_loaded_asset(material,only_if_is_dirty=False): raise RuntimeError('Material save failed: '+path)
    if not u.MaterialEditingLibrary.has_material_usage(material,usage): raise RuntimeError('Instancing usage did not persist: '+path)
    results.append({'material':path,'already_enabled':before,'instancing_enabled':True})
out=root/'Reports/ExpeditionPolish20261029';out.mkdir(parents=True,exist_ok=True)
(out/'material-usage.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
u.log('PRESENTATION_MATERIAL_USAGE_VERIFIED: four existing base materials, no map writes')
