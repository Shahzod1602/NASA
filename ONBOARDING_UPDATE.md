# LunarRescue onboarding update - v0.4.2 archive

Historical record of the 2026-10-24 onboarding update. For current language behavior see [the English edition](ENGLISH_UPDATE.md); for subsequent saving support see [checkpoints](CHECKPOINT_UPDATE.md).

## Changes introduced

The Apollo HUD shows one objective, its next action and scientific purpose. The six-stage checklist moved to the paused journal. Larger text, backed destination markers and a visible antenna bearing help navigation. Nonblocking look/walk/work guidance introduces the controls and disappears after checkout.

**Tab / J** opens the map/journal, **Q** changes pages and **Esc** closes it. The journal pauses movement and time. Reaching Surveyor's work area opens retrieval directly; twelve hidden waypoint gates were removed. The reconstructed historical traverse remains on the map and in an optional bot route.

Version 0.4.2 shipped 139 stable Uzbek/English string-table entries for Apollo and the menu, with L switching/saving the choice. Older Moon/Mars text was English. This language switching is historical: v0.4.4 uses English throughout and ignores old preferences/arguments. Text still uses stable String Table keys and FText::Format, with no internet download.

## Baseline and validation

Backups/before-onboarding-20261024-025542 contains 818 files / 2,083,429,741 bytes: source, configuration, assets, source art, tools, editor module, v0.4.1 package and reports. Its manifest records SHA-256, Git HEAD and exclusions; Git status and a binary patch were saved. Engine/build-tool installations were excluded. Creating it did not reset, revert or commit existing work.

Editor compilation and Windows build/cook/package passed. Each target passed **182 checks**, zero failures: Apollo 57, legacy Moon 50, Mars 54, menu travel 7, suit/input/camera 14. Another 57 Apollo and 14 input checks passed in English. Both movement routes completed in editor and package:

| Route | Distance | Simulated time |
|---|---:|---:|
| Direct Surveyor approach | 1,237.29 m | 932.13 s |
| Historical traverse | 1,344.67 m | 1,009.13 s |

These use real input/collision in accelerated headless simulation, not human timing or GPU benchmarking. Fresh semantic RESULT: 0 failures reports were required; process exit zero alone was insufficient.

Sixteen UI/world fixtures were rendered at 1280x720 Uzbek, 1920x1080 English and 2560x1080 Uzbek. They cover briefing, stages, journal, ending, tutorial and work prompts. Main overlays and menu were visually inspected. PNG dimensions were checked; -ForceRes corrected Unreal's initial downscaling to 888x500. Fixture telemetry is not playtest evidence. All 357 Content/SourceArt hashes matched baseline.

## Historical opening-route performance

Ryzen 5 6600H / RTX 3050 Laptop / 16 GB RAM; DX11 offscreen, 1280x720, existing scalability, VSync off and 60 FPS cap. Each capture had 4,200 frames; 600 warmup frames were removed, leaving about 60 seconds.

| Build | Mean FPS | 1% low FPS | p95 ms | p99 ms |
|---|---:|---:|---:|---:|
| v0.4.1 baseline | 60.00 | 57.11 | 17.11 | 17.37 |
| v0.4.2 | 60.00 | 56.15 | 17.20 | 17.47 |

The v0.4.2 sampled process working-set peak was about 575 MiB. These capped captures do not establish an improvement, Low-preset behavior, whole-mission performance, on-screen presentation performance or VRAM usage. Zero DX11 GPU-memory counters were not treated as VRAM measurements. Files: Reports/Performance/v041-baseline and Reports/Performance/v042.

Tools/summarize_profile.py validates frame count and excludes metadata/startup. The 1% low is 1000 divided by mean milliseconds of the slowest 1% of measured frames. The profiling helper reproduces this method.

## Remaining work at that milestone

Version 0.4.2 had not finished full settings, rebindable Enhanced Input, text-size controls, UMG, saving or distinct physical science interactions. Checkpoints arrived in v0.4.3. The suit remains authored static pieces with procedural posing; a skeletal A7L model, final gait, tool-grip IK, core sampling and physical hardware removal remain future work. Lighting/equipment art also need improvement.

The 30-minute exercise limit is an adaptation, not suit endurance. An untimed history mode and a 5-8-person first-time usability study remain planned. No claim is made that four out of five new players understood the game without help.
