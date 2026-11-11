# Intrepid ascent cabin presentation

Date: 2026-11-11. **Accepted local implementation and verification record.**
The final Game build/cook/stage, 11 local packaged executions and 20 broader
regression reports passed: 1,763 raw C++ PASS / 0 FAIL across 31 report
executions. Four preliminary Editor runs (182 PASS) and the 46-check mesh build
remain separate. Human playtests: zero. No new public release is claimed.
Results from the [earlier surface/foil pass](../IntrepidVisual20261111/IMPLEMENTATION.md)
remain historical evidence and are not counted again for this cabin stage.

## Implemented source scope

`Tools/prepare_intrepid_cabin.py` reproduces the retained GLB-to-OBJ conversion
and recovers explicit authored normals for two broad-cabin object/material pairs:

| OBJ object | Unreal slot | Triangles / corners |
| --- | --- | ---: |
| `polySurfac.002` | `initialShadingGr_001` | 1,029 / 3,087 |
| `polySurfac.002_1` | `blinn3SG_001` | 208 / 624 |

The persistent [source package](../../SourceArt/IntrepidCabin20261111/README.md)
contains 1,237 triangle mappings / 3,711 unit normals, unchanged raw inputs,
source hashes and nine passing synthetic mapping checks. No new vertices,
subdivision, arbitrary smoothing threshold, displacement or model reconstruction
is introduced. Coarse source silhouettes remain coarse.

The accepted `LunarIntrepidCabinEditor.cpp` build saved a separate 1,597,135-byte
mesh after 46 assertions, with no failures or mesh-build diagnostics. It matched
the 1,237 cabin triangles without UV assumptions. The original editable
MeshDescription lacked usable frames, although the original computed render
buffers contained them. The duplicate therefore receives computed source
normal/tangent/sign frames across all 289,380 vertex instances before the 3,711
cabin normal targets are applied. **Raw non-cabin frame attributes intentionally
change; this is not a cabin-only raw MeshDescription patch.** Positions, topology,
UVs, material references, bounds and all other attributes remain exact. Original
source attributes, render data, package bytes, sidecars and body setup stay
unchanged. The old OBJ is not reimported because of its inherited UV-index defect.

Cabin tangents project 3,699 computed source tangents; 12 collapsed cases use a
deterministic axis basis. These are rendering tangents, not GLB-authored tangents
or measured fabric directions. Outside the cabin, 836 demonstrably zero/parallel
tangent instances on 570 faces require reconstruction. Normals and signs remain
retained, and valid source tangents are not repaired. Affected face counts are
`blinn4SG_002: 257`, `blinn6SG_001: 113`, `blinn2SG_002: 4`,
`blinn3SG_001: 36`, and `blinn1SG_002: 160`.

Rendered CPU-buffer comparison preserves exact triangle positions, inherited
Unreal UVs, material assignment and winding. All outside normals and valid
outside tangents stay within one actual 8-bit component step plus float epsilon
(0.007876016); W/sign values remain exact. The explicitly repaired degenerate
tangents may differ more and are checked for finite unit/orthogonal frames within
packing error. Outside frames are not bit-identical. This evidence does not
prove shader, pixel, lighting, collision or performance equivalence.

## Dedicated assets and runtime integration

- Source: `/Game/Lunar/Imported/ApolloLander/ApolloLander`.
- New derivative: `/Game/Lunar/Apollo12/IntrepidVisual/SM_IntrepidCabin`.
- New authored material: `/Game/Lunar/Apollo12/IntrepidVisual/M_IntrepidCabin`.
- Runtime integration: `LunarIntrepidCabin.cpp/.h`, called from `LunarLander.cpp`.
- Asset construction: `Tools/build_intrepid_cabin.py`, using the supported Editor
  builder and material APIs after the Editor module has compiled.

`asset-build-g` saved the validated mesh, then failed in the later Python material
stage because a material pin name was wrong. It is not a wholly successful
commandlet run. The corrected `material-build-h` material-only run succeeded with
zero reported errors/warnings and checked that both source and derived mesh
file hashes stayed unchanged. The accepted byte-preserved [mesh receipt](evidence/mesh-build.json),
[material receipt](evidence/material-build.json) and
[build receipt](evidence/build-receipts.json) are curated separately.

Runtime replacement requires compatible bounds, LOD/material-slot inventory and
disabled visual collision. Missing or incompatible optional assets retain the
source appearance. Existing effective foil/metal and other material overrides
are copied to the derivative. The existing normalization, terrain-foot support
pose, collision and interaction paths are retained. The paired runtime fixtures
verify source/derived bounds, four pad anchors, effective materials and scene
light invariants; broader regressions also pass. Missing/incompatible-asset
fault injection has not been performed. There is no new Tick, terrain generation, world light or map save.

Two dynamic material instances preserve the source GLB's neutral base factors:
`initialShadingGr_001 = 0.498039216` and `blinn3SG_001 = 0.678431392` in RGB.
**These slots also serve retained rim/trim and small other model parts.** The
material finish affects both complete shared slots; only normal recovery is
restricted to the two cabin objects. Other material slots keep their effective
overrides.

The new material uses roughness 0.78, specular 0.18 and metallic 0. Its authored
panel stitching changes color by at most 2.5%, with small fold/weave color
variation. The nominal 80 cm spacing is in source-local model coordinates
(approximately 110 cm after the current height normalization), not surveyed
panel dimensions. Normal output uses the recovered vertex normal; there is no
world-position offset or added geometry.

## Comparison controls

`-IntrepidCabinBaseline` disables only this cabin derivative/material finish.
`-IntrepidVisualBaseline` independently controls the earlier surface/foil pass.
Compare cabin control and feature in the same binary while keeping the earlier
pass enabled in both. `-IntrepidCabinScope` routes isolated evidence into
`Reports/IntrepidCabin20261111`; `-IntrepidCabinReview` selects two close cameras.
See [VERIFICATION.md](VERIFICATION.md) and [verification index](verification-index.json)
for exact execution evidence, reproduction templates and explicit limits.

## Accepted appearance and remaining limits

Representative packaged front, side, wide and ladder captures were directly
inspected. Recovered shading softens some artificial dark panel seams while the
original faceted outline remains. The shared white finish and cloth pattern are
subtle; the sun-facing cover still reads bright, flat and low contrast. A small
Editor image ROI had no all-channel values at or above 254, so that sample does
not support calling it literal pixel clipping; it does not establish global HDR
or exposure correctness. This is bounded presentation polish, not a photoreal
replacement model.

Close reviews use two staged FOV-60 cameras; wide reviews and profiles retain
four FOV-85 views. HUD, player and text-rendered world labels are hidden and the
mission is frozen. Still images do not prove temporal smoothness or ordinary
signage/UI readability. No separate binary saved-asset reload attribute audit,
clean extracted/disconnected distribution test or human playtest was performed.
Fresh processes loading the saved asset provide runtime/visual evidence, not
that binary attribute audit.
