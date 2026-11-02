# Gameplay reference and equipment diagnostics

Local development, 2 November 2026. Base revision: `a0949e4`.
This is a bounded gameplay improvement to the existing project, not a released
version or a claim of measured learning improvement.

## Research and design decision

The newly read NASA task discussion requested gameplay research, a game
reference, and specifically suggested **Space Engineers**. The following primary
Keen Software House pages were read on 2 November 2026:

| Official reference | Observed design | Application here |
| --- | --- | --- |
| [Features](https://www.spaceengineersgame.com/features/) | Its feature list describes electricity shared by connected blocks and manual assembly from components. | **Design inference:** exposing dependencies can help a player understand why a downstream function is unavailable. Keep LunarRescue's existing cell/port/cable sequence; do not import Space Engineers' construction system. |
| [How to play](https://www.spaceengineersgame.com/how-to-play/) | The guide directs players to in-game help and encourages experimenting with the blocks needed to make a ship operational, gravity and electricity. | **Design inference:** put short explanations inside the existing notebook, beside actual mission state, so help is available during play. |
| [Services Terminal Guide](https://www.spaceengineersgame.com/new-players/services-terminal-guide/) | The guide presents repair cost breakdowns and explicitly lists conditions required to store a grid. | **Design inference:** show the first unmet prerequisite and the next concrete action before asking the player to repeat an unsuccessful action. No service, economy or grid-storage mechanic is added. |

These are reference observations and our design inferences. They do not establish
that a particular UI improves learning or that either game's simplified systems
are engineering training. No Space Engineers code, artwork, audio, block designs
or other assets were copied. No multiplayer, crafting, construction inventory,
AI assistant or new resource was added.

## Implemented behavior

The existing **I** evidence/notebook panel contains a read-only diagnostic
section. It uses the panel's existing scrolling, text-scale settings and pause
behavior; there is no new binding or time-consuming inspection minigame.
`READY` means the preceding mission step is recorded. It does not promise that
the player is within interaction range, facing the equipment or unobstructed.
The normal work input still applies after closing the panel.

**SELENE and ARES:** the display explains the existing chain:

1. Recover the power cell.
2. Seat it in the station port (`RepairStep == 1` or later).
3. Connect the cable (`RepairStep == 2`).
4. Command and finish the existing station restart.
5. Recover the science-terminal story data.
6. For ARES, align the orbital relay after the download.

Each row includes a text status and a short reason. A separate `NEXT` line
identifies the first missing dependency and a concrete existing action. Cell
installation does not imply power is online. Entering `CollectData` does not
imply reboot completion: the display separately reads `PowerSequenceActive` and
`StationRebootComplete`. Missing task actors and out-of-range repair state are
identified rather than treated as successful repairs.

**Apollo 12:** a separate fieldwork checklist uses the chapter's existing
checkout stage, instrument bits, committed context-photo bits, sample mask,
Surveyor photograph flag and hardware mask. It explains why context must be
recorded before disturbing a sample or removing a part. A visual tool phase is
never counted as cargo. A pending photograph is not counted as new evidence.
The final return row uses the existing exact `7 / 7 / 3` sample/hardware/instrument
mask condition. It never invents a rescue power-cell task for this chapter.

The feature changes no completion order, stage transitions, physics, terrain,
resource costs, cooldowns, photograph commit rules, save format or controls.
The production code has no Tick, file writes, interaction calls, network access
or persistent diagnostic cache. Rebuilding the notebook reads current state.

## Evidence and model limits

These are state-derived mission records, not sensor readings. The diagnostic
feature does not calculate current, voltage, power, component damage, hardware
health or astronaut safety. SELENE and ARES are fictional authored rescue
scenarios. Their terminal downloads are story tasks, not measured scientific
results. Apollo tool setup, alignment and single-player actions remain authored
simplifications. LROC terrain provenance is separate and is documented in
[Science & Sources](SCIENCE_AND_SOURCES.md).

The existing rescue model does not retain separate download/relay booleans after
`Stage` becomes `Lost`. Those rows therefore say `UNAVAILABLE` after a failure
rather than guessing which action had been completed. Retained repair and reboot
flags may still be shown. Normal later stages provide evidence that their earlier
required download or checkout stage was passed. No diagnosis is based on an
invented physical measurement.

The notebook pauses active play using the existing panel mechanism. Inspection
itself adds no resource cost, work, cargo, score or evidence. It is optional and
can be closed without answering a lesson or finishing a diagnostic action.

## Files and integration

- `Source/LunarRescue/LunarDiagnostics.h`: small read-only snapshot/row API.
- `Source/LunarRescue/LunarDiagnostics.cpp`: mission-specific dependency text and
  notebook presentation using the current fonts, colors, wrapping and scroll
  container. Rows use text labels as well as color; there are no opaque cards or
  wide tables in the game UI.
- `Source/LunarRescue/LunarDiagnosticsChecks.cpp`: dedicated state and notebook
  integration checks. Temporary test substitutions restore their original
  values and do not write checkpoints, photographs or settings.
- `Source/LunarRescue/LunarPanels.cpp`: appends diagnostics to the existing I
  notebook overview, keeping the map and Field lessons separate.
- `Source/LunarRescue/LunarGame.cpp`: exposes the dedicated
  `EquipmentDiagnosticsTest` entry point without a new player input binding.

API: `LunarDiagnostics::Build(const ALunarGameMode*)` returns a value snapshot;
`BuildNotebook(ULunarPanel*)` appends its UI to the existing body;
`RunChecks(ALunarGameMode*)` runs the dedicated check suite and writes its report.

## Validation status and reproduction

During implementation, the existing game, photo and tool code were inspected,
including the asynchronous photo-commit flags, exact cargo requirements, reboot
completion condition and pause handling. Relevant Unreal API declarations were
checked in the locally installed engine. Whitespace/static source checks were
performed. The implementation agent did not run an engine build, executable,
rendered review, human playtest or performance capture during initial coding.
The main validation task subsequently compiled, fully cooked and staged the
final ground-contact executable:
`b034765110ad30f5a60ffcf1e874ddac679002b16c74f0e9f72bea54017a9fb4`.
The collector verified 1,254 exact PASS executions: 1,128 functional, 76
actual-travel and 50 staged HUD geometry. This includes 152 equipment/diagnostic
functional checks and 55 loading typography checks. The six equipment/diagnostic
cases cover three missions: 38 diagnostic checks per mission, plus 14 Apollo,
12 SELENE and 12 ARES equipment checks. Eighteen equipment assertions test the
later bounds-normalized cosmetic grounding; diagnostics still read actual
mission flags and reject invalid/missing evidence.

Wrappers additionally validated 110 staged PNGs and six routed Slate pointer
outcomes; those metrics are not added to the PASS total. All 17 queue runs and
the separate typography suite were rerun after the SELENE rover-map and rock
contact corrections. [The per-executable evidence](Docs/Feedback2026-11-02/evidence.json)
keeps earlier 2ed/e738 results under their own hashes; their repeated outcomes
are not additional unique tests or transferred final-build coverage. Matched
performance runs and the 264/264 unchanged protected-file comparison completed;
conditions and limits are separate in [TEST_REPORT](TEST_REPORT.md). One run per
scene does not establish whole-mission sustained FPS. Human learning results
remain unmeasured.

With the integrated `EquipmentDiagnosticsTest` entry point, reproduce checks
against an isolated test installation or with the project's normal
player-data preservation wrapper:

```powershell
$editor = 'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
& $editor 'D:/LunarRescue/LunarRescue.uproject' '/Game/Lunar/Maps/MoonBase' -game -nullrhi -unattended -nosound -EquipmentDiagnosticsTest
& $editor 'D:/LunarRescue/LunarRescue.uproject' '/Game/Mars/Maps/MarsBase' -game -nullrhi -unattended -nosound -EquipmentDiagnosticsTest
& $editor 'D:/LunarRescue/LunarRescue.uproject' '/Game/Lunar/Maps/Apollo12' -game -nullrhi -unattended -nosound -EquipmentDiagnosticsTest
```

Expected reports: `Reports/equipment-diagnostics-checks.txt` for Moon/Apollo and
`Reports/mars-equipment-diagnostics-checks.txt` for Mars. Preserve each run's
report separately: the two Moon reports share a default filename. Exit code 0
and `RESULT: 0 failures` are required; these commands are instructions, not
claims that they passed.

The suite covers missing/menu context; briefing; all cell/cable/startup states;
CollectData before reboot completion; paused read-only inspection; Mars relay;
failed missions without retained download flags; missing actors; invalid repair
values; Apollo checkout/setup; per-sample photo prerequisites; visual work versus
committed cargo; pending photos; Surveyor context before removal; exact return
masks; fresh-state reset; and the notebook integration guard for settings.

Final-build staged HUD/notebook geometry checks ran in all three missions at
1280x720 and 1920x1080, with 100/150% text fixtures. Functional regressions also
covered pause/restart, committed work, photographs and ordinary mission flow.
These automated outcomes do not establish that every screenshot was manually
reviewed or that novice players understood each row. Remaining independent
inspection should scroll the complete notebook and assess readability beside
normal work interactions. NullRHI checks are not graphical performance tests;
first-time-player learning gains require the separate real playtest protocol.


## Authorship and related local changes

The dependency formatter, optional notebook presentation and dedicated checks
are authored project code developed with OpenAI Codex AI coding assistance.
Existing licences and NASA/third-party asset notices remain unchanged. The
borderless HUD and compact loading typography are presentation changes; neither
turns a preparation percentage into byte progress or a mission flag into a
physical measurement. The separate equipment pass tracks 32 authored scene-detail
treatments, five existing tools reused in a display and two unchanged targets
across 39 research/coverage rows. It does not add 39 imported models. See
[the asset scope](ASSET_IMPROVEMENTS_UPDATE.md), [source catalogue](ASSET_SOURCE_CATALOG.md)
and [loading typography](LOADING_TYPOGRAPHY_UPDATE.md).

The later ground-contact correction levels only the existing SELENE display
rover through a backed-up supported Editor map edit. Cosmetic rocks/talus use
actual mesh bounds and the measured terrain normal with shallow 20% burial.
These are presentation repairs, not additional NASA measurements, equipment
simulation, physics changes or Space Engineers-derived mechanics. The optional
notebook still reads the same recorded mission dependencies. The final full
runtime queue includes the contact corrections. Fourteen fresh Editor Python
equipment views were visually reviewed; the latest solar-junction detail is
still inconclusive. Offline retained-OBJ triangle tests place its cube outside
the panel slab, but neither that result nor a clear collision trace visually
certifies the rendered junction.
