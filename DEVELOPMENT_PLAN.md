# Rivojlantirish rejasi

2026-09-21 holati. **Bu backlog; quyidagi yangi funksiyalar bajarilmagan.** [2026 vaqt cheklovi](COMPETITION_READINESS.md) sabab challenge implementatsiyasi hakatonga qoldirildi. Oldingi v0.2.0 va ushbu tayyorgarlikning qayta ishlatilishi ham tasdiqlanishi kerak.

## Bajarilgan

- Auditdan keyingi maintenance: mavjud menu click chegarasi tuzatildi; [alohida qayd](MAINTENANCE_FOLLOWUP.md). Bu yangi challenge funksiyasi emas.
- Repository, source/config/tools/credits, Unreal muhiti va LFS tekshirildi.
- Mavjud Moon/Mars oqimi va yangi tizimga ta’sir qiluvchi muammolar yozildi.
- Mavjud editor target va packaged gameplay/traversal uchun yangi baseline olindi; [natijalar](TEST_REPORT.md).
- Rasmiy 2026 qoidalari va LOLA nomzodining labeli tekshirildi; integratsiya bo‘lmaganligi aniq qayd etildi.
- Ilmiy chegaralar, litsenziya ochiqlari, playtest va shartli taqdimot hujjatlari tayyorlandi.

## Eng kichik to‘liq rivojlantirish bosqichi

Bir haqiqiy Oy ROI, ikki yuriladigan yo‘l, bitta planning oynasi, mavjud rescue, haqiqiy session debrief va sources oynasi. Mars avvalgi holatda saqlanadi. VR/multiplayer/open world/crafting/chatbot scopega kirmaydi.

| Bosqich | Amal | Tugash dalili |
| --- | --- | --- |
| 0. Ruxsat/moslik | Full challenge va reuse javoblarini tekshirish; litsenziya tanlovi; tanlov scope yoki mustaqil mahsulot yo‘nalishini belgilash | Talab → dalil jadvali; haqiqiy sanalar |
| 1. Data | SCIENCE_AND_SOURCES pipeline; NoData/projection aniqlash, ROI tanlash | Raw/derived hash, koordinata va slope fixture testlari |
| 2. Terrain | Yangi `/Game/Lunar/Maps/LolaExpedition`; asl MoonBase/MarsBasega tegmaslik | Unit/collision QA va oddiy movement bilan ikki route traversal |
| 3. Planning/resource | Xarita, start/station/science/lander, A/B, shared forecast, confirmation | Tanlov active missionga o‘tadi; hisob birliklari va xatolik chegarasi testlangan |
| 4. Science/debrief | Route elevation/slope comparison, local telemetry, result va sources oynalari | Terminal shu datasetni ishlatadi; natija telemetrydan tekshiriladi |
| 5. Package/QA | Cook metadata, offline run, regressiya, bir xil grafik baseline, playtest | 17 acceptance mezoni; haqiqiy yangi foydalanuvchi qaydlari |

## Kichik arxitektura

`LunarGame.cpp`ning mavjud rescue state mashinasi saqlanadi. Yangi profil uchun `UDataAsset` va oddiy `USTRUCT`lar yetarli: mission budget/warnings, route points/stats va dataset metadata. Resource/forecast hisobini bitta kichik C++ modulga ajratish; planner, HUD va gameplay shu funksiyalarni chaqiradi. Session telemetry mission-owned struct bo‘lib BeginPlay/restartda yangilanadi. Debrief formatter immutable session snapshotdan matn yaratadi. HUD faqat taqdim etadi. Hozir bu klasslar yaratilmagan; Blueprintlarda keraksiz ko‘p komponent qo‘shilmaydi.

Capacity profilidan o‘qilsin, BeginPlay uni majburan 300/480 bilan almashtirmasin. Planet/map parametrlarini nom satridan taxmin qilish o‘rniga profilga bog‘lash yangi map uchun yetarli minimal o‘zgarish bo‘ladi.

## Yo‘l va resurs qarori

Route A qisqaroq/tikroq, B uzunroq/tekisroq bo‘lishi **haqiqiy grid bilan** isbotlanadi. Mos kelmasa boshqa ROI/yo‘l tanlanadi. Hududni kerakli xulosaga moslab buzish mumkin emas. Xarita actual polyline va poi nuqtalaridan chiziladi. O‘yinchi yo‘ldan chiqishi mumkin: debrief tanlangan yo‘lni va haqiqatan bosilgan segmentlarni ajratadi.

Avval tushunarli baseline saqlanadi: `budget_used = active_elapsed_seconds`. Planner `sum(segment_length / calibrated_segment_speed) + required_hold_and_restart_time`ni hisoblaydi; masofa 3Dmi yoki horizontalmi UI/manifestda ko‘rsatiladi. 320 cm/s maksimal yurish tezligi acceleration, slope va collision tufayli har doim erishiladigan o‘rtacha tezlik emas. Prediction kalibrovkasi oddiy traversal bilan qilinadi; optional detour alohida qo‘shiladi.

Harakatga bog‘liq sarf zarur bo‘lsa, tuning koeffitsiyentlari ochiq formulada aniqlanadi: `rate = base_rate + speed_coefficient * normalized_speed + uphill_coefficient * uphill_grade`; barcha qiymatlar finite/nonnegative, interval/clamp siyosati hujjatlangan. Forecast va live integrator ayni formulani ishlatadi. Koeffitsiyentlar ilmiy fiziologiya sifatida berilmaydi va hozir raqamlar o‘ylab topilmaydi. Qo‘shimcha resource turi rejalashtirilmagan.

A yoki Bni sun’iy “to‘g‘ri javob” qilmaslik uchun turli aniq ko‘rsatilgan budget va optional science/recorder ustuvorliklari bilan sinash kerak. Har ikkisi bilan asosiy missiya bajariladigan holat va detourdan voz kechish foydali holat o‘lchanadi; yutuq route labelga bog‘lanmaydi.

## Telemetry va debrief shartnomasi

Local session: profile/dataset ID, selected route, planned duration/budget, active elapsed, distance, slope-bin time, actual resource use, objective flags/timestamps, outcome. Pause, briefing, planner, result vaqtini active hisobga kiritmaslik; teleports/restart/map travel masofaga qo‘shilmasin. No PII, tashqi analytics yo‘q. Persistence kerak bo‘lmasa xotirada saqlash kifoya.

Debrief reja/actual, masofa, budget, main/optional flags, NASA data roli va bitta keyingi tavsiyani beradi. “Tik joylarda X sekund o‘tkazdingiz” faqat sampled slope va elapsed bilan chiqadi. “Tikligi sabab kechikdingiz” degan sababiy xulosa faqat vaqt farqidan chiqarilmaydi. Planned va actual farqi bor, ammo sabab o‘lchanmagan bo‘lsa matn shuni tan oladi. Kamida 2–3 izoh aniq telemetry shartlari bilan testlanadi.

## UX va qabul qilish

English UI: “Plan expedition”, “Route A — shorter, steeper”, “Route B — longer, gentler”, “Gameplay oxygen budget”, “Science & Sources”, “Mission debrief”. Confirmgacha resource yurmaydi. Pause controls va objectivega kirish beradi; rangdan tashqari matn/belgi; kontrastli subtitr; uzun metadata optional panelda. 1280×720, 1920×1080 va keng ekranlarda clipping tekshiruvi. Tutorial: move/look → interact/hold → budget va optional decision.

Yangi testlar: flat/known slope, endian/units/axis/round-trip, invalid/NoData/missing product, forecast/live tengligi, zero/negative/NaN/large delta, pause, restart reset, route confirm, truthful debrief, two routes/collision, optional-free win, cooked metadata/offline, Moon/Mars regression. Testlar input yoki real movementni tekshiryaptimi yoxud teleport/API assertionmi aniq yozilsin.

Performance: raster/slope offline; bounded cached route samples; metadata per-frame parse qilinmaydi. Yangi Tick faqat actual session hisoblash uchun kerak bo‘lsa mavjud tickka qo‘shiladi. Baseline bilan bir xil GPU, resolution, settings, traversal sharoiti; yangi mapning boshqa workload ekanligi yashirilmaydi.

## Keyingi Mars bosqichi

Oy stabil bo‘lgach resource, planner, telemetry va debriefni umumiy profil orqali Marsga qo‘llash. Mars uchun alohida rasmiy DEM va datum/unit/projection tekshiruvi; lunar LOLA yoki Moon atmosfera izohlarini ko‘chirmaslik. ARES relay ketma-ketligi regressiya testlarida qoladi.
