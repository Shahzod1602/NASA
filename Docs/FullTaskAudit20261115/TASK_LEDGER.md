# Current Moon/Mars demo task audit — 15 November 2026

Scope confirmed by the project owner: finish the current Moon/Mars demo. The separate OXYGEN/rocket cabin game and Mission Designer are backlog, not silently claimed as delivered. This ledger supersedes older pending-status notes for this worktree; dated test receipts remain historical evidence for their own binaries.

## Coverage

The authorized local Telegram MCP returned 480 accessible NASA-group messages, IDs 993–1500, when requested with a 5,000-message limit. All referenced reply parents were present. Subsequent deltas through 1510 were checked; no additional written game task or answer to the model/FIELD GUIDE questions was present. This does not establish coverage of deleted or inaccessible older messages. Raw chat history and identities remain outside the repository.

Fourteen voice/video notes were downloaded on a bounded retry and processed locally with Whisper small/int8: 1038, 1103, 1134, 1207, 1236, 1241, 1251, 1286, 1303, 1316, 1317, 1346, 1352, 1370. A targeted local large-v3-turbo retry covered 1303, 1316, 1317, 1346, 1352. No audio was uploaded to an external transcription service. The first Telegram transcription attempt required Premium; a local decoding compatibility problem was resolved by resampling through PyAV. Automatic Uzbek transcription is imperfect. Message 1303 concerns loading-page presentation, 1346 asks about challenge requirements and 1352 requests the already-delivered core concept (1353); these are corroborated by written follow-ups. 1370 expresses differentiation concerns, without a concrete additional feature. 1316/1317 concern reference-page image/generation coordination, but remain unsuitable as exact independent acceptance criteria; 1241 produced no speech text. Do not treat this as a human listening audit or claim every spoken nuance is resolved. Raw audio/transcripts remain private.

## Current demo requirements

| Group message IDs | Requirement / final behavior | Implementation and verification target |
|---|---|---|
| 1083, 1098, 1099 | Space Tech: names, short specifications, mission and purpose; no new 3D scene required | `LunarSpaceTech.cpp`; mission-specific read-only hardware guide in Science & Sources; native source/side-effect checks |
| 1135 | Prioritize existing SELENE mission | Current repair/science/return loop retained; field/gameplay/rescue suites |
| 1138 | Remove forced resource-profile modal | Balanced profile applied after EVA; existing field checks |
| 1139, 1377 | No forced cabin/camera zoom takeover | Normal surface entry; legacy departure scene remains explicit-test-only |
| 1140, 1141 | Preflight equipment interaction lag | Progress updates existing widgets; cached map retained. Rendered functional checks; no unmeasured UI FPS claim |
| 1153, 1392, 1487 | Pre-EVA reference; sequential checks, no Next, progress, blue Open hatch | Shared expedition UI and clean cabin illustration; four real timed checks, production hatch callback at 100/150% text |
| 1156, 1157 | Suit and first/third-person movement | Existing adapted expedition rig, pose and camera fixes; gameplay/stride checks |
| 1159–1164, 1171 | Remove redundant props/PIP | Prior cleanup retained; interactive mission equipment preserved |
| 1167, 1168 | Museum Moon rover | Credited CC BY-SA adaptation retained; actual drive/camera/collision tests |
| 1169, 1170, 1196 | Ground habitat, solar and power board | Runtime terrain contact retained; source maps unchanged |
| 1175, 1176, 1200, 1201 | Bottom FIELD GUIDE card | Currently hidden; primary objective/prompts retained. Conflicting later wording awaits clarification (question 1503). No arbitrary reversal |
| 1189–1191 | Perseverance appearance for NOMAD | Existing NASA/JPL reference retained; fictional Moon deployment explicitly identified |
| 1192, 1193, 1307, 1349 | Better habitat/lander models | Both supplied files located and inspected; original source/license absent. Questions 1504/1505 pending. Existing credited NASA habitat/lander stay usable; supplied alternatives are not falsely marked integrated |
| 1194, 1195, 1381, 1382 | Hands and power-cell grip | Prior paired handles/grips and palm correction retained; rescue/visual checks |
| 1198, 1199, 1398 | Cable board and immediate close after Verify | Existing dark drag board, guarded Verify, normal station restart retained; native and rendered callbacks |
| 1304, 1305, 1314, 1315, 1339, 1420 | Startup branding/art | Separate supplied initial AstroKid art remains; latest full-plate requirement below supersedes naming sketches |
| 1375, 1401 | Moon walking/hop and no airborne steering | Existing Moon tuning retained; Mars remains separate; stride/native input checks |
| 1378 | Remove instruction board | Earlier board removal retained; useful service pad preserved |
| 1380 | Rover driver, mouse camera, V view, stationary mudguards | Existing drive controller/visual rig retained; actual input checks |
| 1385, 1383, 1488 | Supplied Moon arrival video | Authorized local H.264/AAC clip bundled; new Moon entry only; actual EOF/world-clock checks. AI fictional portrayal label, no archival claim |
| 1394 | Planet map reference: rotate and zoom | Preparation opens offline globe by default; local mission points remain separate and selectable; actual material/callback checks |
| 1395 | Inventory reference | Shared charcoal expedition UI, live item grid, categories, scroll area and details; no fabricated stock/quantities |
| 1399 | Quiet and immediate rover entry | Existing direct boarding retained; no forced scene/audio |
| 1402 | List all Moon/Mars missions | Current selection remains Apollo 12, SELENE and ARES; selection suite |
| 1413, 1414 | Floating cyan power-cell beacon | Existing attached/hide-on-pickup behavior retained |
| 1415, 1416 | Lunokhod prop capsule collision | Existing sweep/collision fixes retained; rescue tests |
| 1417–1419 | Mast/red light floating | Existing terrain/roof-supported placement and mounted port light retained |
| 1422, 1489, 1497 | Full supplied intro plate after initial loading | Previously only retained in Credits; now separate cooked T_Acknowledgements, aspect-fit intro and team-provided/AI caption; startup checks distinguish both textures |
| 1474 | Real gameplay screenshots for presentation | Deliver actual fresh engine captures, identify scripted fixtures; never label them human playtests |
| 1500 | Updated GitHub project link | Delivery must identify the exact tested feedback build; old v0.9.2 is not silently overwritten |

## Explicit backlog and non-code work

- 1010–1012, 1027, 1104: separate OXYGEN/rocket cabin mission. Supplied DOCX was read, including timed emergency/tool/console outcomes. Deferred by owner scope decision.
- 1333–1335: Mission Designer concept/requirements. Earlier assessment and core concept were supplied (1348, 1353); the new mode is deferred. Optional multiplayer/sandbox suggestions are not current acceptance criteria.
- 1448: submitted-artifact stability request. Keep dated public artifacts intact and separate feedback development; this work does not alter a competition submission.
- 1459: team names/photos/roles for a presentation are team-owned information, not invented by the coding agent.
- Formal competition eligibility remains unconfirmed. The team-supplied 14-page guide capture was read; the live official pages failed to load. Its AI/branding restrictions are a separate readiness issue and do not negate the owner's authorization for the local demo.

## Closure rules

Code-complete is distinct from build-tested, packaged, published and human-approved. See TEST_REPORT.md in this folder for this batch's actual checks. Unanswered model provenance, ambiguous FIELD GUIDE preference and unavailable voice content remain explicit exceptions; they are not concealed by an “all tasks done” claim.
