"""Validate authored glove topology, finite attributes and digit bind lengths."""
import json
import math
import struct
from pathlib import Path

root = Path(__file__).resolve().parents[1]
results = []
for name in ('SK_A7L', 'SK_A7L_Hands'):
    raw = (root / 'SourceArt/Suit/Rig' / (name + '.glb')).read_bytes()
    size = struct.unpack_from('<I', raw, 12)[0]
    doc = json.loads(raw[20:20 + size])
    binary = raw[28 + size:]

    def values(index, fmt):
        access = doc['accessors'][index]
        view = doc['bufferViews'][access['bufferView']]
        return list(struct.iter_unpack('<' + fmt, binary[view['byteOffset']:view['byteOffset'] + view['byteLength']]))

    attrs = doc['meshes'][0]['primitives'][0]['attributes']
    positions = values(attrs['POSITION'], '3f')
    normals = values(attrs['NORMAL'], '3f')
    skin_weights = values(attrs['WEIGHTS_0'], '4f')
    assert all(all(math.isfinite(w) and 0 <= w <= 1 for w in row) and abs(sum(row)-1) < 1e-5 for row in skin_weights), 'Invalid garment skin weights'
    assert all(math.isfinite(v) for p in positions + normals for v in p)
    assert all(.98 < sum(v*v for v in n) < 1.02 for n in normals), 'Invalid smooth normal'
    triangles = 0
    for primitive in doc['meshes'][0]['primitives']:
        ids = [v[0] for v in values(primitive['indices'], 'I')]
        assert len(ids) % 3 == 0 and max(ids) < len(positions)
        for i in range(0, len(ids), 3):
            a, b, c = [positions[k] for k in ids[i:i+3]]
            u, v = [b[k]-a[k] for k in range(3)], [c[k]-a[k] for k in range(3)]
            area = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
            assert sum(x*x for x in area) > 1e-20, 'Degenerate triangle'
        triangles += len(ids) // 3
    nodes = {n['name']: n for n in doc['nodes']}
    assert len(doc['skins'][0]['joints']) == 36, 'Suit skeleton contract changed'
    for side in ('l', 'r'):
        lengths = [math.sqrt(sum(v*v for v in nodes[f'tip{i}_{side}']['translation'])) * 100 for i in range(5)]
        assert abs(lengths[1] - 4) < .001
        assert 2.7 < lengths[4] < lengths[3] < lengths[2] < lengths[0] < lengths[1]
    # Keep this iteration within the existing character-scale budget.
    assert len(positions) < (125000 if name == 'SK_A7L' else 24000)
    results.append({'mesh': name, 'vertices': len(positions), 'triangles': triangles, 'finite_unit_normals': True, 'nondegenerate': True, 'digit_lengths_verified': True, 'normalized_skin_weights': True, 'skeleton_bones': 36})
# Check the silhouette on the actual source vertices, before Editor import.
def source_vertices(name):
    return [tuple(map(float,line.split()[1:])) for line in (root/'SourceArt/Suit'/('SM_Suit'+name+'.obj')).read_text().splitlines() if line.startswith('v ')]
forearm=source_vertices('Forearm')
elbow=max(abs(p[1]) for p in forearm if 1 < p[0] < 4)
wrist=max(abs(p[1]) for p in forearm if 23.5 < p[0] < 24.1)
assert .6 < wrist/elbow < .76, 'Sleeve taper lost'
assert min(p[0] for p in forearm) >= -.46 and max(p[0] for p in forearm) <= 24.21
thigh=source_vertices('Thigh')
assert min(p[0] for p in thigh) >= -.46 and max(p[0] for p in thigh) <= 36.46, 'Rounded end caps returned'
palm=source_vertices('Palm')
assert all(math.isfinite(v) for p in forearm+thigh+palm for v in p)
results.append({'source_profiles': ['Forearm','Thigh','Palm'], 'sleeve_wrist_to_elbow_width_ratio': wrist/elbow, 'thigh_longitudinal_cm': [min(p[0] for p in thigh),max(p[0] for p in thigh)], 'shallow_closures_verified': True})
out = root / 'Reports/Suit20261029'
out.mkdir(exist_ok=True)
(out / 'geometry-validation.json').write_text(json.dumps(results, indent=2))
print('PASS: both suit meshes, topology, normals, skin weights, 36-bone contract, digit lengths, vertex budgets and source silhouettes')
