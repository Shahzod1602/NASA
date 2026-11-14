"""Adapt the credited LasquetiSpice skin to the existing gameplay rig.

Original geometry/UVs/materials are retained; an authored bind-pose/weight
adaptation targets the existing 36-bone procedural controller. The original
floating clip stays in the unchanged source, not in ground locomotion.
"""
import argparse
import copy
import hashlib
import io
import json
import math
from pathlib import Path
import struct
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source', type=Path, required=True)
parser.add_argument('--dependency-dir', default='C:/Users/shaxz/lunar_python_libs')
args = parser.parse_args()
sys.path.insert(0, args.dependency_dir)
import numpy as np
from PIL import Image

root = Path(__file__).resolve().parents[1]
out = root / 'SourceArt/ExpeditionSuit20261113'
out.mkdir(parents=True, exist_ok=True)
source = args.source.read_bytes()
expected = '6008551be3e7f9668b73d2cfb12c1c7499db6e432303210f753791873f5c861d'
assert hashlib.sha256(source).hexdigest() == expected
n = struct.unpack_from('<I', source, 12)[0]
doc = json.loads(source[20:20+n])
binary = source[28+n:]
types = {5121:np.uint8, 5123:np.dtype('<u2'), 5125:np.dtype('<u4'), 5126:np.dtype('<f4')}
widths = dict(SCALAR=1, VEC2=2, VEC3=3, VEC4=4, MAT4=16)

def read(index):
    a = doc['accessors'][index]
    assert 'sparse' not in a
    v = doc['bufferViews'][a['bufferView']]
    dtype = np.dtype(types[a['componentType']]); width = widths[a['type']]
    start = v.get('byteOffset',0) + a.get('byteOffset',0)
    data = np.ndarray((a['count'],width), dtype, binary, start,
                      strides=(v.get('byteStride',dtype.itemsize*width),dtype.itemsize)).copy()
    if a.get('normalized') and a['componentType'] != 5126:
        data = data.astype(float) / np.iinfo(dtype).max
    return data

def unit(v):
    length = np.linalg.norm(v)
    assert math.isfinite(length) and length > 1e-9
    return v / length

def align(a,b):
    a,b=unit(a),unit(b);v=np.cross(a,b);c=float(np.dot(a,b))
    if c < -.99999:
        axis=unit(np.cross(a,np.eye(3)[np.argmin(np.abs(a))]))
        return 2*np.outer(axis,axis)-np.eye(3)
    k=np.array([[0,-v[2],v[1]],[v[2],0,-v[0]],[-v[1],v[0],0]])
    return np.eye(3)+k+k@k/max(1e-9,1+c)

def frame(primary,lateral):
    """A direction alone cannot determine the roll of a palm or finger."""
    x=unit(primary);y=unit(lateral-x*np.dot(x,lateral))
    result=np.stack([x,y,np.cross(x,y)],axis=1)
    assert np.allclose(result.T@result,np.eye(3),atol=1e-8) and np.linalg.det(result)>.99999
    return result

skin=doc['skins'][0]
joint_nodes=skin['joints']
bind=np.linalg.inv(read(skin['inverseBindMatrices']).reshape(-1,4,4).transpose(0,2,1))
source_positions=bind[:,:3,3]
names=[doc['nodes'][i].get('name','') for i in joint_nodes]
parents={child:i for i,node in enumerate(doc['nodes']) for child in node.get('children',[])}
node_joint={node:j for j,node in enumerate(joint_nodes)}

def find(prefix):
    found=[i for i,name in enumerate(names) if name.startswith(prefix)]
    assert len(found)==1, (prefix,found)
    return found[0]

legacy=json.loads((root/'SourceArt/Suit/Rig/rig.json').read_text())['bones']
target_names=[b['name'] for b in legacy]
target_positions=np.array([[b['position'][0],b['position'][2],b['position'][1]] for b in legacy])/100
mapping={find('GLTF_created_0_rootJoint'):'root',find('Hip.'):'pelvis',
         find('Spine_3.'):'chest',find('head.'):'head'}
for side,letter in [('l','L'),('r','R')]:
    for original,target in [('Arm','upperarm'),('Elbow','lowerarm'),('Wrist','hand'),
                            ('Thigh','thigh'),('Knee','calf'),('Ankle','foot')]:
        mapping[find(letter+'_'+original+'.')]=target+'_'+side
    for digit,original in enumerate(['index','middle','ring','pinky','thumb']):
        mapping[find(letter+'_'+original+'_1.')]=f'finger{digit}_{side}'
        mapping[find(letter+'_'+original+'_3.')]=f'tip{digit}_{side}'
assert set(mapping.values())==set(target_names)
original_for={name:j for j,name in mapping.items()}
right=unit(source_positions[find('R_Thigh.')]-source_positions[find('L_Thigh.')])
up=unit(source_positions[find('head.')]-source_positions[find('Hip.')])
forward=unit(np.cross(up,right));right=unit(np.cross(forward,up))
basis=np.stack([forward,up,right])
height=float(np.dot(source_positions[find('head.')]-source_positions[find('L_Ankle.')],up))
scale=(target_positions[target_names.index('head'),1]-target_positions[target_names.index('foot_l'),1])/height
assert .001 < scale < 100
source_frame=source_positions@basis.T*scale
source_frame-=source_frame[find('Hip.')]-target_positions[target_names.index('pelvis')]

# Each existing controller bone gets a direction-preserving local correction.
child_direction={'root':'chest','pelvis':'chest','chest':'head','head':'head'}
for side in ('l','r'):
    child_direction.update({f'upperarm_{side}':f'lowerarm_{side}',f'lowerarm_{side}':f'hand_{side}',
                            f'hand_{side}':f'finger1_{side}',f'thigh_{side}':f'calf_{side}',
                            f'calf_{side}':f'foot_{side}',f'foot_{side}':f'foot_{side}'})
    for digit in range(5):
        child_direction[f'finger{digit}_{side}']=f'tip{digit}_{side}'
        child_direction[f'tip{digit}_{side}']=f'tip{digit}_{side}'
rotations={}
stretches={}
palm_checks=[]
for name in target_names:
    child=child_direction[name]
    if child==name:
        parent=legacy[target_names.index(name)]['parent']
        rotations[name]=rotations.get(target_names[parent],np.eye(3)) if parent>=0 else np.eye(3)
    else:
        rotations[name]=align(source_frame[original_for[child]]-source_frame[original_for[name]],
                              target_positions[target_names.index(child)]-target_positions[target_names.index(name)])
    stretches[name]=np.eye(3)
    if child!=name and name.startswith(('upperarm_','lowerarm_','hand_','finger')):
        source_axis=source_frame[original_for[child]]-source_frame[original_for[name]]
        target_axis=target_positions[target_names.index(child)]-target_positions[target_names.index(name)]
        ratio=np.linalg.norm(target_axis)/np.linalg.norm(source_axis);assert .2<ratio<4
        direction=unit(source_axis);stretches[name]+=(ratio-1)*np.outer(direction,direction)
        if name.startswith(('hand_','finger')):
            side=name[-1]
            source_lateral=source_frame[original_for['finger0_'+side]]-source_frame[original_for['finger3_'+side]]
            target_lateral=target_positions[target_names.index('finger0_'+side)]-target_positions[target_names.index('finger3_'+side)]
            source_basis=frame(source_axis,source_lateral);target_basis=frame(target_axis,target_lateral)
            old_lateral=rotations[name]@source_basis[:,1]
            rotations[name]=target_basis@source_basis.T
            if name.startswith('hand_'):
                # Fit the same palm's length and MCP spread, preserving depth.
                # The small shear accounts for the non-orthogonal source spread.
                sl=source_basis.T@source_lateral;tl=target_basis.T@target_lateral
                fit=np.eye(3);fit[0,0]=ratio;fit[1,1]=tl[1]/sl[1]
                fit[0,1]=(tl[0]-ratio*sl[0])/sl[1]
                assert .2<fit[1,1]<4 and np.linalg.det(fit)>0
                stretches[name]=source_basis@fit@source_basis.T
                after=rotations[name]@source_basis[:,1]
                before_angle=float(np.degrees(np.arccos(np.clip(np.dot(old_lateral,target_basis[:,1]),-1,1))))
                after_angle=float(np.degrees(np.arccos(np.clip(np.dot(after,target_basis[:,1]),-1,1))))
                assert after_angle<.001
                palm_checks.append(dict(side=side,previous_roll_error_degrees=before_angle,corrected_roll_error_degrees=after_angle,palm_width_scale=float(fit[1,1])))

# Helper/twist segments inherit their closest explicitly mapped ancestor.
joint_map=[]
for j,node in enumerate(joint_nodes):
    cursor=node
    while node_joint.get(cursor) not in mapping:
        cursor=parents.get(cursor)
        if cursor is None:break
    name=mapping.get(node_joint.get(cursor),'root')
    # These torso/accessory branches are siblings of the mapped hip/spine,
    # rather than descendants. Folding them to the global origin stretches
    # the backpack and lower torso. They follow the anatomical torso anchor.
    original_name=names[j]
    if original_name.startswith(('Center.','Spine_1.')):name='pelvis'
    elif original_name.startswith(('Spine_2.','bagPack.','L_bagPackHandle.','R_bagPackHandle.','necklace_')):name='chest'
    joint_map.append(target_names.index(name))
joint_map=np.array(joint_map,dtype=np.uint16)
transforms=[]
for j,target in enumerate(joint_map):
    name=target_names[int(target)];rotation=rotations[name]@stretches[name]
    matrix=np.eye(4);matrix[:3,:3]=rotation@basis*scale
    matrix[:3,3]=target_positions[target]-rotation@source_frame[original_for[name]]
    # source_frame includes a common translated origin.
    origin_shift=source_frame[j]-basis@source_positions[j]*scale
    matrix[:3,3]+=rotation@origin_shift
    transforms.append(matrix)
transforms=np.array(transforms)
normal_transforms=np.linalg.inv(transforms[:,:3,:3]).transpose(0,2,1)
# An arm's adapted source endpoint must meet the next legacy bind pivot. A
# rotation-only retarget leaves a long sleeve extending over its glove.
for name in target_names:
    child=child_direction[name]
    if child!=name and name.startswith(('upperarm_','lowerarm_','hand_','finger')):
        at=transforms[original_for[name]]@np.r_[source_positions[original_for[child]],1]
        assert np.allclose(at[:3],target_positions[target_names.index(child)],atol=1e-5),name

prepared=[]
for mesh in doc['meshes']:
    for primitive in mesh['primitives']:
        attributes=primitive['attributes'];positions=read(attributes['POSITION']).astype(float)
        joints=read(attributes['JOINTS_0']).astype(int);weights=read(attributes['WEIGHTS_0']).astype(float)
        assert np.all(np.isfinite(weights)) and np.allclose(weights.sum(axis=1),1,atol=1e-5)
        homogeneous=np.column_stack([positions,np.ones(len(positions))])
        moved=np.zeros((len(positions),3));normals=np.zeros_like(moved)
        source_normals=read(attributes['NORMAL']).astype(float)
        for k in range(4):
            matrices=transforms[joints[:,k]]
            moved+=np.einsum('nij,nj->ni',matrices,homogeneous)[:,:3]*weights[:,k,None]
            normals+=np.einsum('nij,nj->ni',normal_transforms[joints[:,k]],source_normals)*weights[:,k,None]
        normals/=np.maximum(np.linalg.norm(normals,axis=1)[:,None],1e-12)
        mapped=joint_map[joints]
        attrs={'POSITION':moved.astype('<f4'),'NORMAL':normals.astype('<f4'),
               'JOINTS_0':mapped.astype('<u2'),'WEIGHTS_0':weights.astype('<f4')}
        for key,index in attributes.items():
            if key.startswith('TEXCOORD_'):attrs[key]=read(index).astype('<f4')
        prepared.append((attrs,read(primitive['indices']).flatten().astype('<u4'),primitive.get('material',0)))

def write(name,hands):
    blob=bytearray();views=[];accessors=[]
    def append(data):
        while len(blob)%4:blob.append(0)
        start=len(blob);blob.extend(data);views.append(dict(buffer=0,byteOffset=start,byteLength=len(data)))
        return len(views)-1
    def accessor(array):
        array=np.ascontiguousarray(array)
        if array.ndim==1:array=array[:,None]
        width=array.shape[1];component={np.dtype('<f4'):5126,np.dtype('<u2'):5123,np.dtype('<u4'):5125}[array.dtype]
        data=dict(bufferView=append(array.tobytes()),componentType=component,count=len(array),type={1:'SCALAR',2:'VEC2',3:'VEC3',4:'VEC4',16:'MAT4'}[width])
        if component==5126 and width==3:data.update(min=array.min(axis=0).tolist(),max=array.max(axis=0).tolist())
        accessors.append(data);return len(accessors)-1
    primitives=[]
    allowed=np.array([n.startswith(('lowerarm_','hand_','finger','tip')) for n in target_names])
    for attributes,indices,material in prepared:
        selected=indices.reshape(-1,3)
        if hands:
            hand_weight=np.sum(allowed[attributes['JOINTS_0']]*attributes['WEIGHTS_0'],axis=1)
            selected=selected[np.all(hand_weight[selected]>.5,axis=1)]
        if not len(selected):continue
        used,inverse=np.unique(selected.flatten(),return_inverse=True)
        primitives.append(dict(attributes={k:accessor(v[used]) for k,v in attributes.items()},
                               indices=accessor(inverse.astype('<u4')),material=material))
    assert primitives
    nodes=[];inverse=[]
    for i,bone in enumerate(legacy):
        position=target_positions[i];parent=bone['parent']
        node=dict(name=bone['name'],translation=(position-target_positions[parent] if parent>=0 else position).tolist())
        children=[j for j,b in enumerate(legacy) if b['parent']==i]
        if children:node['children']=children
        nodes.append(node);matrix=np.eye(4);matrix[:3,3]=-position;inverse.append(matrix.T.flatten())
    inv=accessor(np.array(inverse,dtype='<f4'))
    nodes.append(dict(name=name,mesh=0,skin=0))
    images=[]
    for original in doc.get('images',[]):
        v=doc['bufferViews'][original['bufferView']];data=binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']]
        image=Image.open(io.BytesIO(data));image.thumbnail((2048,2048),Image.Resampling.LANCZOS)
        encoded=io.BytesIO();image.save(encoded,format='PNG')
        images.append(dict(bufferView=append(encoded.getvalue()),mimeType='image/png'))
    # UE's imported legacy specular/glossiness instances did not retain diffuse
    # colour in the rendered review. Use core glTF metallic/roughness inputs.
    # This is an explicit visual approximation, not a measured suit material.
    materials=copy.deepcopy(doc['materials']);textures=copy.deepcopy(doc['textures'])
    for material in materials:
        spec=material.get('extensions',{}).pop('KHR_materials_pbrSpecularGlossiness',None)
        if not spec:continue
        if not material.get('extensions'):material.pop('extensions',None)
        gloss=spec.get('glossinessFactor',1.)
        pbr={'baseColorFactor':spec.get('diffuseFactor',[1,1,1,1]),'metallicFactor':0.,'roughnessFactor':max(.12,1-gloss)}
        if 'diffuseTexture' in spec:pbr['baseColorTexture']=spec['diffuseTexture']
        if 'specularGlossinessTexture' in spec:
            entry=spec['specularGlossinessTexture'];original=doc['images'][doc['textures'][entry['index']]['source']];v=doc['bufferViews'][original['bufferView']];start=v.get('byteOffset',0)
            picture=Image.open(io.BytesIO(binary[start:start+v['byteLength']])).convert('RGBA');picture.thumbnail((2048,2048),Image.Resampling.LANCZOS)
            alpha=np.asarray(picture)[:,:,3].astype(float)/255
            roughness=np.maximum(.12,1-alpha*gloss)
            packed=np.zeros((*alpha.shape,3),np.uint8);packed[:,:,0]=255;packed[:,:,1]=(roughness*255).round().astype(np.uint8)
            encoded=io.BytesIO();Image.fromarray(packed).save(encoded,format='PNG');image_index=len(images);images.append(dict(bufferView=append(encoded.getvalue()),mimeType='image/png'))
            texture_index=len(textures);textures.append({'source':image_index});pbr['roughnessFactor']=1.;pbr['metallicRoughnessTexture']={'index':texture_index,'texCoord':entry.get('texCoord',0)}
        material['pbrMetallicRoughness']=pbr
    result=dict(asset=dict(version='2.0',generator='LunarRescue credited skin adaptation 2026-11-13',extras=doc['asset']['extras']),
                scene=0,scenes=[dict(nodes=[0,len(nodes)-1])],nodes=nodes,
                meshes=[dict(name=name,primitives=primitives)],
                skins=[dict(name='ExpeditionGameplayRig',inverseBindMatrices=inv,joints=list(range(len(legacy))),skeleton=0)],
                materials=materials,images=images,textures=textures,
                samplers=doc.get('samplers',[]),extensionsUsed=[x for x in doc.get('extensionsUsed',[]) if x!='KHR_materials_pbrSpecularGlossiness'],extensionsRequired=[x for x in doc.get('extensionsRequired',[]) if x!='KHR_materials_pbrSpecularGlossiness'],
                accessors=accessors,bufferViews=views,buffers=[dict(byteLength=len(blob))])
    js=json.dumps(result,separators=(',',':')).encode();js+=b' '*((-len(js))%4);blob+=b'\0'*((-len(blob))%4)
    packed=struct.pack('<III',0x46546c67,2,28+len(js)+len(blob))+struct.pack('<II',len(js),0x4e4f534a)+js+struct.pack('<II',len(blob),0x004e4942)+blob
    destination=out/'Derived'/(name+'.glb');destination.parent.mkdir(exist_ok=True)
    destination.write_bytes(packed)
    return dict(file=destination.relative_to(root).as_posix(),bytes=len(packed),sha256=hashlib.sha256(packed).hexdigest(),
                triangles=sum(accessors[p['indices']]['count']//3 for p in primitives),bones=len(legacy),materials=len(result['materials']))

original=out/'Raw'/args.source.name;original.parent.mkdir(exist_ok=True)
if original.exists():assert original.read_bytes()==source
else:original.write_bytes(source)
results=[write('SK_ExpeditionSuit',False),write('SK_ExpeditionHands',True)]
(out/'adaptation.json').write_text(json.dumps(dict(source_sha256=expected,uniform_source_scale=scale,
    source_bones=len(joint_nodes),gameplay_bones=len(legacy),arm_segment_endpoints_fit=True,palm_frames=palm_checks,outputs=results,
    limits=['Authored bind-pose adaptation; not a historical Apollo A7L reconstruction.',
            'Original floating animation retained only in unchanged Raw source.',
            'Helper weights folded to nearest mapped ancestor; deformation needs rendered review.',
            'Legacy specular/glossiness adapted to core metallic/roughness after rendered colour failure.',
            'Base colour/normal UVs preserved; authored metallic=0 and roughness=max(0.12,1-source glossiness).',
            'Embedded images capped at 2K.']),indent=2)+'\n')
print(json.dumps(results,indent=2))
