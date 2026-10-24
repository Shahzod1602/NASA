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
            wrinkle=1+ribs*sin(t*16*pi)*sin(pi*t)**.6
            return (offset[0]+x,offset[1]+cos(a)*y*wrinkle,offset[2]+sin(a)*z*wrinkle)
        self.grid(max(20,len(profile)*3),n,point,mat)
    def blob(self,c,r,mat='Cloth',box=.8):
        def power(v):return copysign(abs(v)**box,v)
        self.grid(25,40,lambda t,a:(c[0]-r[0]*power(cos(pi*t)),c[1]+r[1]*power(sin(pi*t))*power(cos(a)),c[2]+r[2]*power(sin(pi*t))*power(sin(a))),mat)
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
m=Mesh();m.tube([(0,6.3,6.5),(3,7,7),(10,6.6,6.9),(17,5.9,6.3),(24,5.3,5.6)],ribs=.045)
for x in (2,5,8,11,14,17,20):m.ring(x,6.4-(x/24),6.8-(x/24),.24,'Cloth')
m.save('SM_SuitForearm')
m=Mesh();m.ring(-1.7,5.6,5.8,1,'Rubber');m.ring(-.7,5.8,6,1.3,'Blue');m.ring(.6,5.5,5.7,.65,'Metal');m.save('SM_SuitCuff')
m=Mesh();m.tube([(0,3.9,2.7),(2,4.5,2.8),(6,4.6,2.8),(9,4.1,2.2),(9.2,.1,.1)],lambda t,a:'Rubber' if sin(a)<-.35 else 'Cloth')
m.blob((4.5,0,2.35),(3.5,3.5,.7),'Cloth');m.blob((4.4,0,-2.3),(3.4,3.6,.7),'Rubber')
m.save('SM_SuitPalm')
m=Mesh()
def finger(t,a):
    radius=(.45+.55*sin(pi*(.08+.85*t))**.35)*(1-.4*t)
    return (8*t,cos(a)*1.15*radius,-2.7*t*t+sin(a)*1.35*radius)
m.grid(27,24,finger,lambda t,a:'Rubber' if sin(a)<-.25 or .42<t<.47 or .72<t<.77 else 'Cloth')
m.blob((7.95,0,-2.7),(.6,.58,.65),'Rubber');m.save('SM_SuitFinger')
for name,length,r in [('SM_SuitUpperArm',28,9),('SM_SuitThigh',36,10),('SM_SuitShin',34,8)]:
    m=Mesh();m.tube([(0,r*.95,r),(3,r,r*1.05),(length*.4,r*1.03,r),(length*.8,r*.85,r*.9),(length,r*.79,r*.84)],ribs=.035)
    if name=='SM_SuitUpperArm':m.blob((1,0,0),(8,9.6,9.6))
    if name=='SM_SuitThigh':m.blob((1,0,0),(5,r*.95,r))
    if name in ('SM_SuitUpperArm','SM_SuitThigh'):m.blob((length-.5,0,0),(5,r*.8,r*.85))
    for x in (length*.72,length*.78,length*.84):m.ring(x,r*.87,r*.92,.75,'Cloth')
    m.save(name)
m=Mesh();m.blob((0,0,-10),(13,20,15));m.save('SM_SuitPelvis')
m=Mesh();m.blob((0,0,20),(14,23,31),box=.68)
m.blob((13,0,24),(3.7,12,11),box=.45)
for y,mat in [(-9,'Blue'),(9,'Red')]:
    m.tube([(12,3,3),(16,3,3),(17,2.6,2.6)],mat,offset=(0,y,9))
    m.blob((14,y,36),(2,4.5,4.5),'Patch')
    sign=1 if y>0 else -1
    m.hose([(18,y,9),(30,sign*25,0),(2,sign*31,-4),(-16,sign*18,5)])
m.blob((17,0,26),(1,9,2.6),'Patch')
m.tube([(0,9,9),(3,9,9)],'Metal',offset=(0,0,50))
# Rotate neck ring from X to Z.
for i in range(len(m.v)-20*33,len(m.v)):
    x,y,z=m.v[i];m.v[i]=(50-z,y,50+x)
m.save('SM_SuitTorso')
m=Mesh();m.blob((-17,0,22),(9,17,25),box=.4);m.blob((-25.5,0,20),(1.2,13,19),'Cloth',box=.4)
m.blob((-25.9,0,40),(.8,8,3),'Patch',box=.3);m.save('SM_SuitPack')
m=Mesh();m.blob((0,0,0),(15,16,18),'Cloth');m.blob((10,0,0),(8.7,14.2,13.8),'Rubber');m.blob((12,0,.4),(7.4,12.8,12.4),'Gold');m.save('SM_SuitHelmet')
m=Mesh();m.blob((3,0,-.2),(18,8.8,9.1),box=.5);m.blob((-2,0,8),(8.5,8.2,11),box=.55)
m.blob((4,0,-8.6),(18.5,9.2,2.6),'Rubber',box=.25)
for x in range(-10,21,4):m.blob((x,0,-10.5),(1.1,8.9,.8),'Rubber',box=.25)
m.save('SM_SuitBoot')
(OUT/'CREDITS.txt').write_text('Original mesh geometry authored for LunarRescue. A7L-inspired visual approximation, not a dimensionally exact NASA suit. Generated by Tools/prepare_suit.py. Procedural animations and materials are project-authored.\n')
print('Prepared',len(list(OUT.glob('*.obj'))),'suit meshes')
