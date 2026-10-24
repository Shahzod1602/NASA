"""Reproducible NASA/LROC Apollo 12 terrain and mission-point preparation.
Run with Tools/pydeps on PYTHONPATH (rasterio, pyshp, numpy, Pillow).
No invented elevation detail is added to the measured/derived terrain.
"""
from pathlib import Path
import hashlib, json, math, sys, zipfile
sys.path.insert(0, str(Path(__file__).parent / 'pydeps'))
import numpy as np
import rasterio
import shapefile
import requests
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'SourceArt/Apollo12'
RAW = ART / 'Raw'
RAW.mkdir(parents=True, exist_ok=True)
BASE = 'https://pds.mcp.nasa.gov/data/store/img/lunar_reconnaissance_orbiter/pds4/lroc/lro-l-lroc-5-rdr/LROLRC_2001/'
files = ['DATA/SDP/NAC_DTM/APOLLO12/' + s for s in (
    'NAC_DTM_APOLLO12.TIF', 'NAC_DTM_APOLLO12.LBL', 'NAC_DTM_APOLLO12_README.TXT')]
files += ['EXTRAS/BROWSE/NAC_DTM/APOLLO12/' + s for s in (
    'NAC_DTM_APOLLO12_CONF.TIF', 'NAC_DTM_APOLLO12_M120005333_2M.TIF')]
files += ['EXTRAS/SHAPEFILE/APOLLO_12/APOLLO_12.ZIP']
downloads = []
for rel in files:
    path = RAW / rel.rsplit('/', 1)[1]
    if not path.exists():
        response = requests.get(BASE + rel, timeout=120)
        response.raise_for_status()
        path.write_bytes(response.content)
    downloads.append(dict(file=path.name, url=BASE+rel, bytes=path.stat().st_size,
                          sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
with zipfile.ZipFile(RAW / 'APOLLO_12.ZIP') as archive:
    for name in archive.namelist():
        if Path(name).name != name:
            raise ValueError('Unexpected nested archive member')
    archive.extractall(RAW / 'mapping')

def records(name):
    return list(shapefile.Reader(str(RAW / 'mapping' / (name + '.SHP'))).iterShapeRecords())

stations = {r.record['Abbrev']: r for r in records('APOLLO_12_STATIONS')}
equipment = {r.record['Equipment']: r for r in records('APOLLO_12_EQUIPMENT')}
origin = stations['LM'].shape.points[0]
with rasterio.open(RAW / 'NAC_DTM_APOLLO12.TIF') as dataset:
    window = rasterio.windows.from_bounds(origin[0]-600, origin[1]-450,
        origin[0]+450, origin[1]+400, dataset.transform).round_offsets().round_lengths()
    dem = dataset.read(1, window=window, masked=True)
    assert np.ma.count_masked(dem) == 0, 'Playable crop contains missing elevations'
    dem = np.asarray(dem)
    transform = dataset.window_transform(window)
    crs = str(dataset.crs)
    source_transform = list(dataset.transform)
rows, cols = dem.shape
x0, north0 = transform * (.5, .5)
def height(east, north):
    """Interpolate the same two triangles per cell that the OBJ exports."""
    fx, fy = (east-x0)/2, (north0-north)/2
    i, j = int(math.floor(fx)), int(math.floor(fy))
    if not (0 <= i < cols-1 and 0 <= j < rows-1):
        raise ValueError('Point outside playable terrain')
    u, v = fx-i, fy-j
    a,b,c,d = [float(t) for t in (dem[j,i],dem[j,i+1],dem[j+1,i],dem[j+1,i+1])]
    return a*(1-u)+b*(u-v)+d*v if u >= v else a*(1-v)+c*(v-u)+d*u
z_origin = height(*origin)
def local(point):
    e,n = point[:2]
    return [round((e-origin[0])*100, 3), round((origin[1]-n)*100, 3), round((height(e,n)-z_origin)*100, 3)]
def item(rec, name):
    return dict(name=name, xyz=local(rec.shape.points[0]), source=rec.record.as_dict())

sites = {'home':item(stations['LM'],'Intrepid'), 'surveyor':item(stations['SRV'],'Surveyor 3')}
sites['check'] = dict(name='Intrepid equipment table',xyz=local((origin[0]-5,origin[1]+1)),source={'placement':'authored interaction point 5 m west of LM'})
sites['instruments'] = [item(equipment['PSE'],'Passive seismometer'),item(equipment['Central Station'],'ALSEP central station')]
sample_records = records('APOLLO_12-2_SAMPLES')
sites['samples'] = [item(next(r for r in sample_records if r.record['ID']==sid),name)
    for sid,name in [('12031','Head Crater'),('12032','Bench Crater'),('12027','Sharp Crater core')]]
pts = records('APOLLO_12-2_CDR_PTS')
# Real mapped Conrad positions, not an invented straight line through Surveyor crater.
route_ids = [695,700,705,725,775,795,885,930,940,950,960,965]
sites['approach'] = [item(next(r for r in pts if r.record['ID']==sid),f'Route point {i+1}') for i,sid in enumerate(route_ids)]
# Reference route segments retain source ordering, including discontinuities.
segments=[]
for record in records('APOLLO_12-2_CDR_PATHS'):
    points=record.shape.points
    for a,b in zip(points,points[1:]):segments.append([local(a),local(b)])

path=ART/'SM_Apollo12Terrain.obj'
with path.open('w',newline='\n') as out:
    out.write('o SM_Apollo12Terrain\n')
    for j in range(rows):
        for i in range(cols):
            x=(x0+i*2-origin[0])*100; y=(origin[1]-north0+j*2)*100; z=(float(dem[j,i])-z_origin)*100
            # UE's legacy OBJ importer mirrors Y. Keep vertical and horizontal units in cm.
            out.write(f'v {x:.3f} {-y:.3f} {z:.3f}\n')
    for j in range(rows):
        for i in range(cols):out.write(f'vt {i/(cols-1):.7f} {1-j/(rows-1):.7f}\n')
    out.write('s 1\n')
    for j in range(rows-1):
        for i in range(cols-1):
            a=j*cols+i+1;b=a+1;c=a+cols;d=c+1
            out.write(f'f {a}/{a} {d}/{d} {b}/{b}\nf {a}/{a} {c}/{c} {d}/{d}\n')

with rasterio.open(RAW/'NAC_DTM_APOLLO12_M120005333_2M.TIF') as photo:
    assert photo.transform.almost_equals(transform * rasterio.Affine.translation(-window.col_off,-window.row_off))
    im=photo.read(1,window=window)
    Image.fromarray(im).save(ART/'T_Apollo12_Orbital.png')
with rasterio.open(RAW/'NAC_DTM_APOLLO12_CONF.TIF') as conf:
    values,counts=np.unique(conf.read(1,window=window),return_counts=True)
    confidence={int(k):int(v) for k,v in zip(values,counts)}
manifest=dict(title='Apollo 12 / Surveyor 3', sources=downloads,
    credit='NASA/GSFC/Arizona State University, LROC science team; NASA Apollo Lunar Surface Journal',
    crs=crs, source_transform=source_transform, crop_window=list(window.flatten()),
    origin_easting_northing_m=list(origin), origin_elevation_m=z_origin,
    axes='UE X east, Y south, Z up; 100 Unreal units = 1 metre; no horizontal or vertical exaggeration',
    source_spacing_m=2, crop_vertices=[cols,rows], triangles=(cols-1)*(rows-1)*2,
    local_bounds_cm=[(x0-origin[0])*100,(origin[1]-north0)*100,(x0+(cols-1)*2-origin[0])*100,(origin[1]-north0+(rows-1)*2)*100],
    confidence_histogram=confidence, sites=sites, reference_segments=segments,
    limitations=['DTM is stereo-derived with reported 0.8 m precision and 1.31 m LOLA RMS; 2 m spacing is not absolute accuracy.',
    'Source orbital images were acquired in 2010, not during Apollo 12.',
    'Surveyor station point is a mapped work location, not a millimetre-accurate spacecraft survey.',
    'Two historical EVAs are condensed into one educational route; action durations, single-player view and suit budget are gameplay adaptations.',
    'Small rocks, equipment geometry, lighting and radio paraphrases are authored; the orbital image is used as a journal map, not as surface albedo.'])
(ART/'manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
(ART/'heightfield.json').write_text(json.dumps(dict(x0=(x0-origin[0])*100,y0=(origin[1]-north0)*100,step=200,z=((dem-z_origin)*100).round(3).tolist())),encoding='utf-8')
def vec(p):return 'FVector('+','.join(f'{n:.3f}' for n in p)+')'
header=['// Generated by Tools/prepare_apollo12.py from NASA/LROC sources. Do not edit.','#pragma once','namespace ApolloData {']
for key in ['home','check','surveyor']:header.append(f'inline const FVector {key.title()} = {vec(sites[key]["xyz"])};')
for key in ['instruments','samples','approach']:
    header.append(f'inline const FVector {key.title()}[] = {{'+','.join(vec(p['xyz']) for p in sites[key])+'};')
header.append('inline const FVector ReferenceRoute[][2] = {'+','.join('{'+vec(a)+','+vec(b)+'}' for a,b in segments)+'};')
header.append('}')
(ROOT/'Source/LunarRescue/ApolloData.h').write_text('\n'.join(header)+'\n')
print(json.dumps({k:manifest[k] for k in ['crop_vertices','triangles','origin_elevation_m','confidence_histogram']}))
print('Apollo 12 source data, terrain and mission points prepared.')
