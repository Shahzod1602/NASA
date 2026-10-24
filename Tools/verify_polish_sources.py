"""Verify immutable science inputs and skin data independently of Unreal."""
import hashlib,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
result={'science_sources':[],'skins':[]}
manifest=json.loads((ROOT/'SourceArt/Apollo12/manifest.json').read_text())
for source in manifest['sources']:
    path=ROOT/'SourceArt/Apollo12/Raw'/source['file']
    digest=hashlib.sha256(path.read_bytes()).hexdigest()
    if path.stat().st_size!=source['bytes'] or digest!=source['sha256']:raise RuntimeError('Altered science source '+str(path))
    result['science_sources'].append({'file':source['file'],'sha256':digest})
subprocess.run(['git','diff','--exit-code','HEAD','--','Source/LunarRescue/ApolloData.h'],cwd=ROOT,check=True)
for path in (ROOT/'SourceArt/Suit/Rig').glob('*.glb'):
    raw=path.read_bytes();magic,version,size=struct.unpack_from('<III',raw)
    if magic!=0x46546c67 or version!=2 or size!=len(raw):raise RuntimeError('Invalid GLB header')
    js_size,kind=struct.unpack_from('<II',raw,12);doc=json.loads(raw[20:20+js_size]);binary=raw[28+js_size:]
    attrs=doc['meshes'][0]['primitives'][0]['attributes']
    def values(key,fmt):
        a=doc['accessors'][attrs[key]];v=doc['bufferViews'][a['bufferView']]
        return list(struct.iter_unpack('<'+fmt,binary[v['byteOffset']:v['byteOffset']+v['byteLength']]))
    joints=values('JOINTS_0','4H');weights=values('WEIGHTS_0','4f')
    if any(abs(sum(w)-1)>.00001 or min(w)<0 for w in weights):raise RuntimeError('Unnormalized skin weights')
    if any(max(j)>=len(doc['skins'][0]['joints']) for j in joints):raise RuntimeError('Invalid joint index')
    if not any(sum(wi>0.01 for wi in w)>1 for w in weights):raise RuntimeError('No blended joint weights')
    if path.stem=='SK_A7L':
        positions=values('POSITION','3f');normals=values('NORMAL','3f')
        cloth=next(p for p in doc['meshes'][0]['primitives'] if doc['materials'][p['material']]['name']=='Cloth')
        a=doc['accessors'][cloth['indices']];v=doc['bufferViews'][a['bufferView']]
        ids={i[0] for i in struct.iter_unpack('<I',binary[v['byteOffset']:v['byteOffset']+v['byteLength']]) if joints[i[0]][0]==3}
        good=sum(sum((positions[i][j]-(0,.64,0)[j])*normals[i][j] for j in range(3))>0 for i in ids)
        if not ids or good/len(ids)<.95:raise RuntimeError('Helmet outer surface normals point inward')
    result['skins'].append({'file':path.name,'skin_vertices':len(weights),'bones':len(doc['skins'][0]['joints']),'normalized':True,'blended':True})
(ROOT/'Reports/v050-source-validation.json').write_text(json.dumps(result,indent=2))
print('Verified',len(result['science_sources']),'unchanged science files and',len(result['skins']),'weighted GLB skins')
