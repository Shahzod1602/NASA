"""Import the revised suit, rebuild LODs, then apply the authored surface pass."""
import json
from pathlib import Path
import unreal as u
root=Path(u.Paths.project_dir()).resolve()
scripts=('apply_apollo_realism.py',) if '-realismfinishonly' in u.SystemLibrary.get_command_line().lower() else ('import_suit.py','import_suit_rig.py','apply_apollo_realism.py')
for filename in scripts:
    path=root/'Tools'/filename
    exec(compile(path.read_text(encoding='utf-8'),str(path),'exec'),{'__name__':'__main__','__file__':str(path)})
sub=u.get_editor_subsystem(u.SkeletalMeshEditorSubsystem)
results=[]
for name in ('SK_A7L','SK_A7L_Hands'):
    mesh=u.EditorAssetLibrary.load_asset('/Game/Lunar/Suit/Rig/'+name+'/SkeletalMeshes/'+name)
    if not sub.regenerate_lod(mesh,3,True,False):raise RuntimeError('LOD rebuild failed: '+name)
    vertices=[sub.get_num_verts(mesh,i) for i in range(sub.get_lod_count(mesh))]
    if len(vertices)!=3 or not vertices[0]>vertices[1]>vertices[2]>0:raise RuntimeError('Invalid LODs: '+name)
    u.EditorAssetLibrary.save_loaded_asset(mesh)
    results.append({'name':name,'vertices_per_lod':vertices})
(root/'Reports/realism-suit-lods.json').write_text(json.dumps(results,indent=2))
u.EditorAssetLibrary.save_directory('/Game/Lunar/Suit',recursive=True)
path=root/'Tools/bind_realism_materials.py'
exec(compile(path.read_text(encoding='utf-8'),str(path),'exec'),{'__name__':'__main__','__file__':str(path)})
u.log('APOLLO_REALISM_ASSETS_READY')
