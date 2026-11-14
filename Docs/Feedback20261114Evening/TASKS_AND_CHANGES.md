# Surface presentation and control feedback — 14 November 2026

Implemented as a verified local candidate against main 53dce53533ee00a25176c9b3a03e919503472016 (v0.9.1).
This document does not certify a build; see TEST_REPORT for actual executed checks.

| Feedback | Change | Verification status |
|---|---|---|
| Deliberate lunar movement | Moon walking 140 cm/s, fast stride 220 cm/s; acceleration 420, braking 650 cm/s²; bounded 140 cm/s hop. Mars retains 320/500. | Passed final Windows native and rendered checks; see TEST_REPORT. |
| Remove cabin camera takeover | Normal pre-EVA opens at the existing surface pawn; completing checks commits gameplay without a departure actor, FOV change or teleport. Legacy cabin remains an explicit automation fixture. | Passed final Windows field checks on Apollo/SELENE/ARES. |
| Remove SELENE instruction board | Retain the service anchor and small ground pad; omit pole, upright sign and three instruction text components. B and resource mechanics remain. | Passed final Windows native and rendered checks; see TEST_REPORT. |
| Rover driver and cameras | One seated poseable visual follows the existing possessed pawn's suit/materials. Mouse orbit and cockpit free-look; V changes rover camera only. Existing swept drive, resource cost and safe dismount remain. | Passed final Windows native and rendered checks; see TEST_REPORT. |
| Stationary rover mudguards | Reassign 12,900 connected stationary triangles to body; retain all 464,116 source triangles, textures and UVs. Tire pivots use circular tire centres. New dated imported folder preserves prior assets. | Derived data, supported import, runtime and rendered checks passed. |
| Two-handed cell carrying | Add authored side handles in the same fitted item frame; target palms to handle bars; retain skeletal wrist/finger posing. Lower-centre pickup settles with existing WorkBlend. | Actual bone/surface probes, 27 rendered views and paired pixel checks passed. |
| Intro after selected loading | Optional local MoonArrival.mp4 via MoviePlayer after ready/loading completion; world Tick remains blocked. No extra travel, no intro for resume/restart, no Moon clip used for Mars. Missing decoder/file must permit entry. | Two complete Windows clips and missing-file fallback passed. Redistribution/source permission and Mars footage pending. |
| Preserve airborne momentum | Moon ignores movement input while falling; air steering/friction/braking disabled. Look remains available. Ground control resumes on contact. | Actual movement simulation and input checks passed. |

All locomotion, grip, seated pose and vehicle settings are authored game tuning, not measured spacesuit physiology, certified astronaut training or validated vehicle dynamics. Existing NASA terrain and mission anchors are retained.

## Intro content boundary
The supplied reference contains a Moon view: 30.04 s, 854×480, 24 fps H.264 with AAC stereo 32 kHz. It is not the separate pre-EVA screen recording. Preserve the original file rather than creating detail through upscaling. Original author/source/redistribution permission has been requested in the group; no identity or NASA-archive claim is made. The unverified binary is excluded from Git. A missing optional video is an explicit graceful fallback, not completed content integration. A separate appropriate Mars clip is required.

## Additional feedback received during validation

| Feedback | Implementation | Acceptance scope (final results in TEST_REPORT) |
|---|---|---|
| Sequential direct pre-EVA checks | Complete a system to unlock the next clickable row; no Next button. Each row retains a real progress bar and completed state. Hatch still requires all four checks. | Final native and 100/150% rendered checks. |
| Interactive planet map | Retain the offline analytic sphere shader and credited local colour texture; drag now rotates vertically over poles as well as horizontally, with bounded zoom. The local mission map still carries actual mission points. | Final globe/material interaction review. This is global colour context, not a new elevation dataset or globally geolocated mission map. |
| Backpack reference layout | Category sidebar, equipment thumbnails, selected item detail and guarded actions. Original supplied reference is sampled as an atlas without raster alteration. Baked quantities are excluded; all displayed quantities, locks and actions use live mission state. | Final UI layout at ordinary and large text sizes. Nine actual equipment entries are retained; no fictional medkits or oxygen refills are granted to mimic reference quantities. |
| System Connect verification | Successful Verify commits the cable step once, displays an in-world confirmation and closes the paused panel immediately. | Actual authority callbacks, rejection/retry and rendered return-to-game check. |
| Immediate rover entry | Eligible nearby work-key interaction boards without opening Vehicle Menu or adding a control-instruction radio; existing preparation, mission briefing, charge, visibility and safe-seat guards remain. Scanner readiness is recorded during the interaction. | Native production work-key entry, camera and dismount regression. |
| Stray cyan module beacon | Mount the small locator and light on the actual cell. Real pickup hides both and clears light intensity. | Actual pickup callback and two rendered approach angles. |
| Walk-through static Lunokhod | Preserve the original triangle collision; disable character step-up and add bounded authored chassis/eight-tire solid envelopes in source OBJ centimetres. | Eight actual character-capsule swept approaches, visual review; not a claim of vehicle physics simulation. |
| Floating antenna/status lamps | Extend the legacy mast to a small footing on an actual supporting roof or terrain hit, preserving dish top/rotation anchor. The source habitat is open at this mast position, so the measured terrain is its support. Mount status lamps and their lights on the actual power port. | Actual support/footing contact, paired light alignment, normal power sequence and rendered angles. |
| AstroKid initial loading | Dated local reference texture, aspect fit and live Slate spinner on initial MissionSelect load. Initial ready hold is 0.35 s minimum, not the selected mission's 2.5 s presentation. | Actual initial rendered startup and no replay on menu return. Engine initialization before PreLoadMap is outside this widget's scope. |
| AstroKid intro | Same standalone logo appears immediately after loading, then fades to the main menu after a bounded 1.75 s presentation. Technical project name remains LunarRescue. | Actual startup render and native input gating. Full NASA/partner montage remains pending factual partnership/logo-source confirmation; it is not imported or shipped. |

### Asset and map preservation
All four serialized mission maps remain identical to the current Git HEAD/LFS source. A temporary tag-only Editor save was backed up and replaced with the exact pre-change map after verifying its SHA against HEAD's LFS object ID. Runtime lookup uses verified persistent actor IDs plus mesh/class checks for four previously untagged legacy props, then caches tags. Missing/replaced props must fail the native checks instead of matching arbitrary similarly named scene content. `Tools/import_startup_feedback.py` imports only two dated UI textures and performs no map writes. The original rover/terrain geometry, power port, module and dish anchors are retained.

The loading percentage remains a preparation-stage indicator rather than a measured disk-byte percentage. Locomotion, collision envelopes, grip targets, seated pose, spinner/intro timings and resource coefficients are authored gameplay/presentation rules.

### Optional recorder reach after movement tuning
The authored SELENE recorder ledge is 40 cm above terrain (90 cm mesh height with 50 cm burial), within the tuned 140 cm/s lunar hop's approximately 60.5 cm ballistic rise under the game's 162 cm/s² gravity. Its recorder and marker follow the same ledge-top parameter. Mars retains its original 90 cm top. The Moon ledge input regression now establishes walking momentum before jumping, because airborne directional acceleration is deliberately disabled. Actual normal and missing-mesh fallback traversal must pass before accepting this change.

## Final verification status
Final Windows candidate: 1,623 native assertions excluding loading phases; 75 fresh review PNGs; full and missing-media loading runs passed. See TEST_REPORT and VALIDATION_EVIDENCE.json for exact build identity and limits. Reference layouts are adapted to existing styling and actual mission state, not certified as pixel-identical copies. Full partner montage remains pending factual confirmation; no public release or competition eligibility is implied. All original player files and source maps match their prior hashes.
