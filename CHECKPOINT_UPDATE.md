# LunarRescue checkpoint update - v0.4.3

This records the save system introduced in v0.4.3. The current English-only release is described in [ENGLISH_UPDATE.md](ENGLISH_UPDATE.md). Checkpoint format v1 and saved mission identifiers are unchanged.

## Playing and saving

- Starting Apollo and completing each committed action creates a checkpoint: checkout, ALSEP instruments, context photos, samples, Surveyor arrival, hardware and cargo stowage.
- **C** in mission selection, the Apollo briefing or the exercise-ended screen continues the save. A completed save opens its result.
- **Esc**, then **R** reloads the last checkpoint. Time, distance and work return to their saved values.
- Another expedition asks for confirmation: **Enter** starts a new run; **Esc** cancels. An extra mouse click cannot accept the confirmation.

Closing while walking returns to the last completed task, rather than the exact exit position. Partial E holds, airborne velocity and pause state are not restored. Loading validates terrain collision and places the astronaut on the ground. Collected samples and hardware remain collected.

## Files and recovery

The snapshot stores 17 stable work identifiers, mission/mode/data profile, run ID, position/view, telemetry, antenna bearing, selected destination, tutorial progress and photo ID/site/time/path. Actor pointers and enum/array ordering are not serialized.

```text
LunarRescue/Saved/SaveGames/Apollo12/Apollo12.sav
LunarRescue/Saved/SaveGames/Apollo12/Apollo12.sav.bak
LunarRescue/Saved/Apollo12/Photos/<run-id>/*.png
```

The editor uses the project Saved folder. Keep LunarRescue/Saved when moving the standalone game. Packaging backs up saves, run photos and GameUserSettings.ini under Backups/player-data-<date>, restoring files lost during staging without replacing newer copies.

Every run has its own photo folder. Missing images warn without removing completed work. An unexpected shutdown between a screenshot request and its disk write may leave an image missing. There is no in-journal photo album yet.

The small USaveGame object uses [Unreal memory serialization](https://dev.epicgames.com/documentation/unreal-engine/saving-and-loading-your-game-in-unreal-engine?lang=en-US). An envelope checks version, size and CRC before deserialization, with a 512 KiB limit. Semantic checks reject unknown/duplicate work, missing prerequisites, incompatible mission/profile/mode, invalid coordinates, non-finite telemetry and unsafe photo paths.

A temporary file is written and checked before replacement. A valid primary rotates into the backup first. Failed temporary writes or backup rotation retain the primary; a damaged primary never replaces a good backup. Loading tries the primary and then backup, with a recovery notice. This is not hardware power-loss testing. Saves are synchronous; the separate-process fixture produced a 3,127-byte primary.

## Historical v0.4.3 validation

| Checks | Editor | Windows package |
|---|---:|---:|
| Apollo, legacy Moon, Mars, menu, suit and input | 182 | 182 |
| Save round trips, fault recovery, confirmation and R | 51 | 51 |
| Separate-process write | 2 | 2 |
| Fresh-process menu C resume | 7 | 7 |
| Total, zero failures | **242** | **242** |

Package writer/reader PIDs were 8796/13124; editor PIDs were 18640/7612. Resume checked telemetry, destination, evidence, hidden objects and checkpointing the next sample. Tests use Saved/AutomatedTests; ordinary regression/review runs do not access player checkpoints.

Both input/collision walkthroughs passed: direct route **1,237.29 m / 932.13 simulated seconds**; historical route **1,344.67 m / 1,009.13 seconds**. These are accelerated headless runs with fixed 1/60-second steps, not human timing or GPU benchmarks.

The v0.4.3 DX11 offscreen opening capture used 1280x720, normal delta time and a 60 FPS cap. Of 4,200 frames, 600 warmup frames were removed. The remaining 60.01 seconds averaged **59.99 FPS**, 1% low **55.74 FPS**, p95 **17.18 ms**, p99 **17.44 ms**, maximum **27.95 ms**. Sampled process working-set peak was about 570 MiB, not VRAM or total system memory.

Four saves succeeded: mean **10.61 ms**, maximum **11.25 ms**; three were inside warmup. This covers Intrepid checkout, not full-mission save cost, Low-preset performance or a general guarantee. Large future saves should reconsider asynchronous writing. Evidence: Reports/Performance/v043.

There were 21 validated PNGs at 1280x720 Uzbek and 21 at 1920x1080 English, plus a menu image. Five save states were visually reviewed in both languages. These were prepared fixtures, not first-time player tests. That bilingual behavior was superseded in v0.4.4.

All 357 Content/SourceArt hashes matched baseline. The backup is Backups/before-checkpoints-20261024-135347/manifest.json. Initial virtual-memory and linker failures were resolved by disabling the module PCH and building editor/game in separate UBT processes; failed logs were retained and final build/tests passed.

Use Tools/verify_apollo12.ps1 -Checkpoints, adding -Packaged for the standalone build. Reports are under Reports/*-apollo12-*-result.txt. Full settings, a skeletal suit, distinct science interactions and UMG remain separate work.
