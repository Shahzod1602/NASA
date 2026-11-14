# Current demo verification — 15 November 2026

This batch closes written Moon/Mars demo gaps: full supplied acknowledgement intro, shared EVA/Map/Inventory presentation, initial offline globe, and mission-specific hardware information. Separate OXYGEN/rocket and Mission Designer modes are owner-deferred. The full task mapping and remaining external decisions are in TASK_LEDGER.md.

## Tested artifact

UE 5.8.3, Windows 11 x64, VS 2022 / MSVC 14.44.35217, Windows SDK 10.0.22621. Ryzen 5 6600H, RTX 3050 Laptop 4 GB, approximately 16 GB RAM.

EXE SHA-256: `76fb953cbd2263cd3f41e023e59ddaa778ee3a4b2b1ddb4770cd84bff0927d78`. Source fingerprint: `aa319dc99a0ce83a29b04a5e6225711c692e192f45892195605497ded4302835`. Run interval: 2026-11-14T21:57:19.557353+00:00 to 2026-11-14T22:27:00.1053144Z. The initial Editor attempt failed with a .NET heap limit during UHT; the successful retry used DOTNET_GCHeapHardLimit=40000000 and sequential compilation. Existing Reset-hiding and double-to-float warnings remain.

## Executed checks

Both Editor and Game Win64 Development builds succeeded. Five authorized UI textures were imported with Editor Python, without map writes. All four maps cooked and staged. Commands are recorded in the checked-in VALIDATION_COMMANDS.ps1 recipe and exact fresh receipt paths/hashes in VALIDATION_EVIDENCE.json.

| Packaged suite | Passed assertions |
|---|---:|
| Field operations, Apollo/SELENE/ARES | 750 |
| Gameplay, all three destinations | 618 |
| Rescue feedback | 126 |
| Stride/input | 57 |
| Mission selection | 66 |
| Startup, TeamMediaDemo | 17 |

Apollo rendered review adds 92 assertions and 34 PNGs.

Total: 1634 native assertion executions excluding loading and rendered checks. Rendered SELENE review: 110 assertions / 41 PNGs; Mars: 104 / 36. Startup adds seven PNGs; across all three rendered destinations this gives 118 fresh captures. Tests exercise real menu/operation callbacks, live preflight progress, locked/ready hatch, map modes and 100/150% text allocation. Later world/cargo states are explicitly seeded fixtures, not human playtests.

The new-Moon cinematic reached actual EOF after 30.219 / 30.106 seconds with world time held. Resume/restart and Mars retain their own behavior. Movie SHA-256: `6da19e1461a4de7d6b8b5d1129a46155ff6ea1abfeb8d65a59dc7b63e238380b`. AI-generated fictional portrayal remains visible; audio was enabled but no human hearing review was performed.

All four source map hashes match HEAD LFS objects. All 53 original player save/photo/settings files are unchanged. Additional saved/config snapshot differences: 0; exact paths remain in the local integrity receipt.

## Bounded graphics sample

DX11 offscreen 1280x720 Medium, VSync off, 60 FPS cap, 1,200 frames, 600 warmup excluded, 600 analyzed. Scripted Apollo opening route with parked actors. Mean 59.994 FPS, 1% low 52.507 FPS, p95 17.459 ms, p99 18.128 ms. Sampled peak working set 664,977,408 bytes. This is not a preflight, cinematic or occupied-driving performance claim.

## Play and limits

Run the staged Windows/LunarRescue.exe after extracting the entire archive. Unreal Editor is unnecessary for players. Choose Moon or Mars, then a specific mission. New Moon missions show the supplied fictional arrival clip. Complete four EVA checks and Open hatch; Map and Inventory remain available before departure. Mission hardware opens optional read-only sources.

No human playtest of this exact build, audible human review, OS network-isolation test, full occupied-driving benchmark or 1080p visual pass is claimed. Supplied alternative 3D models await provenance; FIELD GUIDE preference awaits clarification. See COMPETITION_NOTE.md for separate, unconfirmed eligibility. Prior v0.9.2 and the previous authorized-demo ZIP remain unchanged.
