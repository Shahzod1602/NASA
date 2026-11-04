# Test reports

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
