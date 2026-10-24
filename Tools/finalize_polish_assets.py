"""Enable skeletal material permutations and validate the cooked art inputs."""
import json
from pathlib import Path
import unreal as u
lib=u.EditorAssetLibrary;root=Path(u.Paths.project_dir()).resolve();result={'skeletal_meshes':[],'tools':[],'radio':[]}
for name in ('Cloth','Rubber','Metal','Blue','Red','Gold','Patch'):
    material=lib.load_asset('/Game/Lunar/Suit/Materials/M_'+name)
    if not material:raise RuntimeError('Missing suit material '+name)
    u.MaterialEditingLibrary.set_material_usage(material,u.MaterialUsage.MATUSAGE_SKELETAL_MESH)
    lib.save_loaded_asset(material)
sub=u.get_editor_subsystem(u.SkeletalMeshEditorSubsystem)
for name in ('SK_A7L','SK_A7L_Hands'):
    mesh=lib.load_asset('/Game/Lunar/Suit/Rig/'+name+'/SkeletalMeshes/'+name)
    if not mesh or not isinstance(mesh,u.SkeletalMesh):raise RuntimeError('Missing real skeletal mesh '+name)
    count=sub.get_lod_count(mesh)
    verts=[sub.get_num_verts(mesh,i) for i in range(count)]
    if count<3 or not (verts[0]>verts[1]>verts[2]>0):raise RuntimeError('Missing reduced LODs '+name)
    slots=mesh.get_editor_property('materials')
    for slot in slots:
        material=lib.load_asset('/Game/Lunar/Suit/Materials/M_'+str(slot.material_slot_name))
        if not material:raise RuntimeError('Unknown suit material slot '+str(slot.material_slot_name))
        slot.material_interface=material
    mesh.set_editor_property('materials',slots);lib.save_loaded_asset(mesh)
    result['skeletal_meshes'].append({'name':name,'vertices_per_lod':verts,'material_slots':len(slots)})
for name in ('SM_ContextCamera','SM_SampleTongs','SM_SoilScoop','SM_DriveTube','SM_GeologyHammer','SM_Cutters'):
    mesh=lib.load_asset('/Game/Lunar/Apollo12/Tools/'+name)
    if not mesh:raise RuntimeError('Missing tool '+name)
    if any(not s.material_interface for s in mesh.get_editor_property('static_materials')):raise RuntimeError('Missing tool material '+name)
    result['tools'].append(name)
for name in ('StartMessage','CheckoutDone','InstrumentsDone','SamplesDone','SurveyorReached','PartsDone','MissionDone'):
    wave=lib.load_asset('/Game/Lunar/Apollo12/Radio/'+name)
    if not wave or not isinstance(wave,u.SoundWave):raise RuntimeError('Missing radio clip '+name)
    result['radio'].append(name)
(root/'Reports/v050-art-validation.json').write_text(json.dumps(result,indent=2))
u.log('POLISH_ART_VALIDATED')
