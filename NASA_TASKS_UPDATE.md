# Intro, compact mission interface and field lessons

Local development - 1 November 2026. This is project UI and presentation work;
the filename does not imply a NASA commission, endorsement or competition entry.
No private messages, reference recordings or participant information are included.

## Reviewed task scope

The authorized group's available history was retrieved: 704 accessible records.
Written requests and the supplied current visual references were compared with
the existing project. Older media were classified using metadata and adjacent
text; four voice notes and older illustrations were not individually reviewed.
A withdrawn planning document was excluded. Raw history and account information
remain private. No group messages were sent during implementation or testing;
release publication and its group announcement are separately authorized work.

This pass covers the current concrete interface and POV requests listed below.
Earlier proposals for an engineering campaign, equipment/power/supply allocation,
an Earth/south-pole site planner, launch/training interiors and a companion
character remain separate future scope. Field lessons do not implement those
systems. Group challenge quotations do not establish current competition rules.

## Implemented scope

- **Intro:** the unchanged user-supplied `IntroReference.jpg` is imported through
  the supported Unreal texture API. A 2.2-second authored fade runs once per
  process before home-menu input is enabled. The original image credit and
  checksum are in `SourceArt/Frontend/intro-manifest.json`; a missing texture
  uses a clearly authored text fallback.
- **Home:** Continue, Explore the Moon, Explore Mars, Settings and Exit share
  blue glass styling. Destination overview text is uppercase. Explore still
  opens the destination-specific mission picker; card selection plus explicit
  Start mission is required before travel. Continue keeps checkpoint semantics.
- **Mission HUD:** compact frosted objective/stage/progress panels, navigation,
  resource/gravity, a field guide, subtitles and contextual controls replace the
  broad active-play bands. Values come from actual mission state. Rescue work
  displays hold progress; Apollo tools retain discrete click steps. A hold ring
  does not imply automatic completion of an Apollo click-based task.
- **Pause:** Resume, Settings, Restart Mission, Back to Main Menu and Exit Game
  use a blurred live scene. The objective and remaining resource stay visible.
  Background clicks do not resume play. Restart has its own confirmation and
  reloads the current briefing while keeping the saved Apollo checkpoint. It is
  separate from paused Apollo `R`, which reloads the last checkpoint. Cancelling
  restart and returning from Settings retain the paused mission.
- **First-person view:** free hands lower and disappear during steep downward
  inspection; tool, held-cell and cargo-preview hands remain available. The
  12032 soil task's displayed step 2/2 omits near-cut leg proxies, preserving
  the separate shadow rig. These are presentation changes, not capsule or DEM
  edits. Soil lift/deposit/sealing, cargo carry and rescue work poses use
  existing action progress and do not award additional cargo.
- **Field lessons:** the `I` notebook offers three optional questions with two
  answer buttons, immediate explanations and sources. Apollo questions cover
  LROC terrain, photo context and the gameplay budget. Rescue questions identify
  authored terrain, fictional downloads and the gameplay budget. The latest
  answers are session-only, reset with a new run or checkpoint restore, and do
  not alter mission resources or cargo. The debrief adds an answer summary only
  after a real session response. This is not a validated comprehension measure.

## Controls

Use the displayed rebound work key. In rescue tasks, hold it only when prompted;
release cancels the current hold. Apollo tools open with the work key and use
left-click for each displayed work step. Esc cancels unfinished tool work before
opening pause. In pause, select an action with the mouse; menu navigation also
uses the centrally owned input paths. F1 opens Settings. `I` opens the notebook;
select **Field lessons**, then use `1` / `2` to answer and left/right to change
questions. Esc closes the notebook. Apollo `H` hides/shows its field guide.

## Data, assets and limits

Apollo 12 ground/collision still uses `NAC_DTM_APOLLO12` at 2 m source spacing,
with modern LROC map/mapped worksite data. SELENE and ARES retain authored maps.
Small fragments, visual poses, intro art and cargo arrangements are authored.
See [current provenance](SCIENCE_AND_SOURCES.md) and [credits](CREDITS.md).
The user-supplied intro is an illustration, not a NASA survey image.

The game remains a stylized educational expedition. Neither rendered hand poses
nor lesson answers establish biomechanical accuracy, suit endurance, astronaut
competence or learning gains. A/B route planning, slope-based forecasts and
route-causation telemetry remain separate backlog work. Human playtest sessions
have not been supplied for this iteration. Competition eligibility is unchanged
and requires its own evidence.

## Build and verification

Use `Tools/package_apollo12.ps1` with the installed Unreal toolchain. The supplied
intro texture can be imported with `Tools/import_intro.py` through Unreal's
Python commandlet; map regeneration is not part of this pass. The existing local
launcher is `Play Lunar Rescue.cmd` and the packaged bootstrap is
`Dist/Apollo12/Windows/LunarRescue.exe` after a successful package.

The following are reproduction commands, **not results**:

```powershell
./Tools/verify_nasa_tasks.ps1 -Packaged
./Tools/verify_nasa_tasks.ps1 -Packaged -Review -Width 1280 -Height 720
./Tools/verify_nasa_tasks.ps1 -Packaged -Review -Width 1920 -Height 1080
./Tools/verify_mission_selection.ps1 -Packaged
./Tools/verify_frontend.ps1 -Packaged
./Tools/verify_loading.ps1 -Packaged -Graphics
./Tools/verify_apollo12.ps1 -Packaged -Regression -Checkpoints
```

**Final local validation:** Editor/Game builds and Windows cook/stage/archive passed.
The final executable `ee207b31703b981eeec7cd9637e5a69092bd028a820d2d16d02e6274b28d4b6d` passed **997 packaged assertion/outcomes**
(275 new and 722 regressions), with zero failures. 156 fresh staged PNGs at
720p/1080p and 30 routed Slate pointer checks were validated. Targeted visual
review confirmed Apollo win/loss lesson summaries, 150% text and the exact soil
step 2/2 work view. Full ordinary-movement Apollo traversal also completed.
131 original player/config files match their preserved hashes after restoring
one automatically written Editor preference. Human playtest sessions: **0**.

A short DX11/720p/Medium/60-cap pair measured mean 59.978 FPS for the final menu
and 59.985 FPS for the opening walk. Walk p99 increased from 17.669 to 18.075 ms;
these short capped samples do not establish whole-mission stability or GPU
headroom. No network-disconnected or interactive PIE run was performed.

See [TEST_REPORT](TEST_REPORT.md) for exact commands, environment, counts,
performance limits and unperformed checks. The machine receipt is
`Reports/NasaTasks20261101/validation.json`. No remote push, release or
competition submission was performed.
