# v0.5: suit, tools, settings and evidence

This is the implementation and validation record for the v0.5.0 playable
milestone, built with Unreal Engine 5.8.3 on Windows on 2026-10-25.

## Character

The original authored suit geometry is assembled into actual skinned GLB meshes
with a 36-bone skeleton and weighted cloth joint collars. Both the full suit and
first-person hands use this rig. Fingers have a base and tip joint. Source GLBs,
JSON bone positions and generation/import scripts are included. A reference
stride animation is imported, while runtime bone poses use terrain-aware code.
Stance feet are held in world space to reduce sliding. The art remains an
A7L-inspired approximation; it is not a dimensionally exact or scanned suit.

## Science actions

E (or the rebound work key) opens a tool. Left click performs its primary action;
Esc cancels before pausing the game. Work tools use a close view and stop movement;
the context camera allows walking to compose the photograph.

- Context camera: center the target and release the shutter. Surveyor's legs and
  solar panel must fit inside the frame. Opening the camera alone creates no evidence.
- Seismometer: adjust and confirm a centered bubble. Antenna: rotate and confirm
  a strong signal. Both support mouse and arrow keys.
- Head 12031: tongs, then bag and label. Bench 12032: soil scoop, then seal.
- Sharp 12027: place a drive tube, make four separated hammer actions, extract
  and cap it. These are simplified actions, not historical stroke counts.
- Surveyor: release two fittings or cut both cable ends, then remove and pack.
  The original component disappears only when packing is committed.
- Intrepid: check cargo and confirm stowage. The evidence album and debrief use
  the actual session's cargo, timestamps, distance and photographs.

Tool progress is transient. Cancelling cannot award partial cargo or overwrite a
stable checkpoint with an unfinished action. Completed photo/task IDs retain the
v0.4.3 save schema, with explicit historical or timed mode.

## Presentation and settings

UMG panels provide scrollable settings and the evidence/photo album. F1 opens
settings; I opens the album. Settings are separate from mission saves and include
key bindings, sensitivity, invert Y, task/subtitle size, brightness, view angle,
radio/effect volume, hand motion, stride toggle and three graphics presets.
Reserved and duplicate bindings are rejected. Existing Canvas mission/journal
screens remain; this is not a claim that every UI screen has migrated to UMG.

Historical exploration is untimed by default. T changes the mode before a new
run. A separate timed exercise retains the 30-minute budget; old saves keep their
original mode. New English radio narration is locally synthesized from the same
text as the subtitles and is labeled as authored game dialogue.

The map retains the source terrain and work-site coordinates. Additional Surveyor
and ALSEP details, tool meshes, limited pawn collision and stronger readability
fill are visual/gameplay approximations. See `HISTORICAL_AUDIT.md` for claims and
sources, and the asset-specific credits for provenance.

## Acceptance work

The new tool integration suite exercises cancellation, prerequisites, different
sample actions, hardware removal, mode persistence and settings/panel behavior.
Existing legacy completion and checkpoint suites remain separately identified.
The full movement route uses the same new tools, input, collision and committed
task transitions as gameplay.

`Tools/profile_full_mission.ps1` captures a full rendered automated mission,
including frame times and sampled process/GPU memory where supported. Offscreen
profiling is not an on-screen or human comfort test. Zero GPU timing counters are
not treated as measured GPU performance.

`PLAYTEST_GUIDE.md` prepares the requested 5-8 first-time-player acceptance test.
That human study is pending until real participants play the build. No invented
player observations, completion rates or usability results are included.

## Recorded validation

- Editor: 300 checks passed across Apollo, legacy Moon, Mars, menu, suit, tools,
  settings and checkpoint suites. Packaged game: the same 300 checks passed,
  plus three fresh-process checks of the completed historical checkpoint and
  its four real PNG photographs. No failed checks were reported.
- After the final photo HUD suppression, 129 packaged Apollo/suit/tool checks
  passed again. A new rendered photograph was inspected: no HUD, hands or
  camera tool appeared in the saved image. Existing photographs are preserved.
- 28 packaged 1280 x 720 interface views and eight suit/movement views were
  reviewed. The actual completed-run album was also opened in a fresh process.
- Both skinned meshes have 36 bones, normalized blended weights and three
  decreasing LODs. Six tool meshes and seven authored radio clips loaded.
  Six raw NASA/LROC source files and the generated science table remained
  unchanged. The player's existing save and settings remained byte-identical.

The full rendered Medium-preset route completed all three samples, all three
Surveyor parts, ALSEP and stowage: **1,238.01 m in 570.52 seconds**. This is an
automated fast-stride run, not a person's first-play completion time.

| Capture | Measured time after warmup | Mean FPS | 1% low FPS | Largest frame |
|---|---:|---:|---:|---:|
| Medium, full mission | 562.60 s | 59.90 | 51.18 | 206.05 ms |
| Low, opening segment | 60.04 s | 59.96 | 52.17 | 57.65 ms |

Both captures used DX11, 1280 x 720, rendered offscreen, a 60 FPS cap, normal
delta time and no audio playback. The machine has a Ryzen 5 6600H, 16 GB RAM
and an RTX 3050 Laptop GPU. Full-mission capture contained 34,298 frames; the
first 600 were excluded. Sampled process working-set peak was 632.56 MiB.
The GPU memory counter became unavailable, so a complete-run VRAM peak and
GPU execution time are not claimed. The 206 ms hitch remains measured; its
cause has not been isolated. High quality and other PCs were not profiled.

Full-route and Low captures preceded the final photo-only HUD guard and
album-caption wording change. A separate 120-second Medium opening run
verified the new clean photo (59.85 mean FPS, 43.81 1% low, 244.52 ms largest
frame); it is not a replacement full-route measurement. Captures, raw CSVs,
environment records and individual test results are local under `Reports/`;
the release's `BUILD_INFO.json` records the validation summary and file hashes.

Remaining acceptance work is the real 5-8-player study, on-screen comfort and
audio review, and broader hardware testing. Procedural poses and guided tool
motions remain approximations. This milestone does not claim final animation
quality, hitch-free performance or completed human usability validation.
