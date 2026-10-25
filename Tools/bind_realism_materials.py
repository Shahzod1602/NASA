"""Bind shared suit materials by replacing array values, then read them back."""
import json
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve();lib=u.EditorAssetLibrary;results=[]
for name in ('SK_A7L','SK_A7L_Hands'):
    mesh=lib.load_asset('/Game/Lunar/Suit/Rig/'+name+'/SkeletalMeshes/'+name)
    if not mesh:raise RuntimeError('Missing suit mesh '+name)
    slots=list(mesh.get_editor_property('materials'));before=[]
    for i,slot in enumerate(slots):
        key=str(slot.material_slot_name)
        before.append(slot.material_interface.get_path_name() if slot.material_interface else '')
        material=lib.load_asset('/Game/Lunar/Suit/Materials/M_'+key)
        if not material:raise RuntimeError('Unrecognized suit slot '+key)
        slot.material_interface=material;slots[i]=slot
    mesh.set_editor_property('materials',slots)
    lib.save_loaded_asset(mesh,only_if_is_dirty=False)
    after=[slot.material_interface.get_path_name() for slot in mesh.get_editor_property('materials')]
    if any('/Game/Lunar/Suit/Materials/M_' not in path for path in after):raise RuntimeError('Material assignment did not persist')
    results.append({'mesh':name,'before':before,'after':after})
(root/'Reports/realism-material-bindings.json').write_text(json.dumps(results,indent=2))
u.log('REALISM_SUIT_BINDINGS_VERIFIED')
