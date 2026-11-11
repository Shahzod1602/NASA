# Intrepid visual pass — review and verification checklist

Date: 2026-11-11. Status: the final staged game passed nine local executions (335 raw C++ PASS / 0 FAIL), four review runs with 32 structurally valid 720p/1080p PNGs and two scoped static captures. Representative images were inspected; individual aesthetic review of all 32 frames and temporal checks were not performed. The broader 20-report regressions (1,280 PASS / 0 FAIL), 154/154 original payload identity and 869-file final owner/config/map preservation checks also passed. Combined final packaged evidence is 1,615 PASS / 0 FAIL across 29 raw report executions; Editor evidence remains separate. Earlier b screenshots are iteration history: their 40 passing harness assertions did not establish visual acceptance; dark foil and incorrect extra rung-strip placement were rejected, the strips were removed, and the review camera was corrected to the retained model's +Y ladder side. Checkboxes are acceptance criteria, not completed tests. Source inspection, a successful material script, a C++ build, a screenshot and a human playtest are separate kinds of evidence.

## Separate preliminary Editor observations, 720 c pair

`editor-control-720-c` and `editor-feature-720-c` each report 40 PASS / 0 FAIL, with eight valid 1280 × 720 PNGs and matching requested/actual Low or Medium quality. Their four camera transforms/FOV, all 85 terrain samples/grid fingerprint, 8,057 outside original-instance fingerprint and directional-light snapshots match exactly. Source-controller environments identify the same Editor executable/module. Feature logs confirm 3 foil / 1 metal slot, 12 added folds and 54 lander instances in six batches; baseline has 42.

Direct image inspection confirms the original ladder's rungs/rails on the corrected side and removal of floating added strips. Lit foil is readable gold, while shaded foil remains brown/dark. The white ascent cover retains visible coarse facets. This is acceptable local prototype polish, not a new photoreal model. HUD, the player and `UTextRenderComponent` world/poster labels are hidden in these staged captures; ordinary signage/UI still needs its own check. Static screenshots do not establish temporal quality, full-mission usability or FPS.

## Evidence to record

- [ ] Record source revision/dirty state, source fingerprint, Unreal version, compiler, build configuration, staged inner executable SHA-256, hardware/driver and exact commands.
- [ ] Record hashes of the protected NASA raw files/manifest, imported terrain mesh, original source image/materials, existing maps and owner save/photo data before and after the run. Separate intended new assets from protected originals.
- [x] Confirm all four `/Game/Lunar/Apollo12/IntrepidVisual/M_Intrepid*` materials compile and cook, including instanced-mesh usage where required. No default/checkerboard material or missing cooked dependency may be accepted.
- [ ] Retain `material-build.json`, raw C++ assertion report, manifest, terrain grid/outside-instance fingerprints, output hashes, image dimensions and profile CSV with their exact executable/source provenance. Never count wrapper stdout and the same raw report twice.

## Functional and preservation checks

- [ ] Verify the actual imported LROC mesh, identity transform, complex collision and 85 sampled local/outside height/normal values remain unchanged. Sample agreement is evidence for the sampled locations; protected asset hashes provide the full-file identity check.
- [x] Compare original outside-scatter inventories between the same-build baseline and feature mode. The old random sequence must remain stable outside the local region.
- [x] Check one local layer actor, six fine plus three shadow HISM batches, positive accepted counts within the 2,048/64 budgets, finite transforms, bounded culling, no collision/navigation and no new visual Tick.
- [x] Check transformed rock bounds, partial burial against independent ground rays, clear kit/lander foreground and medium-site separation. Follow any failed assertion back to the placement rather than treating a placement budget as a success count.
- [x] Walk all three local corridors using the actual collision-enabled character after fixture setup. Confirm the kit is reachable/visible, traveled distance comes from movement after setup, no added blocker appears, and resource/mission state is not advanced by the visual fixture.
- [ ] Verify optional-material absence has an explicit, usable previous-presentation fallback. Check missing retained rock dependencies too; a removed old scatter layer without its replacement is not a successful fallback.
- [x] Run existing staged Apollo, legacy Moon/Mars, mission-selection, stride/input and field-operations regressions. The separate 20 raw reports pass with 1,280 assertions; these remain automated coverage, not full human playtests.
- [ ] Verify no owner checkpoint/photo/settings are replaced by automation. Use a clean local offline extracted package to confirm assets do not depend on runtime internet or Unreal Editor.

## Fixed-camera image review

Capture the same four views (`intrepid-wide`, `fp-ground`, `ladder-foot`, `outside-ground`) at Low and Medium and at both 1280 × 720 and 1920 × 1080. Run each setup with and without `-IntrepidVisualBaseline` on the same executable, retaining identical transforms, FOV 85, quality and display settings. The review driver stages cameras and hides HUD/player presentation and text-rendered world/poster labels; it is not a natural player recording.

- [ ] Ground has readable near detail without a repeated stamped pattern, excessive grain, shimmer, unstable normals, bright sparkles or obvious distance-fade bands. Inspect stills and actual camera movement; a still image cannot prove temporal stability.
- [ ] Ground material/rock density transitions are unobtrusive at the 3.5 m edge fade. Inspect all sides of the square and its corners, including the AO blend ending at the intended boundary.
- [ ] Camera samples outside the square retain the previous terrain presentation. A matching outside frame does not prove every outside pixel/camera is identical.
- [ ] Fine fragments appear embedded rather than floating, visibly sliced, implausibly uniform or stacked in dense shadow clumps. Check from ground height as well as the wide view.
- [ ] Lander apron, field-kit approach and ladder remain readable and clear; no stones or fold strips obscure an interaction target or visibly intersect a hand/tool.
- [ ] Gold foil has restrained rough highlights and folded relief without white glare, plastic uniformity, obvious floating fold strips or z-fighting. Structural metal remains distinct from foil.
- [ ] Image-bearing panels, engine and white ascent cover retain their original appearance. Inspect the three foil and one structure material overrides, 12 folds and unchanged original ladder. Confirm no rejected extra rung strips or corresponding tag remain, and that the new ladder-foot image visibly includes the actual ladder rather than relying on source coordinates alone. Confirm six existing detail batches and the current logged total (expected 54 versus baseline 42, subject to contact-collar fixture state).
- [ ] Footpads remain grounded in the current reference-model pose. Treat this as a contact/presentation review, not a certification of lander dynamics or historical exactness.
- [ ] Black sky, shadow direction and baseline lighting remain consistent. The bounded AO must not crush useful shape detail or create a hard dark region. Existing first-person glove/carry readability must remain usable.

## Matched graphical performance

- [ ] Run no other UE process or heavy asset/hash work during captures; record any unavoidable competing load. Use normal engine delta time, not benchmark/fixed-step acceleration.
- [ ] Compare baseline and feature captures with identical hardware, resolution, quality, render scale, FOV, frame cap, VSync, camera phases and support-actor state. The only feature-control difference is `-IntrepidVisualBaseline`.
- [x] Keep the 300-frame settling period and equivalent sample count; record each fixed-camera phase. The two 6,000-frame captures pass; actual CSV camera row counts are 1,501/1,500/1,500/1,499 in both captures rather than exactly 1,500 each. Exclude and document process startup/shader compilation rather than hiding hitches inside a mean.
- [ ] Report mean and p95/p99 frame times plus relevant game/render/GPU timing, draw/instance workload and memory where actually measured. State unavailable metrics. A capped FPS mean alone cannot establish extra GPU headroom.
- [ ] Separate rendered static-fixture captures from local walking, rover/full mission workloads and human playtests. NullRHI results are functional evidence only.
- [ ] Keep the earlier `06c1b022…` opening capture (Q1, 59.995 FPS; p99 18.2171 ms) labeled historical context. It is not the local same-build baseline. Avoid a stable-60-FPS or performance-improvement claim until the matched captures support it.

## Result record

| Evidence category | Status | Receipt / limitations |
| --- | --- | --- |
| Editor/Game compile and stage | Passed | Final Game build, fresh cook and stage/archive; inner executable `dc0b5864…`; full SHA/log markers in verification index |
| Four material assets | Passed in staged game | Dedicated material paths available at runtime; retained AI bitmap SHA recorded; full original inventory comparison remains separate |
| Local baseline/feature functional checks | Passed, local scope | Three packaged runs ×35 PASS; about 28.36 m post-setup capsule movement each; complete-mission/human traversal not inferred |
| Rendered review at both resolutions/qualities | Structural matrix passed; representative inspection accepted | Four packaged runs ×40 PASS / 32 valid PNGs; exact paired cameras/quality; not every image individually aesthetically approved; stills only |
| Same-build graphical performance | Two scoped captures completed | 6,000 frames each after 300 settling; Medium/DX11/720p/cap60; 59.9821 vs59.9961 mean FPS, p99 16.7302 vs16.6994 ms; single capped static pair, not universal FPS or proof of improvement |
| Broader regression and offline package | 20 broader raw reports passed; clean extraction untested | 1,280 PASS / 0 FAIL, NullRHI; do not infer human usability or disconnected clean-distribution behavior |
| Protected source/map/user-data preservation | Passed | 154/154 payload size/SHA identity;869 original bytes/mtime match (Saved860+Config5+maps4);128 files/251 restore operations;80 true owner save/photo files unchanged without restoration;9 test fixtures separate;46 generated engine INIs retained |
| New-user playtest | Not performed | Do not invent participants, comments or completion rates |

Final acceptance requires reviewing failures, visual regressions and measured cost together. The intended outcome is a better local Apollo presentation with retained scientific terrain provenance, not a claim that the complete game has become photorealistic.
