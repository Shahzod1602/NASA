# Graphics and comfort pass — 2026-10-29

This local iteration improves the existing Apollo scene; it does not replace
the astronaut model or claim photorealism.

## Changes

- The two existing shadowless fill lights are brighter, so equipment and suit
  panels remain readable on the shaded side of Intrepid. These are authored
  bounce-light approximations, not additional astronomical light sources.
- The sun's screen-space contact-shadow trace is disabled after a matched
  diagnostic isolated distant ground stripes to that effect. Ordinary shadow
  maps and ambient occlusion remain, as do sun direction, intensity, black sky
  and the imported elevation mesh. Shadow bias was tested, then left unchanged.
- Planted boots trace the ground at their locked position and orient their
  soles to the local slope, bounded to 25 degrees. The same pose drives the
  skinned suit. Swing and airborne poses remain separate; this is procedural
  animation rather than motion capture or physiological simulation.
- Gameplay settings add a saved Apollo tutorial preference, walking crosshair
  toggle and 50–100% HUD background opacity. H updates the same tutorial
  preference. Paused H still opens help. Menus keep their background contrast;
  camera framing and work cues are not hidden by the walking-crosshair option.

No map regeneration or binary-asset text editing is required. Scientific source
files, task coordinates and collision geometry are outside this change.
No new per-frame lighting or material processing is introduced; boot alignment
uses the existing two foot traces, repositioned after stance locking.

The local engine's Fab plugin descriptor changed during this run and referred
to an unavailable `FabToolset` module. The optional editor marketplace plugin
is explicitly disabled in this project's `.uproject` so builds can proceed.
No engine/plugin files were edited; gameplay assets do not require Fab.

## Reproduction

```powershell
& .\Tools\verify_frontend.ps1
& .\Tools\verify_apollo12.ps1 -Suit -Guidance -Polish -Regression
& .\Tools\review_realism.ps1 -Label graphics-after-20261029 -Quality 1
```

Before-image set: `Reports/Realism/graphics-before-20261029/` (packaged,
DX11, 1280×720, Medium). Runtime assertions include actual flat, 15-degree
side-slope and combined uphill/side-slope collision fixtures for both boot soles,
settings serialization, tutorial restart behavior, opacity bounds and
non-finite-input handling.

## Validation — 2026-10-29

UE 5.8.3 editor and Win64 Development targets compiled successfully. Cook,
stage and archive completed with ExitCode 0. Existing C4305 conversion warnings
remain; the build is not described as warning-free. Launch with
`Play Lunar Rescue.cmd` or `Dist/Apollo12/Windows/LunarRescue.exe`.

The final package passed **314 distinct assertions, 0 failures**: Apollo 57,
legacy Moon 50, Mars 54, menu travel 7, suit 17, polish 63, guidance 46 and
frontend 20. The suit suite exercises actual collision fixtures for flat,
side-sloped and combined-sloped ground. The gameplay assertions run with
NullRHI; they are not graphics-performance evidence.

Eight final packaged 1280×720 Medium rendered poses were generated and visually
inspected: hands, body, third-person, front, work, walk, stride and hop.
Compared opening views show readable shaded equipment and removal of the
screen-space ground stripes. Seven frontend screenshots were generated, and
the expanded Gameplay settings page was inspected. These are automated render
reviews, not a human playtest. Final poses: `Reports/Realism/graphics-final-20261029/`.

Unreal's shutdown can rewrite scalability settings even during automation.
Both `verify_frontend.ps1` and `verify_apollo12.ps1` now preserve the original
GameUserSettings bytes around the test process, including failure cleanup.
The diagnostic run's engine-written config was restored from a backup whose
hash matches this task's starting inventory. Preservation results are recorded
in `Reports/Accessibility20261029/preservation.json`.
Final hashes match all 82 inventoried player save/photo/settings files and
all 113 files in the raw science-source inventory. A repeated Apollo test and
frontend render review confirmed that the corrected wrappers preserve config.

### Matched opening performance

Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB RAM, DX11 rendered offscreen,
1280×720 Medium, no audio, 60 FPS cap. Each run captured 2,400 frames and excluded
600 warmup frames, leaving about 30 seconds. Neither build compilation nor
another game test ran concurrently with these captures.

| Measurement | Before | Final package |
| --- | ---: | ---: |
| Mean FPS | 59.96 | 60.00 |
| 1% low FPS | 51.68 | 56.11 |
| p99 frame time | 18.24 ms | 17.58 ms |
| Largest measured frame | 24.21 ms | 19.49 ms |
| Sampled peak process working set | 595.1 MiB | 601.9 MiB |

This single pair supports no claim of a general speedup or stable 60 FPS across
the complete mission, other settings or other hardware. Working set is not
VRAM. Raw captures, summaries and environment records are under
`Reports/Performance/graphics-{before,after}-20261029/`. Build/test logs are
under `Reports/Accessibility20261029/`.

## Remaining work

1. Refine the authored glove, suit-joint and torso silhouettes; these remain
   simplified procedural models, not scanned A7L garments.
2. Review Intrepid landing-pad contact and foil/material detail from more
   viewpoints before a dedicated model pass.
3. Run a real-player graphics/comfort test and longer matched performance
   captures at 1080p and High. Full-mission performance was not remeasured.

No new map, scientific elevation detail or NASA endorsement is claimed.
This iteration has not been committed, pushed or released.
