"""Repair only this batch's new material assets; no mission map edits."""
import os,runpy
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve()
os.environ['LUNAR_SUIT_REIMPORT']='1';os.environ['LUNAR_CONTEXT_REPAIR']='1'
runpy.run_path(str(root/'Tools/import_expedition_suit.py'),run_name='__main__')
runpy.run_path(str(root/'Tools/import_planet_context.py'),run_name='__main__')
lib=u.EditorAssetLibrary
body=lib.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionSuit/SkeletalMeshes/SK_ExpeditionSuit')
hands=lib.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionHands/SkeletalMeshes/SK_ExpeditionHands')
materials={str(s.get_editor_property('material_slot_name')):s.get_editor_property('material_interface') for s in body.get_editor_property('materials')}
slots=hands.get_editor_property('materials')
for slot in slots:slot.set_editor_property('material_interface',materials[str(slot.get_editor_property('material_slot_name'))])
hands.set_editor_property('materials',slots);lib.save_loaded_asset(hands,only_if_is_dirty=False)
u.log('FEEDBACK_MATERIAL_REPAIR_COMPLETE')
