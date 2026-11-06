# November 6 interaction and presentation fixes

Date: 6 November 2026. Status: verified on the final local Windows build; scope and limits below.

This iteration addresses six reported problems in the existing Windows game.
It does not declare a new public release. Executable-specific receipts and
canonical counts are retained in `verification-index.json`; `TEST_REPORT.md`
provides the top-level report. Commands below also include optional variants;
only the runs identified by the index are reported as executed.

## Implemented behavior and causes

| Area | Finding | Change |
| --- | --- | --- |
| Mission-screen responsiveness | The report does not identify hardware or an exact screen. The destination/mission picker is the working assumption. Its cards changed blur strength during selection, small action buttons each used an additional blur pass, and presentation properties were reapplied after animations settled. These are identified rendering/update costs, not a proven sole cause of the reported lag. | Keep card frost at a fixed strength, retain the blue tint/glow transition and persistent selected state, remove separate action-button blur passes, and stop updating settled presentation values. A diagnostic switch retains the earlier blur policy for a labelled same-build cost experiment. |
| Action-button sizing | The button child overlay and glass face used content-sized alignment, allowing the label-sized fill to sit inside a larger outline. | Fill both horizontal and vertical button slots, fill the face overlay slot, center the caption, and keep decorative layers outside hit testing. The outer button owns the full clickable region; hover, press and keyboard-focus styling use that region. The shared button builder applies the correction across its users. |
| PIP-3 placement | A simple offset from the return/lander anchor could place a static guide beneath the non-colliding reference lander. A collision-only query could miss that visual footprint. | Search a bounded set of terrain-supported positions outside both the anchor and reference-lander bounds, with an approach apron, slope/height checks, blocking-scene clearance and separation from other guides. Reject an unavailable position rather than placing the guide inside geometry. |
| Held-object contact | Carried-item placement and glove placement used independent offsets. A fitted item could remain visibly separated from the palms. | Introduce shared component grip frames and derive glove contact from each visible carried mesh in the same camera/body coordinate space. Retain fitted first-person bounds and third-person proxies. Cell waist contact is distinct from its raised end bands; existing sample, pouch/core and hardware previews use the common grip calculation. |
| Moon science-station interaction | ALSEP's real work sites are about 125 m from the lander field kit, but the guide advertised an actionable level-adjustment key even at the lander. A selected support waypoint could make the nearby lander look like the intended work site. The native handler correctly required the actual site. | Show the main objective's current distance and actual work reach when far away, facing guidance when turned away, and obstruction guidance when blocked. Show the work-key action only when the registered site is usable, and explain pending photograph work. Preserve existing bubble/antenna confirmation requirements. Reject stale or unrelated instrument callbacks and reset the antenna on a new run. |
| Shift stride | Rescue movement could set the stride/pose flag without increasing `MaxWalkSpeed`. State changes could also leave stride state out of sync with allowed movement. | Update actual movement speed when stride begins, ends or becomes disallowed. Keep the existing configurable hold/toggle preference, and reject stride during panels, pause, work, holds, departure/finale, vehicle riding or an unconfirmed resource briefing. |

## Scope and limits

- PIP-3 remains a stationary fictional guide. No following AI, navigation system,
  pathfinding or continuous relocation was added. Its original implementation
  was static; the fix prevents an unsafe initial placement.
- Carried-item changes cover the existing visible carry/preview archetypes.
  They do not introduce a replacement skeletal model, a physics grasp or a new
  animation asset. New item archetypes still need their own appearance/contact
  review.
- Committed first-person previews use bounded nominal view distances of
  **55-58 cm**, with frustum fitting and the source mesh's original proportions
  retained. Third-person carried bounds remain wholly in front of the torso:
  their minimum actor-space forward X must be **at least 20 cm**. Carry poses use
  the existing two-joint **28 cm upper-arm / 24 cm forearm** construction.
  The view shoulder stays below the visor, and the body shoulder follows the
  crouch driver. Recorded hardware preview rotation stays in its component's
  local frame so camera world yaw does not distort the grip or arm reach.
- A pooled camera-attached **3-lumen** readability fill uses lighting channel 1.
  Owned first-person carried props, hands and sleeves receive channels 0 and 1;
  ordinary world geometry remains on channel 0. The fill is disabled during
  pause, evidence photography, panels and third-person viewing. Two reusable
  components of the existing 28 cm upper-sleeve mesh extend the visible garment
  toward the offscreen shoulders. No new mesh asset or component Tick was
  introduced for the fill or sleeve additions.
- First-person cell palm spacing is calibrated for the retained mesh.
  Only the thumb receives an outward adjustment of up to **22 degrees at
  45-degree FOV**, smoothly reducing to zero at 55-degree FOV; its root, palm
  and other digits retain the common grip frame. The closed-cube hardware
  grip's **5.6 cm palm gap** is an authored pose parameter.
- Strengthened carry checks measure actual rendered bone positions, require
  28/24 cm segment lengths within their 1.5 cm test tolerance and limit total
  shoulder-to-hand reach to **52.1 cm**. A synthetic moving/crouching carry pose
  exercises the body transform and torso-clearance constraints; it is not a
  completed mission, a human movement study or a performance result.
  The pause check snapshots the actual carried component's transform relative
  to the camera. Contact probes trace the world mesh's complex collision
  triangles and require the exact component and a valid triangle FaceIndex.
  All five fingertip queries on each hand must yield valid surface results,
  and at least one fingertip/contact per hand must be within 3 cm.
  The open/inward-facing `SM_SamplePouch` is explicitly tested by unsigned
  distance to actual triangles: its normals do not establish a closed solid
  volume. Catalogued solid grip regions retain signed-depth checks and fail
  when probed penetration exceeds 1.5 cm. Unsupported surface domains require
  an explicit test-domain decision rather than an assumed solid-volume claim.
  Protected cell-face clearance uses **66 samples of actual visible HandRig
  geometry and their depth**, rather than hidden forearm component bounds.
  These surface-sampled checks are not full pixel proof, certification of
  full-glove physical accuracy or proof of no intersection in every carried
  mesh and pose.
- NASA-derived Apollo terrain and work-site coordinates are unchanged.
  The player must travel to ALSEP; the lander field kit does not grant distant
  instrument progress.
- Both ALSEP tasks still need separate deliberate confirmations: center the
  seismometer bubble, then align and verify the Earth link. Their committed
  instrument mask drives the existing 0/2, 1/2 and 2/2 progression.
- Apollo movement rates are **140 cm/s walk and 220 cm/s fast stride**.
  SELENE/ARES rescue rates are **320 cm/s walk and 500 cm/s fast stride**.
  These are authored gameplay values. No additional sprint oxygen multiplier
  was introduced; the existing mission-resource rules remain authoritative.
- Product work and competition eligibility are separate. This document makes
  no claim of NASA endorsement or competition acceptance.

## Relevant files

- `Source/LunarRescue/LunarFrontend.cpp`: shared full-area action buttons.
- `Source/LunarRescue/LunarMissionCard.cpp/.h`: selected-card presentation,
  settled-state updates and layout validation.
- `Source/LunarRescue/LunarMissionSelectionChecks.cpp` and
  `LunarMissionSelectionReview.cpp`: selection/layout checks and review hooks.
- `Source/LunarRescue/LunarCrew.cpp`: bounded static-guide placement.
- `Source/LunarRescue/LunarCarryPresentation.cpp/.h` and
  `LunarSuit.cpp`: shared grip transforms and actual stride movement.
- `Source/LunarRescue/LunarApolloGuidance.cpp` and `LunarApollo.cpp`:
  truthful work guidance and registered-site/new-run guards.
- `Source/LunarRescue/LunarApolloRegression.cpp/.h`: isolated controller-input
  and physical-traversal regression driver.
- `Source/LunarRescue/LunarNasaPovChecks.cpp`: legacy POV regression updated to
  measure actual camera-relative pouch lowering, invariant cargo/resources and
  actual stow visibility. The cell fixture follows the production
  carry-update / suit-pose / carry-update order and checks both rendered HandRig
  wrists as well as the existing palm/contact-frame assertion.
- `Tools/verify_task_fixes.ps1`: new stride/Apollo checks with fresh semantic
  reports, source/module fingerprints and configuration preservation.
- `Tools/profile_mission_menu.ps1`: labelled home/picker CSV captures.

## Reproduction after building

Run from the repository root. First build/package using the repository's
existing workflow:

```powershell
Tools/package_apollo12.ps1 -MaxCompileActions 2
```

Run the new functional checks against the packaged executable:

```powershell
Tools/verify_task_fixes.ps1 -Packaged -Suite all
Tools/verify_task_fixes.ps1 -Packaged -Rendered -Suite all
```

The default invocation uses NullRHI. `-Rendered` uses DX11 rendering offscreen;
it is a functional run, not a performance measurement or an interactive human
playtest. The wrapper runs the stride suite on Apollo, SELENE and Mars, and the
Apollo interaction suite on Apollo. It restores the relevant original
configuration files and records the actual executable/module and source
fingerprints.

The Apollo driver uses `-ApolloInteractionRegressionTest`, fixed 1/60-second
simulation steps and uncapped execution. It uses actual controller keys,
normal CharacterMovement and collision to travel between sites. It must
complete pre-EVA checks, confirm the resource briefing, reach ALSEP from the
lander, operate the bubble and Earth-link controls, complete the historical
photo/sample/hardware mission and use R/map travel to restart. Fixed-step
simulation may execute faster than real time; it is not a graphics-performance
result. Temporary visibility obstacles and a rebound work key exercise negative
input paths without granting successful work. Presentation skipping does not
skip a task or a traverse.

Expected fresh semantic report names:

| Suite | Report |
| --- | --- |
| Apollo stride | `stride-regression-checks.txt` |
| SELENE stride | `stride-regression-checks.txt` |
| Mars stride | `mars-stride-regression-checks.txt` |
| Apollo interaction | `apollo-interaction-regression.txt` |

Per-run copied reports and `run-summary.json` are written beneath
`Reports/TaskFixes20261106`. A zero process exit alone is insufficient:
the wrapper also requires a fresh report containing PASS assertions, exactly
one `RESULT: 0 failures` line, and no FAIL assertion.

Rendered Apollo regression photographs use
`Saved/AutomatedTests/ApolloInteraction/Photos/<run-id>`; normal player
photographs and checkpoint files are not the regression destination.
The helper checks an in-memory version-1 checkpoint without additive field
data. An external old binary fixture is only checked when
`-ApolloInteractionLegacyFixture=<path>` is supplied directly to the
executable. That decode/validation and the empty-field-state contract are not
a claim that a complete legacy disk-resume scenario has passed.

Use existing independent regressions for the affected shared systems:

```powershell
Tools/verify_mission_selection.ps1 -Packaged
Tools/verify_mission_selection.ps1 -Packaged -Review -Width 1280 -Height 720
Tools/verify_mission_selection.ps1 -Packaged -Review -Width 1920 -Height 1080
Tools/verify_gameplay_tasks.ps1 -Packaged
Tools/verify_gameplay_tasks.ps1 -Packaged -Rendered
Tools/verify_apollo12.ps1 -Packaged -Checkpoints -Polish -Guidance -Fieldwork
```

Match a command's actual build, renderer and resolution to its dated receipt
in `verification-index.json`. Rendered screenshots also need visual inspection;
creating a PNG does not establish a correct grasp, label alignment or readable
HUD.

## Performance comparison protocol

The reported lag is interpreted as the picker screen pending more precise
reproduction. Keep that assumption visible in the final findings.

Use one newly built executable for the two blur-policy captures. Close other
game/editor/compiler processes and choose fresh labels:

```powershell
Tools/profile_mission_menu.ps1 -Label nov06-picker-reduced -Scene Picker -Frames 6000 -WarmupFrames 1500 -Width 1280 -Height 720 -Quality 1 -FpsCap 0
Tools/profile_mission_menu.ps1 -Label nov06-picker-fullglass -Scene Picker -Frames 6000 -WarmupFrames 1500 -Width 1280 -Height 720 -Quality 1 -FpsCap 0 -FullGlassBaseline
```

The diagnostic `-FullGlassBaseline` switch restores the earlier blur policy
inside the **same new build**. It does not reproduce every property of the old
executable, so this is a **same-build blur-cost experiment**, not a historical
before/after result.

An old-build **Home** capture is a distinct historical baseline. Compare it only
with a matching new-build Home workload and matching hardware, renderer,
resolution, quality, frame cap and warm-up policy. Do not compare that static
home baseline directly with scripted Moon/Mars picker browsing. The executed
matching new-build Home command is:

```powershell
Tools/profile_mission_menu.ps1 -Label nov06-after-home -Scene Home -Frames 1800 -WarmupFrames 300 -Width 1280 -Height 720 -Quality 1 -FpsCap 60
```

The capture records CPU, adapters, RAM, executable hash, DX11/offscreen settings,
CSV frame times and sampled process working set. `-WarmupFrames` defaults to
300. The planned Picker experiment uses 6,000 captured frames, excludes the
first 1,500 and runs uncapped (`-FpsCap 0`); the separate matching Home
comparison uses 1,800 frames, 300 warm-up frames and a 60 FPS cap. The
working-set maximum is a maximum of one-second samples, not true peak memory
or VRAM. A single capture, especially at a frame
cap, does not establish consistently smooth animation or whole-mission
performance. NullRHI results must not be used as graphics-FPS evidence.

## Verification status

The final local packaged executable is
`06c1b022f92b4e13e8300fc8d71296f290ee4f2565ac8597ebcd337519facda8`.
Both Unreal targets and staging completed successfully. Its canonical
validation set contains **4,092 C++ PASS executions / 0 failures in 65 raw
reports**: 2,683 in the full 16-suite regression queue, 99 in focused
stride/Apollo checks and 1,310 in two Field Operations runs. These are
executions across repeated missions, renderers and resolutions, not 4,092
unique tests or human participants.

The Editor selection test is a separate **66 PASS / 0 failures** with its
own game-module identity. A separate prequeue NASA smoke passed 277
assertions, including the corrected 30-assertion POV suite; those 277 are
retained as extra evidence and excluded from the canonical packaged total.

The index records **230 staged PNGs** by local path, hash and dimensions
and **8 paired pixel checks**. PNGs are local evidence and are not copied
into this documentation bundle. The two menu reviews each cover six action
records (three actions at 100% and 150% text), five sampled hit-test paths
per record and actual keyboard focus. The five sampled paths are not five
clicks. Separate real pointer routes select the three mission cards at each
review resolution.

The actual-input Apollo walkthrough completed 1,285.90 m of collision-enabled
travel and all historical mission stages before a real R/map restart.
Its fixed-step simulation time is not real-time or graphics-performance
evidence. The old external binary checkpoint fixture was not supplied to that
new focused driver; its memory/additive-state checks and the independent
checkpoint suites are distinguished in their reports.

Earlier 17a development receipts, including the two obsolete POV test-contract
failures, remain under `evidence/history` and are excluded from the final
executable's totals. The test-only correction changed one of 115
source/config/data fingerprint entries; production code did not change during
that correction. The final 115-entry fingerprint matches the verified source.

No human playtest, network-isolation test or whole-mission graphics-performance
claim follows from these suites. Performance captures have their own controls
and receipts and are not counted as C++ assertions.


## Closure records

[Performance comparison](performance-comparison.json) records the two historical
Home captures separately from the same-final-build picker blur-cost control.
Each condition has one capture; the observed picker difference is not a
whole-game performance guarantee. [Preservation](preservation-final.json)
confirms all 567 protected original files and all four maps by hash. Removed
old crash-reporter cache INIs and the editor timestamp were restored from
validated backups; gameplay saves and photos retained their original bytes.
