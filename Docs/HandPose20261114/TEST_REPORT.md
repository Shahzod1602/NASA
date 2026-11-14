# Local hand-pose correction — 14 November 2026

Release packaging: [v0.9.1 notes](../Releases/apollo12-v0.9.1/RELEASE_NOTES.md). Statements and immutable records below that describe local/unpublished work refer to the original pre-publication verification stage; the release source snapshot is recorded separately from that compile-time provenance.

Base: `c9f966fe3d5d3167318a48f22c2804df7fd18ce8` / published v0.9.0. This new local correction is not yet committed, pushed or published. The existing v0.9.0 download still contains the reported defect.

## Cause and correction

The supplied suit was aligned using one bone direction. A longitudinal direction does not determine palm roll. In the retained source, wrist-to-middle-finger alignment left approximately 83.3° of raw transverse error (84.35° in the orthogonal palm frame). Skin influenced by the palm and neighbouring finger bones therefore received incompatible orientations. The new skin was additionally drawn over legacy static forearm proxies in the free-hand state.

`Tools/prepare_expedition_suit.py` now builds proper frames from the longitudinal direction and index-to-little-finger spread for palms/fingers. Palm length and transverse spread fit the existing controller; normal transforms account for the authored scale/shear. The corrected orthogonal roll error is below 0.001°. `LunarRig.cpp` retains legacy proxy transforms as pose guides while hiding their duplicate surfaces when the expedition skin is active. Legacy asset fallback remains available.

Source topology, UVs, licences, mission state, character collision, oxygen and interaction rules are unchanged. The floating source animation remains unused for ground movement. Original sources/assets/tools were backed up under `Backups/HandPose20261114/BeforePalmRoll` before correction. Current owner files were copied before the old game window was closed normally and again after shutdown.

## Verification

- Baseline reproduced with actual v0.9.0 source assets: `Reports/NasaTasks20261101/editor-review-1280x720-20261114-110245-891`, 15 Moon / 6 Mars PNGs. The free-hand screenshot reproduces the folded palm/overlapping sleeve.
- Independent derivative geometry inspection: dominant-palm vertices had robust breadth/depth ratio ~0.752 before correction, ~1.043 afterwards. A model-specific geometry regression check now rejects the previous sideways envelope. This threshold is an asset regression guard, not a biological measurement.
- Corrected model data checks passed: bind pivots, finite geometry/normals, weights, unchanged triangle counts and palm envelope.
- Editor C++ build succeeded in 87.29 seconds (`Reports/HandPose20261114/editor-build.log`).
- Native POV checks: **31 PASS / zero FAIL** (`Reports/NasaTasks20261101/editor-test-1280x720-20261114-111226-492`). New runtime regression checks ensure no visible legacy forearm proxies overlap the new skin.
- Corrected Editor review: **18 Moon / 6 Mars fresh PNGs**, `Reports/NasaTasks20261101/editor-review-1280x720-20261114-111303-840`. Added free-hand 70°/110° FOV and lowering-state frames. Inspected free hands, soil scoop and Mars carry; fixtures remain synthetic.

Fresh Windows Game build succeeded in 95.03 seconds; cook command completed in 106.30 seconds and stage/archive in 45.08 seconds. The local EXE is `092b2b025a25920ed26521b99050c2ed01ab40a06aa783a5e0f8f6d48287b55c`.

**765 packaged assertion executions / zero FAIL in seven raw case reports, with 51 staged PNGs.** The total includes repeated configurations, not 765 distinct tests. Packaged POV: 31; gameplay Apollo/SELENE/Mars: 237/198/183; carry review: 27; Apollo/suit: 57/32. POV rendering produced 18 Moon + 6 Mars frames and the carry review another 27. Independent carry-pixel contribution checks passed. Exact EXE/payload hashes and copied raw reports are in [Evidence/LOCAL_VALIDATION.json](Evidence/LOCAL_VALIDATION.json).

All 53 owner save/photo/config files were verified and restored byte-for-byte after staging and tests. Python preparation/import/geometry-check scripts compile, and `git diff --check` passed. The old 2,019-assertion v0.9.0 receipt remains historical and does not certify this correction. No new performance, human-playtest or disconnected-machine claim is made. The source and local package are updated; no GitHub push or release was performed for this correction.

## Reproduction

```powershell
python Tools/prepare_expedition_suit.py --source SourceArt/ExpeditionSuit20261113/Raw/animated_floating_astronaut_in_space_suit_loop.glb
python Tools/verify_expedition_models.py
# Explicitly opt into replacing only the backed-up expedition derivative folder.
$env:LUNAR_SUIT_REIMPORT='1'
& D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe D:/LunarRescue/LunarRescue.uproject -run=pythonscript -script=D:/LunarRescue/Tools/import_expedition_suit.py -nullrhi -unattended -nosound -nop4
./Tools/verify_nasa_tasks.ps1 -Pov
./Tools/verify_nasa_tasks.ps1 -Pov -Review
```

Rebuild the appropriate Editor/Game target before running changed native checks. Fresh cook and staging are required to update an installed package; replacing C++ source alone does not fix the published EXE/assets. Binary assets were imported through Unreal APIs, not edited as text.
