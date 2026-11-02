# Development plan

## Current local work - 2 November 2026 (unreleased)

Implemented: borderless mission/resource HUD text with state-driven counters and
hold feedback, compact measured loading typography, optional read-only I
notebook diagnostics, and bounded authored equipment presentation. Rescue
power dependencies and Apollo photo/setup/cargo prerequisites use the actual
mission records; opening the notebook cannot finish work. The 39-row asset
inventory accounts for 32 scene-detail treatments, five existing tools reused
in a storage display and two unchanged targets, not 39 new mesh imports.
See [gameplay scope](GAMEPLAY_REFERENCE_UPDATE.md),
[asset scope](ASSET_IMPROVEMENTS_UPDATE.md), [source candidates](ASSET_SOURCE_CATALOG.md)
and [loading typography](LOADING_TYPOGRAPHY_UPDATE.md).

Final local executable SHA-256
`b034765110ad30f5a60ffcf1e874ddac679002b16c74f0e9f72bea54017a9fb4`
compiled/staged after a full cook and passed all 17 queue runs plus typography:
1,254 exact PASS executions (1,128 functional + 76 actual-travel + 50 staged
HUD layout). This includes the six new grounding assertions per mission.
Wrappers separately validated 110 staged PNGs and six routed Slate pointer
outcomes. [The receipt](Docs/Feedback2026-11-02/evidence.json) records earlier
2ed/e738 executables under their own hashes; repeated prior results are not
additional unique tests or substitutes for final-build coverage. Earlier
release totals below remain historical. Matched controlled performance runs
and the 264/264 unchanged protected-file comparison completed; conditions and
limits are in [TEST_REPORT](TEST_REPORT.md). One run per scene does not establish
whole-mission sustained FPS. No human learning gain, new release or competition
eligibility is established.

Later visual correction: the supported Editor repair leveled the original
SELENE MoonBase rover (pitch 0/yaw 15/roll 0, world Z 4.55465 cm), with eight
wheels and 48 contact entries sampled against existing terrain. The map backup,
before/after hashes and successful receipt confirm unchanged rover properties
and 1,910 other actors. ARES and terrain are unchanged. Cosmetic basalt/talus
now uses bounds-normalized source dimensions, local terrain-normal alignment
and 20% height burial. Neither fix changes the mission dependency model.

Immediate priorities:

1. Use the final executable-specific receipt and [TEST_REPORT](TEST_REPORT.md)
   to assess release readiness, including matched-condition performance and
   protected-player-file preservation. Fourteen fresh Editor equipment views
   were reviewed; resolve the still-inconclusive solar-junction presentation in
   a future focused visual pass without treating source geometry or a clear
   line trace as rendered visibility proof.
2. Review the bounded equipment pass with normal packaged gameplay before any
   replacement mesh. Select candidates individually from the verified source
   catalogue; verify licence, dimensions, import and work/contact behavior.
   Research availability alone is not implementation.
3. Run the prepared novice playtest with real participants and record observed
   completion, comprehension answers and confusing points. Keep human sessions
   separate from scripted UI fixtures and functional assertions.

The later Moon A/B route/resource planner and sourced Mars elevation work remain
backlog. Space Apps prior-work and challenge eligibility still require their own
verified decision; product improvements do not establish competition acceptance.

## Previous verified milestone - 1 November 2026

The following record describes the earlier v0.7.0 milestone. Its totals do not
certify the local 2 November feedback/typography build.

The current local source adds the supplied-art intro, five glass home actions,
compact live mission HUD, an interactive pause page, confirmed full restart,
first-person work-view fixes and optional Field lessons. The existing Moon/Mars
mission picker, rescue sequence, real Apollo 12 terrain, checkpoint and local
expedition trace remain the foundation. See [this iteration's scope and evidence
status](NASA_TASKS_UPDATE.md) and [the broader plan](GAME_MASTER_PLAN.md).

The local Windows iteration is verified: 997 packaged assertion/outcomes,
156 staged 720p/1080p PNGs, full Apollo traversal and paired rendered performance
captures. The final executable and limitations are in TEST_REPORT.md. All 131
original save/photo/config files match their preserved hashes. Human sessions
remain zero; competition acceptance remains unconfirmed.

The next three priorities are:

1. Add terrain-supported A/B route choices, a matching forecast/live resource
   model and route-aware debrief telemetry to the Moon chapter.
2. Run the prepared novice playtest when participants are available; record
   completion, before/after answers and confusing moments without fabricating gains.
3. Verify the full current 2026 challenge, prior-work reuse and licensing terms
   before preparing a challenge-specific implementation or submission.

Later educational development still includes two terrain-supported route
choices, a mission planner, matching forecast/live resource calculations and
route-aware debrief telemetry. Those systems are not supplied by the current
three-question lesson panel. Mars needs its own verified elevation product
before its authored canyon can be described as a measured NASA terrain map.

## Historical planning record - 21 September 2026

The remainder is retained as the earlier backlog and competition-dependent plan.
Its references to unimplemented terrain work concern the proposed LOLA profile;
the current Apollo chapter instead uses the separate LROC product described in
SCIENCE_AND_SOURCES.md.

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
