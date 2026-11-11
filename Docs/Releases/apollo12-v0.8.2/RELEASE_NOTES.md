# LunarRescue Expeditions v0.8.2 — Intrepid Surface and Cabin

Apollo 12's landing area and Intrepid reference model receive a focused visual
update. Existing Apollo 12, SELENE and ARES expeditions remain available.

- Add a bounded 50 × 50 m authored regolith and grounded-fragment art layer.
- Refine foil and metal finishes, with restrained blanket folds and contact shading.
- Recover the retained GLB's authored normals on two main cabin objects in a
  separate mesh, preserving geometry, UVs, material assignments and lander pose.
- Bake the source rendering frames and explicitly repair invalid legacy tangent
  bases. Shared white cabin/trim slots receive subtle cloth shading with their
  original distinct neutral tints.
- Preserve existing NASA/LROC elevations, collision, work-site locations and maps.
  Added surface/cloth details are authored art, not new planetary measurements.

Download and extract the **entire Windows ZIP**, then run **LunarRescue.exe**.
Keep Engine, LunarRescue and Credits together. Unreal Editor is not required.
If Windows needs the Microsoft C++ runtime, its installer is included under
Engine/Extras/Redist/en-us. Open **Explore the Moon → Apollo 12 → Start Mission**
to see the Intrepid changes. Settings and READ_ME.txt explain the controls.

## Verification and limits

Inner executable SHA-256:
`59d4f795b1141694741264370efe9b79dad66eaa9b4487270ffe7f9517fc358c`.

**1,763 C++ PASS executions / 0 FAIL across 31 raw packaged reports**, including
the focused control/feature checks and selected Moon/Mars regressions. The 32
packaged review PNGs are separate staged presentation evidence. Counts include
repeated configurations; they are not 1,763 unique tests. Preliminary Editor
checks (182 PASS), asset-builder checks (46) and previous-build results are not
added to the canonical packaged total. Human playtests: **0**.

The final Windows Development executable and Editor target built with Unreal
5.8.3; a fresh four-map cook and staging completed successfully. The original
model silhouette remains coarse and the cloth pattern is subtle. This is not
a photoreal reconstruction or proof of complete temporal/animation quality.

On Ryzen 5 6600H / RTX 3050 Laptop 4 GB / approximately 16 GB RAM, a single matched
DX11 offscreen four-camera pair at 1280 × 720, Medium, 60 FPS cap and 6,000 frames
per run measured feature mean 59.98 FPS and p99 frame time 16.73 ms. This capped
workload establishes neither a causal speedup nor sustained full-mission FPS or
performance on other hardware. NullRHI functional runs are not graphics benchmarks.

No new runtime internet dependency was introduced. Offline science/source notes
are included. A firewall-isolated test and human playtest were not performed.
Any later clean-extracted release smoke is documented separately in
RELEASE_SMOKE.json on the GitHub release and is not added to the canonical total.

The clean ZIP excludes player saves/photos, generated reports/logs, crash dumps
and debug symbols. Every included runtime file is SHA-256 recorded and the ZIP
is independently read back after creation. BUILD_INFO.json links the release
source commit separately from original compile-time base/dirty provenance.
Unchanged original records are retained here as COMPILE_BUILD_INFO.json and
ORIGINAL_LOCAL_VALIDATION.json; their unpublished/local flags describe that
earlier stage. VALIDATION_INPUT.json links all 31 canonical raw reports.

NASA / Michael D. Carbajal is credited for the retained lunar-module reference
model. Existing third-party licences and credits remain applicable. No NASA
endorsement, astronaut-training certification or Space Apps eligibility is claimed.
Competition eligibility remains unconfirmed; product quality is a separate matter.

See TEST_REPORT.md, SCIENCE_AND_SOURCES.md and the dated
Docs/IntrepidCabin20261111 and Docs/IntrepidVisual20261111 evidence bundles.
