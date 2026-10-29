# Expedition Experience — yakuniy mahalliy hisobot

Ish 2026-10-29 kuni boshlandi, tekshiruvlar 2026-10-30 kuni yakunlandi (Asia/Tashkent).
Bu mahalliy ishlab chiqish buildi; yangi GitHub relizi yoki tanlov topshirig‘i emas.
Boshlang‘ich commit: `bc22df2b40116060b9dbaf3b5caee3e7e388ac63`. Avvalgi commit qilinmagan loading va presentation ishlari
`Backups/ExpeditionExperience-20261029-232227` ichida saqlanib, yangi o‘zgarishlar bilan birga qoldirildi.

## Amalga oshirilgan imkoniyatlar

1. **Skafandr va qo‘lqop:** yeng torayishi, kaft qalinligi va son/shin uchlari qayta ishlangan.
   36 ta suyak va asbob tutish pivotlari saqlangan. Birinchi shaxsdagi shim uchlari kamera
   ortiga davom ettiriladi; tizza/etik nishonlari, kapsula va uchinchi shaxsning haqiqiy hip
   nuqtalari bundan mustaqil. Bu kamera uchun badiiy yechim, anatomik skafandr modeli emas.
2. **My Expedition:** I orqali albomni ochib, My Expedition / map tanlanadi. Shaxsiy yurish
   yo‘li oltin, qayta tiklangan tarixiy marshrut ingichka cyan chiziqda. Qayd qilingan foto
   va yig‘ilgan yuk ish joyi belgilariga bog‘lanadi; ular shutter pozitsiyasi deb ko‘rsatilmaydi.
   Jurnal xaritasi ham shaxsiy izni ko‘rsatadi. 150% matnda kontent scroll orqali ochiladi.
3. **Etik izlari:** avval ham mavjud tread materialidan foydalaniladi. Endi izlar posed chap/o‘ng
   etikka moslanadi, prop, havo, pauza va teleport holatlari tekshiriladi. Eng ko‘pi 160 decal
   komponenti qayta ishlatiladi; restart/reload eski izlarni tozalaydi.
4. **Yakuniy yuk sahnasi:** g‘alaba va checkpoint yozish muvaffaqiyatidan keyin 8,8 soniyalik
   badiiy stowage ko‘rsatiladi. Faqat yig‘ilgan namunalar va Surveyor qismlari chiqadi.
   Work tugmasi, chap bosish yoki Enter o‘tkazadi; Esc pauza qiladi. Ochiq sozlama/albom
   sahnani muzlatadi va fon ortidagi input skip qilmaydi. Oldingi kamera qaytariladi,
   tugallangan save ochilganda sahna qayta boshlanmaydi. Parvoz simulyatsiyasi qo‘shilmagan.
5. **Playtest tayyorgarligi:** 3–5 yangi o‘yinchi uchun oldin/keyin savollar, kuzatuvlar va
   bo‘sh forma tayyor. Foydalanuvchi ishtirokchilar hozir yo‘qligini bildirdi. **Inson sessiyalari: 0.**

## Ma’lumot va saqlash

Marshrut mavjud world-centimetre koordinatalarida yoziladi; odatiy display oralig‘i 150 cm,
eng ko‘pi 1 024 nuqta. Limitda iz soddalashtiriladi, birinchi/oxirgi nuqta va uzilishlar
saqlanadi. Teleport va reload bo‘shliqlari ulanmaydi. Masofa sampled polylinedan qayta
hisoblanmaydi; u mustaqil gorizontal movement telemetrydan olinadi. Yangi checkpoint
maydonlari ixtiyoriy: eski v1 savelar o‘qiladi va yetishmagan iz "partial trace" sifatida belgilanadi.

NASA/LROC balandliklari, zamonaviy orbital xarita va manbali tarixiy marshrut mavjud
joylashuvlarini saqladi. My Expedition haqiqiy ma’lumotga bog‘langan hududda o‘yinchining
yo‘lini ko‘rishga xizmat qiladi. Yangi NASA dataset yuklanmadi; relyef qayta yaratilmagan.
Shim, iz, qo‘lqop, rack va stowage harakati badiiy elementlar. Nishab astronavt uchun
mutlaq xavfsizlik reytingi sifatida talqin qilinmaydi. NASA tasdig‘i yoki hamkorligi da’vo qilinmaydi.

## Build va ishga tushirish

Muhit: Unreal Engine 5.8.3, MSVC 14.44, Windows SDK 10.0.22621.0; Windows Development.
Editor/game kompilyatsiyasi, cook/stage/archive muvaffaqiyatli. AutomationTool yakuniy ExitCode=0.
Editor orqali `Tools/import_suit_refinement.py` bajarilib, uassetlar qo‘llab-quvvatlangan importer
bilan yozildi (0 commandlet errors/warnings); binary fayllar matn sifatida tahrirlanmadi.

- Tayyor o‘yin: `Play Lunar Rescue.cmd` yoki `Dist/Apollo12/Windows/LunarRescue.exe`.
- Build: `./Tools/package_apollo12.ps1`.
- Faqat tayyor binarini stage qilish: `./Tools/package_apollo12.ps1 -StageOnly`.
- Suite: `./Tools/verify_experience.ps1 -Packaged`.
- Regressiya: `./Tools/verify_apollo12.ps1 -Packaged -Regression -Walkthrough -Suit -Polish -Guidance -Fieldwork -Checkpoints`.
- Qo‘shimcha Mars: `./Tools/verify_planet_movement.ps1 -Packaged`.
- Render: `./Tools/verify_experience.ps1 -Packaged -Review`.
- Harakat: `./Tools/review_suit_motion.ps1 -Packaged -Label <fresh-label>`.

Package skriptidagi past xotira flaglarini saqlang: NoPCH/NoSharedPCH, bitta action;
Windows game uchun NoDebugInfo/NoLinkerDebugInfo. Editor startup SDK validatsiyasi
UBT mutexini kutishi mumkin; Editor testini C++ build bilan bir vaqtda boshlamang.

Final executable SHA-256: `1aad851146a01e143e6a8464b24694d1c9835a0e37d26955fdf61fcacff9a957`.

## Bajarilgan avtomatik tekshiruvlar

| Guruh | O‘tdi | Xato |
|---|---:|---:|
| apollo12-checks | 57 | 0 |
| gameplay-checks | 50 | 0 |
| mars-gameplay-checks | 54 | 0 |
| menu-travel-checks | 7 | 0 |
| apollo12-walkthrough | 2 | 0 |
| suit-checks | 32 | 0 |
| polish-checks | 63 | 0 |
| guidance-checks | 46 | 0 |
| fieldwork-checks | 39 | 0 |
| checkpoint-checks | 51 | 0 |
| checkpoint-write-checks | 2 | 0 |
| checkpoint-read-checks | 7 | 0 |
| experience-checks | 62 | 0 |
| frontend-checks | 20 | 0 |
| loading-checks | 20 | 0 |
| presentation | 33 | 0 |
| audio | 34 | 0 |
| mars-movement | 9 | 0 |
| mars-ledge | 3 | 0 |
| **Jami packaged assertions/outcomes** | **591** | **0** |

Barcha shu jadvaldagi final natijalar aynan yuqoridagi executable bilan olingan.
NullRHI natijalari funksional tekshiruvdir, grafik performance isboti emas.

Oddiy controller inputi va collision bilan to‘liq missiya: 896,81 simulyatsiya soniya,
1 237,50 gorizontal metr, 3/3 namuna, 3/3 Surveyor qismi va ikkala ALSEP vazifasi.
815 ta haqiqiy yurish nuqtasi; sampled display uzunligi 1 234,42 m. Bu avtomatik
walkthrough, inson completion vaqti emas; optional coating study talab qilinmagan.

Render dalillari: 7 ta yangi sahna, 12 ta fieldwork screenshot va 120 ta synthetic
harakat kadri. Map150, stowage/subtitle va tread ko‘rinishlari tekshirildi. Harakatning
8 vakil kadri va asbobning contact/lift/core/cable kadrlari ko‘rildi; asosiy grip saqlangan.
Ushbu kadrlar barcha oraliq finger collision yoki inson qulayligini to‘liq isbotlamaydi.

Dastlabki kompilyatsiyada noto‘g‘ri API nomi va class a’zosini yashirgan lokal nom
aniqlanib tuzatildi. Regressiya eski polish fixture sahnani yopmasdan boshqa synthetic
bosqichga o‘tganini topdi: fixture tozalandi, rebound B bilan haqiqiy tool ochilishi
tekshiruvi saqlangan va finalda o‘tgan. Pauza Enter/click, subtitr va ochiq panel ortidagi
skip himoyalari test bilan qoplangan. Build flagi mos kelmagan bitta urinish va build
mutexini kutgan Editor startup to‘xtatilib, yakuniy buyruqlar ketma-ket qayta bajarilgan.

## Bir xil sharoitdagi FPS juftligi

Ryzen 5 6600H, RTX 3050 Laptop GPU, taxminan 16 GB RAM; DX11, 1280×720,
Medium/quality 1, VSync off, 60 FPS cap, audio off, rendered offscreen.
Har safar 3 600 kadr, 600 warmup chiqarilgan; 3 000 kadr / taxminan 50 s tahlil qilingan.
O‘lchov paytida boshqa game/editor/compiler/shader worker aniqlanmagan.

| Ko‘rsatkich | Oldin | Keyin |
|---|---:|---:|
| O‘rtacha FPS | 59.99269 | 59.99685 |
| 1% low FPS | 54.44073 | 56.37544 |
| P99 frame ms | 17.6059 | 17.5515 |
| Eng katta frame ms | 29.0284 | 18.2106 |
| Sampled working set MiB | 598.89 | 600.95 |

Bu bitta capped opening-route juftligi; tezlashishning statistik isboti yoki butun
missiya/barcha qurilmada barqaror 60 FPS kafolati emas. Sampled working set haqiqiy
memory peak yoki VRAM emas. CSV auto-stop jarayonining qayd qilingan exit kodi ikkala
captureda 777003; fresh CSV/kadr soni va log validatsiyasi o‘tgan. Uni exit-0 funksional
test deb hisoblamadik. Dalillar `Reports/Performance/experience-before-20261029` va
`experience-after-20261030` ichida.

## Fayllar va ma’lumot butunligi

Boshlang‘ich 457 fayldan 454 tasi SHA-256 bo‘yicha o‘zgarmagan. **Barcha 123 ta
tekshirilgan player save/foto/game config va Apollo manba fayllari o‘zgarmagan.** Uchta
farq: EditorPerProjectUserSettings.ini va ikki staged-build manifesti. Editor fayli startup/importdan keyin o‘zgargan; u game/player sozlamasi sifatida talqin qilinmadi.
Oldingi Editor zaxirasi boshlang‘ich hashga mos kelmagani sabab eski sozlama taxmin bilan
ustidan yozilmadi. Har bir farq `preservation-results.json`da aniq qayd etilgan.

Render testining logda kelib chiqishi ko‘rsatilgan ikkita yangi coating fotosi alohida
`generated-test-evidence` ichiga arxivlandi. Asl fotosuratlar ko‘chirilmagan. Toza playtest
nusxasi `Dist/Playtests/P07` ichida tayyor: 88 runtime fayl, final executable bilan mos hash,
Saved/Reports papkalari yo‘q, bo‘sh forma bor. P07 texnik tayyorlash kodi, inson ishtirokchisi emas.
Paketdagi barcha 53 ta player fayli paketlashdan oldingi to‘liq zaxira bilan ham byte/hash bo‘yicha mos.
Tayyorlashning o‘zi inson testi emas; nusxa avtomatik ishga tushirilmagan.

## Muhim fayllar va qolgan chegaralar

- `LunarTrail.cpp/.h`, `LunarTrailMap.cpp/.h`: yozuv, cheklangan xotira va xarita.
- `LunarCheckpoint.h/.cpp`, `LunarApolloSave.cpp`: additive save va validatsiya.
- `LunarFootprints.cpp`: posed tread va qayta ishlatiladigan pool.
- `LunarCargoFinale.cpp/.h`: alohida kamera, maskka mos cargo, pause/skip.
- `LunarSuit.cpp`, `LunarRig.cpp`, `Tools/prepare_suit.py`, suit source/uassetlar: shakl va pose.
- `LunarExperienceChecks.cpp`, verifierlar: test va synthetic render.
- `PLAYTEST_GUIDE.md`, `Tools/fixtures/expedition-playtest-form.md`: bajarilmagan inson sinovi.

Model hali stilizatsiyalangan; yaqin work-camera bilaklari uzun ko‘rinishi mumkin.
Ayrim uchinchi shaxs etiklari pastki HUD chegarasiga yaqinlashadi. Anatomik/motion-capture
aniqlik va inson comforti tekshirilmagan. OS tarmog‘ini uzib sinash bajarilmagan; yangi
runtime imkoniyatlari lokal cooked assetlar bilan ishlaydi va network API qo‘shmaydi.

Eng muhim keyingi uch tekshiruv: haqiqiy 3–5 novice sessiyasi; butun missiyani turli
hardwareda render performance bilan o‘lchash; kengroq FOV/resolution va comfort playtesti.
Competition eligibility bu mahsulot iteratsiyasi bilan qayta tasdiqlanmagan; avvalgi
`COMPETITION_READINESS.md`dagi cheklovlar o‘z kuchida. Ushbu tekshiruvlar source GitHub’ga yuborilishidan oldin olingan.

2026-10-30 nashr scope: foydalanuvchining alohida so‘roviga binoan source va Git LFS assetlar
GitHub main branchiga commit/push uchun tayyorlandi. Yangi Windows relizi yoki tanlov submissioni
ushbu source yuklash scope’iga kiritilmagan.
