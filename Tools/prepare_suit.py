"""Author centimetre-scale A7L-inspired suit meshes. No external model dependency."""
from pathlib import Path
from math import sin, cos, pi, copysign
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'SourceArt/Suit'; OUT.mkdir(parents=True,exist_ok=True)
class Mesh:
    def __init__(self): self.v=[]; self.uv=[]; self.f=[]
    def grid(self, rings, n, point, material):
        first=len(self.v)
        for j in range(rings):
            for i in range(n+1):
                self.v.append(point(j/(rings-1),i/n*2*pi));self.uv.append((i/n,j/(rings-1)))
        for j in range(rings-1):
            for i in range(n):
                a=first+j*(n+1)+i; b=a+1; c=a+n+1;d=c+1
                mat=material((j+.5)/(rings-1),(i+.5)/n*2*pi) if callable(material) else material
                self.f.extend([(mat,(a,d,b)),(mat,(a,c,d))])
    def tube(self, profile, mat='Cloth', offset=(0,0,0), ribs=0, n=32):
        def point(t,a):
            p=t*(len(profile)-1);k=min(int(p),len(profile)-2);u=p-k
            x,y,z=[profile[k][i]*(1-u)+profile[k+1][i]*u for i in range(3)]
            wrinkle=1+ribs*sin(t*13*pi+sin(a*3)*.7)*(0.25+.75*sin(pi*t))
            return (offset[0]+x,offset[1]+cos(a)*y*wrinkle,offset[2]+sin(a)*z*wrinkle)
        self.grid(max(20,len(profile)*3),n,point,mat)
    def blob(self,c,r,mat='Cloth',box=.8,rings=25,n=40):
        def power(v):return copysign(abs(v)**box,v)
        self.grid(rings,n,lambda t,a:(c[0]-r[0]*power(cos(pi*t)),c[1]+r[1]*power(sin(pi*t))*power(cos(a)),c[2]+r[2]*power(sin(pi*t))*power(sin(a))),mat)
    def ring(self,x,ry,rz,length,mat='Metal'):
        self.tube([(x,ry*.94,rz*.94),(x+.12,ry,rz),(x+length-.12,ry,rz),(x+length,ry*.94,rz*.94)],mat)
    def hose(self,points,radius=1.6):
        def center(t):
            return tuple((1-t)**3*points[0][i]+3*(1-t)**2*t*points[1][i]+3*(1-t)*t*t*points[2][i]+t**3*points[3][i] for i in range(3))
        def point(t,a):
            c=center(t);a0=center(max(0,t-.001));b=center(min(1,t+.001));d=[b[i]-a0[i] for i in range(3)];ln=sum(x*x for x in d)**.5;d=[x/ln for x in d]
            normal=(-d[1],d[0],0);ln=max(.001,sum(x*x for x in normal)**.5);normal=[x/ln for x in normal]
            up=(d[1]*normal[2]-d[2]*normal[1],d[2]*normal[0]-d[0]*normal[2],d[0]*normal[1]-d[1]*normal[0])
            r=radius*(1+.10*sin(t*100*pi))
            return tuple(c[i]+r*(normal[i]*cos(a)+up[i]*sin(a)) for i in range(3))
        self.grid(180,16,point,'Rubber')
    def save(self,name):
        # Remove pole degeneracies before Unreal computes normals and tangents.
        valid=[]
        for mat,ids in self.f:
            a,b,c=[tuple(round(v,5) for v in self.v[i]) for i in ids]
            u=[b[i]-a[i] for i in range(3)];v=[c[i]-a[i] for i in range(3)]
            cross=(u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0])
            if sum(x*x for x in cross)>1e-10:valid.append((mat,ids))
        used=sorted(set(i for _,ids in valid for i in ids));remap={old:new for new,old in enumerate(used)}
        with (OUT/(name+'.obj')).open('w') as f:
            f.write('mtllib Suit.mtl\no '+name+'\ns 1\n')
            # OBJ legacy importer mirrors Y; export the desired Unreal coordinate system.
            for i in used:
                x,y,z=self.v[i];f.write(f'v {x:.5f} {-y:.5f} {z:.5f}\n')
            for i in used:
                u,v=self.uv[i];f.write(f'vt {u:.6f} {v:.6f}\n')
            last=''
            for mat,ids in valid:
                if mat!=last:f.write('usemtl '+mat+'\n');last=mat
                f.write('f '+' '.join(f'{remap[i]+1}/{remap[i]+1}' for i in ids)+'\n')

COLORS={'Cloth':(.78,.79,.75),'Rubber':(.055,.065,.07),'Metal':(.4,.44,.46),'Blue':(.06,.21,.38),'Red':(.56,.055,.035),'Gold':(.63,.36,.065),'Patch':(.15,.22,.29)}
(OUT/'Suit.mtl').write_text(''.join(f'newmtl {n}\nKd {r} {g} {b}\n' for n,(r,g,b) in COLORS.items()))
# Layered pressure-garment silhouette, with concentrated joint folds rather
# than evenly inflated rings. Geometry remains an authored approximation.
m=Mesh();m.tube([(0,6.6,6.7),(2,6.9,7),(5,6.4,6.6),(10,6.1,6.4),(17,5.6,5.9),(22,5.1,5.4),(24,5,5.2)],ribs=.035)
for x in (2,4.3,20.7,22.2):m.ring(x,6.8-x*.072,7-x*.072,.22,'Cloth')
# Flattened outer reinforcement and paired sewn edges.
m.blob((12,0,6.05),(6.7,3,.32),'Cloth',box=.65,rings=9,n=16)
for y in (-3.1,3.1):m.blob((12,y,5.65),(6.4,.14,.18),'Cloth',rings=9,n=16)
m.save('SM_SuitForearm')
m=Mesh();m.ring(-1.7,5.1,5.3,.8,'Rubber');m.ring(-.9,5.35,5.5,1.15,'Blue');m.ring(.25,5.1,5.3,.55,'Metal')
for y in (-4.9,4.9):m.blob((-.15,y,2),(.65,.4,.65),'Metal',rings=9,n=16)
m.save('SM_SuitCuff')
m=Mesh();m.tube([(0,3.9,2.5),(1.5,4.2,2.6),(4.5,4.45,2.6),(7.4,4.2,2.35),(9,3.85,1.8),(9.3,.15,.15)],lambda t,a:'Rubber' if sin(a)<-.65 else 'Cloth')
m.blob((4.6,0,-2.05),(3.2,3.5,.55),'Rubber',box=.75,rings=9,n=16)
# Separate dorsal pads and raised seam edges read at first-person distance.
for y in (-2.9,-.95,.95,2.9):
    m.blob((6,y,2.25),(2.4,.72,.48),'Cloth',rings=9,n=16)
    m.blob((3.6,y,2.55),(1.7,.12,.12),'Cloth',rings=9,n=16)
for y in (-4.05,4.05):m.blob((4.2,y,.6),(3.2,.18,.25),'Cloth',rings=9,n=16)
m.save('SM_SuitPalm')
m=Mesh()
def finger(t,a):
    # Tapered closed end, mild knuckle swelling, underside grip reinforcement.
    cap=min(1, max(0,(1-t)/.13))**.5
    radius=(1-.29*t)*cap*(1+.08*sin(t*3*pi)**2)
    return (8*t,cos(a)*1.05*radius,-.9*t*t+sin(a)*1.2*radius)
m.grid(29,24,finger,lambda t,a:'Rubber' if sin(a)<-.5 else 'Cloth')
for x in (2.9,5.2):m.blob((x,0,1.02-.9*(x/8)**2),(.28,.78,.17),'Cloth',rings=9,n=16)
m.save('SM_SuitFinger')
for name,length,r in [('SM_SuitUpperArm',28,8.3),('SM_SuitThigh',36,9.8),('SM_SuitShin',34,7.7)]:
    m=Mesh();m.tube([(0,r*.9,r*.93),(3,r,r),(length*.3,r*.98,r),(length*.64,r*.89,r*.94),(length*.85,r*.8,r*.87),(length,r*.78,r*.83)],ribs=.032)
    # Closed shallow end gussets prevent hollow tubes at exposed joints.
    m.blob((0,0,0),(2.5,r*.92,r*.96),'Cloth',rings=11,n=24)
    m.blob((length,0,0),(1.7,r*.78,r*.83),'Cloth',rings=9,n=20)
    # Overlapping cloth gussets replace the conspicuous spherical joint caps.
    for x in (length*.76,length*.83,length*.9):m.ring(x,r*.82,r*.89,.38,'Cloth')
    if name=='SM_SuitUpperArm':
        m.tube([(-3,6.9,7.1),(0,8.3,8.6),(3,8.5,8.7),(7,8,8.3)],ribs=.015)
        m.blob((-2,0,0),(3.2,7.9,8.1),'Cloth',rings=11,n=24)
        m.blob((9,0,8.05),(4.5,3.3,.35),'Cloth',rings=9,n=16)
    if name=='SM_SuitThigh':m.blob((length-4,0,r*.82),(4.8,5.1,.7),'Cloth',box=.65,rings=9,n=16)
    if name=='SM_SuitShin':
        for y in (-3.5,3.5):m.blob((length*.43,y,r*.84),(8.3,.17,.22),'Cloth',rings=9,n=16)
    m.save(name)
m=Mesh();m.blob((0,0,-10),(13,20,15));m.save('SM_SuitPelvis')
m=Mesh();m.blob((0,0,20),(14,23,31),box=.68)
m.blob((13,0,26),(3.1,10,7),box=.4)
for y,mat in [(-9,'Blue'),(9,'Red')]:
    m.tube([(12,3,3),(16,3,3),(17,2.6,2.6)],mat,offset=(0,y,9))
    m.blob((14,y,36),(2,4.5,4.5),'Patch')
    sign=1 if y>0 else -1
    m.hose([(18,y,9),(30,sign*25,0),(2,sign*31,-4),(-16,sign*18,5)])
m.blob((17,0,26),(1,9,2.6),'Patch')
# Raised seams, chest-unit switches and connector retaining rims.
for y in (-17,17):m.blob((12.8,y,22),(.35,.55,20),'Cloth',box=.3)
for y in (-5,-1.7,1.7,5):
    m.blob((17.3,y,29),(.7,.65,1.4),'Metal',box=.3)
for y in (-9,9):
    m.tube([(16.6,3.2,3.2),(17.5,3.2,3.2)],'Metal',offset=(0,y,9))
m.blob((14.1,-11,38),(.5,3.8,4.4),'Cloth',box=.3)
m.tube([(0,9,9),(3,9,9)],'Metal',offset=(0,0,50))
# Rotate neck ring from X to Z.
for i in range(len(m.v)-20*33,len(m.v)):
    x,y,z=m.v[i];m.v[i]=(50-z,y,50+x)
m.save('SM_SuitTorso')
m=Mesh();m.blob((-17,0,22),(9,17,25),box=.4);m.blob((-25.5,0,20),(1.2,13,19),'Cloth',box=.4)
m.blob((-25.9,0,40),(.8,8,3),'Patch',box=.3)
for y in (-10,10):m.blob((-27,y,22),(.45,1,21),'Patch',box=.3)
for z in (3,18,37):m.blob((-27.2,0,z),(.45,13,.35),'Cloth',box=.3)
m.save('SM_SuitPack')
m=Mesh();m.blob((0,0,0),(15,16,18),'Cloth',box=.56)
# Apollo 12's cloth hood and visor form a rounded rectangle, rather than a ball.
m.blob((11,0,-1),(6.7,13.2,12.8),'Rubber',box=.46)
m.blob((13.2,0,-1),(4.4,11.6,10.7),'Gold',box=.56)
for y in (-13.4,13.4):m.blob((10,y,2),(3.5,.8,11.5),'Cloth',box=.35)
m.blob((9.5,0,14),(6,14,.9),'Cloth',box=.3)
for y in (-15.2,15.2):
    m.blob((3,y,1),(3.1,1.1,4),'Metal',box=.6)
    m.blob((3,y*1.045,1),(1.7,.35,1.7),'Rubber',box=.7)
m.save('SM_SuitHelmet')
m=Mesh();m.blob((3,0,-.2),(18,8.8,9.1),box=.5);m.blob((-2,0,8),(8.5,8.2,11),box=.55)
m.blob((4,0,-8.6),(18.5,9.2,2.6),'Rubber',box=.25)
for x in range(-10,21,4):m.blob((x,0,-10.5),(1.1,8.9,.8),'Rubber',box=.25)
for x in (-4,7):m.blob((x,0,8),(1.4,8.5,.7),'Cloth',box=.3)
m.save('SM_SuitBoot')
(OUT/'CREDITS.txt').write_text('Original mesh geometry authored for LunarRescue. A7L-inspired visual approximation, not a dimensionally exact NASA suit. Generated by Tools/prepare_suit.py. Procedural animations and materials are project-authored. Glove and garment refinement assisted by OpenAI Codex (2026-10-29).\n')
print('Prepared',len(list(OUT.glob('*.obj'))),'suit meshes')
