# LunarRescue — o‘yinni yaxshilash va tugallash rejasi

**Sana:** 2026-10-24. **Asos:** mahalliy v0.4.1, Unreal Engine 5.8.3, Windows.

Bu hujjat keyingi ishlar uchun asosiy reja. Bo‘limlardagi talablar yakuniy maqsadni bildiradi; barchasi bajarilgan degani emas. Amalga oshirish holati quyidagi jadvalda ajratilgan. [2026-09-21 rejasi](DEVELOPMENT_PLAN.md) avvalgi holatni saqlaydi.

## Yangi bosqich — 2026-10-29/30, mahalliy ishlab chiqish

My Expedition xaritasi, checkpoint ichidagi cheklangan shaxsiy marshrut,
refine qilingan skafandr/qo‘lqoplar, haqiqiy etik holatiga mos 160 ta qayta
ishlatiladigan iz va ixtiyoriy o‘tkazib yuboriladigan yuk joylash sahnasi
amalga oshirildi. Aniq holat va bajarilgan tekshiruvlar
[EXPEDITION_EXPERIENCE.md](EXPEDITION_EXPERIENCE.md) da qayd qilinadi.

Foydalanuvchi hozir playtest ishtirokchilari yo‘qligini bildirdi. 3–5 kishilik
sinov ssenariysi va bo‘sh formalar tayyor, inson natijalari mavjud emas.
Quyidagi sanalangan bo‘limlar o‘z davridagi tarixiy holatni saqlaydi.

## Yangi amalga oshirish — 2026-10-25 / v0.5

Olti yo‘nalish bo‘yicha yangi kod va aktivlar qo‘shildi:

- **ART/ANIM:** 36 suyakli skafandr va birinchi shaxs qo‘llari, yumshoq bo‘g‘im og‘irliklari, barmoq uchlari, tayanch oyoqni yerga mahkamlash, burilish qadami, asbobga mos qo‘l holatlari va uchta LOD. GLB/OBJ/JSON hamda qayta yaratish skriptlari loyihada. Runtime holatlar skelet suyaklarini boshqaruvchi kod orqali bajariladi; tayyor motion capture yoki muzey skani deb ko‘rsatilmaydi.
- **PLAY:** alohida foto, asbob tekislash, antenna, tosh/tuproq, bolg‘ali kern va detal yechish oqimlari. E asbobni ochadi, chap bosish uning amalini bajaradi; Esc tugallanmagan ishni bekor qiladi. Ish kamerasi nishonga yaqinlashadi. Yakuniy qadoqlashgacha yuk qo‘shilmaydi.
- **UX/settings:** UMG sozlamalar va dalillar albomi; qayta biriktiriladigan tugmalar, sezgirlik, yorqinlik, FOV, matn/subtitr kattaligi, ovoz, qo‘l harakati, tez qadamni almashtirish va grafik presetlar. Asosiy Canvas HUD/jurnal hali mavjud; butun UI UMG ga ko‘chdi deb belgilanmaydi.
- **WORLD/AUDIO:** qo‘shimcha ALSEP/Surveyor detali, qo‘l asboblari, Surveyor tanasiga pawn collision, soyadagi o‘qilishni yaxshilovchi fill va subtitrga mos yettita original inglizcha radio xabari.
- **PLAY/HIST:** odatiy rejim vaqtsiz, 30 daqiqalik mashq alohida. Eski checkpoint o‘z rejimini saqlaydi. Bench/12032 tuproq sifatida, Sharp/12027 esa zamonaviy burg‘i o‘rniga drive tube sifatida ishlanadi. NASA/LROC ning oltita xom manba fayli hash bilan o‘zgarmagani tekshirildi.
- **QA/REL:** Editor’da 300, Windows paketida 303 tekshiruv xatosiz o‘tdi; foto tuzatishidan keyin 129 paket tekshiruvi qaytarildi. Barcha asboblar bilan to‘liq avtomatik missiya 1 238,01 metr / 570,52 soniyada tugadi. 720p Medium profilida o‘rtacha 59,90 FPS, 1% low 51,18 FPS; 206 ms uzun kadr qayd etildi. Natijalar va chegaralar [v0.5 qaydi](POLISH_UPDATE.md) da. [5–8 yangi o‘yinchi sinov yo‘riqnomasi](PLAYTEST_GUIDE.md) tayyor; haqiqiy insonlar bilan sinov hali bajarilmagan.

Quyidagi jadval v0.4.4 gacha bo‘lgan tarixiy holatni saqlaydi.

## Amalga oshirish holati — 2026-10-24

| Reja vazifasi | Joriy holat |
|---|---|
| BASE-01 | v0.4.1 fayllari/paketi, hash manifesti va yangi baseline tekshiruvlari saqlandi |
| UX-01 | Bitta vazifa + harakat + maqsad HUD i yozildi, 720p ko‘rib chiqildi; Canvas dan UMG ga o‘tish keyingi ish |
| UX-02 | Qarash/yurish/ishlash yordami ishlaydi; yangi foydalanuvchi bilan qabul sinovi hali kerak |
| UX-03 | v0.4.4: foydalanuvchi tanloviga binoan butun interfeys inglizcha; 153 ta barqaror English String Table yozuvi |
| UX-05 | Majburiy 12 nuqta qulfi olindi; bevosita va tarixiy yo‘l bilan to‘liq avtomatik yurish o‘tdi |
| PERF-01 | v0.4.1–0.4.3 uchun boshlang‘ich 60 soniyalik render profillari olindi; v0.4.3 da saqlash I/O si yoqilgan. Butun missiya profili hali navbatda |
| SYS-01 | Checkpoint chegarasida barqaror missiya/ish/joy ID lari qo‘shildi; gameplay ichidagi umumiy holat refaktori hali navbatda |
| SAVE-01, SAVE-02 | Avtomatik checkpoint, C Continue, pauzada R, dalil/detal holati va zaxira tiklash ishlaydi; editor va paketda 60 tadan saqlash tekshiruvi o‘tdi. To‘liq sozlamalar va jurnal foto albomi hali navbatda |
| ART/ANIM, qolgan vazifalar | Ushbu yangilanishda bajarilgan deb belgilanmagan; ketma-ket davom etadi |

Tafsilot va mavjud chegaralar: [0.4.2 tushunarlilik yangilanishi](ONBOARDING_UPDATE.md), [0.4.3 checkpoint yangilanishi](CHECKPOINT_UPDATE.md). 0-bosqich bajarildi; 1-bosqichda tushunarlilik va saqlashning o‘ynaladigan qismi amalga oshirildi. Yangi o‘yinchi bilan qabul sinovi, sozlamalar va skeletli skafandr hali navbatda.

## 1. Yakuniy mahsulot

**LunarRescue: Apollo 12 — Surveyor merosi.** O‘yinchi Oy yuzasida ilmiy asboblarni joylashtiradi, namunalarni hujjatlashtiradi, Surveyor 3 qismlarini olib, Intrepid moduliga qaytadi. Qiziqish manbai: noma’lum joyni kuzatish, asbobdan to‘g‘ri foydalanish va Yerga nima uchun kerakligini tushunadigan dalil bilan qaytish.

Birinchi tugallangan versiya: Windows uchun internet talab qilmaydigan, bir kishilik, birinchi shaxsdagi bitta yaxlit bob. Uchinchi shaxs ko‘rinishi yurish va skafandrni kuzatish uchun qo‘shimcha imkoniyat. Birinchi o‘tish maqsadi taxminan **20–30 daqiqa**; bu hozircha dizayn maqsadi, insonlar bilan o‘lchanmagan.

Asosiy qarorlar:

- Birinchi navbatda tushunarlilik, skafandr, qo‘l va yurish, so‘ng missiyalardagi harakatlar yaxshilanadi.
- Asosiy til - English. 2026-10-24 dagi foydalanuvchi talabi avvalgi ikki tilli rejani almashtiradi.
- Asosiy tarixiy rejimda vaqt tugab qolishi yo‘q. Alohida mashq rejimida hozirgi 30 daqiqalik cheklov qolishi mumkin; u skafandrning haqiqiy kislorod sig‘imi deb ko‘rsatilmaydi.
- Apollo 12 ning ikki EVA sidagi tanlangan ishlar bitta bobga jamlanganligi boshida tushuntiriladi. Bu aniq ketma-ketlikdagi to‘liq tarixiy rekonstruksiya emas.
- NASA ma’lumoti, hisoblangan natija va o‘yin uchun yaratilgan detal bir-biridan ajratib ko‘rsatiladi.
- Birinchi relizga multiplayer, VR, mobil versiya, katta ochiq dunyo, qurilish/crafting va butun parvoz simulyatsiyasi kiritilmaydi.

## 2. Reja boshlangan paytdagi holat ? v0.4.1

| Yo‘nalish | v0.4.1 da mavjud | Keyingi zarur natija |
|---|---|---|
| Oy hududi | NASA/LROC 2 m qadamli DTM, xarita va joylashuvlar | Masshtab va manbalarni saqlab, ko‘rinish va navigatsiyani yaxshilash |
| Missiya | Intrepid → ALSEP → uch namuna → Surveyor → qaytish | Har bir ishning maqsadi tushunarli, harakati bir-biridan farqli bo‘lishi |
| Boshqaruv | Yurish, tezroq qadam, sakrash, E, Q, jurnal, ikki kamera | Bosqichma-bosqich o‘rgatish, kamroq majburiy tugma, qayta sozlash |
| Skafandr | Alohida statik qismlar, dasturiy qo‘l/oyoq holatlari | Skeletli yaxlit model, mos animatsiyalar va asbobni to‘g‘ri ushlash |
| Interfeys | Inglizcha Canvas HUD, markerlar, to‘rt sahifali jurnal | O‘qiladigan o‘zbekcha HUD, xarita, dalillar va qulay sozlamalar |
| Ilmiy ish | Foto olish, E ushlab yig‘ish, antenna burchagi | Foto, tosh yig‘ish, kern olish va detal yechishning alohida usullari |
| Saqlash | Tekshirilgan checkpoint/save tizimi yo‘q | Davom ettirish, qayta yuklash va topshiriqdan xavfsiz tiklanish |
| Sinov | Avtomatik tekshiruvlar va mahalliy Windows paket | Yangi o‘yinchilar bilan tushunarlilik va haqiqiy grafik tezlik sinovi |

[Oxirgi tekshiruv](Reports/SUIT_VALIDATION.md): editor bo‘yicha 173 tekshiruv o‘tgan, xato qayd etilmagan; paketdagi Apollo va skafandr tekshiruvlari ham o‘tgan. Avtomatik yurish 1 344,67 metrni 1 009,13 simulyatsiya soniyasida bosib o‘tgan. Bu natijalar missiyaning dasturiy o‘tishini tekshiradi; professional animatsiya, insonning o‘tish vaqti yoki FPS haqida dalil bermaydi.

Hozirgi asosiy muammolar: mayda va zich matnlar, birdan ko‘p tugma, takroriy E ushlab turish, majburiy 12 yo‘l nuqtasi, juda qorong‘i soyalar va yaxlit tanaga o‘xshamaydigan skafandr bo‘g‘imlari. Oldingi tuzatishlar foydali asos, lekin yakuniy sifat mezoni emas.

## 3. O‘yinchining dastlabki uch daqiqasi

1. **Bosh menyu:** “Davom ettirish”, “Yangi ekspeditsiya”, “Sozlamalar”, “Manbalar”. Apollo 12 asosiy tanlov; eski Moon/Mars prototiplari qo‘shimcha bo‘limda aniq nomlanadi.
2. **Qisqa kirish:** “1969. Sizning vazifangiz — Oy namunalarini va Surveyor 3 ning ayrim qismlarini Yerda o‘rganish uchun olib qaytish.” Bir ekran yoki 20–30 soniyali o‘tkazib yuboriladigan taqdimot yetadi.
3. **Intrepid yonida boshlash:** ko‘rinadigan bitta nishon, bitta ko‘rsatma. Avval atrofga qarash, keyin yurish, keyin E bilan yaqin narsani tekshirish o‘rgatiladi.
4. **Birinchi foydali harakat:** qo‘lqopdagi yoki ko‘krakdagi panelni tekshirish. Ko‘rsatkich o‘zgaradi, qo‘l animatsiyasi va qisqa tasdiq keladi. Uchta bir xil stol interactioni takrorlanmaydi.
5. **Asbobni olish:** o‘yinchi kamera va namuna to‘plamini ko‘radi. Xarita faqat dastlabki manzil va qaytish nuqtasini ko‘rsatadi.

Qabul qilish maqsadi: yangi o‘yinchi 60–90 soniyada birinchi harakatni bajarishi; besh kishidan kamida to‘rttasi yonidan aytib turilmasdan uni topishi. Bajarilmasa yangi kontent qo‘shishdan oldin shu oqim qayta ishlanadi.

## 4. Asosiy o‘yin sikli va qiziqish

**Joyni kuzatish → yo‘lni tanlash → mos asbobni ishlatish → dalilni qayd qilish → yukni tekshirish → qaytib topshirish.**

O‘yinchi faqat markerga borib kutmaydi. U foto uchun to‘g‘ri kadrni topadi, nishabdan xavfsizroq o‘tadi, asbobni tekislaydi va kerakli detalni ajratadi. Qarorlar ko‘rinadigan natija beradi: yaxshi kontekst fotosi, to‘liq yorliq, sozlangan signal va joyidan olingan detal.

Asosiy baholash: vazifani tugatish va dalilning to‘liqligi. Ixtiyoriy baholash: yo‘l samaradorligi va mashq vaqti. Oddiy rejimda tezlik uchun ilmiy ishni tashlab ketish rag‘batlantirilmaydi. Xato foto qayta olinadi, xato antenna qayta sozlanadi; tarixiy bo‘lmagan portlash yoki o‘lim bilan jazolash kiritilmaydi.

Bitta topshiriq uzun tushuntirishsiz bajariladi, lekin xohlagan o‘yinchi jurnal orqali uning tarixini chuqurroq o‘qiy oladi. Muhim radio xabari bir marta aytiladi; keyin jurnalga tushadi.

## 5. Olti bosqichli ssenariy

Vaqtlar birinchi o‘tish uchun taxmin. Yurish va asboblar tayyor bo‘lgach insonlar bilan qayta o‘lchanadi.

| Bosqich | O‘yinchi amali va mazmuni | Bosqich tugash sharti | Mo‘ljal |
|---|---|---|---|
| 1. Intrepid yonida tayyorgarlik | Skafandr holatini ko‘rish, kamera/namuna asboblarini olish, qaytish nuqtasini belgilash | Kerakli to‘plam bor, birinchi yo‘nalish tushunarli | 2–3 daqiqa |
| 2. ALSEP ilmiy stansiyasi | Seysmometrni joylashtirib tekislash, markaziy stansiya bilan bog‘lash, antennani Yer tomonga sozlash | Joylashuv va aloqa tasdiqlangan | 3–4 daqiqa |
| 3. Oy namunalarini olish | Head/12031 va Bench/12032 joylarida kontekst fotosi va yorliqli namuna; Sharp/12027 da kern asbobi | Uch namuna uchun ID, foto va yig‘ish qaydi bor | 6–8 daqiqa |
| 4. Surveyor tomon yurish | Xarita va relyef orqali yo‘l tanlash, krater yonbag‘rini kuzatish | Surveyor ish zonasiga yetib borish; yashirin yo‘l qulfi yo‘q | 3–4 daqiqa |
| 5. Surveyor qismlarini olish | Avval umumiy holat fotosi, keyin TV kamera, tuproq kuragi va kabel qismini tegishli joyidan yechib joylash | Uch detal va boshlang‘ich foto inventarda | 4–6 daqiqa |
| 6. Intrepidga qaytish | Yuk ro‘yxatini tekshirish, modul yonida dalillarni topshirish | Namunalar va qismlar saqlangan, natija ochiladi | 2–3 daqiqa |
| Natija | Bosilgan yo‘l, fotosuratlar va tarixiy izohni ko‘rish | Qayta o‘ynash yoki menyuga qaytish | 1–2 daqiqa |

Jami mo‘ljal: 21–30 daqiqa. Namuna joylariga borish tartibi o‘yinchi tanloviga ochiq qoladi. Vaqtni sun’iy cho‘zadigan majburiy kutish qo‘shilmaydi.

### 5.1. ALSEP: ko‘rsatkichni o‘zgartiradigan ish

Asbob yaqinida tekislashning sodda ko‘rsatkichi bo‘ladi. O‘yinchi tayanchni sozlaganda ko‘rsatkich markazga keladi. Antenna uchun yaqin ko‘rinish ochilib, sichqoncha bilan buriladi; radio signal holati matn va grafik bilan ko‘rinadi. Burchakning raqamini yodlash shart bo‘lmaydi. Hozirgi 353° qiymat soddalashtirilgan yo‘nalish ekanligi izohlanadi; aniq sana/soatli astronomik hisob qilinmaguncha u qat’iy tarixiy aniqlik sifatida berilmaydi.

Bu ALSEP ning to‘liq yig‘ish tartibi emas. Qaysi ikki amal o‘yin uchun soddalashtirilgani manbalar sahifasida yoziladi.

### 5.2. Namuna: uchta bir xil kutishdan voz kechish

- **Foto:** namuna va atrofidagi relyefni ko‘rsatadigan kadr. Ekranda zarur obyekt ko‘rinishi va masofa bo‘yicha sodda yordam. Foto sifati uchun noma’lum mezon bilan jazolash yo‘q.
- **Tosh:** qisqich yoki kurak bilan olish, qopga joylash, ID ni tasdiqlash. Kamera qo‘l holati bilan mos keladi.
- **Kern:** asbobni joylashtirish, qisqa boshqariladigan kiritish, chiqarish va qadoqlash. Tarixiy asbob ko‘rinishi alohida manba bilan tekshiriladi; zamonaviy burg‘i tasodifan ishlatilmaydi.
- **Yorliq:** joy, ID, foto va vaqt avtomatik bog‘lanadi. Qayta yuklaganda ham bog‘lanish yo‘qolmaydi.

12031, 12032 va 12027 identifikatorlari hamda joylari mavjud mapping ma’lumotidan olinadi. Kichik toshlarning shakli asl namunaning skani deb ko‘rsatilmaydi.

### 5.3. Yo‘l topish: o‘yinchining qarori

Hozirgi 12 ta majburiy yaqinlashish nuqtasi tarixiy marshrutni jurnalga ko‘rsatish uchun saqlanadi, ammo bosqichni ochish uchun har biridan o‘tish talabi olib tashlanadi. Ko‘mak yoqilganida bir vaqtda bitta manzil va zarur bo‘lsa 2–3 ta aniq oraliq yo‘nalish ko‘rsatiladi. Ular haqiqiy relyef va tekshirilgan joylar asosida qo‘yiladi.

Xaritada tarixiy yo‘l va o‘yinchining yo‘li turli belgi bilan ko‘rinadi. “Qisqa yo‘l” yoki “tekis yo‘l” yorlig‘i faqat haqiqiy masofa/nishab hisobidan keyin ishlatiladi. Relyef bunday farq bermasa, soxta tanlov yaratilmaydi. To‘g‘ridan-to‘g‘ri Surveyorga yetib kelgan o‘yinchi ham vazifani davom ettira oladi.

### 5.4. Surveyor: bobning asosiy voqeasi

Yaqinlashganda asbobning Oyda uzoq vaqt turgani va qismlarni Yerda tekshirish nima uchun kerakligi qisqa tushuntiriladi. Avval umumiy foto olinadi. Keyin kamera biriktirgichi, kurak va kabel alohida nishonga olinadi. Tanlangan detal cheti yengil ajralib ko‘rinadi, nomi chiqadi; qo‘l/asbob shu joyga yetadi. Tugagach detal qurilmadan yo‘qolib, yuk ro‘yxatida paydo bo‘ladi.

Har birida uzoq vint burash majburiy emas: qisqa, tushunarli fizik harakat yetarli. Bekor qilingan ish noto‘g‘ri “yig‘ildi” deb belgilanmaydi. Qurilma ta’mirlanmaydi va kislorod bermaydi. Bu harakatsiz Surveyor 3 dan ilmiy tekshiruv uchun qism olish voqeasidir. [NASA/JPL manbasi](https://www.jpl.nasa.gov/blog/2012/1/surveyor-3-camera-returned-from-the-moon).

### 5.5. Yakun va o‘rganilgan narsa

Natijada uch savolga javob ko‘rinadi: nima olib kelindi, bu nima uchun kerak, qaysi ma’lumot haqiqiy manbadan olingan? Kartalarda o‘yinchining fotosi, namuna/detal nomi va 1–2 jumla izoh beriladi. Yo‘l uzunligi real yurish qaydidan hisoblanadi. Uchish va Yerga qaytish qisqa tarixiy yakun sifatida tushuntiriladi; alohida boshqariladigan parvoz bosqichi qo‘shilmaydi.

## 6. Boshqaruv va interfeys

### 6.1. Tavsiya etiladigan standart tugmalar

| Tugma | Barqaror vazifa |
|---|---|
| WASD + sichqoncha | Yurish va atrofga qarash |
| Shift | Tezroq qadam; sozlamada ushlab turish/almashtirish |
| Space | Past gravitatsiyadagi qisqa sakrash |
| E | Yaqin obyektni tekshirish yoki ish rejimini ochish |
| Sichqonchaning chap tugmasi | Kamera/asbob ochilganda uning asosiy amali |
| Esc | Asbobdan chiqish; keyingi bosishda pauza. Menyuda ortga |
| Tab | Planshet: vazifa, xarita va dalillar; vaqt pauzada |
| V | Ixtiyoriy birinchi/uchinchi shaxs ko‘rinishi |

J va M mavjud foydalanuvchilar uchun planshet sahifalariga muqobil tugma bo‘lishi mumkin. Q bir paytda ham manzil, ham antenna boshqaruvi bo‘lmasligi kerak. Alt orbit va F zoom qo‘shimcha boshqaruv sifatida sozlamada turadi; ularni bilish missiyani tugatish uchun zarur bo‘lmaydi. Tugmalar Enhanced Input orqali qayta biriktiriladi; asbob va menyu kontekstlari yurish bilan to‘qnashmaydi.

### 6.2. HUD qoidasi

Ekranda bir vaqtning o‘zida bitta joriy topshiriq: **qayerga → nima qilish → nima uchun**. Qolgan ro‘yxat planshetda. Masalan: “Surveyor 3 — 42 m. Avval qurilmaning umumiy fotosini oling. Qismlarni yechishdan oldingi holat kerak.”

Pastda faqat yaqin ish ko‘rsatmasi: “E — kamerani tayyorlash”. Ish davomida borish ko‘rsatkichi, bekor qilish va natija aniq chiqadi. Navbatdagi vazifa avvalgisining tasdig‘ini bosib ketmaydi. Marker obyektni berkitmaydi; ekrandan tashqarida chet belgisi bilan ko‘rsatiladi. Uzoq ro‘yxat va radio matni bir-biri ustiga tushmaydi.

720p da asosiy matn taxminan 18–22 piksel ko‘rinishidan boshlanadi; yakuniy o‘lcham real ekran sinovi bilan belgilanadi. 100/125/150% matn o‘lchami, alohida subtitr kattaligi, rangdan tashqari belgi va yetarli kontrast bo‘ladi. Harakatdagi kamera tebranishini o‘chirish, sezgirlik va invert Y ham sozlanadi. Harakatni ushlab turish qiyin bo‘lsa, interactionni bosib boshlash/bosib bekor qilish varianti beriladi.

### 6.3. Planshet va yordam

Uchta asosiy bo‘lim: joriy vazifa/ko‘rsatma, xarita, dalillar. Tarix va manbalar alohida o‘qish sahifasida. Xarita shimol belgisi, masofa shkalasi, Intrepid, o‘yinchi va kerakli manzilni ko‘rsatadi. LROC tasviri “zamonaviy orbital ma’lumot” deb belgilangan bo‘ladi.

Yordam darajalari: oddiy ko‘rsatma → o‘yinchi so‘rasa batafsil izoh → uzoq turib qolsa yo‘nalish taklifi. Avtomatik yordamni o‘chirish mumkin. Vazifa bajarilmasligining sababi matnda aytiladi: masalan, “Avval umumiy foto oling”, “Asbobga yaqinroq boring”.

## 7. Kosmonavt, qo‘llar va yurish

Bu ish oddiy rang almashtirish bilan tugamaydi. Hozirgi statik bo‘laklar o‘rniga tarixiy A7L ko‘rinishiga asoslangan, yaxlit UV/materiallarga ega **Skeletal Mesh** kerak. Avval model, keyin rig va mos animatsiyalar, shundan keyin IK bilan yer/asbobga moslash ishlanadi.

### 7.1. Model talablari

- Tana, yelka, tirsak, bilak, son, tizza va etik tutashuvi uzilmasin. Bukilgan joylarda deformatsiya sinovi bo‘lsin.
- Shlem, oltin visor, ko‘krak ulanishlari, shlanglar, ryukzak va etiklar bir davrga mos reference bo‘yicha ishlansin. Zamonaviy skafandr qismlari aralashmasin.
- Oq mato tekis plastik bo‘lib ko‘rinmasin: tikuv, burma, roughness va normal detali me’yorida ishlatilsin.
- Birinchi shaxs qo‘llari va tashqi modelning rang/materiali mos bo‘lsin. Birinchi shaxs uchun ko‘rinadigan geometriya kamera bilan kesishmasin; uchinchi shaxsda tana yo‘qolmasin.
- Barmoqlar asbobni ushlasin, kaftdan chiqib turmasin. O‘ng/chap qo‘lning vazifasi animatsiyada izchil bo‘lsin.
- Model/animatsiya fayllarining kelib chiqishi, foydalanish sharti va tahrirlanadigan nusxasi loyiha bilan saqlansin.

### 7.2. Animatsiyalar to‘plami

Bo‘sh turish; oldinga/orqaga/yonlama yurish; tezroq Oy qadami; joyida burilish; sakrash, havoda holat va qo‘nish; cho‘kkalab ishlash; kamera ushlash/foto; namuna olish; kern asbobi; antenna bilan ishlash; Surveyor qismini olish; yukni joylash.

Animation Blueprint holatlar va o‘tishlarni boshqaradi. Oyoq IK yer balandligiga, qo‘l IK asbob tutqichiga moslaydi. Control Rig faqat shu tuzatishlarni bajaradi; u sifatli rig va asosiy animatsiya o‘rnini bosa olmaydi. Kapsula harakati, qadam uzunligi va animatsiya tezligi birgalikda sozlanadi. Avval oldinga yurish va to‘xtash mukammallashtiriladi, keyin qolgan yo‘nalishlar qo‘shiladi.

### 7.3. Qabul qilish mezonlari

1. Old, orqa va yon tomondan sekin/tez yurish, burilish, pastga qarash va sakrash yozuvlari ko‘rib chiqiladi.
2. Tekis joydagi tayanch fazasida etik sirpanishi uchun boshlang‘ich maqsad 5 sm dan kam; turli qiyaliklar alohida tekshiriladi. Bu sifat maqsadi, hozir o‘lchangan natija emas.
3. Kamera shlem, ko‘krak, qo‘l yoki asbob ichiga kirmaydi. Tashqi kamera devor/relyef ortidan o‘tib ketmaydi.
4. Relyefga tegishda oyoq to‘g‘ri turadi; haddan tashqari qiyalikda tizza cho‘zilib buzilmaydi. Yaroqsiz joyga turishni collision/movement cheklaydi.
5. O‘yinchi harakatni boshlash/to‘xtatishda boshqaruvni yo‘qotmaydi. Majburiy uzoq animatsiyadan Esc orqali xavfsiz chiqish qoidasi bor.
6. Bir xil sifat past grafik sozlamada ham saqlanadi. Ko‘rinmaydigan qo‘shimcha qo‘l yoki noto‘g‘ri soya chiqmaydi.

Oy gravitatsiyasi 1,62 m/s² saqlanadi. Animatsiya tarixiy videolar bilan solishtiriladi, lekin u aniq biomekanik/trening modeli deb taqdim etilmaydi.

## 8. Relyef, grafika va ovoz

**Ko‘rinish maqsadi:** ishonarli ilmiy ekspeditsiya, bir xil sifatdagi kichik hudud. Katta reklama sahnasi uchun kundalik o‘ynash qulayligi qurbon qilinmaydi.

- Mavjud NASA balandliklari saqlanadi; tasodifiy shovqin yoki sun’iy vertikal kattalashtirish qo‘shilmaydi. Mayda tosh, iz va chang alohida badiiy detaldir.
- Intrepid, Surveyor va ALSEP modellari masshtab, asosiy detallar, material va collision bo‘yicha bir xil darajaga keltiriladi. Avval interaction tutqichlari aniqlanib, keyin yakuniy model ishlanadi.
- Quyosh soyasi keskin bo‘ladi, lekin nishondagi detal mutlaqo qora bo‘lib qolmaydi. Yuzadan qaytgan yorug‘likning soddalashtirilgan yoritishi, material va yorqinlik kalibrlanishi bilan o‘qilish ta’minlanadi. Ichkarida/osmon ostida tasodifiy atmosferali tuman ishlatilmaydi.
- Chang qisqa, yerga qaytadigan zarrachalar bilan cheklanadi; shamol yoki uzoq suzuvchi atmosfera changi tasviri berilmaydi. Izlar dekorativ, ilmiy qayta tiklangan aniq iz emas.
- Skafandr ichidagi nafas, tana/asbob orqali eshitiladigan qadam va ish ovozi, radio filtri muhitni beradi. Tashqarida oddiy Yer shamoli ovozi ishlatilmaydi.
- Radio, effekt va musiqa ovozi alohida sozlanadi. Har bir muhim xabar subtitr va gapiruvchi nomi bilan beriladi. Musiqa majburiy bo‘lmaydi.
- Dastlab sifatli matn va qisqa signal yetarli; ovozli dialog keyin qo‘shiladi. Yangi yozilgan dialog original NASA radio yozuvi deb ko‘rsatilmaydi. Arxiv media ishlatilsa, aynan o‘sha faylning foydalanish sharti tekshiriladi.

## 9. Tarixiy va ilmiy aniqlik

Apollo 12 ning asosiy voqeasi — aniq qo‘nish, yuzadagi ilmiy ish va Surveyor 3 qismlarini olish. Shu voqea birinchi bobni boshqaradi. [NASA missiya tavsifi](https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/), [JPL apparat qismlari haqidagi maqola](https://www.jpl.nasa.gov/blog/2012/1/surveyor-3-camera-returned-from-the-moon).

| Ma’lumot turi | O‘yindagi qoida | Dalil |
|---|---|---|
| Hujjatlangan tarix | Ism, sana, missiya va asbob vazifasi tekshiriladi | Birlamchi NASA/ALSJ/JPL manbasi va tekshirish sanasi |
| O‘lchangan/reyestr ma’lumoti | DTM, joylar va namuna ID lari buzilmaydi | Xom fayl, metadata, hash, koordinata transformatsiyasi |
| Hisoblangan natija | Masofa, nishab va marshrut hisoblash usuli yoziladi | Birlik va aniqlik chegarasi, tekshirish namunasi |
| O‘yin talqini | Qisqartirilgan vaqt, yakka o‘yinchi, marker, stol, asbob harakati ochiq aytiladi | “O‘yin uchun soddalashtirilgan” izohi |

2 m DTM qadami 2 m mutlaq aniqlik degani emas. Kichik toshlar va skafandr detali bu DTM dan olinmagan. Zamonaviy LROC orbital suratini 1969 yilda ekipaj ishlatgan xarita deb ko‘rsatish mumkin emas. Orbital surat soyasi relyef materialining doimiy rangi sifatida ishlatilmaydi.

[LROC DTM](https://data.lroc.im-ldi.com/lroc/view_rdr/NAC_DTM_APOLLO12), [joy va marshrutlar arxivi](https://data.lroc.im-ldi.com/lroc/view_rdr/SHAPEFILE_APOLLO_12), [Gonzales va boshqalar, 2020, LPSC 1578](https://www.hou.usra.edu/meetings/lpsc2020/pdf/1578.pdf) bilan bog‘lanish `SourceArt/Apollo12/manifest.json` da saqlanadi. Mavjud tafsilotlar: [APOLLO12.md](APOLLO12.md).

Uchish paytidagi chaqmoq voqeasi xohishga ko‘ra qisqa kirishda ishlatilishi mumkin; NASA ma’lumotiga mos bo‘lishi kerak. U yangi katta cockpit darajasini talab qilmaydi. [NASA: Lightning Strikes Twice](https://science.nasa.gov/resource/lightning-strikes-twice/).

`MoonMission.docx` dagi Apollo 13, Skylab, ISS va Hubble ilhomlarini 1969 yilgi bitta haqiqiy missiyaga birlashtirmaymiz. Ular keyinchalik alohida tarixiy bob yoki ochiq belgilangan badiiy ekspeditsiya uchun g‘oya bo‘lishi mumkin. NASA logotipi yoki materialining ishlatilishi NASA tasdig‘i borligini anglatadigan shaklda bo‘lmaydi.

## 10. Saqlash, sozlamalar va tiklanish

- Asosiy bosqichdan keyin avtomatik checkpoint; pauzada “Davom ettirish”, “Oxirgi checkpoint”, “Sozlamalar”, “Menyu”. Yangi o‘yin mavjud saqlovni almashtirsa aniq ogohlantiradi.
- `USaveGame`: missiya/profil versiyasi, bosqich, bajarilgan ishlarning barqaror ID lari, namuna va detallar, fotolarga nisbiy yo‘l, vaqt/mashq rejimi, xavfsiz pozitsiya va ko‘rish yo‘nalishi. Actor pointer yoki massivning tasodifiy tartibi saqlanmaydi.
- Interaction natijasi faqat tugaganda tasdiqlanadi. Yarim yechilgan detalda saqlash zarur bo‘lsa, oxirgi barqaror holat saqlanadi; qayta kirganda yo‘qolgan yoki ikki marta olingan buyum chiqmaydi.
- Fayl avval vaqtinchalik nusxaga yozilib, keyin almashtiriladi; oldingi yaroqli nusxa tiklash uchun saqlanadi. Format versiyasi yangilanganda moslik yoki tushunarli xabar bo‘ladi.
- Foto fayli yo‘q bo‘lsa UI buzilmaydi: o‘rnida izoh chiqadi. Sozlamalar missiya saqlovidan alohida bo‘ladi.
- Til, ovoz, grafik profil, tugmalar, subtitr, matn kattaligi va kamera qulayligi qayta ishga tushganda saqlanadi. Offlayn paketda bular uchun server talab qilinmaydi.

## 11. Texnik tuzilma va mavjud kod bilan bog‘lanish

Refaktor bir martada barcha eski missiyalarni qayta yozish shaklida qilinmaydi. Har ajratilgan qismdan keyin Apollo hamda eski Moon/Mars tekshiruvlari bajariladi.

| Mavjud joy | Tavsiya etilgan o‘zgarish |
|---|---|
| `LunarGame.cpp/.h` | Character, input, kamera, UI va missiya vazifalarini bosqichma-bosqich ajratish; ishlayotgan oqimni saqlash |
| `LunarApollo.cpp` | Bosqich qoidalari va interaction natijasini UI matnidan ajratish; 12 majburiy yo‘l nuqtasi qulfini almashtirish |
| `LunarApolloHUD.cpp` | Qattiq Canvas koordinatalaridan UMG widgetlarga bosqichma-bosqich o‘tish; safe zone va DPI bilan tekshirish |
| `LunarSuit.cpp` | Statik tana qismlarini ishlab chiqarish rigiga almashtirish; kamera qoidalari va mavjud tekshiruvlarni saqlash |
| `LunarApolloChecks.cpp` | Yangi shartlar, asbob holati, save/reload va muqobil yo‘l holatlarini tekshirish |
| `LunarRescue.Build.cs` va `Config` | Kerak bo‘lganda EnhancedInput/UMG va animatsiya sozlamalarini qo‘shish; hozirgi modulda ular hali yo‘q |
| `SourceArt/Apollo12` va `Tools/prepare_apollo12.py` | Xom manba/manifestni saqlash; takroriy eksportda o‘lchov va yo‘nalishni tekshirish |
| `Tools/verify_apollo12.ps1` | Yangi hisobotlarni semantik tekshirishni davom ettirish; faqat process exit 0 ga ishonmaslik |
| `Tools/package_apollo12.ps1` | Toza paket, kerakli assetlar, offline manba va credits fayllarini tekshirish |

Yangi kichik bo‘limlar:

1. **Missiya profili/data asset:** manzillar, topshiriq ID lari, rejim, limitlar va manba metadata. Xarita nomining satridan qaysi missiya ekanini taxmin qilish kamaytiriladi.
2. **Interaction interface/component:** barqaror obyekt ID, yaqinlik/ko‘rinish, bajarilish sababi, nishon nuqtasi, boshlash/bekor qilish/tasdiqlash. Surveyor kabi yaqin qismlarda qarash yo‘nalishi ustuvor.
3. **Dalil va inventar holati:** foto, namuna ID, detal, joy va vaqt bog‘lanishi. UI va save bitta holatni o‘qiydi.
4. **UMG va lokalizatsiya:** matnlar `FText`/string table da, formatlash joy nomi va masofani alohida beradi. O‘zbek apostroflari va barcha gliflar paketdagi shrift bilan tekshiriladi.
5. **Skeletal character/Animation Blueprint:** yurish, ish holati, IK va kamera qatlamlari. Instrument animatsiyasi gameplay natijasini o‘zi yashirincha belgilamaydi.

Bitta bob uchun haddan tashqari umumiy quest tizimi yoki yangi tarmoq arxitekturasi kerak emas. Mavjud terrain meshni Landscape/World Partition ga ko‘chirish faqat o‘lchangan muammo buni talab qilsa qilinadi.

Texnik asos: [Epic — Animation Blueprints](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprints-in-unreal-engine), [Full Body IK](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-full-body-ik-in-unreal-engine), [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine), [SaveGame](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine), [Localization](https://dev.epicgames.com/documentation/en-us/unreal-engine/localization-overview-for-unreal-engine). Bular 2026-10-24 da tekshirilgan yo‘nalishlar; loyiha integratsiyasi hali bajarilmagan.

## 12. Grafik tezlik va qurilma maqsadi

Boshlang‘ich nishon: hozirgi 16 GB RAM / 4 GB VRAM sinfidagi Windows kompyuter, DX11, 1280×720 Low. Aniq CPU/GPU modeli benchmark hisobotiga yoziladi; RAM/VRAM hajmining o‘zi yetarli tavsif emas.

**O‘lchanishi kerak bo‘lgan maqsadlar:** o‘rtacha 60 FPS, 1% low kamida 45 FPS. Bu hozir erishilgan natija emas. Xuddi bir yo‘l, kamera, grafik profil va yig‘ilgan paket bilan frame time, RAM va VRAM yoziladi. Birinchi shader/load kutishi alohida, tayyorlangan yurish o‘lchovi alohida beriladi. VSync/FPS cap holati hisobotda bo‘ladi.

Boshlang‘ich xotira budjeti: o‘yin jarayoni RAM i taxminan 4 GB gacha, umumiy GPU xotirasi 3,2 GB atrofidan oshmasligi. Profilingga qarab bu budjet qayta ko‘riladi. Hozirgi 384 MB texture streaming pool umumiy VRAM sarfi degani emas.

Modelda LOD, yaqin/uzoq tekstura o‘lchami va kamroq material bo‘limi ishlatiladi. Skafandrning LOD0 varianti uchun 100 ming uchburchak atrofidagi dastlabki chegara ko‘rib chiqiladi; haqiqiy xarajat draw call, material va animatsiya profilida aniqlanadi. Nanite/Lumen yoki qimmat effektlar faqat zarur natija va qurilma imkoniyati bilan oqlansa qo‘shiladi. Hozirgi DX11 yo‘li birinchi reliz uchun asos bo‘lib qoladi.

## 13. Ishlar ro‘yxati va qabul qilish dalili

**P0:** birinchi tugallangan bob uchun shart. **P1:** bobni sifatli qilish uchun keyingi ishlar. **P2:** birinchi relizdan keyingi imkoniyat. “Qabul qilish dalili” ustuni vazifa haqiqatan tugaganini ko‘rsatadi; bular mavjud natija emas.

| ID | Daraja | Ish | Bog‘liqlik | Qabul qilish dalili |
|---|---|---|---|---|
| BASE-01 | P0 | v0.4.1 source/content/package holatini qayta tiklanadigan baseline qilish | Yo‘q | Versiya, LFS va asset ro‘yxati; mavjud natijalar bog‘langan |
| UX-01 | P0 | Bir topshiriqli HUD va aniq E ko‘rsatmasi | BASE-01 | 720p da ustma-ust tushmaydigan barcha olti bosqich suratlari |
| UX-02 | P0 | Dastlabki 90 soniya tutoriali | UX-01 | Kamida 4/5 yangi o‘yinchi birinchi ishni yordamsiz bajaradi |
| UX-03 | P0 | English matn va barqaror string table | UX-01 | Butun missiya inglizcha; kesilgan matn yoki yoqolgan glif yoq |
| UX-04 | P0 | Input kontekstlari, Esc va hold/toggle | UX-02 | Asbob/menyu/yurish to‘qnashmaydi; almashtirilgan tugma ishlaydi |
| UX-05 | P0 | 12 majburiy yo‘l nuqtasi o‘rniga yumshoq navigatsiya | UX-01 | Tarixiy yo‘l va bevosita yo‘l bilan Surveyor bosqichi ochiladi |
| UX-06 | P0 | Matn/subtitr, kamera va ovoz sozlamalari | UX-03, UX-04 | 100–150% matn, kamera tebranishi o‘chiq holatda bob tugaydi |
| SYS-01 | P0 | Barqaror missiya/obyekt ID va holat ajratilishi | BASE-01 | Bir xil natijani gameplay, jurnal va save ko‘rsatadi |
| SAVE-01 | P0 | Dastlabki checkpoint va Continue oqimi | SYS-01 | Intrepid/ALSEP dan qayta ochish va sozlamalarni saqlash |
| SAVE-02 | P0 | Barcha dalil, foto va detallarni saqlash | SAVE-01, PLAY-01 | Har katta bosqichda reload; buyum takrorlanmaydi/yo‘qolmaydi |
| ART-01 | P0 | A7L reference va sifatli model/rig manbasini belgilash | BASE-01 | Old/yon/orqa eskiz, foydalanish sharti, tahrirlanadigan model |
| ART-02 | P0 | Skeletli tana, qo‘l, barmoqlar va mos material | ART-01 | Yelka/tirsak/tizza bukilish sinovi, yaxlit tutashuvlar |
| ANIM-01 | P0 | Yurish, to‘xtash, yonlama va burilish | ART-02 | Front/profile video, tayanchda oyoq sirpanishi o‘lchovi |
| ANIM-02 | P0 | Sakrash/qo‘nish va oyoq IK | ANIM-01 | Relyef/qiyalik sinovi, noto‘g‘ri cho‘zilish va teshilish yo‘q |
| ANIM-03 | P0 | Kamera, qo‘l IK va asbob pozalari | ART-02, SYS-02 | Birinchi/uchinchi shaxsda bir xil asbob va to‘g‘ri tutqich |
| SYS-02 | P0 | Interaction component va cancellation qoidasi | SYS-01 | Qaralgan yaqin detal tanlanadi; bekor qilingan ish tasdiqlanmaydi |
| PLAY-01 | P0 | Inventar/dalil modeli va jurnal kartasi | SYS-01 | Foto + ID + joy bog‘lanishi barcha uch namunada to‘g‘ri |
| PLAY-02 | P0 | Foto va tosh olish interactioni | PLAY-01, ANIM-03 | Kadr, yig‘ish va qadoqlash o‘yinchiga ko‘rinadi |
| PLAY-03 | P0 | Kern uchun alohida asbob harakati | PLAY-02 | Oddiy tosh bilan bir xil kutish bo‘lmaydi, natija qayd etiladi |
| PLAY-04 | P0 | ALSEP tekislash va antenna yaqin ko‘rinishi | SYS-02, UX-04 | Noto‘g‘ri holat sababi va to‘g‘ri holat tasdig‘i bor |
| PLAY-05 | P0 | Surveyor foto va uch detalni ajratish | PLAY-01, ANIM-03 | Har detal modeldan inventarga bir marta o‘tadi |
| PLAY-06 | P0 | Qaytish, topshirish va mazmunli debrief | PLAY-02–05 | Yo‘l/foto/yuk haqiqiy session holatidan ko‘rsatiladi |
| PLAY-07 | P0 | Vaqtsiz tarixiy rejim va alohida mashq vaqti | SYS-01 | Pauza/reload vaqtni buzmaydi; limit suit O2 deb yozilmaydi |
| WORLD-01 | P1 | Intrepid/Surveyor/ALSEP model va collision sifati | SYS-02 | Masshtab, tutqich, ko‘rinish va yurish sinovi |
| WORLD-02 | P0 | Soyadagi o‘qilish va brightness kalibrlash | UX-01 | Muhim detal Low profilda yorug‘/soya holatda taniladi |
| WORLD-03 | P1 | Mato/metall/regolit va cheklangan iz/chang | ART-02, PERF-01 | Bir xil art yo‘nalishi; tezlik budjetini buzmaydi |
| AUDIO-01 | P1 | Qisqa radio, asbob feedback va subtitr | PLAY-06, UX-03 | Muhim ko‘rsatma ovozsiz ham tushunarli; takroriy spam yo‘q |
| HIST-01 | P0 | Fakt, talqin, media va data credits auditi | PLAY-06 | Har tarixiy da’vo/manba bog‘langan; barcha assetlar qayd etilgan |
| PERF-01 | P0 | Paketdagi belgilangan yo‘l profilingi | BASE-01 | CPU/GPU/profil/frame time/RAM/VRAM hisobotda |
| PERF-02 | P0 | LOD, material va effektlarni budjetga moslash | ART-02, WORLD-01, PERF-01 | Maqsad qurilmada qayta o‘lchov, grafik buzilish yo‘q |
| QA-01 | P0 | Progression, bekor qilish, save va regressiya | PLAY-06, SAVE-02 | Semantik 0 failures; eski Moon/Mars/menu tekshiruvlari |
| QA-02 | P0 | 5–8 yangi odam bilan kuzatuvli playtest | UX-06, QA-01 | Yordam berilgan joylar/vaqt/tushunish yozuvi va tuzatishlar |
| REL-01 | P0 | Toza offline Win64 paket | QA-02, PERF-02, HIST-01 | Editorschiz ochiladi, Continue ishlaydi, credits bilan ko‘chiriladi |
| EXTRA-01 | P2 | Qisqa chaqmoq prologi yoki qo‘shimcha tarixiy foto | REL-01 | Alohida media manbasi, o‘tkazish imkoniyati |
| EXTRA-02 | P2 | Keyingi mustaqil tarixiy/badiiy bob | REL-01 | Alohida tadqiqot, scope va smeta; birinchi bobni buzmaydi |

Har P0 vazifa alohida ko‘rib chiqiladigan o‘zgarish sifatida bajariladi. Funksiya ishlashi bilan birga foydalanuvchi ko‘radigan natija ham tekshiriladi. Oddiy matn o‘zgarishiga keraksiz avtomatik test yozilmaydi; holat/progression/save kabi xatarli qoidalarga mazmunli tekshiruv qo‘shiladi.

## 14. Bajarish tartibi va taxminiy muddat

Quyidagi taxmin **bitta Unreal dasturchi va qisman vaqt ajratadigan 3D model/animatsiya mutaxassisi** uchun. Mavjud assetning mosligi, ishga ajratiladigan soat va playtest natijasi muddatni o‘zgartiradi. Bu kafolatlangan topshirish sanasi emas. Yakka ishlab, model va animatsiyani ham boshidan yaratishda 12–18 hafta yoki ko‘proq vaqt rejalashtirilishi mumkin.

| Bosqich | Taxminiy davomiylik | Ish va mas’ul yo‘nalish | Ko‘rib chiqiladigan natija |
|---|---|---|---|
| 0. Holatni mahkamlash | 2 ish kuni | Dasturchi: baseline, ochiq xatolar, profiling yo‘li; dizayn/art reference | Tiklanadigan v0.4.1 va ustuvor xatolar ro‘yxati |
| 1. Tushunarli o‘ynash | 1 hafta | Dasturchi/dizayn: HUD, tutorial, matn, yo‘l qulfi; dastlabki save; artist reference | Dastlabki 3 daqiqa va to‘liq marshrut aniq; birinchi novice sinov |
| 2. Kosmonavt | 2–3 hafta | Artist: model/rig/animatsiya; dasturchi: input, kamera, IK integratsiyasi | Yurish va qo‘l/asbob ko‘rinishi tasdiqlangan character build |
| 3. Mazmunli ilmiy ish | 2–3 hafta | Dasturchi/animator: foto, namunalar, kern, ALSEP, Surveyor, inventar/save | Olti bosqich takroriy holdlardan tashqari ishlaydi |
| 4. Taqdimot va tezlik | 1–2 hafta | Art/audio/UI: obyektlar, yoritish, material, radio, debrief; profiling | Tashqi sifat bir xil, maqsad qurilma natijalari yozilgan |
| 5. Sinov va relizga tayyorlash | 1–2 hafta | Playtester/dasturchi: yangi foydalanuvchi, xatolar, offline paket | Reliz nomzodi va qabul qilish hisobotlari |

Jami rejalash oralig‘i taxminan **8–12 hafta**. Bir haftalik hisob besh ish kuniga asoslangan; bosqichlarning yig‘indisi yaxlitlangan. Bu jamoa mavjudligi faraziga bog‘liq. Har bosqichdan keyin kichik o‘ynaladigan paket bo‘ladi. Muhim refaktor/asset importdan oldin tiklanadigan nusxa olinadi; tasodifiy uncommitted ishlar yo‘qotilmaydi.

Muhim bog‘liqliklar: rig tayyor bo‘lmasdan yakuniy yurish chiqmaydi; interaction nuqtalari belgilanmasdan asbob qo‘li yakunlanmaydi; dalil modeli bo‘lmasdan yakuniy save chiqmaydi; inson playtesti bo‘lmasdan tushunarlilik tasdiqlanmaydi. Model ishi kechiksa HUD, save va interaction qoidalarini vaqtinchalik model bilan davom ettirish mumkin.

Bir haftalik tartib: hafta boshida bitta asosiy natijani tanlash, o‘rtada ishlaydigan variantni ko‘rish, oxirida paket va qisqa sinov. Bir ishni tugatmasdan ko‘p yangi tizim boshlash kamaytiriladi. Reja o‘zgarsa sabab va ta’sir shu hujjatga yoziladi.

## 15. Sinov rejasi

### 15.1. Avtomatik va texnik tekshiruv

- Olti bosqich to‘g‘ri tartibda ochiladi; namuna tartibi erkin, Surveyorga muqobil yo‘l ishlaydi.
- Interactionni qo‘yib yuborish, masofadan chiqish, boshqa obyektga qarash, pauza va Esc holatlari tekshiriladi. Ikki marta mukofot/buyum berilmaydi.
- Foto talabi, uch detal tanlovi, antenna sharti va inventar holati mos keladi.
- Har checkpointdan save/reload; yo‘q foto, eski/buzuq save va qayta boshlash holati tushunarli boshqariladi.
- Menyu, eski Moon va Mars regressiyalari saqlanadi. Sinovning yangi semantik hisobotida xato yo‘qligi tekshiriladi.
- DTM transformatsiyasi, birlik, collision balandligi va asosiy koordinatalar tekshiriladi. Assetning hash/manba qaydi saqlanadi.
- Editor ishlamagan holatda yakuniy paket ochiladi; kerakli map/material/asbob, font, audio va credits mavjud.

### 15.2. Qo‘lda vizual tekshiruv

720p, 1080p va bitta keng ekran formatida; o‘zbek/ingliz matni va kattalashtirish; yorug‘lik/soya; birinchi/uchinchi shaxs; tekis va qiya yer; barcha qo‘l asboblari. Har bosqichda HUD hamda character surat/video yozuvi olinadi. Faqat chiroyli bitta frontal surat bilan qabul qilinmaydi.

### 15.3. Yangi foydalanuvchi bilan tekshiruv

5–8 kishilik kichik guruh: o‘yinni ilgari ko‘rmagan odamlar, turli o‘yin tajribasi. Bu guruh statistik umumlashma uchun emas, yirik tushunarlilik xatolarini topish uchun.

Kuzatuvchi avval gapirmaydi. Birinchi harakatgacha vaqt, noto‘g‘ri tugmalar, yo‘l yo‘qotish, yordam so‘rash, qo‘l/asbob ko‘rinishi va noqulay kamera holati qayd qilinadi. Oxirida o‘yinchidan Surveyor qismlari nima uchun olingani va qaysi ma’lumot haqiqiy bo‘lganini o‘z so‘zi bilan tushuntirish so‘raladi.

Boshlang‘ich qabul maqsadlari: 4/5 birinchi ishni yordamsiz bajaradi; kamida 4/5 bobni og‘zaki yordam talab qilmasdan yakunlaydi va Surveyor vazifasini tushuntira oladi. Beshdan ko‘p odam sinov qilinsa kamida 80% mo‘ljal olinadi. O‘tish vaqti tarqalishi va yordam soni yoziladi; taxminiy 20–30 daqiqaga mos kelmasa yo‘l/ishlar qayta sozlanadi. Bular kelajakdagi mezonlar, mavjud natijalar emas.

## 16. Asosiy xatarlar va javob

| Xatar | Amaliy javob |
|---|---|
| Rig/model sifati yetmasligi | Xarid/importdan oldin bukilish va yurish sinovi; manba fayl va foydalanish sharti bilan ishlash |
| Skafandrni tuzatish muddati uzayishi | Avval bitta yaxshi yurish va asbob tutish; qolgan animatsiyalar shu asosda |
| Kontent haddan tashqari kengayishi | Birinchi bobning P0 ro‘yxati yakunlanmaguncha yangi planeta/bob boshlamaslik |
| “Real NASA” nomi ostida badiiy detalni fakt qilish | Fakt–hisob–talqin jadvali, manba qaydi va qisqa UI izohi |
| Qorong‘i grafika vazifani yashirishi | Muhim obyektlarni Low/soya holatida tekshirish, brightness va marker yordam |
| 4 GB GPU da xotira/tezlik muammosi | Erta paket profilingi, LOD va material budjeti; effektlarni keyin qo‘shish |
| Katta C++ fayllari sabab regressiya | Kichik ajratishlar, eski tekshiruvlar, oraliq paketlar |
| Lokalizatsiya/katta matn HUD ni buzishi | FText va responsive UI; har bosqichni ikki tilda tekshirish |
| Arxiv media/asset kelib chiqishi noaniqligi | Har faylga manba/foydalanish qaydi; zarur bo‘lsa o‘zimiz yozgan alternativ |
| Save o‘zgarishida progress yo‘qolishi | Format versiyasi, barqaror ID, oldingi yaroqli nusxa va reload sinovi |

Aniq pul smetasi hozir belgilanmaydi: model/rig mavjudligi va kim bajarishi aniqlanmasdan narx ishonchli bo‘lmaydi. Smeta soat/asset ro‘yxati asosida alohida tuziladi; hozirgi reja pulli xizmat yoki xaridni boshlamaydi.

## 17. “Tugallangan” deb qabul qilish

Quyidagi shartlar birgalikda bajarilganda birinchi bob tayyor hisoblanadi:

- Yangi o‘yinchi menyudan kirib, asosiy boshqaruv va birinchi ishni mustaqil tushunadi.
- Olti bosqichda yo‘l yoki interaction sabab to‘xtab qolish yo‘q; barcha dalillar Intrepidga qaytadi.
- Skafandr yaxlit ko‘rinadi; qo‘l, asbob, oyoq va kamera barcha asosiy holatlarda tekshirilgan.
- Matnlar o‘qiladi; asosiy topshiriq ovoz va rangga qaram bo‘lmaydi; til/sozlama saqlanadi.
- Continue/checkpoint ishlaydi; mavjud progress yo‘qolishi yoki buyum takrorlanishi kabi kritik xato yo‘q.
- Maqsad kompyuterda ishlash tezligi o‘lchangan; maqsadga yetmagan joy va grafik sozlama ochiq ko‘rsatilgan.
- Tarixiy da’vo, NASA data va o‘yin talqini farqi aniq, offline manbalar va asset credits mavjud.
- Avtomatik tekshiruv, qo‘lda tasvir ko‘rigi va yangi foydalanuvchi sinovi qaydlari bor.
- Butun Windows papkasi ko‘chirilganda Unreal Editorsiz va internetsiz ishlaydi.

Ommaviy joylashtirish/release yuklash ushbu reja yozilishi bilan bajarilmaydi; bu yerda mahalliy reliz nomzodini tayyorlash rejalashtirilgan.

## 18. Birinchi navbatdagi aniq ish paketi

Keyingi implementatsiya boshlanganda tartib:

1. Baseline va bir xil yo‘lda grafik o‘lchov olish; mavjud regressiyalarni qayd etish.
2. Ekrandagi ortiqcha ro‘yxatni planshetga ko‘chirish, joriy vazifani o‘zbekcha va aniq qilish.
3. Dastlabki uch daqiqani qayta qurish va majburiy 12 nuqta qulfini olib tashlash.
4. Asosiy input/cancel qoidasi va boshlang‘ich checkpointni ulash.
5. A7L model/rig reference, qo‘l tutqichi va bitta yurish animatsiyasini tasdiqlash.
6. Kichik paketni yangi o‘yinchiga berib kuzatish; topilgan chalkashliklarni keyingi ishga kiritish.

Shundan keyin character va alohida ilmiy interactionlar yuqoridagi bog‘liqliklar bo‘yicha yakunlanadi. Keyingi katta kengayish faqat birinchi bob ushbu qabul mezonlaridan o‘tgach rejalashtiriladi.
