# Mavjud menyu tuzatishi — 2026-09-21

Oldingi auditdan keyingi lokal maintenance. Bu challenge implementatsiyasi emas; NASA terrain, planner va debrief kechiktirilganicha qoladi. Ushbu o‘zgarish ham hakatondan oldingi ish sifatida oshkor qilinishi kerak; tanlovda ishlatish huquqi tasdiqlanmagan.

## Xato va tuzatish

Oldin menu sahifasining chap yoki o‘ng yarmidagi istalgan click darhol planet tanlardi. Sarlavha, footer yoki ikki karta orasidagi bo‘sh joy ham missiyani boshlardi. Endi mouse faqat chizilgan karta ichida tanlaydi. Cursor koordinatasi yoki viewport bo‘lmasa, tanlash bajarilmaydi.

`MissionCardBounds` HUD chizishi va pointer hit tekshiruvi uchun yagona geometriya beradi. `PrimaryClick` mouse hodisasini alohida boshqaradi; Enter Moon tanlaydi, 1/2 avvalgidek ishlaydi. Missiya ichida mouse start/resume/restartning mavjud xatti-harakati saqlanadi. Resource yoki mission ketma-ketligi o‘zgarmadi.

Fayllar: `Source/LunarRescue/LunarGame.cpp`, `LunarGame.h`, `Config/DefaultInput.ini`. Binary asset tahriri va map regeneration bajarilmadi.

## Tekshirish

Natija va aniq cheklovlar [TEST_REPORT](TEST_REPORT.md)ning maintenance bo‘limida qayd etiladi. Oldingi packaged baseline bu source o‘zgarishi uchun yangi packaged test deb ko‘rsatilmaydi.

Qo‘lda UX tekshirish tartibi: MissionSelectda sarlavha, footer va kartalar orasini bosish — shu sahifada qolish; har bir karta ichini bosish — tegishli briefingga o‘tish; Enter — Moon; 1/2 — tegishli planet; M bilan qaytish; briefing/pause/resultdagi clicklar. 1280×720, 1920×1080 va keng ekranda takrorlash. Ushbu protokolning mavjudligi bajarilgan playtest degani emas.

`Dist/Windows` va GitHubdagi v0.2.0 hali eski build. Tuzatishni source/editor buildda sinash mumkin; tarqatish uchun keyin yangi local package talab qilinadi. Remote push yoki release bajarilmadi.
