"""One supported Editor-only MoonBase rover transform repair. No map generation.

Prepare/launch through repair_selene_rover.ps1 -Apply. This script never starts
PIE, changes gameplay objects, edits terrain/materials, or saves other maps.
"""
import datetime
import hashlib
import json
import math
from pathlib import Path
import re
import unreal as u

ROOT=Path(u.Paths.project_dir()).resolve()
MAP='/Game/Lunar/Maps/MoonBase'
MAP_FILE=ROOT/'Content/Lunar/Maps/MoonBase.umap'
COMMAND=u.SystemLibrary.get_command_line()

def arg(name):
    match=re.search(r'(?:^|\s)-'+re.escape(name)+r'=(?:"([^"]+)"|([^\s]+))',COMMAND)
    if not match:raise RuntimeError('Missing explicit repair argument '+name)
    return match.group(1) or match.group(2)

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def xyz(v):return [float(v.x),float(v.y),float(v.z)]
def rot(v):return {'pitch':float(v.pitch),'yaw':float(v.yaw),'roll':float(v.roll)}
def finite(values):return all(math.isfinite(v) for v in values)
def close(a,b,tolerance=.01):return all(abs(x-y)<=tolerance for x,y in zip(a,b))
def mesh_of(c):return c.get_editor_property('static_mesh')
def transform_row(a):return {'location':xyz(a.get_actor_location()),'rotation':rot(a.get_actor_rotation()),'scale':xyz(a.get_actor_scale3d())}
def property_row(a):
    components=[]
    for c in a.get_components_by_class(u.StaticMeshComponent):
        mesh=mesh_of(c)
        components.append({'path':c.get_path_name(),'mesh':mesh.get_path_name() if mesh else None,'materials':[c.get_material(i).get_path_name() if c.get_material(i) else None for i in range(c.get_num_materials())],'collision':str(c.get_collision_enabled()),'profile':str(c.get_collision_profile_name())})
    return {'tags':[str(t) for t in a.get_editor_property('tags')],'scale':xyz(a.get_actor_scale3d()),'components':components}
def stable_hash(rows):return hashlib.sha256(json.dumps(rows,sort_keys=True,separators=(',',':')).encode()).hexdigest()


def terrain_hit_vectors(hit,terrain,terrain_component):
    # FHitResult HasNativeBreak = GameplayStatics.BreakHitResult (UE5.8).
    # Native output order: blocking, overlap, time, distance, location,
    # impact point, normal, impact normal, physical material, actor, component,
    # hit bone, bone, item, element, face, trace start, trace end.
    if hit is None or not callable(getattr(hit,'to_tuple',None)):
        raise RuntimeError('Missing terrain collision or native HitResult break; no repair saved')
    parts=hit.to_tuple()
    if not isinstance(parts,tuple) or len(parts)!=18:
        raise RuntimeError('Unexpected HitResult native-break layout: '+str(type(parts))+' length '+str(len(parts) if hasattr(parts,'__len__') else 'unknown'))
    if parts[0] is not True or parts[1] is not False or parts[9]!=terrain or parts[10]!=terrain_component:
        raise RuntimeError('Trace did not hit the expected terrain without initial overlap; no repair saved')
    ground,normal=parts[5],parts[7]
    if not isinstance(ground,u.Vector) or not isinstance(normal,u.Vector) or not finite(xyz(ground)+xyz(normal)) or normal.z<.95:
        raise RuntimeError('Invalid or steep terrain contact; no repair saved')
    return ground,normal

def main():
    if not re.search(r'(?:^|\s)-RepairSeleneRover(?:\s|$)',COMMAND):raise RuntimeError('Explicit -RepairSeleneRover flag required')
    output=Path(arg('RepairReceipt')).resolve()
    if not output.is_relative_to(ROOT/'Reports') or output.exists():raise RuntimeError('Use a new repair receipt under project Reports')
    expected=arg('RepairMapSHA').lower()
    if not re.fullmatch('[0-9a-f]{64}',expected) or digest(MAP_FILE)!=expected:raise RuntimeError('Map differs from wrapper backup/hash')
    fixture=ROOT/'Tools/fixtures/selene-rover-wheel-contacts.json'
    data=json.loads(fixture.read_text(encoding='utf-8'))
    if len(data['wheels'])!=8 or any(len(w['contacts_local_cm'])!=6 for w in data['wheels']):raise RuntimeError('Expected exactly8 wheel groups and48 source contact points')
    levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
    if not levels.load_level(MAP):raise RuntimeError('Cannot load existing MoonBase')
    world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
    if u.GameplayStatics.get_current_level_name(world,True)!='MoonBase':raise RuntimeError('Wrong editor map; no changes')
    actors=list(u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors())
    rovers=[];terrains=[]
    for a in actors:
        for c in a.get_components_by_class(u.StaticMeshComponent):
            mesh=mesh_of(c)
            if not mesh:continue
            if mesh.get_path_name()=='/Game/Lunar/Imported/Lunokhod/Lunokhod.Lunokhod':rovers.append((a,c))
            if mesh.get_path_name()=='/Game/Lunar/V3/Meshes/SM_LunarTerrain.SM_LunarTerrain':terrains.append((a,c))
    if len(rovers)!=1 or len(terrains)!=1:raise RuntimeError('Require exactly one original Lunokhod and one V3 terrain')
    rover,component=rovers[0];terrain,terrain_component=terrains[0]
    if rover.get_editor_property('root_component')!=component:raise RuntimeError('Rover mesh must remain the actor root; refuse unexpected hierarchy')
    before=transform_row(rover);properties=property_row(rover);rotation=before['rotation']
    if abs(rotation['pitch']-15)>.01 or abs(rotation['yaw'])>.01 or abs(rotation['roll'])>.01:raise RuntimeError('Expected original pitch15/yaw0/roll0; refuse repeated or unrelated repair')
    low,high=component.get_local_bounds()
    if not close(xyz(low),[-103.576640771552,-170,0],.1) or not close(xyz(high),[103.576640771552,170,237.555946193295],.1):raise RuntimeError('Retained rover bounds/axes do not match verified full-wheel source')
    if not finite(before['location']+before['scale']) or min(before['scale'])<=0:raise RuntimeError('Invalid original transform')
    other_before={a.get_path_name():{'transform':transform_row(a),'properties':property_row(a)} for a in actors if a!=rover}
    ignore=[a for a in actors if a!=terrain]
    level_rotation=u.Rotator(pitch=0.0,yaw=15.0,roll=0.0)
    # HasNativeMake exports this as the Transform constructor in Python,
    # not MathLibrary.make_transform. Use named arguments to avoid axis order.
    proposed=u.Transform(location=rover.get_actor_location(),rotation=level_rotation,scale=rover.get_actor_scale3d())
    for owner,name in ((u.MathLibrary,'transform_location'),(u.SystemLibrary,'line_trace_single'),(levels,'save_current_level'),(rover,'modify'),(rover,'set_actor_rotation'),(rover,'set_actor_location')):
        if not callable(getattr(owner,name,None)):raise RuntimeError('Required Python API unavailable before mutation: '+name)
    samples=[]
    for wheel in data['wheels']:
        for point in wheel['contacts_local_cm']:
            p=u.MathLibrary.transform_location(proposed,u.Vector(*point))
            hit=u.SystemLibrary.line_trace_single(world,u.Vector(p.x,p.y,p.z+1500),u.Vector(p.x,p.y,p.z-1500),u.TraceTypeQuery.ECC_VISIBILITY,True,ignore,u.DrawDebugTrace.NONE,True)
            ground,normal=terrain_hit_vectors(hit,terrain,terrain_component)
            samples.append({'wheel':wheel['id'],'source_cm':point,'proposed_world_cm':xyz(p),'terrain_cm':xyz(ground),'required_raise_cm':float(ground.z-p.z+1.0)})
    rise=max(s['required_raise_cm'] for s in samples)
    if not math.isfinite(rise) or abs(rise)>12:raise RuntimeError('Required Z adjustment exceeds12cm guarded scope')
    target=u.Vector(before['location'][0],before['location'][1],before['location'][2]+rise)
    clearances=[s['proposed_world_cm'][2]+rise-s['terrain_cm'][2] for s in samples]
    if min(clearances)<.99 or max(clearances)>9:raise RuntimeError('Terrain clearance outside expected shallow support envelope')
    # The only persistent object changes: this actor's rotation and world Z.
    rover.modify()
    rover.set_actor_rotation(level_rotation,False)
    rover.set_actor_location(target,False,False)
    after=transform_row(rover)
    actual_rotation=after['rotation']
    if abs(actual_rotation['pitch'])>.001 or abs(actual_rotation['roll'])>.001 or abs((actual_rotation['yaw']-15+180)%360-180)>.001 or abs(after['location'][2]-target.z)>.001:raise RuntimeError('Editor did not apply the exact verified target pose; refuse save')
    for sample in samples:
        point=u.MathLibrary.transform_location(component.get_world_transform(),u.Vector(*sample['source_cm']))
        if point.z-sample['terrain_cm'][2]<.99:raise RuntimeError('Applied pose still has a wheel contact below its clearance; refuse save')
    if not close(before['location'][:2],after['location'][:2],.001) or properties!=property_row(rover):raise RuntimeError('Rover XY/scale/tags/material/collision changed unexpectedly')
    other_after={a.get_path_name():{'transform':transform_row(a),'properties':property_row(a)} for a in actors if a!=rover}
    if other_before!=other_after:raise RuntimeError('An unrelated actor changed; refuse save')
    if not levels.save_current_level():raise RuntimeError('Supported save_current_level failed; keep wrapper backup')
    after_sha=digest(MAP_FILE)
    if after_sha==expected:raise RuntimeError('Map save did not change its hash')
    receipt={'success':True,'map':MAP,'map_file':'Content/Lunar/Maps/MoonBase.umap','before_sha256':expected,'after_sha256':after_sha,'actor':rover.get_path_name(),'before':before,'after':after,'unchanged_rover_properties':properties,'other_actor_count':len(other_before),'other_actor_snapshots_equal':True,'other_actor_snapshot_sha256':stable_hash(other_before),'source_contact_fixture_sha256':digest(fixture),'full_original_wheels':data['original_full_wheel_evidence'],'terrain_contact_samples':samples,'clearance_min_cm':min(clearances),'clearance_max_cm':max(clearances),'clearance_by_wheel_cm':{w['id']:min(c for c,s in zip(clearances,samples) if s['wheel']==w['id']) for w in data['wheels']},'only_persistent_changes':'MoonBase existing Lunokhod rotation and world Z; actorXY/scale/tags/material/collision and every other actor preserved','mars_map_changed':False,'nasa_terrain_changed':False,'completed_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
    output.write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    u.log('SELENE_ROVER_REPAIR_COMPLETE '+after_sha)

main()
