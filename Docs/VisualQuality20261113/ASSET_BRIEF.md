# LunarRescue production-art acquisition brief

Review date: 2026-11-13 (Asia/Tashkent).
Reviewed source: v0.8.2 / 1b3bd3716a2f38d138bb084ae89c10e7ad3f267c.
Scope: source, credits, existing validation and selected archived screenshots.
No new game run, traversal, animation playback, performance measurement or human
playtest was performed for this review. No new asset was acquired or imported.

## Findings

1. Suit geometry and poses are project-authored approximations. Finger/cuff
   shapes and the full-body proportions need production art. Code already drives
   a body and first-person poseable rig; replacing the mesh is an integration task.
2. The November 11 packaged cabin image still exposes coarse silhouette and broad
   low-contrast white faces. Normal recovery cannot create missing geometry.
3. The driveable vehicle is explicitly assembled from Engine Cube/Cylinder meshes
   in LunarSurfaceVehicle.cpp. It needs separate body, wheels and moving joints.
4. Existing magnifier/cartridge views expose segmented rings, basic boxes and
   oversized labels. Tool geometry, scale, grip and interface framing all matter.
5. Ground detail reads repeatedly over broad distances. The model catalogue says
   only diffuse images from the two Poly Haven rock packages were imported onto
   procedural meshes; obtaining the actual mesh/PBR set is a different step.
6. Renderer defaults use DX11/SM5 without dynamic GI or reflections. Material
   response, exposure/contact balance and detail scale need work alongside meshes.
   An 8K-only cinematic asset or a Lumen-only demonstration is not acceptance proof.

## Acquisition priority

| Order | Asset | Required contents | Useful English search terms |
|---|---|---|---|
| 1 | Apollo A7L astronaut and first-person gloves | Rigged body, articulated finger joints, separate/derivable first-person arms; PLSS pack, hoses, helmet/visor, boots, cloth seams and folds | Apollo A7L astronaut rigged PBR; Apollo EVA gloves rigged first person |
| 2 | Hand-operated field tool kit | Scoop, articulated tongs, hammer, core tube/caps, sample pouch and camera; separate jaws/lids/handles and correct grip scale | Apollo lunar geology tools game ready PBR; Apollo Hasselblad camera game ready |
| 3 | Apollo LM-6 Intrepid exterior | Ascent/descent stages, real silhouette, windows/hatch, foil blankets, thrusters, landing gear, pads, ladder; movable parts separated | Apollo 12 LM-6 Intrepid game ready PBR |
| 4 | Surveyor 3 and Apollo 12 ALSEP instruments | Correct reference identity, tripod/camera/sampler; removable task hardware and separate instrument meshes/cables | Surveyor 3 lander textured; Apollo 12 ALSEP instruments PBR |
| 5 | Lunar rock/regolith kit | 6-12 varied optimized rock meshes, small fragments, tileable PBR regolith, scale references and LODs | lunar basalt rock PBR game ready; lunar regolith seamless material |
| 6 | Fictional rescue rover and habitat kit | Open lunar training vehicle, enclosed Mars vehicle; separate wheels/steering/suspension/hatches, matching modular habitat/airlock/power/antenna set | lunar exploration rover game ready; Mars pressurized rover PBR; modular space habitat game ready |

First delivery should be an astronaut candidate, close-up glove candidate and
one complete tool kit. Assess source pages and files before paying for a larger
bundle. A separate Mars astronaut is not necessary for the first Moon art pass.
The rover belongs in the modern training/rescue layer; Apollo 12 did not use an LRV.

## Delivery requirements for every candidate

- Original source URL, creator, included licence/receipt and permitted game
  redistribution terms. Preview-only, editorial-only and unclear-rights items
  need resolution before acceptance. Rights review is item-specific.
- Source .blend if available; FBX preferred for rigged delivery, GLB acceptable
  for inspection/static import. Include textures separately. A .uasset-only
  package needs explicit Unreal version compatibility.
- PBR Base Color, tangent-space Normal (state DirectX/OpenGL orientation),
  Roughness, Metallic and AO or a documented packed ORM texture. No baked studio
  lighting in Base Color. 2K runtime sets initially; optional 4K source for hero
  hands/objects subject to memory measurement, not blanket 4K/8K use.
- Physical dimensions/units, useful UVs, clean silhouette and shading normals,
  manageable material slots, optimized geometry and usable LODs. Triangle counts
  must be stated as triangles and approved after inspection, not seller labels.
- For rigged assets: skeleton/bone list, reference pose, skin weights, finger
  joints and test poses. Epic-compatible rigs are helpful but do not automatically
  match this project's custom suit driver. Retargeting/pose adaptation is required.
- For tools/vehicles: jaw, wheel, lid and hinge parts with correctly placed
  pivots; grip points and sockets can then be authored. Static props do not
  require an unnecessary character skeleton.
- Screenshots: front/back/side, close-up materials, wireframe, rig/pose if present;
  material count, texture dimensions, archive size and LOD statistics.
- Require conventional raster mesh/LOD fallback. Nanite/Lumen-dependent marketing
  footage does not prove compatibility with the current DX11/SM5 renderer.

## Animation work accompanying the models

Needed motions: relaxed hands, tool grip, tool use/release, walk/stop/turn, slow
lunar hop/landing, crouch/sample collection, pouch deposit and rover seated pose.
These may be supplied as compatible clips or authored after rig integration.
A render-ready static astronaut cannot provide these motions by itself.

## Integration and acceptance

Build one consistent Apollo landing-area presentation first: suit/hands + one
sampling tool + Intrepid + nearby rocks. Match proportions, texture detail,
roughness and lighting before expanding to Mars. Keep measured terrain intact.

For each candidate: import into a separate folder; validate units/bounds/materials;
map current grip/pose controls; check 45/85 FOV and first/third-person views; test
normal walking and every affected interaction; verify collision/ladder/vehicle
entry and saved-game compatibility. Review in the actual game at 720p/1080p.
Measure walking, tool use, lander approach and occupied driving with the same
hardware/preset before and after. The historical capped four-camera result is
not a full-mission FPS guarantee. Human readability playtesting remains pending.

## Evidence inspected

- Source/LunarRescue/LunarSuit.cpp, LunarGame.h, LunarSurfaceVehicle.cpp.
- SourceArt/Suit/CREDITS.txt and Rig/CREDITS.txt.
- SourceArt/ExternalModels/CREDITS.md and ASSET_SOURCE_CATALOG.md.
- Config/DefaultEngine.ini and Docs/IntrepidCabin20261111/VERIFICATION.md.
- Nov 11 packaged cabin-side-q1.png (latest cabin pass).
- Nov 4 FieldOperations world-magnifier-cartridge.png / Mars world-rover-mounted.png.
- Oct 29 suit-hands.png and suit-front.png: historical visual references only;
  these do not certify the current build's exact hand pose or UI.

## Historical reference sources

- A7L/A7LB distinction: NASA Technical Publication 2009-214786,
  https://www.lpi.usra.edu/lunar/strategies/ChristoffersenEtAl_NASA-TP-2009-214786_LunarDustEffectsSpacesuitSystems.pdf
- Apollo 12 mission and Surveyor hardware recovery:
  https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/
- First lunar rover mission:
  https://science.nasa.gov/solar-system/moon/history-of-lunar-exploration/

These references support identity/history, not a candidate asset's licence,
accuracy, topology, rig quality or performance.
