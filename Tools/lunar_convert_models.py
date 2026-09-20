import json
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, r'C:\Users\shaxz\lunar_python_libs')
import trimesh
from trimesh.exchange.obj import export_obj

downloads = Path(r'D:\Users\shaxz\Downloads')
output = Path(r'C:\Users\shaxz\lunar_staging')
output.mkdir(exist_ok=True)

models = {
    'ApolloLander': ('Apollo Lunar Module.glb', 650),
    'HabitatMain': ('Habitat Demonstration Unit (part 2).glb', 1250),
    'HabitatUtility': ('Habitat Demonstration Unit (part 1).glb', 700),
    'Lunokhod': ('lunar_rover__lunokhod_low_poly.glb', 340),
    'PowerCell': ('sci-fi_battery.glb', 65),
    'ScienceTerminal': ('sci-fi_terminal.glb', 230),
    'SolarPanel': ('solar_panel.glb', 460),
}

report = {}
for key, (filename, target_extent) in models.items():
    scene = trimesh.load(downloads / filename, force='scene')
    if not len(scene.geometry):
        raise RuntimeError(f'{filename}: no geometry')
    if key == 'ScienceTerminal':
        target_measure = scene.extents[1]
    else:
        target_measure = max(scene.extents[0], scene.extents[2])
    scale = target_extent / target_measure
    # glTF is Y-up; Unreal is Z-up. Keep the source's front/back orientation.
    basis = np.array([
        [1, 0, 0, 0],
        [0, 0, -1, 0],
        [0, 1, 0, 0],
        [0, 0, 0, 1],
    ], dtype=float)
    basis[:3, :3] *= scale
    scene.apply_transform(basis)
    mins, maxs = scene.bounds
    move = np.eye(4)
    move[:3, 3] = (-(mins[0] + maxs[0]) / 2, -(mins[1] + maxs[1]) / 2, -mins[2])
    scene.apply_transform(move)
    data, textures = export_obj(scene, include_texture=True, return_texture=True)
    folder = output / key
    folder.mkdir(exist_ok=True)
    (folder / (key + '.obj')).write_text(data, encoding='utf-8')
    for name, payload in textures.items():
        target = folder / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(payload)
    report[key] = {
        'source': filename,
        'triangles': sum(len(g.faces) for g in scene.geometry.values()),
        'bounds_cm': scene.bounds.tolist(),
        'textures': list(textures),
    }
    print(key, report[key]['triangles'], scene.extents.round(1).tolist(), len(textures), flush=True)
(output / 'manifest.json').write_text(json.dumps(report, indent=2))
