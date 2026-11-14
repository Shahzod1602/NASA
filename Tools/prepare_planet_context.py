"""Cache the official NASA global colour illustrations for offline UI context."""
from pathlib import Path
from datetime import datetime,timezone
import hashlib,json,urllib.request

root=Path(__file__).resolve().parents[1];out=root/'SourceArt/PlanetContext20261114';(out/'Raw').mkdir(parents=True,exist_ok=True)
sources=[
 ('Moon','lroc_color_2k.jpg','https://svs.gsfc.nasa.gov/4720/','https://svs.gsfc.nasa.gov/vis/a000000/a004700/a004720/lroc_color_2k.jpg','NASA SVS / Ernie Wright; LRO LROC and LOLA teams','SVS 4720 / December 2025 colour version'),
 ('Mars','Mars.jpg','https://science.nasa.gov/3d-resources/mars/','https://assets.science.nasa.gov/content/dam/science/cds/3d/resources/image/mars/Mars.jpg','NASA/JPL-Caltech; Viking imagery processed at USGS','Mars Image Texture / NASA 3D Resources'),
]
manifest=[]
for planet,name,page,url,credit,product in sources:
 path=out/'Raw'/name
 if not path.exists():
  with urllib.request.urlopen(url,timeout=45) as response:payload=response.read(5*1024*1024+1)
  if len(payload)>5*1024*1024:raise ValueError('Unexpected global colour image size')
  path.write_bytes(payload)
 payload=path.read_bytes();assert payload[:2]==b'\xff\xd8'
 manifest.append(dict(planet=planet,name=product,source_page=page,source_url=url,downloaded_at_utc=datetime.now(timezone.utc).isoformat(),credit=credit,bytes=len(payload),sha256=hashlib.sha256(payload).hexdigest(),role='Interactive globe colour context only; no terrain displacement, mission geolocation, grade or resource calculation',limits='NASA colour illustrations; not quantitative elevation products. Moon poles include filled albedo and image processing. Mars texture page does not supply a numeric CRS. No survey precision is inferred.'))
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8');print(json.dumps(manifest,indent=2))
