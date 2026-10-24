# LunarRescue 0.4.2 — tushunarli boshlanish

2026-10-24. Rejaning 0-bosqichi va 1-bosqichidagi dastlabki o‘ynaladigan o‘zgarishlar.

## O‘yinchi ko‘radigan o‘zgarishlar

- Apollo 12 va bosh menyu o‘zbekcha ochiladi. **L** inglizcha/o‘zbekcha tilni almashtiradi; tanlov keyingi ochilish uchun saqlanadi. Eski Moon/Mars boblarining ichki interfeysi inglizcha qolgan.
- Ekranda bitta joriy vazifa, keyingi harakat va uning ilmiy maqsadi ko‘rsatiladi. Oldingi olti qatorli yon ro‘yxat jurnalga ko‘chirildi.
- Matn, nishon nomi va E ko‘rsatmasi kattalashtirildi. Marker ostida fon bor; antenna hozirgi burchagini ham ko‘rsatadi.
- Boshlanishdagi yordam ketma-ket qarash, yurish va ishlashni tanishtiradi. Bu yordam erkin o‘ynashni to‘sib qo‘ymaydi va birinchi ish tugagach yo‘qoladi.
- **Tab / J** xarita va jurnalni ochadi. **Q** jurnal sahifasini almashtiradi, **Esc** yopib o‘yinga qaytaradi. Jurnal vaqt va yurishni pauzaga qo‘yadi.
- Surveyor 3 ga o‘yinchi tanlagan yo‘l bilan borish mumkin. Uning ish zonasiga yetib kelish keyingi bosqichni ochadi. O‘n ikki nuqtani majburan ketma-ket bosib o‘tish talabi olib tashlandi; tarixiy marshrut xaritada saqlangan.
- Yaqin qismlar uchun markerlar kamera, tuproq kuragi va kabelni nomi bilan ajratadi. Radio va natija matnlari ham ikki tilda.

Ochish: `Play Lunar Rescue.cmd` yoki `Dist/Apollo12/Windows/LunarRescue.exe`. Windows papkasini barcha ichki fayllari bilan birga saqlash kerak.

## Saqlangan boshlang‘ich holat

Mahalliy zaxira: `Backups/before-onboarding-20261024-025542`. Unda source, config, content, source art, tools, editor moduli, v0.4.1 Windows paketi va tekshiruv qaydlari bor: **818 fayl, 2 083 429 741 bayt**. `baseline-manifest.json` har faylning SHA-256 qiymatini, Git HEAD ni va chiqarib tashlangan generatsiya kataloglarini qayd etadi. Git holati va tracked o‘zgarishlarning binary patchi ham saqlangan. Unreal Engine va build vositalari bu nusxaga kiritilmagan.

O‘zgartirishdan oldingi Windows paketida Apollo, eski Moon, Mars, menyu va to‘liq avtomatik yurish qayta tekshirildi; xato qayd etilmadi. Zaxira yaratish mavjud foydalanuvchi ishlarini commit, reset yoki revert qilmagan.

## Texnik amalga oshirish

`LunarApolloStrings.inl` ichida **139 ta barqaror kalitli o‘zbekcha/inglizcha yozuv** bor. `LunarApolloText` ularni kodda yaratiladigan Unreal String Table larga ro‘yxatdan o‘tkazadi. `FText::Format` hisoblagich, masofa va burchaklarni tarjima matniga biriktiradi. Barcha ikki tilli format argumentlari bir-biriga mosligi tekshirildi. Alohida CSV yoki qo‘shimcha internet yuklash talab qilinmaydi.

Bu bosqichda ikkita aniq til jadvali qo‘llanadi; butun loyihaning GatherText/locres jarayoni va UMG ga ko‘chishi hali tugallanmagan. Asos: [Epic String Tables hujjati](https://dev.epicgames.com/documentation/unreal-engine/using-string-tables-for-text-in-unreal-engine).

Til afzalligi `GameUserSettings.ini` dagi `LunarRescue.Accessibility/Language` ga yoziladi. Avtomatik input sinovi foydalanuvchining til afzalligini almashtirib yubormaydi. Tekshirish uchun `-ApolloLanguage=uz` yoki `-ApolloLanguage=en` ishlatiladi.

Tarixiy yo‘lni o‘tuvchi bot uchun `-ApolloHistoricalRoute` faqat tekshiruv yo‘nalishini tanlaydi. Oddiy o‘yinchiga yashirin waypoint qulfi qaytarilmaydi.

## Tekshiruvlar

Editor kompilyatsiyasi va Windows build/cook/package o‘tdi. Ikkala muhitda bir xil tekshiruvlar bajarildi:

| Yo‘nalish | Editor o‘tgan | Windows paketi o‘tgan | Xato |
|---|---:|---:|---:|
| Apollo bosqichlari, til va boshlang‘ich yordam | 57 | 57 | 0 |
| Eski Moon | 50 | 50 | 0 |
| Mars | 54 | 54 | 0 |
| Menyu va xaritalar orasidagi o‘tish | 7 | 7 | 0 |
| Skafandr/input/kamera, Tab, Esc va L | 14 | 14 | 0 |
| **Jami** | **182** | **182** | **0** |

Inglizcha rejimda qo‘shimcha 57 ta Apollo va 14 ta input tekshiruvi o‘tdi. To‘liq yurishlar ham editor, ham paketda yakunlandi. Paket logi loyiha versiyasini 0.4.2 deb tasdiqladi.

Ikki to‘liq avtomatik yurish:

| Yo‘nalish | Masofa | Simulyatsiya vaqti | Natija |
|---|---:|---:|---|
| Surveyorga bevosita tanlangan yo‘l | 1 237,29 m | 932,13 s | Barcha namuna, asbob va qismlar bilan yakunlangan |
| Tarixiy marshrutdan foydalanish | 1 344,67 m | 1 009,13 s | Barcha namuna, asbob va qismlar bilan yakunlangan |

Bu yurishlar haqiqiy controller/movement/collision dan foydalanadi, ammo tezlashtirilgan headless simulyatsiyada bajariladi. Ular insonning o‘tish vaqti yoki GPU tezligini o‘lchamaydi. Har sinovda yangi semantik `RESULT: 0 failures` hisoboti tekshiriladi; process exit 0 ning o‘zi yetarli emas.

720p o‘zbekcha interfeysning 16 ta holati qo‘lda ko‘rib chiqildi: kirish, olti bosqichning tegishli ekranlari, jurnalning to‘rt sahifasi, natija, tutorial va ish ko‘rsatkichlari. Screenshotlar holatlarni maxsus tayyorlaydigan vizual fixture lardir; ulardagi vaqt va masofa haqiqiy playtest natijasi emas.

Paketda shu 16 holat 1280×720/o‘zbekcha, 1920×1080/inglizcha va 2560×1080/o‘zbekcha o‘lchamlarida render qilindi. Katta ekranlardagi asosiy oynalar ham ko‘rib chiqildi. Tekshiruv har PNG ning haqiqiy o‘lchamini o‘qiydi va eski suratlarni natija deb qabul qilmaydi. Dastlab Unreal katta o‘lchamni 888×500 ga avtomatik kichraytirgani topildi; `-ForceRes` qo‘shilib, sinovlar haqiqiy o‘lchamda qayta bajarildi. Bosh menyuning o‘zbekcha ko‘rinishi ham paketda tekshirildi.

Content/SourceArt dagi 357 ta fayl baseline SHA-256 qiymatlari bilan solishtirildi: o‘zgarish topilmadi. NASA balandliklari, mapping ma’lumoti va mavjud modellar saqlangan.

Mahalliy dalillar `Reports/Onboarding`, `Reports/editor-apollo12-*-result.txt` va build loglarida saqlanadi.

## Dastlabki grafik o‘lchov

v0.4.1 paketida Ryzen 5 6600H / RTX 3050 Laptop GPU / 16 GB RAM kompyuterda 1280×720, DX11, o‘sha paytdagi grafik sozlamalar, VSync o‘chiq va 60 FPS cheklovi bilan 4 200 kadr yozildi. Dastlabki 600 kadr chiqarilib, qolgan 3 600 kadr tahlil qilindi:

- O‘rtacha: **60,00 FPS**; 1% low: **57,11 FPS**.
- P95 kadr vaqti: **17,11 ms**; P99: **17,37 ms**.
- Namuna davomiyligi: taxminan **60 soniya**.

Bu render ishlagan, offscreen, oddiy delta time dagi boshlang‘ich yo‘l o‘lchovidir. Low preset, butun missiya, ko‘rinadigan oynadagi present jarayoni yoki inson boshqaruvi bo‘yicha umumiy kafolat emas. DX11 dagi nol GPU-memory CSV hisoblagichlaridan VRAM sarfi haqida xulosa chiqarilmadi. Oldingi profil natijasi yangi v0.4.2 tezligi deb ko‘rsatilmaydi.

**v0.4.2 ning alohida yangi o‘lchovi:** xuddi shu 4 200/600/3 600 kadr tartibida o‘rtacha **60,00 FPS**, 1% low **56,15 FPS**, P95 **17,20 ms**, P99 **17,47 ms** qayd etildi. Namuna 60,00 soniya. Bir soniyalik kuzatuvdagi eng katta process working set taxminan 575 MiB bo‘ldi; bu umumiy VRAM yoki aniq cho‘qqi xotira sarfi emas. Ikkala profil ham 60 FPS cheklovida; ulardan tezlik oshgan degan xulosa chiqarilmaydi. Natijalar va muhit `Reports/Performance/v041-baseline` hamda `Reports/Performance/v042` da.

Qayta o‘lchash: `Tools/profile_apollo12.ps1 -Label v042`. `Tools/summarize_profile.py` startup va metadata qatorlarini kadrlar bilan aralashtirmaydi; kutilgan kadr sonini tekshiradi. 1% low — eng sekin 1% kadrlarning o‘rtacha ms qiymatiga teskari FPS.

## Hali bajarilmagan ishlar

Bu to‘liq 1-bosqich yakuni emas. Checkpoint/Continue, to‘liq sozlamalar, qayta biriktiriladigan Enhanced Input, matn kattaligi tanlovi va UMG hali navbatda. Dastlabki uchta checkout harakati hozircha mavjud stol yonidagi E ushlab turish oqimidan foydalanadi; ularni real panel/asbob harakatlariga almashtirish keyingi ish.

Skafandr hozircha v0.4.1 dagi statik bo‘laklar va dasturiy holatlardan iborat. Skeletli A7L modeli, yakuniy yurish animatsiyalari, asbobni ushlaydigan qo‘l IK, alohida kern ishlatish va Surveyor qismini fizik yechish hali tayyor deb belgilanmagan. Qorong‘i soyalar va sodda qurilma modellari ham keyingi art bosqichiga kiradi.

Asosiy o‘yinda hozirgi 30 daqiqalik mashq cheklovi saqlangan; vaqtsiz tarixiy rejim keyingi alohida o‘zgarish. 5–8 ta yangi foydalanuvchi bilan sinov hali o‘tkazilmagan. Shuning uchun “4/5 kishi yordamsiz tushundi” mezoni bajarildi deb aytilmaydi.

Keyingi ketma-ket ish: missiya/obyektlar uchun barqaror ID va holatni ajratish, dastlabki checkpoint/Continue, so‘ng skafandrning model–rig–animatsiya bosqichi.
