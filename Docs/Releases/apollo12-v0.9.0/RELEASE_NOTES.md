# LunarRescue Expeditions v0.9.0 — EVA, Equipment and Exploration

This Windows update improves mission preparation, equipment interaction and the
astronaut/rover presentation across the retained Apollo 12, SELENE and ARES game.

- Four ordered pre-EVA checks with live progress, explicit Next and guarded
  Open Hatch. Map and Inventory work before departure without spending oxygen.
- Stable hatch/player-camera handover and direct mission entry without the
  extra automatic resource-profile modal.
- Offline rotatable NASA planet colour overview, local mission navigation,
  actual inventory filters/details and retained progress widgets.
- Dark cable board with drag connections, labelled signals, animated wires,
  explicit Verify, Online feedback and a click/keyboard alternative.
- Credited astronaut body/gloves adapted to the procedural controller; corrected
  torso/arm fit and visible lower-centre power-cell grip.
- Deutsches Museum lunar-rover model, separate source wheels and four LODs,
  swept body collision, enter/exit, terrain contact and gameplay resource use.
- Grounded habitat/power/solar supports; retired redundant decorative robots,
  prototype foundations, duplicate signs and PIP while retaining useful guidance.
- NASA/JPL Perseverance visual reference for the fictional SELENE NOMAD-7 actor;
  original recovery progression retained. Perseverance is real Mars hardware,
  not historical lunar equipment. The Moon reuse is explicitly fictional.

Download and extract the **entire Windows ZIP**, then run **LunarRescue.exe**.
Keep Engine, LunarRescue and Credits together. Unreal Editor and Python are not
required to play. If Windows needs the Microsoft C++ runtime, its installer is
included under Engine/Extras/Redist/en-us. Select **Explore the Moon** or
**Explore Mars**, choose a mission, then start it. Controls are in Settings.

## Verification and boundaries

Inner EXE SHA-256:
`953d5f4abaefaca3e13b60b468c9d24c5742371a87256bb5fb47471d6a6637b2`.

**2,019 packaged C++ PASS executions / zero FAIL, 20 raw case reports, 109 staged
review PNGs.** These are repeated automated executions, not 2,019 unique tests
or human playtests. Human participants: **0**. Selected native Moon/Mars/menu,
mission selection, suit/guidance, field operations, grip/departure and rendered
UI checks passed on the exact executable and cooked payloads. See the retained
raw evidence under Docs/Feedback20261113. NullRHI tests are not GPU benchmarks.

Unreal 5.8.3 Editor and Win64 Development Game builds, fresh four-map cook and
local staging succeeded. Original local compile/test records retain their true
base-commit/dirty provenance and unpublished flags. The archive's BUILD_INFO
records the reviewed release source commit separately; history is not rewritten.
Later clean-extracted smoke results are separate from the canonical 2,019 total.

One matched short DX11 offscreen 1280×720, quality-1, uncapped opening-walk pair
on Ryzen 5 6600H / RTX 3050 Laptop 4 GB / approximately 16 GB RAM measured mean
230.24→233.39 FPS and p95 5.289→5.210 ms, but 1% low 150.00→139.46 FPS and largest
frame 12.340→15.247 ms. Only 1,800 post-warmup samples (~8 seconds) per build were
analysed. **This does not establish a performance improvement, sustained 60 FPS,
occupied-rover performance or whole-mission smoothness.** No causal explanation
for the variance is established.

NASA colour imagery drives the planet-overview appearance; it is not a new DEM,
survey precision or mission-geolocation claim. Existing Apollo LROC terrain is
retained. Animation, support feet, vehicle dynamics and resource coefficients
are authored game models, not astronaut training certification. Required assets
and source notes are local; an OS/network-disconnected test was not performed.

Known open work: clarify the later FIELD GUIDE preference, obtain better
habitat/lander replacement models, human playtesting and longer driving/stress
measurements. Existing base models received contact fixes, not new replacements.
No new NASA endorsement or Space Apps eligibility is claimed.

Credits include LasquetiSpice (CC BY 4.0), Deutsches Museum | Digital (CC BY-SA
4.0), NASA/JPL and NASA SVS/LROC/LOLA/USGS sources. Adapted rover model assets
remain CC BY-SA 4.0; other assets retain their own terms. The code has no new
blanket licence. Source/adaptation metadata are included offline.

The clean ZIP excludes owner saves/photos, generated logs/reports, crash dumps
and debug symbols. Included runtime files have individual SHA-256 values, and
all ZIP entries are independently read back and verified before upload.
