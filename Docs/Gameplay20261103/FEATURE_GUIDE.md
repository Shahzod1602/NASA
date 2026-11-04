# Gameplay feature guide — 3 November 2026

Four additions are integrated: departure presentation, carried objects, fictional
guides, and an optional water-evidence study. The final local Windows executable
5171c1b0 passed 2,400 C++ PASS executions, 160 staged PNG checks and eight paired
cell-contribution checks. These are repeated automated executions, not unique
tests or human playtests. See [the exact evidence and limits](../../TEST_REPORT.md).
LunarRescue is an educational game, without NASA endorsement or partnership.

## Departure cabin

Choose a destination and mission, use **Start Mission**, then press **Enter** at
the briefing. The new-run presentation opens a hatch and moves the camera toward
the existing surface view, handing control back after an intended 9.4 seconds.
**Enter** or **left click** skips after a 0.3-second guard; **Esc** pauses. The work
key does not skip it. Mission resources remain in the briefing state until the
normal start is committed once.

This is a transient authored transfer cabin built from retained shapes/materials,
not a walkable habitat, launch simulator or reconstruction of Intrepid's interior.
It does not teleport the pawn, reshape terrain or add travel distance. A failed
presentation build falls back to the normal start path. Source:
[LunarDeparture](../../Source/LunarRescue/LunarDeparture.cpp).

## Carried-object presentation

The rescue power cell is shown after pickup while awaiting installation. Its
first-person fit uses actual retained mesh bounds, FOV and viewport aspect;
a body-space proxy supports the existing third-person view (**Camera view**,
default **V**). Inserting the cell still uses the ordinary interaction sequence.

Apollo's brief packed-sample/hardware previews require already committed cargo
flags and retain the existing stow clocks. These cosmetic proxies do not collect
cargo or add a physics inventory. Pause, notebooks and clean photo capture
suppress view objects. Retained model/material licences remain intact. Rendered 720p/1080p FOV45/85 cell views, 100/150% text and third-person cargo
were reviewed. First-person cuffs/fingers remain a prototype art limitation. Source:
[LunarCarryPresentation](../../Source/LunarRescue/LunarCarryPresentation.cpp).

## PIP-3 and Dr. Mira Vale

PIP-3 is an authored robot display near the return point. Dr. Mira Vale is a
fictional modern remote scientist represented by a communicator/person icon near
station equipment, not an Apollo crew member, NASA employee or scanned/rigged
human. Their dialogue is scripted from the current mission checklist. PIP does
not autonomously follow, navigate, repair or collect.

Approach, face a guide and use the displayed work binding (default **E**) when
the talk prompt appears. A clear view is required, and primary mission work has
priority. Talking opens the crew notebook and records a local contact; merely
reading biographies does not fabricate a conversation. Alternatively press **I**,
then **Mission Crew**. Advice grants no cargo, repairs, oxygen or hardware-health
measurement. Mira separates Cabeus/2009 Moon science from Apollo history and the
fictional ARES survey. Source: [LunarCrew](../../Source/LunarRescue/LunarCrew.cpp).

## Water Research: remote evidence, not a local discovery

During an active **Apollo 12** or **SELENE** mission, press **I** and choose
**Water Research**. It is not offered as an ARES/Mars investigation. The notebook
pauses the gameplay resource clock, which is a game rule rather than EVA physiology.

Read the result comparison, then make three decisions: identify Cabeus/LCROSS
2009, interpret a mass fraction using a hypothetical 100 kg example, and interpret
percentage-point uncertainty. Click an answer or use **1/2/3**. Wrong choices
explain the error without advancing. After three correct decisions, click
**Record remote research report** or press **Enter**; completion is not automatic.
An unfinished exercise can be restarted. **Science & Sources** expands the
citation/limits; scroll as needed. **Esc/F1** closes the notebook and preserves
its prior pause state. No new world activation key or online chatbot is required.

The bundled [publication-facts JSON](../../Content/Science/LunarWater/study.json)
contains real published estimates:

| Value | Scope |
| --- | --- |
| 155 +/- 12 kg | Maximum water vapour plus ice within the observing instrument's field of view; not a whole-crater inventory. |
| 5.6 +/- 2.9 percent by mass | Impact-site regolith estimate inferred using the observable excavated material. |
| 2.7–8.5 percent | Arithmetic estimate-minus/plus-uncertainty endpoints, not guaranteed deposit bounds or a new confidence interval. |

Source: [Colaprete et al. (2010)](https://doi.org/10.1126/science.1186986),
[official PubMed record](https://pubmed.ncbi.nlm.nih.gov/20966242/), and
[NASA LCROSS context](https://science.nasa.gov/mission/lcross/).
The +/- is retained without assigning a new confidence level. The 100 kg example
is hypothetical, not a player sample. Crater-context coordinates are approximate,
not impact coordinates or a navigation target. No raw/fabricated spectrum,
drilling result, extraction yield or water detection at Apollo 12 is generated.
Apollo's modern LROC terrain and SELENE/ARES authored terrain remain unchanged.

The report summary uses actual local attempts and committed state. Apollo's
debrief can include it; successful Apollo checkpoint saves retain valid optional
state, and report recording attempts a save. A save failure is shown explicitly.
Individual answer clicks are not guaranteed saves. SELENE keeps the result for
the current session. A fresh mission resets it; checkpoint continuation may
restore an Apollo record. It grants no historical cargo, oxygen, repair or primary
objective. Missing/invalid local data disables only this optional study without
inventing measurements. Display uses local cached data; disconnected operation
still needs its own test. Source: [ResearchPanel](../../Source/LunarRescue/LunarResearchPanel.cpp),
[WaterStudy](../../Source/LunarRescue/LunarWaterStudy.cpp),
[provenance](../../SourceArt/LunarWater/CREDITS.md),
[manifest](../../Content/Science/LunarWater/manifest.json).

## Validation status

The final executable-specific [evidence](evidence.json) records 16 passed suites.
The report/config runner also has 10 helper-fixture passes and the pixel helper
has 13; these are separate from Unreal outcomes. Reproduction commands:

```powershell
./Tools/verify_gameplay_tasks.ps1 -Packaged
./Tools/verify_gameplay_tasks.ps1 -Packaged -Rendered
./Tools/verify_gameplay_tasks.ps1 -Packaged -Review -Width 1280 -Height 720
./Tools/verify_gameplay_tasks.ps1 -Packaged -Review -Width 1920 -Height 1080
```

Review PNGs are staged visuals, not human playtests or FPS measurements. Confirm
pause/skip, carry/photo visibility, real interactions, save/restore, missing-data
behavior, bundled credits/data, and existing Moon/Mars regressions before release.
The final runtime outcomes and remaining limits are in TEST_REPORT. Existing asset licences and the unresolved
project code-licence issue remain separate.
