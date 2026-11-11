# Cabin verification record

Date: 2026-11-11. **Final local evidence:** 1,763 raw packaged C++ PASS /
0 FAIL across 31 report executions, with 32 packaged review PNGs. Preliminary
Editor (182 assertions / four executions / 24 PNGs), asset-builder (46 checks)
and offline source fixtures (nine checks) are separate. No human playtest or
new public release is claimed. Every raw report is counted once; repeating the
same assertions in another execution does not create unique checks.

| Scope | Current evidence | Status |
| --- | --- | --- |
| Exact source conversion reproduction | 226,776 original vertex/UV/object/material/face records match regenerated geometry | PASS |
| Authored normal mapping | 1,237 selected triangles / 3,711 finite unit corner normals; no duplicate triangle ambiguity at 0.0001 cm quantization | PASS |
| Offline fixture cases | Nine checks: hard-edge distinction, target scope, determinism, changed UV/topology rejection, missing/non-finite/zero normal rejection, duplicate-triangle rejection | PASS |
| Persistent source and determinism | Raw bytes unchanged; repeated public JSON byte-identical; relative public source paths | PASS |
| Python/source hygiene | `py_compile`, UTF-8, final newline and trailing-whitespace checks | PASS |
| Official GLB identity | Separate 2026-11-11 HTTP comparison: 716,840 bytes and identical retained/official SHA-256 | PASS |
| Editor compile | `editor-build-g.log` ends `Result: Succeeded` | PASS |
| New mesh construction | `mesh-build.json`: 46 assertions / 0 failures; saved 1,597,135-byte derivative | PASS |
| MeshDescription preservation | Positions, topology, UVs, material references and all attributes except intentionally baked normal/tangent/sign frames remain exact; original package/data unchanged | PASS |
| CPU rendered geometry and frame policy | Exact geometry/UE UV/material/winding; outside normals and valid tangents within one packed component step, explicit degenerate-tangent exceptions, exact W/sign | PASS |
| Dedicated material | `material-build-h`: material-only run succeeds with 0 errors/warnings; source/derived mesh hashes unchanged | PASS |
| Runtime controls and local traversal | Three final packaged runs ×43 PASS: control/feature NullRHI and rendered feature; bounds, materials, pads/pose/lights and actual capsule corridors | PASS, local scope |
| Visual comparison | Four close runs ×43 and two wide runs ×48 PASS; 32 valid PNGs, paired cameras/quality at 720p/1080p; representative direct inspection | PASS, staged/static scope |
| New Windows build/cook/stage | New Game build, fresh cook and stage/archive complete; all final runs bind to 59d4f795… | PASS; clean disconnected extraction untested |
| Broader gameplay regression | 20 raw NullRHI reports:99 stride/input +460 Apollo/rescue +66 selection +655 field operations =1,280 PASS /0FAIL | PASS, automated |
| Matched static performance | Two 6,000-frame captures ×43 PASS after 300 settling; same final binary/Medium/720p/cap60; actual CSV alignment recorded below | PASS, limited capture |
| Owner/map/config/source preservation |915 original protected files restored/verified;32 original identities and154 earlier payloads match;80 owner save/photo files unchanged without restoration | PASS |

Offline results are in [the source report](../../SourceArt/IntrepidCabin20261111/source-normal-recovery-report.json)
and [fixture report](../../SourceArt/IntrepidCabin20261111/mapping-fixture-checks.json).
Private reproduction checks remain under `Reports/IntrepidCabin20261111/source`.
The inherited OBJ has 3,744 out-of-range UV corner references; the builder
duplicates existing Unreal attributes instead of reimporting those references.

## Accepted mesh and material evidence

The canonical [mesh receipt](evidence/mesh-build.json)
records 64,787 vertices, 289,380 vertex instances, 96,460 retained Unreal
triangles and 12 material slots. The raw OBJ's 97,588 triangles describe the
earlier source file; the current Unreal source already had 96,460 before this
stage. The derivative does not remove more geometry. Its render vertex count
is 102,708 versus 104,651 in the source, reflecting render-buffer packing, while
all 96,460 rendered triangles retain their geometry/UV/material/winding multiset.

The source editable normal/tangent/sign frames were not usable for a direct
raw-frame patch. All 289,380 frame records are baked from actual computed source
render buffers in the duplicate. The 1,237 cabin triangles then receive 3,711
authored normal targets; 3,623 normal instances differ from the baked source.
Cabin tangent projection succeeds for 3,699 corners, with 12 deterministic axis
fallbacks for collapsed untextured cases. Packed cabin targets match within the
actual component half-step plus epsilon; maximum recorded normal angle error is
0.357181 degrees. Technical tangent fallbacks are not scientific measurements.

Outside the cabin, all 95,223 rendered faces match the geometry keys. Their
frames are not bit-identical: 7,161 face rows differ. All normals and valid
tangents satisfy a component limit of `1/127 + 0.000002 = 0.007876016`, with
maximum normal/regular-tangent deltas about 0.007874072. W/sign components remain
exact. Separately, 836 originally zero/parallel tangent corners across 570 faces
are reconstructed and checked for finite unit/orthogonal packed frames:

| Affected source slot | Repaired faces |
| --- | ---: |
| `blinn4SG_002` | 257 |
| `blinn6SG_001` | 113 |
| `blinn2SG_002` | 4 |
| `blinn3SG_001` | 36 |
| `blinn1SG_002` | 160 |

The repaired tangents can differ by up to 1.0 component / 90 degrees from the
degenerate source basis; they do not satisfy the regular-tangent delta bound.
Their normals and W/sign values receive no exception. This is an explicit
rendering repair policy, not outside-frame identity or GPU-pixel equivalence.
The original imported package's before/after SHA-1 remains
`cdf287f6b1713240224a4dfcbe449de0fa2a76ad`; its attributes and computed buffers
also remain unchanged.

The `asset-build-g` commandlet saved the mesh after its 46 passing checks, then
failed during the Python material stage at a pin binding. Keep that failed run
as iteration history. The corrected material-only `material-build-h` commandlet
finished with no reported errors/warnings and produced a passing
[material receipt](evidence/material-build.json),
without rebuilding or changing either mesh. The derived mesh SHA-256 there is
`73a284516833b619a4adcce50da7d5e9afed0737aee024288afc39379664afeb`.
These asset-builder checks remain distinct from the completed final runtime,
review, regression, capture and preservation receipts in the
[verification index](verification-index.json). A separate binary saved-asset
reload attribute audit was not implemented; disk-loaded fresh-process views are
visual/runtime evidence only.

## Reproduction templates

The commands below are templates with fresh example labels; exact accepted
labels/arguments/hashes are in the index. Run sequentially with no other Editor,
game, shader compilation or build active. These templates select the staged
package; omit `-Packaged` only when deliberately reproducing Editor scope. Omit the older `-Baseline` flag so
the previous surface/foil feature stays enabled in both cabin modes.

```powershell
# Isolated functional control and feature; default NullRHI is not visual proof.
./Tools/verify_intrepid_visual.ps1 -Packaged -Cabin -CabinBaseline -Label cabin-control-test
./Tools/verify_intrepid_visual.ps1 -Packaged -Cabin -Label cabin-feature-test

# Four PNGs per run: two close cameras, Low and Medium, FOV 60.
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -CabinReview -CabinBaseline -Label cabin-control-close-720
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -CabinReview -Label cabin-feature-close-720

# Keep the established four overview/ground cameras for comparison.
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Cabin -CabinBaseline -Label cabin-control-wide-1080 -Width 1920 -Height 1080
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Cabin -Label cabin-feature-wide-1080 -Width 1920 -Height 1080

# Same four static cameras as the prior profile; these do not profile gameplay.
./Tools/profile_intrepid_slice.ps1 -Packaged -Cabin -CabinBaseline -Label cabin-control-q1 -Frames 6000 -GpuTelemetry
./Tools/profile_intrepid_slice.ps1 -Packaged -Cabin -Label cabin-feature-q1 -Frames 6000 -GpuTelemetry
```

`Tools/build_intrepid_cabin.py` must run inside Unreal's Python commandlet after
the Editor module compiles; it invokes `LunarRescue.BuildIntrepidCabin` and writes
fresh asset receipts. It intentionally refuses an existing builder receipt or
destination mesh rather than silently overwriting them. Asset generation is
separate from `Tools/package_apollo12.ps1`; the new assets must exist before cook.

The review fixtures are staged cameras with frozen mission/player presentation,
not human screenshots, UI tests or a full mission run. The static profile uses
normal delta time and a 300-frame settling phase, with identical resolution,
quality, FOV/cameras, render scale, cap/VSync, parked actors and audio settings in
each pair. Record hardware, exact binary/module hashes, sample counts, frame-time
statistics and memory definitions. NullRHI, still PNGs, a 60 FPS cap or an earlier
result cannot demonstrate a new cabin performance result or motion stability.

## Canonical execution and visual evidence

The final inner Game SHA-256 is
`59d4f795b1141694741264370efe9b79dad66eaa9b4487270ffe7f9517fc358c`.
The selected final input fingerprint is
`d9336b2dbdda2eacbb184893e7a2186ce1ac879e9eed403efa837203866cefa9`.
The base commit remains 4d1d651…, with a dirty working tree recorded honestly.
Four earlier Editor runs use their separate 97dff3eb… input fingerprint because
source/protocol instructions were clarified afterward; C++/assets did not
change after the accepted Editor views and normal JSON remained fa414e9d….

| Counted scope | Raw report executions | PASS / FAIL |
| --- | ---: | ---: |
| Local functional + reviews | 9 | 397 / 0 |
| Local static profiles | 2 | 86 / 0 |
| Broader regressions | 20 | 1,280 / 0 |
| Combined final package | 31 | 1,763 / 0 |

All reports have one zero-failure RESULT, matching controller/manifests,
freshness, final executable/source fingerprints and artifact hashes. Broader
Apollo raw outputs were copied immutably before conventional earlier report
names were restored. An initial local recount could not decode UTF-16 FieldOps
text; its BOM decoder was corrected and the unchanged raw reports re-read.
No game result was changed or rerun for that decoding issue.

The package has 16 close PNGs (two FOV-60 cameras × Low/Medium × control/feature × two
resolutions) and 16 wide PNGs (four FOV-85 cameras × Low/Medium × control/feature at 720p).
All 32 are structurally verified; representative front/side/wide/ladder views were
inspected directly, not every frame aesthetically approved. Panels have softer
artificial dark seams; silhouette facets and bright/flat low-contrast sunlit
white remain. A small Editor ROI had no all-RGB>=254 pixels, which does not
establish global HDR/exposure correctness. Cloth detail is subtle and this is
bounded polish, not a new photoreal model. World labels/HUD/player are hidden;
ordinary UI readability and temporal shimmer are untested by these stills.

## Matched static capture

Both captures use the final binary, DX11 offscreen, Ryzen 5 6600H / selected
RTX 3050 Laptop GPU, Medium, 1280 × 720, FOV 85, cap 60, VSync/audio off, parked actors and a
frozen mission/player. Each settles 300 frames before 6,000 recorded frames; none
of those recorded rows is discarded. No competing UE/compiler work was seen.
The source cap and fixed order limit interpretation of one run per condition.

| Recorded metric | Cabin control | Cabin feature |
| --- | ---: | ---: |
| Mean FPS | 59.9024 | 59.9837 |
| p99 frame time, ms | 16.7491 | 16.7256 |
| 1% low FPS | 51.6663 | 58.2050 |
| Largest frame, ms | 44.0685 | 25.1989 |
| Maximum sampled process working set, MiB | 627.2227 | 610.7188 |

The control includes three frames over 33.333 ms. These capped captures do not
prove stable 60 FPS, a causal speedup, full-mission cost or on-screen smoothness.
GPUTime has 6,000 positive samples per run; SlateUI 5,999, and LocalUsedMB 5,994/5,998.
These are timestamp/memory counters, not isolated shader cost or validated peak
VRAM. Process working set uses 82/83 samples; actual median poll intervals are
about 1.37/1.35 seconds, not an exact 1-second series.

Actual CSV view rows are 1,501/1,500/1,500/1,499 in both captures; metadata nominal
quarters are 1,500 each. View-position sampling trails game-thread phase changes
by one row. All 6,000 rows remain in each aggregate. The profiles use the original
four views, including an outside-ROI camera; they do not measure the two close
review views. See [independent analysis](evidence/performance-independent.json).

## Final preservation and untested limits

The [sanitized preservation summary](evidence/final-preservation-summary.json)
confirms 915 originals: 906 Saved + 5 Config + 4 maps, exact bytes and modification
times. Eight isolated/config file records required 11 operations (3 byte restores
and 8 mtime restores), including 3 isolated automated save fixtures. All 80 true owner
save/photo files stayed unchanged with zero restores. 46 new engine INIs were
retained; no new owner saves/photos appeared. 32 original asset/source identities,
154 earlier payloads and the previous 860-file manifest match. An additional 114
previous visual documents/accepted receipts retain their hashes/mtimes. Private
owner filenames/inventories remain in ignored local records.

Not performed: binary saved-asset reload attribute audit; missing/incompatible
asset fault injection; clean extracted disconnected-distribution test; complete
temporal/aesthetic review of every frame; human playtest. These gaps are not
converted into successful tests by a build, still image or capped capture.
