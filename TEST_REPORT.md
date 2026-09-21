# Test hisoboti — 2026-09-21

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
