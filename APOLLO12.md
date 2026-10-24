# Apollo 12: Surveyor's Legacy — v0.4.3

LunarRescue's default Moon destination is now a historical surface expedition at the Apollo 12 landing site. The previous MoonBase and MarsBase missions remain available in the project and package. No public deployment or release upload is performed by the local build.

## Play

Run `Play Lunar Rescue.cmd`, choose Moon, then press Enter. The new package is `Dist/Apollo12/Windows/LunarRescue.exe`. Keep its entire Windows folder together. Keyboard: WASD/arrows move, mouse looks, Space hops, E interacts or holds to work, Q cycles sample/hardware markers or turns the ALSEP antenna, F zooms, Shift holds a faster stride, V switches helmet/following view (hold Alt to orbit), Tab/J opens the field journal, Escape pauses, M returns to mission selection from briefing/pause/results. The journal pauses movement and the exercise clock; Q changes its page. Apollo and mission selection default to Uzbek; L switches to English and saves the choice. Older Moon/Mars mission text remains English. See [the onboarding update](ONBOARDING_UPDATE.md) and [the suit update](SUIT_UPDATE.md).

## Playable scenario

Committed Apollo work now creates an automatic checkpoint. Press **C** in mission selection, the briefing or the timeout screen to continue, or **Esc**, then **R** to reload the last checkpoint. A new expedition asks before replacing an existing save. This restores the last completed task rather than the exact position where the game was closed. See [checkpoint storage, recovery and validation](CHECKPOINT_UPDATE.md).

This single-player chapter condenses selected work from Apollo 12's two historical EVAs into one continuous route. It is an educational adaptation, not an exact chronological reenactment or astronaut training simulator. Design target: 20–30 minutes for a first visit; faster traversal is possible. The 30-minute exercise window is not actual Apollo suit oxygen endurance. Timeout ends the exercise with the crew returning safely, without depicting death.

1. **Intrepid checkout.** At a small authored equipment table beside the mapped lander location, check suit support, prepare tools and confirm the return route. These three interactions teach the controls and resource planning.
2. **Surface science.** Walk to the mapped ALSEP area. Deploy the passive seismometer and central station. Turn the simplified antenna toward the displayed approximate Earth bearing, then verify the link. The real ALSEP was more complex; this is a two-instrument teaching interaction, not a complete assembly procedure.
3. **Document lunar samples.** Visit the mapped collection locations for 12031 at Head, 12032 at Bench and core 12027 at Sharp. Take an in-game context screenshot before labeling and sealing each sample. Q changes navigation; any nearby unfinished sample remains interactive. The work sites and identifiers come from the dataset. Rock geometry and bagging durations are authored.
4. **Approach Surveyor.** Choose a route to Surveyor 3. Arriving within its work area unlocks the recovery task; visiting twelve individual positions is no longer required. Conrad's reconstructed traverse remains a historical map reference. The twelve former navigation positions (EVA 2 CDR point IDs 695, 700, 705, 725, 775, 795, 885, 930, 940, 950, 960 and 965) remain available to an optional historical-route walkthrough check; they are not physical objects installed on the Moon.
5. **Recover evidence.** Photograph the inactive Surveyor spacecraft before recovering three physical components: TV camera, soil scoop and cable section. Components disappear from the model when packed. This is hardware retrieval, not a rover rescue, memory download, repair or oxygen refill.
6. **Return to Intrepid.** Carry the samples and components back to the field kit and stow them. The result records field time, distance and completed science. The playable chapter ends here; ascent, rendezvous and Earth return are historical context, not simulated flight segments.

The field journal includes the orbital reference map, reconstructed EVA 2 traverse, player and objective locations, mission history, evidence explanations and sources/adaptation notes. The launch lightning incident is explained in the journal; there is no playable launch cockpit. Radio messages are newly written paraphrases displayed as text, with existing suit/interface sounds; they are not original mission recordings. Field photographs are saved under the game's `Saved/Apollo12/Photos/<run-id>` directory, one named file per sample and Surveyor visit in each expedition.

## Actual NASA data in the build

- **Terrain and collision:** [LROC NAC_DTM_APOLLO12](https://data.lroc.im-ldi.com/lroc/view_rdr/NAC_DTM_APOLLO12), product v1.9, 2 m grid spacing. A 525 × 425 vertex crop creates 444,352 triangles. Heights are copied from the source; no procedural height noise or vertical exaggeration is added. Small decorative rocks are separate authored meshes without collision.
- **Geography and navigation:** [Apollo 12 spatio-temporal mapping archive](https://data.lroc.im-ldi.com/lroc/view_rdr/SHAPEFILE_APOLLO_12) supplies the LM, Surveyor work station, ALSEP equipment, three sample locations and traverse points. Cite **Gonzales et al. (2020), LPSC LI, Abstract 1578**, as required by the archive READMEs: [paper](https://www.hou.usra.edu/meetings/lpsc2020/pdf/1578.pdf).
- **Field map image:** `NAC_DTM_APOLLO12_M120005333_2M.TIF`, a 2 m/pixel orthophoto derived from LROC images acquired on 2010-02-05. Used as a modern journal reference, not claimed to have been available to the 1969 crew. Image shadows are not used as physical surface albedo.
- **Coordinates:** Equirectangular Moon projection, standard parallel −3°, central meridian 180°, spherical radius 1,737,400 m. The mapped LM is local origin: easting 4,741,465.253661918 m, northing −91,358.83139541923 m. Unreal X is east, Y south, Z up; 100 units equal 1 metre. Vertical origin is the source terrain height at the LM. Terrain heights at work sites use the same triangle interpolation as the exported mesh.
- **Uncertainty:** 2 m is grid spacing, not absolute accuracy. The source README reports 0.8 m precision and 1.31 m RMS difference against LOLA tracks. All playable-crop height cells are populated, but some source confidence-map pixels indicate shadowed or processed areas. The browse confidence GeoTIFF uses scaled display values, which are retained in the manifest without being falsely interpreted as raw class IDs. The Surveyor station is a mapped work location; its roughly 158 m separation from the LM need not exactly match the approximately 155 m spacecraft separation quoted by NASA.

Raw files, original PDS label/README, SHA-256 hashes, coordinate conventions, crop bounds, positions and limitations are stored in `SourceArt/Apollo12/manifest.json` and `Raw/`. The package carries a copy of the manifest and this document in `Credits/Apollo12/`. Credit: **NASA/GSFC/Arizona State University, LROC science team; NASA Apollo Lunar Surface Journal**.

## Historical sources

- [NASA: Apollo 12 — The Pinpoint Mission](https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/): surface activities, ALSEP and return.
- [NASA/JPL: Surveyor 3 Camera Returned from the Moon](https://www.jpl.nasa.gov/blog/2012/1/surveyor-3-camera-returned-from-the-moon): camera, cable, tubing and scoop recovery for exposure studies. This chapter selects three of those hardware categories.
- [NASA: LRO views of Apollo 12 and Surveyor 3](https://www.nasa.gov/missions/lunar-reconnaissance-orbiter-looks-at-apollo-12-surveyor-3-landing-sites/): landing geography and visible tracks.
- [NASA: Lightning Strikes Twice](https://science.nasa.gov/resource/lightning-strikes-twice/): the two launch strikes and recovery of telemetry. The journal paraphrases the event.

The lander is an existing credited visual asset. Surveyor and ALSEP are authored simplified models, not dimensionally exact NASA CAD. Lighting, Earth orientation, tiny rocks, first-person movement, fictional interaction table, navigation markers, single-player condensation, work durations and exercise budget are explicitly game adaptations. Existing lunar gravity is 1.62 m/s². No NASA endorsement or simulation certification is implied.

## Rebuild and validation

1. `python Tools/prepare_apollo12.py` uses dependencies installed in `Tools/pydeps` (rasterio, pyshp, numpy, Pillow, requests). Downloads are cached on D:, and the raw inputs are never overwritten by transformation.
2. Unreal Python commandlet: `-run=pythonscript -script=Tools/build_apollo12.py`, using the project. This writes only the separate Apollo12 map and assets, leaving MoonBase/MarsBase intact.
3. Build `LunarRescueEditor Win64 Development -gather` after adding the new C++ files.
4. `Tools/verify_apollo12.ps1 -Regression -Walkthrough -HistoricalRoute -Suit` verifies terrain registration, mission gates, tutorial guidance, input, interaction cancellation, photo prerequisites, antenna aiming, journal pause, hardware recovery, timeout, both route choices and prior Moon/Mars/menu behavior.
5. `Tools/verify_apollo12.ps1 -Review -Language uz` renders sixteen UI/world screenshots. Repeat with `-Language en`, or `-Width 1920 -Height 1080`, for manual inspection.
6. `Tools/package_apollo12.ps1` builds the separate standalone package; repeat checks with `-Packaged` and rendering with `-Graphics`.

The movement walkthrough uses the real character input and collision at fixed 1/60-second simulation steps, accelerated by uncapped headless execution. Its elapsed value measures simulated gameplay duration, not a human playtest or benchmark FPS. Onboarding v0.4.2 results are recorded in [ONBOARDING_UPDATE.md](ONBOARDING_UPDATE.md); current v0.4.3 save, movement and rendered performance validation is in [CHECKPOINT_UPDATE.md](CHECKPOINT_UPDATE.md). Generated semantic reports remain under `Reports`.

`Tools/profile_apollo12.ps1 -Label v042` separately captures the opening route with real rendering and normal delta time at 1280x720/DX11. The default capture is 4,200 frames with the first 600 excluded from its summary. It uses the configured graphics settings and a 60 FPS cap; it is not a whole-mission benchmark or a claim about the Low preset. See `Reports/Performance` and [ONBOARDING_UPDATE.md](ONBOARDING_UPDATE.md).
