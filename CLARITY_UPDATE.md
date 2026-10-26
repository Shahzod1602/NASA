# Guided first expedition

## Contextual input help - 0.6.3

Work input outside usable reach, a blocked view, a pending photograph, and a
primary click without an open tool now request a six-second instruction. It
occupies the existing action card and updates from the current position, aim,
target, and rebound work key. Reach uses the same calculation as interaction:
three metres for normal work, seven metres to open Surveyor context photography.
Walking to Surveyor needs no activation button; the hint makes this explicit.

Repeated input does not extend the timer or replace radio speech/subtitles.
Pause freezes the timer; panels, the journal, tools and terminal screens suppress
the hint. H clears/hides it. Opening a tool, starting over, or restoring a
checkpoint clears it. Pending photographs no longer show a premature work
prompt. The nearby first-minute instruction asks the player to face the kit
without assuming that the player is already facing the right direction.

No mission stages, science assets, saves, movement physics or tool completion
rules change. The five-to-eight-person novice study remains pending.

Validation on 27 October 2026: the Editor passed 99 assertions, followed by
333 assertions in the rebuilt Windows package (42 guidance, 57 Apollo, 63 polish,
60 checkpoint/cross-process, and 111 Moon/Mars/menu checks). All passed. Seventeen
fresh 1280x720 DX11 frames were checked: the five new hint screens at full size
and the twelve existing layouts on a contact sheet. The new hints fit at 100%
and 150% text size. The packaged executable matches the compiled binary, and
all 53 inventoried player files retained their SHA-256 hashes. No new FPS
benchmark or human comprehension study was performed. Local verification
artifacts are under `Reports/Clarity-input-2026-10-27`.

## Original guided expedition - 0.5.2

The first equipment check now teaches looking, walking to the marker, facing
the field kit, opening the tool, and confirming it with the primary action.
Guidance follows actual input and proximity; it never awards work automatically
or prevents an experienced player from proceeding. H hides/shows the hints and
precise work markers. Pause, then H opens the revisitable fifth journal page.

The active HUD uses one short instruction. Detailed scientific purpose remains
in the field journal. Work instructions identify the current action and step.
The map's route page highlights only the current destination and Intrepid. The
nearby object under the player's aim supplies its name and prompt even when
Q has selected a different navigation destination. Rebound movement/work keys
are reflected in the guide. The photo album shortcut appears after evidence or
cargo exists. Radio subtitles remain visible; the work prompt stacks above them.

Close-up guides point to the sample or the actual first/second authored Surveyor
fitting. These are visual guides, not new precision-click requirements. Sealed
samples show their ID and actual cargo count. A pending rock/soil pouch is visible
before sealing; committed pouches/cores have a short lowering preview. Partial
work remains cancellable, does not count as cargo, and is never checkpointed.
The pouch and fittings are simplified authored equipment and animation.

Mission selection, NASA terrain/site coordinates, task IDs and the existing save
format remain compatible. Full English UI is retained. Historical exploration
remains untimed. Graphics and owner saves/photos are preserved during validation.

## Reproduce validation

Build LunarRescueEditor and LunarRescue using the existing low-memory build
arguments, then package with `Tools/package_apollo12.ps1`.
The helper retains editor debugging symbols but omits standalone compiler/linker
symbols to keep the monolithic Windows link within the development PC's memory
budget. Standalone crash stacks consequently have less diagnostic detail.
IoStore packaging uses one concurrent source read and bounded source/compression
buffers. If staging fails after a successful cook, `-StageOnly` retries packaging
from the existing build/cook; rerun the normal helper after changing source/assets.

```
Tools/verify_apollo12.ps1 -Guidance -Polish
Tools/verify_apollo12.ps1 -Packaged -Guidance -Polish -Checkpoints -Regression -Suit -Walkthrough
Tools/verify_apollo12.ps1 -Packaged -GuideReview
```

The guidance checks exercise actual opening/confirmation/cancellation, rebound
keys in instructions, target selection, pending versus committed pouch visibility,
feedback expiry/pause, fitting changes and journal access. The graphical review
produces seventeen real DX11 frames, including contextual input hints, large
text and help-page layouts.
Automation validates behavior; it does not prove that first-time players
understand it. The five-to-eight-person study in PLAYTEST_GUIDE.md remains pending.

The interaction approach follows [Xbox's objective clarity guidance](https://learn.microsoft.com/en-us/xbox/accessibility/xbox-accessibility-guidelines/109).

## Recorded validation - 0.5.2

The final Windows package passed **325 checks, zero failures**, including the
complete simulated traverse, distinct tools, guidance, suit, Moon/Mars regression,
checkpoint recovery and cross-process restore. Twelve fresh packaged 1280x720
DX11 frames were inspected, including 150% text/subtitles and help/map layouts;
no material compile fallback or fatal rendering error was reported. A preliminary
editor run passed 139 behavioral checks before the final visual adjustments.
The six original science files and two weighted source skins passed their
independent integrity checks. Owner mission/photos and original settings were
verified/restored byte for byte. No new performance benchmark or human novice
study was completed; the latter remains the next acceptance step.
