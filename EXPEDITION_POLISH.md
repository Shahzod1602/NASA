# Expedition presentation and feedback — local development, 29 October 2026

This iteration develops the Apollo expedition's Intrepid presentation, movement
transitions, nearby regolith detail, work feedback and mission debrief. It also
prepares a novice playtest workflow. The scope below records implemented source/asset changes and the verified
Windows results. Human novice sessions remain unperformed.

## Intrepid contact and materials

`LunarLander.cpp` retains the existing NASA/Michael D. Carbajal reference mesh
and its existing approximately 698 cm height normalization. Four foot-pad
centres were derived from that model's local geometry. They are authored model
pivots, not measured touchdown coordinates.

The new pass traces only the existing `SM_Apollo12Terrain` collision component,
fits a visual support plane through the four contacts and adjusts the reference
model's orientation and vertical position. Its mission X/Y and uniform scale
remain unchanged. The pass refuses to move an effectively enabled collider;
all interaction actors and work-site coordinates remain in their existing
positions. Missing traces, a support tilt above 12 degrees or a vertical
correction above 80 cm cause a logged skip rather than invented terrain values.
These bounds protect presentation stability; they do not define astronaut or
lander safety.

Four thin, ground-aligned cosmetic soles and bounded shallow collars bridge
the rigid imported mesh's residual mismatch on the uneven 2 m terrain. The sole
cylinder centres sit 0.9 cm above their traced planes, with 1.8 cm total thickness.
This is a visual approximation, not articulated landing-gear physics. Foil
seams, blanket edges, equipment plates and fasteners reuse existing basic meshes
and materials. The retained untextured ladder/strut surface receives the existing
brushed-metal material; existing textured panels and thermal blanket remain.

The additions use six HISM batches, with collision, navigation contribution and
shadows disabled and a 70 m end-cull distance. No per-frame lander updater is
added. Repeated initialization is guarded by `Apollo.LanderDetailBuilt`.
Diagnostic output begins `APOLLO_LANDER_DETAIL`. Presentation tests identify
the auxiliary actor by `Apollo.LanderDetail` and the four sole instances by the
component tag `Apollo.LanderContact`.

## Nearby surface detail

`LunarSurface.cpp` retains the existing 9,000 placement-attempt cap and six
collision-free HISM batches, with an end-cull distance no greater than 42 m.
It redistributes 32% of attempts toward work-site surroundings and gives part
of that subset a finer fragment size. It does not increase the attempt budget
or add a runtime scatter Tick.

Fragments align to the imported terrain normal instead of a flat world plane.
Terrain-only component traces avoid placing debris on equipment tops. Clear
foregrounds remain around work targets, Surveyor and the lander. The actual
number accepted depends on the collision traces; logs record the count rather
than claiming that every attempt created a fragment. An initialization guard
also prevents repeated fill-light amplification in the same world.

The fragments, their material variation and small-scale surface appearance are
authored content. They are not additional NASA measurements or evidence that
the source DTM resolves centimetre-sized stones. No scientific terrain vertices,
collision mesh, raster, geographic coordinates, source metadata or map assets
are rewritten by this presentation pass.

## Movement and comfort

`LunarSuit.cpp` smooths gait speed, turning follow-through, working blend and
landing compression using bounded exponential presentation filters. The code
contains checks for comparable 30/60 Hz filter response, invalid delta values,
settling after stopping and the existing photograph hand-hiding behaviour.

The visual torso and hand offsets do not add camera sway or move the gameplay
capsule. The existing hand/camera-motion preference suppresses decorative hand
landing movement. Normal pause ticks keep the presentation still. This remains
procedural character animation; it is not a new motion-capture set, a changed
gravity model or a physiological simulation.

## Progress, recording and contact cues

Accepted intermediate tool input now produces a short `IN PROGRESS` description
of the actual step: lifting a rock, moving soil, positioning/hammering a core,
releasing a fitting or cutting a cable end. The text distinguishes work in
progress from cargo that has actually been sealed or packed.

Committed work uses `RECORDED`; pending or failed photograph messages use
`STATUS`. Intermediate feedback does not grant samples or hardware. Rapid input
cannot bypass the existing cooldown, pause holds feedback and resource timers,
and cancellation clears transient feedback without recording unfinished work.
Starting a new attempt clears these presentation facts.

The short contact cue reuses the existing `PowerContact` sound, with tool-specific
pitch/gain and the saved master/effects controls. It represents authored
suit/tool feedback. It does not simulate sound propagating through the lunar
vacuum. No new third-party sound library is introduced.

## Debrief tied to recorded state

`LunarDebrief.cpp`, the result HUD and the evidence panel report documented
sample sites separately from packed samples and recovered Surveyor parts.
Optional coating photographs do not imply that their comparison was completed.
The result distinguishes untimed exploration from the gameplay exercise budget;
the latter is not described as suit endurance. Displayed budget values are
bounded and handle negative/non-finite values.

Next-attempt advice follows actual incomplete cargo, the measured exercise
balance and optional-study completion. It does not invent a chosen route or
claim that steep terrain caused a delay when those facts were not recorded.
The debrief also explains the role of the existing LROC elevation data and
identifies the lander detail, small fragments and tool motions as authored.

## Human playtest preparation

`PLAYTEST_GUIDE.md` prepares 3–5 novice sessions with three questions before and
after play, independent participant installations, unassisted milestones,
completion, assistance, confusion, readability and comfort observations.
`Tools/fixtures/expedition-playtest-form.md` is a blank session form;
`Tools/start_playtest.ps1` prepares a separate installation from a selected clean
archive or the current local `-PackageRoot`, without replacing an existing
participant folder. `-PrepareOnly` copies clean runtime files without launching
the game; Saved/Reports, debug files and manifests are excluded.

This is preparation only. No participants have been recruited, no human sessions
have been run and no learning/usability results are claimed. Automated checks
and staged screenshot fixtures must never be entered as participant evidence.
The forms remain blank until a real session supplies observations.

## Reproduction commands

Run from the repository in PowerShell. The standard local engine root is
`D:\UnrealEngine\UE_5.8`; the existing package script accepts `-EngineRoot`
when a different installation is required.

```powershell
./Tools/package_apollo12.ps1
./Tools/verify_presentation.ps1
./Tools/verify_presentation.ps1 -Graphics -Audio
./Tools/verify_presentation.ps1 -Packaged
./Tools/verify_presentation.ps1 -Packaged -Graphics -Audio
./Tools/verify_presentation.ps1 -Packaged -Review -Width 1280 -Height 720
./Tools/verify_apollo12.ps1 -Packaged -Regression -Suit -Polish -Guidance -Fieldwork
./Tools/verify_frontend.ps1 -Packaged
./Tools/verify_loading.ps1 -Packaged -Graphics
```

`-Packaged` selects the staged Windows executable. Without it, the script uses
the local Editor commandlet. `-Graphics` selects DX11 rendered offscreen;
without it, NullRHI is a functional test, not a graphics-performance result.
`-Audio` enables the real contact-audio gain/pitch assertion and requires a
working audio environment. The script preserves and restores engine-level
GameUserSettings, bounds its owned process and requires a fresh semantic result.

`-Review` produces eight rendered synthetic scenes for Intrepid, foot contact,
regolith, tongs and success/failure/large-text debrief layouts. The adjacent
`presentation-fixtures.json` explicitly marks staged cargo/time/distance values.
They are layout fixtures, not a played mission or human test. The review avoids
capturing in-game evidence photographs or reusing the player's photos.

The completed comparison used these commands against the respective baseline and candidate packages. For a new comparison, use fresh labels and keep hardware, resolution, graphics, route, frame cap and audio identical:

```powershell
./Tools/profile_apollo12.ps1 -Label expedition-before-20261029 -Frames 3600 -Quality 1 -Width 1280 -Height 720 -FpsCap 60
./Tools/profile_apollo12.ps1 -Label expedition-after-20261029 -Frames 3600 -Quality 1 -Width 1280 -Height 720 -FpsCap 60
```

Do not build, cook, run another game/editor or take screenshots during a clean
sample. Keep GPU telemetry optional because polling can perturb timing. Record
hardware, settings and the actual capture limits. A near-60 average from an old
build is not proof of this iteration's frame pacing or memory use.

## Verified results - 29 October 2026

Unreal Engine 5.8.3, MSVC 14.44 and Windows SDK 10.0.22621.0; Windows
Development package. Editor and game compilation succeeded. The complete
cook/stage/archive finished with AutomationTool ExitCode 0. The initial Editor
compile caught pointer/type, shadowed-name and checked-format issues; those
were repaired before accepting the succeeding build.

| Packaged checks | Passed | Failed |
| --- | ---: | ---: |
| Apollo, legacy Moon, Mars, selection, suit, polish, guidance, fieldwork | 344 | 0 |
| Scripted normal-movement walkthrough | 1 | 0 |
| Checkpoint validation, write and fresh-process restore | 60 | 0 |
| New presentation checks, silent | 33 | 0 |
| New presentation checks, rendered with audio | 34 | 0 |
| Frontend | 20 | 0 |
| Rendered loading/Continue/restart | 20 | 0 |
| Total | 512 | 0 |

The walkthrough completed 896.81 **simulated** seconds and 1237.50 recorded
horizontal metres, with 3/3 samples, hardware mask 7 and ALSEP mask 3.
It uses fixed-step accelerated headless execution; it is not a human session
or a rendered full-mission FPS measurement. Required work finished without
requiring the optional coating comparison.

Eight packaged presentation scenes, eight suit scenes and twelve fieldwork
scenes were generated. The lander contact, work-grip, result/debrief and large
text frames were inspected. Static poses show visible gloves and attached
primary tool grips; they do not prove complete animation smoothness or
absence of transient clipping over time. The debrief scenes contain declared
synthetic fixture data, not player or educational-study results.

Observed startup detail: four contacts, six lander HISM batches/42 instances,
698 cm normalized model height; 8735 accepted cosmetic surface fragments
(including 821 fine fragments) within six batches and the 9000-attempt cap.
The sampled support-plane tilt is 7.18 degrees and rigid-model contact residual
up to 9.45 cm. These describe the authored adaptation to the imported collision
surface, not the historical spacecraft touchdown attitude or measured gear
behaviour.

### Material and large-text defects found during review

The first rendered review exposed missing InstancedStaticMeshes usage on four
existing materials. `Tools/prepare_presentation_materials.py` persists that flag
with the supported `MaterialEditingLibrary.set_base_material_usage` Editor API
on M_EquipmentDark, M_EquipmentPaint, M_LunarFoil and M_Metal. It saves only
those base materials and never edits binary text or maps. The package binding
script invokes this preparation before cooking. Subsequent rendered logs have
no missing-usage/fallback warning; the saved report is `material-usage.json`.

The first 150% debrief frame exposed auto-wrap rows overlapping. Evidence rows
now receive an explicit viewport/DPI-aware wrap width before layout, button
captions stay on one line, and width changes trigger one cached panel rebuild.
The replacement 100/150% frames show separated rows and scrollable overflow.

### Matched opening-route performance

Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB RAM; DX11, 1280 x 720, Medium,
60 FPS cap, VSync disabled, no sound, rendered offscreen. Both captures contain
3600 frames; startup 600 excluded, 3000 sampled (about 50 seconds). Other
build/editor/game processes were absent during the after capture. The
unrelated engine build that delayed compilation started after the baseline
capture, not during it. No screenshots or GPU driver queries were taken while
measuring.

| Measurement | Before | After |
| --- | ---: | ---: |
| Mean FPS | 59.993 | 59.997 |
| 1% low FPS | 55.750 | 56.552 |
| p99 frame ms | 17.578 | 17.522 |
| Largest sampled frame ms | 19.974 | 18.048 |
| Sampled process working set bytes | 628178944 | 624570368 |

Evidence: `Reports/Performance/expedition-before-20261029/` and
`expedition-after-20261029/`. This one capped pair supports near-60 operation
in the measured opening, not a proven speedup, stable full-mission presentation
or other resolutions/hardware. Sampled working set is neither true peak memory
nor VRAM. NullRHI assertions are not graphics-performance evidence.

### Preservation and prepared human workflow

All **234** inventoried original save/photo/config/science files retained their
SHA-256 values. Two newly generated, log-attributed review photographs were
archived individually under `generated-test-evidence/`; original player images
were not moved. The packaged executable matches the compiled game byte for
byte; its checksum and all case counts are in
`Reports/ExpeditionPolish20261029/validation.json`.

The playtest helper rejected an empty runtime before copying. A separate P08
smoke installation was prepared without launching; it excludes owner data,
contains the matching executable and cannot be overwritten by a second call.
P08 preparation is not a participant session. Forms remain blank and real
human playtests remain **not run**. No third-party invitations were sent.

`git diff --check`, supported material preparation, config restoration and
clean process exits passed. No remote push or release publication was made.

## Preservation, credits and limits

Existing loading-screen work remains part of the local working tree. The
presentation changes do not require map regeneration or direct binary-asset
editing. Normal player checkpoints, photographs and settings are to be preserved;
automated test/review flags isolate synthetic state, and packaging retains the
existing player-data backup/restore workflow. The completed comparison checked the original files rather than assuming
preservation from flags alone.

The existing code and third-party asset licences are unchanged. The lander,
materials, rock meshes and contact sound are reused under their existing credits;
this iteration adds authored code/geometry arrangement, not a new asset licence
or external asset download. OpenAI Codex assisted implementation, code review and
documentation. Human review and the recorded verification results remain the
basis for accepting the work. No NASA endorsement, new historical reconstruction
claim or professional astronaut-training claim is made.

No remote push, release publication or competition submission is part of this
iteration without separate instruction.

Remaining presentation limits: the suit and motion remain stylized/procedural.
Rounded look-down thigh caps and close-view forearm proportions remain visible.
An extreme third-person stride may meet the bottom HUD strip; still review
is not continuous motion or human comfort testing. The highest-priority next
validation is actual novice play with the prepared anonymous forms.
