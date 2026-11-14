"""Preserve the credited museum rover and derive a body plus four wheel pivots.

Geometry and UVs are retained. Connected stationary mudguard shells in the
shared Wheels material remain on the chassis; tire shells are separated into
four spatial quadrants with measured circular tire pivots. Embedded textures are capped at 2K.
"""
import argparse, copy, hashlib, io, json, struct, sys
from pathlib import Path

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source',type=Path,required=True)
args=parser.parse_args()
sys.path.insert(0,'C:/Users/shaxz/lunar_python_libs')
import numpy as np
from PIL import Image

root=Path(__file__).resolve().parents[1]
output=root/'SourceArt/MuseumRover20261113'
raw=args.source.read_bytes()
assert hashlib.sha256(raw).hexdigest()=='f416182a410f98e562d50661ae408affa613e67cfd1e9af72e82a1c6d66fb19b'
n=struct.unpack_from('<I',raw,12)[0];source=json.loads(raw[20:20+n]);binary=raw[28+n:]
types={5121:np.dtype('u1'),5123:np.dtype('<u2'),5125:np.dtype('<u4'),5126:np.dtype('<f4')}
widths={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}
def read(index):
 a=source['accessors'][index];v=source['bufferViews'][a['bufferView']];dt=types[a['componentType']];k=widths[a['type']]
 assert 'sparse' not in a
 return np.ndarray((a['count'],k),dt,binary,v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',dt.itemsize*k),dt.itemsize)).copy()

parts=[]
def walk(index,parent):
 node=source['nodes'][index]
 assert not any(k in node for k in ('rotation','scale','translation')), 'Review unexpected TRS source before conversion'
 world=parent@np.array(node.get('matrix',np.eye(4).T.ravel()),float).reshape(4,4).T
 if 'mesh' in node:
  for primitive in source['meshes'][node['mesh']]['primitives']:
   assert primitive.get('mode',4)==4
   values={k:read(v) for k,v in primitive['attributes'].items()}
   values['POSITION']=(values['POSITION']@world[:3,:3].T+world[:3,3]).astype('<f4')
   for name in ('NORMAL','TANGENT'):
    if name in values:
     normals=values[name][:,:3]@np.linalg.inv(world[:3,:3]);normals/=np.maximum(np.linalg.norm(normals,axis=1,keepdims=True),1e-12);values[name][:,:3]=normals
   faces=read(primitive['indices']).reshape(-1,3).astype('<u4')
   if np.linalg.det(world[:3,:3])<0:faces=faces[:,::-1]
   assert np.isfinite(values['POSITION']).all() and faces.max()<len(values['POSITION'])
   parts.append((primitive,values,faces))
 for child in node.get('children',[]):walk(child,world)
for node in source['scenes'][source.get('scene',0)]['nodes']:walk(node,np.eye(4))

all_positions=np.concatenate([values['POSITION'] for _,values,_ in parts])
ground_y=float(all_positions[:,1].min())
# The shared Wheels material also contains stationary orange mudguards.
# Classify complete connected shells against the measured tire envelope; never
# cut triangles or infer a moving part merely from its material name.
def tire_faces(values,faces):
 positions=values['POSITION'];_,inverse=np.unique(np.round(positions,5),axis=0,return_inverse=True)
 indexed=inverse[faces];parent=list(range(int(inverse.max())+1))
 def find(a):
  while parent[a]!=a:parent[a]=parent[parent[a]];a=parent[a]
  return a
 for a,b,c in indexed:
  a=find(int(a));b=find(int(b));c=find(int(c));parent[b]=a;parent[c]=a
 labels=np.array([find(i) for i in range(len(parent))]);shell=labels[indexed[:,0]]
 keep=np.ones(len(faces),dtype=bool)
 for label in np.unique(shell):
  mask=shell==label;points=positions[np.unique(faces[mask])]
  centre_x=.942 if points[:,0].mean()>0 else -1.027
  radial=np.hypot(points[:,0]-centre_x,points[:,1]-.3723)
  if radial.max()>.365:keep[mask]=False
 return keep

groups=[[] for _ in range(5)]
stationary_triangles=0
for primitive,values,faces in parts:
 is_wheel=source['materials'][primitive['material']].get('name')=='Wheels'
 if not is_wheel:groups[0].append((primitive,values,faces));continue
 moving=tire_faces(values,faces)
 if (~moving).any():groups[0].append((primitive,values,faces[~moving]));stationary_triangles+=int((~moving).sum())
 assigned=(~moving).astype(np.uint8)
 for group,(x,z) in enumerate([(1,-1),(1,1),(-1,-1),(-1,1)],1):
  vertices=values['POSITION'][faces]
  keep=moving&(vertices[:,:,0]*x>0).all(axis=1)&(vertices[:,:,2]*z>0).all(axis=1)
  assert keep.any();assigned+=keep;groups[group].append((primitive,values,faces[keep]))
 assert (assigned==1).all(), 'A wheel triangle crosses quadrant boundaries; manual review required'

images=[]
for image in source.get('images',[]):
 view=source['bufferViews'][image['bufferView']];start=view.get('byteOffset',0)
 with Image.open(io.BytesIO(binary[start:start+view['byteLength']])) as picture:
  picture.thumbnail((2048,2048),Image.Resampling.LANCZOS);encoded=io.BytesIO();picture.save(encoded,format='PNG');images.append(encoded.getvalue())

def make(name,group,origin):
 doc={k:copy.deepcopy(source[k]) for k in ('asset','materials','textures','samplers') if k in source}
 used_materials=sorted({p['material'] for p,_,_ in group})
 material_map={old:new for new,old in enumerate(used_materials)}
 doc['materials']=[copy.deepcopy(source['materials'][i]) for i in used_materials]
 used_textures=sorted({m['pbrMetallicRoughness']['baseColorTexture']['index'] for m in doc['materials'] if 'baseColorTexture' in m.get('pbrMetallicRoughness',{})})
 texture_map={old:new for new,old in enumerate(used_textures)}
 used_images=sorted({source['textures'][i]['source'] for i in used_textures});image_map={old:new for new,old in enumerate(used_images)}
 for material in doc['materials']:
  if 'baseColorTexture' in material.get('pbrMetallicRoughness',{}):material['pbrMetallicRoughness']['baseColorTexture']['index']=texture_map[material['pbrMetallicRoughness']['baseColorTexture']['index']]
 doc['textures']=[copy.deepcopy(source['textures'][i]) for i in used_textures]
 for texture in doc['textures']:texture['source']=image_map[texture['source']]
 doc['asset']['generator']='LunarRescue museum-rover adaptation / original credited in SourceArt'
 doc.update(bufferViews=[],accessors=[],images=[],nodes=[{'name':name,'mesh':0}],meshes=[{'name':name,'primitives':[]}],scenes=[{'nodes':[0]}],scene=0)
 data=bytearray()
 def view(payload):
  while len(data)%4:data.append(0)
  start=len(data);data.extend(payload);doc['bufferViews'].append({'buffer':0,'byteOffset':start,'byteLength':len(payload)});return len(doc['bufferViews'])-1
 def accessor(values,kind,component):
  a={'bufferView':view(values.tobytes()),'componentType':component,'count':len(values),'type':kind}
  if kind=='VEC3':a.update(min=values.min(axis=0).tolist(),max=values.max(axis=0).tolist())
  doc['accessors'].append(a);return len(doc['accessors'])-1
 triangles=0
 for primitive,values,faces in group:
  used,remap=np.unique(faces,return_inverse=True);new=copy.deepcopy(primitive);new['attributes']={};new['material']=material_map[primitive['material']]
  for key,index in primitive['attributes'].items():
   old=source['accessors'][index];v=values[key][used].copy()
   if key=='POSITION':v=(v-origin).astype('<f4')
   new_index=accessor(v,old['type'],old['componentType']);new['attributes'][key]=new_index
   if old.get('normalized'):doc['accessors'][new_index]['normalized']=True
  new['indices']=accessor(remap.astype('<u4').reshape(-1,1),'SCALAR',5125);doc['meshes'][0]['primitives'].append(new);triangles+=len(faces)
 for image_index in used_images:doc['images'].append({'bufferView':view(images[image_index]),'mimeType':'image/png'})
 doc['buffers']=[{'byteLength':len(data)}]
 text=json.dumps(doc,separators=(',',':')).encode();text+=b' '*((-len(text))%4);data+=b'\0'*((-len(data))%4)
 payload=struct.pack('<4sII',b'glTF',2,28+len(text)+len(data))+struct.pack('<I4s',len(text),b'JSON')+text+struct.pack('<I4s',len(data),b'BIN\0')+data
 destination=output/'Derived'/(name+'.glb');destination.parent.mkdir(parents=True,exist_ok=True);destination.write_bytes(payload)
 return {'name':name,'triangles':triangles,'sha256':hashlib.sha256(payload).hexdigest(),'bytes':len(payload),'source_pivot_m':origin.tolist()}

entries=[make('SM_MuseumRover_Body',groups[0],np.array([0,ground_y,0]))]
for index,group in enumerate(groups[1:]):
 positions=np.concatenate([values['POSITION'][np.unique(faces)] for _,values,faces in group]);pivot=np.array([.942 if index<2 else -1.027,.3722925,-.774 if index%2==0 else .786])
 entry=make('SM_MuseumRover_Wheel'+str(index),group,pivot);entry['contact_offset_cm']=[float(pivot[0]*100),float(pivot[2]*100),0];entry['radius_cm']=float((positions[:,1].max()-positions[:,1].min())*50);entries.append(entry)
assert sum(e['triangles'] for e in entries)==sum(len(faces) for _,_,faces in parts)
original=output/'Raw'/args.source.name;original.parent.mkdir(parents=True,exist_ok=True)
if original.exists():assert original.read_bytes()==raw
else:original.write_bytes(raw)
manifest={'source':source['asset'],'source_sha256':hashlib.sha256(raw).hexdigest(),'ground_y_m':ground_y,'geometry_triangles_preserved':True,'stationary_mudguard_triangles':stationary_triangles,'separation':'Connected shells outside measured tire circle stay attached to chassis; all source triangles retained','coordinate_contract':'glTF metres; Unreal importer converts Y-up to Z-up centimetres','texture_max_dimension':2048,'derivative_license':'CC-BY-SA-4.0','parts':entries}
(output/'adaptation.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8');print(json.dumps(manifest,indent=2))
