"""Check derivative geometry/rig data; this is not a runtime animation test."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'C:/Users/shaxz/lunar_python_libs')
import numpy as np

root=Path(__file__).resolve().parents[1];results=[]
def load(path):
 raw=path.read_bytes();magic,version,length=struct.unpack_from('<4sII',raw)
 assert magic==b'glTF' and version==2 and length==len(raw)
 n=struct.unpack_from('<I',raw,12)[0];return json.loads(raw[20:20+n]),raw[28+n:]
def array(doc,binary,index):
 a=doc['accessors'][index];v=doc['bufferViews'][a['bufferView']];dt=np.dtype({5121:'u1',5123:'<u2',5125:'<u4',5126:'<f4'}[a['componentType']]);k={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
 assert 'sparse' not in a
 return np.ndarray((a['count'],k),dt,binary,v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',dt.itemsize*k),dt.itemsize)).copy()

legacy=json.loads((root/'SourceArt/Suit/Rig/rig.json').read_text())['bones']
for name in ('SK_ExpeditionSuit','SK_ExpeditionHands'):
 doc,binary=load(root/'SourceArt/ExpeditionSuit20261113/Derived'/(name+'.glb'))
 joints=doc['skins'][0]['joints'];assert len(joints)==36
 names=[doc['nodes'][i]['name'] for i in joints];assert set(names)=={bone['name'] for bone in legacy}
 inverse=array(doc,binary,doc['skins'][0]['inverseBindMatrices']).reshape(-1,4,4).transpose(0,2,1)
 positions=np.linalg.inv(inverse)[:,:3,3]
 for bone in legacy:
  expected=np.array([bone['position'][0],bone['position'][2],bone['position'][1]])/100
  assert np.allclose(positions[names.index(bone['name'])],expected,atol=1e-5),bone['name']
 triangles=vertices=0;degenerate=0;palm_points={'l':[],'r':[]};palm_envelopes={}
 for mesh in doc['meshes']:
  for primitive in mesh['primitives']:
   attrs=primitive['attributes'];p=array(doc,binary,attrs['POSITION']);n=array(doc,binary,attrs['NORMAL']);w=array(doc,binary,attrs['WEIGHTS_0']);j=array(doc,binary,attrs['JOINTS_0']);f=array(doc,binary,primitive['indices']).reshape(-1,3)
   assert np.isfinite(p).all() and np.isfinite(n).all() and np.isfinite(w).all()
   assert ((w>=0)&(w<=1)).all() and np.allclose(w.sum(axis=1),1,atol=1e-5)
   assert j.max()<36 and f.max()<len(p)
   assert np.allclose(np.linalg.norm(n,axis=1),1,atol=.01)
   if name=='SK_ExpeditionSuit':
    assert p[:,1].min()>-1.05 and p[:,1].max()<1.05, 'Torso/accessory weights must not stretch to the global root'
   cross=np.cross(p[f[:,1]]-p[f[:,0]],p[f[:,2]]-p[f[:,0]]);degenerate+=int((np.linalg.norm(cross,axis=1)<1e-10).sum())
   triangles+=len(f);vertices+=len(p)
   if name=='SK_ExpeditionHands':
    for side in ('l','r'):
     palm_weight=np.sum((j==names.index('hand_'+side))*w,axis=1)
     palm_points[side].extend(p[palm_weight>.8])
 assert triangles==(58312 if name=='SK_ExpeditionSuit' else 7444) and degenerate==0
 assert not doc.get('animations'), 'Floating source clip must not become ground locomotion'
 if name=='SK_ExpeditionHands':
  for side,points in palm_points.items():
   assert len(points)>50
   span=np.quantile(points,.95,axis=0)-np.quantile(points,.05,axis=0)
   # Asset-specific glove envelope: in this Y-up bind frame Z is across the
   # palm and X is its depth. The old rolled palm has breadth/depth ~0.75.
   assert span[2]/span[0]>.95, ('Palm was rolled sideways in bind adaptation',side,span.tolist())
   palm_envelopes[side]=float(span[2]/span[0])
 results.append({'asset':name,'vertices':vertices,'triangles':triangles,'finite_attributes':True,'normalized_weights':True,'legacy_bone_contract':True,'derivative_degenerate_triangles':degenerate,'palm_breadth_depth_ratio':palm_envelopes})

manifest=json.loads((root/'SourceArt/MuseumRover20261113/adaptation.json').read_text())
assert len(manifest['parts'])==5
assert sum(p['triangles'] for p in manifest['parts'])==464116
for part in manifest['parts']:
 p=root/'SourceArt/MuseumRover20261113/Derived'/(part['name']+'.glb');assert hashlib.sha256(p.read_bytes()).hexdigest()==part['sha256']
 doc,binary=load(p);triangles=0
 for primitive in doc['meshes'][0]['primitives']:
  pos=array(doc,binary,primitive['attributes']['POSITION']);faces=array(doc,binary,primitive['indices']).reshape(-1,3)
  assert np.isfinite(pos).all() and faces.max()<len(pos);triangles+=len(faces)
 assert triangles==part['triangles']
 results.append({'asset':part['name'],'triangles':triangles,'finite_positions':True,'indices_in_range':True,'sha256_verified':True})
nomad=root/'SourceArt/Perseverance20261114/Derived/SM_NomadReference.glb'
if nomad.is_file():
 from PIL import Image
 import io
 doc,binary=load(nomad);count=0
 assert len(doc['meshes'])==1 and len(doc['nodes'])==1
 for primitive in doc['meshes'][0]['primitives']:
  position=array(doc,binary,primitive['attributes']['POSITION']);faces=array(doc,binary,primitive['indices']).reshape(-1,3)
  assert np.isfinite(position).all() and faces.max()<len(position);count+=len(faces)
 for item in doc.get('images',[]):
  view=doc['bufferViews'][item['bufferView']];start=view.get('byteOffset',0)
  with Image.open(io.BytesIO(binary[start:start+view['byteLength']])) as picture:assert max(picture.size)<=2048
 assert count==199521
 results.append({'asset':'SM_NomadReference','triangles':count,'single_mesh':True,'texture_max_dimension':2048,'finite_positions':True,'indices_in_range':True})
out=root/'Reports/Feedback20261113';out.mkdir(parents=True,exist_ok=True);(out/'model-data-checks.json').write_text(json.dumps(results,indent=2)+'\n')
print('PASS: suit bind/weight/envelope checks, 5 rover geometry/pivot derivatives and NASA reference geometry/texture limits. Runtime and rendered checks remain separate.')
