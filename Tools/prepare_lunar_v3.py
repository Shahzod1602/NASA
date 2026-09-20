"""Deterministic game geometry in OBJ; no surveyed lunar elevations are claimed."""
from pathlib import Path
import math, json, urllib.request,sys
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'SourceArt/LunarV3'
ART.mkdir(parents=True,exist_ok=True)
sources={
 'stars.6.json':'https://raw.githubusercontent.com/ofrohn/d3-celestial/master/data/stars.6.json',
 'T_Earth_Clouds.jpg':'https://raw.githubusercontent.com/mrdoob/three.js/dev/examples/textures/planets/earth_atmos_2048.jpg',
}
for name,url in []:
    dest=ART/name
    if not dest.exists():
        urllib.request.urlretrieve(url,dest)
    print(name,dest.stat().st_size,flush=True)

def noise(x,y):
    return (np.sin(x*.00071+np.sin(y*.00048)*2.1)*np.cos(y*.00057+x*.00012)*.48
           +np.sin(x*.0019-y*.0011)*np.sin(y*.0023+x*.0008)*.28
           +np.sin(x*.0049+y*.0043)*np.cos(y*.0061-x*.0037)*.15
           +np.sin(x*.019-y*.013)*np.cos(y*.023+x*.016)*.09)

craters=[(4500,2200,1400,330),(-4100,4100,1800,450),(7200,-6000,2500,650),
         (-7000,-6300,2800,720),(11700,6900,3400,900),(16500,-11500,4300,1100),
         (-15000,12500,5200,1350),(19000,18000,6100,1400)]
rng=np.random.default_rng(1969)
for i in range(105):
    cx,cy=rng.uniform(-29000,29000,2)
    r=rng.uniform(150,650)
    craters.append((float(cx),float(cy),float(r),float(r*.19)))

# Small overlapping impacts break up the smooth original basin.
for i in range(210):
    cx,cy=rng.uniform(-9500,9500,2)
    if -3400<cx<5300 and -4400<cy<800: continue
    r=float(rng.uniform(110,480))
    craters.append((float(cx),float(cy),r,r*.24))

def heights(x,y):
    z=noise(x,y)*75+noise(x*.23,y*.23)*150
    for cx,cy,r,depth in craters:
        t=np.arctan2(y-cy,x-cx)
        d=np.hypot(x-cx,y-cy)/(r*(1+.045*np.sin(t*7+cx)+.025*np.cos(t*13+cy)))
        bowl=-depth*np.maximum(0,1-d*d)**1.15
        rim=depth*(.34+.07*np.sin(t*9+cx))*np.exp(-((d-1.01)/.115)**2)
        ejecta=depth*.055*np.exp(-((d-1.25)/.44)**2)*(1+.5*np.sin(t*31+cx))
        bowl+=ejecta+noise((x-cx)*2,(y-cy)*2)*depth*.09*np.exp(-((d-.95)/.6)**2)
        z+=bowl+rim
    # Flat working area feathered into the natural surface; no steps at the boundary.
    dx=np.maximum(np.maximum(-3200-x,x-5000),0)
    dy=np.maximum(np.maximum(-4200-y,y-650),0)
    blend=np.clip(np.hypot(dx,dy)/1400,0,1)
    blend=blend*blend*(3-2*blend)
    return z*blend + noise(x*3.1,y*3.1)*7*(1-blend)

def grid(name,extent,n,hfn):
    axis=np.concatenate([np.arange(-30000,-10000,150),np.arange(-10000,10000,50),np.arange(10000,30001,150)]) if name=='SM_LunarTerrain' else np.linspace(-extent,extent,n)
    n=len(axis)
    x,y=np.meshgrid(axis,axis)
    z=hfn(x,y)
    if name=='SM_LunarTerrain':
        (ART/'heightfield.json').write_text(json.dumps({'axis':axis.tolist(),'z':np.round(z,2).tolist()},separators=(',',':')))
    # Legacy Unreal OBJ importer flips Y but preserves Z. Author directly in Z-up.
    with (ART/(name+'.obj')).open('w') as f:
        f.write('o '+name+'\n')
        for a,b,c in zip(x.flat,y.flat,z.flat): f.write(f'v {a:.3f} {-b:.3f} {c:.3f}\n')
        for j in range(n):
            for i in range(n): f.write(f'vt {i/(n-1):.6f} {j/(n-1):.6f}\n')
        f.write('s 1\n')
        for j in range(n-1):
            for i in range(n-1):
                a=j*n+i+1;b=a+1;c=a+n;d=c+1
                f.write(f'f {a}/{a} {d}/{d} {b}/{b}\nf {a}/{a} {c}/{c} {d}/{d}\n')
    print(name, 'triangles=',2*(n-1)**2,flush=True)

grid('SM_LunarTerrain',30000,401,heights)
def far(x,y):
    r=np.hypot(x,y)
    t=np.arctan2(y,x)
    ridges=(np.sin(t*7+1.5)*.24+np.sin(t*13)*.13+.67)*3200
    return -250+np.exp(-((r-59000)/17000)**2)*ridges+noise(x*.17,y*.17)*1000*np.clip((r-26000)/12000,0,1)
grid('SM_LunarHorizon',110000,201,far)

for seed in range(6):
    nlat,nlon=7,11
    vs=[];uvs=[];fs=[]
    for j in range(nlat+1):
        lat=.001+(math.pi-.002)*j/nlat
        for i in range(nlon+1):
            lon=math.tau*i/nlon
            irregular=1+.15*math.sin(lon*3+lat*5+seed)+.09*math.cos(lon*7-lat*4+seed*.7)
            x=math.sin(lat)*math.cos(lon)*irregular*50
            y=math.sin(lat)*math.sin(lon)*irregular*40
            z=math.cos(lat)*irregular*34
            # Fractured, partially buried blocks with planar clipped faces.
            x=max(-38-seed,min(40+seed,x))
            y=max(-30,min(32,y))
            z=max(-22,min(24+seed,z))
            vs.append((x,-y,z));uvs.append((i/nlon,j/nlat))
    for j in range(nlat):
        for i in range(nlon):
            a=j*(nlon+1)+i+1;b=a+1;c=a+nlon+1;d=c+1
            fs.extend([(a,c,b),(b,c,d)])
    with (ART/f'SM_Basalt_{seed}.obj').open('w') as f:
        for v in vs:f.write('v %.5f %.5f %.5f\n'%v)
        for uv in uvs:f.write('vt %.6f %.6f\n'%uv)
        f.write('s off\n')
        for face in fs:f.write('f '+' '.join(f'{k}/{k}' for k in reversed(face))+'\n')
(ART/'geometry.json').write_text(json.dumps({'terrain_extent_cm':30000,'craters':craters,'sources':sources},indent=2))
print('LUNAR_GEOMETRY_READY',flush=True)

if (ART/'stars.6.json').exists():
    stars=json.loads((ART/'stars.6.json').read_text())['features']
    for group in ['Blue','White','Warm']:
        verts=[];faces=[]
        for star in stars:
            p=star['properties'];mag=float(p['mag']);ci=float(p.get('bv',.5) or .5)
            g='Blue' if ci<.2 else 'Warm' if ci>.9 else 'White'
            if group!=g:continue
            ra,dec=map(math.radians,star['geometry']['coordinates'])
            direction=np.array([math.cos(dec)*math.cos(ra),math.cos(dec)*math.sin(ra),math.sin(dec)])
            right=np.cross(direction,np.array([0,0,1.]));right/=np.linalg.norm(right)
            up=np.cross(right,direction)
            radius=3.2*(350+1200*10**(-.13*(mag+1.5)))
            center=direction*1800000
            idx=len(verts)+1
            for ax,ay in [(-1,-1),(1,-1),(1,1),(-1,1)]:
                v=center+(right*ax+up*ay)*radius
                verts.append((v[0],-v[1],v[2]))
            faces.extend([(idx,idx+1,idx+2),(idx,idx+2,idx+3)])
        with (ART/f'SM_Stars_{group}.obj').open('w') as f:
            for v in verts:f.write('v %.4f %.4f %.4f\n'%v)
            for i in range(len(verts)//4):
                for uv in [(0,0),(1,0),(1,1),(0,1)]:f.write('vt %s %s\n'%uv)
            for face in faces:f.write('f '+' '.join(f'{k}/{k}' for k in face)+'\n')
        print(group,'stars',len(verts)//4,flush=True)
