# Mission selection yangilanishi - 31 October / 1 November 2026

## Amalga oshirilgan oqim

Explore the Moon va Explore Mars avval xaritani bevosita ochar edi. Endi ular
Mission Selection sahifasini ochadi. Oyda mavjud Apollo 12 historical exploration
va SELENE fictional rescue, Marsda ARES fictional prototype ko'rsatiladi.
Yangi missiya mazmuni to'qilmagan.

Karta bosilganda missiya tanlanadi. Start mission yoki Enter tasdiqlagandan keyin
loading boshlanadi. Tanlovsiz start bloklangan. Hover boshqa missiyani tanlamaydi.
Esc/Back asosiy menyuga qaytaradi. 1/2, Enter va chap/o'ng klavishlar bir xil
tekshiriladigan oqimdan foydalanadi. Continue avvalgi Apollo checkpoint va
ResumeApollo parametrini saqlaydi. Settings ostidagi tanlash va start bloklangan;
oyna yopilganda tanlov qoladi. Takroriy loading so'rovlari almashtirishga yo'l
qo'ymaydi.

## Ko'rinish

Uchta asosiy expedition tugmasi va uchta haqiqiy missiya kartasi bitta umumiy
widgetdan foydalanadi. Unreal BackgroundBlur, shaffof ko'k qatlam, yumaloq chet va
cheklangan glow qo'llanadi. Tanlash animatsiyasi 280 ms smoothstep bilan tugaydi
va karta ko'k bo'lib qoladi; doimiy miltillash yo'q. Katta kartalarda SELECTED
matni ham bor. Past sifatli blur uchun o'qiladigan to'q ko'k fallback mavjud.
150% matnda tavsif karta ichida scroll qilinadi; tanlov belgisi yopilmaydi.

Ikkinchi videoda uchta expedition tugmasi ko'rsatilgan. Frosted blur va silliq
animatsiya yozma talabdan olingan; 280 ms muallif tanlagan UI parametri, videodan
o'lchangan vaqt emas. 1280x720 dizayn ekran o'lchamiga mutanosib joylashadi.

SELENE kartasi koddagi haqiqiy 480 soniyalik gameplay oxygen limitini ko'rsatadi.
Eski besh daqiqalik tavsif qo'llanmaydi. Missiya balansi, NASA relyefi, datasetlar,
xaritalar va checkpoint formati o'zgartirilmagan.

## Qayta tekshirish

```powershell
./Tools/verify_mission_selection.ps1
./Tools/verify_mission_selection.ps1 -Review
./Tools/verify_mission_selection.ps1 -Packaged
./Tools/verify_mission_selection.ps1 -Packaged -Review
./Tools/verify_loading.ps1 -Packaged -Graphics
./Tools/verify_apollo12.ps1 -Packaged -Regression -Checkpoints
```

Faqat C++ o'zgargan holatda package helperdagi kam xotirali bayroqlar bilan
Editor va Game build qilinadi, so'ng ./Tools/package_apollo12.ps1 -StageOnly
ishlatiladi. To'liq ./Tools/package_apollo12.ps1 build va cook ham qiladi.
Yangilangan paket Play Lunar Rescue.cmd orqali ochiladi.

## Tekshiruvlar

**Yakuniy Windows paketida 502 assertion/outcome o'tdi, 0 xato.**
Exe SHA-256: `5afb3476db95cf844059a0ae8cb6cf1da8bfb323897a75a91e9e5f5e2f3bbeeb`.

Editor/Game compile va StageOnly stage/archive ExitCode=0. Ushbu C++ o'zgarishida
avvalgi cook qayta ishlatildi; yangi cook yoki asset import bajarilgani da'vo
qilinmaydi. Shu final binaryda ishlaydigan test natijalari:

| Suite | PASS | FAIL |
|---|---:|---:|
| mission-selection-checks | 51 | 0 |
| frontend-checks | 20 | 0 |
| apollo12-checks | 57 | 0 |
| gameplay-checks | 50 | 0 |
| mars-gameplay-checks | 54 | 0 |
| menu-travel-checks | 55 | 0 |
| polish-checks | 63 | 0 |
| checkpoint-checks | 51 | 0 |
| checkpoint-write-checks | 2 | 0 |
| checkpoint-read-checks | 7 | 0 |
| mars-movement-checks | 13 | 0 |
| mars-ledge-checks | 3 | 0 |
| loading-checks | 76 | 0 |

720p va 1080p uchun 13 tadan yangi DX11 PNG olindi (jami 26). Har bir renderda
Apollo/SELENE/ARES tavsifidagi ScrollBox hududiga mouse-enter, pointer-down/up va
mouse-exit haqiqiy Slate hit-test yo'li orqali yuborildi: jami 6 pointer check.
Tanlov qolishi, faqat tanlangan ko'rinadigan karta ko'k bo'lishi, oraliq blendlar
va 150% matndagi alohida scroll maydoni tekshirildi. Collapsed kartaning eski
ko'rinmaydigan blend qiymati ekrandagi faol tanlov deb hisoblanmaydi.

Oldindan inventoried 84 save/photo/config/editor preference fayli hash bilan
solishtirildi. Player fayllari saqlandi. Avtomatik testdan keyin originaliga
qaytarilgan Editor preference fayllari soni: 1. To'liq qayd:
`Reports/MissionSelection20261031/preservation-results.json`.

### Menyu performance juftligi

Ryzen 5 6600H, RTX 3050 Laptop, 16 GB RAM; DX11 RenderOffscreen, 1280x720,
Medium preset 1, cap60, VSync off, audio off. Har birida 1800 frame;
300 warmup chiqarilib, 1500 frame (taxminan 25 s) baholandi.

| Static home menu | Mean FPS | 1% low FPS | p99 ms | Max ms |
|---|---:|---:|---:|---:|
| Before | 59.998 | 55.565 | 17.6407 | 19.6466 |
| After | 59.996 | 55.730 | 17.7060 | 18.0999 |

Bu bitta statik menyu juftligi: butun missiya yoki animatsiya FPS kafolati emas.
Natija tezlashuv isboti sifatida talqin qilinmaydi. CSV auto-stop ikkala
jarayonda raw exit 777003 qaytardi; fresh 1800-frame CSV va toza Capture Ended
logi alohida tasdiqlandi. Functional test/build exit0 bilan aralashtirilmaydi.
Working set 1 s oralig'ida o'lchangan, VRAM yoki haqiqiy peak memory emas.
`performance-after` va `performance-after-final` oldingi candidate binarylarga
tegishli; final executable uchun `performance-after-input` ishlatiladi.

### Test davomida tuzatilgan holatlar

- Review harnessdagi TObjectPtr `.Get()` va screenshot callback umrini tuzatish.
- Pointer testiga mouse-enter/exit qo'shish va brush outline rangini alohida belgilash.
- Optimized Windows buildda animatsiya yakunini aniq 0/1 ga saturate qilish;
  avvalgi 6 endpoint assertion rad etilgani alohida logda saqlangan.
- Dastlabki build davomida tahrirlangan Game.cpp eski obyekt sifatida qolgan;
  fayl alohida qayta compile qilindi. Final movement logi sakkiz tugmani,
  jumladan Up/Right/Down/Left bilan haqiqiy yurishni tasdiqlaydi.

### Muhim fayllar

- `LunarFrontend.cpp`, `LunarPanels.h`: destination va mission-selection sahifalari.
- `LunarMissionCard.h/.cpp`: umumiy blue-glass widget va endpoint himoyasi.
- `LunarGame.cpp/.h`: Enter/Esc/arrow routing, menu ownership va movement regression.
- `LunarMissionSelectionChecks.cpp`, `LunarMissionSelectionReview.cpp`: test va render driver.
- `Tools/verify_mission_selection.ps1`, `Tools/profile_mission_menu.ps1`: fresh dalillarni tekshirish.

Mashina hisoboti: `Reports/MissionSelection20261031/validation.json`.
Oldingi TEST_REPORT bo'limlari tarixiy natijalar bo'lib qoladi; ular bu
iterationda bajarilgan suite sifatida qayta sanalmaydi.

Inson playtesti o'tkazilmadi. Renderlar tayyorlangan UI holatlari; ular inson
sessiyasi, missiyani tugatish yoki FPS benchmark emas. Ushbu o'zgarish uchun
reliz chiqarish yoki tanlovga yuborish bajarilmagan.
