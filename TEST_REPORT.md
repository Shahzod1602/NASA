# Test reports

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
