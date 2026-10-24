# Lunar Rescue

## Checkpoint update — 0.4.3

Apollo 12 now saves committed work automatically. **C** continues the saved expedition from mission selection or the Apollo briefing; **Esc**, then **R** reloads the last checkpoint. Starting a new expedition asks before replacing existing progress. Samples, photographs, Surveyor hardware and exercise telemetry survive a restart. See [save locations, recovery and validation](CHECKPOINT_UPDATE.md). Walking after the last completed task is not saved on exit.

## Onboarding update — 0.4.2

Apollo 12 and mission selection now default to Uzbek, with **L** switching to English. The Apollo HUD shows one current task with its purpose; the full checklist is in the paused **Tab / J** journal. A short look/walk/work guide introduces the controls. Surveyor can be reached along a player-chosen route; the historical traverse remains on the map. See [the update and validation notes](ONBOARDING_UPDATE.md).

## Current development plan — 2026-10-24

The [full game improvement plan in Uzbek](GAME_MASTER_PLAN.md) covers onboarding, a rigged astronaut and hand animations, distinct science interactions, saving, NASA source accuracy, performance, testing and an estimated 8–12 week roadmap. It describes future work, not completed features.

## Historical Moon expedition — 2026-10-24

The Moon selection now opens **Apollo 12: Surveyor's Legacy**, a playable chapter based on Apollo 12's surface work. NASA/LROC terrain, mapped sample sites and EVA traverse points support a route from Intrepid to Surveyor 3 and back. See [Apollo 12 implementation and sources](APOLLO12.md). The earlier fictional SELENE expedition remains in the project as a separate map.

A first-person expedition prototype with separate Moon and Mars missions. Apollo 12 and the menu have Uzbek/English text; the older Moon/Mars mission interfaces remain English.

The 0.4.1 [astronaut update](SUIT_UPDATE.md) adds fitted suit meshes, articulated gloves, visible boots and legs, Shift fast stride and a V following-camera view. Hold Alt in that view to inspect the suit. Apollo instructions now name the destination and explain the next action.

## Old audit snapshot (2026-09-21)

This section records the project state before the Apollo 12 update. Its statements about planned NASA terrain and deferred implementation are historical; Apollo 12 now uses NASA/LROC data described in [APOLLO12.md](APOLLO12.md).

Auditdan keyin mavjud menu click xatosi source darajasida tuzatildi: endi faqat planet kartasini bosish missiyani boshlaydi. [Maintenance qaydi](MAINTENANCE_FOLLOWUP.md). Tarqatilgan v0.2.0 bu tuzatishni hali o'z ichiga olmaydi.

Oldingi Moon/Mars o'yini va audit yozuvlari o'sha sanada bo'lgan holatni aks ettiradi. Tafsilotlar: [texnik audit](TECHNICAL_AUDIT.md), [ilmiy manbalar](SCIENCE_AND_SOURCES.md), [credits](CREDITS.md), [o'zgarishlar](CHANGELOG.md) va [playtest protokoli](PLAYTEST_PROTOCOL.md).

## Play the standalone Windows build

Download the Windows ZIP from the [Apollo 12 v0.4.3 release](https://github.com/Shahzod1602/NASA/releases/tag/apollo12-v0.4.3), extract the entire archive, and run **LunarRescue.exe**. Unreal Editor is not required. The package includes its asset credits; keep the Engine, LunarRescue and Credits folders together. The earlier [v0.2.0 Mars release](https://github.com/Shahzod1602/NASA/releases/tag/mars-expedition-v0.2.0) remains available.

The local Apollo 12 package is under `Dist/Apollo12/Windows`.

Open **Play Lunar Rescue.cmd**. It prefers `Dist/Apollo12/Windows/LunarRescue.exe`, then `Dist/MoonExpedition/Windows/LunarRescue.exe`, then the previous `Dist/Windows` package or editor game build. The standalone build does not require Unreal Editor.
Keep the entire `Dist/Apollo12/Windows` folder together when copying the historical chapter, including
its Engine, LunarRescue and Credits subfolders.

To regenerate the package after source or content changes, run
`Tools/package_apollo12.ps1` (`-EngineRoot` selects a different engine installation). This writes `Dist/Apollo12/Windows` and preserves the earlier packages.
If no standalone package is present, the launchers fall back to the editor game build.

## Get the project

Requires Unreal Engine **5.8.3**, Git LFS, and the Visual Studio C++/Windows SDK
toolchain for Unreal projects on Windows. This repository contains the editable
project and assets, not a standalone executable.

```sh
git lfs install
git clone https://github.com/Shahzod1602/NASA.git
cd NASA
git lfs pull
```

Open `LunarRescue.uproject` with Unreal Engine 5.8 and allow it to build the
missing C++ modules. If needed, generate Visual Studio project files and build
the `LunarRescueEditor` target in **Development Editor / Win64** first.
Open `/Game/Rescue/Maps/MissionSelect`, then use Play in the editor.

The `.cmd` launchers currently expect the engine under
`D:\UnrealEngine\UE_5.8`; edit that engine path for another installation.
They resolve the project relative to the launcher location. The low-memory
launcher limits the texture streaming pool to 384 MB and uses shadow quality 2.
The project now uses that conservative profile by default, with a 60 FPS cap and
texture residency limited to available VRAM. The low-memory launcher remains compatible.
Generated build files, caches, local backups, and test screenshots are excluded.
Use Git LFS when cloning: the large binary assets must be downloaded, not left
as pointer files. Asset credits are included under `SourceArt`.

Open **Play Lunar Rescue.cmd** to play, or **Open Lunar Editor.cmd** to edit the project.
The editor fallback uses the installed Unreal Engine 5.8.3.

## Controls

- Enter or left click: start the mission; resume from pause.
- WASD or arrow keys: move.
- Mouse: look around.
- Space: jump.
- Shift: hold for a faster stride in Apollo 12.
- V: toggle helmet/following view; hold Alt and move the mouse to orbit the suit.
- E: document a site, interact, or hold to work and stow cargo.
- Q: select the next Apollo sample or turn the ALSEP antenna. In the legacy SELENE map, Q cycles its markers.
- F: toggle visor zoom.
- Tab / J: open the Apollo field journal; it pauses movement and the exercise clock.
- L: switch Uzbek/English in Apollo 12 or mission selection; the choice is saved.
- Escape: pause or resume.
- R or Enter: restart after winning or losing.
- Alt+F4: close the game.

## Apollo 12: Surveyor's Legacy

This condensed educational route covers selected work from Apollo 12's two historical EVAs. Check out at Intrepid, deploy a simplified ALSEP station, document three mapped samples, follow Conrad's reconstructed traverse to Surveyor 3, photograph it, recover its camera, scoop and cable, and return the evidence to Intrepid. The real crew later returned the samples and hardware to Earth. The playable chapter ends at cargo stowage; ascent, rendezvous and Earth return remain historical context.

NASA/LROC terrain and mapped work locations retain their source coordinates. The chapter combines historical activities into one single-player route. Models, interaction durations, the 30-minute exercise clock and paraphrased messages are gameplay adaptations. The clock is not suit endurance. See [APOLLO12.md](APOLLO12.md) for details and sources.

The earlier fictional SELENE chapter, **The Lost Expedition**, remains documented in [MOON_EXPEDITION.md](MOON_EXPEDITION.md) and is retained as a separate legacy map.

## Legacy SELENE suit feedback

The first-person view includes procedural glove geometry, gentle hand sway and a peripheral
helmet seal. Visor zoom hides the hands. Grounded movement leaves alternating tread decals
on lunar terrain; the latest 160 prints remain until the mission restarts. Prints have no collision.
Landing on regolith throws up a small pool of 16 grain particles following ballistic arcs;
particles stop at terrain contact and freeze during pause.
Breathing and muffled suit-conducted steps play during the mission and pause with gameplay.
English radio messages follow the mission stages, with seven additional story
recordings using a filtered synthetic voice.
There is no outdoor wind or airborne footstep sound effect.

Original synthetic audio is generated by Tools/prepare_immersion.py; radio source speech was
created locally using Windows Microsoft Zira. Tools/import_immersion.py imports audio and
creates the glove and footprint materials. The hands are a procedural prototype, not a rigged
character model. Carrying and held interactions use simple procedural hand poses;
a station interior and fully rigged interaction animations are not implemented.

## Editing

- Default Moon map: Content/Lunar/Maps/Apollo12. Legacy SELENE map: Content/Lunar/Maps/MoonBase.
- Astronaut: Content/Lunar/Blueprints/BP_Astronaut. Adjust speed and jumping under Character Movement.
- Mission: Content/Lunar/Blueprints/BP_LunarGameMode. C++ sets 480 seconds for Mars and the legacy SELENE map; Apollo 12 sets a separate 1,800-second exercise window. Changing Class Defaults alone does not change these runtime budgets.
- Mission tags: Module, Power, Data, Home. Place exactly one actor with each tag.
- Gameplay and interface: Source/LunarRescue/LunarGame.cpp.
- This project combines editable Blueprint child classes with C++ gameplay.
- Tools/build_moon.py creates the initial map. Do not run it over a manually edited map.

## Scientific scope

Moon gravity is -162 cm/s2 (1.62 m/s2); Mars gravity is -371 cm/s2 (3.71 m/s2). Movement and jumping are simplified for gameplay. Apollo 12 uses LROC stereo-derived elevation data described in [APOLLO12.md](APOLLO12.md); the legacy SELENE and Mars terrain remain authored scenery. Legacy research records are narrative objectives, not measurements. VR is not implemented.
Environment sources and rendering compromises are recorded in SourceArt/LunarV2/CREDITS.md.

The V3 surface adds irregular overlapping impact bowls, scalloped rims and radial ejecta relief,
denser near-field terrain, fractured rock variants, clustered debris and layered regolith shading.
The station area retains shallow relief for accessible movement. Small debris casts shadows but
does not obstruct the astronaut or interaction traces. On the legacy SELENE map this is authored procedural scenery, not a reconstruction of a surveyed landing site.
Rebuild V3 assets with Tools/prepare_lunar_v3.py, then run Tools/import_lunar_v3.py and
Tools/upgrade_lunar_v3.py through the Unreal Python commandlet. The upgrade replaces generated
environment actors, so back up the map before rerunning it after manual environment edits.

## Verification

Build the LunarRescueEditor Win64 Development target using the installed engine's Build.bat.
Run UnrealEditor-Cmd with the project and /Game/Lunar/Maps/MoonBase -game -nullrhi -unattended -LunarTest
for the mission checks, or -LunarMovementTest for sustained keyboard movement and jump checks.
`-LunarWalkthroughTest` walks from the actual spawn through all objectives, including the
optional recorder ledge and the return to the lander, using normal movement and collision.
It writes route progress, oxygen remaining, frame timing and peak process memory to
`Reports/walkthrough-checks.txt`. NullRHI results do not measure graphics performance.
Run `Tools/verify_audit_fixes.ps1` for the complete suite, or add `-Graphics` for an
offscreen 1280x720 DX11 run with the low-memory settings. Set `-EngineRoot` if needed.
`-LunarMissingLedgeMesh -LunarTest` exercises the safe fallback for a missing ledge mesh.
Reports are saved under Reports.
`-LunarLedgeTest` checks the optional ledge using simulated keyboard movement and
jumping. The updated mission suite passes 50 checks without sound or 52 with sound;
the ledge suite passes 3 checks. See [audit fixes and measured results](AUDIT_FIXES.md).
Mission checks teleport between objectives and are not a timed full playthrough.
See [story update notes](Reports/STORY_UPDATE.md) for the implemented scope.

## Legacy SELENE imported models
The legacy map uses an Apollo lander, two habitat parts, a Lunokhod rover, solar arrays, a power cell and a science terminal from the user's downloaded files. Existing procedural rocks use Poly Haven Moon Rock 01/06 diffuse textures at 2K. Asset sources and licenses are in SourceArt/ExternalModels/CREDITS.md. Import scripts are in Tools; their staging paths refer to this workstation.

## Moon and Mars

The startup menu offers **1 / Moon** and **2 / Mars**. Click the corresponding
card or press its number. Enter starts Moon. Press **M** from a briefing, paused
mission, or result screen to return to selection. **R** on a result screen
restarts the current planet.

Moon opens Apollo 12: Surveyor's Legacy. Mars has eight minutes of oxygen,
3.71 m/s2 gravity, a separate authored canyon map and ARES outpost. Retrieve the
rover cell, insert it, connect the cable, hold E to restart the station, download
the ice survey, and hold E for four seconds at the orbital relay. Return to the
ascent lander; Q optionally routes to the crew recorder after relay alignment.
Mars terrain is fictional; no surveyed Mars elevations are claimed.

`Tools/build_mars.py` regenerates only Mars and the menu using Unreal Python.
`Tools/import_mars_audio.py` imports the supplied English radio WAV files.
`Tools/verify_planets.ps1` tests map travel, both gameplay sequences, Mars movement
and the recorder ledge. Add `-Walkthrough -Graphics` for complete traversals with
rendering and performance reports, or `-Packaged` to test the standalone binary.
Tests are sequential to fit a 16 GB development machine.

See [MARS_UPDATE.md](MARS_UPDATE.md) for implementation and measured standalone validation.
