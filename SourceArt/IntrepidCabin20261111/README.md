# Intrepid cabin normal recovery

This package preserves the locally supplied Apollo Lunar Module source and
recovers the original model author's cabin shading normals. It is presentation
work on a reference model, not new lunar terrain data or a claim to reconstruct
every detail of Apollo 12's LM-6 Intrepid.

## Files and provenance

- `Raw/Apollo Lunar Module.glb`: unchanged original NASA/Michael D. Carbajal model.
- `Raw/ApolloLander.obj`: unchanged project conversion previously used to import
  `/Game/Lunar/Imported/ApolloLander/ApolloLander`.
- `cabin-normals.json`: 1,237 cabin triangle records and 3,711 authored corner
  normals, with source positions, material identities and source indices.
- `source-normal-recovery-report.json`: input/output hashes, exact conversion
  reproduction proof, coverage and explicit processing limits.
- `mapping-fixture-checks.json`: nine synthetic mapping checks. These are not
  Unreal build, visual, traversal, performance or human playtest results.

The raw inputs were copied from the existing local project sources on
2026-11-11. They were not newly downloaded for this stage. The earlier source
import is credited in [External model credits](../ExternalModels/CREDITS.md).
See [CREDITS.md](CREDITS.md) for source links and usage boundaries.

A separate 2026-11-11 HTTP identity check of NASA's
[official GLB download](https://assets.science.nasa.gov/content/dam/science/cds/3d/resources/model/apollo-lunar-module/Apollo%20Lunar%20Module.glb)
confirmed the same 716,840 bytes and SHA-256 as the retained raw GLB. That check
establishes file identity, not exact historical geometry or NASA endorsement.

| Input | Bytes | SHA-256 |
| --- | ---: | --- |
| `Raw/Apollo Lunar Module.glb` | 716,840 | `379101dfcee399267addf771709107c35e826b0b9e5233272f123e4c8a585c4e` |
| `Raw/ApolloLander.obj` | 8,031,978 | `7b034359fbfa5c42cd576e129e36e29a9272c178b5b99d95de0ed260c843b8d8` |

## What caused the visible faceting

The decoded GLB retains explicit normal attributes. The original conversion
exported 64,787 vertex positions, 64,087 UV records and 97,588 triangles, but no
`vn` or smoothing records. The `trimesh` scene export copies individual meshes
without their cached authored normals. Unreal therefore had to create its own
shading normals. Some visible faceting also comes from the original coarse
geometry; recovering normals cannot improve the silhouette or add geometry.

The broad ascent cabin is represented by these exact object/material pairs:

| OBJ object | Source material | Unreal material slot | Triangles |
| --- | --- | --- | ---: |
| `polySurfac.002` | `initialShadingGr.001` | `initialShadingGr_001` | 1,029 |
| `polySurfac.002_1` | `blinn3SG.001` | `blinn3SG_001` | 208 |

The bright white `blinn7SG.002` slot mainly covers smaller RCS/antenna parts;
changing that slot alone would miss the broad cabin. Conversely, the two named
cabin material slots also serve some other objects in the source model. The
normal mapping selects the exact object/material pairs above, not every triangle
which happens to share their material slot.

## Reproduce the offline mapping

From the repository root, with Python, NumPy, and `trimesh` including Draco GLB
decoding available:

```powershell
python Tools/prepare_intrepid_cabin.py `
  --obj SourceArt/IntrepidCabin20261111/Raw/ApolloLander.obj `
  --glb 'SourceArt/IntrepidCabin20261111/Raw/Apollo Lunar Module.glb' `
  --outputdir SourceArt/IntrepidCabin20261111
```

If these dependencies are supplied in a separate local directory, append
`--dependency-dir '<absolute dependency directory>'`. This run used NumPy 2.4.6
and `trimesh` 5.1.0. No dependency is installed or downloaded by the processor.

The synthetic fixture checks can also run independently of these packages:

```powershell
python Tools/prepare_intrepid_cabin.py --self-test `
  --outputdir Reports/IntrepidCabin20261111/mapping-fixtures
```

The processor reproduces the exact earlier 650 cm width conversion, glTF Y-up
to OBJ Z-up basis, scene transforms and datum translation. All original vertex,
UV, face, object and material records must match exactly before mapping by source
vertex index. It reads explicit authored normals before a mesh copy can discard
them and transforms normals with each node's inverse transpose. It does not
calculate arbitrary smoothed replacements, weld coincident hard-edge vertices,
guess a nearest face, or round the source metal panel geometry.

Positions in the JSON are original converted OBJ centimeters, before the game's
698 cm height normalization. The documented OBJ-to-Unreal Y reflection applies
to both positions and normals. The Editor builder must verify its actual mesh
matching rather than assume a source corner order. At the documented 0.0001 cm
position quantization, this cabin selection has no duplicate oriented or
unordered triangles. All 3,711 normals are finite and unit length.

## Editor integration and limits

The accepted Editor mesh build passed 46 assertions and saved a separate
1,597,135-byte `SM_IntrepidCabin` asset. The original editable MeshDescription
lacked usable normal/tangent frames, while its computed render buffers contained
the displayed source frames. The builder therefore bakes those computed frames
into all 289,380 vertex instances of the duplicate, then applies the 3,711 cabin
normal targets. This intentionally changes raw non-cabin MeshDescription frame
attributes; it is not a cabin-only raw-attribute patch. Positions, topology, UVs,
materials, bounds and all other attributes remain exact, and the original asset,
its computed render data, body setup and package bytes remain unchanged.

Cabin tangents project 3,699 source tangents onto the authored normals; 12
untextured/degenerate cases use a deterministic rendering basis. Outside the
cabin, 836 demonstrably zero/parallel source tangent instances on 570 faces are
reconstructed while preserving normals and binormal signs. Affected face counts
are `blinn4SG_002: 257`, `blinn6SG_001: 113`, `blinn2SG_002: 4`,
`blinn3SG_001: 36`, and `blinn1SG_002: 160`. These are technical rendering repairs,
not new measured data or repaired UVs. Valid outside tangents and all outside
normals stay within one actual 8-bit component step plus float epsilon; packed
W/sign values remain exact. Non-cabin rendered frames are not bit-identical.
See [the verification record](../../Docs/IntrepidCabin20261111/VERIFICATION.md)
for the accepted builder receipt and the separately pending runtime work.

Do not import this old OBJ again as a replacement asset. Its inherited UV table
contains 3,744 face corners whose UV index exceeds the 64,087-entry table, and
the complete source also contains degenerate triangles. The selected cabin
triangles are nondegenerate. This stage reports those inherited defects and
preserves the existing Unreal geometry/UV attributes rather than propagating the
OBJ UV issue through a new import. The tangent repairs above affect only the new
duplicate's technical rendering frames.

The offline recovery, nine fixture checks and 46 mesh-builder assertions passed.
The dedicated material commandlet subsequently succeeded separately; that result
does not establish visual or packaged acceptance. Packaged rendering, visual
comparison, regression and performance verification remain separate steps;
consult the final parent validation report for their results. The model's source
normal choices are retained without claiming perfect source normals or exact
historical shape.
No map, NASA heightfield, mission work site, route, interaction or Mars content
is written by this processor. Runtime use does not require an internet connection.
