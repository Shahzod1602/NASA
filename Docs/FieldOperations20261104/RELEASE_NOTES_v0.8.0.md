# LunarRescue Expeditions v0.8.0 — Field Operations

Explore Apollo 12, SELENE and ARES with a modern educational training extension.
This release combines the November departure, carry, crew and LCROSS work with
new pre-EVA checks, resource planning and surface support gameplay.

- Complete four ordered suit/PLSS/radio/internal-power checks before departure.
- Preview and lock a difficulty budget for time, shared utility, payload and data.
- Use the resource map and support waypoints without replacing the main objective.
- Inspect and drive an authored Moon/Mars rover; battery pays for actual travel,
  collision checks constrain movement, and dismount requires clear grounded space.
- Use the centred magnifier to inspect labelled simulated cartridges, pack limited
  cargo, deposit at the lander and use powered recharge/downlink operations.
- Match labelled cable ports in SELENE/ARES; retain legacy Apollo save compatibility.
- Read improved 720p/1080p UI at 100%/150% text scales and offline Science & Sources.

Download the Windows ZIP, extract the entire folder, then run LunarRescue.exe.
Keep Engine, LunarRescue and Credits together. Unreal Editor is not required.
If Windows needs the Microsoft C++ runtime, its installer is included under
Engine/Extras/Redist/en-us. Existing saves should be backed up before updating.

Controls: B resources/map/inventory; G magnifier; hold left click to inspect;
E work/pack/exit; W/S drive and A/D steer. B/G are reserved; other controls are
listed in Settings and READ_ME.txt.

Validation of inner executable 08485e289426127af57fca0bd79fa1b9bc1f67a3708f8d4385068140c8733a7a:
4,018 C++ PASS executions, 0 FAIL across 67 fresh reports; 16/16 legacy suites;
280 staged PNGs and 8 paired-pixel checks. These include repeated map/resolution
executions and are not 4,018 unique tests or human participants. UE5.8.3 Windows
Development build. Opening benchmarks used DX11,1280x720,Quality1 and a60FPS cap;
whole-mission, sustained driving/scanning and resource-board performance are unmeasured.

Existing LROC elevations shape Apollo terrain. The bundled LCROSS publication
summary labels simulated remote-delivery cartridges; it does not detect water
at Apollo 12. SELENE/ARES terrain, rover/sampler models, EVA checks and costs are
authored gameplay. Apollo12 did not use an LRV. NASA SEV is a reference, not an
imported vehicle model. No NASA endorsement or astronaut-training certification.

Human playtests:0. Vehicle/sampler art remains a simple proxy, with no polished
visible driver/gripping hand. No firewall-isolated network test was performed.
NASA Space Apps2026 challenge/reuse eligibility remains unconfirmed; this is
independent pre-event product work, with existing releases and history disclosed.

Source/validation: see README, TEST_REPORT, SCIENCE_AND_SOURCES and
Docs/FieldOperations20261104/verification-index.json. The release source commit
records the reviewed snapshot; compile-time base/dirty provenance stays in
BUILD_INFO.original_build_record. Historical local verification records retain
their original dates and pre-publication status. The archive replaces stale
local validation metadata and excludes player saves/photos, logs and debug files.
