# Lunar Rescue audit fixes — 2026-09-21

## Gameplay

- Radio subtitles now use a remaining-time countdown that freezes while paused.
  The display duration also covers the associated voice clip.
- All station contact/restart sounds are tracked and paused/resumed together.
- Falling outside the mission area has its own failure message, separate from oxygen depletion.
- Q selects the navigation marker. It no longer prevents using a nearby lander or recorder.
  Prompts and hold durations follow the usable object.
- The optional ledge validates its mesh and bounds before use and falls back to an engine cube.

## Presentation and memory

- Smaller, lower first-person hands and carried power cell clear more of the view.
- The power port has a smaller cabinet, trim, vents and a small status light.
- The cable follows a curved path with end connectors; all cable segments appear together.
- The survey ledge uses the surrounding world-space regolith material.
- The default profile uses a 384 MB texture streaming pool, shadow quality 2 and a 60 FPS cap.
  Texture residency is limited to available VRAM.
- `DefaultGameUserSettings.ini` supplies the 60 FPS limit and windowed 1280×720 defaults
  for standalone launches, preventing user-settings initialization from resetting the cap.
- The original map is backed up at `Backups/BeforeAuditFixes/MoonBase.umap`.
  `Tools/polish_power_panel.py` reapplies the panel changes without rebuilding the world.

## Verification

The Development Editor build succeeded. There are existing float-literal conversion warnings
in the Canvas HUD; no compilation errors.

- 50 gameplay checks passed without sound.
- 52 gameplay checks passed with graphics and sound, including both power audio pause/resume checks.
- 9 movement checks and 3 ledge traversal checks passed.
- A separate run forced the ledge mesh to be unavailable: all 50 gameplay checks
  passed using the fallback, with no crash.
- The new walkthrough starts at the real spawn and uses normal character movement and collision.
  It collects the cell, repairs the station, downloads data, jumps onto the recorder ledge,
  retrieves the recorder and returns to the lander. It does not teleport or override mission stages.
- Headless walkthrough: 127.65 seconds, 172.33 seconds of oxygen remaining.
- Graphics walkthrough: 127.67 seconds, 172.40 seconds of oxygen remaining.
- Graphics timing: 7,656 frames, 59.98 mean FPS, 16.67 ms mean / 16.84 ms p95 frame time.
  Peak process physical memory: 2,491.4 MiB. DX11, 1280×720 offscreen,
  RTX 3050 Laptop GPU, conservative profile, 60 FPS cap.

These measurements describe this machine and this route. They do not establish performance
at other resolutions or on all hardware, and peak process RAM is not a VRAM measurement.
The visuals remain a prototype: the hands are procedural, not a newly rigged astronaut model.

Logs, screenshots and raw results are under `Reports/`. Run
`Tools/verify_audit_fixes.ps1` to repeat the full suite, adding `-Graphics` for the graphics profile.
`Tools/package_windows.ps1` builds and archives a standalone Windows Development version to `Dist/`.

## Standalone Windows build

The Windows build/cook/stage/archive completed successfully. The standalone executable is
`Dist/Windows/LunarRescue.exe`; the normal and low-memory launchers now prefer it when present.
No Unreal Editor installation is required to run this package. Copy the complete `Dist/Windows`
folder, including its Engine and LunarRescue folders. Asset credits and bundled license texts
are included under `Dist/Windows/Credits`.

All 52 gameplay checks also passed in the standalone binary with graphics and sound enabled.
The final package also completed the full walked route in 127.65 seconds with 172.42 seconds
of oxygen remaining. At the default 1280×720 profile it averaged 60.00 FPS over 7,657 frames
(16.67 ms mean, 16.81 ms p95); peak process physical memory was 704.9 MiB.
The outer `LunarRescue.exe` bootstrap also passed a separate 50-check headless smoke run.
Final raw results: `Reports/packaged-gameplay-final.txt`,
`Reports/packaged-walkthrough-final.txt`, and `Reports/bootstrap-smoke.log`.
