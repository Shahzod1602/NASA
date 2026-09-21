# Texnik audit — 2026-09-21

Bu boshlang‘ich audit snapshoti. Keyingi menu click tuzatishi [MAINTENANCE_FOLLOWUP](MAINTENANCE_FOLLOWUP.md)da alohida qayd etilgan.

Tekshirilgan checkout: `D:\LunarRescue`, `main`, HEAD `20b4b4be2b53f0dc08585d259bc7ee42d6493e44`. Boshlang‘ich `git status` toza. Remote: `https://github.com/Shahzod1602/NASA.git`. Commit qilinmagan foydalanuvchi ishi topilmadi; tarix, binary maplar va assetlar o‘zgartirilmadi. Root/loyiha ichida `AGENTS.md` topilmadi.

README, MARS_UPDATE, AUDIT_FIXES, SourceArt credits, Source, Config, Tools va mavjud Reports ko‘rib chiqildi. Oldingi hisobotlar yangi test o‘rnida ishlatilmadi.

## Muhit va real tekshiruv imkoniyati

Windows, Unreal Engine **5.8.2** (Build.version CL 56702186), Win64 Development; AMD Ryzen 5 6600H, 16 GB RAM, NVIDIA RTX 3050 Laptop GPU. `.uproject` EngineAssociation `5.8`. Git LFS fsck muvaffaqiyatli; 141 ta tracked Content fayli orasida ochilmagan LFS pointer yo‘q. Unreal Editor build vositasi va standalone binary mavjud. Shu sababli incremental editor build va grafik packaged regressiya/traversal haqiqatan bajarildi. [Test dalillari](TEST_REPORT.md).

## Mavjud oqim va mas’uliyatlar

`Source/LunarRescue/LunarGame.cpp` bir faylda Character, GameMode, Canvas HUD va ichki testlarni saqlaydi; `LunarGame.h` state va parametrlarni belgilaydi. Butun faylni qayta yozishga zarurat yo‘q.

| Qism | Kod/config va haqiqiy xatti-harakat |
| --- | --- |
| Selection/travel | `ChoosePlanet`, `ReturnToMenu`, `RestartMission`, `TravelCheck`; MissionSelect → MoonBase/MarsBase. R faqat natijada; M briefing/pause/natijada. |
| Mission | `BeginPlay`, `Interact`, `AdvanceInteraction`: Briefing → FindModule → RestorePower → CollectData → ReturnHome → Won/Lost. Marsda CollectData va ReturnHome orasida AlignRelay bor. |
| Repair | Cell o‘rnatish → kabel → 2 s ushlab yoqish → 4.2 s reboot; science 3 s; Mars relay 4 s. |
| Optional | Recorder 2 s hold va ledge sharti; Q navigatsiya markerini almashtiradi, vazifani majburiy qilmaydi. |
| Input | `SetupPlayerInputComponent`, `Config/DefaultInput.ini`; movement/hold/pause, raqamlar bilan planet, mouse bilan qarash. |
| Resource | `AdvanceTime`: active va paused emas bo‘lsa vaqt sarfi; 300/480 gameplay sekund. 60 s ogohlantirish; nol yoki pastga yiqilishda Lost. |
| Pause/audio | `PauseMission`: movement tick, tracked radio va power soundlar, gameplay timer/hold to‘xtaydi. Global world pause emas. |
| HUD | `ALunarHUD::DrawHUD`: Canvas, objective, marker, O2, subtitr, pause va natija. 1280×720 bazaga scale; uzun matn wrap/multires alohida QA talab qiladi. |
| Save/config | USaveGame va oddiy session telemetry topilmadi. Graphics defaultlari DefaultGameUserSettings.ini/DefaultEngine.ini; test hisoboti FFileHelper bilan yoziladi. |
| Terrain | Authored procedural meshlar; haqiqiy elevatsiya, georeference, slope layer yoki dataset manifest yo‘q. BuildStoryProps mapga bog‘langan koordinatalardan foydalanadi. |

## Ustuvor muammolar

### Ishni to‘xtatadigan

- Tanlovga tegishli yangi implementatsiya: [2026 qoida va ochiq savollar](COMPETITION_READINESS.md). To‘liq challenge hali tekshirish uchun e’lon qilinmagan.
- NASA hududida ikki yo‘l ishlashini hozir isbotlab bo‘lmaydi: raster importi va traversal qilinmagan. Nomzodning NoData talqini ham ochiq.
- Kodning root litsenziyasi yo‘q; tarqatish/tanlovga moslik alohida aniqlanishi kerak. Mavjud local testlarni to‘xtatadigan build xatosi aniqlanmadi.

### Yangi funksiyaga ta’sir qiladigan

- `BeginPlay()` `OxygenCapacity` qiymatini planetga qarab 300/480 bilan almashtiradi. Blueprint Class Defaults orqali o‘zgartirish haqidagi eski README ko‘rsatmasi amalda ishlamaydi; hujjat tuzatildi. Kod tuzatishi backlogda.
- Forecast va haqiqiy hisob uchun umumiy resource funksiyasi yo‘q. Hozir sarf harakatga/qiyalikka bog‘liq emas.
- Oddiy session masofa, active elapsed time, tanlangan yo‘l, slope exposure va natija telemetrysi yo‘q. Walkthrough test counterlarini foydalanuvchi telemetrysi deb ishlatmaslik kerak.
- Dataset loader, NoData mask, koordinata/unit tekshiruvi va packaged metadata staging yo‘q.
- Mission actor koordinatalari va map nomiga bog‘langan planet aniqlash yangi hududga ko‘chirishni talab qiladi.
- Nolga tushish va pause mavjud testlarda bor; nonfinite/negative capacity, reset telemetry, yangi predictor chegaralari testlanmagan.

### Keyinroq tuzatiladigan

- Menu click ekran yarmiga qarab tanlaydi; karta chegarasi/Y hitbox tekshiruvi alohida yaxshilanish.
- Radio tanlash matn prefikslariga bog‘langan. Yangi mission eventlar ko‘payganda barqaror ID bilan ajratish mumkin.
- Bir fayldagi HUD/gameplay/testlar: faqat yangi tizimga tegishli mas’uliyatlarni ajratish; umumiy refactor kerak emas.
- Turli ekran nisbatlari, matn kesilishi va yangi foydalanuvchi tushunishi qo‘lda tekshirilmagan. Avtomatik test accessibility xulosasi bermaydi.

## Map va test xavfsizligi

`Tools/build_moon.py`, `upgrade_lunar_v3.py`, `build_mars.py` map yaratadi/almashtiradi yoki actorlarni o‘chiradi. Ushbu auditda ishlatilmadi. Yangi haqiqiy hudud alohida map bo‘lishi kerak; MoonBase/MarsBase saqlanadi. `.uasset`/`.umap` oddiy matn sifatida tahrir qilinmadi. GPU performance uchun NullRHI ishlatilmadi.
