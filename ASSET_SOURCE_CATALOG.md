# Verified asset source catalogue

Research date: **2 November 2026**. This is a download/source assessment, not an
import receipt. No model, texture, map or binary asset was downloaded or changed
by this research pass. No engine, rendering or performance test was run here.

The 39 targets are tracked in [ASSET_IMPROVEMENTS_UPDATE.md](ASSET_IMPROVEMENTS_UPDATE.md).
That implementation records 32 authored detail treatments, five existing tools
in a new display and two retained targets. It does **not** represent 39 newly
obtained meshes. The candidates below do not change those completion counts.

## Rights and verification method

The [NASA 3D hub](https://science.nasa.gov/3d-resources/) links its models to the
[NASA repository](https://github.com/nasa/NASA-3D-Resources) and
[Media Usage Guidelines](https://www.nasa.gov/nasa-brand-center/images-and-media/).
NASA-origin polygon and texture data are generally reusable for educational or
informational work under those guidelines; retain the item credit, avoid implied
endorsement, and review logos and identifiable people separately. Third-party
copyright material hosted by NASA is not automatically relicensed. This catalogue
does not apply the project's code licence to any external material.

Repository metadata was checked through the public GitHub contents/recursive-tree
API at commit
[11ebb4ee043715aefbba6aeec8a61746fad67fa7](https://github.com/nasa/NASA-3D-Resources/commit/11ebb4ee043715aefbba6aeec8a61746fad67fa7).
File sizes below are either GitHub metadata, HTTP HEAD Content-Length in bytes,
or clearly marked sizes advertised by the source page. A working metadata or HEAD
response does not establish mesh topology, UV quality, rigging, scale correctness
or Unreal import success. Checksums require actually obtaining the selected file.

## Already used official meshes

| Asset | Verified primary source and format | Size | Project status and limit |
| --- | --- | --- | --- |
| Apollo Lunar Module | [NASA / Michael D. Carbajal](https://science.nasa.gov/3d-resources/apollo-lunar-module/); [GLB file](https://raw.githubusercontent.com/nasa/NASA-3D-Resources/11ebb4ee043715aefbba6aeec8a61746fad67fa7/3D%20Models/Apollo%20Lunar%20Module/Apollo%20Lunar%20Module.glb) | 716,840 B | Already imported as ApolloLander; reused for Intrepid and fictional rescue. Generic Apollo LM presentation is not a verified bespoke LM-6 reconstruction. |
| Habitat Demonstration Unit, part 1 | [NASA / LaRC / Advanced Concepts Lab](https://science.nasa.gov/3d-resources/habitat-demonstration-unit/); [GLB file](https://raw.githubusercontent.com/nasa/NASA-3D-Resources/11ebb4ee043715aefbba6aeec8a61746fad67fa7/3D%20Models/Habitat%20Demonstration%20Unit/Habitat%20Demonstration%20Unit%20(part%201).glb) | 516,748 B | Already imported as HabitatUtility. |
| Habitat Demonstration Unit, part 2 | Same [official item](https://science.nasa.gov/3d-resources/habitat-demonstration-unit/); [GLB file](https://raw.githubusercontent.com/nasa/NASA-3D-Resources/11ebb4ee043715aefbba6aeec8a61746fad67fa7/3D%20Models/Habitat%20Demonstration%20Unit/Habitat%20Demonstration%20Unit%20(part%202).glb) | 710,988 B | Already imported as HabitatMain in Moon/Mars rescue environments. An imported exterior is not a completed walkable habitat interior. |

Existing import provenance remains in
[SourceArt/ExternalModels/CREDITS.md](SourceArt/ExternalModels/CREDITS.md) and
[Tools/lunar_convert_models.py](Tools/lunar_convert_models.py). Re-downloading
these three files would not create three new improvements.

## Downloadable Apollo tool candidates

NASA's [Spacewalking Tools](https://www.nasa.gov/suits-and-rovers/spacewalking-tools/)
page provides six **Apollo 17 display replicas**, with individual metric STL files
and a [complete ZIP](https://www.nasa.gov/wp-content/uploads/2024/01/3d-files-of-apollo-geology-tools.zip)
(30,614,774 B; page labels it 29.20 MB). Apply NASA media rules and preserve NASA
credit; no separate CC-BY/CC0 licence was established on this page.

| Download | Verified individual size | Fit for this project |
| --- | --- | --- |
| [Drive tube STL](https://www.nasa.gov/wp-content/uploads/2024/01/apollo-drivetube-oml-metric.stl) | 2,330,684 B | Core-tube shape reference; compare Apollo 12 dimensions/caps. |
| [Extension handle STL](https://www.nasa.gov/wp-content/uploads/2024/01/apollo-ext-handle-oml-metric.stl) | 6,132,584 B | Shared tool-family reference. |
| [Hammer STL](https://www.nasa.gov/wp-content/uploads/2024/01/apollo-hammer-oml-metric.stl) | 5,787,584 B | Geology-hammer candidate after historical comparison. |
| [Rake STL](https://www.nasa.gov/wp-content/uploads/2024/01/apollo-rake-oml-metric.stl) | 27,008,084 B | Later-mission reference; do not silently add to Apollo 12 kit. |
| [Scoop STL](https://www.nasa.gov/wp-content/uploads/2024/01/apollo-scoop-oml-metric.stl) | 21,831,284 B | Apollo 17 hand tool; not Surveyor's motorised sampler scoop. |
| [Tongs STL](https://www.nasa.gov/wp-content/uploads/2024/01/apollo-tongs-oml-metric.stl) | 24,013,984 B | Static shape candidate; articulated jaws/grips still need authoring. |

STL does not supply a textured, rigged game asset. Inspect the package instructions
for units, then budget retopology, UVs, materials, separated moving parts and LODs.
The [Apollo collection-tool technical reference](https://www.nasa.gov/wp-content/uploads/2019/04/02_allton_corrected_apollo.pdf)
can support the historical comparison. Do not present an Apollo 17 replica as an
exact Apollo 12 instrument without that work. The official
[Pistol Grip Tool GLB](https://github.com/nasa/NASA-3D-Resources/tree/master/3D%20Models/Pistol%20Grip%20Tool)
is 236,300 B, but is a modern tool, not a substitute for Apollo manual tools.

## Downloadable measured rock candidates

NASA [Astromaterials 3D](https://ares.jsc.nasa.gov/astromaterials3d/index.htm)
provides textured OBJ ZIPs. Its [FAQ](https://ares.jsc.nasa.gov/astromaterials3d/faqs.htm)
identifies the data as public domain and supplies distinct lunar/meteorite credits.
Retain the applicable full credit from that source when importing. Use the web
resolution variant first; full-resolution meshes/XCT volumes are unnecessary for
this demo. Published mesh sizes differ from compressed ZIP sizes.

| Specific source | Verified web ZIP | Source metadata / intended interpretation |
| --- | --- | --- |
| [Apollo 12 sample 12019,0](https://ares.jsc.nasa.gov/astromaterials3d/sample-details.htm?sample=12019-0) | [OBJ ZIP](https://ares-a3d.s3.us-gov-east-1.amazonaws.com/samples/12019-0/2b_12019-0_SFM_Web-Resolution-Model_Coordinate-Registered/12019-0_SFM_Web-Resolution-Model_Coordinate-Registered.zip), 9,262,968 B | Low-Ti pigeonite basalt; web mesh 122,102 faces, advertised mesh 14 MB. A real Apollo 12 specimen candidate. |
| [Apollo 12 sample 12038,7](https://ares.jsc.nasa.gov/astromaterials3d/sample-details.htm?sample=12038-7) | [OBJ ZIP](https://ares-a3d.s3.us-gov-east-1.amazonaws.com/samples/12038-7/2b_12038-7_SFM_Web-Resolution-Model_Coordinate-Registered/12038-7_Web-Resolution-Model_Coordinate-Registered.zip), 8,956,287 B | Feldspathic basalt; advertised web mesh 12.3 MB. Curated subsample shape, not a scan of undisturbed field placement. |
| [ALH 84001,55](https://ares.jsc.nasa.gov/astromaterials3d/sample-details.htm?sample=ALH84001-55) | [OBJ ZIP](https://ares-a3d.s3.us-gov-east-1.amazonaws.com/samples/ALH84001-55/2b_ALH84001-55_SFM_Web-Resolution-Model_Coordinate-Registered/ALH84001-55_SFM_Web-Resolution-Model_Coordinate-Registered.zip), 3,574,041 B | Mars-origin Antarctic meteorite, SNC orthopyroxenite; 89,760 web faces, advertised mesh 6.87 MB. **Not basalt.** |
| [EET 79001,312](https://ares.jsc.nasa.gov/astromaterials3d/sample-details.htm?sample=EET79001-312) | [OBJ ZIP](https://ares-a3d.s3.us-gov-east-1.amazonaws.com/samples/EET79001-312/2b_EET79001-312_SFM_Web-Resolution-Model_Coordinate-Registered/EET79001-312_SFM_Web-Resolution-Model_Coordinate-Registered.zip), 8,306,514 B | Mars-origin Antarctic meteorite, shergottite; 99,440 web faces, advertised mesh 11.8 MB. |

Sizes/classification came from the official public catalogue JSON and per-sample
metadata used by [sample-details.js](https://ares.jsc.nasa.gov/astromaterials3d/sample-details.js),
with ZIP HEAD responses checked. The collection's laboratory reconstructions
must retain specimen identity. They do not establish the geology, dimensions or
placement of arbitrary rocks in the fictional ARES region. Enlarged/scattered
copies must be labelled authored scenery rather than measured in-situ boulders.
No imported or scattered copy is claimed by this research.

## Verified references with unresolved replacement requirements

**Surveyor 3, camera, scoop and cable:** no matching downloadable mesh was found
in the checked official NASA repository or NASA 3D hub. The recursive tree's
Surveyor results are **Mars Global Surveyor**, an unrelated orbiter. This bounded
search does not prove no model exists anywhere. The
[Surveyor 3 mission page](https://science.nasa.gov/mission/surveyor-3/) and
[NASA SP-284](https://ntrs.nasa.gov/citations/19720019081) are primary geometry/photo
references; NTRS marks the report as US-government work with public use permitted.
They are not GLB downloads. A reviewed custom model preserving separate removable
parts and work anchors is the current concrete path. No small official Surveyor 3
GLB was verified.

**ALSEP / passive seismometer:** the checked NASA model tree contains no ALSEP or
seismometer replacement. The Smithsonian's
[passive seismic array record](https://airandspace.si.edu/collection-objects/alsep-passive-seismic-experiment-array/nasm_A19820511000)
was discoverable as an object/image reference, but direct retrieval returned 403;
a downloadable mesh and reusable image licence were not verified. The NASA
[Apollo experiment operations report](https://www.nasa.gov/wp-content/uploads/static/history/alsj/RP-1994-1317.pdf)
is a technical reference, not a mesh. Keep custom historical reconstruction as an
open task; do not relabel a later-mission museum instrument as Apollo 12 hardware.

**A7L suit:** the official
[Smithsonian Armstrong suit record](https://3d.si.edu/object/3d/d8c63ba6-4ebc-11ea-b77f-2e728ce88125)
lists OBJ, FBX, STL, low-resolution GLB and USDZ, with several mm-scale options.
Its asset-specific conditions restrict downloads to non-commercial, educational
and personal use; it is **not an unrestricted CC0 game asset**. Indexed official
metadata was readable, while direct requests returned 500/403; binary sizes and
package contents were not verified. A public distributable game/repository import
is not cleared here. Use as a reference only until permission/scope is resolved,
or author an independently reviewed suit. NASA Gemini/Mark III/Z2/ACES models are
not historical A7L replacements. A static scan also needs retopology, rigging,
first-person hands and animation integration.

## Mars rover and measured terrain alternatives

A [NASA/JPL-Caltech Perseverance model](https://science.nasa.gov/resource/mars-perseverance-rover-3d-model/)
has a [GLB download](https://assets.science.nasa.gov/content/dam/science/psd/mars/resources/gltf_files/25042_Perseverance.glb)
advertised as 11.15 MB and USDZ as 12.70 MB. It is a possible **separately named
Perseverance exhibit/mission**, not Lunokhod, a repair-ready ARES rover or a lunar
vehicle. Import, articulated suspension and interaction compatibility are untested.
Apply the item credit and NASA media rules; do not imply mission partnership.

For actual Mars surface geometry, the official
[HiRISE DTM overview](https://www.uahirise.org/dtm/about.php) explains the raster
pipeline: PDS IMG elevation plus JP2/LBL orthoimagery. It is not a ready-made
Unreal cliff kit. Two exact candidates were verified without downloading them:

| Product / primary page | Resolution and advertised downloads | Use boundary |
| --- | --- | --- |
| [Mesa in southwest Nili Fossae](https://www.uahirise.org/dtm/ESP_019898_2000), DTEEC_019898_2000_019753_2000_A01 | 1.00 m/pixel; DTM 523 MB; lower-resolution IRB orthos 17/19 MB and RED orthos 38/39 MB; centre 19.73 N, 72.46 E | Potential measured mesa terrain after crop/projection/NoData/slope checks. |
| [Western edge of a layered mound in Juventae Chasma](https://www.uahirise.org/dtm/ESP_020470_1755), DTEEC_020470_1755_014378_1755_A01 | 1.01 m/pixel; DTM 233 MB; lower-resolution RED orthos 25/28 MB; centre -4.47 N, 297.6 E | Potential layered-landform terrain, not interchangeable with the fictional ARES location. |

The [HiRISE image policy](https://www.uahirise.org/media/usage.php) permits image
reuse and requests NASA/JPL/University of Arizona credit. Preserve each product's
producer/credit and inspect its embedded label and PDS documentation before
conversion. NoData, vertical reference, local coordinates, Unreal centimetres,
interpolation and runtime budgets still need a separate dataset manifest/test
pass. Metre-scale topography cannot establish millimetre sand ripples. New Mars
height data has not been integrated, and no terrain download was performed here.

## Coverage and exact outstanding gaps for all 39 targets

“Candidate” means only the verified source above is available. It does not mean
obtained, imported or accepted. Existing credited third-party assets remain under
their own recorded terms.

| Inventory rows | Source coverage | Remaining work |
| --- | --- | --- |
| 1 Intrepid | Existing NASA LM | Exact LM-6 panels/detail and acceptance review. |
| 2-5 Surveyor / camera / scoop / cable | Official photo/report references only | Custom historical meshes; no verified official replacement download. |
| 6-7 ALSEP / PSE | Technical references only | Reviewed station, antenna, skirt and instrument meshes. |
| 8 Tongs; 9 scoop; 10 core/hammer | Apollo 17 STL candidates | Apollo 12 comparison, retopology, materials and moving/grip parts. |
| 11 Bags/container; 12 surface camera | No replacement source verified in this pass | Custom matching carried/open/closed models. |
| 13 Lunar rocks/regolith | Apollo 12 OBJ specimen candidates | Import, unit/LOD/material checks; regolith still authored. |
| 14 Suit/gloves/boots | Restricted Smithsonian scan reference | Redistribution scope unresolved; custom rigged suit remains. |
| 15 Rescue lander | Existing NASA LM reuse | Distinct authored fictional rescue-craft shell. |
| 16 HDU | Existing two official GLBs | Foundation/airlock/interior work; existing mesh is not new. |
| 17 Lunokhod | Retain existing separately credited model | No new official Lunokhod source verified; tread/suspension polish. |
| 18 Cell; 19 socket/cable; 20 terminal | Existing credited assets/authored detail | Matching carried/installed forms, doors, connectors, labels. |
| 21 Solar; 22 dish; 23 mast | Existing assets/authored detail | Specific frames, gimbals and sensor meshes; no new source verified. |
| 24 Canister; 25 memory; 26 recorder | Fictional authored equipment | Separate shells/lids/carry variants; no claimed NASA replica. |
| 27 Beacons; 28 stairs/platforms | Existing authored environment | Custom modular kit plus independent traversal checks. |
| 29 Mars rover | Official Perseverance candidate, different vehicle | Decide exhibit/mission identity; retain current rover until validated. |
| 30 Mars cell; 31 ARES/HDU; 32 power | Existing shared assets/HDU | Bespoke fictional kit and support/power variants. |
| 33 Terminal; 34 antenna; 35 mast; 36 recorder | Existing shared authored equipment | Custom shell/gimbal/sensor/carry refinements. |
| 37 Cliffs/mesas | Exact HiRISE raster candidates | New terrain pipeline/location decision; current canyon remains authored. |
| 38 Mars rocks | Mars-origin meteorite OBJ candidates | Specimen-faithful exhibit or clearly authored scenery, not in-situ scan claims. |
| 39 Sand/gravel/ripples | No new physical-material source verified | Authored shader/texture work; do not label microdetail measured Mars data. |

## Next import decisions

1. Start with one **Apollo 12 basalt web OBJ**, record URL/date/checksum/credit and
   original dimensions, make a bounded LOD, then compare close-up readability and
   packaged material support. Preserve terrain and sample objective identities.
2. Compare the individual Apollo tool STLs with Apollo 12 references before
   rebuilding the existing tools; preserve grips, interaction markers and staged
   gameplay commit points. Do not replace all six at once without validation.
3. Commission/author the missing Surveyor/ALSEP pieces from primary references;
   keep the A7L restriction and Mars terrain integration explicitly unresolved.

No account, purchase, permission request or external message was initiated. This
research is independent of competition eligibility and supplies no new NASA
approval or simulator-accuracy claim.
