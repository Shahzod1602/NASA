# LunarRescue Expeditions v0.9.1 — Hand Pose Fix

Fix the sideways, folded astronaut gloves reported in v0.9.0.

- Align the credited suit's palms and fingers using longitudinal and transverse
  directions. The previous direction-only adaptation left about 84° of palm
  roll error and distorted the skin between palm and finger influences.
- Fit palm spread to the existing controller and transform normals correctly.
  Retain source topology, UVs, materials and the original model licence.
- Hide duplicate legacy forearm surfaces when the expedition skin is active;
  keep their pose guides and the original asset fallback.
- Add an independent glove-envelope regression, a runtime overlap assertion,
  and free-hand FOV/lowering review frames. Preserve mission state, collision,
  oxygen rules, tool/carry controls and the retained Moon/Mars missions.

Download and extract the **entire Windows ZIP**, then run **LunarRescue.exe**.
Keep Engine, LunarRescue and Credits together. Unreal Editor/Python are not
required to play. The Microsoft C++ runtime installer is included if needed.

## Verification and limits

Inner EXE SHA-256:
`092b2b025a25920ed26521b99050c2ed01ab40a06aa783a5e0f8f6d48287b55c`.

**765 packaged C++ PASS executions / zero FAIL in seven raw case reports,
51 staged review PNGs.** These are repeated automated executions, not 765 unique
tests or human playtests. Selected POV, Apollo/SELENE/Mars gameplay, carry/grip,
Apollo and suit checks passed. The patch's canonical total is separate from
v0.9.0's 2,019 executions on a different EXE and assets. Human playtests: **0**.
Independent carry-pixel contribution and model/rig geometry checks passed.

Unreal 5.8.3 Editor/Game builds and a fresh four-map cook/stage succeeded.
Before/after free-hand frames and exact raw evidence are retained under
Docs/HandPose20261114. Rendered fixtures are synthetic. No new performance,
anatomical accuracy, astronaut-training or network-disconnected test is claimed.
Later clean-extracted smoke results are recorded separately in RELEASE_SMOKE.json
and are not added to the canonical 765 total.

The unchanged original local compile/test records retain their true dirty
base-commit provenance and pre-publication flags. Archive BUILD_INFO links the
reviewed release source snapshot separately. Dates/history are not rewritten.

The clean ZIP excludes owner saves/photos, generated logs/reports, crash dumps
and debug symbols. Each included runtime file has a SHA-256 entry; the archive
is independently decompressed and hash-verified before upload. Offline credits,
licence notices and original validation records remain included. LasquetiSpice's
suit retains CC BY 4.0; other assets retain their separate terms. No new NASA
data, NASA endorsement or competition eligibility is claimed.

Existing open work remains: final FIELD GUIDE preference, better habitat/lander
replacement assets, human playtesting and longer driving/stress measurements.
