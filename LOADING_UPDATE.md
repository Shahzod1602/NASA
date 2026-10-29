# Mission loading screens — local development, 29 October 2026

## Feature and presentation

Selecting Moon or Mars opens a destination-specific loading screen with an
English mission name, a short guide and a percentage/ring in the bottom-right
corner. The Moon guide follows the Apollo checkout, ALSEP, sample documentation,
Surveyor 3 hardware and return sequence. The Mars guide follows ARES power,
survey data, relay alignment and return, with an optional recorder reminder.
The legacy MoonBase destination uses a SELENE rescue guide.
Returning to mission selection uses an expeditions guide. Continue preserves
the existing `ResumeApollo=1` travel option; full restart uses the current map.
Restoring a checkpoint inside an already loaded mission retains its existing
in-place behaviour and does not replay a map-loading screen.

The illustrations are local authored art derived from the user's two supplied
loading-screen references. Built-in OpenAI image editing was used to remove
the reference text and loading graphics, leaving clean backgrounds; all mission
labels, guides and progress indicators are drawn dynamically in English.
The generated source copies are `SourceArt/Frontend/LoadingMoon.png` and
`SourceArt/Frontend/LoadingMars.png`. Their spacecraft and compositions are
fictional illustrations, not NASA photographs, historical reconstructions or
scientifically scaled views. The screen includes an illustration disclaimer.

## Progress and travel lifecycle

The indicator is labelled **Preparation progress**. Its targets represent
preparation milestones, not measured package bytes or a time estimate:

| Target | Event |
| --- | --- |
| 0% | Initial displayed state at map-load entry |
| 15% | Loading screen prepared |
| 70% | Matching `PreLoadMap` callback begins map loading |
| 100% | Matching `PostLoadMapWithWorld` callback confirms the loaded world |

The displayed value approaches the current target at 70 percentage points per
second. It never increases beyond that target and does not reach 100% before
the matching destination world loads. The initial zero state is held for at
least three Slate ticks. Completion is designed to show the screen for at least
2.5 seconds overall and paint 100% at least 15 times before automatic dismissal.
The post-load completion hold is bounded to four seconds so an unavailable
loading renderer cannot leave the game waiting indefinitely. The completed
Editor rendered test verified painted zero and 100% states, monotonic progress
and held world time for all eight transitions in its nine-phase flow.

`MoviePlayer` drives the Slate loading screen while map loading blocks the game
thread. Engine ticking is disabled during the completion hold, so world time
and mission resource timers remain held. The controller checks world time
around that hold. Duplicate travel requests are rejected while loading is busy;
missing map packages are rejected before travel, and travel failure clears the
loading screen. Rooted UI textures are retained for later transitions and
released by idempotent cleanup on `OnEnginePreExit`, before UObject teardown.
The subsequent module-shutdown cleanup is a no-op. No runtime internet
connection is needed.

## Import, build and validation

Import only the two textures with the supported Unreal Python commandlet:

```powershell
& 'D:/UnrealEngine/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' `
  'D:/LunarRescue/LunarRescue.uproject' -run=pythonscript `
  '-script=D:/LunarRescue/Tools/import_loading.py' -unattended -nop4 -nosound -nullrhi
./Tools/package_apollo12.ps1
./Tools/verify_loading.ps1 -Packaged
./Tools/verify_loading.ps1 -Packaged -Graphics
./Tools/verify_loading.ps1 -Packaged -Graphics -Onscreen -Width 1280 -Height 720
```

`Tools/import_loading.py` creates `/Game/Lunar/Frontend/T_LoadingMoon` and
`T_LoadingMars` as UI textures with streaming disabled. It does not edit maps.
The verification script launches the nine-phase menu/Moon/restart/menu/
Continue/menu/Mars/restart/menu flow. `-LoadingTest` uses an isolated automated
checkpoint directory; the script preserves and restores GameUserSettings bytes.
Graphics runs require the rendered-completion assertions and omit `-unattended`
because that flag can disable MoviePlayer. NullRHI runs test travel behaviour,
not loading-screen visibility, smoothness or graphics performance.

**Editor validation:** the latest Editor build succeeded. A DX11 rendered
offscreen run at 1280 x 720 completed all nine phases with 20 assertions passed,
zero failures and a clean process exit. Evidence:

- `Reports/Loading20261029/editor-offscreen-1280x720-20261029-193712-045.log`
- `Reports/Loading20261029/editor-offscreen-1280x720-20261029-193712-045-checks.txt`

Earlier visible-window inspection checked the Moon screen at 15% and the Mars
screen at 100%. The first implementation stalled at 15% because MoviePlayer
started before queued travel could execute; starting it only in `PreLoadMap`
resolved that issue. A subsequent run passed its checks but crashed at shutdown
because texture roots were removed after UObject teardown. Moving idempotent
cleanup to `OnEnginePreExit` fixed that crash; the final rendered run above
exited cleanly.

## Final packaged validation

The Editor and Windows Development targets compiled successfully with Unreal
5.8.3, MSVC 14.44 and the Windows SDK. The first full cook/package completed
with AutomationTool ExitCode 0. Final guide-text changes were rebuilt in both
targets and staged using the existing cook (`package_apollo12.ps1 -StageOnly`);
that staging run also completed with ExitCode 0.

The final executable matches `Binaries/Win64/LunarRescue.exe` byte for byte.
Both loading textures appear in the cooked runtime asset registry, and neither
rendered runtime log reports missing loading artwork.

| Final packaged checks | Assertions | Failures |
| --- | ---: | ---: |
| Apollo, legacy Moon, Mars, menu, suit, polish, guidance and fieldwork | 339 | 0 |
| Frontend settings/menu | 20 | 0 |
| Visible 1280 x 720 loading/Continue/restart flow | 20 | 0 |
| Offscreen 960 x 720 (4:3) loading flow | 20 | 0 |
| Total | 399 | 0 |

Both loading runs completed all eight transfers in the nine-world sequence.
Every transfer logged a matching destination, visible zero and at least fifteen
100% paint frames, monotonic progress and a successful arrival. Continue restored
the isolated Apollo checkpoint. Both processes exited cleanly. The validation
also rejects a missing map and a duplicate destination request.

All 110 inventoried player save/photo/config files retained their SHA-256 hashes.
The verification scripts restore GameUserSettings and use isolated automated
checkpoints. Evidence, result copies, asset metadata and executable checksum are
in `Reports/Loading20261029/validation.json` and that folder's logs.

The initial Editor visual review inspected the actual Moon and Mars screens.
The final packaged Computer Use review was stopped with physical Escape; no
further Computer Use was performed. The packaged 4:3 run validates rendering and
completion automatically; its layout was not visually reviewed. These functional
captures are not FPS benchmarks or a human playtest. No new performance claim
is made. Import Python syntax, PowerShell parsing and `git diff --check` passed.

The existing scientific terrain and source-data pipeline are not changed by
this presentation feature. No additional scientific accuracy, NASA partnership
or endorsement is claimed. GitHub push and release publication are not
authorized for this request; this iteration is local until separately requested.

## Artwork prompts and saved files

Built-in `image_gen` editing was used, with the supplied destination screenshot
as the edit target. Selected outputs were copied into the repository before
Unreal import. No CLI/API fallback was used.

- Saved Moon background: `SourceArt/Frontend/LoadingMoon.png`.
- Saved Mars background: `SourceArt/Frontend/LoadingMars.png`.

Moon prompt:

> Edit target: supplied Moon loading-screen illustration. Remove ALL typography, letters, mission name, mission tip, lower-right loading indicator, ring and percent. Inpaint only removed text/UI areas naturally with surrounding dark space and background. Preserve the existing spacecraft, Earth, Moon, lighting, composition and widescreen aspect. No new text, logos or interface elements. This is a fictional game loading backdrop; do not add NASA insignia. Save a clean text-free background for dynamic game UI.

Mars prompt:

> Edit target: supplied Mars loading-screen illustration. Remove ALL typography, letters, mission name, mission tip, lower-right loading indicator, ring and percent. Inpaint only removed text/UI areas naturally with surrounding dark space. Preserve the existing orange Mars globe, golden relay satellite with solar panels, sun flare, stars, lighting and widescreen composition. No new text, logos or UI elements. Clean text-free backdrop for dynamic game UI; fictional game illustration, no NASA insignia.
