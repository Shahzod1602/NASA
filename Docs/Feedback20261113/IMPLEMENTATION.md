# NASA group feedback implementation — local development

Intake: 13–14 November 2026. Base: `1b3bd3716a2f38d138bb084ae89c10e7ad3f267c` / published `apollo12-v0.8.2`. This work is not a published release. The private Telegram history and credentials remain outside this repository.

## Mission entry and field operations

Four sequential authored suit checks report their actual 0–100% progress. Each completed check requires **Next** before the following check. **Open Hatch** becomes available after all four. The mission/resource clock remains stopped during preflight. Map and Inventory remain available without awarding equipment or starting the mission. Progress changes update retained widgets instead of rebuilding the page each frame.

Departure keeps the existing player view and FOV, opens the hatch over a short presentation, and hands back the exact player view. It does not fly the camera through the cabin, teleport the player or spend oxygen. Active play uses the balanced gameplay resource profile without an automatic extra profile-selection modal.

The map supports bounded zoom, drag and tracking of actual local world locations. Apollo's retained LROC map remains the local basemap. The optional rotatable planet overview uses offline NASA colour images and authored sphere shading. It is not a new elevation dataset. Fictional SELENE/ARES locations are not assigned invented planetary coordinates.

Inventory filters actual Tools, Supplies and Samples, shows current counts and respects existing tool unlock, proximity and cargo rules. It does not invent consumables, refill oxygen or award samples.

## Cable reference interaction

The rescue cable panel presents a dark, bounded circuit board with colour plus printed signal identifiers and shapes. Players isolate the toy circuit, drag a source cable to its matching socket, then press **Verify**. Invalid/outside drops do not complete a connection. Matching all cables does not advance the mission until verification. Verification applies the existing cable-repair step once; the player must still restart the station through the existing hold interaction. An expandable click/keyboard alternative retains source/socket controls. Apollo ALSEP deployment remains a separate historical task.

The operations component remains authoritative. Widget presentation cannot bypass mission stage, ownership, physical reach, line of sight or completed-state guards. This is an authored matching exercise, not a real electrical repair procedure.

## World and models

- The supplied LasquetiSpice suit has separate body and first-person derivatives targeting the existing 36-bone controller. The original floating clip is retained in the unchanged raw source and is not used for ground walking. Torso/accessory branches explicitly follow anatomical torso anchors; a root-folding error found during the first render was corrected. Arm-axis retargeting aligns source elbow/wrist/finger endpoints to controller bind pivots, with inverse-transpose normal correction; this fixes sleeves overlapping palms. Lower-centre cell framing retains visible gloves and clears the central view. The original A7L assets remain available as fallback.
- Moon training vehicles use Deutsches Museum's credited lunar rover body and four wheel pivots. The existing swept kinematic controller supplies acceleration, braking, steering, resource cost, terrain sampling and enter/exit. Source geometry is preserved in derivatives; four authored LODs bound rendering cost. The imported body fits its swept hull; clearance, turn padding and parked overlap queries use that same hull. This is not a validated lunar vehicle physics model. Mars retains its own training vehicle.
- Fictional SELENE's NOMAD-7 appearance uses the official NASA/JPL Perseverance visual reference. The original mission actor/anchor, controls and rewards remain intact. **Perseverance is a Mars mission, not historical Moon hardware.** Its original PBR assignments and UVs are retained; the rejected oversized atlas import was backed up and replaced during development.
- Base modules and solar supports are positioned against sampled terrain. Authored support feet bridge remaining uneven gaps. Solar panels have a conservative query collision envelope. Their imported plane is `z + y = 95.000818 cm`; a reversed slope sign in underside attachment placement was corrected and checked against source-mesh line of sight. The power panel gains terrain-tested support legs without moving its installed-cell/cable anchors. Measured Apollo terrain is not flattened or regenerated.
- PIP-3 and unused delivery-robot decorations are retired. The optional functional Mira communicator, mission objectives, cartridges and notebook guidance remain. Redundant route stakes and duplicate floating labels are hidden with targeted Editor changes to a backed-up MoonBase map.

## HUD clarification still pending

The initial request to remove the duplicate lower field-guide card has a later ambiguous “qolib tursin bu” follow-up. A clarification was sent to Boburjon in NASA as message 1212. Current local source hides the duplicate card while retaining the objective-line interaction prompt and actual tool/hold guidance. Do not describe the final card preference as confirmed until a reply is received.

## Validation boundaries

Important changed source: `LunarFieldOperations.cpp/.h` and `LunarFieldState.cpp` (ordered checks and authoritative inventory/cable guards); `LunarOperationsPanel.cpp/.h` (retained progress, map/inventory/board UI); new `LunarWireBoard.cpp/.h` (drag presentation); `LunarDeparture.cpp/.h` (stable hatch); `LunarSurfaceVehicle.cpp` and `LunarVehicleChecks.cpp` (model, measured hull and driving checks); `LunarRig.cpp`, `LunarCarryPresentation.cpp` and `LunarCarryChecks.cpp` (suit/grip); `LunarEquipmentDetail.cpp`, `LunarExpedition.cpp`, `LunarCrew.cpp`, `LunarFieldWorld.cpp` and `LunarFlightHUD.cpp` (grounding, NOMAD and relevant scene/HUD cleanup). Dated `SourceArt`/`Content` directories contain model derivatives and manifests; `Tools/prepare_*`, `import_*` and `package_apollo12.ps1` provide preparation/import/offline-credit packaging.

Native Editor builds and runtime assertions, rendered UI/model reviews, packaged checks, performance measurements and human playtests are separate evidence. Consult the dated test report for actual commands and results. Synthetic review fixtures do not prove a human completed the mission. No human playtest results are fabricated. No GitHub push or release is implied by these local changes.

The fresh local Windows candidate is `953d5f4abaef…`: 2,019 packaged assertion executions / zero FAIL, 20 raw case reports and 109 staged PNGs. The selected short performance comparison does not establish a sustained improvement: mean/p95 were similar, but 1% low and the largest sampled frame were worse. See `Evidence/LOCAL_VALIDATION.json` and the dated test report before interpreting these numbers.
