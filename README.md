# Lunar Rescue

## v0.9.1 — Hand Pose Fix — 14 November 2026

[Windows release](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.9.1) · [Download Windows ZIP](https://github.com/Shahzod1602/NASA/releases/download/apollo12-v0.9.1/LunarRescue-Expeditions-v0.9.1-Windows.zip).

Fixes the sideways palm/finger adaptation and overlapping legacy forearms reported in v0.9.0. Extract the entire ZIP and run **LunarRescue.exe**; Unreal Editor is not required. Exact patch verification: **765 C++ PASS executions / zero FAIL in seven reports, 51 staged PNGs**. These are selected repeated automated checks, not human playtests. [Release notes and limits](Docs/Releases/apollo12-v0.9.1/RELEASE_NOTES.md).

The dated local notes below retain their original pre-publication context. Compile-time dirty provenance and the reviewed release source commit are recorded separately.

## Local hand-pose correction — 14 November 2026 (unreleased)

The working tree corrects the expedition skin's sideways palm/finger adaptation and removes overlapping legacy forearm surfaces. See [exact cause, before/after frames and verification](Docs/HandPose20261114/TEST_REPORT.md). This is separate from the published v0.9.0 below; its Windows ZIP still contains the original free-hand defect.

The corrected local Windows package is `Dist/Apollo12/Windows/LunarRescue.exe`: 765 selected packaged assertion executions / zero FAIL in seven reports, 51 staged PNGs, EXE `092b2b025a25…`. This is an unpublished patch; the earlier 2,019 total belongs to the original v0.9.0 executable.

## v0.9.0 — EVA, Equipment and Exploration — 14 November 2026

[Windows release](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.9.0) · [Download Windows ZIP](https://github.com/Shahzod1602/NASA/releases/download/apollo12-v0.9.0/LunarRescue-Expeditions-v0.9.0-Windows.zip).

Extract the entire ZIP and run **LunarRescue.exe**; keep Engine, LunarRescue and Credits together. Unreal Editor/Python are not required to play. Explore the Moon or Mars, choose a mission, then start. This update adds ordered pre-EVA checks, functional Map/Inventory, offline NASA colour globes, a drag/Verify cable board, credited suit/gloves and lunar rover, stable hatch handover and grounded base supports.

Exact packaged verification: **2,019 C++ PASS executions / zero FAIL in 20 case reports, with 109 staged review PNGs**. Counts include repeated configurations; human playtests: **0**. The short paired performance capture does not establish sustained improvement. Better habitat/lander models and the final FIELD GUIDE preference remain open. [Release notes and scientific/performance limits](Docs/Releases/apollo12-v0.9.0/RELEASE_NOTES.md).

The dated local-development records below retain their original pre-publication context. Archive metadata links the release source snapshot separately from the true compile-time base/dirty provenance.

## Local feedback development — 14 November 2026 (unreleased)

The working tree adds four ordered pre-EVA checks with live progress and **Next**, read-only preflight **Map / Inventory**, an offline rotatable NASA planet overview, an explicit **Verify** cable board with drag and keyboard/click controls, credited astronaut and Moon-rover visuals, and grounded base/power/solar supports. Active play uses the balanced resource profile without an extra automatic modal. The hatch handover preserves the player's view instead of flying the camera through the cabin.

See [implementation and limits](Docs/Feedback20261113/IMPLEMENTATION.md), [task intake](Docs/Feedback20261113/TASK_INTAKE.md) and [science/source notes](SCIENCE_AND_SOURCES.md). The lower field-guide card preference is awaiting clarification of a later Telegram follow-up. Historical performance and test counts in the published-release sections below do not certify this working tree. A fresh build and its dated verification receipt are required.

The fresh local Windows candidate is now verified: **2,019 C++ PASS executions / zero FAIL across 20 case reports, with 109 staged review PNGs**. Run `Dist/Apollo12/Windows/LunarRescue.exe` with the complete Windows folder present. This candidate is unpublished; the existing public download below is v0.8.2. [Exact tests, executable/payload hashes and performance limits](Docs/Feedback20261113/TEST_REPORT.md). No human or network-isolated playtest is claimed.

For editable development, fetch Git LFS content and open `LunarRescue.uproject` with the reviewed Unreal **5.8.3** installation. The bounded Windows Editor build used here is:

```powershell
& 'D:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat' LunarRescueEditor Win64 Development -Project=D:/LunarRescue/LunarRescue.uproject -WaitMutex -MaxParallelActions=1 -NoUBA -NoPCH -NoSharedPCH
```

Asset-preparation scripts require Python with NumPy, Pillow, trimesh and DracoPy; they are development tools, not player/runtime dependencies. Raw models and source credits remain in their dated `SourceArt` directories. The original A7L assets and existing Mars training vehicle remain available. Do not regenerate the authored maps to import these new visuals.

## v0.8.2 — Intrepid surface and cabin (11 November 2026)

**Play on Windows:** [v0.8.2 release](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.8.2) · [Windows ZIP](https://github.com/Shahzod1602/NASA/releases/download/apollo12-v0.8.2/LunarRescue-Expeditions-v0.8.2-Windows.zip).

This release includes the two dated Intrepid development passes below: a bounded
50 × 50 m landing-area art layer, refined foil/metal and recovered cabin shading
with a subtle cloth finish. Existing NASA/LROC elevations, mission locations and
Moon/Mars missions remain in use. Extract the whole ZIP and run `LunarRescue.exe`;
Unreal Editor is not required. Choose **Explore the Moon → Apollo 12 → Start Mission**.

The verified executable is `59d4f795b114`: 1,763 packaged C++ PASS executions / 0
failures across 31 reports, plus 32 staged review PNGs. These are repeated automated
executions, not unique tests or human playtests. The capped four-camera 720p Medium
capture averages 59.98 FPS; full-mission performance is not established.
[Release details](Docs/Releases/apollo12-v0.8.2/RELEASE_NOTES.md) · [Test report](TEST_REPORT.md).

Dated local/unreleased records below retain their original development context.
Release source identity and original dirty compile provenance are recorded separately.

## Local development — Intrepid cabin shading (11 November 2026)

The cabin pass recovers authored normals that were omitted by the original GLB
to OBJ conversion. A separate static-mesh asset preserves the existing positions,
UVs, material layout and lander placement. Its rendering frames are baked from
the retained model and invalid legacy tangent bases are repaired explicitly.
Two shared white material slots receive a subtle cloth finish; their original
GLB neutral color factors remain distinct. No new measured terrain is added.

The source still has a coarse silhouette. This is a shading improvement, not a
new photoreal spacecraft reconstruction. The local Windows build passes 1,763
C++ assertion executions across 31 raw reports, with 32 packaged review PNGs.
Its capped 720p Medium four-camera feature capture averages 59.98 FPS; this does
not establish whole-game performance. [Exact tests and limits](TEST_REPORT.md).
[Implementation and limits](Docs/IntrepidCabin20261111/IMPLEMENTATION.md).
The public v0.8.1 download below predates both local Intrepid passes.
Run `Dist/Apollo12/Windows/LunarRescue.exe`, then choose **Explore the Moon →
Apollo 12 → Start Mission**. Keep the complete Windows folder beside its launcher.

## Earlier local build — Intrepid visual slice (11 November 2026)

A bounded 50 × 50 m Apollo landing-area art pass adds distance-filtered regolith
shading, grounded instanced fragments, limited small-rock shadows, revised foil
and metal finishes, and restrained blanket folds. Existing LROC elevations,
collision, mapped worksites and all four maps remain the source of ground truth.
The original ladder is retained. The source model remains a simplified reference.

This work uses the project's existing C++ instancing pipeline; no new PCG/MCP
plugin, cloud world generator or runtime internet dependency is required.
The earlier `dc0b5864…` Windows build compiled, cooked and staged successfully. Its focused
visual checks pass: 335 C++ assertion executions, 32 packaged Low/Medium PNGs at
720p/1080p, and paired 6,000-frame captures. At 1280 × 720 Medium with a 60 FPS
cap, the four-camera feature capture averages 60.00 FPS; this is a limited
offscreen workload, not a whole-game performance claim. Regression results,
hardware and limitations are recorded in [TEST_REPORT](TEST_REPORT.md).
[Scope and reproduction](Docs/IntrepidVisual20261111/IMPLEMENTATION.md) ·
[Science and art](Docs/IntrepidVisual20261111/SCIENCE_AND_ART.md).
The public v0.8.1 download below predates this local art pass.

To play the local build, run `Dist/Apollo12/Windows/LunarRescue.exe`, then choose
**Explore the Moon → Apollo 12 → Start Mission**. Keep the entire Windows folder;
Unreal Editor and runtime internet are not required by the game.


## v0.8.1 — Mission interaction fixes (6 November 2026)

**Windows download:** [v0.8.1 release](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.8.1) · [Windows ZIP](https://github.com/Shahzod1602/NASA/releases/download/apollo12-v0.8.1/LunarRescue-Expeditions-v0.8.1-Windows.zip).

This patch improves mission-picker responsiveness and full-area action buttons,
places PIP-3 outside the lander, aligns existing carried items with the gloves,
clarifies ALSEP work guidance and restores rescue fast stride. Extract the entire
ZIP and run `LunarRescue.exe`; Unreal Editor is not required. The Microsoft C++
runtime installer is included if Windows needs it.

The verified inner executable is `06c1b022f92b`: 4,092 packaged C++ PASS
executions / 0 failures and 16/16 regression suites. Editor selection is a
separate 66 PASS. Human participants: 0. Historical local verification records
below retain their dates and compile-time provenance. See the
[release notes](Docs/TaskFixes20261106/RELEASE_NOTES.md) and [test report](TEST_REPORT.md).


## Local verification record — 6 November 2026 (before v0.8.1 publication)

Mission actions now fill their outlined button areas and share centred labels,
hover, press and keyboard-focus feedback. The picker reuses destination textures
and avoids redundant hover updates; performance evidence is scoped to the menu.
PIP-3 is a stationary fictional guide placed on clear terrain outside the lander.
Carried items and gloves use a common grip frame in first and third person.

ALSEP guidance distinguishes the lander from the distant registered instrument
sites and only offers work input when it is reachable. Hold the configured
**Fast stride** key (default **Left Shift**) to move faster; release to walk.
Toggle mode remains available in Settings. Movement speeds are gameplay tuning;
the oxygen clock continues to count active gameplay seconds.

See [scope and reproduction](Docs/TaskFixes20261106/IMPLEMENTATION.md) and
[current test results](TEST_REPORT.md). The original local verification predates v0.8.1 publication;
its dates, source hashes and compile-time provenance are retained.

Final local Windows build: **4,092 packaged C++ PASS executions / 0 failures**,
16/16 regression suites; Editor selection adds a separate 66 PASS.
Human participants: 0. Launch `Dist/Apollo12/Windows/LunarRescue.exe`.

## Field operations — v0.8.0 (4 November 2026)

**Windows download:** [v0.8.0 release](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.8.0) · [Windows ZIP](https://github.com/Shahzod1602/NASA/releases/download/apollo12-v0.8.0/LunarRescue-Expeditions-v0.8.0-Windows.zip).

This release includes the dated November2–4 development records below. Their
local/unreleased labels describe their original verification dates.

Surface missions now include four ordered pre-EVA training checks and a required
resource briefing. Choose and confirm a gameplay budget before surface work.
**B** opens the board/map/inventory; yellow stars use current world positions.
Selecting a support waypoint changes the displayed navigation, while the main
mission target and objective stay intact. Select Main objective to follow it again.

Inspect the authored training rover to unlock driving and the optical magnifier.
**W/S** drive, **A/D** steer, the work key (default **E**) exits onto clear grounded
space. **G** toggles the magnifier; hold **left click** to inspect a visible labelled
cartridge, release to cancel, then **E** packs it. Cargo mass and data storage are
limited. Deposit near the lander; restored mission power/ALSEP links enable timed
recharge and downlink beside the board. SELENE/ARES cable installation now opens
labelled terminal matching. B/G are reserved and cannot also be rebound to work.

These are **fictional modern training additions**. Apollo 12 did not use an LRV.
Vehicle/sampler geometry is authored; the NASA SEV model remains a reference.
Cartridges label remote LCROSS publication evidence, never local water discovery
or historical Apollo cargo. Existing LROC terrain and all four maps are retained.
Legacy Apollo checkpoints without additive training data resume the original run.

Final local Windows build: UE **5.8.3**, executable **08485e289426**.
**4,018 C++ PASS executions / 0 FAIL**, 280 staged PNGs and 8 paired-pixel checks
were verified on this executable. Counts include repeated mission/resolution cases,
not unique tests or human participants. Opening performance was measured separately;
whole-mission/occupied-driving performance and human playtesting remain unmeasured.
See [test report](TEST_REPORT.md), [implementation](Docs/FieldOperations20261104/IMPLEMENTATION.md),
[science](Docs/FieldOperations20261104/SCIENCE_RESEARCH.md),
[story and assets](Docs/FieldOperations20261104/STORY_AND_REFERENCES.md),
[verification index](Docs/FieldOperations20261104/verification-index.json).
Competition eligibility remains unconfirmed; this is independent product work.

Run the whole extracted Windows folder using Dist/Apollo12/Windows/LunarRescue.exe.
Editable-project build: Tools/package_apollo12.ps1 -MaxCompileActions 2.
Reproduce feature checks with Tools/verify_field_operations.ps1 -Packaged, then
-Packaged -Rendered and -Packaged -Review at 1280x720 and 1920x1080. The existing
Tools/verify_gameplay_regression.ps1 runs the 16 legacy suites.

## Gameplay additions (local, unreleased - 3 November 2026)

New missions have a short authored cabin departure before surface control. Enter
starts it from the briefing; Enter/left click skips after the input guard, and
Esc pauses. The mission resource clock starts at handover.

Picked-up rescue cells now fit the first-person view across supported FOVs and
have a third-person body proxy. Apollo's committed sample/hardware previews
also support the body view and keep their existing automatic stow timing.

Approach PIP-3 or Mira's communicator and use the displayed work binding, or
press **I > Mission Crew**. Both are fictional, scripted guides. Moon missions
also offer **I > Water Research**: interpret real LCROSS publication values,
answer three short evidence questions and explicitly record an optional report.
**Science & Sources** separates measured observations, inferred values and the
hypothetical calculation. This remote Cabeus/2009 exercise does not detect
water at Apollo 12 or grant historical cargo/resources.

See [controls, scope and scientific limits](Docs/Gameplay20261103/FEATURE_GUIDE.md)
and [current executable-specific validation](TEST_REPORT.md). Earlier 2 November
counts below describe their named older executable. The public v0.7.0 download
remains the previously published artifact; this iteration is a local build.


Windows v0.7.0: [release and download](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.7.0).
Extract the complete ZIP and run `LunarRescue.exe`; Unreal Editor is not required.

## Feedback improvements (local, unreleased - 2 November 2026)

The active HUD now uses borderless mission/objective and resource text, with
compact work guidance and subtitles. Layout widths respond to 100-150% text;
settings labels update with the text-size choice. Mission loading uses smaller,
measured typography and separates the guide from corner preparation progress.
The percentage describes preparation stages, not bytes loaded.

Press **I** for optional state-derived equipment diagnostics. SELENE/ARES explain
the cell -> port -> cable -> startup -> science chain; Apollo 12 shows its own
photo, instrument and packed-cargo prerequisites. Inspection grants no progress
or resources. See [gameplay research and limits](GAMEPLAY_REFERENCE_UPDATE.md).

The equipment pass adds **32 authored scene-detail treatments**, reuses five
existing tools in a storage display and retains two targets unchanged. The
39-row inventory is research/coverage tracking, not 39 newly imported meshes.
See [asset scope](ASSET_IMPROVEMENTS_UPDATE.md), [verified source candidates](ASSET_SOURCE_CATALOG.md)
and [loading typography](LOADING_TYPOGRAPHY_UPDATE.md). NASA terrain and existing
asset credits remain separate from cosmetic detail.

The final local Windows build (`b0347651...a9fb4` SHA-256) compiled, fully
cooked and staged after the contact corrections below. All 17 queued runs and
the separate typography suite passed: **1,254 exact PASS executions** (1,128
functional, 76 actual-loading travel and 50 staged HUD-layout checks). Wrappers
validated **110 staged PNGs** and six routed Slate pointer outcomes separately.
[The evidence receipt](Docs/Feedback2026-11-02/evidence.json) keeps both earlier
executables and their repeated results under their own hashes. For matched
performance measurements and the completed **264/264 unchanged protected-file**
comparison, see [TEST_REPORT](TEST_REPORT.md). One controlled performance run per
scene does not establish sustained whole-mission FPS. No human playtest result
is claimed. The public v0.7.0 download above is the
previously published artifact; this paragraph records the pre-v0.8.0 local development state.

The later equipment review found a tilted SELENE rover and buried cosmetic
rocks. The supported Editor repair changed only the existing MoonBase rover
rotation and world Z: pitch 15 degrees became level pitch with 15-degree yaw,
and Z rose 4.555 cm. All eight wheels were sampled at 48 contact entries
(40 distinct source vertices), with 1.000-6.875 cm sampled terrain clearance.
The receipt preserved rover XY/scale/materials/collision and all 1,910 other
actors; ARES and planetary terrain were not edited. Authored rock/talus detail
now scales from each retained mesh bounding box and follows the measured local
terrain normal, with 20% of its requested height buried. This is cosmetic
placement, not a geologic measurement. The full final-build retest includes
18 new grounding assertions across all three missions. Fourteen fresh Editor
Python equipment views were visually reviewed; the latest solar junction view
remains inconclusive. Source OBJ triangle checks put the junction outside the
panel slab, but that is not visual certification of the rendered detail.

## Intro, mission HUD and field lessons (local development, 1 November 2026)

The supplied intro artwork fades into the menu without repainting the source
image. All five home actions share the blue glass style; destination overview
text is uppercase. Moon/Mars still open their mission picker before loading.
Active missions now use compact frosted panels for objectives, actual work
progress, navigation, subtitles and the gameplay resource clock. The pause page
provides Resume, Settings, Restart Mission, Main Menu and Exit; full restart
returns to the briefing and keeps the saved Apollo checkpoint.

Looking sharply down lowers and hides free hands while tool/carry hands remain
available. The 12032 soil step 2/2 work view omits the near-cut leg proxies;
soil deposit/sealing and rescue work gestures use the existing gameplay state.
Optional **Field lessons** in the `I` notebook provide three short questions,
explanations and sources. Latest answers are session-only and do not affect
cargo or resources. They are not evidence of measured learning gains.

See [scope, controls and validation status](NASA_TASKS_UPDATE.md). Current terrain
provenance is in [Science & Sources](SCIENCE_AND_SOURCES.md): Apollo 12 uses the
2 m LROC terrain product; SELENE and ARES keep authored terrain.

## Mission selection and blue glass cards (local development)

**Explore the Moon** opens a choice of **Apollo 12** and **SELENE**. **Explore
Mars** opens **ARES**. Choose a card, then press **Start mission**; browsing a
destination never begins loading. Continue expedition still resumes the existing
Apollo checkpoint. Blue frosted cards keep the selected mission highlighted
when the pointer leaves. Use `1` / `2` for destinations, left/right to select,
Enter to open/confirm, Esc to go back, and F1 for settings. See
[implementation and verification](MISSION_SELECTION_UPDATE.md).

## My Expedition and cargo stowage (local development)

The journal map overlays your sampled walking trace on the modern LROC map, while
the evidence panel adds a **My Expedition / map** view with committed worksite
markers. Walking traces survive new checkpoints; older saves remain readable and
are labelled as partial coverage. The suit has more tapered sleeves/gloves and
shallower garment end caps. Existing boot treads now follow posed feet and reuse
up to 160 components. Completing the expedition starts a short skippable authored
cargo-stowage scene. See [scope and actual validation](EXPEDITION_EXPERIENCE.md).

## Expedition presentation and feedback (local development)

Intrepid gains terrain-aligned contact detail, nearby regolith stays within a
bounded instancing budget, and suit/tool transitions settle more smoothly.
Work progress is distinguished from recorded cargo; the debrief explains the
observed evidence, optional study and exercise budget. See [scope and validation](EXPEDITION_POLISH.md).

## Destination loading screens (local development)

Moon and Mars travel displays illustrated mission guides and smoothly animated
preparation progress. The selected destination opens automatically when ready.
Map travel for Continue and full restart uses the same transition. See [implementation and checks](LOADING_UPDATE.md).

Local graphics performance checks and reproduction commands: [PERFORMANCE_UPDATE.md](PERFORMANCE_UPDATE.md).

## Graphics and comfort pass (local development)

Apollo equipment is more readable in shadow, and planted boot soles follow
the terrain slope. Gameplay settings add saved tutorial guidance, walking
crosshair and HUD background-opacity controls. See [scope and validation](GRAPHICS_UPDATE.md).

## Reference-led frontend refresh (unreleased)

The destination menu now uses separate illustrated Moon and Mars backgrounds,
Continue, Settings and Exit buttons. Settings are arranged in four tabs with
two scrollable columns. Graphics, audio and input options connect to the actual
game; display changes require confirmation within 15 seconds. Unsupported
reference options are explained instead of presented as working controls.
See [frontend scope and validation](FRONTEND_UPDATE.md).

## Contextual input help - 0.6.3

Unsuccessful work input and a primary click outside a tool now explain the next
action in the existing instruction card: approach the marker, face the site,
move around an obstacle, use the current work key, or wait for a photograph.
The hint updates as the player moves, respects the actual interaction distance,
and expires after six unpaused seconds. Repeated input does not restart it or
interrupt mission radio. H also hides this optional help. Work and cargo still
require the normal tool actions. See [guidance details](CLARITY_UPDATE.md).

Windows validation on 27 October 2026: 333 assertions passed, zero failures;
17 fresh 720p DX11 guidance screenshots were reviewed, including 150% text.
All 53 inventoried save, photograph and settings files retained their hashes.

## Audit fixes - 0.6.2

An expired timed exercise closes unfinished tools and shows its result. Moon
interface and mission-selection text scales consistently, with measured
briefing/result layouts and full-width journal reading pages. The route page
retains the historical traverse.

Footsteps and legacy radio respect their volume settings. Prompts use rebound
keys, and Apollo narration no longer assumes default controls. An open evidence
album refreshes when a photograph finishes or fails; checkpoint reload waits
until a pending photograph is saved. Rendered workflow tests now wait for the
actual asynchronous photo result.

Windows validation on 27 October 2026: 368 headless and 154 rendered
assertions passed across 15 suite runs. The rendered total includes the basic
and polish suites repeated with real graphics. The full movement walkthrough
returned all cargo, and all 42 inventoried player files retained their hashes.

## Photo and animation optimization - 0.6.1

Mission photographs are encoded and written on a background queue. Evidence
is committed after a successful file write, with retry if writing fails. The
work camera, tool contact and glove pose now update in that order once per
normal frame. The album retains its three most recently viewed photographs.

The audio-enabled 720p Medium mission capture averaged 59.99 FPS; its largest
frame fell from 169.85 ms to 27.45 ms in the measured build pair. The Windows
build passed 383 checks. See [measurement limits](FIELDWORK_UPDATE.md).

## Contact and coating investigation - 0.6.0

Animated tool jaws meet the actual rock or fitting. Samples lift into a labeled
soft pouch; the core tube stays at the surface during hammering. Reference-led
suit and Surveyor detail accompany a correctly scaled Intrepid model. K opens
an optional two-photo coating investigation after the Surveyor context photo.
The illustrated comparison follows NASA's returned-camera study. Old saves
remain valid. See [scope and limitations](FIELDWORK_UPDATE.md) and the
[pending five-person study](PLAYTEST_GUIDE.md).

## Guided first expedition - 0.5.2

The first field-kit check introduces one action at a time. Shorter work prompts,
precise fitting markers, sample sealing feedback and a revisitable how-to page
make the Apollo workflow easier to follow. H toggles guidance; pause, then H opens
the guide. See [scope and validation](CLARITY_UPDATE.md).

## Realism pass - 0.5.1

Revised cloth folds, garment dust and seams, equipment paint and crumpled foil,
filtered regolith and instanced surface fragments give the Apollo chapter more
surface detail. Contact shading, a bounded EVA hop and smaller ballistic landing
fragments support the new appearance. See [scope, reproduction and validation](REALISM_UPDATE.md).

## Suit and science tools - 0.5.0

The Apollo chapter adds a 36-bone skinned astronaut and hands, separate photo,
leveling, antenna, rock/soil, drive-core and hardware tools, UMG settings, a photo
album and a cargo debrief. Historical exploration is untimed; **T** selects the
separate exercise before starting. **F1** opens settings and **I** opens evidence.
See [implementation and validation](POLISH_UPDATE.md), [historical audit](HISTORICAL_AUDIT.md)
and the [pending first-time player protocol](PLAYTEST_GUIDE.md). Earlier update
sections below are historical milestone records.

## English edition - 0.4.4

The entire game interface is English: menus, objectives, instructions, journal, radio messages, results and save notices. The former L language switch is removed, and old language preferences cannot switch the game back. See [the English edition notes](ENGLISH_UPDATE.md).

## Checkpoint update — 0.4.3

Apollo 12 now saves committed work automatically. **C** continues the saved expedition from mission selection or the Apollo briefing; **Esc**, then **R** reloads the last checkpoint. Starting a new expedition asks before replacing existing progress. Samples, photographs, Surveyor hardware and exercise telemetry survive a restart. See [save locations, recovery and validation](CHECKPOINT_UPDATE.md). Walking after the last completed task is not saved on exit.

## Onboarding update — 0.4.2

The Apollo HUD shows one current task with its purpose; the full checklist is in the paused **Tab / J** journal. A short look/walk/work guide introduces controls. Surveyor can be reached along a player-chosen route; the historical traverse remains on the map. The former bilingual UI from this milestone was superseded by the English edition. See [the historical validation notes](ONBOARDING_UPDATE.md).

## Current development plan — 2026-10-24

The [full game improvement plan in Uzbek](GAME_MASTER_PLAN.md) records the original roadmap, historical milestone status and the v0.5 implementation. Current results and remaining human acceptance work are listed in [the v0.5 validation record](POLISH_UPDATE.md).

## Historical Moon expedition — 2026-10-24

The Moon selection now opens **Apollo 12: Surveyor's Legacy**, a playable chapter based on Apollo 12's surface work. NASA/LROC terrain, mapped sample sites and EVA traverse points support a route from Intrepid to Surveyor 3 and back. See [Apollo 12 implementation and sources](APOLLO12.md). The earlier fictional SELENE expedition remains in the project as a separate map.

A first-person expedition prototype with separate Moon and Mars missions, all with English player-facing text and guidance.

The 0.4.1 [astronaut update](SUIT_UPDATE.md) adds fitted suit meshes, articulated gloves, visible boots and legs, Shift fast stride and a V following-camera view. Hold Alt in that view to inspect the suit. Apollo instructions now name the destination and explain the next action.

## Old audit snapshot (2026-09-21)

This section records the project state before the Apollo 12 update. Its statements about planned NASA terrain and deferred implementation are historical; Apollo 12 now uses NASA/LROC data described in [APOLLO12.md](APOLLO12.md).

The post-audit menu click fix starts a mission only when its planet card is clicked. See [the maintenance record](MAINTENANCE_FOLLOWUP.md). The old v0.2.0 package predates this fix.

The older Moon/Mars missions and audit notes describe the project at their recorded dates. See [technical audit](TECHNICAL_AUDIT.md), [sources](SCIENCE_AND_SOURCES.md), [credits](CREDITS.md), [changelog](CHANGELOG.md) and [playtest protocol](PLAYTEST_PROTOCOL.md).

## Play the standalone Windows build

Download the Windows ZIP from the [latest English Windows release](https://github.com/Shahzod1602/NASA/releases/latest), extract the entire archive, and run **LunarRescue.exe**. Unreal Editor is not required. The package includes its asset credits; keep the Engine, LunarRescue and Credits folders together. The earlier [v0.2.0 Mars release](https://github.com/Shahzod1602/NASA/releases/tag/mars-expedition-v0.2.0) remains available.

The local Apollo 12 package is under `Dist/Apollo12/Windows`.

Open **Play Lunar Rescue.cmd**. It prefers `Dist/Apollo12/Windows/LunarRescue.exe`, then `Dist/MoonExpedition/Windows/LunarRescue.exe`, then the previous `Dist/Windows` package or editor game build. The standalone build does not require Unreal Editor.
Keep the entire `Dist/Apollo12/Windows` folder together when copying the historical chapter, including
its Engine, LunarRescue and Credits subfolders.

To regenerate the package after source or content changes, run
`Tools/package_apollo12.ps1` (`-EngineRoot` selects a different engine installation). This writes `Dist/Apollo12/Windows` and preserves the earlier packages.
If no standalone package is present, the launchers fall back to the editor game build.

## Get the project

Requires Unreal Engine **5.8.3**, Git LFS, and the Visual Studio C++/Windows SDK
toolchain for Unreal projects on Windows. This repository contains the editable
project and assets, not a standalone executable.

```sh
git lfs install
git clone https://github.com/Shahzod1602/NASA.git
cd NASA
git lfs pull
```

Open `LunarRescue.uproject` with Unreal Engine 5.8 and allow it to build the
missing C++ modules. If needed, generate Visual Studio project files and build
the `LunarRescueEditor` target in **Development Editor / Win64** first.
Open `/Game/Rescue/Maps/MissionSelect`, then use Play in the editor.

The `.cmd` launchers currently expect the engine under
`D:\UnrealEngine\UE_5.8`; edit that engine path for another installation.
They resolve the project relative to the launcher location. The low-memory
launcher limits the texture streaming pool to 384 MB and uses shadow quality 2.
The project now uses that conservative profile by default, with a 60 FPS cap and
texture residency limited to available VRAM. The low-memory launcher remains compatible.
Generated build files, caches, local backups, and test screenshots are excluded.
Use Git LFS when cloning: the large binary assets must be downloaded, not left
as pointer files. Asset credits are included under `SourceArt`.

Open **Play Lunar Rescue.cmd** to play, or **Open Lunar Editor.cmd** to edit the project.
The editor fallback uses the installed Unreal Engine 5.8.3.

## Controls

- Enter: confirm mission selection or activate the selected pause action. Left click activates the visible button; background clicks do not resume.
- WASD or arrow keys: move.
- Mouse: look around.
- Space: jump.
- Shift: hold for a faster stride in Apollo 12.
- V: toggle helmet/following view; hold Alt and move the mouse to orbit the suit.
- E: open an Apollo work tool. Left click performs the current tool action.
- Mouse / arrows: adjust a level or antenna while its tool is open.
- Q: select the next site or journal page. Legacy SELENE retains its original controls.
- F: toggle visor zoom.
- Tab / J: open the Apollo field journal; it pauses movement and the exercise clock.
- Escape: pause immediately and cancel unfinished tool work; press again to resume.
- F1: settings, including key bindings, text size, audio and graphics.
- I: evidence album and the actual cargo debrief.
- T: choose historical exploration or timed exercise at the Apollo briefing.
- R or Enter: restart after winning or losing.
- Alt+F4: close the game.

## Apollo 12: Surveyor's Legacy

This condensed educational route covers selected work from Apollo 12's two historical EVAs. Check out at Intrepid, deploy a simplified ALSEP station, document three mapped samples, follow Conrad's reconstructed traverse to Surveyor 3, photograph it, recover its camera, scoop and cable, and return the evidence to Intrepid. The real crew later returned the samples and hardware to Earth. The playable chapter ends at cargo stowage; ascent, rendezvous and Earth return remain historical context.

NASA/LROC terrain and mapped work locations retain their source coordinates. The chapter combines historical activities into one single-player route. Models, interaction durations, the 30-minute exercise clock and paraphrased messages are gameplay adaptations. The clock is not suit endurance. See [APOLLO12.md](APOLLO12.md) for details and sources.

The earlier fictional SELENE chapter, **The Lost Expedition**, remains documented in [MOON_EXPEDITION.md](MOON_EXPEDITION.md) and is retained as a separate legacy map.

## Legacy SELENE suit feedback

The first-person view includes procedural glove geometry, gentle hand sway and a peripheral
helmet seal. Visor zoom hides the hands. Grounded movement leaves alternating tread decals
on lunar terrain; the latest 160 prints remain until the mission restarts. Prints have no collision.
Landing on regolith throws up a small pool of 16 grain particles following ballistic arcs;
particles stop at terrain contact and freeze during pause.
Breathing and muffled suit-conducted steps play during the mission and pause with gameplay.
English radio messages follow the mission stages, with seven additional story
recordings using a filtered synthetic voice.
There is no outdoor wind or airborne footstep sound effect.

Original synthetic audio is generated by Tools/prepare_immersion.py; radio source speech was
created locally using Windows Microsoft Zira. Tools/import_immersion.py imports audio and
creates the glove and footprint materials. The hands are a procedural prototype, not a rigged
character model. Carrying and held interactions use simple procedural hand poses;
a station interior and fully rigged interaction animations are not implemented.

## Editing

- Default Moon map: Content/Lunar/Maps/Apollo12. Legacy SELENE map: Content/Lunar/Maps/MoonBase.
- Astronaut: Content/Lunar/Blueprints/BP_Astronaut. Adjust speed and jumping under Character Movement.
- Mission: Content/Lunar/Blueprints/BP_LunarGameMode. C++ sets 480 seconds for Mars and the legacy SELENE map; Apollo 12 sets a separate 1,800-second exercise window. Changing Class Defaults alone does not change these runtime budgets.
- Mission tags: Module, Power, Data, Home. Place exactly one actor with each tag.
- Gameplay and interface: Source/LunarRescue/LunarGame.cpp.
- This project combines editable Blueprint child classes with C++ gameplay.
- Tools/build_moon.py creates the initial map. Do not run it over a manually edited map.

## Scientific scope

Moon gravity is -162 cm/s2 (1.62 m/s2); Mars gravity is -371 cm/s2 (3.71 m/s2). Movement and jumping are simplified for gameplay. Apollo 12 uses LROC stereo-derived elevation data described in [APOLLO12.md](APOLLO12.md); the legacy SELENE and Mars terrain remain authored scenery. Legacy research records are narrative objectives, not measurements. VR is not implemented.
Environment sources and rendering compromises are recorded in SourceArt/LunarV2/CREDITS.md.

The V3 surface adds irregular overlapping impact bowls, scalloped rims and radial ejecta relief,
denser near-field terrain, fractured rock variants, clustered debris and layered regolith shading.
The station area retains shallow relief for accessible movement. Small debris casts shadows but
does not obstruct the astronaut or interaction traces. On the legacy SELENE map this is authored procedural scenery, not a reconstruction of a surveyed landing site.
Rebuild V3 assets with Tools/prepare_lunar_v3.py, then run Tools/import_lunar_v3.py and
Tools/upgrade_lunar_v3.py through the Unreal Python commandlet. The upgrade replaces generated
environment actors, so back up the map before rerunning it after manual environment edits.

## Verification

Build the LunarRescueEditor Win64 Development target using the installed engine's Build.bat.
Run UnrealEditor-Cmd with the project and /Game/Lunar/Maps/MoonBase -game -nullrhi -unattended -LunarTest
for the mission checks, or -LunarMovementTest for sustained keyboard movement and jump checks.
`-LunarWalkthroughTest` walks from the actual spawn through all objectives, including the
optional recorder ledge and the return to the lander, using normal movement and collision.
It writes route progress, oxygen remaining, frame timing and peak process memory to
`Reports/walkthrough-checks.txt`. NullRHI results do not measure graphics performance.
Run `Tools/verify_audit_fixes.ps1` for the complete suite, or add `-Graphics` for an
offscreen 1280x720 DX11 run with the low-memory settings. Set `-EngineRoot` if needed.
`-LunarMissingLedgeMesh -LunarTest` exercises the safe fallback for a missing ledge mesh.
Reports are saved under Reports.
`-LunarLedgeTest` checks the optional ledge using simulated keyboard movement and
jumping. The updated mission suite passes 50 checks without sound or 52 with sound;
the ledge suite passes 3 checks. See [audit fixes and measured results](AUDIT_FIXES.md).
Mission checks teleport between objectives and are not a timed full playthrough.
See [story update notes](Reports/STORY_UPDATE.md) for the implemented scope.

## Legacy SELENE imported models
The legacy map uses an Apollo lander, two habitat parts, a Lunokhod rover, solar arrays, a power cell and a science terminal from the user's downloaded files. Existing procedural rocks use Poly Haven Moon Rock 01/06 diffuse textures at 2K. Asset sources and licenses are in SourceArt/ExternalModels/CREDITS.md. Import scripts are in Tools; their staging paths refer to this workstation.

## Moon and Mars

The startup menu offers **1 / Moon** and **2 / Mars**. Click the corresponding
card or press its number. Enter starts Moon. Press **M** from a briefing, paused
mission, or result screen to return to selection. **R** on a result screen
restarts the current planet.

Moon opens Apollo 12: Surveyor's Legacy. Mars has eight minutes of oxygen,
3.71 m/s2 gravity, a separate authored canyon map and ARES outpost. Retrieve the
rover cell, insert it, connect the cable, hold E to restart the station, download
the ice survey, and hold E for four seconds at the orbital relay. Return to the
ascent lander; Q optionally routes to the crew recorder after relay alignment.
Mars terrain is fictional; no surveyed Mars elevations are claimed.

`Tools/build_mars.py` regenerates only Mars and the menu using Unreal Python.
`Tools/import_mars_audio.py` imports the supplied English radio WAV files.
`Tools/verify_planets.ps1` tests map travel, both gameplay sequences, Mars movement
and the recorder ledge. Add `-Walkthrough -Graphics` for complete traversals with
rendering and performance reports, or `-Packaged` to test the standalone binary.
Tests are sequential to fit a 16 GB development machine.

See [MARS_UPDATE.md](MARS_UPDATE.md) for implementation and measured standalone validation.

### Glove and garment refinement (local development)

The shared suit now has shaped garment joints, detailed gloves and tool-aligned
first-person grips. Rebuild instructions and validation: [SUIT_REFINEMENT.md](SUIT_REFINEMENT.md).
