"""Finalize only new feedback assets and tag redundant authored map decoration.

Existing MoonBase is edited through supported Editor APIs without regeneration.
An exact pre-edit map copy is retained before the targeted visibility changes.
"""
import json,hashlib,re,runpy,shutil
from pathlib import Path
import unreal as u

root=Path(u.Paths.project_dir()).resolve();report=root/'Reports/Feedback20261113';report.mkdir(parents=True,exist_ok=True)
lib=u.EditorAssetLibrary
if not lib.does_asset_exist('/Game/Lunar/UI/PlanetContext20261114/M_PlanetContext'):
 runpy.run_path(str(root/'Tools/import_planet_context.py'),run_name='__main__')

body=lib.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionSuit/SkeletalMeshes/SK_ExpeditionSuit')
hands=lib.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionHands/SkeletalMeshes/SK_ExpeditionHands')
if not body or not hands:raise RuntimeError('Missing new suit derivative')
body_slots={str(s.get_editor_property('material_slot_name')):s.get_editor_property('material_interface') for s in body.get_editor_property('materials')}
slots=hands.get_editor_property('materials')
for slot in slots:
 key=str(slot.get_editor_property('material_slot_name'))
 if key not in body_slots:raise RuntimeError('Unmatched source suit slot '+key)
 slot.set_editor_property('material_interface',body_slots[key])
hands.set_editor_property('materials',slots);lib.save_loaded_asset(hands,only_if_is_dirty=False)
static=u.get_editor_subsystem(u.StaticMeshEditorSubsystem) or u.get_default_object(u.StaticMeshEditorSubsystem)
manifest=json.loads((root/'SourceArt/MuseumRover20261113/adaptation.json').read_text());assets=[]
for part in manifest['parts']:
 name=part['name'];path='/Game/Lunar/Vehicles/Museum20261113/'+name+'/StaticMeshes/'+name;asset=lib.load_asset(path)
 if not asset or static.get_lod_count(asset)!=4:raise RuntimeError('Missing rover LOD chain: '+path)
 if asset.get_editor_property('nanite_settings').enabled:raise RuntimeError('DX11 baseline must not depend on Nanite: '+path)
 verts=[static.get_number_verts(asset,i) for i in range(4)]
 if not all(verts[i]>verts[i+1]>0 for i in range(3)):raise RuntimeError('Rover LOD counts are not decreasing: '+str(verts))
 assets.append({'path':path,'lod_vertices':verts,'nanite':False})

map_file=root/'Content/Lunar/Maps/MoonBase.umap';backup=root/'Backups/Feedback20261114/BeforeDecorationTags/MoonBase.umap'
backup.parent.mkdir(parents=True,exist_ok=True)
if not backup.exists():shutil.copy2(map_file,backup)
backup_sha=hashlib.sha256(backup.read_bytes()).hexdigest()
u.EditorLoadingAndSavingUtils.load_map('/Game/Lunar/Maps/MoonBase')
actors=u.get_editor_subsystem(u.EditorActorSubsystem) or u.get_default_object(u.EditorActorSubsystem)
changed=[]
for actor in actors.get_all_level_actors():
 label=actor.get_actor_label()
 # The foundation/feet belong to the replaced box-habitat prototype. Retiring
 # them prevents the now terrain-grounded HDU from intersecting the old block.
 if label not in ('Rover ID','Lander label','Station foundation','Station feet','SELENE branding','Station number') and not re.fullmatch(r'Route marker (?:pole|cap)\d*',label):continue
 actor.modify();tags=list(actor.tags)
 if 'Lunar.RetiredDecoration' not in map(str,tags):tags.append(u.Name('Lunar.RetiredDecoration'))
 actor.tags=tags;actor.set_actor_hidden_in_game(True);actor.set_actor_enable_collision(False);changed.append(label)
if not changed:raise RuntimeError('Expected redundant authored labels/stakes were not found')
if not u.EditorLoadingAndSavingUtils.save_current_level():raise RuntimeError('Targeted MoonBase visibility edit was not saved')

result={'rover_assets':assets,'suit_material_slots_shared':len(slots),'retired_decoration_labels':changed,'map_backup_sha256':backup_sha,'map_regenerated':False,'mission_actors_deleted':0}
(report/'asset-finalization.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
u.log('FEEDBACK_ASSET_FINALIZATION_COMPLETE '+json.dumps(result))
