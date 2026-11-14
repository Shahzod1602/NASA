"""Share only new suit materials and render its real, warmed preview."""
from pathlib import Path
import runpy
import unreal as u
root=Path(u.Paths.project_dir()).resolve();lib=u.EditorAssetLibrary
body=lib.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionSuit/SkeletalMeshes/SK_ExpeditionSuit')
hands=lib.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionHands/SkeletalMeshes/SK_ExpeditionHands')
lookup={str(s.get_editor_property('material_slot_name')):s.get_editor_property('material_interface') for s in body.get_editor_property('materials')}
slots=hands.get_editor_property('materials')
for slot in slots:slot.set_editor_property('material_interface',lookup[str(slot.get_editor_property('material_slot_name'))])
hands.set_editor_property('materials',slots);lib.save_loaded_asset(hands,only_if_is_dirty=False)
lib.load_asset('/Game/Lunar/UI/PlanetContext20261114/M_PlanetContext')
runpy.run_path(str(root/'Tools/render_suit_preview.py'),run_name='__main__')
u.log('FEEDBACK_VISUAL_FINALIZATION_COMPLETE')
