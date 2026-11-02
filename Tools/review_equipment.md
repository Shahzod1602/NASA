# Runtime equipment screenshot review

`review_equipment.ps1` launches three separate Editor **game** processes and
captures fourteen runtime equipment views: Apollo 8, SELENE 4, ARES 2. It is a
synthetic presentation review, not a playtest, mission-completion test or
performance measurement. It does not build C++, save assets/maps, regenerate
levels or access the non-reflected mission Stage property.

Run after all engine/build processes finish:

```powershell
& D:\LunarRescue\Tools\review_equipment.ps1
```

For a camera-only recheck of the two originally occluded targets and a wider
rover view, preserving all original captures:

```powershell
& D:\LunarRescue\Tools\review_equipment.ps1 -Shots @('selene-rover-wheel-axes','selene-solar-underside','ares-mesa-talus')
```

`-Shots` accepts only exact distinct names from the fourteen-view plan, filters
mission processes automatically, and records that this is a partial recapture.
The mesa view selects actual normalized 70x45x25 cm source-box dimensions
(not a fixed instance scale) and positions
the camera outward from the mesa with a trace against the retained terrain.
The solar pose uses the actual tagged junction Cube instance and its world
normal. The retained panel normally has NoCollision: only inside the transient
review game world, query collision is enabled for the visibility trace and its
exact original mode is restored in finally. No asset/map is saved.
The camera changes are transient; the original fourteen-frame receipt remains
an honest record of target occlusion and the apparent rover-ground issue.

Or first check the eight Apollo frames:

```powershell
& D:\LunarRescue\Tools\review_equipment.ps1 -Mission Apollo
```

The exact single-process form (use a fresh output directory) is:

```powershell
& 'D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' `
  'D:\LunarRescue\LunarRescue.uproject' '/Game/Lunar/Maps/Apollo12' `
  -game -EquipmentPythonReview -unattended -nosound -dx11 -RenderOffscreen `
  -ForceRes -ResX=1280 -ResY=720 `
  '-ExecCmds=py D:/LunarRescue/Tools/review_equipment.py' `
  '-EquipmentReviewOutput=D:/LunarRescue/Reports/Feedback20261102/equipment-python-apollo-new' `
  -EquipmentReviewWidth=1280 -EquipmentReviewHeight=720
```

The wrapper supplies quotes in `Start-Process` arguments and restores both the
Windows and WindowsEditor GameUserSettings.ini files to their original bytes.
Review flags select the project's isolated automation settings/save behavior.
Both entry points confine output to a fresh child directory of this project
Reports; the Python entry point also validates the explicit isolation flag and
dimension limits. Existing directories/PNGs are refused, and the wrapper checks
the exact map and ordered shot names rather than accepting duplicate evidence. It fails on missing actors/camera/APIs, imported local
bounds differing by more than 0.1 cm from retained OBJ bounds, over-budget
geometry, camera-position/aim errors, screenshot timeout or wrong PNG size.

The primary imported mesh's actual local bounds, expected source bounds and
axis errors are recorded, along with ISM component/instance totals, actual actor
bounds, requested/actual camera eye, target, rotation, FOV and each PNG SHA256.
Bounding checks establish the imported axes/extent for this review, not NASA
hardware dimensions. The images still require human visual inspection for
floating geometry, excessive occlusion, texture/material problems and contact.

The first actual Apollo attempt on 2026-11-02 reached 39 detail batches / 546
instances and passed the retained ApolloLander bounds comparison, then failed
before any screenshots because the script used a C++ UMG library name as a
Python export. Its failed receipt is preserved; it is not a successful review.
The corrected script uses the documented `unreal.WidgetLibrary` alias declared
by `UWidgetBlueprintLibrary`'s ScriptName metadata and records which current-world
viewport widgets it removed. The failed receipt is historical; the subsequent
full runs below executed successfully.

## Supported API choices

- [WidgetLibrary.get_all_widgets_of_class](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/WidgetLibrary?application_version=5.6)
  filters widgets by the actual game world and `top_level_only=True`; its
  documented `UserWidget` type is checked before use. Only those direct viewport
  widgets are removed, and the script checks that none remain. It does not scan
  or modify editor-world UMG objects.
- World discovery uses [ObjectIterator](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/ObjectIterator?application_version=5.6),
  then public [GameplayStatics](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GameplayStatics?application_version=5.6)
  getters. Only a mission world with a real controller/pawn and the completed
  equipment-builder actor tag is eligible.
- [SceneComponent](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/SceneComponent?application_version=5.6)
  exposes world transforms and world location/rotation setters. Their names are
  also confirmed by ScriptName metadata in the local Unreal 5.8 headers.
- [StaticMeshComponent.get_local_bounds](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/StaticMeshComponent?application_version=5.6)
  and [MathLibrary.transform_location](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/MathLibrary?application_version=5.6)
  provide actual imported bounds and parent-space conversion.
- Epic's [Python screenshot sample](https://github.com/EpicGames/PythonSamples/blob/main/scripts/Misc/TakeHighResScreenshot_from_all_cam.py)
  demonstrates Slate post-tick scheduling. This script uses monotonic deadlines
  and confirms complete PNG output before moving to the next pose.
- [PrimitiveComponent.line_trace_component](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/PrimitiveComponent?application_version=5.6)
  provides terrain-only camera clearance. Actual bounded ISM transforms select
  the mesa talus target; no new mesh CPU access or terrain edits are needed.
- The [documented HighResShot command](https://dev.epicgames.com/documentation/en-us/unreal-engine/taking-screenshots-in-unreal-engine)
  supports game/standalone viewports, explicit resolution and a filename. It is
  used instead of the editor-viewport-specific AutomationLibrary screenshot call.

Runtime deferred actor spawning is not exposed by the documented Python
GameplayStatics API. The script therefore uses the existing game pawn camera;
it does not create an editor-world CameraActor as a workaround. GameMode and
pawn actor ticks plus character movement are stopped; pawn rendering/collision
are hidden, UMG widgets are removed and the documented
[HUD.show_hud](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/HUD?application_version=5.6)
property is explicitly set false. Output path checks use
[Paths](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/Paths?application_version=5.6)
project-directory helpers. The existing
camera is positioned in the transient game world, independently of controller
rotation. No object is persisted. The fresh process quits after writing the
manifest, and the wrapper checks the resulting evidence.

## Executed evidence and remaining visual limits

The final full run completed on 2 November 2026 at
`Reports/Feedback20261102/equipment-python-1280x720-20261102-074613-288`:
8 Apollo + 4 SELENE + 2 Mars fresh 1280x720 PNGs and all three manifests passed
the wrapper. Every image was actually inspected with `view_image`; independent
notes and exact hashes are in `equipment-visual-qa-ground-final.json` beside the
run directory. Old failed and occluded captures/receipts remain separate.

The matching packaged game executable SHA256 is
`b034765110ad30f5a60ffcf1e874ddac679002b16c74f0e9f72bea54017a9fb4`;
these images themselves were made by **Editor -game**, not that packaged RHI.
Construction counts are Apollo 39/546, SELENE 63/709 and Mars 46/594
(components/instances). Current packaged focused checks passed 152 assertions:
asset checks 14/12/12 and equipment diagnostics 38 per map. The final 17-suite queue
plus separate typography run completed with 1,254 PASS and zero failures; these
152 are included in that total. See [TEST_REPORT](../TEST_REPORT.md) for the
separate controlled performance and preservation results. None of these stills
is a human playtest or FPS measurement.

The new source bounds/20% burial placement visibly exposes the small fragments
and selected talus target. These are authored shapes and tuning, not NASA rock
measurements. The existing MoonBase rover was repaired through Editor in one
map: one actor's pitch/yaw/Z changed, all 1,910 other actor snapshots were
preserved. No map regeneration or terrain change occurred. The final view shows
near-side wheel circles reaching the ground, with label/light readability limits.

Solar is **not visually signed off**: even the exact Cube-instance target, at
projected screen centre (640,360), is covered by the rendered blue panel face.
Its recorded no-hit trace and collision restore do not establish actual visible
attachment. A different earlier rear view shows dark fittings, but is explicitly
older-source evidence. Retained OBJ ray analysis confirms outside-source-slab
mounting, which does not certify visibility in the imported rendered mesh. Surveyor cable connectors
and all HDU support contacts also lack complete coverage in these single views.

The review's 39-row inventory covers 32 authored scene-detail rows, five reused
held-tool meshes in a storage display and two retained rows. It does not claim
39 replacement models, new first-person tool meshes or historical accuracy.
Static parsing is useful before a run; successful manifests/PNGs prove capture
execution, and actual image inspection provides separate visual evidence with
the limits above.
