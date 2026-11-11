# Intrepid local visual pass — implementation and verification

Date: 2026-11-11. Base: `4d1d651f43809071da42409f483506487099f164` (the published v0.8.1 source). Status: final Windows Game build, fresh cook/stage, nine local packaged executions and 20 broader regression reports passed. Final original-payload and owner/map/config preservation receipts also passed. There has been no new public release or human playtest for this pass.

## Scope

The pass improves the immediate Apollo 12 Intrepid landing-site presentation within a 50 × 50 m square centered on the existing `ApolloData::Home` anchor. The local square uses a 2,500 cm half-extent and a 350 cm material/scatter edge fade. It reuses the imported LROC terrain and the established lander model. No new elevation data, map regeneration, mission-coordinate relocation, collision surface, or resource/physiology model is part of this work.

The new surface layer is Apollo-only. SELENE and ARES keep their existing authored environments. The lander finish is limited to the Apollo reference model. Missing optional finish materials retain the previous lander appearance; required ground material availability gates the surface replacement. Missing-asset behavior still requires a runtime check.

## Source and asset changes

| Location | Responsibility |
| --- | --- |
| `Source/LunarRescue/LunarIntrepidVisual.cpp/.h` | Bounded surface material, deterministic cosmetic scatter and local contact shading |
| `Source/LunarRescue/LunarSurface.cpp` | Retains the existing outside scatter sequence and replaces eligible local scatter with the new layer |
| `Source/LunarRescue/LunarLander.cpp` | Restricted foil/metal overrides and small cosmetic blanket-fold details |
| `Tools/build_intrepid_materials.py` | Supported Unreal Python construction of four dedicated material assets |
| `Source/LunarRescue/LunarIntrepidVisualChecks.cpp/.h` | Isolated functional, fixed-camera review and normal-delta profiling drivers; scoped receipts below |
| `Source/LunarRescue/LunarApollo.cpp`, `LunarSettings.cpp` | Driver dispatch and automation isolation |

The material script writes only `M_IntrepidRegolith`, `M_IntrepidRock`, `M_IntrepidFoil` and `M_IntrepidMetal` under `/Game/Lunar/Apollo12/IntrepidVisual`. It requires the existing `T_RegolithAuthored` texture, rebuilds only these new material graphs, and writes a local material-build receipt. It does not load/save maps or save the original materials, texture, source image or terrain. Run it through Unreal's supported Python commandlet, not as ordinary system Python. Material generation is separate from compiling and cooking the game.

## Surface presentation

The new ground material uses the existing AI-authored bitmap with additional filtered, world-space color and normal detail. It adds no world-position offset, displacement or collision vertices. Outside its mask it reconstructs the prior ground color/normal formulas and retains roughness 0.94/specular 0.16; rendered seam comparison remains required.

Fine scatter has a cap of 2,048 non-shadowing fragments across six existing basalt mesh variants. Medium scatter has a separate cap of 64 shadowing fragments across three batches. These are placement budgets; the final feature fixtures also reached 2,048/64 accepted instances. Fine widths are 1.5–11 cm; medium widths are 11–26 cm. The fixed random seed, clustered fine distribution and 1.8 m minimum medium-site separation make comparisons reproducible. Placement is cosmetic and does not represent surveyed rock locations.

Burial uses the eight corners of each rotated, scaled source bound projected onto the hit normal. The placement rule puts 25% of that projected bound height below the terrain tangent plane. It is an art rule, not a physical soil or sediment estimate. The lander apron, field-kit foreground and approach corridor have authored placement exclusions. The final driver checks transformed-bound containment, sampled grounding and local clearance; 132 grounding samples pass in each feature fixture.

The nine new HISM components have collision and navigation disabled, build their instance trees once, and add no actor/component Tick. Fine batches cull over 20–32 m; shadowed batches over 35–50 m. Instancing and culling bound the workload but do not establish an FPS result.

Contact shading is a bounded post-process AO override: intensity 0.78 and radius 35 cm. Its box has 22.5 m horizontal half-extents plus a 2.5 m outer blend, intended to finish at the 25 m square boundary. It adds no light and no sun, sky, fill, exposure or bloom override. Existing baseline lighting/presentation code runs in both modes. The AO settings are art tuning, not calibrated lunar radiometry.

## Intrepid finish

Three source model slots (`blinn10SG_001`, `blinn9SG_001`, `blinn1SG_002`) receive the new foil material; the metal structure slot matching `blinn4SG` receives the metal material. Source image-bearing panels, engine and white ascent cover retain their established materials.

Twelve shallow foil folds add 12 instances to the six established detail batches. The c Editor review logs confirm 54 instances versus 42 in the paired baseline, in six batches. These totals describe this fixture; contact-collar placement depends on ground gaps. No added rung strips remain: rendered inspection revealed that the earlier OBJ-space assumption put them on the wrong side after import handedness conversion. The original ladder remains intact and its existing structure receives the metal finish. There are no new lander batches, ladder collision, climbing behavior, or historically certified tread details. Existing detail culling remains 45–70 m. The retained terrain-footpad alignment is presentation placement, not new landing physics.

The revised foil graph uses a warm base coefficient `(0.58, 0.34, 0.10)`, metallic 0.62, procedural normal gain 1.8 and roughness `0.48 + 0.14 × noise`. These are readability/art coefficients, without new lights or emissive output. The c images were reviewed as a readable local prototype improvement; they are not a calibrated material reproduction. The earlier `editor-control-720-b` / `editor-feature-720-b` runs passed 40 harness assertions each but were rejected during visual inspection for dark bronze foil and misplaced extra strips. They are retained as iteration history, not acceptance evidence for the revised presentation. A subsequent ladder-camera correction targets the imported model's actual +Y ladder side; that change moves the review camera, not model geometry.

## Controlled comparison and verification

`-IntrepidVisualBaseline` disables this pass for a same-build comparison. The implemented driver entry flags are `-IntrepidVisualTest`, `-IntrepidVisualReview`, and `-IntrepidVisualProfile`; `-IntrepidRun=<unique-label>` isolates output folders under `Reports/IntrepidVisual20261111`. The profile driver uses normal engine delta time and a 300-frame settling period; `-IntrepidProfileFrames=<count>` is bounded to 1,200–12,000 frames in four camera phases. The wrappers are `Tools/verify_intrepid_visual.ps1` (functional/review) and `Tools/profile_intrepid_slice.ps1` (normal-delta CSV profiling). Every run requires a fresh label. For example, the paired Editor review command shape is `./Tools/verify_intrepid_visual.ps1 -Review -Baseline -Label <unique-control> -Width 1280 -Height 720`, followed sequentially by the same command without `-Baseline` and with a fresh feature label. Add `-Packaged` only for the separately staged game; these Editor receipts do not imply that it has passed.

The test driver checks the imported collision mesh/transform, a local/outside height grid, outside original-instance fingerprints, visual budgets, materials, collision/navigation flags, sampled grounding and local capsule corridors. Corridor starts are fixture placements; subsequent corridor distance is measured from collision-enabled character movement. This is local automated traversal, not a complete human mission playtest. Review/profile fixtures freeze the mission and pawn, hide HUD, player presentation and all `UTextRenderComponent` world labels, and use four fixed cameras; profile workload is not representative of rover driving or a complete mission.

The earlier v0.8.1/`06c1b022…` Q1 opening capture (59.995 FPS; p99 frame time 18.2171 ms) is historical context only. It is not a matched before/after result for this local pass. New comparisons must use the same executable, hardware, resolution, quality, render scale, FOV, camera phases, frame cap/VSync and workload, changing only the baseline switch. NullRHI functional checks cannot establish graphical performance.

## Final local packaged evidence

The staged inner Windows executable SHA-256 is `dc0b5864f7dd539006711408c00c7c0a880e7a40dc55afffe58d5b26f13bcfe2`. Successful final logs are `editor-final-build.log`, `game-accepted-build.log`, `fresh-cook.log` and `stage.log` under `Reports/IntrepidVisual20261111`. The fresh cook reports zero errors/warnings; cook and stage/archive AutomationTool markers finish with exit code 0. These are local build/stage receipts, not publication or clean-extraction receipts.

[Verification index](verification-index.json) records each raw report once and distinguishes repeated executions from distinct tests:

| Canonical packaged scope | Executions | Raw C++ PASS / FAIL |
| --- | ---: | ---: |
| NullRHI baseline + feature and rendered feature local traversal | 3 | 105 / 0 |
| Baseline + feature fixed-camera reviews at 720p and 1080p | 4 | 160 / 0 |
| Baseline + feature static Medium profiles | 2 | 70 / 0 |
| Local pass subtotal | 9 | 335 / 0 |
| Separate broader regressions | 20 raw reports | 1,280 / 0 |
| Combined final packaged report executions | 29 | 1,615 / 0 |

The separate 20-report NullRHI regression set totals 99 stride/Apollo-input assertions, 460 Apollo/legacy Moon/Mars/mission-flow assertions, 66 mission-selection assertions and 655 field-operations assertions. All source report hashes and counts were independently rechecked. A local PowerShell wrapper initially failed to sum OrderedDictionary entries; its aggregation was corrected by rereading the unchanged raw reports. This was an output-aggregation correction, not a game failure, rerun or changed test result.

Every report has exactly one `RESULT: 0 failures`; its manifest, controller completion/exit, artifact hashes, freshness, unchanged executable and selected input fingerprints are cross-checked. The selected input fingerprint covers 131 files (112 Source entries, four new materials and other recorded inputs); it is not a substitute for the separate full original science/asset inventory. That independent read-only comparison confirms 154/154 original payloads (38 Content, 116 SourceArt) retain their size/SHA-256, without overwrite or restoration. Per-run map/config/owner-data restoration receipts pass. Final all-run preservation confirms 869 original files (860 Saved, five Config and four maps) have their original bytes and modification times. Across cleanup, 128 files required 251 byte/time restoration operations; 80 true owner save/photo files remained unchanged without restoration. Nine isolated automated save/photo fixtures are counted separately, including three restored fixtures. All 46 newly generated retained files are engine INIs; no new owner photos or saves were created. Private file inventories are omitted from the public evidence summary.

Local collision-enabled corridor movement totals are 28.364 m (control), 28.356 m (feature NullRHI) and 28.364 m (rendered feature), excluding fixture placement. This is three short local corridors, not complete-mission or human traversal. Optional-material/mesh fault injection and a clean extracted offline distribution have not been tested by these nine runs.

The four packaged reviews contain 32 valid PNGs: four fixed cameras × Low/Medium × baseline/feature × 720p/1080p. Requested/actual quality, exact camera transforms/FOV 85, all 85 height/normal objects (`def58c9c1a2151d3679ea3140b9d1d0ddeef8de9`), 8,057 outside original instances (`280b43e980d9837783ca8f74dc9d81e88faf3f66`), imported terrain path and directional-light snapshot objects match in each pair. Feature logs confirm 3 foil / 1 metal slot, 12 folds and 54 lander instances in six batches versus 42 baseline. These are recorded state invariants, not equality of every rendered outside pixel.

Representative packaged views were inspected directly, including wide, ladder, near-ground and outside views across the two resolutions. The original ladder's rungs/rails are visible; the rejected floating strips are absent. Lit foil is readable gold and shaded foil remains brown/dark. More irregular chips are visible beyond the clear kit approach. Low-quality near-ground rendering still appears relatively soft, and the original white ascent cover remains coarse/faceted. Not every one of the 32 images has received individual aesthetic approval. The pass is accepted as bounded prototype polish, not a new photoreal model. HUD, player and text-rendered world/poster labels are hidden in these staged images, so ordinary signage, interaction UI and subtitle readability are not verified by them. Motion shimmer or smoothness cannot be established from still captures.

The earlier c Editor pair is separate: two executions with 40 PASS / 0 FAIL and eight valid 720p PNGs each. Its 80 PASS are excluded from the packaged total. Earlier failed capture-driver folders, the visually rejected b iteration and the earlier 35-assertion Editor corridor test are retained as history and also excluded.

## Matched graphical capture

The final same-binary opt-out pair uses DX11 offscreen rendering, Medium, 1280 × 720, FOV 85, a 60 FPS cap, VSync/audio off, frozen mission/pawn and parked support actors. Each capture settles 300 frames before recording 6,000 frames; no captured CSV frames are discarded. No competing UE/compiler/shader work was observed by the controller. Hardware is Ryzen 5 6600H; the RHI log selects NVIDIA GeForce RTX 3050 Laptop GPU (driver 576.80), with 16,312,721,408 system RAM bytes reported. The curated environments retain the full recorded hardware/conditions; the enumerated AMD adapter is not treated as the active renderer.

| Recorded metric | Baseline opt-out | Feature enabled |
| --- | ---: | ---: |
| Mean FPS | 59.9821 | 59.9961 |
| p95 frame time (ms) | 16.6805 | 16.6769 |
| p99 frame time (ms) | 16.7302 | 16.6994 |
| Largest frame (ms) | 23.4549 | 17.2367 |
| Maximum sampled process working set (bytes) | 625,115,136 | 625,872,896 |

These are one capped execution per mode, not evidence that the feature improves performance or that every mission sustains 60 FPS. CSV GameThreadTime, RenderThreadTime, GPUTime and GPUMem/LocalUsedMB are present with positive finite coverage; counter names do not establish true peak memory, total VRAM or an isolated cost for the new art. Process working set is sampled once per second. External GPU samples and optional-counter coverage are retained in the local receipts.

The driver metadata divides the capture into four nominal 1,500-frame phases. Actual CSV view-position transitions occur at row indices 1,501/3,001/4,501 in both runs, giving 1,501/1,500/1,500/1,499 observed camera rows. Aggregate matched-capture comparison remains scoped to the same workload; do not present the metadata boundaries as exactly 1,500 rendered rows per camera.

A separate current opening-route capture records 2,400 frames, excludes its first 600 and reports 59.9982 mean FPS / 17.8246 ms p99 over 1,800 samples. The older `06c1b022…` opening capture reported 59.995 FPS / 18.2171 ms p99. They are historical/current opening-route context from different executables and intervals, not the four-camera local baseline pair and not a causal performance-improvement result. The generic wrapper uses its intentional success exit code 777003 and does not perform its own executable-hash check; this is adjacent-final-build context only and is excluded from raw C++ assertion totals.

See [visual review checklist](VISUAL_REVIEW_CHECKLIST.md) and [science/art provenance](SCIENCE_AND_ART.md). Human playtests remain zero. No claim of finished photorealism, universal stable FPS, NASA endorsement or competition eligibility follows from these receipts.
