# Performance investigation — 29 October 2026

The next graphics step was to validate the reported frame-time drop before adding more geometry. This iteration changes the profiling tools, not the game executable or graphics settings. The previous 47 FPS captures are retained as observations; their cause is not established.

## Reproducible checks

Run from the repository in PowerShell, with other game/editor/build processes stopped:

```powershell
./Tools/profile_apollo12.ps1 -Label visible-check -Frames 3600 -Quality 1 -Onscreen
./Tools/profile_apollo12.ps1 -Label offscreen-check -Frames 2400 -Quality 1
./Tools/profile_full_mission.ps1 -Label mission-check -Quality 1
python Tools/test_summarize_profile.py
```

Use unique labels to retain previous evidence. Do not minimize the visible test window, capture screenshots, build, or launch another game while sampling. `-GpuTelemetry` is optional diagnostic instrumentation and is off by default; driver queries can perturb timing. Offscreen measurements do not validate display presentation. These scripts run automated traversal, not a human playtest.

Both profilers reject concurrent game/editor/shader-worker processes before launch and invalidate runs if they appear during sampling. Full-mission results must be freshly written and cover the mission duration. Settings are restored after execution. A completed fresh CSV and clean capture log are required; the packaged Unreal CSV shutdown can return a nonzero status, which is recorded rather than described as an ordinary successful exit.

## Measured configuration and results

Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB system RAM; packaged DX11, 1280 x 720, Medium (quality 1), 60 FPS cap, VSync disabled, sound disabled. The first 600 frames are excluded. 1% low is 1000 divided by the mean duration of the slowest 1% of sampled frames.

| Capture | Mean FPS | 1% low FPS | p99 frame ms | Largest frame ms |
| --- | ---: | ---: | ---: | ---: |
| Clean visible opening, 3,000 sampled frames | 59.99 | 53.49 | 18.19 | 23.76 |
| Repeat offscreen opening, 1,800 sampled frames | 59.98 | 49.20 | 18.69 | 33.51 |
| Full offscreen mission, 34,110 sampled frames | 59.97 | 57.02 | 16.70 | 111.87 |

Evidence: `Reports/Performance/perf-onscreen-clean-20261029/` and `perf-offscreen-repeat-20261029/`, each with CSV, log, summary and environment metadata. Visible-run peak sampled process working set: 648,331,264 bytes; this is not VRAM or total system memory.

## Interpretation and limitations

The same packaged binary returned approximately 60 FPS without a game-code change. This does not prove a permanent performance fix or identify the earlier 47 FPS cause. Hiding skeletal rendering previously reduced draw calls without restoring FPS, so there is not enough evidence to blame glove rendering alone.

The old `suit-before-20261029` run overlapped a suit-review process and is excluded as a clean baseline. `perf-current-20261029` included periodic GPU queries; `perf-onscreen-20261029` included a UI screenshot during sampling and its summary was recovered after a script argument bug. These instrumented runs are not the clean comparison results above. The one-element PowerShell argument-array bug was fixed and verified by the clean visible run.

Four summary tests cover constant frame times, an isolated hitch, an incomplete capture and non-finite/invalid frame values. No new game build is required for these Python/PowerShell changes. The earlier 359 packaged assertions belong to the suit iteration and were not rerun here.

1080p, High quality, audio-enabled performance and human playability remain unmeasured in this iteration. NASA elevation data, the mission resource model and suit geometry are not altered. No push or release is performed.

## Full-mission acceptance

`perf-full-mission-20261029` completed with zero failures: 576.75 seconds, 1,238.04 metres, three samples, all three hardware parts and both ALSEP tasks. The 34,710-frame capture covers the complete run; 600 warmup frames were excluded. Peak sampled process working set was 663,396,352 bytes. The 111.87 ms maximum frame means occasional hitches remain despite the near-60 average. This was rendered offscreen with fast-stride traversal and sound disabled.

Both overlap guards were exercised while the full mission was running and rejected a second launch. Both PowerShell scripts parsed without errors; the four Python tests passed, and `git diff --check` passed. User save/photo/config hashes matched the pre-mission inventory; newly generated mission photographs were archived under `Reports/PerformanceInvestigation20261029/generated-mission-photos/`. The executable SHA-256 still matches the final suit package (`ef0633a2145aee16c0de1fed07ccf5fda3c373375dc934070d7e5f30c0c0d043`). Preservation details are in `Reports/PerformanceInvestigation20261029/preservation.json`.

Next: measure a human 1080p session with audio; investigate the occasional long frame with targeted capture; then refine Intrepid materials/foot contact without increasing uncontrolled visual load.
