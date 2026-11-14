# Authorized-media demo validation — 15 November 2026

This local Windows demo follows the user's declaration that the supplied images/video were team-created with Higgsfield and authorization to use them in this project demo. It restores the unchanged original startup/inventory art and Moon clip, and adds a cleaned Pre-EVA cabin illustration with live controls. The prior v0.9.2 public archive is unchanged. The current demo is not presented as a competition submission or NASA archive/endorsement.

## Exact tested build

Windows 11 x64; UE 5.8.3; VS 2022 toolchain 14.44.35217; SDK 10.0.22621. Ryzen 5 6600H, RTX 3050 Laptop 4 GB, approximately 16 GB RAM. Tests ran 2026-11-14T20:31:37.181546+00:00 through 2026-11-14T20:45:35.5350098Z. Game EXE SHA-256: `59fc0a1544dd440f0caf3b03e16cd7724d32f3d0e1092398f7f73ef72d9a375f`. Source fingerprint: `0dd88e6c5347a8892edf4379150239321985ee159abe20f7d0b7c838bf325533`; compile-time base is recorded separately from later commits.

## Completed commands and outcomes

| Command | Observed result |
|---|---|
| Editor and Game Build.bat, Win64 Development | Both succeeded with one compile action at a time; no PCH/UBA. Game has no debug symbols. |
| Tools/import_startup_feedback.py via Editor Python commandlet | Four real UI textures imported; no map writes. |
| Tools/verify_field_operations.ps1 -Review -Mission selene | Editor reference review passed, including actual hatch callback. Initial navigation-allocation defects were caught and fixed before the final package. |
| Four-map BuildCookRun and Tools/package_apollo12.ps1 -StageOnly | Cook/stage succeeded; movie is loose NonUFS and UI textures are cooked. |
| Tools/verify_field_operations.ps1 -Packaged -Mission all | 744 PASS across Apollo, SELENE, ARES. |
| Tools/verify_gameplay_tasks.ps1 -Packaged -Mission all | 618 PASS across all three destinations. |
| Tools/verify_rescue_feedback.ps1 -Packaged | 126 PASS across six rescue, movement and recorder cases. |
| Tools/verify_task_fixes.ps1 -Packaged -Suite stride | 57 PASS through actual input. |
| Tools/verify_mission_selection.ps1 -Packaged | 66 PASS. |
| Tools/verify_nasa_tasks.ps1 -Packaged -Intro -TeamMediaDemo | 16 PASS; actual startup art, inventory atlas, clean cabin/icon source and complete cinematic exist in the package. |
| Tools/verify_field_operations.ps1 -Packaged -Review -Mission selene | 108 assertions / 41 fresh PNGs; 100/150% text, ordered checks, in-place progress and real blue hatch callback. |
| Same rendered review, -Mission mars | 102 assertions / 36 fresh PNGs. |
| Tools/verify_nasa_tasks.ps1 -Packaged -Intro -Review -TeamMediaDemo | Seven fresh startup/menu PNGs. |
| Tools/verify_loading.ps1 -Packaged -Graphics -Intro | Both selected Moon clips reached actual EOF after 30.258 / 30.138 seconds; world time held. Resume/restart and Mars do not replay lunar footage. Audio enabled; no human hearing audit. |

Total: **1627 native assertion executions** excluding loading phases, plus the rendered assertions and **84 fresh PNGs**. These are automated/synthetic checks, not unique test counts or human playtests. Exact receipt paths/checksums are in VALIDATION_EVIDENCE.json. Root camera/terrain/gameplay anchors remain unchanged; all four source maps match HEAD LFS hashes. All 53 original player save/photo/settings files remain byte-identical; generated config-cache differences, if any, are separately listed in local saved-integrity.json.

## Runtime and provenance

Pre-EVA uses a cleaned static illustration behind live UMG navigation, four guarded sequential checks and actual progress/lock states. The Open hatch control enables and turns cyan-blue only when all four checks finish; it uses the production launch authority. Larger text uses a scrollable single-column layout. The rendered review exercises the real callback instead of inferring success from the picture.

Startup/inventory originals are unmodified. The supplied 854x480 H.264/AAC Moon cinematic is unmodified and has an in-player AI-generated fictional-portrayal caption. It plays after new Moon mission loading and before world time advances. Replays/Continue do not force it again. Existing NASA planetary data remains separate; no new scientific measurements, real suit tests or NASA partnership are claimed. Credits, original illustration references (including the team collage as reference art, not a partner list), the clean background and exact imagegen prompt are retained under SourceArt/AuthorizedMedia20261115 and bundled in Credits. All live UI values still come from gameplay.

## Bounded graphics sample

`Tools/profile_apollo12.ps1 -Label authorized-demo-20261115 -Frames 1200 -Quality 1 -FieldOperationsProfile -FpsCap 60 -GpuTelemetry`: DX11 offscreen, 1280x720 Medium, VSync off, 60 FPS cap; 1,200 captured frames, 600 warmup excluded, 600 analyzed (about 10 seconds). Scripted Apollo opening walk with parked actors; not a preflight/movie or occupied-driving benchmark.

Mean 59.9999 FPS; 1% low 55.7453 FPS; p95 17.2821 ms; p99 17.6489 ms; maximum 18.9653 ms; sampled peak process working set 670,126,080 bytes. Compare only the same bounded recipe in Reports/Performance/public-v092-final. A capped opening sample does not establish full-game stable FPS or a causal improvement. Exit 777003 is the accepted profiling utility exit.

## Play and limits

Launch Dist/Apollo12/Windows/LunarRescue.exe. Choose a **new Moon mission** to see the cinematic; Continue/restart intentionally bypass it. Finish the four direct check buttons, then choose Open hatch. Mars retains its own mission and tuning; lunar footage is not used for Mars.

Use Tools/archive_windows.py with the tested EXE/movie hashes to generate a clean Windows ZIP; it excludes all Saved/Reports/backups, environment files and debug symbols while retaining authorized media. Archive CRC/hash and a clean extracted-launch smoke receipt are retained next to that ZIP; they are not implied by packaging alone. Extract the full archive and launch Windows/LunarRescue.exe. The bundled VC++ redistributable is available if needed; Unreal Editor is not needed to play.

No human playtest of this new demo, audible human review, OS network isolation, full occupied-driving/scanning profile or 1080p visual pass has been performed in this batch. Adiz's supplied screenshots describe the prior v0.9.2 preview, not this exact EXE. A new visual playthrough remains the next human check.
