"""Reproducible, project-authored A7L-inspired skin, skeleton and animation GLBs.

No downloaded model or rig. Coordinates in the authoring code are Unreal cm.
Cloth joint collars blend across two bones; rigid hardware remains rigid.
"""
import json, math, struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'SourceArt/Suit/Rig'
OUT.mkdir(parents=True, exist_ok=True)
add = lambda a,b: tuple(x+y for x,y in zip(a,b))
sub = lambda a,b: tuple(x-y for x,y in zip(a,b))
mul = lambda a,s: tuple(x*s for x in a)
dot = lambda a,b: sum(x*y for x,y in zip(a,b))
cross = lambda a,b: (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def norm(a): return mul(a,1/max(.00001,math.sqrt(dot(a,a))))
# Verified against UE 5.8 Interchange bind coordinates: glTF (X,Y,Z)
# imports as Unreal centimetres (X,Z,Y). Preserve the authored forward/right.
def convert(p): return (p[0]/100,p[2]/100,p[1]/100)
bones=[]
def bone(name, parent, pos):
    bones.append((name,parent,pos));return len(bones)-1
root=bone('root',-1,(0,0,0));pelvis=bone('pelvis',root,(0,0,-12))
chest=bone('chest',pelvis,(0,0,30));head=bone('head',chest,(0,0,64))
legs=[];arms=[]
for side,sign in [('l',-1),('r',1)]:
    hip=bone('thigh_'+side,pelvis,(0,sign*12,-12))
    knee=bone('calf_'+side,hip,(4,sign*12,-47))
    foot=bone('foot_'+side,knee,(3,sign*12,-78));legs.append((hip,knee,foot))
    upper=bone('upperarm_'+side,chest,(0,sign*25,40))
    lower=bone('lowerarm_'+side,upper,(3,sign*34,14))
    hand=bone('hand_'+side,lower,(6,sign*38,-9));digits=[]
    for f in range(5):
        p=add(bones[hand][2],(0,sign*(-3.15+f*2.1) if f<4 else -sign*4,-9 if f<4 else -4))
        a=bone(('finger%d_'%f)+side,hand,p)
        b=bone(('tip%d_'%f)+side,a,add(p,(0,0,-4)));digits.append((a,b))
    arms.append((upper,lower,hand,digits))

COLORS={'Cloth':(.52,.54,.51),'Rubber':(.055,.065,.07),'Metal':(.4,.44,.46),'Blue':(.035,.12,.28),'Red':(.42,.025,.015),'Gold':(.63,.36,.065),'Patch':(.065,.10,.14)}
def frame(a,b):
    x=norm(sub(b,a)); y=norm(cross((0,0,1) if abs(x[2])<.95 else (1,0,0),x));z=cross(x,y)
    return lambda p:add(a,add(mul(x,p[0]),add(mul(y,p[1]),mul(z,p[2]))))

def generate(name, hands=False):
    vertices=[];uvs=[];joints=[];weights=[];colors=[];faces={k:[] for k in COLORS}
    def vertex(p,uv,skin):
        vertices.append(convert(p));uvs.append(uv)
        # Bind-space dust mask stays on the garment as the bones move. Green=0
        # distinguishes authored masks from a static mesh's default white color.
        dust=min(.78,max(.035,(-p[2]-12)/100))
        colors.append((dust,0.,0.,1.))
        joints.append(tuple([i for i,w in skin]+[0]*(4-len(skin))))
        weights.append(tuple([w for i,w in skin]+[0]*(4-len(skin))))
        return len(vertices)-1
    def part(obj,transform,skin):
        vs=[];ts=[];mat='Cloth';cache={}
        for line in (ROOT/'SourceArt/Suit'/('SM_Suit'+obj+'.obj')).read_text().splitlines():
            t=line.split()
            if not t:continue
            if t[0]=='v':vs.append((float(t[1]),-float(t[2]),float(t[3])))
            elif t[0]=='vt':ts.append((float(t[1]),1-float(t[2])))
            elif t[0]=='usemtl':mat=t[1]
            elif t[0]=='f':
                ids=[]
                for item in t[1:]:
                    v,u=map(int,item.split('/')[:2]);key=(v,u)
                    if key not in cache:
                        p=vs[v-1];cache[key]=vertex(transform(p),ts[u-1],skin(p))
                    ids.append(cache[key])
                faces[mat].extend(ids)
    fixed=lambda i:lambda p:[(i,1)]
    identity=lambda p:p
    def collar(a,b,point,radius):
        # Continuous soft collar, weighted rather than separate articulated balls.
        axis=norm(sub(bones[b][2],bones[a][2]));basis=frame(point,add(point,axis))
        start=len(vertices);n=20;rings=9
        for j in range(rings):
            t=j/(rings-1);r=radius*(1+.045*math.cos(t*math.pi*10))
            for i in range(n+1):
                angle=i/n*2*math.pi
                vertex(basis(((t-.5)*10,r*math.cos(angle),r*math.sin(angle))),(i/n,t),[(a,1-t),(b,t)])
        for j in range(rings-1):
            for i in range(n):
                q=start+j*(n+1)+i
                faces['Cloth'].extend((q,q+n+2,q+1,q,q+n+1,q+n+2))
    if not hands:
        part('Torso',identity,fixed(chest));part('Pack',identity,fixed(chest))
        part('Pelvis',identity,fixed(pelvis));part('Helmet',lambda p:add(p,bones[head][2]),fixed(head))
        collar(pelvis,chest,(0,0,1),14)
        for hip,knee,foot in legs:
            part('Thigh',frame(bones[hip][2],bones[knee][2]),fixed(hip))
            part('Shin',frame(bones[knee][2],bones[foot][2]),fixed(knee))
            part('Boot',lambda p,f=foot:add(p,bones[f][2]),fixed(foot))
            collar(hip,knee,bones[knee][2],7.5)
    for upper,lower,hand,digits in arms:
        if not hands:
            part('UpperArm',frame(bones[upper][2],bones[lower][2]),fixed(upper))
            collar(upper,lower,bones[lower][2],6.5)
        part('Forearm',frame(bones[lower][2],bones[hand][2]),fixed(lower))
        part('Cuff',frame(bones[hand][2],add(bones[hand][2],(0,0,-10))),fixed(hand))
        part('Palm',frame(bones[hand][2],add(bones[hand][2],(0,0,-10))),fixed(hand))
        for a,b in digits:
            xf=frame(bones[a][2],add(bones[a][2],(0,0,-8)))
            part('Finger',xf,lambda p,a=a,b=b:[(a,1-min(1,max(0,(p[0]-2)/4))),(b,min(1,max(0,(p[0]-2)/4)))])
    # This Y-up conversion includes a reflection. Its winding is therefore
    # already outward; independently verify the helmet normals before import.
    binary=bytearray();views=[];accessors=[]
    def acc(data,fmt,typ,component,minimum=None,maximum=None):
        while len(binary)%4:binary.append(0)
        offset=len(binary)
        for row in data:binary.extend(struct.pack('<'+fmt,*row))
        views.append({'buffer':0,'byteOffset':offset,'byteLength':len(binary)-offset})
        result={'bufferView':len(views)-1,'componentType':component,'count':len(data),'type':typ}
        if minimum is not None:result.update(min=minimum,max=maximum)
        accessors.append(result);return len(accessors)-1
    pos=acc(vertices,'3f','VEC3',5126,[min(p[i] for p in vertices) for i in range(3)],[max(p[i] for p in vertices) for i in range(3)])
    # Smooth normals are computed per position, including duplicated UV seam vertices.
    normal_sums={}
    for ids in faces.values():
        for j in range(0,len(ids),3):
            a,b,c=[vertices[i] for i in ids[j:j+3]];n=cross(sub(b,a),sub(c,a))
            for p in (a,b,c):normal_sums[p]=add(normal_sums.get(p,(0,0,0)),n)
    normals=acc([norm(normal_sums.get(p,(0,1,0))) for p in vertices],'3f','VEC3',5126)
    uv=acc(uvs,'2f','VEC2',5126);ji=acc(joints,'4H','VEC4',5123);we=acc(weights,'4f','VEC4',5126)
    vc=acc(colors,'4f','VEC4',5126)
    mats=[];primitives=[]
    for k,color in COLORS.items():
        mats.append({'name':k,'pbrMetallicRoughness':{'baseColorFactor':[*color,1],'metallicFactor':.85 if k in ('Metal','Gold','Blue') else 0,'roughnessFactor':.82 if k=='Cloth' else .4}})
        if faces[k]:primitives.append({'attributes':{'POSITION':pos,'NORMAL':normals,'TEXCOORD_0':uv,'JOINTS_0':ji,'WEIGHTS_0':we,'COLOR_0':vc},'indices':acc([(v,) for v in faces[k]],'I','SCALAR',5125),'material':len(mats)-1})
    nodes=[];inverse=[]
    for i,(n,parent,p) in enumerate(bones):
        v=convert(p);local=convert(sub(p,bones[parent][2])) if parent>=0 else v
        node={'name':n,'translation':local};children=[j for j,b in enumerate(bones) if b[1]==i]
        if children:node['children']=children
        nodes.append(node);inverse.append((1,0,0,0,0,1,0,0,0,0,1,0,-v[0],-v[1],-v[2],1))
    inv=acc(inverse,'16f','MAT4',5126)
    nodes.append({'name':name,'mesh':0,'skin':0})
    # Editable reference clips. Runtime uses terrain-aware bone poses on this same rig.
    times=[(i/24,) for i in range(25)];ta=acc(times,'f','SCALAR',5126,[0],[1]);samplers=[];channels=[]
    for side,(hip,knee,foot) in enumerate(legs):
        rotations=[]
        for i in range(25):
            angle=math.sin(i/24*2*math.pi+side*math.pi)*.25
            rotations.append((0,0,math.sin(angle/2),math.cos(angle/2)))
        ro=acc(rotations,'4f','VEC4',5126);samplers.append({'input':ta,'output':ro,'interpolation':'LINEAR'});channels.append({'sampler':len(samplers)-1,'target':{'node':hip,'path':'rotation'}})
    doc={'asset':{'version':'2.0','generator':'LunarRescue project-authored suit rig v0.5'},'scene':0,'scenes':[{'nodes':[0,len(nodes)-1]}],'nodes':nodes,'meshes':[{'name':name,'primitives':primitives}],'skins':[{'name':'A7L_ProjectRig','inverseBindMatrices':inv,'joints':list(range(len(bones))),'skeleton':0}],'materials':mats,'animations':[{'name':'LunarStride','samplers':samplers,'channels':channels}],'buffers':[{'byteLength':len(binary)}],'bufferViews':views,'accessors':accessors}
    js=json.dumps(doc,separators=(',',':')).encode();js+=b' '*((-len(js))%4);binary+=b'\0'*((-len(binary))%4)
    glb=struct.pack('<III',0x46546c67,2,12+8+len(js)+8+len(binary))+struct.pack('<II',len(js),0x4e4f534a)+js+struct.pack('<II',len(binary),0x004e4942)+binary
    (OUT/(name+'.glb')).write_bytes(glb)
    print(name,len(vertices),'vertices',sum(map(len,faces.values()))//3,'triangles',len(bones),'bones')

generate('SK_A7L');generate('SK_A7L_Hands',True)
(OUT/'rig.json').write_text(json.dumps({'units':'Unreal centimetres','bones':[{'name':n,'parent':p,'position':v} for n,p,v in bones]},indent=2))
(OUT/'CREDITS.txt').write_text('Original project-authored A7L-inspired geometry, skin weights, skeleton and reference stride. Rebuild with Tools/prepare_suit.py and Tools/prepare_suit_rig.py. Not a scanned or dimensionally exact historical suit. No third-party model or animation. Runtime poses are procedural, not motion capture.\n')
