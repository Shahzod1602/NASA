# UE Project Context

EngineAssociation: `5.8`; installed reviewed engine: 5.8.3.
Code-derived draft, reviewed 2026-11-13. User-specific conventions not visible in
source remain unknown. This context does not change gameplay or build settings.

## Project and build
- LunarRescue: C++ educational first/third-person Moon/Mars exploration game.
- Reviewed Git base: 1b3bd3716a2f38d138bb084ae89c10e7ad3f267c (v0.8.2).
- Primary Runtime module: LunarRescue, Default loading phase.
- Public dependencies: Core, CoreUObject, Engine, InputCore, UMG, Slate,
  SlateCore, ImageWriteQueue, MoviePlayer, Json.
- Editor-only private dependencies: MeshDescription, StaticMeshDescription,
  AssetRegistry. NoPCHs, bUseUnity=false.
- Game and Editor targets; BuildSettingsVersion.V7; Unreal5_8 include order.
- Existing verified distribution platform: Windows 64-bit Development.
- Explicit .uproject plugins enabled: PythonScriptPlugin, EditorScriptingUtilities.
  Fab explicitly disabled. Other default engine plugin states not audited here.

## Rendering
Config/DefaultEngine.ini selects DX11 / SM5, TAA (method 2), no dynamic GI or
reflection method, no static lighting or virtual shadow maps. Auto exposure and
motion blur disabled. Default cap 60; texture pool 384 MB. These are configured
defaults, not a claim that every runtime quality option has the same value.
Development reference hardware in TEST_REPORT: RTX 3050 Laptop 4 GB / 16 GB RAM.
Do not assume an asset requiring Nanite/Lumen/DX12 will work in this configuration.

## Gameplay and presentation
- ALunarGameMode: mission flow; ALunarCharacter: ACharacter-based player.
- ALunarSurfaceVehicle: authored kinematic training vehicle, swept collision.
- Game default map: /Game/Rescue/Maps/MissionSelect.
- Editor startup: /Game/Lunar/Maps/MoonBase.
- Default mode asset: /Game/Lunar/Blueprints/BP_LunarGameMode.
- Explicit cooked maps: MissionSelect, Apollo12, MoonBase, MarsBase.
- Input: legacy Engine.PlayerInput/InputComponent axis/action bindings, with
  project runtime settings/rebinding. Do not assume Enhanced Input is active.
- UI: UMG and Slate dependencies; project widget/HUD classes. CommonUI/MVVM
  activation is not established.
- Suit: authored static fallback parts and UPoseableMeshComponent body/hand rigs;
  procedural poses, not imported motion capture. A new rig needs explicit adaptation.
- Persistence: UApolloCheckpoint : USaveGame; additional state documented under
  Docs/FieldOperations20261104. Science JSON staged locally as NonUFS.
- Networking/team policy/engine modifications: not established by this scan.
- Header layout: flat Source/LunarRescue; reflected classes use LUNARRESCUE_API
  and GENERATED_BODY, with TObjectPtr references visible in LunarGame.h.

## Scientific and art boundaries
Apollo12 uses retained LROC terrain. SELENE/ARES are fictional authored settings.
The generic NASA Apollo LM reference is not a bespoke verified Intrepid recreation.
Apollo modern training rover and remote cartridges are fictional extensions.
Keep measured terrain/mission anchors intact when adding authored cosmetic meshes.
Never edit .uasset/.umap as text. Preserve dated verification and source credits.

## Local feedback additions — 14 November 2026

- `UFieldOperationsComponent` owns sequential preflight, inventory guards and cable matching. All cables await explicit `WireVerify` before the existing native cable-repair step is applied once; station restart remains separate.
- `ULunarOperationsPanel` presents EVA/Map/Inventory, retaining live progress and cached map mode. `ULunarWireBoard` paints bounded cable geometry and forwards drag release to the same guarded component authority as its click/keyboard alternative.
- Credited LasquetiSpice derivatives under `/Game/Lunar/Suit/Expedition20261113` target the existing 36-bone procedural controller; original A7L fallback remains. Source floating animation is not ground locomotion.
- Moon training vehicles load `/Game/Lunar/Vehicles/Museum20261113`; Mars keeps its existing vehicle. NOMAD-7's fictional SELENE appearance uses the NASA/JPL reference in `/Game/Lunar/Robots/Perseverance20261114`; no real lunar history is implied.
- Planet context UI assets are colour imagery and authored sphere shading, separate from Apollo terrain. Runtime downloads are unnecessary.
- Targeted, backed-up MoonBase Editor edits retire obsolete prototype foundation/feet and duplicate labels/stakes. Imported structures receive terrain-supported placement at runtime; mission target anchors remain intact.
