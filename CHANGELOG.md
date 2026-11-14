# Changelog

## v0.9.1 — palm and free-hand pose correction — 2026-11-14

- Correct the expedition skin's palm/finger roll using longitudinal and transverse references; fit palm spread to its controller rather than twisting the glove sideways.
- Hide overlapping legacy forearm surfaces while the new skin is active; retain their pose guides and original fallback.
- Add a real-skin palm-envelope regression, proxy-overlap assertion and free-hand FOV/lowering review frames. See Docs/HandPose20261114/TEST_REPORT.md for exact verification status. The published v0.9.0 archive is unchanged.
- Package the corrected EXE/assets as v0.9.1 with 765 selected assertion executions / zero FAIL, 51 staged PNGs and separately preserved compile/test provenance. See Docs/Releases/apollo12-v0.9.1/RELEASE_NOTES.md.

## v0.9.0 — EVA, equipment and exploration — 2026-11-14

- Package ordered pre-EVA checks, working Map/Inventory, offline NASA colour globe and retained progress updates.
- Add drag/Verify cable matching and click/keyboard alternative; preserve native repair guards.
- Adapt credited suit/glove and Moon-rover visuals; correct grip, arm fit, swept hull and base/solar supports.
- Keep a stable hatch view, remove the automatic resource-profile modal and retire redundant props/PIP.
- Use a credited Perseverance visual for fictional SELENE NOMAD-7; preserve mission controls/rewards and scientific/art distinctions.
- Exact package: 2,019 assertion executions / zero FAIL, 20 case reports and 109 staged PNGs; no human playtest claim.
- Retain performance uncertainty and pending FIELD GUIDE/model requests. See Docs/Releases/apollo12-v0.9.0/RELEASE_NOTES.md.

## v0.8.2 — Intrepid surface and cabin — 2026-11-11

- Package the two Intrepid development passes below: local regolith/rock art,
  foil/metal finishes, authored cabin normal recovery and restrained white cloth.
- Preserve NASA/LROC terrain, mission coordinates, original model geometry/UVs
  and the existing Apollo 12, SELENE and ARES mission flow.
- Include offline science/credits and exact-executable verification evidence.
  Exclude player saves/photos, generated reports, logs and debug files from ZIP.
- Canonical validation: 1,763 packaged C++ PASS executions / 0 FAIL in 31 reports;
  32 staged PNGs. Human participants: 0. Earlier totals remain historical.
- Keep original compile/local-test records unchanged and link the prepared release
  to its reviewed Git commit. See Docs/Releases/apollo12-v0.8.2/RELEASE_NOTES.md.

The dated unreleased entries below are the original development records now
included in v0.8.2; their dates and executable-specific evidence are retained.

## Unreleased — Intrepid cabin normal recovery — 2026-11-11

- Restore the retained GLB's authored normals on two main cabin objects in a
  separate mesh, preserving positions, topology, UVs and material assignments.
- Bake the source's computed rendering frames; explicitly repair invalid
  legacy tangent bases without moving vertices or rewriting source assets.
- Add a restrained authored cloth finish to the two shared white slots,
  retaining their distinct neutral source color factors.
- Add an independent cabin comparison control and two close Low/Medium camera
  views while keeping the earlier four-camera profiling workload.
- Preserve the existing ground/foil pass, all maps and mission coordinates.
  Executable-specific results and limitations are recorded in TEST_REPORT.

## Unreleased — Intrepid visual slice — 2026-11-11

- Add a soft-edged 50 m local regolith material using the retained authored bitmap.
- Place cosmetic fragments from actual mesh bounds with deliberate partial burial;
  cap the new layer at 2,048 fine and 64 separated shadowed fragments in nine batches.
- Preserve the old outside-fragment random sequence and the imported LROC surface.
- Refine three untextured foil slots and the metal structure using four dedicated
  new materials; reuse six lander batches for 12 shallow blanket folds.
- Retain the original ladder and all source image-bearing panels. Added tread
  strips were rejected during visual review and removed before final acceptance.
- Add bounded contact shading, explicit same-build baseline controls, fixed-camera
  Low/Medium review, physical corridor checks and a finite CSV profiling workload.


## v0.8.1 — Mission interaction fixes — 2026-11-06

- Fill mission action frames with the button background and hit area, centre
  captions, and preserve blue hover/press/keyboard-focus feedback.
- Cache destination previews, ignore redundant hover updates and reduce layered
  menu blur work while retaining frosted mission cards.
- Place stationary PIP-3 and the communicator on terrain with structure and
  capsule clearance instead of an unchecked offset beneath the lander.
- Share visible-item grip transforms with hands, including cell waist, pouch,
  core and Surveyor hardware in first and third person.
- Make ALSEP prompts reflect distance, facing and blockage; guard registered
  instrument commits and reset antenna alignment for a new expedition.
- Restore fast-stride movement speed in SELENE/ARES and clear stride during
  work, pause, planning and other UI states. Keep the elapsed-second oxygen model.
- Add real input/velocity regressions, full Apollo interaction traversal and
  rendered button/grip checks. Validation results are recorded in TEST_REPORT.

Final local executable `06c1b022f92b`: 4,092 C++ PASS executions / 0 failures,
16/16 regression suites, 230 staged PNGs and 8 paired-pixel checks. Editor
selection: separate 66 PASS. Human participants: 0. See TEST_REPORT.md for
menu-only performance controls and the untested limits.

## v0.8.0 — Modern field training — 2026-11-04

- Four ordered timed EVA checks and a mandatory one-time resource/difficulty plan.
- Board with actual-world map points, visible support navigation, shared utility,
  limited payload/storage, timed recharge and powered-link downlink.
- Authored Moon/Mars rover with swept collision, wheel contact/animation, battery
  travel cost, safe exits and Apollo camera clearance.
- Centred magnifier, held optical inspection, separate simulated cartridge cargo
  and lander deposit. Terrain/capsule/sight checks choose exposed authored sites.
- Labelled rescue cable matching; bounded strict-type additive Apollo save state.
- Pause/restart guards, faithful local session totals and offline source notes.
- Readable 100%/150% UI at 720p/1080p with separated map callouts.

Local executable 08485e289426: 4,018 C++ PASS executions / 0 FAIL,
280 staged PNGs, 8 paired-pixel checks and 16/16 legacy suites. Human participants:0.
No licence, map geometry or historical Git dates were changed. Publication of v0.8.0 is authorized separately after local verification.

## Unreleased - Departure, carried objects, fictional crew and water evidence - 2026-11-03

Final local exe 5171c1b0: 2,400 C++ PASS executions / 0 FAIL, 160 staged PNGs,
8 paired pixel checks and unchanged original 544-file snapshot. These are
automated repeated executions, not human playtests; see TEST_REPORT.md.

- Add a skippable, pause-aware authored transfer-cabin departure to new missions;
  hand over to the existing surface view before starting mission clocks.
- Fit the rescue cell from its actual mesh bounds/FOV; add a body carry proxy
  and committed Apollo sample/core/hardware previews in third person.
- Add PIP-3 and Dr. Mira Vale as explicitly fictional, optional scripted guides;
  advice reads current mission facts and grants no repairs, cargo or resources.
- Add an offline LCROSS publication-evidence exercise with three decisions,
  error feedback, explicit report commit, source/uncertainty explanations and
  additive valid Apollo checkpoint state. Keep SELENE research session-local.
- Wrap notebook navigation, guard stale/hidden callbacks and repeated answer
  keys, and use measured, content-sized departure captions at 100-150% text.
- Add combined functional/rendered fixtures and packaged-data/provenance checks.
  See [feature scope](Docs/Gameplay20261103/FEATURE_GUIDE.md) and
  [actual validation](TEST_REPORT.md); scripted results are not human playtests.


## Unreleased - Feedback, equipment diagnostics and loading typography - 2026-11-02

- Present active mission/objective and resource information without large HUD
  borders; keep compact contextual work guidance and subtitles, scale-dependent
  widths, actual hold progress and state-derived counters.
- Use compact measured loading typography, a separately reserved progress area
  and 100-150% text support. Keep preparation progress distinct from file bytes.
- Add an optional I notebook diagnostic chain for SELENE/ARES and a separate
  Apollo photo/instrument/cargo checklist, with a next action and explicit model
  limits. Read-only inspection grants no work or resource reward.
- Add 32 authored scene-detail treatments; reuse five existing tools in a storage
  display; retain two targets unchanged. The 39 inventory/research rows do not
  mean 39 new downloaded or imported meshes. Keep existing terrain and credits.
- Document verified external model candidates and the official Space Engineers
  gameplay references; label design inferences and avoid engineering claims.
- Credit AI coding assistance for the new authored implementation.
- Final local executable `b0347651...a9fb4` compiled, fully cooked/staged and
  passed all 17 queue runs plus typography: 1,254 exact PASS executions (1,128
  functional, 76 actual-travel and 50 staged HUD geometry). Validate 110 staged
  PNGs and six routed Slate pointer outcomes separately. Earlier 2ed/e738
  results remain historical under their own hashes, not additional unique
  tests. See [the per-executable receipt](Docs/Feedback2026-11-02/evidence.json).
- Repair only the original SELENE MoonBase rover pose using supported Editor
  Python, after a full map backup and hash check. Preserve XY/scale/materials/
  collision and all 1,910 other actors. Use explicit Rotator keywords in the
  original rover placement call to prevent recurrence. ARES is unchanged.
- Normalize cosmetic basalt/talus dimensions by the actual source bounding box;
  align to measured terrain normals and bury 20% of the requested height.
  Existing terrain, collision, navigation, instance caps and no-Tick behavior
  remain unchanged. Six new geometric assertions per mission passed on the
  final executable and are included in its 1,254 total.
- Review 14 fresh Editor Python equipment views. The latest solar-junction view
  remains visually inconclusive; source OBJ placement proof is not a rendered
  detail certification. Matched controlled performance runs and the 264/264
  unchanged protected-file comparison completed; see [TEST_REPORT](TEST_REPORT.md)
  for conditions and limits. No sustained-FPS or human-playtest result is claimed.
  The existing v0.7.0 release remains a separate published artifact.

## v0.7.0 — Expedition UI and Field Lessons — 2026-11-01

- Ship the explicit Moon/Mars mission picker, mission loading guides, expedition trail and cargo finale since v0.6.4.
- Add the supplied-art intro, five glass home actions, compact HUD, real hold indicator and five-action pause/restart.
- Fix first-person work views and add optional sourced lessons with win/loss session summaries.
- Package a clean Windows build with credits and checksums; omit player saves, test outputs and debug symbols.
- Local validation: 997 assertion/outcomes, zero failures; human sessions remain zero. See TEST_REPORT.md.

## 2026-11-01 - Intro, compact mission interface and field lessons (local)

- Fade the unchanged user-supplied intro image into the menu; share glass styling across five home actions and uppercase overview text.
- Replace active-play HUD bands with compact frosted objective, guide, resource and subtitle panels driven by mission state.
- Distinguish actual hold progress in rescue tasks from step-by-step Apollo tool clicks.
- Add a five-action pause page and confirmed full restart to the briefing while preserving the Apollo checkpoint.
- Lower/hide free hands during steep downward viewing, retain task/carry hands, and omit near-cut leg proxies in the 12032 step 2/2 view.
- Refine authored soil deposit/sealing, cargo carry and rescue work gestures without awarding additional progress.
- Add optional sourced Field lessons with latest session answers and a factual debrief summary; no human comprehension result is claimed.
- Correct the science overview to distinguish the integrated Apollo 12 LROC product from authored rescue maps and the historical LOLA candidate.
- Validation for this iteration is recorded separately in TEST_REPORT.md and NASA_TASKS_UPDATE.md; earlier totals are historical.

## 2026-10-31 - Mission selection (local)

- Open a destination-specific mission picker before loading Apollo 12, SELENE or ARES.
- Require a chosen card and explicit Start mission; keep Continue as checkpoint resume.
- Share animated blue glass, real background blur and persistent selected cues across expedition entries and mission cards.
- Support keyboard selection, Back, settings overlays and scrollable 150% card text.
- Add selection and rendered-transition checks; expand travel checks to all three maps.


## 2026-10-29 - Expedition presentation and feedback (local)

- Ground Intrepid contact detail on imported terrain, with instanced seams, rims and equipment fasteners.
- Redistribute nearby regolith without expanding the six-batch surface budget or changing NASA collision.
- Smooth stride/turn/landing and tool visual paths while preserving exact primary grip and comfort options.
- Distinguish in-progress, pending status and recorded work; reuse bounded suit/tool audio cues.
- Add session-fact debrief advice and a blank 3-5 novice playtest protocol; human sessions are not yet run.

## 2026-10-29 — Destination loading screens (local)

- Add Moon/Mars mission illustrations, accurate gameplay tips and corner preparation progress.
- Keep Slate animation active during map loading and display completion before automatic entry.
- Preserve Continue options, block duplicate travel requests and isolate loading-test checkpoints.

## 2026-10-29 — Controlled graphics profiling (local)

- Add visible-window profiling and optional GPU telemetry.
- Reject overlapping game/editor/shader jobs and stale full-mission evidence.
- Test capture completeness, invalid values and hitch statistics.
- Document repeatable measurements and unresolved earlier variability in PERFORMANCE_UPDATE.md; no game-code speedup claimed.

## 2026-10-29 — Glove and garment refinement (local)

- Shaped sleeves, shoulder gussets, knee reinforcement and detailed glove pads/seams.
- Distinct digit lengths, relaxed/working curls and tool-aligned primary palm contact.
- Fixed packaged-build glove culling with a bounded viewmodel envelope; raised resting hands and retained photography hand hiding.
- Rebuilt skin LODs; added source geometry and visible-skeleton checks.


## Graphics and comfort pass — local development, 2026-10-29

- Brighter authored bounce-light fill around the existing Apollo scene.
- Ground-aligned boot poses using the actual planted-foot contact.
- Saved tutorial hints, walking crosshair and bounded HUD background opacity.
- Optional Fab editor plugin disabled for this project after a local missing-module build error.
- Validation and remaining limits: [GRAPHICS_UPDATE.md](GRAPHICS_UPDATE.md).

## Unreleased — frontend refresh, 2026-10-29

- Add destination artwork and a Continue/Moon/Mars/Settings/Exit menu.
- Organize settings into Video, Sound, Gameplay and Controls tabs.
- Add persistent graphics/audio options and reversible display-mode previews.
- Preserve mission saves, evidence album and rebound controls; document unsupported reference controls explicitly.

## 0.6.1 - 2026-10-26

- Passed 383 packaged checks and 32 rendered review states; the measured audio-enabled 720p Medium route reduced the largest frame from 169.85 ms to 27.45 ms, with 59.99 mean FPS.

- Encode and write mission photographs on the engine image-write queue; award evidence only after a successful file write, with retry on failure.
- Update the work camera before tool contacts and glove poses, removing the second tool-visual update from each normal frame.
- Retain the three most recently viewed album textures with bounded, mission-path-checked caching.
- Add rendered integration checks for photo completion, failure/retry and cache retention; extend opening-route profiles with explicit resolution and frame cap.

## 0.6.0 - 2026-10-26

- Added source-contact tongs/cutter jaws, moving sample payloads, a soft labeled pouch and a persistent driven core tube.
- Revised the cloth helmet hood, Surveyor truss and solar-cell dividers from NASA image AS12-48-7136; normalized the existing Intrepid model's height.
- Added the optional K coating investigation, two evidence photos and a selectable interpretation informed by NASA's returned-camera analysis. Panel colors and geometry remain illustrative.
- Retained checkpoint format 1 and added six-photo validation plus an unchanged v0.5.2 binary compatibility fixture.
- Cached suit bind rotations and work meshes; added audio-enabled full-route performance capture and isolated participant launch support. Human study results remain pending.

## 0.5.2 - 2026-10-25

- Guided the initial equipment check through looking, walking, facing the kit, opening and confirming; added H to hide/revisit help.
- Shortened active instructions and made prompts match the nearby aimed object and current key bindings.
- Added physical fitting markers, visible pending sample pouches and labeled committed-cargo confirmations.
- Stacked contextual work prompts above radio subtitles and introduced the evidence shortcut after evidence exists.
- Added a fifth journal help page, behavioral guidance checks and twelve rendered review states. The human novice study remains pending.
- Passed 325 final packaged checks and verified the original source-data hashes. Windows packaging now uses bounded IoStore concurrency and omits standalone debug symbols on the low-memory development machine.

## 0.5.1 - 2026-10-25

- Revised suit folds and hardware details, added bind-space cloth dust masks and rebuilt suit LODs.
- Added dedicated filtered regolith, fragment, painted equipment and crumpled foil materials, with six collision-free instanced surface batches.
- Added field-kit handles, hinges and closures; adjusted lunar contact shading and readability fill.
- Bounded the Apollo EVA hop under the unchanged lunar gravity and reduced landing debris to smaller angular fragments. See REALISM_UPDATE.md for scope and validation.

## 0.5.0 - 2026-10-25

- Added a 36-bone skinned suit and hands, articulated fingers, terrain-aware stance and tool poses, with three mesh LODs.
- Added distinct camera, ALSEP leveling, antenna, tongs, soil scoop, hammer-driven core and Surveyor recovery actions with cancellable work and committed cargo.
- Added English UMG settings with key rebinding and accessibility controls, a real-photo evidence album, radio transcript and cargo debrief.
- Made historical exploration untimed by default; retained the separate timed exercise and existing checkpoint compatibility.
- Improved work-site details, collision and shadow readability; added seven original synthetic English radio messages and documented NASA source corrections.
- Passed 300 Editor and 303 packaged checks, repeated 129 packaged checks after the photo fix, and completed a rendered full-mission profile. See POLISH_UPDATE.md for measurements and limitations; the real first-time-player study remains pending.

## 0.4.4 - 2026-10-24

- Made all player-facing game text English, including the menu, Apollo journal, task guidance, radio, endings and checkpoint notices.
- Removed the Uzbek text table, L language switch and language hint; old settings/launcher arguments cannot restore another language.
- Updated packaged help to English while preserving mission and checkpoint identifiers.
- Replaced language-switch checks with English-only input and legacy-launcher regressions.

## 0.4.3 - 2026-10-24

- Added versioned Apollo checkpoints using stable work/site IDs, evidence metadata, exercise telemetry and a grounded player position.
- Added C Continue from the menu/briefing/timeout, R checkpoint reload while paused, and explicit confirmation before a new expedition replaces saved progress.
- Added verified temporary writes, CRC and semantic validation, previous-copy recovery, and separate photograph folders per expedition.
- Added isolated save fault tests and a two-process menu-to-mission resume check; packaging now backs up player saves and photos.

## 0.4.2 - 2026-10-24

- Replaced the Apollo HUD's simultaneous mission checklist with one readable objective, next action and science purpose; moved all six tasks into the journal.
- Added 139 stable bilingual string-table entries for Apollo and mission selection, Uzbek defaults, L language switching and a saved language preference.
- Added contextual look/walk/work guidance and Tab as a journal shortcut, preserving J and Escape behavior.
- Removed mandatory traversal of twelve hidden waypoints. Surveyor arrival now depends on reaching its work area; the historical route remains visible and separately testable.
- Expanded input/progression and visual review checks, and added reproducible rendered frame-time captures with explicit warmup exclusion.

## 0.4.1 - 2026-10-24

- Replaced sphere-based hands with authored suit sleeves, wrist rings, padded palms and individually posed fingers; added a visible full suit, boots, helmet and pack.
- Added travel-driven gait, terrain-aware leg posing, blended work reach, Shift fast stride and V camera switching with Alt orbit.
- Added named destination markers and immediate next-action instructions to the Apollo HUD.
- Added reproducible suit mesh/material authoring and import, and explicit cook inclusion for standalone builds.

## 0.4.0 — 2026-10-24

- Added Apollo 12: Surveyor's Legacy as the Moon's menu-selected expedition, grounded in LROC terrain and mapped EVA work sites.
- Added a six-stage surface route: Intrepid checkout, ALSEP deployment, documented samples, mapped Surveyor approach, hardware recovery, and return.
- Added a paused field journal with sources, map, traverse, sample identifiers and adaptations; preserved MoonBase, Mars and mission-selection maps in the package.
- Added NASA/LROC source manifest, credits, data-preparation/import scripts, and a standalone Apollo 12 package workflow.

## 0.3.0 — 2026-10-24

- Expanded Moon to seven linked objectives with three sample sites, NOMAD-7 memory recovery and a three-channel Earthlink timing challenge.
- Added eight-minute oxygen, a one-time robot reserve, sample navigation, expedition checklist and ending ranks.
- Preserved existing Blueprint enum values, Mars progression and optional crew recorder.
- Added progression, pause, duplicate-reward and out-of-order checks plus a full movement-based expedition walkthrough.


## Chiqarilmagan — mavjud menyu maintenance

- Planet tanlashda sarlavha, footer va kartalar orasidagi tasodifiy click missiyani boshlashi tuzatildi.
- Mouse va Enter alohida boshqariladi; HUD va click ayni karta chegaralarini ishlatadi.
- Bu oldingi auditdan keyingi source o‘zgarishi. Mavjud v0.2.0 package yangilanmagan; [maintenance qaydi](MAINTENANCE_FOLLOWUP.md).

## Chiqarilmagan — 2026-09-21 audit va tayyorgarlik hujjatlari

- Texnik audit, ilmiy manbalar, tanlov holati, kechiktirilgan reja, test hisoboti, credits, playtest protokoli va shartli inglizcha taqdimot qo‘shildi.
- README'da oxygen capacity runtime override ko‘rsatmasi va gameplay resource birliklari aniqlashtirildi.
- Mavjud build va packaged regressiya/traversal tekshiruvlari qayta bajarildi; dalil va chegaralar TEST_REPORT.md'da.
- Gameplay, map, dataset integratsiyasi yoki reliz o‘zgarmadi. Yangi challenge implementatsiyasi 2026 qoida va foydalanuvchi shartiga binoan kechiktirildi.

## v0.2.0 — mavjud reliz, 2026-09-21

- Moon/Mars tanlash, ARES ekspeditsiyasi, orbital relay vazifasi va standalone Windows build.
- Mavjud implementatsiya va avvalgi tekshiruvlar: [MARS_UPDATE](MARS_UPDATE.md), [AUDIT_FIXES](AUDIT_FIXES.md).
- Ushbu reliz 2026 Space Apps hakatonidan oldin yaratilgan; qayta ishlatishga ruxsat tasdiqlanmagan.

## Local development — expedition trace, suit form and stowage

- Add My Expedition with a bounded saved walking trace and committed worksite markers.
- Preserve old checkpoints and explain incomplete trace coverage.
- Refine authored sleeve, glove and leg-end silhouettes; preserve the 36-bone rig.
- Ground existing tread decals at posed boots and reuse a fixed 160-component pool.
- Show an 8.8-second skippable cargo vignette after committed expedition completion.
- Extend the novice protocol; human sessions remain unperformed because participants are unavailable.
# Unreleased — NASA group feedback, 14 November 2026

- Ordered pre-EVA checks with in-place progress, explicit Next/Open Hatch gating and functional preflight Map/Inventory.
- Bounded local-map zoom/pan and optional offline NASA planet context; real local waypoints remain separate from global colour imagery.
- Minimal cable board with draggable signal cables, explicit Verify and an expandable keyboard/click alternative. Cable matching cannot apply station repair before verification.
- Direct balanced-profile mission entry and stable short hatch handover.
- Credited astronaut/body/hand retargets, supplied museum Moon rover with four wheel pivots and LODs, and NASA/JPL Perseverance appearance for fictional NOMAD-7.
- Lower-centre power-cell carry framing; original mission interactions, rewards and cargo authority retained.
- Terrain-tested base, solar and power-panel support placement; retired PIP and obsolete prototype/route decorations.
- The duplicate lower HUD-card preference remains unconfirmed after a later “qolib tursin” follow-up; clarification was requested. This entry is local development, not a published release or human playtest result.
