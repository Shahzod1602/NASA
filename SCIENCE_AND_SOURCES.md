# Science and sources

## Intrepid cabin shading — 11 November 2026

The recovered cabin normals are artist-authored vectors from the retained
NASA / Michael D. Carbajal GLB. They are not elevation measurements, surveyed
surface details or a new engineering reconstruction. Source topology/positions,
UVs, material assignments, LROC terrain and mission coordinates are preserved.
Computed source frames are baked into a separate asset; invalid tangent bases
are reconstructed and their scope/format limits are recorded explicitly.

The new cloth seams and roughness are authored art on two shared white slots,
including their existing trim. The original GLB's neutral color factors are
retained, but the shader is not a calibrated lunar thermal-blanket material.
[Source and interpretation](Docs/IntrepidCabin20261111/SCIENCE_AND_ART.md).

## Intrepid presentation layer — 11 November 2026

The new 50 × 50 m shader mask, grain normals, rock placement, blanket folds,
metallic/roughness coefficients and contact shading are authored presentation.
They do not add measured lunar elevation, resolve sub-2-m terrain, establish a
historical rock distribution or simulate dust transport. The retained regolith
bitmap was AI-authored; it is not a NASA surface photograph or measured normal map.
Existing LROC heights/collision, instrument coordinates and original source files
remain unchanged. The retained NASA/Carbajal lander is a simplified reference,
not a new engineering reconstruction.

The black sky and existing sun/fill settings are retained. NASA's
[lunar lighting explanation](https://www.nasa.gov/centers-and-facilities/ames/the-dark-side-of-the-crater-how-light-looks-different-on-the-moon-and-what-nasa-is-doing-about-it/)
supports the airless lighting context; it does not validate the game's material
coefficients or artistic readability fills. [Science/art scope](Docs/IntrepidVisual20261111/SCIENCE_AND_ART.md)
records the distinction. No new NASA dataset, endorsement or competition status
is claimed by this local update.


## Modern field-training extension — 4 November 2026

Existing LROC Apollo 12 elevations continue to control terrain and work-site
heights; SELENE/ARES terrain is authored. No new DEM, M3 map or local water
measurement/extraction dataset was integrated in this iteration.

Content/Science/LunarWater/study.json remains a published-facts summary of remote
LCROSS evidence at Cabeus in 2009. Three authored delivery cartridges reference
that summary after optical inspection. Their 0.5 kg package mass, storage sizes,
inspection time and resource costs are game rules, never a measured water mass.
The supplied search-result screenshot was treated as reference, not a raster or
redistribution licence.

Apollo 12 had no rover. The lunar rover is a modern fictional overlay; the Mars
vehicle deployment is fictional and inspired by SEV prototype research. Its
4.5 m/s limit, 18 degree traversal threshold and swept hull are tuning, not vehicle
or astronaut safety certification. Four pre-EVA checks are authored actions, not
measured suit pressure or operational NASA procedures. No NASA approval is claimed.
[Research, official links and limits](Docs/FieldOperations20261104/SCIENCE_RESEARCH.md)
identify which facts each source supports. Runtime remains offline.

## Current implementation - 2 November 2026

**Apollo 12 uses a real, stereo-derived LROC terrain product. SELENE and ARES use
authored terrain.** The old LOLA south-pole investigation below is historical
candidate research and does not describe the current Apollo chapter.
LunarRescue is a simplified educational game, not NASA-endorsed astronaut
training or a certified simulator.

| Content | Actual role and source |
| --- | --- |
| Apollo 12 terrain | `NAC_DTM_APOLLO12`, v1.9, dataset `LRO-L-LROC-5-RDR-V1.0`. Source 2 m post spacing. Imported heights control ground collision and the heights of mapped worksites. [Manifest](SourceArt/Apollo12/manifest.json), [product label](SourceArt/Apollo12/Raw/NAC_DTM_APOLLO12.LBL). |
| Apollo map and traverse | The modern LROC orbital image supports the journal map. Mapped Apollo work locations and reconstructed traverse segments are imported separately. The player's gold walking trace is local gameplay telemetry, not a historical astronaut track. |
| SELENE / ARES | Procedurally authored rescue maps and fictional station/science-record scenarios. Their downloads do not constitute a measured NASA dataset or a new discovery. |
| Objects and presentation | Equipment placement details, small fragments, suit/tool poses, boot prints, lighting, cargo displays and UI animation are authored. They do not increase measured elevation resolution. |
| Resources | Both rescue missions currently initialize a 480-second gameplay oxygen budget. Apollo 12 defaults to untimed exploration and offers a 1,800-second optional exercise. A countdown is game tuning, not real suit endurance. |
| Optional Field lessons | Three mission-specific questions distinguish terrain provenance, historical context/fictional objectives and gameplay budgets. The debrief reports latest session responses only. No learning gain or astronaut competence is inferred. |
| Optional equipment diagnostics | The I notebook reads existing repair/reboot flags and Apollo photo/instrument/cargo records. It explains prerequisites and the next normal action. It does not measure voltage, electrical power, hardware damage or safety, and grants no mission progress. |

The official [Apollo 12 archive product](https://pds.mcp.nasa.gov/data/store/img/lunar_reconnaissance_orbiter/pds4/lroc/lro-l-lroc-5-rdr/LROLRC_2001/DATA/SDP/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_README.TXT)
and its local README describe the LROC source and errors. Credit: NASA/GSFC/ASU,
LROC science team. Source images were acquired in 2010; these are modern orbital
products, not photographs or terrain measurements made during the 1969 EVA.
Two metres is grid spacing, not a guarantee of absolute positional accuracy.

The selected crop is 525 x 425 vertices. `Tools/prepare_apollo12.py` reads the
GeoTIFF as a masked raster and rejects missing elevations in that crop, then
exports the same triangulation used to place worksites. The label specifies
32-bit floating-point GeoTIFF and NoData `-3.40282266e+038`. The projection is
lunar equirectangular, with a 1,737,400 m reference sphere, standard parallel
-3 degrees and central meridian 180 degrees. The manifest records the crop,
coordinate transform, local origin, source URLs, byte sizes and SHA-256 hashes.
Per-file acquisition timestamps are not currently recorded in that manifest.

Runtime axes are X east, Y south, Z up; **100 Unreal units = 1 metre**, with no
horizontal or vertical exaggeration. Z is relative to the manifest's local
origin elevation. The source grid and crop were not regenerated in this UI pass.
Data and derived assets are local; reading a lesson or source note does not
require a runtime web request. A network-disconnected run is a separate check.

The photo-context lesson cites [NASA's Apollo 12 mission account](https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/),
which describes photographing Surveyor before hardware retrieval. Extending a
photo-first rule to the game's selected sample tasks is an authored adaptation.
The countdown lesson cites project gameplay rules rather than presenting NASA
suit specifications. Field-lesson answers are optional, session-only and do not
alter cargo, time budgets or scientific measurements.

## Remote water study and fictional guides — 3 November 2026

Source integration adds an optional modern **LCROSS/Cabeus 2009** exercise in the
Moon notebook. It does not detect water at Apollo 12, change its Oceanus Procellarum
terrain, or turn SELENE/ARES scenery into surveyed polar sites. The local Windows build and rendered checks completed; see the executable-specific
[evidence and limitations](TEST_REPORT.md).

The bundled `Content/Science/LunarWater/study.json` is a derived publication-facts
summary with a checksum manifest, not a raw spectrum.
[Colaprete et al. (2010)](https://doi.org/10.1126/science.1186986),
[PubMed record](https://pubmed.ncbi.nlm.nih.gov/20966242/), reports a maximum
155 +/- 12 kg of water vapour plus ice in the observing field of view and a
5.6 +/- 2.9 percent-by-mass impact-site regolith estimate using observable
excavated material. These are not whole-crater inventories or player samples.
The hypothetical 100 kg comparison and 2.7–8.5 percent arithmetic endpoints are
explanations, not newly measured samples, guaranteed deposits or a newly assigned
confidence interval. The reported +/- receives no invented confidence level.

[NASA LCROSS](https://science.nasa.gov/mission/lcross/) supports the mission context;
[NASA quick facts, Appendix C](https://ntrs.nasa.gov/api/citations/20130014470/downloads/20130014470.pdf)
gives approximate crater context, explicitly not an impact navigation coordinate.
The [PDS archive](https://pds-geosciences.wustl.edu/missions/lcross/default.htm)
is a reference; no raw spectral packets are bundled or analysed by this exercise.
Three optional decisions and explicit report commit record local attempts without
granting cargo, repairs, oxygen or primary progress. Successful Apollo checkpoints
can retain valid state; SELENE's result is session-only. Invalid/missing data
only disables the study. No runtime network request is required for its display.

PIP-3 and Dr. Mira Vale are fictional authored guides. PIP reads recorded mission
prerequisites; Mira is a modern communicator icon, not an Apollo crew member or
NASA employee. The transfer-cabin/hatch scene is authored camera presentation,
not a reconstructed Intrepid interior. Carry proxies reuse retained assets and
add no measurement or inventory physics. No NASA partnership or real hardware
validation is implied. See [feature guide](Docs/Gameplay20261103/FEATURE_GUIDE.md)
and [water provenance](SourceArt/LunarWater/CREDITS.md). Existing source/licence
boundaries and earlier validation records remain separate.


## Local feedback and equipment pass - 2 November 2026

Borderless HUD text and compact loading typography present existing mission
state. The loading percentage represents preparation milestones, not file-byte
throughput. Loading artwork remains illustrative and not to scale. Neither the
UI nor the optional diagnostic checklist adds a scientific measurement.

The equipment inventory contains 39 research/coverage rows: 32 authored scene
detail treatments, five existing tools reused in a storage display and two
unchanged targets. These are not 39 imported meshes. Added rails, collars,
connectors, supports, rock clusters and visual contact pads are authored
arrangements using existing assets. Terrain traces position cosmetic details;
they do not modify the NASA/LROC height grid, collision, coordinate units or
measured resolution. The suit/first-person tool meshes are not claimed as newly
replaced by this pass. See [the precise asset scope](ASSET_IMPROVEMENTS_UPDATE.md)
and [candidate provenance research](ASSET_SOURCE_CATALOG.md).

The diagnostic display distinguishes recorded gameplay steps from a physical
equipment diagnosis. In rescue missions the station is not shown as rebooted
merely because its cell/cable steps are complete. Apollo evidence is counted
only from committed photo and cargo flags. If the ended rescue mission does not
retain a separate download or relay flag, that status is unavailable rather
than guessed. Space Engineers informed the dependency/help design; it is a game
reference, not scientific evidence or a copied engineering model. The primary
references and design inferences are in [the gameplay update](GAMEPLAY_REFERENCE_UPDATE.md).

The later SELENE rover correction changes one existing MoonBase actor's
rotation and world Z, not the retained mesh or terrain. Forty-eight contact
sample entries across eight wheels were traced against SELENE's authored
terrain; their clearance is a cosmetic placement check, not NASA-derived
engineering validation. Authored basalt/talus scales from actual source bounds
and follows the local terrain normal, with 20% of its requested height buried.
That tuning does not add measured geological detail. No NASA DEM, source crop,
coordinate transform, collision mesh or measured-resolution claim was changed.

Final local executable `b0347651...a9fb4` passed 1,254 exact PASS executions after
full cooking, including 18 new grounding assertions. The
[per-executable receipt](Docs/Feedback2026-11-02/evidence.json) separates 110
staged PNGs and six routed pointer outcomes from assertions and prior builds.
Fourteen fresh Editor Python equipment views were reviewed; the latest solar
junction view remains inconclusive. Offline OBJ geometry proof does not certify
its rendered visibility. Matched controlled performance runs and the 264/264
unchanged protected-file comparison completed; see [TEST_REPORT](TEST_REPORT.md)
for conditions and limits, rather than a sustained whole-mission FPS claim.

No new terrain dataset, NASA partnership, hardware-health measurement, verified
astronaut safety rule or human learning result is introduced. Runtime remains
local; source links are citations, not a required online service. Build/runtime
assertions do not substitute for a separately performed network-disconnected
run or a real participant study.

## Historical candidate research - 21 September 2026

The retained sections below describe the earlier **LOLA candidate**, its planned
pipeline and the prototype at that date. Statements such as "not integrated"
refer to that candidate and historical plan, not the current LROC Apollo 12
terrain. The A/B route planner and slope-driven resource forecast remain future
work; importing the Apollo terrain does not by itself implement those systems.

## Historical LOLA candidate: south-pole grid

[NASA LOLA](https://science.nasa.gov/mission/lro/lola/) asbob haqida; [PDS Geosciences](https://pds-geosciences.wustl.edu/missions/lro/lola.htm) arxiv va mahsulotlarga kirish beradi. Quyidagi qiymatlar bevosita [PDS mahsulot labeli](https://pds-geosciences.wustl.edu/lro/lro-l-lola-3-rdr-v1/lrolol_1xxx/data/lola_gdr/polar/img/ldem_875s_20m.lbl)dan tekshirildi:

| Maydon | Qiymat |
| --- | --- |
| Dataset / product | `LRO-L-LOLA-4-GDR-V1.0`; `LDEM_875S_20M.IMG`, version V2.0 |
| Producer | LRO LOLA Team, Goddard Space Flight Center; David E. Smith |
| Source observations | 2009-07-13 – 2017-02-02; product created 2017-06-15 |
| Qamrov | Planetocentric latitude −90…−87.5°; label longitude bounds N/A. Aniq missiya ROI hali tanlanmagan. |
| Grid | 7584 × 7584; 20 m/pixel, qutbda haqiqiy; bu o‘lchov aniqligi kafolati emas |
| Format/hajm | Raw IMG, signed little-endian int16; 7584 × 7584 × 2 = **115,034,112 bytes**, taxminan 109.7 MiB |
| Balandlik | `elevation_m = DN * 0.5`; reference sphere radius 1,737,400 m |
| Radius | `planetary_radius_m = elevation_m + 1737400`; Unreal Z sifatida radius ishlatilmaydi |
| Proyeksiya | Spherical polar stereographic, center −90°, longitude 0°, east-positive |
| Koordinata reference | Body-fixed rotating, Mean Earth/Polar Axis of DE421; uchala radius 1737.4 km |
| Registration | Pixel registration; line/sample projection offset 3791.5; indekslar 1…7584 |
| NoData | Labelda explicit sentinel ko‘rsatilmagan. **Tasdiqlanmagan**; barcha int16 qiymatlarni valid deb qabul qilish mumkin emas. |
| Olish holati | Faqat LBL 2026-09-21 olindi. IMG olinmagan; raster checksum va qayta ishlangan ROI mavjud emas. |

Bu allaqachon gridded/interpolated mahsulot: label GMT blockmedian/surface ishlovini qayd etadi. Har bir 20 m piksel mustaqil lazer o‘lchovi emas. Unrealda 0.5 m mesh yaratish yangi o‘lchangan detal bermaydi. Slope aniqligi ham grid va ishlovga bog‘liq. `LDEC` count mahsulotini `LDEM` elevation bilan almashtirmaslik kerak.

Foydalanish uchun [NASA Science data license](https://science.data.nasa.gov/about/license) umumiy siyosati va aynan mahsulotdagi qo‘shimcha shartlar tekshiriladi. Labelda maxsus cheklov ko‘rinmadi; catalog/redistribution tekshiruvi yakunlanmagan. NASA nomi va ochiq data mahsulotga NASA tasdig‘ini bermaydi. Credit: NASA/GSFC, LRO LOLA Science Team; distribution: NASA PDS Geosciences Node. Joriy SourceArt asset litsenziyalari bundan alohida.

## Ruxsatli bosqich uchun pipeline spetsifikatsiyasi — hali bajarilmagan

1. Rasmiy IMG, label va projection catalogni olish; original URL, UTC download date, byte count va SHA-256 saqlash. NoData talqini ochilmaguncha importni bloklash.
2. Endian/type/dimensions/range tekshiruvi; explicit validity mask. Nol balandlikni NoData deb taxmin qilmaslik. Yetishmagan faylda aniq xato; NASA deb belgilangan sun’iy fallback bo‘lmasin.
3. Projection catalog bilan pixel-center/half-pixel konvensiyasini tekshirish; kamida uchta nazorat nuqtasi va lon/lat round-trip. Qutb meridian singularity va Y yo‘nalishini sinash; Earth EPSG:4326 ni qo‘llamaslik.
4. Buzilmagan real relyefda yurishga mos kichik ROI va ikki yo‘lni topish. 20 m grid joriy taxminan yuz metrlik sahnada kam namuna beradi: hudud kengayishi yoki boshqa rasmiy mahsulot kerak bo‘lishi mumkin. Scale yoki balandlikni yashirin o‘zgartirmaslik.
5. Lokal metrik origin, east/north/up o‘qlari, Z origin va georeference manifestda; Unreal 1 m = 100 cm. Mesh uchun `Z_cm = (elevation_m - origin_elevation_m) * 100`. Landscape quantization tanlansa decoded min/max xatosini tekshirish.
6. Valid qo‘shnilar bilan offline slope: `atan(sqrt((dz/dx)^2 + (dz/dy)^2))` radians → degrees; dx/dy metrlarda, map-scale distortion hisobga olinadi. NoData yonidagi natija invalid. Flat va ma’lum plane testlari shart.
7. Author-created baza, tosh va mission obyektlarini alohida qatlamda joylash; yo‘l qilish uchun ilmiy balandliklarni tekislamaslik. Collision va CharacterMovement orqali ikkala yo‘lni o‘tish.
8. Ixcham cooked mesh/height, slope samples, route points va metadata lokal packagega kiritiladi. Runtime internet va har-frame geoprocessing talab qilinmaydi. Raw full grid runtimega kiritish shart emas.

Final manifest quyidagilarni majburiy saqlaydi: product ID/version, URLs, download UTC, raw/derived SHA-256, ROI polygon, CRS/reference sphere, pixel registration, units/grid spacing, NoData/mask policy, origin/axis mapping, processing software/version/commands, interpolation va quantization, credit/terms. Hozir bunday final manifest mavjud emas.

## O‘yinda ma’lumotning rejalashtirilgan aniq roli

Haqiqiy grid balandlik/collisionni, undan hisoblangan slope esa A/B yo‘llarning profili va taqqoslashini boshqaradi. Planner va science terminal aynan shu cached samplesdan foydalanadi. Telemetry haqiqiy harakat vaqtini va masofani o‘lchaydi. Hozir bu bog‘lanishlar **amalga oshirilmagan**; mavjud terrain sun’iy.

## Fakt, model va badiiy qatlam

| Qatlam | Izoh va dalil |
| --- | --- |
| Moon gravity | O‘yinda 1.62 m/s²; [NASA Moon Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html). Lokal gravitatsiya maydoni simulyatsiya qilinmaydi. |
| Mars gravity | O‘yinda 3.71 m/s² doimiy gameplay qiymati; [NASA Mars Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/marsfact.html)dagi fizik qiymatlar bilan solishtirish mumkin; spatial model emas. |
| Atmosfera | Oyda juda siyrak exosphere; Marsda siyrak atmosfera bor. [Moon facts](https://science.nasa.gov/moon/facts/), [Mars facts](https://science.nasa.gov/mars/facts/). Ikki sayyora muhiti tenglashtirilmaydi. |
| Audio | Vakuum orqali odatiy mexanik tovush tarqalmaydi: [NASA electromagnetic waves](https://science.nasa.gov/ems/02_anatomy/). Radio, nafas va suit-conducted qadamlar badiiy audio; fizik akustika hisoblanmaydi. Mars audiosi ham o‘lchangan atmosfera akustikasi emas. |
| Resource | 300/480 s — o‘yin balansi. Litrlarda O2 yoki haqiqiy skafandr ishlash vaqti emas. Hozir har active sekundga 1 budget sekund ketadi. |
| Movement | Tezlik, jump, hold vaqtlar va kelajak slope koeffitsiyentlari gameplay tuning; fiziologik aniqlik da’vosi yo‘q. Slope “astronaut-safe” degan mezon emas. |
| Authored content | SELENE/ARES, rescue voqeasi, science records, joylashtirilgan baza/toshlar; [credits](CREDITS.md). |

Science & Sources oynasi kelajakda uch bo‘limni aniq ajratadi: “Measured & derived data”, “Simplified gameplay rules”, “Authored story & objects”. Asosiy HUD qisqa qoladi.

## Presentation update - 29 October 2026

Intrepid contact detail is fitted to the existing imported terrain collision.
It is an authored visual accommodation to the triangulated DEM, not measured
landing-gear physics or a reconstruction of the 1969 touchdown attitude.
Small surface fragments and tool motions remain authored detail; they do not
increase the spatial resolution of the NASA/LROC elevation source. Debrief
reports recorded horizontal travel, cargo/documentation and optional work.
The timed exercise balance is a gameplay time budget. See EXPEDITION_POLISH.md.

## Local expedition trace and presentation additions

My Expedition draws a sampled local gameplay path in the existing world-centimetre
coordinate system over the modern LROC background. Gold is the current player's
recorded trace; thin cyan is the separately sourced reconstructed historical
traverse. The display is bounded to 1,024 points and may be simplified. Teleports
and restored checkpoints begin separate segments; missing pre-update data is
labelled partial rather than reconstructed. Display polyline length is not used
to replace the independently accumulated horizontal movement distance.

Photo/cargo markers identify mapped work sites whose tasks were committed. They
do not claim to record the camera's exact shutter location. Boot marks, garment
shapes, cargo arrangement and stowage animation are authored visual content. They
add no terrain measurements, astronaut biomechanical validation or safety rating.
# Offline planet context and model references — 14 November 2026

The optional planet overview uses the [NASA SVS CGI Moon Kit](https://svs.gsfc.nasa.gov/4720/) LROC colour map and the [NASA/JPL-Caltech Mars texture](https://science.nasa.gov/3d-resources/mars/), assembled from Viking imagery processed by USGS. These are colour/context images, not a newly integrated elevation dataset. The CGI Moon Kit explicitly prioritizes visual appearance, including polar filling; do not treat its displayed pixels as quantitative terrain measurements. Authored sphere shading, rotation and zoom add no scientific observations. Source URLs, checksums, download dates and limits are retained in [the planet manifest](SourceArt/PlanetContext20261114/manifest.json) and [credits](SourceArt/PlanetContext20261114/CREDITS.md).

Mission locations and distances come from actual current local world positions. Apollo's existing LROC local basemap and measured terrain remain separate. Fictional SELENE/ARES coordinates are not invented on the global colour sphere. The game does not claim a terrain safety rating from this overview.

The Moon training rover uses a [Deutsches Museum model](SourceArt/MuseumRover20261113/CREDITS.md) with authored LODs and separately retained source-wheel pivots. Its swept kinematic controller is a simplified gameplay model with acceleration, braking, steering and terrain support tests; it is not validated lunar vehicle dynamics. The LasquetiSpice [astronaut derivative](SourceArt/ExpeditionSuit20261113/CREDITS.md) uses authored retargeting and procedural poses, not the source floating clip as a walk cycle. Legacy specular/glossiness is adapted to core metallic/roughness: base colours and normal-map UVs are retained, metallic is authored as zero and roughness is `max(0.12, 1 - source glossiness)`. These are visual approximations, not measured pressure-suit material properties.

Fictional SELENE's NOMAD-7 uses the official [NASA/JPL Perseverance model](SourceArt/Perseverance20261114/CREDITS.md) as visual reference while retaining the original game memory-recovery objective. Perseverance is a Mars mission and did not visit the Moon. This use is not historical Apollo equipment, evidence of a real lunar experiment or NASA endorsement. The source's materials, UVs and geometry remain documented separately from gameplay rewards and story.

The four pre-EVA checks and isolated cable matching exercise are authored educational game rules. They do not measure a real suit or teach an operational electrical repair procedure. The **Verify** action applies only the existing cable step; the station still requires its normal restart interaction.
