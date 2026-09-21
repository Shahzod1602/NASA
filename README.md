# Lunar Rescue

A first-person rescue prototype with separate Moon and Mars expeditions. All in-game text is in English.

## 2026-09-21 audit holati

Auditdan keyin mavjud menu click xatosi source darajasida tuzatildi:
endi faqat planet kartasini bosish missiyani boshlaydi.
[Maintenance qaydi](MAINTENANCE_FOLLOWUP.md). Tarqatilgan v0.2.0 bu tuzatishni hali o‘z ichiga olmaydi.

Mavjud Moon/Mars o‘yini qayta tekshirildi. NASA terrain, route planner va ta’limiy
debrief hozircha **amalga oshirilmagan**. Joriy 2026 Space Apps qoidasi challenge
ustida hakatondan oldin ishlashni taqiqlagani uchun yangi implementatsiya backlogda.
Oldingi v0.2.0 kodini tanlovda qayta ishlatish huquqi tasdiqlanmagan.

- [Texnik audit](TECHNICAL_AUDIT.md) va [yangi test natijalari](TEST_REPORT.md).
- [Ilmiy manbalar va LOLA nomzodi](SCIENCE_AND_SOURCES.md).
- [Tanlovga tayyorlik](COMPETITION_READINESS.md) va [rivojlantirish rejasi](DEVELOPMENT_PLAN.md).
- [Credits/litsenziya](CREDITS.md), [o‘zgarishlar](CHANGELOG.md), [playtest](PLAYTEST_PROTOCOL.md).
- [Shartli inglizcha taqdimot](PRESENTATION_OUTLINE.md); tayyor submission emas.

Rejalashtirilgan mahsulot yo‘nalishi: “LunarRescue is an educational exploration
game that uses real planetary data to help players understand how terrain,
limited resources, and mission priorities affect decisions on the Moon and Mars.”
Bu kelajak maqsadi; joriy build hali haqiqiy planetary elevation ishlatmaydi.

## Play the standalone Windows build

Download the tested Windows ZIP from the [GitHub release](https://github.com/Shahzod1602/NASA/releases/tag/mars-expedition-v0.2.0),
extract the complete folder, and run `LunarRescue.exe`.

Open **Play Lunar Rescue.cmd**. When `Dist/Windows/LunarRescue.exe` is present,
the launcher uses the standalone build, which does not require Unreal Editor.
Keep the entire `Dist/Windows` folder together when copying the game, including
its Engine, LunarRescue and Credits subfolders.

To regenerate the package after source or content changes, run
`Tools/package_windows.ps1` (`-EngineRoot` selects a different engine installation).
If no standalone package is present, the launchers fall back to the editor game build.

## Get the project

Requires Unreal Engine **5.8.2**, Git LFS, and the Visual Studio C++/Windows SDK
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
The editor fallback uses the installed Unreal Engine 5.8.2.

## Controls

- Enter or left click: start the mission; resume from pause.
- WASD or arrow keys: move.
- Mouse: look around.
- Space: jump.
- E: interact; hold for station restart, science download, and crew recorder.
- Q: after collecting science data, switch between the optional recorder and lander route.
  This changes the navigation marker only: a nearby lander or recorder remains usable.
- F: toggle visor zoom to inspect Earth and the night sky.
- Escape: pause or resume.
- R or Enter: restart after winning or losing.
- Alt+F4: close the game.

## Mission

Find the power module beside the stranded rover, install it at SELENE's external power port,
download the research data from the nearby terminal, and return to the lander console.
You have a 300-second gameplay oxygen budget, not a modeled real suit endurance.
The briefing and pause screen do not consume this budget.

Repair has three steps: insert the cell, connect the cable, then hold E for two
seconds to start a 4.2-second station restart. The emergency red signal
fades, four roof lights switch on in order, the relay dish turns, and the science terminal
comes online. The suit plays short switching cues and English radio confirms restored power.
Movement and oxygen continue during the restart; pause freezes its progress. The science
terminal can be used as soon as the restart finishes.
Pause also preserves the radio subtitle and pauses all station switching sounds.
Falling outside the mission area shows a distinct failure reason from oxygen depletion.

After downloading the science data, return directly to the lander or follow
the optional crew recorder marker. Jump onto the survey ledge and hold E to
recover the crew's story. The detour uses the same oxygen budget. Both routes
can win; recovering the recorder unlocks a different ending. Results show
remaining oxygen, restored power, secured science, and recovered logs.

The lunar ambience uses hard sunlight, dark unlit terrain, a restrained emergency beacon,
and local work lights. In a vacuum there is no wind, airborne dust cloud, or atmospheric fog.

## Suit feedback

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

- Map: Content/Lunar/Maps/MoonBase.
- Astronaut: Content/Lunar/Blueprints/BP_Astronaut. Adjust speed and jumping under Character Movement.
- Mission: Content/Lunar/Blueprints/BP_LunarGameMode. The current C++ BeginPlay overrides
  Oxygen Capacity with 300 seconds for Moon or 480 for Mars; changing Class Defaults alone
  does not change the runtime budget. Configurable mission profiles remain planned work.
- Mission tags: Module, Power, Data, Home. Place exactly one actor with each tag.
- Gameplay and interface: Source/LunarRescue/LunarGame.cpp.
- This project combines editable Blueprint child classes with C++ gameplay.
- Tools/build_moon.py creates the initial map. Do not run it over a manually edited map.

## Scientific scope

Gravity is -162 cm/s2 (1.62 m/s2). Movement and jumping are simplified for gameplay.
The terrain is procedural, Earth uses a cloud/continent texture, the stars use a real catalog,
and the research records are a narrative objective,
not an actual measurement dataset. VR, NASA terrain data, and competition-specific requirements are not implemented.
Environment sources and rendering compromises are recorded in SourceArt/LunarV2/CREDITS.md.

The V3 surface adds irregular overlapping impact bowls, scalloped rims and radial ejecta relief,
denser near-field terrain, fractured rock variants, clustered debris and layered regolith shading.
The station area retains shallow relief for accessible movement. Small debris casts shadows but
does not obstruct the astronaut or interaction traces. This is authored procedural scenery,
not a reconstruction of a surveyed landing site.
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

## Imported free models
The map now uses an Apollo lander, two habitat parts, a Lunokhod rover, solar arrays, a power cell and a science terminal from the user's downloaded files. Existing procedural rocks use Poly Haven Moon Rock 01/06 diffuse textures at 2K. Asset sources and licenses are in SourceArt/ExternalModels/CREDITS.md. Import scripts are in Tools; their staging paths refer to this workstation. The mission actors and English UI remain intact.

## Moon and Mars

The startup menu offers **1 / Moon** and **2 / Mars**. Click the corresponding
card or press its number. Enter starts Moon. Press **M** from a briefing, paused
mission, or result screen to return to selection. **R** on a result screen
restarts the current planet.

Moon retains its five-minute SELENE rescue. Mars has eight minutes of oxygen,
3.71 m/s? gravity, a separate authored canyon map and ARES outpost. Retrieve the
rover cell, insert it, connect the cable, hold E to restart the station, download
the ice survey, and hold E for four seconds at the orbital relay. Return to the
ascent lander; Q optionally routes to the crew recorder after relay alignment.
All terrain is fictional; no surveyed planetary elevations are claimed.

`Tools/build_mars.py` regenerates only Mars and the menu using Unreal Python.
`Tools/import_mars_audio.py` imports the supplied English radio WAV files.
`Tools/verify_planets.ps1` tests map travel, both gameplay sequences, Mars movement
and the recorder ledge. Add `-Walkthrough -Graphics` for complete traversals with
rendering and performance reports, or `-Packaged` to test the standalone binary.
Tests are sequential to fit a 16 GB development machine.

See [MARS_UPDATE.md](MARS_UPDATE.md) for implementation and measured standalone validation.
