# v0.5.1 - authored realism pass

This pass targets the visible prototype surfaces: plastic-looking suit cloth,
uniform hardware, distant ground speckle and the empty ground around work areas.
It remains a real-time authored approximation, not a photographic reconstruction.

- Suit geometry has less regular limb folds, finer joint seams, chest switches,
  connector rims, pack straps and visor hinge details. The same 36-bone rig is
  retained. The suit and first-person hands have rebuilt three-level LODs.
- Cloth has filtered weave, irregular folds, seam shading and bind-space dust
  masks. Dust stays on the garment when bones move. The mask is authored initial
  weathering; it does not simulate accumulating lunar dust or suit abrasion.
- Hardware has separate paint, dark equipment and crumpled foil materials.
  The field kit has handles, hinges, closures and panel trim. Kit, instruments
  and tools are visual approximations, not engineering replicas.
- Terrain uses an AI-authored regolith albedo texture with mip filtering and
  an inferred bump response. It is a visual asset, not a NASA photograph or
  measured normal map; the original PNG and provenance remain in the repository. Fine grains
  converge to their average with distance. Six instanced batches add small
  authored fragments near the gameplay route; these are cosmetic, have no
  collision and disappear beyond 42 m. The NASA 2 m terrain collision, source
  files, site coordinates and scientific task IDs are unchanged.
- Lower readability fill, contact shadows and local ambient occlusion improve
  contrast and grounding. The black lunar sky is retained. Gold visor shading
  includes an authored lunar hemisphere reflection approximation, not a live
  mirror or a captured environment. Fill is an artistic bounce-light proxy.
- Apollo hop velocity is 1.4 m/s under the existing 1.62 m/s2 lunar gravity
  (ideal ballistic apex about 0.60 m). This is a bounded gameplay hop, not a
  measured reproduction of a particular astronaut. Body height varies subtly
  through the stride. Apollo landing debris uses smaller angular fragments
  following gravity; no atmospheric smoke is added. Legacy Moon/Mars jumps
  retain their settings.

Reference: [NASA's lunar lighting explanation](https://www.nasa.gov/centers-and-facilities/ames/the-dark-side-of-the-crater-how-light-looks-different-on-the-moon-and-what-nasa-is-doing-about-it/)
and [NASA's lunar dust description](https://www.nasa.gov/reference/moonbase-environment/).
The appearance of dust-covered Apollo suits also informs the authored wear;
none of these procedural textures is a NASA measured material or photograph.

Rebuild geometry with `Tools/prepare_suit.py` and `Tools/prepare_suit_rig.py`,
then run `Tools/import_apollo_realism.py` in the Unreal Python commandlet. This
imports the suit, applies materials/lighting and regenerates LODs. Run the
realism pass after rebuilding the base map. `Tools/review_realism.ps1` captures
eight actual rendered suit frames with the HUD hidden for visual inspection.

The material pass also fixes a persistent import issue: edits to temporary
skeletal-material array values did not update the asset's imported materials.
The importer now replaces the array entries explicitly. Packaging reads back
both meshes' shared suit material paths before cooking, so the cloth and visor
changes actually reach the rendered skeletal character.

## Validation

- Editor: **301 checks passed** across Apollo, legacy Moon/Mars, menu, suit,
  settings/tools, checkpoints and a complete movement route. Packaged game:
  **304 checks passed**, including the same route and three fresh-process
  checks restoring the completed historical save and decoding real photographs.
  No failed checks were reported. The final material-binding correction is
  included in the packaged checks and render reviews.
- The complete packaged route collected all three samples and all three parts,
  deployed ALSEP and stowed cargo: **1,237.50 m / 890.28 simulation seconds**.
  This was accelerated automated movement with normal gameplay speeds, not
  a human completion-time measurement or a full-mission graphics profile.
- Eight packaged suit/work views and eight explicitly High-quality views were
  inspected at 1280 x 720. The rendered logs reported no failed-material
  fallbacks. Shared skeletal material paths were read back before cooking.
- Full suit LOD vertex counts: 112,832 / 31,474 / 16,731. Hands: 32,812 /
  9,157 / 4,826. Skin weights, outward helmet normals and authored mask values
  were independently validated. All six raw science files remained unchanged.

| New opening capture | Mean FPS | 1% low FPS | Largest frame |
|---|---:|---:|---:|
| Medium | 60.00 | 56.07 | 19.77 ms |
| High | 59.94 | 48.38 | 77.35 ms |

Both used DX11 rendered offscreen, 1280 x 720, normal delta time, no audio and
a 60 FPS cap on this Ryzen 5 6600H / RTX 3050 Laptop / 16 GB PC. Each captured
4,200 frames, excluded the first 600 and measured about 60 seconds. These are
opening-segment results, not a new full-route GPU profile. No complete VRAM
peak, separate GPU execution time, hitch-free result or other-PC performance
is claimed. Raw CSVs and environment records are local under
`Reports/Performance/v051-*-opening/`; the release manifest records the summary.

Automated runs preserved all seven pre-package player files byte-for-byte.
After testing, the local preview's graphics preset was deliberately set to
High, with the previous settings file backed up. Mission save and photographs
remain unchanged; other player preferences are retained. Fresh downloads keep
the conservative default; **F1 > Graphics > High** enables the sharper preview.

The astronaut is still a simplified authored model with procedural poses.
This pass does not make it a scanned, photorealistic A7L suit or a motion-capture
performance. Real first-time-player and broader hardware acceptance tests
remain pending. v0.5.0 full-route performance numbers describe that older build.
