"""Original low-poly visual approximations of Apollo hand tools, in cm."""
from pathlib import Path
import math
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'SourceArt/Apollo12/Tools';OUT.mkdir(parents=True,exist_ok=True)
(OUT/'Tools.mtl').write_text('newmtl Metal\nKd .4 .44 .46\nnewmtl Rubber\nKd .055 .065 .07\nnewmtl Blue\nKd .035 .12 .28\n')
class Mesh:
 def __init__(self):self.v=[];self.f=[]
 def box(self,c,s,mat='Metal'):
  first=len(self.v)
  for x,y,z in ((-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)):self.v.append(tuple(c[i]+(x,y,z)[i]*s[i]/2 for i in range(3)))
  for ids in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)):
   self.f.extend([(mat,(first+ids[0],first+ids[1],first+ids[2])),(mat,(first+ids[0],first+ids[2],first+ids[3]))])
 def rod(self,a,b,r,mat='Metal',n=16):
  d=[b[i]-a[i] for i in range(3)];length=sum(v*v for v in d)**.5;d=[v/length for v in d]
  ref=(0,0,1) if abs(d[2])<.9 else (1,0,0)
  u=(ref[1]*d[2]-ref[2]*d[1],ref[2]*d[0]-ref[0]*d[2],ref[0]*d[1]-ref[1]*d[0]);l=sum(x*x for x in u)**.5;u=[x/l for x in u]
  v=(d[1]*u[2]-d[2]*u[1],d[2]*u[0]-d[0]*u[2],d[0]*u[1]-d[1]*u[0]);start=len(self.v)
  for p in (a,b):
   for i in range(n):
    t=i/n*2*math.pi;self.v.append(tuple(p[j]+r*(u[j]*math.cos(t)+v[j]*math.sin(t)) for j in range(3)))
  for i in range(n):
   j=(i+1)%n;self.f.extend([(mat,(start+i,start+n+i,start+n+j)),(mat,(start+i,start+n+j,start+j))])
  for k in range(1,n-1):self.f.extend([(mat,(start,start+k+1,start+k)),(mat,(start+n,start+n+k,start+n+k+1))])
 def save(self,name):
  with (OUT/(name+'.obj')).open('w') as f:
   f.write('mtllib Tools.mtl\no '+name+'\ns 1\n')
   for x,y,z in self.v:f.write(f'v {x:.5f} {-y:.5f} {z:.5f}\n')
   mat=''
   for m,ids in self.f:
    if mat!=m:f.write('usemtl '+m+'\n');mat=m
    f.write('f '+' '.join(str(i+1) for i in ids)+'\n')
m=Mesh();m.box((0,0,0),(10,13,10),'Rubber');m.box((-5,0,0),(2,12,9));m.box((1,0,6),(8,8,2));m.rod((5,0,0),(13,0,0),3);m.rod((12.5,0,0),(13.2,0,0),2.5,'Blue');m.box((0,-8,0),(6,2,8));m.box((0,8,0),(6,2,8));m.save('SM_ContextCamera')
m=Mesh()
for sign in (-1,1):m.rod((-14,sign*5,0),(12,sign*1.5,0),.6);m.rod((12,sign*1.5,0),(22,sign*3,-4),.65);m.rod((-14,sign*5,0),(-8,sign*5,0),1,'Rubber')
m.rod((10,-3,0),(10,3,0),1);m.save('SM_SampleTongs')
m=Mesh();m.rod((-18,0,0),(8,0,0),.8);m.rod((-18,0,0),(-10,0,0),1.2,'Rubber');m.box((14,0,-2),(14,9,1));m.box((14,-4.5,0),(14,1,4));m.box((14,4.5,0),(14,1,4));m.box((21,0,0),(1,9,4));m.save('SM_SoilScoop')
m=Mesh();m.rod((0,0,-18),(0,0,18),1.35);m.rod((0,0,17),(0,0,19),1.8);m.rod((0,0,-19),(0,0,-17),1.8);m.save('SM_DriveTube')
m=Mesh();m.rod((0,0,-15),(0,0,8),.8,'Rubber');m.box((0,0,10),(12,5,5));m.save('SM_GeologyHammer')
m=Mesh()
for sign in (-1,1):m.rod((-14,sign*4,0),(6,sign*1.5,0),.75);m.rod((-14,sign*4,0),(-6,sign*3.5,0),1.1,'Rubber');m.box((9,sign*1.5,0),(6,2,3))
m.rod((5,-3,0),(5,3,0),1.5);m.save('SM_Cutters')
(OUT/'CREDITS.txt').write_text('Project-authored visual approximations of a Hasselblad-style context camera, tongs, scoop, drive tube, geology hammer and cutters. Rebuild with Tools/prepare_apollo_tools.py. Not museum scans or engineering models. Apollo 12 core collection is represented as a hammer-driven tube; no powered drill is used. Historical references are in HISTORICAL_AUDIT.md.\n')
print('Prepared 6 original tool meshes')
