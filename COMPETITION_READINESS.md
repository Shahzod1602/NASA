# Space Apps 2026: tekshiruv holati

Tekshirilgan sana: **2026-09-21**, Asia/Tashkent. Holat: **eligibility tasdiqlanmagan; yangi challenge implementatsiyasi kechiktirildi**. Mahsulotning ishlashi tanlovga qabul qilinishini anglatmaydi.

## Joriy yil manbalari

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
