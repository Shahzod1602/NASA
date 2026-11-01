# Space Apps 2026: eligibility status

## Current revalidation - 1 November 2026, Asia/Tashkent

**Eligibility is not confirmed.** No current full 2026 challenge has been read
and matched to this version of LunarRescue. Product quality and successful local
builds do not establish competition acceptance.

The session's web fetches returned 502 errors. Fresh command-line requests to
the official FAQ, legal terms and 2026 challenge directory were bounded to
30 seconds each and failed certificate validation (`SEC_E_UNTRUSTED_ROOT`, curl
error 60, zero downloaded bytes). Certificate verification was not disabled.
No current page content or rule was verified through those failed requests.
Attempt metadata and error-log hashes are in the ignored
`Reports/competition-refresh-20261101/attempt-results.json`.

| Official page | Result of this revalidation |
| --- | --- |
| [Participant FAQ](https://www.spaceappschallenge.org/resources/participant-faqs/) | Unavailable; no fresh rule conclusion. |
| [Participant terms](https://www.spaceappschallenge.org/legal/) | Unavailable; prior terms are not represented as rechecked today. |
| [2026 challenge directory](https://www.spaceappschallenge.org/2026/challenges/) | Current full statements were not read; no challenge fit is approved. |

The September audit retained below is a historical record. Its summary-phase
status, event dates, publication schedule and submission expectations must not
be presented as freshly confirmed November requirements. Current video duration,
slide limits, reuse permissions and judging eligibility remain unconfirmed.

The existing releases, LROC integration, Apollo/Mars gameplay and later UI or
presentation work are pre-existing independent product development. Their real
Git dates must be retained and disclosed; separating old and new work does not
grant permission to reuse either for a challenge. This update does not declare
that work eligible, enter a team or submit the project. If current rules restrict
challenge-specific implementation to the hackathon period, that implementation
must remain backlog until the permitted period and scope are established.

A current organizer clarification is needed before choosing competition work.
Draft question, **not sent**:

> May a team reuse LunarRescue's pre-existing source, maps, assets and independent
> product improvements through 1 November 2026, with original Git dates and full
> disclosure? Which parts, if any, may be judged for a 2026 challenge, and when may
> challenge-specific implementation begin? Please identify the current full
> challenge, submission/video limits and applicable reuse/open-source rules.

The existing code-licence and third-party-material questions also remain open;
see [credits](CREDITS.md). Continue the game as an independent educational
product while competition permission and challenge fit are unresolved.

## Historical audit - 21 September 2026

The material below records what was read and concluded on that earlier date.
It is preserved for provenance, not asserted as the current event state.

## Sources last read on 21 September 2026 - historical

| Manba | Tekshiruv natijasi |
| --- | --- |
| [2026 Participant FAQs](https://www.spaceappschallenge.org/resources/participant-faqs/) | Challenge ustida hakatondan oldin ishlashga ruxsat berilmaydi. Summaries: 17-sentabr; to‘liq statements: 28-oktabr; hakaton: 14–15-noyabr 2026. |
| [2026 Participant Terms and Conditions](https://www.spaceappschallenge.org/legal/) | Sahifa 2026 yilga tegishli, yangilangan sana 2026-08-07. Challenge mosligi, materiallarni ochiq taqdim etish va uchinchi tomon huquqlari shartlari bor. |
| [2026 challenge directory](https://www.spaceappschallenge.org/2026/challenges/) | Summary bosqichi. To‘liq topshiriq o‘qilmagani sababli hech bir challenge tanlangan yoki mos deb tasdiqlangan emas. |
| [Participant resources](https://www.spaceappschallenge.org/resources/) va FAQ | 2026 submission hamda judging qo‘llanmalari 13-noyabrga rejalashtirilgan. Demo davomiyligi va slayd soni tasdiqlanmagan. Eski qo‘llanma limitlari qo‘llanilmadi. |

FAQ va legal sahifalarining to‘liq matni tekshirildi. Brauzer qidiruv vositasidagi 502 javobi o‘rniga rasmiy sahifalar `curl.exe` bilan olindi. Mahalliy nusxalar va hashlar `Reports/education-audit/` ichida; bu papka Git tomonidan e’tiborga olinmaydi. Qoidalar o‘zgarishi mumkin: implementatsiya va topshirishdan oldin qayta tekshirish kerak.

## Oldingi ishlar va ushbu audit

`v0.2.0` — hakatondan **oldin** yaratilgan mahsulot. Git tarixidagi haqiqiy sanalar o‘zgartirilmadi:

| Commit | Mualliflik vaqti (+05:00) | Ish |
| --- | --- | --- |
| `9e14f53` | 2026-09-21 03:58:37 | Dastlabki loyiha |
| `1aae061` | 2026-09-21 05:02:02 | Oldingi audit tuzatishlari |
| `20b4b4b` | 2026-09-21 06:23:15 | Mars, mission selection va standalone verification |

Ushbu 21-sentabr tekshiruvi: mavjud kod auditi, mavjud build/testlar, dataset metama’lumotini baholash va kelajak ishlar hujjati. Yangi NASA terrain, planner, resource model yoki debrief kodi yozilmadi. Bu hujjatlar ham hakatondan oldingi materialdir; ularni tanlov uchun ruxsatli deb hisoblash mumkin emas.

Foydalanuvchining shartiga binoan challenge implementatsiyasi [backlog](DEVELOPMENT_PLAN.md) sifatida qoldirildi. Oldingi va yangi ishni ajratishning o‘zi qayta ishlatishga ruxsat bermaydi. To‘liq challenge mos kelmasa, mahsulotni mustaqil ta’limiy loyiha sifatida davom ettirish yoki talabga mos yangi, kichik hakaton prototipini yaratish variantlari bor.

## Ochiq huquqiy va tashkiliy masalalar

Terms original materiallar, ochiq litsenziyalash va uchinchi tomon tarkibini belgilaydi; oldindan tayyorlangan ushbu o‘yin uchun aniq istisno topilmadi. Loyiha ildizida kod litsenziyasi yo‘q. Unreal Engine va asset litsenziyalari alohida: tanlovning ochiqlik shartiga avtomatik mos deb bo‘lmaydi. Aniq litsenziya qarori muallifga tegishli; [CREDITS](CREDITS.md)ga qarang.

Tashkilotchiga yuborish uchun savollar tayyorlandi, **yuborilmadi**:

1. “May we reuse LunarRescue v0.2.0, publicly released on September 21, 2026, including its gameplay, maps and assets, if all challenge-specific work begins during November 14–15? What must be disclosed, and what can be judged?”
2. “Does the restriction on pre-hackathon challenge work permit preparatory dataset evaluation, technical audits and planning documents? These documents already exist and would be disclosed.”
3. “Can an Unreal Engine project with source available, a Windows executable, CC BY/CC0 assets and proprietary engine dependencies meet the 2026 submission and third-party-material requirements? Which licenses or substitutions are necessary?”
4. “Where are the final 2026 challenge requirements, demo/video limits and submission format, and which official clarification should our team follow?”

To‘liq statement chiqqach, har bir talabni aniq fayl/demo daliliga bog‘laydigan jadval tuziladi. Moslik aniqlanmaguncha taqdimot ham submission-ready emas.
