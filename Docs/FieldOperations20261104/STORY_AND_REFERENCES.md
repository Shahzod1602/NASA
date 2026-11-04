# Field operations: story, gameplay and references

Source and validation review: **4 November 2026 (Asia/Tashkent)**. Final local Windows executable `08485e289426127af57fca0bd79fa1b9bc1f67a3708f8d4385068140c8733a7a` passed the scoped automated checks in the [verification index](verification-index.json). This is independent product development with real November dates. Competition-specific submission work remains backlog while challenge fit, timing and prior-work permission are unresolved; see [SCIENCE_RESEARCH.md](SCIENCE_RESEARCH.md).

## Premise and mission boundaries

Prepare the systems, inspect the equipment, choose a sortie budget and return a clearly identified delivery. Preserve three distinct identities:

| Mission | Historical / fictional boundary | Field-operation role |
| --- | --- | --- |
| Apollo 12 — Surveyor's Legacy | Oceanus Procellarum terrain and existing historical sample/hardware objectives; modern LROC measurements are later data. Apollo 12 did not use a lunar rover. | A labelled modern training overlay, with authored transport and remote-delivery cartridges separate from historical cargo. No local water discovery is introduced. |
| SELENE | Authored Moon rescue station and terrain. | Fictional readiness, vehicle support, utility management, package delivery and circuit matching during the existing cable-restoration task. |
| ARES | Authored Mars expedition and terrain. | Fictional SEV-inspired transport and equipment operations, not a crewed rover actually deployed on Mars. |

Dr. Mira Vale remains a fictional modern communicator and PIP-3 an authored guide display. New cartridge delivery robots are static authored props, not autonomous collectors or following NPCs. No character represents a real NASA employee or historical Apollo crew member.

## Player flow

1. Select a destination and mission through the existing selection/loading flow, then begin the briefing. The field extension opens **Pre-EVA / Training Checks** in the authored departure setting.
2. Complete **Inspect suit seals**, **Attach PLSS life-support pack**, **Check communications** and **Select internal suit power**, in order. Each short timed game action commits once. Hatch/start remains gated until all four are complete; closing the panel does not complete them. **Enter** reopens pending checks.
3. Use **Checks complete / open hatch and begin surface operations**. Departure hands off to the surface briefing. Choose a gameplay resource profile, review its proposed budget and confirm once. Reopening cannot refill or change the locked profile.
4. Approach and face the training vehicle, open its **Vehicle** page and commit **Inspect vehicle systems**. This unlocks the magnifier. Entry still needs proximity and unblocked access; inspection does not teleport the player into a seat.
5. Follow existing primary mission work and optionally inspect/pack training cartridges. A board waypoint leaves the primary mission target unchanged. Restore the native power/communication link before using the board's data transfer or utility recharge.
6. Return to the lander to deposit carried training packages. Training inventory is independent of historical Apollo samples/hardware, the existing optional recorder and the remote water-evidence report.

Default controls are **B** for the board, **G** for the magnifier, held **left click** for inspection, the displayed work key (default **E**) for interactions and movement bindings (default **W/S**, **A/D**) for drive/steer. **E** exits on a validated nearby ground position; a blocked exit leaves the rider inside. **Esc/F1** closes field panels; ordinary pause remains available. See [IMPLEMENTATION.md](IMPLEMENTATION.md) for developer details and validation boundaries.

## Five implemented feature threads

### Electrical matching

For SELENE/ARES cable restoration, isolate the toy circuit, choose a left source, then the right socket with the same printed identifier. Port codes/signal names are the rule; colour is secondary. Difficulty selects two, three or four required pairs. Wrong choices count an attempt while preserving completed connections; unfinished work can be reset. Repeating a connection cannot award progress twice.

The final valid match requests the ordinary native cable interaction, which must still validate the current mission stage and physical target. The component does not independently award the whole power-restoration sequence. Apollo keeps its historical tasks, with no invented historical wire-repair objective. This is an authored logic puzzle, not a NASA electrical procedure.

### Readiness and material-label inspection

Readiness states are simplified teaching actions. NASA's [spacesuit overview](https://www.nasa.gov/centers-and-facilities/johnson/spacewalk-spacesuit-basics/) supports the roles of pressure, life support, cooling, power and communication; the [Apollo PLSS account](https://www.nasa.gov/wp-content/uploads/static/history/alsj/ALSJ-FlightPLSS.pdf) supplies historical context. Neither validates one-second game checks or a flight-ready certification.

The magnifier narrows first-person camera FOV to 35 degrees and shows an authored optical-ring prop. Aim at a visible training cartridge within 4.5 m and hold left click for 1.2 s. First completion records its label, spends 0.8 utility RU and adds 0.4 MB of **simulated training metadata**. Looking away, releasing early, insufficient charge or full storage does not award a record. It measures no composition, water abundance or hardware health. Pause, other panels, riding, native tools and carried rescue cells suppress/cancel this view and restore configured FOV.

### Planet-specific vehicle presentation

The authored Moon vehicle has an open lunar-rover-inspired presentation; ARES has an authored SEV-inspired enclosure. Both use retained basic meshes/materials rather than a newly imported NASA vehicle model. Driving uses sampled terrain, a swept hull, a game slope limit and safe entry/exit checks. It is a bounded kinematic model, not a validated physical rover simulator.

NASA's [Apollo 15 record](https://www.nasa.gov/mission/apollo-15/) and [Apollo 17 record](https://www.nasa.gov/mission/apollo-17/) establish the historical distinction. NASA JSC describes [SEV](https://www.nasa.gov/software-robotics-and-simulation-division/robotic-systems-technology-branch/) as a terrestrial-tested prototype. Authored placement, handling and thresholds do not become measured performance through visual inspiration. A future historical Apollo 17 mission requires its own site and reconstruction; this update does not rename Apollo 12 terrain Taurus-Littrow.

### Surface Operations board and map

The board shows local gameplay state: oxygen/capacity, elapsed time, walking/vehicle distance, utility charge, training cargo, stored/transmitted data, native communication-link state, readiness and rover status. Its direct-distance forecast uses configured movement speed and RU/m, with a warning that obstacles and grades are ignored.

Yellow stars represent actual world actors. Apollo can use the retained orbital map when coordinate coverage supports it; other cases use a local schematic. Choosing a support waypoint leaves primary mission work unchanged. No water-deposit map, safety rating, two-route slope optimiser or orbital launch simulation is introduced.

Beginner, Intermediate and Professional are authored profiles, with exact capacities/costs in [SCIENCE_RESEARCH.md](SCIENCE_RESEARCH.md). They are not NASA safety classes. Finance/budget simulation is deferred. Resource themes have a possible relationship to **Space Mission Design Game**, but the complete current official statement was unavailable; no challenge-fit claim is made.

### Remote-delivery training cartridges

Three delivery targets are authored near existing anchors. Each target searches at most 25 candidate sites on retained terrain, requiring prop clearance and a clear capsule/line-of-sight observation position. If none fits, that optional prop is omitted rather than embedded in geometry. This site check does not flatten terrain or prove every approach is traversable. The sequence is **visible package → inspected label → carried inventory → deposited at the lander**. Pickup requires inspection and carried-mass capacity, and hides that world cartridge. Deposit retains its history while removing carried mass. Repeated pickup/deposit cannot duplicate it. Rover travel transports the player and inventory; it does not place visible containers in a cargo bed.

Visible wording is **SIMULATED REMOTE DELIVERY / training cartridge**. Existing **Water Research** remains the separate real-publication exercise about **LCROSS / Cabeus / 2009**. Its local record contains no raw/fabricated spectrum or new NASA result. Label inspection references that source without changing its measurements.

Water may support future life-support and propellant systems after suitable processing. Moving these packages demonstrates logistics, not extraction, purification or fuel production. Sources: [NASA ISRU technology](https://www.nasa.gov/lunar-surface-technology/) and [LCROSS](https://science.nasa.gov/mission/lcross/). No pickup refills suit oxygen, certifies potable water, produces fuel or proves water at Apollo 12 or the fictional ARES location.

## Narrative and reference ledger

Current text explicitly describes authored checks, toy connections, game budgets, prototype inspiration and simulated delivery. A longer dramatic scene/dialogue script is future content, not a completed cinematic campaign or historical conversation. Existing fictional guides keep scripted checklist dialogue; they are not a live science service.

Source-backed elements are suit-system functions, Apollo history, SEV prototype identity, LCROSS estimates and general ISRU motivation. Check order, circuit labels, vehicle geometry/handling, overlay, containers, placement, coefficients, storage sizes and difficulty profiles are authored/simplified content.

Model references and unresolved LRV/suit/Surveyor replacement gaps are in [SCIENCE_RESEARCH.md](SCIENCE_RESEARCH.md) and the [asset catalogue](../../ASSET_SOURCE_CATALOG.md). Retain [external model credits](../../SourceArt/ExternalModels/CREDITS.md) and [water provenance](../../SourceArt/LunarWater/CREDITS.md). The [official SEV GLB](https://science.nasa.gov/3d-resources/space-exploration-vehicle/) remains a future candidate; a research link is not an import receipt, rig or performance result.

## Persistence and verification evidence

Apollo's optional schema-1 blob records committed masks, resources/telemetry, profile/source/mission identity and optional parked-vehicle transform. Numeric and Boolean fields require actual JSON Number/Boolean types; quoted lookalikes are rejected. Transient actions, scanning and UI selection reset on import. Invalid optional data must not reject historical cargo; older empty checkpoints disable the extension for that continued run. SELENE/ARES progress is session state unless a separate persistence path is added.

Final evidence records **4,018 C++ PASS executions / 0 FAIL across 67 raw reports**, **280 staged PNGs**, and **8 separate paired carry-pixel checks**. This comprises 1,310 new functional passes, 308 new review passes and 2,400 retained regression passes; the 16 legacy suites all passed. These are repeated scripted executions, not unique test counts or human playtests. See the [verification index](verification-index.json), [legacy receipt](legacy-verification.json) and [TEST_REPORT](../../TEST_REPORT.md) for the exact final hash and coverage.

Gates, real rescue wire callbacks, scan sight/resource atomicity, pause, pickup/deposit, strict save/legacy handling, blocked vehicle exits, restart and mission regressions were exercised. Fresh 720p/1080p frames include 150% text and actual layout checks. The [three performance comparisons](performance-comparison.json) cover menu and Apollo opening workloads; they do not establish occupied-driving/board performance or whole-mission smoothness. Cartridge placement observation checks are not full-route traversal proof.

Human playtests remain **0** and firewall/network isolation was not tested. Proxy art, visible hand/driver limitations and future terrain-aware routing remain separate work. [Preservation](preservation-final.json) records 84/84 protected files matching after the verified editor timestamp restoration. Competition eligibility remains unconfirmed; these quality results do not grant contest permission.
