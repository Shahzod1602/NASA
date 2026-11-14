# Local feedback validation — 14 November 2026

This record concerns an unpublished dirty worktree based on `1b3bd3716a2f38d138bb084ae89c10e7ad3f267c` / v0.8.2. All timestamps in folder names use Asia/Tashkent unless a field explicitly says UTC. Historical v0.8.2 release results do not certify these changes.

## Environment and preservation

- Windows 11, Unreal Engine 5.8.3, Visual Studio MSVC 14.44.35217, Windows SDK 10.0.22621.0.
- Ryzen 5 6600H, RTX 3050 Laptop 4 GB, approximately 16 GB system RAM. Only one compiler action was allowed at a time because of this machine's available memory.
- Existing work and owner saves/photos/settings were backed up before mutation. Original package copied to `Backups/DistBaseline-v082-20261114`; 53 owner files were hashed before new staging.
- Original A7L and Apollo terrain were retained. MoonBase was backed up before targeted Editor visibility/tag edits; no map regeneration was used.
- Imported `.uasset`/`.umap` files were created or edited through supported Unreal Editor APIs, never as text. Raw source models and dated source/adaptation manifests were preserved. Local Git LFS assets were checked as real binary files.

## Editor evidence

| Check | Exact local evidence | Result |
| --- | --- | --- |
| Final Editor C++ build | `Reports/Feedback20261113/editor-build12.log` | Succeeded, 45.63 seconds; warnings remain. |
| Game C++ build | `Reports/Feedback20261113/game-build.log` | Succeeded, 783.45 seconds, 43 actions. Fresh EXE SHA-256 `953d5f4abaefaca3e13b60b468c9d24c5742371a87256bb5fb47471d6a6637b2`. This compile alone does not verify cooked assets. |
| Final native field operations | `Reports/FieldOperations20261104/editor-test-1280x720-20261114-023508-148` | Apollo 220, SELENE 226, Mars 226 PASS; zero FAIL. Includes actual mesh-body hull containment and swept barrier, parking, driver/resource/pause/cleanup tests. |
| Final SELENE UI render | `Reports/FieldOperations20261104/editor-review-1280x720-20261114-023634-633` | 89 PASS, 28 fresh 1280×720 PNGs; includes keyboard alternative retained through refresh. |
| Earlier three-mission UI render, before the final expander/hull corrections | `Reports/FieldOperations20261104/editor-review-1280x720-20261114-022036-390` | Apollo 78 / SELENE 88 / Mars 88 PASS; 26/28/28 PNGs. Separate development snapshot, not the final module. |
| Native gameplay before final hull/expander corrections | `Reports/Gameplay20261103/editor-test-1280x720-20261114-015353-692` | Apollo 237 / SELENE 198 / Mars 183 PASS; zero FAIL. |
| Final arm-fit grip/departure render, before final hull/expander corrections | `Reports/Gameplay20261103/editor-review-1280x720-20261114-020910-531` | SELENE 27 PASS / 27 PNGs plus independent carry pixel-contribution checks. |
| Ground/contact and NOMAD render | `Reports/Feedback20261102/equipment-python-1280x720-20261114-021929-537` | Four fresh views: habitat, power panel, actual solar underside and NOMAD. Source-mesh line-of-sight assertions passed. |
| Synthetic motion review | `Reports/ExpeditionExperience/motion-feedback20261114-20261114-022001-300` | 120 frames and pose CSV, fixed 60 Hz simulation sampled at 12 Hz on authored flat/12° fixtures. Reviewed walk/stride/turn/jump/stop. Not a frame-rate benchmark or human study. |
| Model/rig derivative checks | `Evidence/model-data-checks.json` | Both suit meshes' exact 36-bone bind contract, weights, finite geometry/normals, zero derivative degenerates, source rover pivots/geometry/hash, reference geometry and texture bounds passed. |

Repeated assertions across scenarios are executions, not distinct tests. Native NullRHI assertions cannot establish graphical quality or GPU performance. Screenshots/arranged widget checks and pixel contribution do not establish human comprehension or animation taste.

## Reproduction

From the repository root in PowerShell, with no other Unreal/game process running:

```powershell
$env:DOTNET_GCHeapHardLimit='40000000'
& D:/UnrealEngine/UE_5.8/Engine/Build/BatchFiles/Build.bat LunarRescueEditor Win64 Development -Project=D:/LunarRescue/LunarRescue.uproject -WaitMutex -MaxParallelActions=1 -NoUBA -NoPCH -NoSharedPCH
./Tools/verify_field_operations.ps1
./Tools/verify_field_operations.ps1 -Review -Mission selene
python Tools/verify_expedition_models.py
git diff --check
```

For the game target use `LunarRescue` and add `-NoDebugInfo -NoLinkerDebugInfo`. A fresh Windows cook of MissionSelect/Apollo12/MoonBase/MarsBase must precede `./Tools/package_apollo12.ps1 -StageOnly`. That staging mode avoids rerunning the legacy material-rebinding import tool. Do not use `-StageOnly` with an old cook to claim a fresh build.

Packaged verification uses `verify_field_operations.ps1 -Packaged`, `verify_gameplay_tasks.ps1 -Packaged`, their rendered `-Review` modes, `verify_nasa_tasks.ps1 -Packaged -UI`, `verify_mission_selection.ps1 -Packaged`, and `verify_apollo12.ps1 -Packaged -Regression -Suit -Guidance`. The runner scripts preserve settings and reject missing/stale semantic results instead of accepting exit code alone.

## Defects found and corrected during validation

- Suit helper weights folded to root and distorted the torso/backpack; anatomical anchors corrected. Source forearm length exceeded the controller, covering gloves; endpoint fitting and normals corrected.
- Legacy specular/glossiness import appeared white; reproducible core metallic/roughness adaptation restored source colour/normal UVs. This is visual tuning, not measured fabric reflectance.
- A combined NASA mesh created an oversized atlas; rejected import backed up and replaced with preserved per-material PBR/UV assignments. The importer drops 50 degenerate triangles from the original 199,521-triangle reference.
- Globe mode was lost when its map page rebuilt, and a wrong material input produced fallback rendering. Persistent mode and checked shader connections corrected.
- The solar underside followed a reversed slope sign and was embedded in the source mesh; actual triangle-plane/line-of-sight checks exposed and verified the correction.
- The cable keyboard alternative collapsed on every state refresh; expanded state now survives rebuild and has a runtime regression assertion.
- The imported rover body exceeded the legacy swept hull. Actual imported bounds now fit the Moon hull; clearance/turn/park queries use its dimensions.
- A cleanup-review fixture accidentally retained a held cell; isolated mission-state setup corrected the fixture. Failed intermediate runs are retained locally and are not counted as final passes.

## Packaged candidate and performance

**Fresh local Windows candidate: 2,019 C++ assertion executions, zero FAIL, 20 raw case reports, 109 staged review PNGs.** These are repeated executions, not 2,019 unique tests or human playtests. Exact executable and cooked `.pak/.ucas/.utoc` hashes, cases and report checksums are frozen in [Evidence/LOCAL_VALIDATION.json](Evidence/LOCAL_VALIDATION.json).

The 43-action Game build succeeded in 783.45 seconds. Fresh cook completed with exit zero (`cook.log`, 156.56-second cook command), followed by successful local stage/archive (`stage.log`, 45.44-second BuildCookRun). This used a fresh cook, not an old `ue.projectstore`. The unchanged old EXE `59d4f795b114…` remains a comparison baseline in the backup; it was not used for candidate assertions.

| Packaged suite | Assertion executions / frames | Exact local source |
| --- | --- | --- |
| Field native, Apollo/SELENE/Mars | 220 / 226 / 226 PASS | `Reports/FieldOperations20261104/package-test-1280x720-20261114-025421-577` |
| Gameplay native, Apollo/SELENE/Mars | 237 / 198 / 183 PASS | `Reports/Gameplay20261103/package-test-1280x720-20261114-025444-409` |
| Three-map field UI render | 78 / 89 / 89 PASS, 26 / 28 / 28 PNGs | `Reports/FieldOperations20261104/package-review-1280x720-20261114-025503-001` |
| SELENE grip/departure render | 27 PASS, 27 PNGs; carry-pixel proof passed | `Reports/Gameplay20261103/package-review-1280x720-20261114-025635-842` |
| NASA UI, Apollo/SELENE/Mars | 25 / 31 / 29 PASS | `Reports/NasaTasks20261101/package-test-1280x720-20261114-025711-803` |
| Mission selection | 66 PASS | `Reports/MissionSelection20261031/package-test-1280x720-20261114-025729-957-checks.txt` |
| Apollo / legacy Moon / Mars / menu / suit / guidance | 57 / 50 / 54 / 55 / 32 / 47 PASS | `Reports/package-apollo12-*-result.txt`; copies under `Evidence/regression-*-checks.txt` |

An additional uncounted SELENE `-LunarTest`/`obj list class=StaticMesh name=SM_NomadReference` probe confirmed the actual new NASA static mesh is loaded from the package (one matching object), rather than silently using the old visual fallback. See `Evidence/packaged-nomad-presence.json`. That probe is separate from the earlier Editor close-up render and the native mission recovery assertions.

Both performance captures used the same hardware, DX11, offscreen 1280×720, quality 1, uncapped, FieldOperationsProfile, scripted opening Apollo walk and 2,400 requested frames. The first 600 frames were excluded, leaving 1,800 samples per build. Logs, environment/settings, GPU samples and CSV remain under `Reports/Performance/feedback-baseline-v082-20261114` and `feedback-candidate-20261114`; JSON copies are in `Evidence`.

| Short rendered opening-walk sample | Published v0.8.2 baseline | Local candidate |
| --- | ---: | ---: |
| Analysed duration | 7.818 s | 7.712 s |
| Mean FPS | 230.24 | 233.39 |
| 1% low FPS | 150.00 | 139.46 |
| p95 frame time | 5.289 ms | 5.210 ms |
| p99 frame time | 5.865 ms | 5.842 ms |
| Largest sampled frame | 12.340 ms | 15.247 ms |
| Sampled peak process working set | 644.3 MiB | 649.1 MiB |

Mean/p95 are similar in these single captures, while the candidate's 1% low and largest frame are worse. This is **not** evidence of sustained-performance improvement. These short captures do not establish driving/scanning performance, whole-mission presentation smoothness, minimum hardware, VRAM usage or stable 60 FPS. No causal explanation for the variance is established.

All 53 pre-existing owner save/photo/config files were verified byte-for-byte and restored if needed after staging, automated tests, profiling and the final asset probe. Packaging adds the four dated model/context attribution and adaptation manifests under `Credits/SourceArt`, plus the dated notes. Run `Dist/Apollo12/Windows/LunarRescue.exe`, keeping the complete Windows directory together; Unreal/Python are development dependencies, not player requirements.

## Open limits

- Human playtests: **0**. No new participants are available; synthetic results are explicitly labelled.
- OS/network-isolated execution has not been tested. Runtime assets are local, but that is a different claim from a disconnected-machine test.
- No full human traversal, occupied-rover performance capture, uneven-terrain vehicle certification, low-end hardware matrix or long endurance session was completed in this batch.
- Better habitat/lander replacement files and final FIELD GUIDE preference await Boburjon's replies (1216 and 1212).
- Competition rules/eligibility were not re-audited in this feedback batch. No new eligibility or NASA endorsement claim is made.
- No GitHub commit/push, release publication or competition submission was performed for this batch.

Task receipts are summarized in [TASK_INTAKE.md](TASK_INTAKE.md); the full private Telegram archive is excluded. Selected unedited synthetic review frames are under `Evidence`, with their originating report paths above.

Per-task progress/completion replies were verified in NASA as 1214–1224 (1216 requests missing models); final local-candidate summary 1225 explicitly states the build is unpublished and lists the outstanding card/model/performance/human-test limits. Those receipts do not imply GitHub publication.
