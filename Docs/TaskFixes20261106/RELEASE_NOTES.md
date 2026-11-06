# LunarRescue Expeditions v0.8.1 — Mission Interaction Fixes

A patch for the existing Moon/Mars expeditions, including Apollo 12, SELENE and ARES.

- Reduce redundant mission-picker presentation updates and layered blur work while
  keeping the blue frosted selected-card state. The reported lag was interpreted
  as picker responsiveness; a universal lag fix is not claimed.
- Fill Start Mission, Settings and Back button frames with their background and
  clickable area, centre captions and retain hover/press/keyboard-focus feedback.
- Place stationary fictional PIP-3 and the communicator on clear supported ground
  outside the lander footprint. No following AI was added.
- Share carried-item and glove grip frames in first and third person; improve
  sleeve continuity and first-person item readability using retained assets.
- Show truthful ALSEP distance/facing/obstruction guidance, preserve separate
  bubble and Earth-link confirmations, and reset alignment on a new expedition.
- Restore actual fast-stride movement in SELENE/ARES, preserving hold/toggle
  settings and movement gates during pause, work, planning and vehicle riding.

Download and extract the **entire Windows ZIP**, then run **LunarRescue.exe**.
Keep Engine, LunarRescue and Credits together. Unreal Editor is not required.
If needed, the Microsoft C++ runtime installer is included under
Engine/Extras/Redist/en-us. Controls are available in Settings and READ_ME.txt;
the default Fast stride key is Left Shift.

Validation of inner executable
`06c1b022f92b4e13e8300fc8d71296f290ee4f2565ac8597ebcd337519facda8`:
**4,092 C++ PASS executions / 0 FAIL in 65 raw reports, 16/16 legacy suites**.
Editor selection contributes a separate 66 PASS. The 230 staged PNGs and 8
paired-pixel checks are documented as local evidence; screenshots are not copied
into the release notes bundle. Counts include repeated map/renderer/resolution
executions and are not 4,092 unique tests. Human participants: **0**.
The Apollo driver completed 1,285.90 m of collision-enabled input-driven travel,
mission stages and restart; its fixed-step simulation is not an FPS measurement.

Both Unreal 5.8.3 Windows Development targets built successfully. Final staging
reused the existing cook; this was a code-only update with unchanged binary assets
and four maps. Menu captures used DX11 offscreen, 1280x720 and Quality 1. A single
same-final-build uncapped Picker blur-cost control observed about 3.4% higher
throughput with the optimized policy. It is not an old-version picker comparison,
whole-mission benchmark or guarantee for other hardware. Full-mission and sustained
occupied-driving graphics performance remain unmeasured.

Existing LROC elevations and Apollo work-site coordinates are unchanged. No new
planetary dataset was imported. Guide placement, grips, lighting and movement
speeds are authored gameplay. Glove checks sample real geometry; they do not
certify complete physical grasp accuracy. No NASA endorsement or astronaut-training
certification. NASA Space Apps 2026 eligibility remains unconfirmed.

The release uses local science payloads and introduces no runtime internet
requirement. A firewall-isolated test and human playtest were not performed.
The ZIP excludes player saves/photos, logs and debug files, includes offline
source/credits notes and records each runtime file's SHA-256. Release source
commit and compile-time base/dirty provenance are kept separate in BUILD_INFO.
Original development dates and pre-publication receipts are retained.

See README, TEST_REPORT, SCIENCE_AND_SOURCES and
Docs/TaskFixes20261106/verification-index.json for scope and reproducible evidence.
