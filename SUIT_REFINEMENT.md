# Glove and garment refinement — 2026-10-29

This iteration focuses on the first-person gloves and shared astronaut suit.
It is project-authored visual work, not a scan or a historically exact A7L model.

## Changes

- Tapered fingers with separate lengths, rounded ends, dorsal knuckle pads,
  palm grip reinforcement and sewn edges. Small details use a bounded mesh
  resolution rather than the full-resolution body primitive.
- Shaped sleeves, layered shoulder gussets and knee reinforcement replace
  the conspicuous spherical joint caps. Shallow end gussets close exposed
  limb openings when looking down in first person. Existing cloth and hardware materials
  remain shared, with three skeletal LODs rebuilt by Unreal.
- Cooked first-person skin bounds now cover the camera-space posing envelope.
  The imported bind-pose box was centered at x=6 cm while animated hands
  moved to x=36 cm and beyond, causing packaged-build frustum culling. This
  bounded, shadowless envelope is initialized once, without a new Tick or scan.
- First-person resting hands sit in the lower view instead of entirely below
  it. Relaxed fingers curl slightly; active grips close more, with separate
  thumb opposition and index-finger motion.
- The primary hand rotates with the visible tool; its palm center remains on
  the current-frame grip anchor. Tool tips and science interaction coordinates
  remain driven by their existing contact system.
- Tiny authoring-space triangle normals are normalized without the previous
  excessive epsilon, which could shorten normals on small glove details.

No external models, textures, motion capture or additional per-frame traces
are introduced. Original project geometry and procedural code were refined
with OpenAI Codex assistance. Scientific terrain, raw NASA data and map assets
are outside this iteration. Previous local graphics and comfort work remains.

## Rebuild and review

Run `python Tools/prepare_suit.py`, then `python Tools/prepare_suit_rig.py`.
Run `Tools/import_suit_refinement.py` through Unreal's Python commandlet to
import the seven changed static parts, both skins and three LODs while retaining
existing shared materials. Do not edit `.uasset` files as text.

Source checks: `python Tools/verify_suit_geometry.py` and
`python Tools/verify_polish_sources.py`.

Runtime checks: `Tools/verify_apollo12.ps1 -Packaged -Suit -Fieldwork -Regression`.
Visual review: `Tools/review_realism.ps1 -Packaged -Label suit-final-20261029 -Quality 1`,
plus `Tools/verify_apollo12.ps1 -Packaged -FieldworkReview` for physical grips.
The suit review retains the HUD, so it shows the player's actual first-person
composition. Its old packaged opening frame culled the gloves, and its work frame
opened the field-kit panel rather than a held tool; fieldwork captures are
required for checking grips.

## Validation — completed locally

UE 5.8.3 Editor and Win64 Development compiled, cooked and staged successfully
(`package-bounds-final.log`, UAT ExitCode 0). Existing conversion/deprecation
warnings remain. An initial bounds API attempt failed because the setter was
private; the final code uses the public bounds-extension APIs.

**359 distinct final packaged assertions passed; zero failures:** Apollo 57,
legacy Moon 50, Mars 54, menu 7, suit 23, polish 63, guidance 46, fieldwork 39,
frontend 20. Checks include visible-skeleton digit lengths, grip rotation and
contact, cooked bounds containing both gloves, pause, evidence-photo hiding
and restoration, plus the existing slope/mission tests. Most run with NullRHI;
they are gameplay evidence, not graphics benchmarks.

Both source validations passed: finite unit normals, nondegenerate triangles,
bounded vertex counts, distinct finger pivots, normalized skin weights and six
manifest-verified science files. Authoring vertex counts changed from 118,818
to 117,418 for the suit and 26,352 to 19,466 for the hand skin. These counts do
not imply an FPS improvement.

Eight final packaged suit views were visually reviewed under
`Reports/Realism/suit-approved-20261029/`. Twelve final fieldwork images were
generated; contact, lift, core and cable frames were visually inspected and
copied into `Reports/Suit20261029/`. No human playtest was performed.

All 195 baseline files (82 player saves/photos/settings and 113 raw science
files) retain their original hashes. Six photos created by the existing
fieldwork review were moved, individually and with hashes recorded, into
`Reports/Suit20261029/generated-review-evidence/`; original player photographs
were not moved or overwritten. No map asset or scientific terrain changed.

### Performance — an unresolved limitation

Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB RAM, DX11 rendered offscreen,
1280x720 Medium, no audio, 60 FPS cap. Each capture contains 2,400 frames;
600 startup frames are excluded, leaving 1,800 sampled frames.

| Capture | Mean FPS | 1% low FPS | p99 frame ms |
| --- | ---: | ---: | ---: |
| Earlier pre-suit reference, `graphics-after-20261029` | 60.00 | 56.11 | 17.58 |
| Final suit, `suit-after-20261029` | 46.96 | 31.86 | 31.01 |
| Same final build repeated, `suit-confirm-20261029` | 46.84 | 32.06 | 30.82 |
| Diagnostic only, skeletal rendering disabled | 47.07 | 31.64 | 30.73 |

The new `suit-before-20261029` capture overlapped a render review for its last
approximately 12 seconds, so it is excluded as a clean baseline. The earlier
reference used the same documented hardware, route and settings, but was
recorded about two hours earlier; power/thermal/background conditions were
not controlled. The final captures ran after the build and other game tests
had exited.

The repeated final result is about 47 FPS, not stable 60 FPS. The diagnostic
reduced average draw calls from 117.12 to 109.00 without recovering frame rate.
This suggests visible skeletal drawing is not the principal cause of the
observed difference; it does not prove the entire change is performance-neutral
or identify the actual bottleneck. No unrelated quality settings were lowered.
The attempted external GPU-clock logger produced no samples and is excluded.
Raw CSVs and environment metadata remain in `Reports/Performance/`.

Use `Tools/profile_apollo12.ps1 -HideSkeletalMeshes` only for this diagnostic;
ordinary launch renders the gloves normally. This does not change saved user
preferences. Full-mission, 1080p/High and interactive frame pacing were not
remeasured. Further profiling is needed before claiming a 60 FPS target.

## Limits

The model remains stylized and procedurally animated. This is not a historical
suit reconstruction or an ergonomics simulation. Intrepid materials, complete
animation transitions and nearby terrain decoration remain separate stages.

## Next stages

1. Resolve the measured frame-time difference with controlled on-screen and
   full-mission profiling before increasing visual load.
2. Refine Intrepid foot contact/materials, followed by walk/stop/turn/landing
   transitions; review each stage in the packaged game.
3. Add bounded nearby authored regolith/rock detail while preserving the NASA
   elevation source and geographic coordinates.

Launch the current local package with `Play Lunar Rescue.cmd` or
`Dist/Apollo12/Windows/LunarRescue.exe`. No commit, push or release was made
for this iteration.

## Follow-up performance investigation

See [PERFORMANCE_UPDATE.md](PERFORMANCE_UPDATE.md) for later controlled captures. The earlier numbers above remain historical observations; the unchanged binary subsequently returned approximately 60 FPS.
