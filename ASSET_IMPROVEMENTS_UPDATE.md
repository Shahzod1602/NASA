# Equipment and surface asset improvements

Implementation pass: 2 November 2026. The supplied 39-row inventory is a set of
improvement targets, not a verified download catalogue. This pass adds bounded,
authored runtime presentation using meshes and materials already in this
repository. No replacement model was downloaded and no licence was changed.
The initial code pass did not edit maps. Integration later repaired the existing
SELENE rover pose through Unreal Editor in **one** binary map, MoonBase: pitch
15 degrees became yaw 15 degrees, and world Z was raised to 4.554649 cm using
retained wheel/terrain contacts. This was not map regeneration. The repair
receipt verifies all **1,910 other actor snapshots** were preserved; rover XY,
scale, tags, meshes, materials and collision also stayed unchanged.

## What changed

`LunarEquipmentDetail::Build` runs once after the mission actors exist. It adds
separate Surveyor electronics, harness segments, suspension collars, camera
optics/mount details, scoop walls/teeth and cable-end hardware. The two existing
ALSEP instruments gain readable mechanical details and cable routing; new
antenna parts attach to the existing adjustable component and follow its actual
rotation. Each recovered Surveyor part remains its original separate actor, so
its new detail hides with that actor when the existing gameplay packs it.

Intrepid and the retained Apollo-derived rescue lander gain source-mounted
equipment-bay handles. The ARES cylindrical lander remains unchanged. Rescue batteries, power sockets, terminals, recorders,
rover hubs, solar hinges, habitat grab rails and shallow support pads gain
physical definition. Terrain traces place support pads and small rock clusters
against the existing collision surface. Small talus groups diversify the bases
of up to twelve existing Martian mesas; the large canyon meshes remain intact.
A rendered review caught that the first fragment implementation assumed a
100 cm source mesh and buried the short retained basalt shapes. `GroundRock`
now normalizes actual source bounding boxes to the desired dimensions, accounts
for an asymmetric source bottom centre, aligns normal/yaw and buries 20% of the
desired box height. The resulting -20%/+80% tangent-plane span is **authored
visual tuning**, not NASA geological or safety data.

All shapes, hardware arrangements, ground fragments and visual contact pads in
this pass are **authored presentation**. They are not new NASA measurements,
scans, exact hardware reconstructions, geological classifications or a claim
that a route is safe for an astronaut. Existing Apollo LROC elevations retain
their original units, coordinates, collision, source metadata and limits.

## Coverage of the 39 targets

Status **P** means new authored physical/scene detail around the retained asset.
**D** means the existing tool mesh is reused in a new storage display; its
first-person mesh/animation was not improved by this pass. **R** means retained
without a new implementation here. None of these statuses means that a new
third-party replacement model was obtained or that every attachment has passed
a complete render/historical review. The integration and partial visual evidence
below apply to the current build; unresolved target coverage remains explicit.

| # | Inventory target | Existing representation | Status and exact change in this pass | Remaining replacement/polish work |
| --- | --- | --- | --- | --- |
| 1 | Intrepid LM-6 | Credited Carbajal Apollo lander; prior foil/contact/ladder pass | P: source-mounted equipment-bay handles; original reference pose retained | Bespoke LM-6 thermal panels and verified close-up detailing |
| 2 | Surveyor 3 | Authored three-leg truss and solar/instrument layout | P: suspension collars, shelf fasteners, electronics louvers and harness runs | Historically reviewed custom truss and instrument meshes |
| 3 | Surveyor television camera | Separate removable cylinder body and work fittings | P: lens ring, barrel cap, thermal box, mount collars and connector lead | Custom camera housing; matching detailed carried payload |
| 4 | Surveyor soil sampler scoop | Separate removable authored scoop block | P: open bucket walls, teeth, hinge arm and fitting collars | Curved scoop shell and accurate linkage |
| 5 | Surveyor cable section | Separate removable cable bundle and two work fittings | P: segmented curved sheath, protective end boots and connector collars | Continuous curved cable mesh and matching carry geometry |
| 6 | ALSEP central station/antenna | Authored station and existing adjustable antenna component | P: side electronics/louvers, cable sockets, harnesses, fasteners and rotating antenna rods | Historically reviewed station shell and S-band antenna model |
| 7 | Passive seismic experiment | Separate instrument; interactive leveling tool | P: concentric rim, scale marks, leveling screw shapes and routed cable | Actual bubble pose tied to the level model; custom insulation skirt |
| 8 | Sample tongs | Existing articulated tool/jaw assets and exact grips | D: retained tongs displayed in a new field-kit rack with mounting clamps | First-person mesh/finger-detail revision remains separate work |
| 9 | Lunar soil scoop | Existing scoop mesh and staged tool motion | D: retained scoop displayed with dedicated rack clamps | Curved close-up scoop shell and material polish |
| 10 | Core tube/geology hammer | Existing separate tube, hammer and placement workflow | D: retained tube/hammer displayed on separate rack positions | Cap/hammer close-up refinement; existing commit points retained |
| 11 | Sample bags/return container | Existing open/sealed pouch, field kit and cargo finale | D: existing pouch displayed in the rack, with distinct mounting pads | Bespoke return container and matching carried bag variants |
| 12 | Lunar surface camera | Existing context-camera tool and photo framing | D: existing camera displayed on a separate rack position | Detailed body, shutter controls and close-up material pass |
| 13 | Lunar basalt/regolith | Six authored basalt shapes, reused Moon-rock appearance and prior fragments | P: small partially buried, varied rock clusters near mission areas | New measured/scanned rock sources require separate provenance review |
| 14 | A7L-inspired suit/gloves/boots | Existing authored skinned suit, hands, boots and procedural motion | R: unchanged; no new suit or hand work claimed | Custom fabric/joint/finger refinement with independent POV testing |
| 15 | SELENE rescue lander | Existing credited Apollo lander reused in fictional rescue | P: source-mounted equipment-bay handles | Distinct fictional rescue-craft shell and gear kit |
| 16 | Habitat Demonstration Unit | Existing credited HDU main/utility meshes | P: source-mounted service grab rails and terrain-tested contact pylons/pads | Verified full foundation/airlock layout; support traces skip unsafe gaps |
| 17 | Lunokhod rover | Existing credited low-poly Lunokhod mesh; one existing MoonBase actor pose repaired through Editor | P: eight hub rings, suspension links, sensor head and harness; corrected near-side wheels observed in the final capture | New wheel/tread/suspension mesh; no NASA Mars-rover claim |
| 18 | Replaceable power cell | Existing credited battery mesh | P: carry handle, keyed side connectors and bottom insertion rails | Matching detail for the existing carried/installed duplicate meshes |
| 19 | External power socket/cable | Existing staged cell/cable and restoration sequence | P: socket collars and insertion guides at the existing exact cable/installed-cell anchors | Separate socket-door/connector animation; current state flow retained |
| 20 | Space-rated control terminal | Existing credited science terminal | P: source-fascia protective grab rails and input buttons | High-resolution panel labels and rugged computer housing |
| 21 | Solar array | Existing credited solar panels | P: hinge pins, junction box and cable loop | Final exact-instance capture does not resolve fittings; retained OBJ ray audit places mounts outside the panel, with material/visibility review still open |
| 22 | Lunar communications dish | Existing dish/power-antenna actors | P: rim, central feed and three support rods on the retained rotating actor | Custom dish/backframe and physically reviewed gimbal |
| 23 | Survey mast/tower | Existing station/rover masts | P: compact sensor head at the power cabinet and new rover sensor/harness; existing large tower retained | Distinct replaceable instrument-tower mesh |
| 24 | Sealed science canister | Existing separate cylindrical body/lid objective | P: lid rim, crossbar and latch hardware; existing sample actors retained | Animated separate lid; no new sealing physiology/model |
| 25 | NOMAD-7 memory module | Existing fictional robot with side memory cylinder | P: removable-item-style protective side plate, retaining bars and connector pins | Separate carried memory shell and extraction animation |
| 26 | Crew-log recorder | Existing terminal mesh scaled as optional recorder | P: source-fascia carry rails, antenna and buttons | Distinct custom recorder body and carried version |
| 27 | Beacons/marker posts | Existing route posts and timing-beacon objective | P: reflective collars on up to sixteen narrow posts; beacon base rim/channel plates | Purpose-built beacon mesh; current navigation/timing unchanged |
| 28 | Stairs/platforms/railings | Existing station platform, steps and environment collision | P: access grab rails and ground supports around retained station assets | Full modular stair/platform kit with separate traversal acceptance |
| 29 | Mars survey rover | Existing Lunokhod mesh reused in authored ARES scenario | P: same bounded hub/suspension/sensor/harness pass as SELENE | Distinct fictional Mars-rover model; existing credit retained |
| 30 | Mars power module | Existing shared battery mesh | P: shared handle, connector and insertion-rail detail | Matching carry/installed duplicates as row 18 |
| 31 | ARES outpost/platform | Existing HDU meshes and authored equipment deck | P: shared access-rail/contact-support detailing around retained outpost | Custom outpost platform/support kit; collision kept intact |
| 32 | Mars solar/power distribution | Existing arrays, external cabinet and cable state | P: shared solar hinges/junctions plus socket collars/cabinet sensor | Detailed distribution cabinet and cable-end variants |
| 33 | Mars science terminal | Existing shared rugged terminal and powered screen | P: shared source-fascia grab rails and button hardware | Distinct rugged computer shell and panel labels |
| 34 | Gimballed relay antenna | Existing separately moving relay dish | P: rim/feed/support rods parented to the existing dish; rotation ownership unchanged | Custom gimbal/backframe and contact detail |
| 35 | Mars survey mast | Existing rover and power/relay mast arrangement | P: shared sensor head, harness and dish-support treatment | Separate custom scientific instrument heads |
| 36 | Mars data recorder | Existing optional recorder actor | P: shared source-fascia carry rails, antenna and buttons | Bespoke recorder shell and carried animation |
| 37 | Layered cliffs/mesas | Existing authored canyon terrain and faceted mesa meshes | P: varied talus groups at up to twelve existing mesa bases; large shapes unchanged | New cliff silhouettes/layer transitions require a separate authored mesh pass |
| 38 | Martian basalt/boulders | Existing six basalt shapes and authored canyon rocks | P: varied small partially buried clusters plus mesa-base stones using retained rock assets | Broader size/shape kit and separately reviewed material variants |
| 39 | Red sand/gravel/ripples | Existing authored Mars regolith/cliff materials | R: red ground/ripple shader unchanged; new grey basalt fragments do not count as a new sand material | Texture/normal/material-quality work remains future work |

The table records 32 rows with new physical/scene detail, five rows with reused
models in a new display, and two rows retained without a new implementation.
These are coverage categories, **not 39 replacement models**. Partial rows and
future custom replacement work remain explicitly listed above.

## Runtime and integration contract

- API: `LunarEquipmentDetail::Build(ALunarGameMode*)`, called by
  `BuildMissionInterface` after Apollo or rescue actors have been constructed.
- One-time maximum: **128 instanced components and 1,400 instances per world**.
  Repeated fasteners/rods share an ISM batch by actor, attachment parent, mesh
  and material. Missing assets and exhausted caps skip additions with a count.
- Added components have no collision, overlap events, navigation contribution,
  shadows or Tick. Equipment culls by 65 m; small fragments by 28 m. These are
  tuning choices, not measured performance results or exact draw-call counts.
- The runtime helper does not move original actors or modify terrain collision,
  route coordinates, work fittings, grip anchors, durations, resources or mission
  completion. The separate one-actor MoonBase rover repair above is the only
  persistent scene-pose edit in this integration.
- The existing six-batch lander pass and six-batch Apollo surface pass are
  untouched. New batches have a separate `Lunar.EquipmentDetail` tag.
- Habitat support traces use only the existing terrain component; supports are
  added for finite upward-facing contacts and positive gaps up to 180 cm.
  Unsupported placements skip the cosmetic support. Existing structures do not
  move, and new supports do not create walkable collision.
- Surveyor children hide with their original actor when packed. The close-up
  carried payload still reuses the original source mesh, so added world detail
  is not falsely claimed as a newly detailed carried model.
- Reused materials already used by the existing instanced presentation are
  applied to every slot in new batches. No runtime material compilation or new
  scientific material measurement is intended; a packaged rendering check is
  distinct from the packaged functional checks below; counts alone do not
  establish that a fitting is visible through an opaque source mesh.

## Verification and remaining work

The initial source-only pass checked retained assets and Unreal 5.8.3 APIs before
any engine execution. Its original static reports remain historical evidence.
Integration subsequently completed **Editor/Game builds and full cook/stage**.
The current packaged game executable SHA256 is
`b034765110ad30f5a60ffcf1e874ddac679002b16c74f0e9f72bea54017a9fb4`.

On that build, packaged asset checks passed **14 Apollo + 12 SELENE + 12 Mars**;
equipment diagnostics passed **38 per map**, totalling **152 passed checks with
zero failures**. The reports are under
`Reports/Feedback20261102/package-test-1280x720-20261102-074539-206`.
The new checks include independent asymmetric-box flat/known-normal projections,
zero/negative/nonfinite rejection, actual spawned Basalt bounding boxes exposed
above the measured terrain plane, 20% burial/no floating base and cosmetic flags.
Existing checks also cover caps, valid assets, idempotence, original anchors,
state/resources, Surveyor work fittings and the adjustable ALSEP antenna.

The fresh 1280x720 Editor `-game` rendering run captured all **14** planned views,
and an independent agent actually inspected every original PNG with `view_image`.
The capture manifests record these construction counts (not exact draw calls or
performance measurements):

| World | Tagged ISM components | Instances |
| --- | ---: | ---: |
| Apollo 12 | 39 | 546 |
| SELENE | 63 | 709 |
| ARES | 46 | 594 |

Evidence: `Reports/Feedback20261102/equipment-python-1280x720-20261102-074613-288`
and `equipment-visual-qa-ground-final.json` beside that directory. Each viewed
image has its own SHA256. The final views visibly show the corrected near-side
rover wheels and exposed normalized fragments/one selected mesa-base stone.
Lens/handles, ALSEP fixtures, lander side handles and visible station fittings
have no obvious large detached new mount in the inspected angles.

**Visual coverage remains partial.** The final solar exact-instance view still
renders the blue panel face at the projected junction centre; a no-hit trace
cannot replace visible attachment evidence. An earlier different rear view
showed dark fittings, but is kept as older-source evidence. Retained OBJ ray review confirms the shapes lie outside the source panel;
this does not certify visibility in the imported rendered mesh. No complete
final solar attachment pass is claimed. The cable view
also hides some connectors, and one HDU view cannot certify all support contacts.
The rover's world-space label/cyan light still reduce close-up readability.

The final packaged 17-suite queue plus separate typography run completed with
1,254 PASS and zero failures; the 152 focused checks are included in that total.
See [TEST_REPORT](TEST_REPORT.md) for commands, controlled performance and
264-file preservation evidence. These equipment stills use Editor game worlds,
not packaged RHI screenshots, a human playtest or historical certification. Existing terrain, NASA units/provenance and interaction roots remain
unchanged. Future checks should cover all removable parts before/after packing,
carried tools at supported FOV values and the remaining occluded contact angles.

A future asset replacement should use Unreal import tooling, preserve the
existing interaction roots and separate `work.0`, `work.1`, `work.focus` markers,
record exact provenance/licence terms and receive a new traversal/POV review.
A launch vehicle or Saturn V is not added to these surface missions.

## Attachment coordinates

Reference-equipment attachments use coordinates read from the retained converted
OBJ surfaces, transformed through each actual parent component. They are model
coordinates, not NASA hardware measurements. Lunokhod axles follow local Y and
wheel axes follow local X; hub centres use the eight existing wheel housings,
with small authored caps and suspension rods. Its sensor/harness endpoints are
anchored to retained source surfaces. Solar mount coordinates target the retained source plane rather than a
bounding-box face; final rendered junction visibility remains under review. HDU grab-rail mounting points
and support tops use retained surface/contact vertices; the utility unit has
one valid lowest contact, not four invented bounding corners. Terminal controls
and grab rails mount to the inset fascia. Unsupported decorative thruster pods,
HDU bands and terminal hoods are omitted. Existing model geometry, interaction
points and collision remain untouched. The separate Editor rover-pose correction
is recorded above; the runtime detail builder does not move original actors.

The source-anchor triangle-distance audit is a geometric static check. It does
not replace engine rendering, ground-contact review, historical review or a
performance measurement.
