# Public v0.9.2 Windows validation

Executed 2026-11-14T17:47:39.397542+00:00 through 2026-11-14T17:55:48.9889322Z. This report belongs to the public feedback preview with original live text startup/equipment symbols. The historical reference-art candidate has a separate report under Feedback20261114Evening and is not the distributed artifact.

Windows 11 x64; Unreal Engine 5.8.3; VS 2022 toolchain 14.44.35217; SDK 10.0.22621. Ryzen 5 6600H, RTX 3050 Laptop 4 GB, approximately 16 GB RAM. Development Win64, DX11 for rendered checks. Base commit 53dce53533ee00a25176c9b3a03e919503472016; source fingerprint `b9275648ea22c5875490e1a5416b5170f9f015e2ea187b401a61145447b98059`. Inner executable SHA256: `31d256ec1df385b711fb85e32f9f21bf8f4961075a7cd60bd58d9a57537d4867`.

## Executed checks

| Command | Actual result |
|---|---|
| python Tools/verify_expedition_models.py | Passed suit bind/weight/envelope checks, five rover derivatives/pivots and NASA reference geometry/texture bounds; static data only. |
| Game and Editor Build.bat targets | Both succeeded; one compile action at a time, no PCH/UBA; Game no debug symbols. |
| BuildCookRun cook of MissionSelect, Apollo12, MoonBase and MarsBase | Clean cook succeeded with unconfirmed media removed from inputs. |
| Tools/package_apollo12.ps1 -StageOnly | Succeeded; final binaries staged against that cook. |
| Tools/verify_rescue_feedback.ps1 -Packaged | 126 PASS, six actual rescue/movement/recorder cases, including missing-ledge-mesh fallback. |
| Tools/verify_field_operations.ps1 -Packaged -Mission all | 744 PASS across Apollo, SELENE and Mars. |
| Tools/verify_gameplay_tasks.ps1 -Packaged -Mission all | 618 PASS across all three destinations. |
| Tools/verify_task_fixes.ps1 -Packaged -Suite stride | 57 PASS through actual movement input. |
| Tools/verify_mission_selection.ps1 -Packaged | 66 PASS; browsing/settings preserve state and selection travels correctly. |
| Tools/verify_nasa_tasks.ps1 -Packaged -Intro -PublicDistribution | 15 PASS, including live text startup and runtime absence of both unconfirmed texture packages and the optional movie. |
| Tools/verify_field_operations.ps1 -Packaged -Review -Mission selene | 105 assertions; 41 fresh 1280x720 PNGs. |
| Tools/verify_gameplay_tasks.ps1 -Packaged -Review -Mission selene | 27 assertions; 27 fresh PNGs and paired carried-cell pixel checks. |
| Tools/verify_nasa_tasks.ps1 -Packaged -Intro -Review -PublicDistribution | Seven fresh startup/menu PNGs. |
| Rendered selected travel with ReviewMissionIntro and missing optional media | All loading phases passed; exactly two Moon direct-entry fallbacks and no movie playback attempt. |

Native total excluding loading-phase assertions: **1626**. Rendered output: **75** fresh PNGs. NullRHI checks verify game rules, not graphics performance. These are automated/synthetic checks, not human playtests. Exact receipts are identified by paths and checksums in VALIDATION_EVIDENCE.json; raw logs remain local under Reports/PublicRelease20261114 and dated suite folders.

The clean-copy launch and distribution inventory are recorded separately in ARCHIVE_VERIFICATION.json before ZIP creation. The published .sha256 asset records the final archive checksum; CRC validation and the final ZIP-member scan are retained in the local publication receipt. The clean-copy receipt must be present before publication.

## Bounded performance sample

`Tools/profile_apollo12.ps1 -Label public-v092-final -Frames 1200 -Quality 1 -FieldOperationsProfile -FpsCap 60 -GpuTelemetry`: DX11 offscreen, 1280x720, Medium, VSync off, 60 FPS cap. Scripted Apollo opening walk with parked actors; 1,200 frames captured, first 600 excluded, 600 analyzed (about 10 seconds). No occupied driving, scanning or movie included.

Mean 59.9948 FPS; 1% low 54.4957 FPS; p95 17.2378 ms; p99 17.6390 ms; maximum 19.1881 ms. Sampled peak process working set 664,518,656 bytes. This capped short sample does not establish full-game stable FPS or a causal performance improvement. Profiling exit 777003 is the utility's accepted ExitAfterCsvProfiling path.

## Preservation, launch and limits

All 53 original owner save/photo/config files remain byte-identical locally and are excluded from the public archive. All four source maps match their existing HEAD LFS hashes; no map regeneration occurred. No unconfirmed movie or reference-art texture is included in the public cook. No new NASA data was created: existing NASA terrain/data provenance and the verified CC BY-SA museum-rover model credits remain bundled.

Extract the Windows ZIP and run `Windows/LunarRescue.exe`; Unreal Editor is not required. If the machine lacks the Microsoft VC++ runtime, the included redistributable is under Engine/Extras/Redist/en-us. Source build: Build.bat LunarRescueEditor Win64 Development -Project=<checkout>/LunarRescue.uproject, then the LunarRescue Game target, single-process four-map cook, and Tools/package_apollo12.ps1 -StageOnly. This host used a 384 MB managed heap for Editor and 1 GB for Game/stage; those are constrained-host settings, not engine requirements.

No new human playtest participants were available. Audible human review, OS network isolation, occupied-rover/scanning performance and 1080p visual review remain undone. This public prerelease is separate from any competition submission; submission state, eligibility, NASA approval and institutional partnership are not asserted. The important next checks are Boburjon's visual playthrough, real-user playtests, and occupied-driving/scanning profiling.
