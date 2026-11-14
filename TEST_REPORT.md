# Test reports

## Public feedback preview v0.9.2 — 2026-11-14

See [public Windows validation](Docs/PublicRelease20261114/PUBLIC_TEST_REPORT.md), [exact-build evidence](Docs/PublicRelease20261114/VALIDATION_EVIDENCE.json) and [clean-copy receipt](Docs/PublicRelease20261114/ARCHIVE_VERIFICATION.json). These supersede no historical artifact: prior reports below retain their original scope. Unconfirmed new media and owner data are excluded from the public download.

## Local hand-pose correction — 14 November 2026

[Docs/HandPose20261114/TEST_REPORT.md](Docs/HandPose20261114/TEST_REPORT.md) records the new palm-roll and duplicate-sleeve correction. Its executable/payloads are separate from v0.9.0. The historical 2,019-assertion result below is not certification of this new local build.

## Local feedback development — 14 November 2026

Current unpublished work is documented in [Docs/Feedback20261113/TEST_REPORT.md](Docs/Feedback20261113/TEST_REPORT.md). That record distinguishes exact Editor snapshots, the fresh package validation, synthetic graphical fixtures, performance captures and unrun human/network-isolation checks. The historical release results below apply to their original executable and payloads.

## v0.8.2 release preparation — 2026-11-11

The Windows release packages the exact `59d4f795b114` executable and cooked
payloads verified in the dated cabin record below. The canonical result remains
1,763 C++ PASS executions / 0 FAIL in 31 reports. Original compile and local
validation records are preserved under Docs/Releases/apollo12-v0.8.2; the release
source commit is recorded separately by the archive tool. Earlier statements
that no release was published describe the original local verification stage.
Any later clean-extracted release smoke is separate evidence in the release's
RELEASE_SMOKE.json and does not increase this canonical total.

## 2026-11-11 — Intrepid cabin: asl normallarni tiklash

**Final packaged tekshiruvlar: 1 763 C++ PASS bajarilishi / 0 FAIL,
31 raw hisobot va 32 valid PNG.** Bu takroriy konfiguratsiyalardagi bajarilishlar,
1 763 noyob test emas. Editor’ning dastlabki 4 review’i alohida: 182 PASS / 0 FAIL,
24 PNG. Asset builder’ning 46 tekshiruvi, source mapping’ning 9 fixture’i va
wrapper fixture’lari gameplay PASS soniga qo‘shilmadi. Inson playtesti: **0**.

Win64 Development inner executable SHA256:

    59d4f795b1141694741264370efe9b79dad66eaa9b4487270ffe7f9517fc358c

Compile base Git: `4d1d651f43809071da42409f483506487099f164`, worktree dirty.
Build metadata 2026-11-11 17:45:25 UTC’da yozilgan va keyingi test sanasidan
ajratilgan. Bu lokal bosqichda push, reliz yoki Telegram xabari yuborilmadi.
[Dalillar indeksi](Docs/IntrepidCabin20261111/verification-index.json).

| Bir xil final EXE’dagi lokal bajarilishlar | Raw C++ PASS |
| --- | ---: |
| Source/cabin NullRHI va cabin DX11 collision-enabled yurish — 3 run | 129 |
| Ikki yaqin kamera, Q0/Q1, 720p/1080p, control/feature — 4 run | 172 |
| Mavjud to‘rtta keng kamera, Q0/Q1, 720p, control/feature — 2 run | 96 |
| Normal-delta, 6 000-frame profillar — 2 run | 86 |
| **Lokal jami — 11 raw report** | **483** |
| Uch missiya stride va to‘liq Apollo interaction/traversal — 4 report | 99 |
| Apollo/SELENE/Mars/menu/walk/suit/polish/guidance/fieldwork/checkpoint — 12 report | 460 |
| Mission selection — 1 report | 66 |
| Field Operations Apollo/SELENE/Mars — 3 report | 655 |
| **Final umumiy — 31 raw report** | **1763** |

PASS sonlari raw reportlardan bir marta sanaldi; wrapper stdout yoki shu
reportning JSON nusxasi yana qo‘shilmadi. Har fresh reportda bitta RESULT 0 va
FAIL yo‘qligi tekshirildi. Quyidagi avvalgi surface/foil build `dc0b5864…`
natijalari bu yangi EXE uchun qayta ishlatilmagan.
Regressiya recount yordamchisi dastlab uchta UTF16 Field Operations faylini
UTF8 deb o‘qib to‘xtagan; BOM decoder tuzatilib, 20 raw fayl va SHA qayta
tekshirildi: 1 280 PASS / 0 FAIL. Buning uchun o‘yin qayta ishga tushirilmadi.
Oldingi conventional Apollo report yo‘llari backup’dan bayt/mtime bilan tiklandi;
yangi canonical nusxalar Cabin regression papkasida alohida saqlandi.

**Nima tuzatildi.** Asl GLB’da mavjud bo‘lgan normal vektorlari oldingi
Trimesh scene-copy/OBJ eksportida yo‘qolgan: OBJ’da `vn` yoki smoothing record
yo‘q edi. Alohida source vositasi 226 776 geometry/UV/group/face record’ni aynan
qayta hosil qilib, ikki cabin obyektidagi 1 237 triangle / 3 711 corner uchun
asl normalni oldi. GLB’ning 716 840 bayti NASA rasmiy download’i bilan solishtirildi;
SHA256 `379101dfcee399267addf771709107c35e826b0b9e5233272f123e4c8a585c4e`
mos. Bu modelning badiiy normallari; yangi relyef o‘lchovi emas.

Unreal Editor vositasi original assetni alohida `SM_IntrepidCabin` nusxasiga
ko‘chiradi. Mavjud UE meshida 96 460 render triangle, 289 380 vertex instance
bor; uning hisoblangan rendering frame’lari nusxaga bake qilinadi. Original
MD’da tangentlar nol bo‘lgani uchun ularni oddiy nusxalash to‘g‘ri natija bermagan.
836 source-degenerate tangent 570 face, beshta material slotida aniq predicate
(nol yoki normalga parallel X) bilan qayta qurildi. Cabin ichida 3 699 tangent
source-computed X’dan proyeksiya qilindi, 12 tasida ortogonal o‘q ishlatildi.
Bular yangi UV yoki GLB-authored tangent degani emas.

Geometriya, indeks/topology, UV, material joylashuvi va boshqa MD atributlari
qat’iy saqlandi; original fayl/MD/sidecar/dirty holati o‘zgarmadi. Cabin tashqarisi
uchun bitwise render identity **da’vo qilinmaydi**. Barcha normal va valid
tangentlar haqiqiy SNorm8 component step `1/127 + 0.000002` chegarasida;
faqat oldindan yaroqsiz source tangent uchun qayta qurish alohida sanaladi.
W/sign aniq saqlanadi. Cabin normallari actual GPU buffer’da packing half-step
chegarasida tekshirildi. [Asset va frame siyosati](Docs/IntrepidCabin20261111/IMPLEMENTATION.md).

Ikki shared white slot uchun alohida mato materiali qo‘shildi; bu slotlarda
cabin bilan birga eski trim ham bor. GLB ranglari `.498039216` va `.678431392`
MID parametrlarida alohida saqlandi. Roughness `.78`, specular `.18`, metallic
`0`, 2.5% rangli panel izi — authored tuning. 80 source-local cm naqsh hozirgi
masshtabda taxminan 110 cm; tarixiy panel o‘lchovi emas. WPO, yangi vertex,
collision, navigation, yorug‘lik yoki Tick qo‘shilmadi. LROC terrain, 698 cm
reference normalization va to‘rtta pad anchor joyi paired runlarda mos.

**Vizual ko‘rik.** Yangi `.uasset` yangi Editor jarayonida va packaged EXE’da
yuklandi. 720p/1080p yaqin old/yon kadrlar hamda keng/narvon kadrlarining
tanlangan namunalari ko‘rildi. Ayrim keskin qoramtir choklar yumshagan; siluet
hanuz qirrali, quyoshdagi oq joylar kam kontrastli. 720p’da mato izi juda nozik.
Mustaqil ROI ko‘rigida literal clipping tasdiqlanmadi: maksimum 247,
RGB uchalasi >=254 bo‘lgan pixel yo‘q. Antenna, narvon va terrain framing saqlandi.
Bu staged screenshotlar: HUD/pawn/world TextRender yashirilgan. Barcha PNG’lar
alohida estetik ko‘rikdan o‘tdi degan da’vo yo‘q; temporal shimmer yoki fotoreal
sifat isbotlanmadi. Alohida fresh-process MD/render-buffer reload audit yo‘q;
fresh-load visual/runtime evidence shu auditdan ajratilgan.

**Matched performance.** Windows 11 x64; Ryzen 5 6600H; RTX 3050 Laptop GPU
4 GB; taxminan 16 GB RAM; UE 5.8.3; MSVC 14.44; Windows SDK 22621.
DX11 rendered offscreen, 1280 × 720, Medium/Q1, FOV 85, VSync off, cap 60.
Bir xil to‘rtta kamera (bittasi hudud tashqarisida), parked support actors,
frozen pawn/missiya va normal engine delta. Har capture’dan oldin 300 settle
frame, so‘ng 6 000 CSV frame; CSV’dan warmup chiqarilmagan.

| Shu EXE: cabin o‘chiq → yoqilgan | Control | Cabin |
| --- | ---: | ---: |
| O‘rtacha FPS | 59.9024 | 59.9837 |
| p95 frame, ms | 16.6919 | 16.6872 |
| p99 frame, ms | 16.7491 | 16.7256 |
| 1% low FPS | 51.6663 | 58.2050 |
| GameThreadTime o‘rtacha, ms | 2.0761 | 1.9136 |
| RenderThreadTime o‘rtacha, ms | 4.4573 | 4.0621 |
| Pollingdagi maksimal working set, MiB | 627.2227 | 610.7188 |

Bitta capped juftlik tezlashish yoki xotira optimizatsiyasi sababini isbotlamaydi.
Bu butun missiya/rover FPS’i emas; yaqin FOV60 camera uchun alohida performance
capture yo‘q. Working set haqiqiy peak yoki VRAM emas. GPUTime ikkala capture’da
6 000 finite/positive qator, lekin cap/pacing headroom xulosasini bermaydi.
`GPUMem/LocalUsedMB` positive qatorlar 5 994 / 5 998; engine counter’i mustaqil
dedicated-VRAM o‘lchovi deb yozilmaydi. Sample alignment va mustaqil hisoblar
verification index’da saqlanadi. Xotira polling’i nominal 1 000 ms kutadi,
ammo instrumentatsiya bilan haqiqiy median interval 1.3685 / 1.3486 s;
82 / 83 sample butun jarayon, startup va settle’ni ham qamraydi. CSV-only
yoki aniq 1-second peak emas. CSV’dagi haqiqiy kamera qatorlari ikkala yozuvda
1501/1500/1500/1499; o‘tishlar metadata’dan bir qator keyin. Control’da uchta
33.333 ms’dan uzun frame bor, cabin’da nol; bitta fixed-order capture bu
farqning sababini aniqlamaydi.

**Build va buyruqlar.** Editor `editor-build-g.log`, Game `game-build-a.log`
Succeeded; to‘rtta map uchun fresh cook va StageOnly/archive exit 0. Default
map regeneration yoki suit/global material rebinding ishlatilmadi. Asset `g`
runida mesh saqlangach Python interpolator pin nomi xato chiqdi; tuzatilgan
`material-build-h` faqat materialni yaratdi, mesh hashini saqlab, exit 0 bilan
tugadi. Bundan oldingi tangent/pointer diagnostika xatolari tarixda saqlandi,
final PASS sonlariga kirmadi.

```powershell
./Tools/verify_intrepid_visual.ps1 -Packaged -Cabin -Label new-cabin-functional
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Cabin -CabinReview -CabinBaseline -Label new-close-control
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Cabin -CabinReview -Label new-close-feature
./Tools/profile_intrepid_slice.ps1 -Packaged -Cabin -CabinBaseline -Label new-profile-control -GpuTelemetry
./Tools/profile_intrepid_slice.ps1 -Packaged -Cabin -Label new-profile-feature -GpuTelemetry
```

Har run uchun yangi label; 1080p uchun `-Width 1920 -Height 1080`.
`-CabinBaseline` faqat yangi cabin nusxasi/finish’ini o‘chiradi;
`-Baseline` esa oldingi ground/foil qatlamini mustaqil boshqaradi.
Python topilmasa `-Python` bilan Python 3 yo‘lini bering. NullRHI FPS testi emas.

**Yakuniy preservation.** Yangi bosqich boshlanishidagi 906 Saved, 5 Config va
4 map — jami 915 original fayl bayt/mtime bo‘yicha mos. 80 ta owner save/photo
tiklashsiz o‘zgarmagan. Sakkiz izolyatsiyalangan/config faylda 11 tiklash amali
bajarildi; ulardan uchtasi AutomatedTests save fixture’i. 46 yangi engine INI
saqlandi, yangi owner save/photo yo‘q. Avvalgi to‘rt material, original lander
assetlari va manbalar — 32 identity file — bayt/mtime bilan mos; oldingi
154 ilmiy/model payload SHA va original 860 manifest ham o‘zgarmagan.
Tasdiqlangan source/stage nusxasi ortga qaytarilmadi. Shaxsiy save/photo
ro‘yxatlari faqat lokal Reports’da, Docs’da sanitizatsiyalangan hisoblar bor.

Yangi inson playtesti, clean extracted-distribution, firewall/network-isolation,
actual missing-asset injection, temporal video va full-mission performance
bajarilmagan. Runtime yangi internet xizmatini talab qilmaydi. Space Apps
eligibility va NASA tasdig‘i ushbu mahalliy art bosqichining natijasi emas.

## 2026-11-11 — Intrepid atrofidagi 50 × 50 m vizual bosqich

**Yakuniy lokal Windows build: 1 615 C++ PASS bajarilishi / 0 FAIL, 29 raw
hisobot.** Bular takroriy konfiguratsiyalardagi bajarilishlar; 1 615 noyob test
yoki inson sinovi emas. Editor c ko‘rigi alohida: 80 PASS / 0 FAIL, 16 PNG.
GitHub relizi, push yoki Telegram xabari ushbu bosqichda bajarilmadi.

Win64 Development inner executable SHA256:

    dc0b5864f7dd539006711408c00c7c0a880e7a40dc55afffe58d5b26f13bcfe2

Base Git: `4d1d651f43809071da42409f483506487099f164`; worktree dirty.
Source/art fingerprint:
`fd44dc95d2e648e0605a56e6a096270f3e301fc517c6134a2ec264fafeb836da`.
[Verification index](Docs/IntrepidVisual20261111/verification-index.json)
raw hisobotlar, build, kadr, CSV va saqlash dalillarini ajratadi.

| Yakuniy packaged tekshiruv | Raw C++ PASS |
| --- | ---: |
| Lokal yurish: baseline, yangi qatlam, DX11 yangi qatlam | 105 |
| 720p/1080p × baseline/yangi × Low/Medium kadr ko‘rigi | 160 |
| Ikki normal-delta 6 000-frame profile driver tekshiruvi | 70 |
| Uch missiyada stride va to‘liq Apollo interaction/traversal | 99 |
| Apollo, SELENE, Mars, menu, walk, suit, polish, guidance, fieldwork, checkpoint — 12 case | 460 |
| Mission selection | 66 |
| Field Operations — Apollo, SELENE, Mars | 655 |
| **Jami — 29 fresh raw report** | **1615** |

Har bir raw report bir marta sanaldi; bitta `RESULT: 0 failures`, ijobiy PASS
soni va FAIL yo‘qligi mustaqil tekshirildi. Wrapper stdout, manifest va CSV
qatorlari yana PASS sifatida qo‘shilmadi. Regressiya yig‘uvchisida
`Measure-Object` OrderedDictionary sonlarini yig‘olmagani uchun dastlab jami
bo‘sh chiqqan; 20 raw report va SHA qayta o‘qilib 1 280 soni tasdiqlandi.
O‘yin natijalari almashtirilmadi va bu uchun o‘yin qayta ishga tushirilmadi.

**Natija va ilmiy chegaralar.** Yangi qatlam 2 048 mayda va 64 ajratilgan
soyali fragmentni to‘qqiz collision/navigation-free HISM batch’da joylaydi.
Meshning haqiqiy burilgan chegaralari, qisman yerga kirishi, kit/narvon apron’i
va uch yurish yo‘lagi tekshiriladi. Har lokal functional run’da taxminan
28,36 m haqiqiy CharacterMovement yurishi, kitga yetib borish va resurs/missiya
holati o‘zgarmasligi tasdiqlandi. Bu tabiiy o‘yinchi sinovi emas.

Intrepidning uch untextured foil slot’i va bir metall slot’i yangilandi;
12 sayoz folga burmasi mavjud olti batch’dan foydalanadi: baseline 42, yangi
ko‘rinish 54 instance. Asl narvon, rasmli panellar, suit assetlari va import
geometriyasi saqlandi. Yangi regolit normal/grain, toshlar, folga va lokal AO
badiiy qatlam: ular NASA o‘lchagan mikrorelyef yoki tarixiy tosh tarqalishi emas.
Asosiy balandlik/collision manbai mavjud **LROC NAC_DTM_APOLLO12 v1.9, 2 m grid**;
100 Unreal cm = 1 m. Terrain displacement va yangi dataset kiritilmadi.

**Kadrlar.** Bir xil EXE’da to‘rt kamera, FOV 85, haqiqiy Q0/Q1, 720p va
1080p bo‘yicha 32 valid PNG olindi. Paired 85 terrain height/normal sample,
8 057 tashqi original instance inventari, kamera va uch directional-light
holati mos. Tanlangan keng, tuproq va haqiqiy +Y narvon kadrlari ko‘rildi.
Yorug‘ folga burmasi va mayda toshlar o‘qiladi; oq yuqori cover hali qo‘pol
qirrali, soyadagi folga esa qoramtir. Kadrlar HUD/pawn va world TextRender
yozuvlarini yashiradigan staged fixture; ular oddiy gameplay video emas.

**Grafik o‘lchov.** Windows 11 x64; Ryzen 5 6600H; RTX 3050 Laptop GPU,
4 GB; 16 312 721 408 byte tizim RAM; UE 5.8.3; MSVC 14.44; Windows SDK 22621.
DX11 rendered offscreen, 1280 × 720, Medium/Q1, FOV 85, VSync off, cap 60;
normal engine delta, production parked support actors, frozen pawn/mission.
Har sinovda CSV’dan oldin 300 settle frame, keyin 6 000 haqiqiy frame;
CSV ichidan warmup chiqarilmadi. Boshqa UE/compiler/shader jarayoni kuzatilmadi.

| Bir xil build: qatlam o‘chiq → yoqilgan | Control | Yangi |
| --- | ---: | ---: |
| O‘rtacha FPS | 59.9821 | 59.9961 |
| p95 frame, ms | 16.6805 | 16.6769 |
| p99 frame, ms | 16.7302 | 16.6994 |
| 1% low FPS | 58.3277 | 59.6849 |
| GameThreadTime o‘rtacha, ms | 1.7576 | 1.6915 |
| RenderThreadTime o‘rtacha, ms | 3.6055 | 3.4710 |
| Maksimal 1-second sampled working set, MiB | 596.1563 | 596.8789 |

Bu bitta capped juftlik; tezlashishning sababi yoki butun o‘yinda barqaror
60 FPS isbotlanmadi. Working set haqiqiy peak yoki VRAM emas. CSV GPUTime
6 000/6 000 finite/positive (16.6619 → 16.6631 ms), ammo cap/pacing bilan
qo‘shimcha GPU headroom chiqarilmaydi. `GPUMem/LocalUsedMB` 247.4883 → 248.7383
engine counter bo‘lib, mustaqil dedicated-VRAM o‘lchovi emas. Bitta kamera ROI
tashqarisidagi control. CSV View/Pos o‘tishlari ikkala yozuvda ham metadata’dan
bir qator keyin: 1501/3001/4501; haqiqiy kamera qatorlari
1501/1500/1500/1499. Bu sampling moslashuvi cheklovi ochiq saqlandi.

Oldingi EXE `06c1b022…` bilan boshqa — moving-opening — 2 400/600-frame tarixiy
sinov ham qaytarildi: mean 59.9950 → 59.9982 FPS, p99 18.2171 → 17.8246 ms.
U local four-camera baseline emas; sababiy yaxshilanish da’vosi yo‘q.
Generic auto-CSV exit `777003` raw holida qayd etildi: wrapper fresh finite CSV
va log bo‘yicha qabul qiladi, uni functional exit 0 deb ko‘rsatmaydi. Wrapper
EXE hashini o‘zi qayd qilmagani uchun bu qo‘shimcha capture qo‘shni final-build
konteksti sifatida saqlangan; canonical to‘qqiz run hashni tekshiradi.

**Build va takrorlash.** Editor va Game Win64 Development UBT buildlari
`Succeeded`. To‘rt map uchun fresh UAT cook va keyingi StageOnly/archive
`ExitCode=0`; eski cook qayta ishlatildi deb ko‘rsatilmaydi. Yangi to‘rt material
Unreal Python material API bilan yaratildi va paketda yuklandi; binary fayllar
matn tarzida tahrir qilinmadi. Map regeneration, global/suit material rebinding
skriptlari ishlatilmadi. To‘liq build/test buyruqlari
[implementation](Docs/IntrepidVisual20261111/IMPLEMENTATION.md) va build
receiptlarda. Asosiy test buyruqlari:

```powershell
./Tools/verify_intrepid_visual.ps1 -Packaged -Label new-functional
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Baseline -Label new-control
./Tools/verify_intrepid_visual.ps1 -Packaged -Review -Label new-feature
./Tools/profile_intrepid_slice.ps1 -Packaged -Baseline -Label new-control-q1 -GpuTelemetry
./Tools/profile_intrepid_slice.ps1 -Packaged -Label new-feature-q1 -GpuTelemetry
./Tools/verify_task_fixes.ps1 -Packaged -Suite all
./Tools/verify_apollo12.ps1 -Packaged -Regression -Walkthrough -Suit -Guidance -Polish -Checkpoints -Fieldwork
./Tools/verify_mission_selection.ps1 -Packaged
./Tools/verify_field_operations.ps1 -Packaged -Mission all
```

Har Intrepid run uchun yangi label talab qilinadi. 1080p uchun
`-Width 1920 -Height 1080`; wrapper Python’ni topmasa `-Python` bilan mahalliy
Python 3 yo‘lini bering. NullRHI functional natijalar FPS sinovi emas.

**Saqlash va bajarilmagan ishlar.** 154/154 original science/model/material
payload size/SHA mosligi mustaqil tekshirildi. Original 860 Saved fayl va
5 Config/4 map — jami 869 original — yakunda bayt/mtime bo‘yicha mos.
128 faylda 251 tiklash amali bajarildi: engine INI’lari va uchta izolyatsiyalangan
AutomatedTests save fixture’i; 80 haqiqiy owner save/photo faylga tiklash
kerak bo‘lmadi, bayt/mtime holati o‘zgarmagan. Yangi 46 engine INI saqlandi;
yangi save/photo yo‘q. Shaxsiy save/photo inventari Docs’ga ko‘chirilmaydi.
Runtime kerakli ma’lumot/materiallarni lokal paketdan yukladi; yangi internet
bog‘liqligi yo‘q. Alohida clean extraction, firewall/network-isolation va boshqa
hardware’da sinov bajarilmadi. Yangi inson playtest ishtirokchilari: **0**.
To‘liq missiya/rover FPS, tabiiy kamera harakatidagi shimmer, barcha ROI
chegaralarining video ko‘rigi va haqiqiy assetni olib tashlash fallback sinovi
bajarilgan deb ko‘rsatilmaydi. Fallback preflight kodi tekshirildi.

Ilk Editor a screenshot callback lifetime xatosi (8 FAIL) tuzatildi; b
iteration harness’i o‘tgan bo‘lsa ham qora folga va noto‘g‘ri qo‘shimcha rung
tasmalari vizual rad etildi. Yakuniy c va package kadrlarida bu tasmalar yo‘q,
asl narvon saqlangan. Oldingi noto‘g‘ri capture/compile loglari tarixda qoldi;
ular canonical PASS sonlariga kirmaydi. Bu lokal mahsulot bosqichi NASA
tasdig‘i yoki Space Apps eligibility qarori emas; tanlovga yuborish bajarilmadi.

## 2026-11-06 — Oltita interaction/presentation tuzatishi: yakuniy lokal Windows build

**4 092 C++ PASS bajarilishi / 0 FAIL; 65 raw report; eski 16/16 suite o‘tdi.**
Editor selection alohida **66 PASS / 0 FAIL**. Bu takroriy missiya, renderer va
resolution bajarilishlarini ham sanaydi; 4 092 noyob test yoki inson sinovi emas.

Final Win64 Development inner executable SHA256:

    06c1b022f92b4e13e8300fc8d71296f290ee4f2565ac8597ebcd337519facda8

Base Git: `67cd5e3cc5f80c993a9e58b4f582c3ef323426a9`; worktree dirty.
**Dastlabki tekshiruv holati — lokal/unreleased:** quyidagi tekshiruvlar vaqtida
commit/push, GitHub relizi yoki Telegram xabari yuborilmagan edi. Keyingi alohida
foydalanuvchi ruxsati bilan ayni tekshirilgan executable v0.8.1 nashriga tayyorlandi.
Bu asl test sanalari va compile provenance’ini o‘zgartirmaydi; source snapshot
commit’i release metadata’da alohida saqlanadi. Public v0.8.0 ZIP’da bu tuzatishlar yo‘q.

| Yakuniy packaged exe bo‘yicha dalil | PASS execution |
| --- | ---: |
| Mavjud 16-suite regressiya navbati | 2683 |
| Yangi stride va to‘liq Apollo interaction/traversal | 99 |
| Field Operations: NullRHI va DX11 functional, uch missiya | 1310 |
| **Jami** | **4092** |

Faqat tanlangan fresh raw C++ reportlardagi aniq PASS satrlari sanaldi; wrapper
PASS va JSON summary sonlari qayta qo‘shilmadi. Qo‘shimcha standalone NASA smoke
277 PASS bilan saqlandi, jami ichiga kiritilmadi. Editor’ning 66 PASS’i boshqa
executable/game-module identifikatoriga tegishli. [Verification index](Docs/TaskFixes20261106/verification-index.json)
va [dalil yaxlitligi](Docs/TaskFixes20261106/evidence/bundle-integrity.json) sonlar,
raw fayllar, SHA va chegaralarni ajratib beradi.

**Tuzatishlar va tekshiruvlar:**

- Mission picker tekstura keshini qayta ishlatadi, tinch holatda ortiqcha UI
  yangilanishini to‘xtatadi va blue glass tanlovini saqlagan holda ortiqcha blur
  qatlamlarini kamaytiradi. Lag xabari aniq ekran/repro bermagan; picker ishchi
  faraz sifatida olingan. Butun o‘yin lag’i tuzatildi degan xulosa yo‘q.
- Start Mission/Settings/Back fon va hit area’lari tashqi ramkani to‘ldiradi;
  caption markazda, hover/press/keyboard focus holatlari mos. 720p/1080p,
  100%/150% text: 12 action record, 60 sampled hit-test path va 12 haqiqiy Slate
  keyboard-focus record. Bu 60 ta click emas; mission kartalarida 6 real pointer
  route alohida tekshirildi. Action-button click bajarilishi deb da’vo qilinmadi.
- PIP-3 va communicator grounded, terrain/capsule approach clearance bilan lander
  footprint’idan tashqarida joylashadi. PIP-3 stationary fictional guide;
  follow-AI qo‘shilmadi.
- Cell, sample pouch/core va hardware qo‘l bilan bir grip frame’dan joylashadi;
  haqiqiy HandRig/SuitRig contact va arm-length tekshiruvlari kuchaytirildi.
  Ochiq pouch uchun unsigned triangle proximity, belgilangan solid grip uchun
  signed-depth tekshiruvi qo‘llanadi. Bu to‘liq glove physics/collision isboti emas.
  Ikki mavjud sleeve mesh qayta ishlatiladi; 3-lumen pooled camera fill faqat
  first-person owned materiallarni yoritadi. Yangi binary asset qo‘shilmadi.
- ALSEP prompt uzoqlik, qarash tomoni, blockage va pending photo’ga mos. PSE
  bubble va Earth-link antenna alohida tasdiqlanadi: 0/2 → 1/2 → 2/2. Landerdagi
  field kit uzoqdagi instrumentni bajarmaydi; stale/unrelated commit rad etiladi,
  restart antenna holatini tozalaydi. NASA work-site koordinatalari saqlandi.
- Real controller input va CharacterMovement velocity bilan hold/release/toggle
  stride tekshirildi: Apollo 140→220 cm/s, SELENE/ARES 320→500 cm/s. Pause,
  planning, work, panel va vehicle gate’lari tekshirildi. Oxygen active gameplay
  seconds bo‘lib qoldi; qo‘shimcha fiziologik sarf multiplikatori kiritilmadi.

Yangi Apollo driver collision yoqilgan **1 285.90 m**, **946.24 game seconds**
yo‘lni real W/E/U/arrows/click/Esc inputlari bilan bosib o‘tdi: ALSEP, samples,
Surveyor hardware, return va haqiqiy R/map restart. Route teleport yoki to‘g‘ridan
mission-complete chaqiruvi ishlatilmadi. Bu fixed 1/60 simulyatsiya, real-time yoki
FPS dalili emas. Eski tashqi binary checkpoint fixture yangi driver’ga berilmagan
va u aniq SKIP; independent legacy checkpoint suite alohida o‘tdi.

230 staged PNG’ning hash/o‘lchami va 8 paired-pixel check tasdiqlandi. PNGlar
lokal Reports’da; Docs ichiga nusxalanmagan. Pixel farqi render hissasini
ko‘rsatadi, grasp aniqligini isbotlamaydi. Yakuniy qayta renderlar receiptlarda;
qo‘l/pouch/hardware/cell va menu kadrlarining vizual ko‘rigi ham bajarildi.

Muhit: Windows 11 x64, Ryzen 5 6600H, RTX 3050 Laptop GPU, taxminan 16 GB RAM,
Unreal Engine 5.8.3, MSVC 14.44, Windows SDK 22621. Editor va Game UBT build
`Succeeded`; UAT StageOnly `ExitCode=0`. Mavjud cook qayta ishlatildi, fresh full
cook bajarilgan deb ko‘rsatilmaydi; bu iteratsiyada map/asset o‘zgarmagan.
115/115 source/config/data fingerprint entry final source bilan mos.
[Build receiptlari](Docs/TaskFixes20261106/evidence/build-receipts.json).

Bajarilgan asosiy buyruqlar (repository root; aniq suite parametrlar indeksda):

```powershell
& 'D:/UnrealEngine/UE_5.8/Engine/Build/BatchFiles/Build.bat' LunarRescueEditor Win64 Development '-Project=D:/LunarRescue/LunarRescue.uproject' -WaitMutex -MaxParallelActions=2 -NoUBA -NoPCH -NoSharedPCH
& 'D:/UnrealEngine/UE_5.8/Engine/Build/BatchFiles/Build.bat' LunarRescue Win64 Development '-Project=D:/LunarRescue/LunarRescue.uproject' -WaitMutex -MaxParallelActions=2 -NoUBA -NoPCH -NoSharedPCH -NoDebugInfo -NoLinkerDebugInfo
Tools/package_apollo12.ps1 -StageOnly
Tools/verify_task_fixes.ps1 -Packaged -Suite all
Tools/verify_field_operations.ps1 -Packaged
Tools/verify_field_operations.ps1 -Packaged -Rendered
Tools/verify_gameplay_regression.ps1
Tools/verify_mission_selection.ps1 -Packaged -Review -Width 1280 -Height 720
Tools/verify_mission_selection.ps1 -Packaged -Review -Width 1920 -Height 1080
Tools/verify_mission_selection.ps1
Tools/profile_mission_menu.ps1 -Label nov06-after-home -Scene Home -Frames 1800 -WarmupFrames 300 -Width 1280 -Height 720 -Quality 1 -FpsCap 60
Tools/profile_mission_menu.ps1 -Label nov06-picker-fullglass -Scene Picker -Frames 6000 -WarmupFrames 1500 -Width 1280 -Height 720 -Quality 1 -FpsCap 0 -FullGlassBaseline
Tools/profile_mission_menu.ps1 -Label nov06-picker-optimized -Scene Picker -Frames 6000 -WarmupFrames 1500 -Width 1280 -Height 720 -Quality 1 -FpsCap 0
```

Ishga tushirish: `Dist/Apollo12/Windows/LunarRescue.exe`. Yangi cook bilan to‘liq
qayta yig‘ish: `Tools/package_apollo12.ps1 -MaxCompileActions 2` (yuqoridagi
StageOnly o‘rniga). Bu reproduction buyrug‘i; ayni final build fresh cook qildi
va final testni qayta o‘tkazdi degani emas.

Performance: DX11 offscreen, 1280×720, Quality1, bir xil hardware, har sharoitda
bittadan capture. Home 1800 frame/300 warmup, cap60; Picker 6000/1500, uncapped.

| Capture | Mean FPS | p99 frame ms | Sampled working-set max MiB |
| --- | ---: | ---: | ---: |
| Old Home (08485e), cap60 | 59.9977 | 17.4696 | 548.7461 |
| Final Home (06c), cap60 | 59.9613 | 17.4559 | 542.3125 |
| Final Picker, fullglass control | 338.6142 | 4.0859 | 527.2578 |
| Final Picker, optimized | 349.9883 | 3.8483 | 524.6953 |

Picker taqqoslashi **ayni final build’dagi synthetic blur-cost control**, eski
versiya oldin/keyin testi emas. Kuzatilgan +3.359% universal kafolat emas.
GPU/SlateUI mean 1.684625→1.600379 ms; raw CSV’da counter mavjud va valid.
Max working set bir soniyalik sample’lardan, true peak yoki VRAM emas. Raw CSV
footer recordlari frame deb sanalmadi. Capture’lar tugagach 15.911 soniyadan keyin
curation boshlangan; ular ustma-ust tushmagan. Thermal/power to‘liq nazorat
qilinmagan. NullRHI grafik performance emas; full-mission/occupied-driving FPS
yoki “60 FPS stable” da’vosi yo‘q. [Performance dalili](Docs/TaskFixes20261106/performance-comparison.json).

Saqlash: **567/567 protected file SHA va mtime mos**, to‘rtta map SHA o‘zgarmagan.
Build/test housekeeping olib tashlagan 483 eski crash-reporter cache INI
checksum-validated backupdan tiklandi; Editor config’dagi yagona TimeStamp
farqi tekshirilib original baytlar tiklandi. Gameplay saves/photos o‘zgarmagan.
[Saqlash dalili](Docs/TaskFixes20261106/preservation-final.json).

Iteratsiyadagi xatolar yashirilmadi: carry/geometriya checklari tuzatilguncha
29→6→3→0 FAIL; ortiqcha 140-lumen light 3 lumenga tushirildi; Editor diagnostic
linker xatosi tuzatildi. 17a build’dagi obsolete POV fixture ikkita FAIL bergan;
actual HandRig wrist va camera-local pouch/stow kontraktiga mos test correction
qilingach final06c’da POV30 PASS, barcha16 suite PASS. Tarixiy receiptlar final
hisobga qo‘shilmadi. [Kontrakt izohi](Docs/TaskFixes20261106/evidence/test-contract-correction.json).

Chegaralar: human participants0, network/firewall isolation bajarilmadi. Mavjud
lokal science payload paketda, yangi runtime internet dependency qo‘shilmadi.
Mavjud LROC balandliklari saqlandi; yangi NASA dataset yoki yangi ilmiy terrain
import qilinmadi. Grip/light/guide placement authored gameplay presentation.
NASA endorsement va competition eligibility tasdiqlanmagan. Asosiy keyingi ishlar:
real-player playtest, full-mission/occupied-rover performance va production-art
contact/animation ko‘rigi.


Static packaging tekshiruvi: o‘zgargan to‘rtta PowerShell script AST parse’dan
o‘tdi. Oddiy `git diff --check` yangi Apollo regression .cpp/.h oxiridagi ortiqcha
bo‘sh qatorni qayd etdi; ayni tekshirilgan source saqlandi. Boshqa whitespace
tekshiruvi shu EOF ogohlantirishini ajratib o‘tdi. Dated evidence uchun `-text`
qoidasi qo‘shilib, barcha Git-index baytlari va 65 raw-report SHA qayta solishtirildi.
Reliz helper’ining tashqi generated JSON fayllari LF bilan yoziladi; bu ZIP ichidagi
receipt va alohida yuklanadigan receipt xeshlari mosligini saqlaydi. Bu tooling
va hujjat o‘zgarishlari engine source/config/data fingerprint’ini o‘zgartirmaydi.

## 2026-11-04 — Field operations: yakuniy mahalliy Windows build

**4 018 C++ PASS bajarilishi / 0 FAIL; 16/16 eski suite; 280 PNG va 8 pixel-pair
tekshiruvi.** Bular takroriy missiya, render va resolution urinishlarini ham sanaydi;
4 018 noyob test yoki inson sinovi degani emas.

Final inner executable SHA256: 08485e289426127af57fca0bd79fa1b9bc1f67a3708f8d4385068140c8733a7a
Base Git commit: 54140321bb2607b836aa9ae12da468e9f56bf531. Build va quyidagi tekshiruvlar local/unreleased holatda bajarilgan. Keyingi
foydalanuvchi ko‘rsatmasi bilan v0.8.0 GitHub relizi va NASA guruhiga xabar
tayyorlandi; bu lokal tekshiruvning asl sanasi va compile provenance’ini o‘zgartirmaydi.

| Dalil | PASS execution | Raw report | PNG |
|---|---:|---:|---:|
| Yangi Field Operations NullRHI + DX11 functional, uch missiya | 1310 | 6 | 0 |
| Yangi 720p/1080p UI + actual-world review | 308 | 6 | 120 |
| Mavjud 16 suite regressiyasi | 2400 | 55 | 160 |
| Jami | 4018 | 67 | 280 |

Yangi receiptlar to‘liq va ayni executable bilan bog‘langan. Eski suite reportlari
fresh, alohida nusxalar; wrapper PASS qatorlari yoki fixed-output dublikatlari hisobga
qo‘shilmadi. Sakkiz carry pixel-pair tekshiruvi alohida tasdiqlandi.
[Verification index](Docs/FieldOperations20261104/verification-index.json),
[legacy dalili](Docs/FieldOperations20261104/legacy-verification.json) va
[bundle auditi](Docs/FieldOperations20261104/bundle-verification.json) tafsilot beradi.

Muhit: Windows 11 x64, Ryzen 5 6600H, RTX 3050 Laptop GPU, taxminan16GB RAM,
Unreal Engine5.8.3, MSVC14.44/WindowsSDK22621. Editor va game build muvaffaqiyatli;
full cook717 package, UAT ExitCode0. Keyingi test-fikstura o‘zgarishlarida faqat
tegishli C++ modullar qayta yig‘ildi va mavjud cookdan StageOnly bajarildi.

Asosiy buyruqlar:

    Tools/package_apollo12.ps1 -MaxCompileActions 2
    Tools/verify_field_operations.ps1 -Packaged
    Tools/verify_field_operations.ps1 -Packaged -Rendered
    Tools/verify_field_operations.ps1 -Packaged -Review -Width 1280 -Height 720
    Tools/verify_field_operations.ps1 -Packaged -Review -Width 1920 -Height 1080
    Tools/verify_gameplay_regression.ps1
    Tools/profile_mission_menu.ps1 -Label fieldops-20261104-after -Frames 1800 -Quality 1
    Tools/profile_apollo12.ps1 -Label fieldops-20261104-after -Frames 1800 -Quality 1
    Tools/profile_apollo12.ps1 -Label fieldops-20261104-enabled -Frames 1800 -Quality 1 -FieldOperationsProfile

Tekshirilgan xatti-harakatlar: ordered EVA/skip/pause; yopilgan rejalashtirishdan
bepul vaqt olmaslik; bir martalik difficulty va resurslar; B/G reserved/fallback;
haqiqiy ko‘rish chizig‘i va blocker; 0.4MB aniq chegara; cargo limiti/deposit;
recharge/downlink bilan native kislorod va pause; rescue wire isolation/matching;
swept rover/wheel/battery, blocked/zero-charge exit va EndPlay; native piyoda
telemetriyaga transport/dismount sakrashini qo‘shmaslik; strict JSON type/mask/units,
legacy/invalid optional state, SaveGame memory roundtrip; screenshot actual layout.
Har uch cartridge uchun real terrain/capsule/LOS kuzatuv nuqtasi topildi. Bu butun
marshrutning uzluksiz o‘tilganini kafolatlamaydi.

Performance: DX11 rendered offscreen,1280x720,Quality1,60FPS cap,1800captured frame.
Warmup menu300/Apollo600 chiqarildi. Bitta capture’dan kichik farqning sababini
isbotlab bo‘lmaydi; NullRHI grafik performance hisoblanmadi.

| Yuklama | Mean FPS before / after | p99 frame ms before / after |
|---|---|---|
| menu_same_workload | 60.00 / 59.99 | 17.45 / 17.46 |
| apollo_legacy_same_workload | 59.93 / 59.95 | 17.66 / 17.86 |
| apollo_enabled_support_opening | 59.93 / 60.00 | 17.66 / 17.78 |

Legacy opening after optional extensionni o‘chiradi; enabled opening parked rover,
support actors va bookkeeping’ni yoqib scripted walk qiladi. Occupied driving,
scanner/board benchmark va to‘liq missiya FPS barqarorligi o‘lchanmadi. RAM natijasi
one-second sampled process working set; VRAM deb ko‘rsatilmaydi.
[Performance dalili](Docs/FieldOperations20261104/performance-comparison.json).

Ilmiy rol: mavjud LROC balandliklari Apollo ground/work-site heightsni boshqaradi.
LCROSS publication summary simulyatsiya cartridge’ining source label/izohini
boshqaradi; mahalliy suv o‘lchovi, qazib olish yoki yangi rawM3data yo‘q. SELENE/ARES
terrain va yangi rover/sampler props authored. Apollo12 rover ishlatmagan. EVA va
resource koeffitsiyentlari o‘quv game rules, NASA operational procedure emas.

Saqlash: 84/84 himoyalangan player fayli original SHA bilan mos. Editor faqat
EditorPerProjectUserSettings’dagi LunarRescue.TimeStampni yangilagan; tekshirilgan
backupdan asl baytlari tiklandi. 1747editable-before faylidan yo‘qolgan fayl0;
to‘rtta map baselineSHA mos. Oldingi commit qilinmagan ishlar zaxiralangan.

Chegaralar: human playtest0; firewall/network isolation bajarilmadi (zarur lokal
payload paketda va runtime fetch talab qilinmaydi). Rover/sampler oddiy proxy art,
visible gripping hand/driver yo‘q, ayrim world label qorong‘i va map leaderline’lar
band ko‘rinadi. Tasdiqlangan stationary camera PNG haydash animatsiyasi yoki uzoq
traverse silliqligi dalili emas. Competition eligibility tasdiqlanmagan.

Iteratsiya tarixidagi yiqilishlar yashirilmadi: birinchi C++ build helper naming/type
xatolari;25runtimeFAIL strict JSONcoercion va fixture/float masalalari; Mars authored
site LOS; HUD overlap/wrap; actual Apollo camera clearance; legacyBfixtures va
UTF16 verifier reader tuzatildi. Performance navbatidagi noto‘g‘ri helper nomi
alohida qayta chaqirildi. Yakuniy yuqoridagi natijalar faqat final hashga tegishli.

## 2026-11-03 — Departure, carry, crew va LCROSS: yakuniy mahalliy Windows build

**2 400 C++ PASS bajarilishi, 0 FAIL; 16/16 suite o‘tdi.** Bu 2 400 noyob test
yoki inson natijasi emas. 55 ta yangi nusxalangan hisobotdagi aniq PASS satrlari
hisoblandi. 160 ta staged PNG va 8 juft pixel tekshiruvi alohida tasdiqlandi.
[Har bir run va raw hisobot SHA dalili](Docs/Gameplay20261103/evidence.json),
[hisoblash izohlari](Docs/Gameplay20261103/evidence-notes.md).

Yakuniy Win64 Development exe SHA-256:

    5171c1b0a6142dfa4c35e373d731d441ce88eb8d7a41f768779bbeaded52f2f0

Editor module SHA-256:

    c7b0e68232086c6ba8708c79e7256e58976f815888626612f5fce799cd212be0

| Shu yakuniy exe’da bajarilgan qism | PASS bajarilishi |
| --- | ---: |
| Yangi gameplay: uch missiya, NullRHI | 500 |
| Yangi gameplay: uch missiya, DX11 rendered | 536 |
| Yangi 720p/1080p staged frame/UI tekshiruvlari | 110 |
| Eski gameplay, save, suit, selection, diagnostics regressiyalari | 1 128 |
| Haqiqiy loading/travel | 76 |
| Eski HUD: 720p/1080p actual layout | 50 |
| **Jami** | **2 400** |

Rendered 536 va NullRHI 500 orasidagi 36 farq — har missiyada 12 tadan
FOV/pitch viewport corner-projection tekshiruvi. NullRHI loglari viewport
bo‘lmagani uchun ularni ataylab o‘tkazib yuborganini yozadi. NullRHI grafik
performance testi deb ko‘rsatilmagan. 160 layout PASS yuqoridagi jami ichida;
160 PNG yoki 8 pixel juftini yana C++ test soniga qo‘shmadik.

### Nimalar tekshirildi

- Tabiiy 9.4 s departure va skip missiyani bir marta boshlaydi; abort view,
  movement va ko‘rinishni tiklaydi. Cabin vaqtida va pause’da oxygen, mission
  timer, eski radio hamda cargo o‘zgarmaydi; negative/nonfinite delta rad etiladi.
- Cell FP FOV/aspect bounds, pitch, seating, body proxy, photo/pause/notebook
  suppression; committed Apollo sample/core/hardware preview va stow ketma-ketligi.
- Ikki fictional guide’ning materiallari, 40 komponent limiti, range/facing/LOS,
  actual conversation va owned notebook; mission work ustuvorligi va no-reward.
- LCROSS birliklari/koordinata scope, missing/malformed/NoData parser holatlari,
  haqiqiy qiymatlar, noto‘g‘ri javob, uch to‘g‘ri qaror va explicit commit.
  Hidden/unowned/stale callback va repeated key bloklanadi. Apollo SaveGame
  memory roundtrip optional recordni saqlaydi; invalid/absent optional data
  version-one historical cargo’ni bekor qilmaydi. Fresh run recordni tozalaydi.
- Oldingi Moon/Mars picker, power/cable/startup/science/recorder, Apollo
  walkthrough, checkpoints, pause/restart, tools, movement, loading va HUD
  regressiyalari yakuniy exe’da qayta bajarildi.

110 yangi gameplay PNG + 50 oldingi HUD PNG = 160 fresh staged artifact;
1280×720 va 1920×1080, 100/150% text. Bular scripted holatlar, inson sessiyasi emas.
Yakuniy carry ko‘rigida 20 ta PNG ochib tekshirildi: SELENE va ARES, ikki
resolution, 100/150% matn, cell-visible/hidden controls va TP body.
[Ko‘rilgan kadrlarning aniq SHA dalili](Docs/Gameplay20261103/carry-visual-qa.json).

Juft tasvirlarda qo‘l, kamera va world saqlanib, faqat cell yashiriladi.
Markaziy 60% ROI’da max RGB kanal farqi >24 bo‘lgan pixel ulushi kamida 0.12
talab qilindi; sakkiz juftning natijasi 0.9089–0.9886. Bu rendered contribution
ko‘rsatkichi; semantic segmentation, anatomik aniqlik yoki FPS o‘lchovi emas.
Dastlabki fefcbd43 build barcha avtomatik suite’lardan o‘tsa ham qo‘lda ko‘rik
forearm occlusion topdi. Fix’dan keyin besh CPP o‘zgardi va barcha 16 suite
yangi 5171c1b0 exe’da qayta bajarildi. Eski natijalar yangi exe’ga ko‘chirilmagan.

### Build, data va saqlanish dalili

Muhit: Windows 11, UE 5.8.3, MSVC 14.44.35217, SDK 10.0.22621,
Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB RAM. D3D11 loglari oldin/keyin
NVIDIA adapterni tanlaganini ko‘rsatadi. NoUnity/NoPCH va ikki compile action
chegarasi ishlatildi; game debug-symbol link’i o‘chirildi.

To‘liq cook/stage/archive muvaffaqiyatli: 717 processed, 710 cooked va 7
platform skip. Keyingi carry visual fix faqat besh CPP’ni o‘zgartirdi; header,
config, dataset va assetlar o‘zgarmadi. Editor qayta compile: 8 action/72.03 s;
Game qayta compile: 7 action/89.11 s — Succeeded. Yakuniy binary completed cook
bilan StageOnly orqali qayta stage/archive qilindi — BUILD SUCCESSFUL.
Bu incremental C++ build; clean checkout/bo‘sh Intermediate build emas.
Mavjud float, hidden Reset va deprecated skeletal API warninglari saqlandi;
warning-free build da’vosi yo‘q. Dastlabki pointer-type/shadow compile xatolari
va review-helper null dereference loglari raw Reports’da saqlandi va tuzatildi.

98 source/config/dataset input ayni final snapshot bilan mos, built/staged exe
SHA teng, 4 map SHA o‘zgarmagan. Mahalliy LCROSS JSON 3 863 byte:

    40db698b4244ee7f6f28fcb5aa44c7dd3e60177694b34cb18d8bc32a24981105

Runtime JSON/manifest va credits paketda bor, source bilan byte-identical.
Bu publication-facts summary: 155 ±12 kg field-of-view maksimumi va inferred
5.6 ±2.9 mass% qarorlar/barlarni boshqaradi. Raw spectrum yoki Apollo 12’da
suv topilgani da’vo qilinmaydi; mavjud LROC terrain o‘zgarmagan.
[Dataset va ilmiy chegaralar](SCIENCE_AND_SOURCES.md).

Boshlang‘ich 544 save/photo/config snapshot fayli yakunda byte-identical.
Keng baseline engine crash/config va generated stage config’ni ham olgan:
engine tozalagan 180 eski crash config va ikki generated/editor config testlar
tugagach tasdiqlangan original backup’dan tiklandi. O‘yinchi save/fotosida
kutilmagan o‘zgarish bo‘lmadi. [Final bundle/preservation receipt](Docs/Gameplay20261103/bundle-verification.json).

### Bir xil sozlamadagi performance

DX11 RenderOffscreen 1280×720, Medium(1), 60 FPS cap, VSync off, nosound;
har build/scene uchun bir martadan 1 800 frame. Menu: 300 warmup/1 500 sample;
Apollo opening walk: 600 warmup/1 200 sample. [Raw CSV SHA va muhit](Docs/Gameplay20261103/performance.json).

| Scene | Mean FPS oldin → keyin | p99 ms oldin → keyin | Sampled working set MiB oldin → keyin |
| --- | --- | --- | --- |
| Menu | 59.9998 → 59.9999 | 17.5210 → 17.4943 | 553.95 → 529.48 |
| Apollo opening walk | 59.9958 → 59.9992 | 17.7668 → 17.6841 | 604.00 → 603.44 |

Kichik farqlar sababli improvement yoki butun missiyada stable 60 FPS xulosasi
chiqarilmadi. Working set sampled process memory; true peak yoki VRAM emas.
Capture active Apollo opening walk va static menu bilan cheklangan: departure,
rescue cell work va to‘liq missiya performance’i o‘lchanmagan. Ular uchun
rendered functional review bor. Profiling raw exit 777003 completed CSV sharti
bilan tekshirildi; oddiy gameplay test exit’i sifatida qabul qilinmagan.

Asosiy qayta bajarish buyruqlari:

    ./Tools/package_apollo12.ps1 -MaxCompileActions 2
    ./Tools/verify_gameplay_regression.ps1
    python ./Tools/verify_gameplay_bundle.py
    ./Tools/verify_gameplay_tasks.ps1 -Packaged -Review -Width 1280 -Height 720
    ./Tools/verify_gameplay_tasks.ps1 -Packaged -Review -Width 1920 -Height 1080
    ./Tools/profile_mission_menu.ps1 -Label UNIQUE_LABEL -Frames 1800 -Quality 1
    ./Tools/profile_apollo12.ps1 -Label UNIQUE_LABEL -Frames 1800 -Quality 1

Aynan bajarilgan 16-case driver va parametrlar Reports/Gameplay20261103/
final-queue-20261103-075914/queue-summary.json da. Tools’dagi portable nusxa
shu driver bilan bir xil; faqat project root discovery ishlatiladi. Pixel review
uchun development Python/Pillow kerak; packaged o‘yin Python/Unreal Editor talab
qilmaydi. ZIP tarqatishda butun Windows papkasini saqlash kerak.

### Ochiq cheklovlar va keyingi ishlar

- Inson playtesti 0; comprehension yoki mission completion natijalari to‘qilmadi.
- Network uzilgan holda alohida test bajarilmadi; demo bundled lokal data’dan
  foydalanadi, runtime data download/online chatbot talab qilmaydi.
- FP cuffs/round wrist ends va barmoqlar hali sodda; crew world-label kontrasti
  keyingi art polish uchun qoladi. Bu build polished anatomical rig deb taqdim etilmaydi.
- Cabin/PIP/Mira authored fiction; NASA endorsement yoki professional training
  simulyatori da’vosi yo‘q. Space Apps 2026 eligibility tasdiqlanmagan va bu
  product testi bilan belgilanmaydi. Yangi push/release/submission bajarilmadi.
- Keyingi uch ish: measured-terrain A/B planner va live-resource forecast;
  ishtirokchilar bo‘lganda novice playtest; litsenziyasi tekshirilgan art/rig va
  animation polish. [Ustuvorliklar](DEVELOPMENT_PLAN.md).


## 2026-11-02 — HUD, loading, diagnostics va assetlar: yakuniy mahalliy paket

**Yakuniy paketda 1 254 PASS, 0 FAIL.** 18 ta tekshirilgan run: 1 128 funksional,
76 haqiqiy loading/travel va 50 HUD geometriyasi tekshiruvi. Bu takroriy test
bajarilishlari soni; 1 254 noyob test yoki inson natijasi degani emas.
[SHA va buyruqlar dalili](Docs/Feedback2026-11-02/evidence.json).

Win64 Development exe SHA-256:
`b034765110ad30f5a60ffcf1e874ddac679002b16c74f0e9f72bea54017a9fb4`.
Editor module: `251318c8bbb8946f416b76dcef563611778e7dbcb5a343e2a45ec6628a3dcf7e`.
78 ta Source fayli compile oldidagi hashlar bilan bir xil. Built va staged exe
hashlari teng. Editor 4 action/25.89 s, Game 3 action/46.40 s — Succeeded.
To‘liq cook: 710 cooked + 7 platform skip = 717 processed; incremental skip 0.
Cook/stage/archive `BUILD SUCCESSFUL`, UAT exit 0. Bu incremental C++ compile
va to‘liq cook; bo‘sh Intermediate bilan clean-checkout build bajarilgani emas.
Eski float/deprecation warninglari bor, warning-free build da’vo qilinmaydi.

Muhit: Windows 11, UE 5.8.3, MSVC 14.44.35217, SDK 10.0.22621,
Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB RAM.

| Shu final paketda tekshirilgan qism | PASS |
| --- | ---: |
| Jihoz, ground-rock hisoblari va read-only diagnostics: uch missiya | 152 |
| Loading shriftlari: to‘rt manzil, 100/150% | 55 |
| HUD: 720p/1080p, 100/150%, actual Slate geometry | 50 |
| Apollo/SELENE/ARES/menu, walkthrough, suit/polish/guidance/fieldwork/checkpoint | 460 |
| Intro, HUD, pause/restart, POV va Field Lessons regressiyasi | 275 |
| Mission selection va frontend/settings | 71 |
| Mars movement va ledge | 16 |
| Experience va DX11/audio presentation | 99 |
| Haqiqiy loading: 0/100%, monotonic progress, world-time hold | 76 |
| **Jami** | **1 254** |

Bundan tashqari 110 yangi staged UI PNG (50 HUD, 20 loading, 14 frontend,
26 selection) fayl yangiligi/o‘lchami bilan tekshirildi. 6 routed Slate pointer
natijasi alohida saqlandi; ular jismoniy sichqoncha/inson playtesti emas.
Editor -game DX11 orqali 14 yangi jihoz kadri olindi va barchasi ko‘rib chiqildi.
Apollo 39 component/546 instance, SELENE 63/709, ARES 46/594; limit 128/1400.
Bular packaged FPS testi emas. Solar ostki junction oxirgi kadrda aniq
ajralmadi: source OBJ tashqarisida joylashganini tekshirish va clear ray uning
vizual tasdig‘i deb yozilmadi. Shu detal ko‘rigi ochiq cheklov bo‘lib qoladi.

Ko‘rikda topilib tuzatilganlar: 150% loading progress yozuvlari to‘qnashuvi,
optional javob panelining ortiqcha maydoni, yo‘qolgan Route/javob tugmasi
eslatmasi, noto‘g‘ri source masshtabi sabab yer ostida qolgan kosmetik toshlar.
Toshlar haqiqiy mesh bounds bilan o‘lchanadi, target balandlikning 20% qismi
yerga botiriladi; flat/known-normal, invalid va actual-instance ground testlari
qo‘shildi. Bu authored tuning, yangi NASA geologik o‘lchovi emas.

SELENE roverining eski pitch15 holati supported Editor Python orqali
pitch0/yaw15/roll0, Z4.554649 cm ga tuzatildi. To‘liq original map zaxiralandi;
8 g‘ildirakdagi 48 kontakt namunasi (40 distinct source vertex) 1–6.8743 cm
clearance ko‘rsatdi. XY/scale/tags/material/collision va boshqa 1 910 actorning
snapshotlari saqlandi. Faqat `MoonBase.umap` o‘zgardi; map regeneratsiya, ARES
map o‘zgarishi yoki NASA relyefini tahrirlash bajarilmadi. Map SHA:
`d5a76deedb6e6d438834929bc46b59a71bcc5fd14cf30e45dd294cbcc121e1b6`.
Dastlabki ikkita Python API urinishida script save oldida to‘xtadi; uchinchi
urinish muvaffaqiyatli bo‘ldi. Eski Unity compile muammosi module `bUseUnity=false`
bilan tuzatildi; game optimization saqlandi. Xato dalillari Reports’da saqlangan.

Asosiy buyruqlar:

```powershell
./Tools/repair_selene_rover.ps1 -Apply # faqat original pitch15 map uchun; qayta apply rad etiladi
./Tools/package_apollo12.ps1 -MaxCompileActions 2
./Tools/verify_feedback.ps1 -Packaged
./Tools/verify_feedback.ps1 -Packaged -Review -Width 1920 -Height 1080
./Tools/verify_loading_typography.ps1 -Packaged
./Tools/verify_loading_typography.ps1 -Packaged -Review
./Tools/verify_loading.ps1 -Packaged -Graphics
./Tools/verify_nasa_tasks.ps1 -Packaged
./Tools/verify_apollo12.ps1 -Packaged -Regression -Checkpoints -Polish -Suit -Guidance -Fieldwork -Walkthrough
./Tools/verify_mission_selection.ps1 -Packaged
./Tools/verify_frontend.ps1 -Packaged
./Tools/verify_experience.ps1 -Packaged
./Tools/verify_presentation.ps1 -Packaged -Graphics -Audio
./Tools/verify_planet_movement.ps1 -Packaged
./Tools/review_equipment.ps1
```

Frontend/selection/loading/HUD reviewlari 1280×720 va 1920×1080 da bajarildi;
aniq flag/buyruq va raw report hashlari evidence.json’da. NullRHI walkthrough
funksional traversal bo‘lib, grafik FPS yoki inson o‘ynash natijasi emas.

### Bir xil sharoitdagi performance

Oldingi `ee207b31…` va final `b0347651…` uchun DX11 RenderOffscreen 1280×720,
Medium(1), 60 FPS cap, VSync off, nosound; har capture 1 800 frame.
Menu 300 warmup/1 500 sample, Apollo 600 warmup/1 200 sample.
[Hisob va raw fayl SHA-lari](Docs/Feedback2026-11-02/performance.json).

| Sahna | O‘rtacha FPS oldin → keyin | p99 frame ms oldin → keyin | sampled working set MiB oldin → keyin |
| --- | ---: | ---: | ---: |
| Menu | 59.977 → 59.957 | 17.8475 → 17.4734 | 552.8 → 528.8 |
| Apollo opening walk | 59.975 → 59.997 | 18.1036 → 17.8913 | 603.0 → 598.1 |

Har sahna uchun bittadan nazoratli o‘lchov. FPS limitlangan; bu headroom,
barqaror 60 FPS, butun missiya, Mars/SELENE performance yoki animatsiya sifati
kafolati emas. Working set bir soniyada olingan namunalarning maksimumi,
haqiqiy peak yoki VRAM emas. CSV to‘liq va log toza; raw profiling process exit
`777003` o‘zgartirilmagan, exit0 deb ko‘rsatilmagan. 2ed oraliq build o‘lchovlari
finalga ko‘chirilmagan.

### Saqlangan ishlar va chegaralar

Original save/photo/config ro‘yxatidagi **264/264 fayl hash jihatdan saqlandi**,
0 missing, 0 farq. Editor avtomatik o‘zgartirgan bitta
`EditorPerProjectUserSettings.ini` verified original backupdan tiklandi.
[Preservation dalili](Docs/Feedback2026-11-02/preservation.json).

39 asset targetidan 32 tasiga authored scene detail, 5 tasiga existing-tool
storage display qo‘shildi; suit/gloves va Mars ground shader bu passda saqlandi.
39 yangi professional model yuklab/import qilinmadi. Ilmiy NASA datasetlar va
terrain birliklari o‘zgarmadi; yangi hardware/rocks authored content.

Yangi inson playtesti, o‘rganish samarasi, interaktiv PIE, tarmoq uzilgan muhit
sinovi va barcha hardware/ekran/FOV kombinatsiyalari tekshirilmadi. Competition
eligibility bu ish bilan qayta tasdiqlanmadi. Git commit/push, yangi reliz yoki
tanlovga topshirish bajarilmadi. Mahalliy launcher:
`Dist/Apollo12/Windows/LunarRescue.exe`; build/run/controls README va PLAYING’da.
Oldingi e738/2ed runlari o‘z executable guruhlarida saqlandi va final soniga
qo‘shilmadi. Mahalliy raw dalillar `Reports/Feedback20261102/ground-*`,
`compiled-ground-final.json`, `package-ground-final-build.log`,
`selene-rover-repair-20261102-073944-141/`, `equipment-visual-qa-ground-final.json`.

## 2026-11-01 — v0.7.0 relizini tayyorlash

Foydalanuvchi GitHub relizi va NASA guruhiga xulosa yuborishni alohida so‘radi.
`Config/DefaultGame.ini` dagi versiya `0.7.0` bo‘ldi; to‘liq build/cook/stage/archive
`BUILD SUCCESSFUL`, AutomationTool exit 0 bilan tugadi. Game exe yuqorida tekshirilgan
`ee207b31...b28d4b6d` bilan bir xil; compile oldidagi barcha Source hashlar mos.
Qayta cook qilingan paketda **678 ta takroriy tekshiruv, 0 xato**: yangi funksiyalar
275, selection 51, rendered loading 76, Apollo/SELENE/ARES/menu 216, checkpoint 60.
Bu son oldingi 997 ga qo‘shilmaydi; uning qismi yangi paketda qayta bajarildi.

Toza reliz nusxasida barcha 44 runtime fayli (DLL/exe/content containers)
qayta cook qilingan stage bilan SHA-256 orqali solishtirildi. Shaxsiy Saved,
Reports, PDB, test rasmlari va dump/loglar chiqarildi; NASA metadata, barcha
runtime DLLlar, VC++ redist va uchinchi tomon kredit/litsenziyalari saqlandi.
Creditdagi shaxsiy Downloads yo‘li olib tashlandi; ZIP ichidagi credit havolalari
tuzatildi. Release helper uchun 21 fixture va 9 qo‘shimcha mustaqil tekshiruv
bajarildi. Helper ZIP CRC va har bir entry SHA-256 ni tekshiradi; release oldidan
aynan arxivdan chiqarilgan nusxada smoke natijalari alohida receiptga yoziladi.

Buyruqlar: `Tools/package_apollo12.ps1`, `Tools/verify_nasa_tasks.ps1 -Packaged`,
`Tools/verify_mission_selection.ps1 -Packaged`, `Tools/verify_loading.ps1 -Packaged -Graphics`,
`Tools/verify_apollo12.ps1 -Packaged -Regression -Checkpoints`.
Mahalliy dalillar: `Reports/NasaTasks20261101/publish/package-v0.7.0.log`,
`recook-validation-v0.7.0.json`, `release-staging-audit-v0.7.0.json`.
Yangi inson playtesti, network-isolated sinov yoki interaktiv PIE bajarilmadi.

## 2026-11-01 — Intro, HUD, Pause, POV va Field Lessons: final Windows

**997 ta packaged assertion/outcome o‘tdi, 0 xato: 275 yangi + 722 regressiya.**
Yakuniy Win64 Development exe SHA-256:
`ee207b31703b981eeec7cd9637e5a69092bd028a820d2d16d02e6274b28d4b6d`.
Editor va Game build muvaffaqiyatli. Yangi intro uchun to‘liq cook/stage/archive
bajarildi; oxirgi Canvas debrief tuzatishi qayta compile va StageOnly orqali
shu cook bilan paketlandi. Build davomida Source hashlar o‘zgarmagani tekshirildi.
Dastlabki compile urinishlaridagi UWidget member shadowing va TObjectPtr xatolari
tuzatildi; yakuniy loglar muvaffaqiyatli. Eski float/deprecation/Reset warninglari
bor: warning-free build da’vo qilinmaydi.

Muhit: Windows 11 Home 10.0.22631, Unreal Engine 5.8.3, MSVC 14.44.35217,
Windows SDK 10.0.22621; Ryzen 5 6600H, RTX 3050 Laptop/AMD 660M, 16 GB RAM.

| Yakuniy executable’da tekshirilgan qism | Assertion/outcome |
| --- | ---: |
| Intro, uch missiya HUD/pause/restart, POV, Field Lessons | 275 |
| Apollo, SELENE, ARES gameplay va menu travel | 216 |
| To‘liq oddiy yurish orqali Apollo walkthrough | 2 |
| Suit, polish, guidance, fieldwork | 182 |
| Checkpoint, alohida write/read jarayonlari | 60 |
| Mission selection va frontend/settings | 71 |
| Mars movement va recorder ledge | 16 |
| Expedition experience va DX11/audio presentation | 99 |
| Rendered loading: 0/100, monotonic progress, world time hold | 76 |
| **Jami** | **997** |

Walkthrough fixed 1/60 simulyatsiya qadami va odatiy movement qiymatlari bilan
896.81 simulyatsiya soniyasi, 1 237.50 horizontal metr, 815 trail nuqtasida
bajarildi. Samples 3/3, hardware mask 7, ALSEP mask 3. Bu NullRHI funksional
traversal: grafik FPS yoki inson playtesti emas.

720p va 1080p uchun **156 ta yangi staged PNG**: yangi intro/HUD/pause/POV/lessons
130; mission-selection 26. Fayl yangiligi, o‘lchami va fixture holati tekshirildi.
**30 ta routed Slate pointer tekshiruvi**: 24 pause action va 6 mission card.
Home hover tasvirlari hit-testdan keyingi UButton hover-delegate fixture’lari;
ular physical mouse hover sinovi sifatida sanalmaydi. Frame’lar va lesson
javoblari scripted; inson natijalari emas. Tanlab ko‘rilgan tasvirlarning
SHA256 dalillari `independent-visual-qa.json` va `independent-final-visual-qa.json`da.
Apollo win/loss ekranidagi lesson summary ko‘rinmasligi tuzatildi; yangi
packaged win150/loss tasvirlari bilan tasdiqlandi. 1080p hold/pause150/win150
ham bevosita ko‘rildi. Hamma 156 tasvirning inson tomonidan ko‘rilgani da’vo
qilinmaydi.

### Grafik o‘lchov

Oldingi exe `5afb3476db95cf844059a0ae8cb6cf1da8bfb323897a75a91e9e5f5e2f3bbeeb`
va yuqoridagi final exe solishtirildi. DX11, RenderOffscreen, 1280x720, Medium
(quality 1), 60 FPS cap, VSync off, nosound. Har capture 1 800 frame; menu uchun
300 warmup chiqarilib 1 500 sample (~25 s), opening walk uchun 600 warmup
chiqarilib 1 200 sample (~20 s). Har holat uchun bir martalik juft capture.

| Sahna | Oldin/o‘rtacha FPS | Keyin/o‘rtacha FPS | Oldin/keyin p99 ms | Oldin/keyin sampled working set MiB |
| --- | ---: | ---: | ---: | ---: |
| menu | 59.818 | 59.978 | 17.845 / 17.693 | 531.85 / 528.80 |
| opening-walk | 59.959 | 59.985 | 17.669 / 18.075 | 595.98 / 600.84 |

Opening walk p99 0.407 ms oshdi, menu p99 0.152 ms kamaydi. Cap ostidagi qisqa
capture’lar butun missiya uchun stable 60 FPS, GPU headroom yoki animatsiyaning
perceived smoothness’ini isbotlamaydi. Working set — bir soniyalik process
sample’lari; VRAM yoki haqiqiy peak emas. Thermal/background sharoitlar to‘liq
nazorat qilinmagan. Profiler raw exit `777003` qaytardi; helper’lar crash logini
emas, to‘liq 1 800-frame CSV va semantic natijani tekshirib capture’ni qabul qildi.

### Buyruqlar va dalillar

```powershell
./Tools/package_apollo12.ps1
./Tools/package_apollo12.ps1 -StageOnly
./Tools/verify_nasa_tasks.ps1 -Packaged
./Tools/verify_nasa_tasks.ps1 -Packaged -Review
./Tools/verify_nasa_tasks.ps1 -Packaged -Review -Width 1920 -Height 1080
./Tools/verify_apollo12.ps1 -Packaged -Regression -Checkpoints -Polish -Suit -Guidance -Fieldwork -Walkthrough
./Tools/verify_mission_selection.ps1 -Packaged
./Tools/verify_mission_selection.ps1 -Packaged -Review
./Tools/verify_mission_selection.ps1 -Packaged -Review -Width 1920 -Height 1080
./Tools/verify_frontend.ps1 -Packaged
./Tools/verify_planet_movement.ps1 -Packaged
./Tools/verify_experience.ps1 -Packaged
./Tools/verify_presentation.ps1 -Packaged -Graphics -Audio
./Tools/verify_loading.ps1 -Packaged -Graphics
./Tools/profile_mission_menu.ps1 -Label nasa-before
./Tools/profile_mission_menu.ps1 -Label nasa-after
./Tools/profile_apollo12.ps1 -Label nasa-before -Frames 1800 -Quality 1 -Width 1280 -Height 720 -FpsCap 60
./Tools/profile_apollo12.ps1 -Label nasa-after -Frames 1800 -Quality 1 -Width 1280 -Height 720 -FpsCap 60
```

Intro `Tools/import_intro.py` orqali Unreal AssetImportTask bilan import qilindi;
`NASA_INTRO_IMPORT_OK` va real Texture2D yuklanishi tekshirildi. Runtime import
va internet talab qilinmaydi. SourceArt original JPEG checksum saqlandi.
Manba/cook maplari qayta generatsiya qilinmadi; Content’da faqat yangi intro
texture qo‘shildi. Credits va lokal dataset/intro manifestlari paketga kiritildi.

Mashina dalili: `Reports/NasaTasks20261101/validation.json`; fresh nusxalar
`final-functional-results/`, har review’ning manifest/run-summary fayllari va
`editor-build.log`, `game-build.log`, `package.log`, `package-final.log`.
131 original save/photo/config fayli hash bilan mos: 130 o‘zgarmagan, avtomatik
yozilgan bitta Editor preference zaxiradan qaytarildi. Dastlabki dirty ish
`Backups/NasaTasks-20261101`da saqlangan.

### Chegaralar va keyingi ishlar

- Inson playtesti: **0**; oldingi norasmiy feedback yangi tadqiqot sifatida sanalmadi.
- Tarmoq uzilgan/firewall-blocked sinov bajarilmadi: bu jarayonda admin token yo‘q.
  Lokal paket asset/metadata yuklanishi tekshirildi; bu network isolation testi emas.
- Editor testlari standalone `-game`; interaktiv PIE tekshiruvi bajarilmadi.
- Apollo 12 LROC 2 m ground/collision va worksite balandliklari saqlandi; SELENE/ARES
  authored terrain. Dataset pipeline bu UI ishida qayta ishga tushirilmadi.
- A/B route planner, forecast/live resource taqqoslash, engineering campaign,
  south-pole site tanlovi va companion alohida backlog. Quizzes ularni almashtirmaydi.
- 704 accessible tarix yozuvi olindi; eski media metadata/context bo‘yicha tasniflandi.
  To‘rtta voice va eski illustratsiya mazmuni ko‘rilgan deb da’vo qilinmaydi.
- 2026 competition eligibility va prior-work reuse tasdiqlanmagan; rasmiy sahifalar
  qayta olish urinishlari 502/TLS xatosi bilan tugadi. Qoidalar yoki demo limiti
  taxmin qilinmadi. Git sanalari/litsenziya o‘zgartirilmadi, remote push/release
  yoki tashkilotchiga xabar yuborilmadi.

Eng muhim keyingi uch ish: haqiqiy relyefga tayangan A/B planner va debrief;
ishtirokchilar bo‘lganda novice playtest; 2026 challenge/reuse/litsenziya shartlarini
rasmiy tasdiqlash. Tafsilot: [NASA_TASKS_UPDATE](NASA_TASKS_UPDATE.md),
[SCIENCE_AND_SOURCES](SCIENCE_AND_SOURCES.md), [COMPETITION_READINESS](COMPETITION_READINESS.md).

## 2026-11-01 - Mission Selection, final Windows

**502 packaged assertion/outcome o'tdi, 0 xato.** Exe SHA-256
`5afb3476db95cf844059a0ae8cb6cf1da8bfb323897a75a91e9e5f5e2f3bbeeb`. Editor/Game compile va StageOnly stage/archive ExitCode=0;
avvalgi cook qayta ishlatildi. Selection51, uchala destination/restart/Continue,
settings, checkpoint, polish, WASD+arrows va rendered loading qamraldi.
720p/1080p uchun 26 staged PNG va 6 real Slate hit-test pointer check olindi.
84 original fayl hash bilan saqlandi; Editor preference restore soni 1.

Buyruqlar: `verify_mission_selection.ps1 -Packaged`, `verify_frontend.ps1 -Packaged`,
`verify_apollo12.ps1 -Packaged -Regression -Checkpoints -Polish`,
`verify_planet_movement.ps1 -Packaged`, `verify_loading.ps1 -Packaged -Graphics`,
`verify_mission_selection.ps1 -Packaged -Review` (1280x720 va 1920x1080).
Profil: `profile_mission_menu.ps1 -Label before` / `-Label after-input`.
Inson sessiyasi0; sparse renderlar perceived smoothness yoki FPS testi emas.
Tafsilot, failure/repair tarixi, performance va chegaralar:
[MISSION_SELECTION_UPDATE.md](MISSION_SELECTION_UPDATE.md).
Mashina dalili: `Reports/MissionSelection20261031/validation.json`.

## 2026-10-29/30 — Expedition Experience, final Windows

**591 packaged assertion/outcome o‘tdi, 0 xato.** Final exe SHA-256 `1aad851146a01e143e6a8464b24694d1c9835a0e37d26955fdf61fcacff9a957`.
Editor/game build va yakuniy BuildCookRun ExitCode=0. To‘liq oddiy yurish: 896,81 sim s,
1 237,50 horizontal m, 815 trail points. 720p Medium, 60-cap qisqa profilda
59.993 → 59.997 FPS; butun missiya performance kafolati emas.
7 sahna, 12 fieldwork PNG va 120 synthetic motion frame; inson sessiyasi0, ishtirokchilar yo‘q.
Player/science fayllari o‘zgarmagan; Editor metadata/staging manifest farqlari alohida qayd etilgan.
To‘liq buyruqlar, failure/repair tarixi va chegaralar: [EXPEDITION_EXPERIENCE.md](EXPEDITION_EXPERIENCE.md).
Mashina qaydi: `Reports/ExpeditionExperience20261029/validation.json`.

## 2026-10-29 - Expedition presentation (local)

Windows Development build/cook/stage passed. Packaged assertions: **512 passed,
0 failures**, including actual normal-movement walkthrough, checkpoint fresh
process restore, new presentation/audio checks, Moon/Mars, frontend and loading.
Editor and packaged render reviews found and fixed missing instanced material
usage and 150% debrief row overlap. Original save/photo/config/science hashes:
234 checked, unchanged. Two new generated review images archived separately.

Matched 720p Medium DX11 offscreen opening profiles: 59.993 FPS before and
59.997 after; 1% low 55.750 and 56.552. Both use 3600 frames, exclude startup
600, cap60 and disable sound. These are not full-mission or cross-hardware FPS
guarantees. The headless walkthrough's 896.81 simulated seconds/1237.50m are
functional evidence, not graphical performance.

Details, commands and limits: [EXPEDITION_POLISH.md](EXPEDITION_POLISH.md).
Machine-readable evidence: `Reports/ExpeditionPolish20261029/validation.json`.
Human playtests: **not run**, protocol and blank form prepared. No push/release.

## Historical test hisoboti — 2026-09-21

Quyidagi baseline bo‘limlari oldingi audit holatini saqlaydi. Keyingi source tuzatishi uchun alohida “Menu maintenance” bo‘limi fayl oxirida berilgan.

Baseline HEAD: `20b4b4be2b53f0dc08585d259bc7ee42d6493e44`. Gameplay/source/map o‘zgarmadi. Test boshlanishi: **07:01:27 +05:00**. Bu yangi NASA-data build testi emas; mavjud v0.2.0 binary qayta tekshirildi. Remote push, yangi release yoki submission bajarilmadi.

## Muhit

- Windows; UE 5.8.2 CL 56702186; Win64 Development editor target.
- AMD Ryzen 5 6600H; 16 GB RAM; tanlangan DX11 adapter NVIDIA GeForce RTX 3050 Laptop GPU.
- O‘rnatilgan VS 2022 Community MSVC kataloglari: 14.29.30133 va 14.44.35207; Windows SDK: 10.0.22621.0 va 10.0.26100.0. Ushbu incremental build yangi compile action bajarmagani sabab yangi compiler invocation tasdiqlanmaydi.
- Python 3.11; Git LFS mavjud. `git lfs fsck` OK; 141 tracked Content faylida unresolved LFS pointer yo‘q.
- Tested binary: `Dist/Windows/LunarRescue/Binaries/Win64/LunarRescue.exe`; SHA-256 `a52838d9196f26469e146d6c4bc21f6e44bb9b3a1ac55c65e2b752a414cb53f0`.

## Bajarilgan buyruqlar

Loyiha ildizidan PowerShell:

```powershell
git status --short --branch
git lfs fsck
& 'D:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat' LunarRescueEditor Win64 Development -Project=D:\LunarRescue\LunarRescue.uproject -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2
& .\Tools\verify_planets.ps1 -Packaged -Graphics -Walkthrough
```

Editor target: **Succeeded, exit 0, 4.03 s**, “Target is up to date”, **0 actions**. Bu clean rebuild emas. Packaging bu auditda qayta bajarilmadi. Standalone testlari haqiqiy mavjud EXEni ishga tushirdi; statik source tekshiruvi bilan almashtirilmadi.

| Suite | PASS satrlari | Natija |
| --- | ---: | --- |
| menu | 7 | exit 0, 0 failures |
| moon-gameplay | 52 | exit 0, 0 failures |
| mars-gameplay | 56 | exit 0, 0 failures |
| mars-movement | 9 | exit 0, 0 failures |
| mars-ledge | 3 | exit 0, 0 failures |
| moon-walk | 23 | exit 0, 0 failures |
| mars-walk | 27 | exit 0, 0 failures |

PASS soni mustaqil test-case soni degani emas: walkthroughda route step satrlari ham sanalgan. Gameplay suite obyektlar orasida teleport/API chaqiruvlaridan foydalanadi; walkthrough spawn nuqtasidan normal movement/collision bilan yuradi. Menu travel test metod chaqiruvlarini tekshiradi, haqiqiy mouse hitbox yoki barcha tugmalarni emas.

## Grafik baseline

Ketma-ket, sound enabled, `-dx11 -RenderOffscreen -ResX=1280 -ResY=720`, cap 60 FPS, texture streaming pool 384 MB, shadow quality 2. Log tanlangan NVIDIA adapterni, shadow profilini va 384 MB final poolni tasdiqlaydi. NullRHI ishlatilmadi.

| Missiya | Active traversal s | Qolgan budget s | Frames | Mean ms | P95 ms | Mean FPS | Peak process physical MiB |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Moon | 127.65 | 172.42 | 7657 | 16.67 | 16.81 | 60.00 | 707.3 |
| Mars | 353.99 | 126.08 | 21239 | 16.67 | 16.82 | 59.99 | 530.1 |

Ikkala yurish ham optional recorder va lander return bilan yakunlandi. Counter boshlanishi/tick chegarasi tufayli mission seconds bilan qolgan budget yig‘indisi nominal capacityga aynan teng emas. Bu mavjud test instrumentationidir, kelajak debrief uchun shared active timer talab qilinadi.

Bu bitta mashinadagi capped offscreen o‘lchov; “barcha sharoitda stable 60 FPS”, GPU time, VRAM sarfi yoki foydalanuvchi sezgan silliqlik isboti emas. Peak physical memory process working-set ko‘rsatkichi, butun tizim xotirasi emas. Yangi gameplay bo‘lmagani sabab before/after performance taqqoslash yo‘q.

## Qabul mezonlari qamrovi

| # | Mezon | Holat va chegarasi |
| --- | --- | --- |
| 1 | Moon/Mars selection | PASS: menu travel; mouse kartalar QA qilinmagan |
| 2 | Start/pause/resume/restart/exit | Qisman: gameplay va menu travel pass; qo‘lda Alt+F4 tekshirilmagan, test process exit boshqa narsa |
| 3 | Power restoration | PASS: cell, cable, hold, reboot, terminal gating, pause/audio |
| 4 | Science/recorder | PASS: Moon/Mars, relay va recorder flags; ilmiy dataset yo‘q |
| 5 | Planningdan gameplayga route | Bajarilmagan: planner yo‘q |
| 6 | Dataset units/coordinates | Label baholandi; import/round-trip testi bajarilmagan |
| 7 | NoData/missing dataset | Bajarilmagan; NoData hali tasdiqlanmagan |
| 8 | Flat slope fixture | Bajarilmagan: slope pipeline yo‘q |
| 9 | Known slope fixture | Bajarilmagan: slope pipeline yo‘q |
| 10 | Pause timer/resource | Mavjud oxygen/hold/reboot/subtitle pass; yangi session timer yo‘q |
| 11 | Restart telemetry reset | Bajarilmagan: oddiy session telemetry yo‘q; map restartning o‘zi pass |
| 12 | Debrief/session mosligi | Bajarilmagan: yangi debrief yo‘q |
| 13 | A/B traversal | Bajarilmagan; faqat mavjud authored Moon/Mars traversal pass |
| 14 | Optional vazifasiz win | PASS gameplay assertion; yangi NASA route traversal emas |
| 15 | Mars regressiya | Mavjud 56 gameplay, movement, ledge va full walk pass |
| 16 | Packaged metadata/assets | Mavjud core/audio/visual assertions pass; yangi source metadata mavjud emas |
| 17 | Offline demo | Tarmoq uzilgan muhitda test bajarilmagan; local package ishlashi offline proof emas |

Zero/depletion mavjud assertionda tekshirildi. Negative/nonfinite capacity, arbitrary negative delta va yangi predictor chegaralari alohida avtomatik fixture bilan tekshirilmagan. Multi-resolution visual QA, accessibility review, fresh-user playtest va missing-dataset fallback ham bajarilmagan.

## Dalillar va takrorlash

Raw local loglar: `Reports/education-audit/editor-build.log`, `baseline-suite.log`, `*checks.txt`; engine loglar `Reports/package-*.log`. Bu papkalar ignored. Commitga tayyor ixcham dalil: [baseline-evidence.json](Docs/Audit2026-09-21/baseline-evidence.json), unda raw test matnlari, hashlar va source snapshot hashlar saqlanadi.

Hujjatlar yakunida `git diff --check` exit 0 bilan tugadi; yangi Markdown fayllaridagi local havolalarning manzillari mavjud. Yakuniy status [evidence](Docs/Audit2026-09-21/baseline-evidence.json)da. Bular C++ build yoki gameplay test o‘rnini bosmaydi.

O‘ynash: release ZIPni to‘liq extract qilib `LunarRescue.exe`; Unreal Editor kerak emas. Windows-native build brauzerda ochilmaydi. Source build/controls [README](README.md)da. Yangi package kerak bo‘lganda `Tools/package_windows.ps1`; bu auditda qayta ishga tushirilmagan.

## Menu maintenance — auditdan keyingi tekshiruv

`LunarGame.cpp`, `LunarGame.h`, `DefaultInput.ini`dagi menu click tuzatishi uchun editor build qayta bajarildi. Yuqoridagi Build.bat buyrug‘i: **Succeeded, exit 0, 5 actions, 74.37 s**; C++ va generated code compile/link qilindi. Toolchain: MSVC 14.44.35217, SDK 10.0.22621.0. Eski HUD float literal joylarida C4305 warninglar bor; build warning-free deb ko‘rsatilmaydi.

Regressiya buyrug‘i: `Tools/verify_planets.ps1` (editor executable, NullRHI, nosound). Raw loglar: `Reports/education-audit/menu-fix-build.log`, `menu-fix-suite.log`. Bu grafik performance o‘lchovi emas. Yangi mouse hitboxning qo‘lda visual/click tekshiruvi va yangi packaged build bajarilmagan; [UX protokoli](MAINTENANCE_FOLLOWUP.md) alohida berilgan.

**Natija: 5/5 suite exit 0, barcha hisobotlarda 0 failures.** Menu 7, Moon gameplay 50, Mars gameplay 54, Mars movement 9, Mars ledge 3 PASS satri. Sound o‘chiq bo‘lgani uchun audio assertionlar oldingi grafik baselinega qaraganda kamroq. Full walkthrough qayta bajarilmadi: bu o‘zgarish terrain, movement yoki resource hisobiga tegmagan. Dalil: [menu-maintenance-evidence.json](Docs/Audit2026-09-21/menu-maintenance-evidence.json). `git diff --check` exit 0; o‘zgargan hujjatlarning local havolalari mavjud.
