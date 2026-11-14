# Supplied asset review

Reviewed 13 November 2026. This is a structural/provenance review of the supplied
downloads, not an Unreal import, rendered comparison or gameplay acceptance.
Original downloads and the existing game assets were not modified.

| Supplied file | Observed contents | Integration decision |
|---|---|---|
| astronaut_by_jungle_jim.glb | 67,998 triangles in 23 material-split meshes; 14 materials; no skin, animation, UV coordinates or texture images | Credited static character candidate. Needs skeleton/weights, finger articulation and surface-detail work for a playable suit. Mesh divisions are not anatomical rig joints. |
| moon_rock_01_4k.blend (1).zip | CRC-valid ZIP with Blender model, diffuse JPG, OpenGL normal EXR, roughness EXR and displacement PNG | First processing candidate. Inspect Blender geometry/LODs; derive 2K runtime maps and correct the normal convention on import. Geometry counts and material appearance have not been verified in Blender. |
| Space Exploration Vehicle.glb | 413,198 triangles, one mesh/node/material, embedded images; required Draco compression; no skin or animation | Static vehicle source candidate. Decode in a compatible importer, optimize/author LODs, separate wheels and set pivots, collision and scale before driving integration. It does not supply independent animated wheels. |

## Astronaut source and credit

User-supplied original source:
https://sketchfab.com/3d-models/astronaut-by-jungle-jim-c775a977606a4e4e990afe783c6bc845

Author: Jungle Jim. Public Sketchfab API and embedded GLB credit both identify
CC BY 4.0; API reports downloadable=true. Retain title, author, source,
https://creativecommons.org/licenses/by/4.0/ and a description of modifications.
No exact Apollo A7L historical identity has been established.

The API reports 68,017 triangles while this supplied GLB contains 67,998.
Published provenance was matched by model UID/embedded credit; byte identity
with a fresh hosted download was not tested. An initial triangle-equality gate
was too strict and rejected the review; this discrepancy remains documented.

## Additional animated astronaut candidate

`animated_floating_astronaut_in_space_suit_loop.glb`, inspected 13 November 2026:

- 30,543,432 bytes; SHA256
  `6008551be3e7f9668b73d2cfb12c1c7499db6e432303210f753791873f5c861d`.
- 58,312 triangles, four skinned mesh nodes, three materials, eight embedded
  images and UV coordinates on all four primitives.
- One skeleton with 91 joints, including three joints per finger on both hands.
  One approximately 6.97-second clip; source describes a floating loop.
- Verified 122,158 weighted vertex records: joint indices in range, nonnegative
  finite weights, maximum weight-sum error about 1.05e-7. Position records are
  finite, triangle indices in range and animation input times strictly increasing.
- Author: LasquetiSpice. Embedded source and live Sketchfab public API identify
  CC BY 4.0 and downloadable=true. Source triangle and animation counts match.
  This does not establish byte identity to a new download.
- Source: https://sketchfab.com/3d-models/animated-floating-astronaut-in-space-suit-loop-e2c4b146e58141e4b87917456a9970b1
- Required legacy `KHR_materials_pbrSpecularGlossiness` material extension needs
  compatible import/conversion. Existing LunarRescue bone names and procedural
  suit driver need an explicit mapping. Walk/landing/tool-grip clips are not
  supplied as separate actions; compatible poses/motion must be authored.
- Structurally stronger playable-character candidate than the earlier static
  model. No exact Apollo A7L identity, successful Unreal import, visual quality,
  deformation playback or performance has yet been established. Assess actual
  silhouette/materials before accepting it for historical Apollo presentation.

Local detailed receipt: Reports/SuppliedAssets20261113/animated-astronaut-inspection.json.
Original file unchanged; no runtime or packaged asset was replaced.

## Other sources and workflow

Rock source/CC0: https://polyhaven.com/a/moon_rock_01

Vehicle source candidate: https://science.nasa.gov/3d-resources/space-exploration-vehicle/
The supplied filename and structure alone do not establish byte identity with
NASA's live download. Preserve NASA item credit and applicable usage terms
when preparing the derivative.

Suggested order: prepare the actual rock mesh/PBR material first; prepare the
rover as an optimized static preview before modifying the current vehicle;
assess the astronaut silhouette before investing in a custom rig. Do not replace
the existing animated hands with an unrigged static asset.

Local machine-readable inspection and original-file SHA256 values:
Reports/SuppliedAssets20261113/inspection.json (ignored local evidence).
