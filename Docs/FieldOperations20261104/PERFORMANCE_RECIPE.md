# Field operations performance recipe — 2026-11-04

Status: prepared during the package build. No post-build capture was run by this document author. Record final executable SHA-256 and source receipt before assigning any new result to a binary.

## Verified baseline

Hardware recorded by the fresh baseline: AMD Ryzen 5 6600H, NVIDIA GeForce RTX 3050 Laptop GPU (AMD 660M also enumerated), 16 GB installed RAM / 16,312,721,408 bytes reported. Unreal Engine 5.8.3. Adapter enumeration alone is not proof that a particular adapter rendered a run; retain the D3D11 selected-adapter log line.

Both baseline captures use DX11, rendered offscreen 1280×720, quality 1 (Medium), normal delta time, 60 FPS cap, VSync off, no sound, 1,800 CSV frames. Menu analysis excludes the first 300 frames; Apollo opening-route analysis excludes 600.

| Fresh before workload | Mean FPS | 1% low FPS | p99 frame ms | Maximum sampled process working set |
| --- | ---: | ---: | ---: | ---: |
| Static home menu | 60.002521 | 53.633989 | 17.4532 | 580,935,680 bytes |
| Apollo opening walking route | 59.929324 | 50.347186 | 17.6571 | 632,152,064 bytes |

Raw baselines: `Reports/MissionSelection20261031/performance-fieldops-20261104-before/{environment.json,summary.json,capture.csv,capture.log}` and `Reports/Performance/fieldops-20261104-before/{environment.json,summary.json,capture.csv,capture.log}`. Menu environment records executable SHA-256 `5171c1b0a6142dfa4c35e373d731d441ce88eb8d7a41f768779bbeaded52f2f0`; Apollo environment does not independently store an executable hash, so do not invent one from its path.

## Matching after captures

Run these sequentially only after all engine, game, shader compiler, C++ compiler, UBT and packaging processes finish. Use fresh labels. Keep the laptop power/thermal state and graphics configuration comparable; record any change.

```powershell
Set-Location D:\LunarRescue
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tools\profile_mission_menu.ps1 -Label fieldops-20261104-after -Frames 1800 -Quality 1 -Width 1280 -Height 720 -FpsCap 60
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tools\profile_apollo12.ps1 -Label fieldops-20261104-after -Frames 1800 -Quality 1 -Width 1280 -Height 720 -FpsCap 60
```

These existing helpers preserve/restore the config files they explicitly handle. The Apollo helper has no arbitrary extra-launch-argument parameter. `-DiagnosticCommands` is for console commands and cannot enable the command-line `FieldOperationsProfile` flag.

The existing Apollo helper passes `ApolloWalkTest`, `ApolloSaveProfile` and `csvCaptureFrames`. These make `ULunarSettings::IsAutomation()` true, and `UFieldOperationsComponent::Initialize()` consequently disables the extension unless a new field-operations automation flag is present. Therefore this matching before/after walking comparison measures the old route and shared-code regression; it does **not** measure the cost of the new support actors, magnifier, occupied vehicle or preparation UI. The home menu is also not a field-operations gameplay workload.

## Separate extension-enabled opening-route capture

Exact additional flag: **`-FieldOperationsProfile`**. It enables the extension despite CSV/test automation and explicitly supplies prepared/briefed/inspected state so the existing walking route is not stopped by the new preflight panels. The following is a command recipe, not an executed result. Run in an isolated prepared local installation or preserve its relevant Saved config and automated checkpoint files before launching; do not run the helper concurrently. Use a fresh output directory.

```powershell
$gameRoot = 'D:\LunarRescue\Dist\Apollo12\Windows\LunarRescue'
$exe = Join-Path $gameRoot 'Binaries\Win64\LunarRescue.exe'
$report = 'D:\LunarRescue\Reports\Performance\fieldops-20261104-enabled-opening'
if (Test-Path -LiteralPath $report) { throw 'Choose a fresh capture label.' }
New-Item -ItemType Directory -Path $report | Out-Null
$log = Join-Path $report 'capture.log'
$captureArgs = @(
 '/Game/Lunar/Maps/Apollo12', '-game', '-unattended',
 '-ApolloWalkTest', '-ApolloSaveProfile', '-FieldOperationsProfile',
 '-dx11', '-RenderOffscreen', '-ForceRes', '-ResX=1280', '-ResY=720',
 '-ApolloQuality=1', '-nosound', '-ExecCmds="t.MaxFPS 60,r.VSync 0"',
 '-csvCaptureFrames=1800', '-csvGpuStats', '-ExitAfterCsvProfiling',
 "-abslog=`"$log`""
)
$started = Get-Date
Get-FileHash -LiteralPath $exe -Algorithm SHA256
$p = Start-Process -FilePath $exe -ArgumentList $captureArgs -WindowStyle Hidden -PassThru
```

The recipe deliberately does not reuse `profile_apollo12.ps1` because it cannot pass that flag. The runner must sample the child process working set, capture the exact argument list/hardware/selected adapter, wait for only its own process, require a fresh CSV and `Capture Ended` without fatal/assert/material errors, and validate 1,800 frames with `Tools/summarize_profile.py` (600 warmup frames, cap 60). Record raw process exit unchanged: Unreal CSV auto-exit may be 777003; a complete validated capture is not a functional-test exit-0 assertion. Restore any preserved config/checkpoint bytes afterward.

`ApolloWalkTick` drives the on-foot pawn toward native objectives; it never boards the rover. Thus this enabled capture measures the walking opening route with parked rover/support actors and field bookkeeping, **not occupied-rover driving, scanner operation, the resource board, or the full new interaction loop**. Compare it with the same final binary, disabled extension, as a separate workload delta; do not call it an old-version equal-workload before/after result.

An occupied-rover workload still needs an explicit, observed mount/drive/steer/stop/exit capture with distance and charge telemetry and a camera route recorded. An inspection workload needs the magnifier actually open and scanning real visible package targets. `FieldOperationsReview`/`FieldOperationsTest` change fixtures and exit schedules and are functional/render evidence, not a normal-delta fixed performance route. No such driving/scanning performance result is claimed here.

## Reporting

Retain executable SHA-256, source/build receipt, full argument list, fresh log/CSV, requested and observed frame count, discarded warmup, mean FPS, 1% low, p95/p99/max frame time and sampled process working set. Express before/after changes with raw values as well as percentages. One controlled capped offscreen run does not prove stable 60 FPS, full-mission throughput, on-screen smoothness, VRAM use, or human usability. NullRHI is never a graphics performance result.
