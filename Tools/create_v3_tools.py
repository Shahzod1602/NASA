# Historical one-off bootstrap. The generated V3 scripts were refined afterward;
# use prepare_lunar_v3.py/import_lunar_v3.py/upgrade_lunar_v3.py to rebuild assets.
raise SystemExit('Bootstrap archived: use the three V3 asset scripts directly.')
from pathlib import Path
p=Path(__file__).parent
s=(p/'prepare_lunar_geometry.py').read_text()
s=s.replace('LunarV2','LunarV3')
s=s.replace("for name,url in ([] if '--geometry-only' in sys.argv else sources.items()):", "for name,url in []:")
s=s.replace('def heights(x,y):', '''# Small overlapping impacts break up the smooth original basin.
for i in range(210):
    cx,cy=rng.uniform(-9500,9500,2)
    if -3400<cx<5300 and -4400<cy<800: continue
    r=float(rng.uniform(110,480))
    craters.append((float(cx),float(cy),r,r*.24))

def heights(x,y):''')
s=s.replace('d=np.hypot(x-cx,y-cy)/r', '''t=np.arctan2(y-cy,x-cx)
        d=np.hypot(x-cx,y-cy)/(r*(1+.045*np.sin(t*7+cx)+.025*np.cos(t*13+cy)))''')
s=s.replace('bowl=-depth*np.maximum(0,1-d*d)**1.5', 'bowl=-depth*np.maximum(0,1-d*d)**1.15')
s=s.replace('rim=depth*.37*np.exp(-((d-1.03)/.17)**2)', '''rim=depth*(.34+.07*np.sin(t*9+cx))*np.exp(-((d-1.01)/.115)**2)
        ejecta=depth*.055*np.exp(-((d-1.25)/.44)**2)*(1+.5*np.sin(t*31+cx))
        bowl+=ejecta+noise((x-cx)*2,(y-cy)*2)*depth*.09*np.exp(-((d-.95)/.6)**2)''')
s=s.replace('return z*blend','return z*blend + noise(x*3.1,y*3.1)*7*(1-blend)')
s=s.replace('axis=np.linspace(-extent,extent,n)', '''axis=np.concatenate([np.arange(-30000,-10000,150),np.arange(-10000,10000,50),np.arange(10000,30001,150)]) if name=='SM_LunarTerrain' else np.linspace(-extent,extent,n)
    n=len(axis)''')
s=s.replace('z=hfn(x,y)', '''z=hfn(x,y)
    if name=='SM_LunarTerrain':
        (ART/'heightfield.json').write_text(json.dumps({'axis':axis.tolist(),'z':np.round(z,2).tolist()},separators=(',',':')))''')
s=s.replace('nlat,nlon=14,22','nlat,nlon=7,11')
s=s.replace('vs.append((x,-y,z));uvs.append((i/nlon,j/nlat))', '''# Fractured, partially buried blocks with planar clipped faces.
            x=max(-38-seed,min(40+seed,x))
            y=max(-30,min(32,y))
            z=max(-22,min(24+seed,z))
            vs.append((x,-y,z));uvs.append((i/nlon,j/nlat))''')
s=s.replace("f.write('s 1\\n')\n        for face", "f.write('s off\\n')\n        for face")
(p/'prepare_lunar_v3.py').write_text(s)
s=(p/'import_lunar_v2.py').read_text().replace('LunarV2','LunarV3').replace('/V2/','/V3/').replace('v2-import','v3-import').replace('LUNAR_V2','LUNAR_V3')
(p/'import_lunar_v3.py').write_text(s)
s=(p/'upgrade_lunar_v2.py').read_text().replace('/V2/Materials','/V3/Materials').replace('/V2/Meshes/SM_Lunar','/V3/Meshes/SM_Lunar').replace('/V2/Meshes/SM_Basalt','/V3/Meshes/SM_Basalt').replace('SourceArt/LunarV2','SourceArt/LunarV3')
s=s.replace("'V2_']", "'V2_','V3_']").replace("set_actor_label('V2_'", "set_actor_label('V3_'")
s=s.replace(".55+g.f(P*.055)*.27+g.f(P*.004)*.25+g.f(P*.00037)*.23; return float3(.16,.158,.153)*c;", ".38+g.f(P*.17)*.22+g.f(P*.017)*.32+g.f(P*.0009)*.58; return float3(.115,.113,.109)*c;")
s=s.replace("float h=g.f(P*.085); float dx=g.f((P+float3(1,0,0))*.085)-h; float dy=g.f((P+float3(0,1,0))*.085)-h; return normalize(N+float3(-dx,-dy,0)*2.2);", "float h=g.f(P*.14)+g.f(P*.022)*2.5; float dx=g.f((P+float3(.7,0,0))*.14)+g.f((P+float3(.7,0,0))*.022)*2.5-h; float dy=g.f((P+float3(0,.7,0))*.14)+g.f((P+float3(0,.7,0))*.022)*2.5-h; float3 bump=float3(-dx,-dy,0)*5; return normalize(N+bump-N*dot(N,bump));")
s=s.replace("set_intensity(.10)","set_intensity(.065)")
s=s.replace("if not collision:c.set_cast_shadow(False)","if not collision:c.set_cast_shadow(name in ['Fine surface debris','Basalt fragment'])")
start=s.index('def ground(x,y):')
end=s.index('rng=random.Random',start)
s=s[:start]+'''import bisect
hf=json.loads((ROOT/'SourceArt/LunarV3/heightfield.json').read_text())
axis=hf['axis'];zz=hf['z']
def ground(x,y):
    i=max(0,min(len(axis)-2,bisect.bisect_right(axis,x)-1))
    j=max(0,min(len(axis)-2,bisect.bisect_right(axis,y)-1))
    tx=(x-axis[i])/(axis[i+1]-axis[i]);ty=(y-axis[j])/(axis[j+1]-axis[j])
    return (zz[j][i]*(1-tx)+zz[j][i+1]*tx)*(1-ty)+(zz[j+1][i]*(1-tx)+zz[j+1][i+1]*tx)*ty
''' +s[end:]
s=s.replace('range(550)','range(1000)')
s=s.replace("x,y=rng.uniform(-24000,24000),rng.uniform(-24000,24000)", '''if i<500:
        cx,cy,cr,depth=craters[i%len(craters)]
        angle=rng.uniform(0,math.tau);r=cr*rng.uniform(1.0,1.65)
        x,y=cx+math.cos(angle)*r,cy+math.sin(angle)*r
    else:x,y=rng.uniform(-18000,18000),rng.uniform(-18000,18000)''')
s=s.replace('range(110)','range(850)')
s=s.replace('rng.uniform(-3000,5000),rng.uniform(-2500,700)','rng.uniform(-4200,6500),rng.uniform(-3300,3500)')
s=s.replace("size=rng.uniform(.045,.18)","size=rng.uniform(.045,.33)")
s=s.replace("(x,y,1),(size,size,size*.6)","(x,y,ground(x,y)+size*4),(size,size,size*.6)")
s=s.replace("lib.save_directory('/Game/Lunar/V2'", "lib.save_directory('/Game/Lunar/V3'")
s=s.replace('lunar-v2.json','lunar-v3.json').replace('LUNAR_V2_READY','LUNAR_V3_READY').replace("'terrain_triangles':320000", "'terrain_triangles':889778")
(p/'upgrade_lunar_v3.py').write_text(s)
print('V3 scripts created')
