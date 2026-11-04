# Field operations implementation notes

Final local Windows validation: **4 November 2026**, executable SHA-256 `08485e289426127af57fca0bd79fa1b9bc1f67a3708f8d4385068140c8733a7a`. The [verification index](verification-index.json) records 4,018 C++ PASS executions / 0 FAIL, 67 raw reports, 280 staged PNGs and eight separate pixel pairs. Human playtests remain 0.

## UX and default controls

Begin a new selected mission normally. Complete the four ordered **Pre-EVA** actions, then use the hatch/start button. **Enter** reopens unfinished preparation. Surface departure opens the resource briefing; select and confirm a profile once, then inspect the nearby rover to unlock the magnifier.

| Default control / action | Behaviour |
| --- | --- |
| B | Open/close field operations; preparation reopens its own page. Unavailable while riding. |
| G | Toggle first-person magnifier after readiness, briefing and vehicle inspection. |
| Hold left click / release | Begin/interrupt training-label inspection while the magnifier is active. |
| Displayed work key, default E | Existing primary interactions have priority; otherwise interact with board/vehicle, pack inspected cartridge or exit vehicle. |
| Movement bindings, default W/S and A/D | Drive forward/reverse and steer while riding; normal movement otherwise. |
| Esc / F1 in a field panel | Close the owned panel and restore its prior pause relationship. |
| Resource / Inventory pages | Select a yellow-star support waypoint; recharge utility or transmit stored training data at the linked board; deposit carried cartridges near the lander. |

A closed panel and stale button generation cannot authorize retained actions. Reopening the confirmed briefing cannot change difficulty or refill a resource. Preflight is a briefing-state action, not an active EVA resource countdown; explicit pause freezes its current check. Active field panels pause normal resource consumption.

## Source responsibilities

| Files | Responsibility |
| --- | --- |
| [LunarFieldOperations](../../Source/LunarRescue/LunarFieldOperations.cpp) / [header](../../Source/LunarRescue/LunarFieldOperations.h) | Readiness, difficulty, resources, interaction priority, inventory, map points and summaries. Component has no independent Tick; GameMode advances it. |
| [LunarOperationsWork](../../Source/LunarRescue/LunarOperationsWork.cpp) | Circuit pairing, magnifier, sight/range/hold validation, once-only inspection cost and prop visibility. Existing `LunarFieldwork.cpp` is a separate Apollo module. |
| [LunarFieldWorld](../../Source/LunarRescue/LunarFieldWorld.cpp) | Bounded authored board, delivery props and magnifier geometry on retained terrain; no terrain regeneration or new binary model import. |
| [LunarSurfaceVehicle](../../Source/LunarRescue/LunarSurfaceVehicle.cpp) / [header](../../Source/LunarRescue/LunarSurfaceVehicle.h) | Authored kinematic vehicle, terrain support, collision sweep, rider/camera restoration and safe parked-pose validation. |
| [LunarOperationsPanel](../../Source/LunarRescue/LunarOperationsPanel.cpp) | Scrollable preparation/briefing/resource/vehicle/wire/inventory/source pages; actor map positions and guarded callbacks. |
| [LunarFieldState](../../Source/LunarRescue/LunarFieldState.cpp) | Bounded schema-1 checkpoint serialization and transactional core validation. |
| [LunarFieldChecks](../../Source/LunarRescue/LunarFieldChecks.cpp), [integration checks](../../Source/LunarRescue/LunarFieldIntegration.cpp) | Isolated model/corruption fixtures and owned-panel/world integration checks. Vehicle and rendered checks are separate. Source presence is not successful execution. |

The [science assessment](SCIENCE_RESEARCH.md) contains authored costs, capacities and the verified local source receipt. The [story/reference guide](STORY_AND_REFERENCES.md) explains historical versus fictional boundaries.

## State contract

The optional Apollo checkpoint text uses schema `1`, profile `field-operations-training-v1`, source identity `lcross-cabeus-published-estimate-v1`, and a matching `apollo12` / `selene` / `ares` mission identity. Parsing is bounded to 16 KB of UTF-8 and shallow JSON. A local snapshot validates before live fields, actors, cameras or mission capacity are changed.

Scalar/coordinate numeric values require `EJson::Number`; flags require `EJson::Boolean`, because Unreal conversion helpers also accept string values. Quoted numeric/Boolean lookalikes are rejected. Validation includes finite/bounded resources and telemetry, profile capacities, four-check readiness, required wire bits, `delivered ⊆ collected ⊆ scanned`, three known packages and cargo mass derived as `0.5 × undelivered package count`. Remaining native oxygen is not duplicated in the blob; the main checkpoint remains authoritative, then remaining is clamped to the validated effective capacity, at most 1800 gameplay seconds.

Pending checks, selected ports, pending wire application, scanning/held progress, active magnifier, preview timers and UI/waypoint selection are not persisted. Valid import cancels transient presentation. Empty legacy data disables/hides the optional extension without changing primary cargo/resource; invalid optional data is handled separately from the valid main checkpoint. An invalid/blocked/off-terrain parked pose keeps default/current vehicle parking rather than rejecting historical progress. Abort any existing ride before applying the main restored pawn position.

SELENE/ARES share the component and can be exercised by its serializer tests, but no new persistent rescue save system is claimed. Fresh-run restart clears the extension and restores its baseline before applying difficulty again.

## Limits and reproduction

The rover is an authored proxy, not the downloaded NASA SEV/LRV model or a rigid-body dynamics simulation. Its 18-degree default is a game traversal threshold, not a crew safety standard. Containers use carried/deposited inventory flags rather than a physics inventory or visible rover-bed loading. Label inspection is optical framing plus authored metadata; it performs no real spectroscopy, drilling or local water detection. The robots do not navigate or collect autonomously.

Package placement tries at most 25 authored candidates against the retained terrain, a clear prop volume, a clear standing capsule and at least one line-of-sight viewing position. No terrain geometry or measured dataset is modified. Failure omits that optional prop; a valid observation spot is not proof that every approach or full route is traversable.

The map provides support markers and a direct-distance forecast, not a terrain-aware two-route planner. Finance modelling, a new historical Apollo 17 terrain/mission, further cinematic dialogue and licensed vehicle/suit replacements remain future work. LCROSS publication facts are reused without new raw data or invented results. Missing source data disables optional inspection rather than fabricating a measurement.

Automation flags are `-FieldOperationsTest` and `-FieldOperationsReview`; `-FieldOperationsProfile` is a staged profiling setup, and `-NoFieldOperations` is the feature-off baseline. Established automation guards suppress ordinary owner-save paths. Reproduction commands after building:

```powershell
Tools/verify_field_operations.ps1 -Packaged
Tools/verify_field_operations.ps1 -Packaged -Rendered
Tools/verify_field_operations.ps1 -Packaged -Review -Width 1280 -Height 720
Tools/verify_field_operations.ps1 -Packaged -Review -Width 1920 -Height 1080
Tools/verify_gameplay_regression.ps1
```

The final new functional/review coverage contributes 1,310 + 308 C++ passes and 120 PNGs; retained legacy coverage contributes 2,400 passes from 55 reports and 160 PNGs, with 16/16 suites passing. Eight carry-pixel pairs are a separate rendered-contribution metric. Counted assertions are repeated executions, not unique tests; source-model checks, actual-world interactions and rendered layout checks have different scopes. See the [verification index](verification-index.json), [legacy receipt](legacy-verification.json) and [canonical report](../../TEST_REPORT.md).

Final checks exercise gates, resources, pause/restart, both rescue wire flows, scanner atomicity/LOS, actual pickup/deposit, strict SaveGame/legacy state, vehicle obstruction/exit and mission regressions. The 720p/1080p staged images include 150% text. Three [performance comparisons](performance-comparison.json) cover menu and opening workloads only; occupied driving, scanner/board benchmarks and whole-mission FPS stability remain unmeasured. NullRHI is not graphical performance. The sampled placement viewpoints do not prove an end-to-end traversal.

[Preservation](preservation-final.json) records 84/84 protected files matching after verified restoration of the editor timestamp change. Human playtests remain 0; no firewall/network-isolation test was performed. These are local/unreleased quality receipts, not NASA endorsement or confirmed Space Apps eligibility.
