# Field operations: science and source assessment

Research date: **4 November 2026 (Asia/Tashkent)**. The features in [STORY_AND_REFERENCES.md](STORY_AND_REFERENCES.md) and [IMPLEMENTATION.md](IMPLEMENTATION.md) were built and checked on the final local Windows executable `08485e289426127af57fca0bd79fa1b9bc1f67a3708f8d4385068140c8733a7a`. See the [verification index](verification-index.json) and [canonical test report](../../TEST_REPORT.md) for the exact scope. This remains independent product development, not a Space Apps submission or eligibility approval.

## Space Apps 2026: what was and was not verified

NASA's [Prizes, Challenges, and Crowdsourcing page](https://www.nasa.gov/prizes-challenges-and-crowdsourcing/) currently identifies **14–15 November 2026** as the Space Apps dates. A current event date does not establish this project's eligibility.

| Question | Evidence retrieved on this research date | Result |
| --- | --- | --- |
| Exact challenge behind the proposed mass/power/data/comms/time board | The official [2026 directory](https://www.spaceappschallenge.org/2026/challenges/) and candidate [Space Mission Design Game](https://www.spaceappschallenge.org/2026/challenges/space-mission-design-game/) could not be opened: 502/access errors. An independent guide links that candidate title, but is not the official statement. | **Unconfirmed.** The complete official challenge and resources were not read. No requirement-by-requirement match or claim of compliance is made. |
| Current participant terms | The search service returned the official [2026 Participant Terms](https://www.spaceappschallenge.org/legal/), headed “Updated August 7, 2026”; direct opening failed. | Official indexed text was readable, rather than a successful live page refresh. It requires a directly addressed challenge, rights to submitted material, source attribution, and unrestricted/OSI-licensed original content. It describes an Apache 2.0 default if a licence is omitted; this does **not** change this repository's licence. |
| Work before the event / reuse of this game | The current global [FAQ](https://www.spaceappschallenge.org/resources/participant-faqs/) was unavailable. The [2026 Santiago local-event guidance](https://www.spaceappschallenge.org/2026/local-events/santiago-dominican-republic/) says intended competition solutions must be developed during the event, while learning and preparation may precede it. Local-event pages contain Local Lead content, not a substitute for global policy. | No current global reuse exception for LunarRescue was verified. The September FAQ audit in [COMPETITION_READINESS](../../COMPETITION_READINESS.md) remains historical evidence, not a fresh reading. |
| Demo length, slide count, judging and approved reuse scope | No complete current submission/judging guide was read. | **Unconfirmed.** Do not apply previous-year limits. |

Keep challenge-specific submission implementation in the competition backlog until the current full statement and permitted work window are established. Continue separately authorised product work with its real November dates, source history and pre-existing assets disclosed. Calling it an overlay, publishing source, or separating old and new work does not by itself make either eligible. Do not submit the existing game as work produced solely during the hackathon.

Organizer question, prepared but **not sent**:

> May our team reuse LunarRescue's pre-existing code, terrain, assets and independently developed product features through 4 November 2026, retaining their actual dates and disclosing all prior work? What may be judged, and when may challenge-specific implementation begin? Please identify the applicable full 2026 challenge, reuse/open-source conditions and current presentation limits.

The cited resource concepts are useful game-design concerns even without a competition match. The implemented numerical coefficients and three difficulty settings are authored tuning, rather than NASA-derived mission requirements.

## EVA suit and life support

NASA's [Spacewalk Spacesuit Basics](https://www.nasa.gov/centers-and-facilities/johnson/spacewalk-spacesuit-basics/) explains pressure containment, breathing oxygen, circulation and carbon-dioxide removal, electrical supply, communications and liquid cooling. This is general explanatory material; its descriptions of newer suit designs must not be treated as an exact Apollo A7L parts list. The NASA-hosted technical account [The Apollo Portable Life Support System](https://www.nasa.gov/wp-content/uploads/static/history/alsj/ALSJ-FlightPLSS.pdf) separately describes the Apollo PLSS and emergency oxygen equipment.

These sources justify teaching that EVA readiness depends on several cooperating systems. They do not validate the implemented four-action checklist, its order, game timer, suit endurance, or invented pressure/current reading. **Inspect suit seals**, **Attach PLSS life-support pack**, **Check communications** and **Select internal suit power** are explicitly simplified game states. A passed check means that the corresponding game action was completed. It is not a medical, leak-test or flight-readiness certification. Native and difficulty-adjusted resource clocks and pauses remain gameplay rules.

The electrical matching activity is an authored logic puzzle. Terminal names, connection order and feedback are fictional training design, not a NASA wiring diagram or real repair procedure. Material inspection records authored package labels and cited background context; the magnifier does not measure composition, water content or hardware health.

## Historical Moon vehicles versus modern training vehicles

NASA's [Apollo 15 mission page](https://www.nasa.gov/mission/apollo-15/) identifies that flight as the first lunar-rover mission. NASA's [LRV history](https://nssdc.gsfc.nasa.gov/planetary/lunar/apollo_lrv.html) identifies the vehicles used on Apollo 15, 16 and 17. The [Apollo 17 mission record](https://www.nasa.gov/mission/apollo-17/) places its landing in Taurus-Littrow in December 1972.

Apollo 12's Surveyor 3 visit and hardware return are supported by NASA's [Apollo 12 and Surveyor 3 record](https://science.nasa.gov/resource/apollo-12-and-surveyor-3/). They do not include an Apollo 17 rover. Retain the present Oceanus Procellarum identity and historical cargo flow. An LRV-inspired drivable addition there must be a separately labelled **modern training overlay** with authored placement, handling and objectives. Do not rename that terrain Taurus-Littrow or describe the overlay as a vehicle present in 1969. A future historical Apollo 17 mission needs its own site, data and mission reconstruction.

For Mars, NASA JSC describes the [Space Exploration Vehicle](https://www.nasa.gov/software-robotics-and-simulation-division/robotic-systems-technology-branch/) as a Moon/Mars pressurised rover **prototype**, used in terrestrial field tests. The [NASA SEV model page](https://science.nasa.gov/3d-resources/space-exploration-vehicle/) describes a flexible exploration concept. An ARES implementation is a fictional deployment inspired by a concept/prototype, not a crewed vehicle proven or operated on Mars. Model-page concept capabilities are not validated endurance, speed or occupancy settings for this game.

## Water evidence versus a transported training package

The existing optional remote study is grounded in [Colaprete et al. (2010), DOI 10.1126/science.1186986](https://doi.org/10.1126/science.1186986), its [official PubMed abstract](https://pubmed.ncbi.nlm.nih.gov/20966242/) and [NASA's LCROSS mission account](https://science.nasa.gov/mission/lcross/). The 2009 Cabeus impact observations supported lunar water evidence. The published **155 +/- 12 kg** concerns water vapour plus ice in the observing field; **5.6 +/- 2.9 percent by mass** is the impact-site estimate based on the observable excavated material. Neither is a whole-Moon inventory, guaranteed extraction yield, or water measurement at Apollo 12. Preserve the stated uncertainty without assigning an unverified confidence level.

The new pickup/container objective uses **SIMULATED REMOTE DELIVERY** and training-package wording. Authored containers have inspection, pickup, inventory and base-deposit state, but their contents are not new NASA samples or in-situ detections. The rover transports the player; containers are recorded as a carried inventory, not physically attached to a vehicle cargo bed. Their authored mass is not added to the LCROSS result. Logistics and the optional evidence report remain separate from historical Apollo cargo.

NASA's [lunar-surface technology overview](https://www.nasa.gov/lunar-surface-technology/) describes ISRU work involving collection, processing, storage and use of local materials. It supports discussing potential life-support and propellant applications. It does not show that this game's box is potable, supplies suit oxygen directly, produces fuel, or has a validated processing efficiency. Fuel production requires processing and an appropriate system; transporting a sealed training package is not an extraction demonstration. No extraction temperature, purity, conversion yield or local deposit claim is introduced here.

## 3D sources and rights: references are not imports

The [NASA 3D hub](https://science.nasa.gov/3d-resources/) points to downloadable models and the [NASA media-use guidelines](https://www.nasa.gov/nasa-brand-center/images-and-media/). NASA-origin model data are generally usable for educational/informational purposes under those guidelines, with credit and without implied endorsement. NASA-hosted third-party work can retain separate rights; NASA logos and identifiable people require separate consideration. No project or third-party licence is replaced by this assessment.

| Candidate / retained asset | Primary source, format and known size | Status and constraint |
| --- | --- | --- |
| Space Exploration Vehicle | [NASA / LaRC / Advanced Concepts Lab](https://science.nasa.gov/3d-resources/space-exploration-vehicle/); [official GLB](https://assets.science.nasa.gov/content/dam/science/cds/3d/resources/model/space-exploration-vehicle/Space%20Exploration%20Vehicle.glb); source page advertises **21.39 MB**. [NASA repository folder](https://github.com/nasa/NASA-3D-Resources/tree/master/3D%20Models/Space%20Exploration%20Vehicle). | Download link and item metadata verified. No binary downloaded or imported by this research. Topology, scale, wheel pivots, collision, material dependencies and licensing notices still need inspection before use. |
| Apollo LRV visual reference | NASA's [Virtual Lunar Roving Vehicle](https://history.nasa.gov/wp-content/uploads/static/history/alsj/VirtualLRV.html), by Don McMillan. | The page explicitly credits copyright to its creator and permission for NASA's use. That is **not** blanket game redistribution permission. No reusable official LRV mesh was verified in this pass. Use authored geometry with honest “LRV-inspired” wording or obtain a separately cleared model. |
| Apollo Lunar Module and HDU | [Apollo LM](https://science.nasa.gov/3d-resources/apollo-lunar-module/) and [Habitat Demonstration Unit](https://science.nasa.gov/3d-resources/habitat-demonstration-unit/). | Already imported/reused in this project; see [existing model credits](../../SourceArt/ExternalModels/CREDITS.md). Reuse does not establish an accurate Intrepid interior or a new model acquisition. |
| Apollo field tools | NASA's [Spacewalking Tools](https://www.nasa.gov/suits-and-rovers/spacewalking-tools/) provides Apollo 17 replica STLs. | Static untextured geometry candidates, not automatically rigged tools or exact Apollo 12 variants. Prior verified sizes and downloads are in the [asset catalogue](../../ASSET_SOURCE_CATALOG.md). |
| Suit, Surveyor, ALSEP, sample rocks | [Existing bounded asset-source assessment](../../ASSET_SOURCE_CATALOG.md). | Smithsonian suit restrictions, unresolved Surveyor/ALSEP mesh gaps and sample identities still apply. Do not relabel retained authored meshes as newly scanned NASA assets. |

No model checksum, successful Unreal import or asset-equivalence claim was established by this text-only research. Selected future imports need a source/date/hash/credit receipt and a supported editor pipeline.

## Implemented game tuning and local source receipt

The shared utility resource uses **gameplay RU**, not suit oxygen, battery watt-hours or a medical quantity. Values below come from [the component](../../Source/LunarRescue/LunarFieldOperations.cpp), [inspection work](../../Source/LunarRescue/LunarOperationsWork.cpp), and [vehicle tuning](../../Source/LunarRescue/LunarSurfaceVehicle.h).

| Rule | Authored value / scope |
| --- | --- |
| Four ordered pre-EVA checks | 1.0 s and 0.2 RU each; completion records one check once. |
| Idle utility drain | 0.015 RU/s while the mission is active, briefed and unpaused. |
| Actual rover movement | 0.012 RU/m in addition to idle drain; the movement model records accepted travel. |
| Training-label inspection | A held 1.2 s action; 0.8 RU and 0.4 MB only on successful first completion. Visible package within 4.5 m, facing dot at least 0.94, and clear sight required. |
| Cartridge mass | 0.5 kg per carried, undelivered training cartridge; authored mass, not measured water mass. Three world packages are tracked. |
| Board downlink | 1 RU at start, 2 s stay-at-board operation; completed transfer frees local training-data storage. Mission time and oxygen continue while it runs. |
| Board recharge | Adds up to 20 RU, capped at capacity, after 3 s at the board; concurrent idle drain still applies. This is fictional utility transfer, not suit oxygen production. |
| Vehicle defaults | 4.5 m/s forward, 1.7 m/s reverse, 18-degree game slope limit; editable authored movement parameters, not astronaut/rover safety ratings. |
| Map and forecast | Yellow stars use actual actor positions. Direct-distance estimates use configured movement speed and RU/m, but ignore obstacles, acceleration and grades. They are not optimised NASA traverses. |
| Authored package sites | Up to 25 bounded candidate positions on existing terrain; prop clearance, a clear standing capsule and at least one unobstructed observation view are checked. An unsuitable optional site is omitted, without moving or flattening terrain. This does not prove a complete reachable traversal route. |

| Profile | Utility capacity | Local data | Carried mass limit | Wire pairs | Native resource multiplier |
| --- | --- | --- | --- | --- | --- |
| Beginner | 150 RU | 4 MB | 2 kg | 2 | 1.5, capped at 1800 gameplay seconds; Apollo is untimed. |
| Intermediate | 100 RU | 1.2 MB | 1 kg | 3 | 1.0; Apollo is timed. |
| Professional | 80 RU | 0.8 MB | 0.5 kg | 4 | 0.8; Apollo is timed. |

These are capacities, not necessarily remaining charge: the four preflight checks spend a total 0.8 RU, retained when the selected capacity is applied. The profile is locked for the attempt. Paused mission panels stop field resources; preflight action progress intentionally advances while its briefing-state panel is open, and freezes on explicit pause. Financial/budget simulation is deferred.

The existing factual source record was read and hashed locally on this date:

| File / identity | Receipt |
| --- | --- |
| [study.json](../../Content/Science/LunarWater/study.json), `lcross-cabeus-published-estimate-v1` | 3,863 bytes; SHA-256 `40db698b4244ee7f6f28fcb5aa44c7dd3e60177694b34cb18d8bc32a24981105`. |
| [manifest.json](../../Content/Science/LunarWater/manifest.json) | Its declared byte count, product ID and study checksum match the local study file. |
| [SourceArt provenance](../../SourceArt/LunarWater/CREDITS.md) | Publication/mission references and the derived-summary limitation remain intact. |

This is a local text/manifest check, not a packaged-build or disconnected-network test. No raw spectrum, new measurement or additional water dataset was downloaded for the new features. Inspection records only training-label state and references the existing remote publication. Checkpoint schema fields require actual JSON Number/Boolean types; string lookalikes are rejected. This is software integrity validation, not validation of a new scientific measurement.

## Validation evidence and limits

The final local Windows executable `08485e289426127af57fca0bd79fa1b9bc1f67a3708f8d4385068140c8733a7a` has these executable-specific receipts:

| Coverage | C++ PASS executions | Raw reports | Staged PNGs |
| --- | ---: | ---: | ---: |
| New field operations: NullRHI and rendered functional checks, three missions | 1,310 | 6 | 0 |
| New 720p/1080p UI and actual-world review | 308 | 6 | 120 |
| Retained legacy regression coverage, 16/16 suites | 2,400 | 55 | 160 |
| Total | 4,018 | 67 | 280 |

There were **0 C++ failures**. Eight paired carry-pixel checks are separate; PNGs and pixel pairs are not added again to the C++ count. These are repeated automated executions, not 4,018 unique tests or human sessions. The [verification index](verification-index.json), [fresh legacy receipts](legacy-verification.json) and [TEST_REPORT](../../TEST_REPORT.md) retain the hash/counting scope. Earlier failed iterations remain historical results, not evidence for a different executable.

Checks include readiness/pause, resource limits, real scanner sight/blockers and pickup/deposit, both rescue wire paths, vehicle collision/exit, strict checkpoint types/masks, optional-state/legacy compatibility and actual arranged UI geometry. Placement found an observation position for each cartridge; it does not prove a full continuous traverse or every approach.

Three final performance captures cover matched menu, retained Apollo opening and enabled-support Apollo opening workloads. See [performance comparison](performance-comparison.json). Occupied driving, scanner/board benchmarks and whole-mission frame-rate stability were not measured. NullRHI was not treated as graphical performance. [Preservation](preservation-final.json) records 84/84 protected files matching, following restoration of the verified editor timestamp change.

Human playtests remain **0**. Firewall/network isolation was not tested; local payload availability is distinct from such a test. Rover and sampler presentation remain authored proxies with art limitations. The game is not a NASA-certified simulator, and competition eligibility remains unconfirmed.
