# Mars expedition update — 0.2.0

The game now starts at a Moon/Mars mission selection screen. Moon keeps its
existing map and rescue sequence. Mars adds a separate fictional canyon, ARES
outpost, rover, solar arrays, ascent lander and orbital relay. Shared equipment
continues to use the project's existing credited assets.

Mars uses 371 cm/s² gravity, an adjusted jump impulse and 480 seconds of oxygen.
Its mandatory route is cell retrieval, three-step power repair, science download,
four-second antenna alignment and lander return. The optional crew recorder is
available after relay alignment. Holding E animates the antenna; looking away
cancels progress and pausing freezes progress, movement, oxygen and audio.

English ARES dialogue has separate audio and subtitles. HUD, briefing, science
objective and gravity indicators follow the selected planet. R restarts the
current map; M returns to selection from briefing, pause or results. The Windows
launchers and packaged default map open the selection screen.

The Mars generator is deterministic and only replaces the Mars/menu maps. It
builds fictional canyon terrain, eroded mesa meshes and procedural materials.
It does not modify the existing Moon map. Screenshots revealed a sky-shadow
problem and an overlapping directional-light priority; both were corrected
before packaging.

## Reproduce

- Build `LunarRescueEditor Win64 Development` with Unreal Engine 5.8.2.
- Run `Tools/build_mars.py` through the Unreal Python commandlet.
- Import supplied radio WAVs with `Tools/import_mars_audio.py`.
- Run `Tools/package_windows.ps1`.
- Run `Tools/verify_planets.ps1 -Packaged -Graphics -Walkthrough`.

The traversal test uses normal character movement, collision, real mission time
and held interactions. It does not teleport or advance mission stages directly.
The smaller gameplay checks deliberately isolate transitions and edge cases.

## Verification on the standalone Windows package

Unreal Engine 5.8.2, Windows, DirectX 11, 1280×720 offscreen, sound enabled,
RTX 3050 Laptop, 16 GB RAM, 384 MB texture pool, shadow quality 2, 60 FPS cap.
Offscreen measurements are local test results, not a guarantee for other PCs.

- Selection, Moon restart, return to selection, Mars restart, return: 7/7.
- Moon gameplay checks: 52/52.
- Mars gameplay checks: 56/56, including paused relay alignment.
- Mars movement/footprints/dust/pause: 9/9.
- Mars recorder ledge with real Space/W input: 3/3.
- Full Moon traversal including optional recorder: 127.69 seconds; 172.39 seconds
  oxygen left; 59.97 mean FPS; 16.86 ms p95; 708.7 MiB peak process memory.
- Full Mars traversal including relay and optional recorder: 354.00 seconds;
  126.08 seconds oxygen left; 59.97 mean FPS; 16.84 ms p95;
  530.7 MiB peak process memory. No traversal failures on either planet.

The editor build and Windows BuildCookRun completed successfully. Source diff
checks and Python syntax validation passed. Existing HUD float-conversion
warnings and optional profiling DLL notices are unchanged from the baseline.

The outer Windows launcher was also verified: no map argument opens mission
selection, and explicit Mars launch produces the expected ARES screens. After
the full traversal measurements, a final cosmetic pass grounded distant mesas,
the rover and its spare cell. Functional and visual checks were repeated on the
repackaged build; the traversal measurements above precede only that placement
adjustment.
