# Private reference-art candidate test report — 14 November 2026

This report certifies the historical private 7c7af402 candidate only. The public v0.9.2 distribution has original text fallbacks and excludes unconfirmed media; see ../PublicRelease20261114/PUBLIC_TEST_REPORT.md for its separate validation.

Windows 11 x64; Unreal Engine 5.8.3; VS 2022 toolchain 14.44.35217; SDK 10.0.22621.0. Ryzen 5 6600H, RTX 3050 Laptop 4 GB, approximately 16 GB RAM.

Base: `53dce53533ee00a25176c9b3a03e919503472016`. The worktree is uncommitted. This is a local Development build, not a new GitHub release. Inner EXE SHA256:
`7c7af4021b631dc5467bf08724cae34e589136f945ea71b74647c5c628923038`.

## Final Windows results

| Executed command | Result |
|---|---|
| `Tools/verify_rescue_feedback.ps1 -Packaged` | 126 PASS: SELENE primary 50, Mars primary 54, Moon recorder 3, missing-mesh Moon recorder 3, Mars movement 13, Mars recorder 3. Includes full power 0→3 and optional-recorder access with actual movement input. |
| `Tools/verify_field_operations.ps1 -Packaged -Mission all` | Apollo 240, SELENE 261, Mars 243 = 744 PASS. EVA sequencing, pause/resource guards, cable verification, actual module pickup and beacon cleanup, eight character-capsule approaches, rover entry/cameras/driver and state reset; boarding preserves existing mission radio without opening an instruction panel. |
| `Tools/verify_gameplay_tasks.ps1 -Packaged -Mission all` | Apollo 237, SELENE 198, Mars 183 = 618 PASS. |
| `Tools/verify_task_fixes.ps1 -Packaged -Suite stride` | 19 per destination = 57 PASS through actual input handlers. |
| `Tools/verify_mission_selection.ps1 -Packaged` | 66 PASS; browsing/settings preserve state and selected travel works. |
| `Tools/verify_nasa_tasks.ps1 -Packaged -Intro` | 12 PASS: actual AstroKid asset, input gating, automatic completion and once-per-process intro. |
| `Tools/verify_field_operations.ps1 -Packaged -Review -Mission selene` | 105 assertions / 41 fresh PNGs, all passed; 100/150% text, native UI callbacks, globe rotation including poles, production rover cameras, actual pickup before station views and close status-lamp view. |
| `Tools/verify_gameplay_tasks.ps1 -Packaged -Review -Mission selene` | 27 assertions / 27 PNGs plus paired carried-cell pixel contribution checks passed. |
| `Tools/verify_nasa_tasks.ps1 -Packaged -Intro -Review` | 7 fresh PNGs passed. Startup ready phase: 0.358 s, rendered=1, monotonic 100%, 3 zero frames, 14 ready frames, world held. This excludes earlier process/engine initialization. |
| `Tools/verify_loading.ps1 -Packaged -Graphics -Intro` | All 11 travel phases passed. Both complete Moon clips reached EOF after 30.241 / 30.106 s; world clocks held. Audio enabled; no human hearing audit. |
| Missing optional video review | Temporarily moved only the verified staged movie, ran the same rendered selected-travel review with `ReviewMissionIntro`, observed exactly two direct missing-media entries and no playback attempt, then restored the original SHA-verified 3,506,824-byte file in `finally`. All travel phases passed. |

Native total excluding loading-phase assertions: **1,623**. Graphics output: **75 fresh PNGs**, plus complete-video and missing-media rendered runs. These are synthetic checks and model inspection, not human playtests. Exact receipt paths and hashes are in `VALIDATION_EVIDENCE.json`; raw logs remain under `Reports/Feedback20261114Evening` and the referenced dated suite folders.

## Build and import evidence

- Static model-data checks passed (`model-checks.log`): all 464,116 source rover triangles retained; 12,900 stationary mudguard triangles assigned to body, measured tire centres and derivative checksums verified. This does not itself certify rendering.
- Supported Editor import produced 29 dated rover assets; `Tools/import_startup_feedback.py` imported two dated UI textures without map writes (`startup-assets-final.log`, 0 errors/warnings).
- Editor build: successful final synchronization (`editor-build-synchronized.log`, 65.03 s).
- Game build: 41 actions / 876.85 s (`game-build-final.log`), followed by successful synchronization incorporating quiet boarding and its native regression (`game-build-synchronized.log`, 96.64 s).
- Cook: BuildCookRun succeeded (`cook-final.log`, 2m51s AutomationTool). Stage/archive: succeeded (`stage-final.log`, 48 s). Stage preserved 53 owner files in a separate dated backup.
- Reproducible build: use `Build.bat LunarRescueEditor Win64 Development -Project=D:/LunarRescue/LunarRescue.uproject -WaitMutex -MaxParallelActions=1 -NoUBA -NoPCH -NoSharedPCH` and equivalent `LunarRescue` Game target with `-NoDebugInfo -NoLinkerDebugInfo`. The successful constrained-host .NET heap caps were 384 MB for Editor and 1 GB for Game. Cook all four existing maps, single cooker process, then `Tools/package_apollo12.ps1 -StageOnly`. Do not run its unrelated asset-regeneration steps over an edited map.
- Launch: `D:/LunarRescue/Dist/Apollo12/Windows/LunarRescue.exe`.

## Matched-recipe graphics sample

`Tools/profile_apollo12.ps1 -Label feedback-20261114-after-quiet -Frames 1200 -Quality 1 -FieldOperationsProfile -FpsCap 60 -GpuTelemetry` matches the earlier `feedback-20261114-before` recipe. DX11 offscreen, 1280×720, Medium, 60 FPS cap, normal delta, scripted Apollo opening walk with parked support actors. 1,200 frames captured; first 600 excluded; 600 analyzed, approximately 10 seconds. Hardware above. No occupied driving, scanning or intro included.

| Metric | Before v0.9.1 | Final quiet-boarding candidate |
|---|---:|---:|
| Mean FPS | 59.9971 | 59.9852 |
| 1% low FPS | 55.9818 | 54.0333 |
| p95 frame ms | 17.2424 | 17.5618 |
| p99 frame ms | 17.5818 | 17.9458 |
| Largest frame ms | 18.4727 | 19.4242 |
| Sampled peak working set, bytes | 665,075,712 | 664,985,600 |

The capped mean is comparable; the tail was slightly worse in this short sample. This does not establish a causal improvement/regression or full-game “60 FPS stable.” Background processes were not fully controlled. Profiling exit 777003 is the capture utility's accepted ExitAfterCsvProfiling path. Four GPU samples were collected; no precise VRAM conclusion is drawn. CSV/environment/summary evidence is retained in `Reports/Performance/feedback-20261114-before` and `feedback-20261114-after-quiet`. The earlier `after` run belongs to the prior 612ae09e candidate and is not the final measurement.

## Preservation and limits

All **53 original save/photo/config files** retain their SHA256 hashes. All four source maps exactly match current HEAD's LFS object hashes (`final-integrity.json`). A temporary tag-only Editor map save was archived and replaced with its verified pre-change copy; final import tooling has no map writes. Existing dated rover assets remain available. Terrain data, module/power/dish anchors and historical Git dates were not rewritten.

Earlier interrupted/failed compiles, the first failed mast-support check, earlier camera/intro decoder experiments and earlier candidate receipts remain as historical evidence and are not counted as final passes. Existing Reset-hiding and float-conversion warnings remain.

Human playtests: **0**. No OS network-disconnection test, human audio hearing audit, full-route occupied-rover performance result, 1080p review for this batch or pixel-perfect reference-art certification. UI reference layouts are adapted to existing styling and actual available mission items; arbitrary reference kilometres, consumable counts and fictional medkits were not copied into gameplay. NASA/partner montage is excluded pending factual partner confirmation. Original supplied video/art source and redistribution details remain unconfirmed; the movie is excluded from Git. Public release readiness and competition eligibility are not certified by these code tests.

## Next three checks

1. Confirm original artwork/video sources and factual partners before public distribution or activating the partner montage.
2. Have Boburjon review the executable against the visual references; run real-user playtests when participants become available.
3. Profile a full mission with occupied driving/scanning and review cockpit visibility, animation and UI at 1080p before making wider performance claims.

## Quiet-boarding final review
The automatic training-rover instruction radio was removed after the first candidate review. A native assertion now requires a successful board to preserve the preceding mission radio string, alongside no active Vehicle Menu. Both targets were rebuilt, recooked/staged and all final suites in the table rerun on the EXE identified above. Existing mission radio and optional pause/help controls remain available.

## Team handoff boundary
A team message at 2026-11-14 17:10:57 UTC (22:10:57 Asia/Tashkent) requested playtest feedback and no changes after submission. The quiet-boarding code edit predates that message; later operations were validation, documentation and task-status replies. This session made no GitHub push, release or competition submission. The actual submitted artifact/version has not been verified here; do not label this local candidate as the submitted build without team confirmation.
