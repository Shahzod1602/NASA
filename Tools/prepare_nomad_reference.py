"""Flatten the official NASA/JPL Perseverance visual for a fictional NOMAD prop.

This is a visual substitution in fictional SELENE, not a claim Perseverance
visited the Moon. Mission target, rewards and interaction authority stay intact.
"""
from datetime import datetime, timezone
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'C:/Users/shaxz/lunar_python_libs')
import trimesh
import numpy as np

root=Path(__file__).resolve().parents[1];folder=root/'SourceArt/Perseverance20261114';source=folder/'Raw/Mars2020Perseverance.glb'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='10db7c03a5e63a5a3b3e7baa6243aa4918ba045fa8ff0a731d0217491adc727f'
scene=trimesh.load(source,force='scene',process=False)
derived=folder/'Derived/SM_NomadReference.glb';derived.parent.mkdir(exist_ok=True)
export_scene=trimesh.Scene();count=0
for i,node in enumerate(scene.graph.nodes_geometry):
 transform,name=scene.graph.get(node);mesh=scene.geometry[name].copy();mesh.apply_transform(transform);mesh.vertices[:,1]-=scene.bounds[0,1]
 assert np.isfinite(mesh.vertices).all();count+=len(mesh.faces)
 material=mesh.visual.material
 for channel in ['baseColorTexture','normalTexture','metallicRoughnessTexture','occlusionTexture','emissiveTexture']:
  image=getattr(material,channel,None)
  if image is not None:image.thumbnail((2048,2048))
 export_scene.add_geometry(mesh,node_name='Part'+str(i),geom_name='Part'+str(i))
assert count==199521
# Export every baked primitive with its original PBR material, then merge the
# glTF mesh containers. Atlas packing could create an invalid 32768-pixel strip.
packed=trimesh.exchange.gltf.export_glb(export_scene);json_length=struct.unpack_from('<I',packed,12)[0];document=json.loads(packed[20:20+json_length]);binary=packed[28+json_length:]
assert all(not any(k in n for k in ['matrix','translation','rotation','scale']) for n in document['nodes'])
primitives=[p for m in document['meshes'] for p in m['primitives']]
document['meshes']=[{'name':'SM_NomadReference','primitives':primitives}];document['nodes']=[{'name':'SM_NomadReference','mesh':0}];document['scenes']=[{'nodes':[0]}];document['scene']=0
encoded=json.dumps(document,separators=(',',':')).encode();encoded+=b' '*((-len(encoded))%4)
derived.write_bytes(struct.pack('<4sII',b'glTF',2,28+len(encoded)+len(binary))+struct.pack('<I4s',len(encoded),b'JSON')+encoded+struct.pack('<I4s',len(binary),b'BIN\0')+binary)
verification=trimesh.load(derived,force='scene',process=False)
assert sum(len(verification.geometry[verification.graph.get(n)[1]].faces) for n in verification.graph.nodes_geometry)==199521
manifest={'source_page':'https://science.nasa.gov/3d-resources/mars-2020-perseverance-rover/','source_url':'https://assets.science.nasa.gov/content/dam/science/cds/3d/resources/model/mars-2020-perseverance-rover/Mars%202020%20Perseverance%20Rover.glb','credit':'NASA/Jet Propulsion Laboratory','usage':'https://www.nasa.gov/nasa-brand-center/images-and-media/','repository_statement':'https://raw.githubusercontent.com/nasa/NASA-3D-Resources/master/README.md','downloaded_utc':datetime.now(timezone.utc).isoformat(),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'derivative_sha256':hashlib.sha256(derived.read_bytes()).hexdigest(),'triangles':199521,'steps':['Decode Draco without simplification','Bake node transforms and merge static visual','Preserve source materials/UVs; translate minimum Y to zero','Import with authored static LODs'],'coordinates':'glTF Y-up metres; Unreal converts to Z-up centimetres','gameplay_role':'Fictional NOMAD-7 visual only in SELENE; mission memory recovery remains unchanged','limits':['Perseverance is a Mars mission, never a historical Moon mission','This visual substitution does not model real rover operation or imply NASA endorsement']}
(folder/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps({'file':str(derived),'bytes':derived.stat().st_size,'triangles':199521,'bounds_m':verification.bounds.tolist()}))
