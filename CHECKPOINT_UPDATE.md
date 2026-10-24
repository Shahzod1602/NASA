# LunarRescue — checkpoint va davom ettirish, v0.4.3

Apollo 12 ekspeditsiyasi uchun mahalliy saqlash tizimi. Bu bosqich saqlash va qayta tiklashni qo‘shadi; skeletli skafandr, yangi ilmiy harakatlar, UMG va to‘liq sozlamalar keyingi ishlar.

## Qanday ishlatiladi

- Ekspeditsiya boshlanganda va har bir ish yakunlanganda avtomatik checkpoint yaratiladi: uchta checkout ishi, ikki ALSEP asbobi, kontekst fotosuratlari, uch namuna, Surveyor hududiga yetish, uch detal va yukni topshirish.
- Bosh menyu yoki Apollo kirish ekranida **C** — davom ettirish. Tugallangan saqlov bo‘lsa natija ko‘rsatiladi. Mashq vaqti tugaganda ham **C** bilan oxirgi checkpoint qayta ochiladi.
- **Esc** bilan pauza, so‘ng **R** — oxirgi checkpointni yuklash. Qolgan vaqt, bosib o‘tilgan masofa va bajarilgan ishlar o‘sha saqlovdagi qiymatga qaytadi.
- Mavjud saqlov bilan yangi ekspeditsiya boshlash alohida tasdiq chiqaradi: **Enter** — yangisini boshlash, **Esc** — bekor qilish. Tasodifiy qo‘shimcha sichqoncha bosishi tasdiqni qabul qilmaydi.

Bu checkpoint tizimi: o‘yin yurish vaqtida yopilsa, aynan chiqish nuqtasiga emas, oxirgi yakunlangan ish joyiga qaytiladi. Yarim bajarilgan E harakati, havodagi tezlik va pauza holati tiklanmaydi. Joylashuv haqiqiy terrain collision orqali tekshiriladi va kosmonavt yerda tiklanadi.

## Nimalar saqlanadi

Doimiy nomlangan 17 ta ish identifikatori, rejim va ma’lumot profili, ekspeditsiya ID si, vaqt/masofa, antenna burchagi, tanlangan manzil, boshlang‘ich yordam holati, foto ID/joy/vaqt/fayl yo‘li. Aktyor pointerlari yoki enum/array tartibi diskka yozilmaydi. Namunalar va Surveyor detallari qayta paydo bo‘lmaydi.

Paketdagi fayllar:

```text
Dist/Apollo12/Windows/LunarRescue/Saved/SaveGames/Apollo12/Apollo12.sav
Dist/Apollo12/Windows/LunarRescue/Saved/SaveGames/Apollo12/Apollo12.sav.bak
Dist/Apollo12/Windows/LunarRescue/Saved/Apollo12/Photos/<run-id>/*.png
```

Editorda shu tuzilma loyiha ildizidagi `Saved` ichida bo‘ladi. Paketni boshqa kompyuterga ko‘chirganda saqlovni davom ettirish uchun `LunarRescue/Saved` papkasini ham olib o‘ting. `Tools/package_apollo12.ps1` mavjud o‘yinchi saqlovlari, yangi foto papkalari va `GameUserSettings.ini` ni `Backups/player-data-<sana>` ga nusxalaydi, staging yo‘qotgan fayllarni qaytaradi; mavjud yangi nusxalarni almashtirmaydi.

Har ekspeditsiyaning foto papkasi alohida. Rasm yo‘qolgan bo‘lsa bajarilgan ilmiy ish bekor qilinmaydi, ogohlantirish chiqadi. Foto o‘sha kadrdan keyin diskka yozilishi sababli kutilmagan yopilishda rasm yetishmasligi mumkin. Hozircha jurnal ichida foto albom yo‘q.

## Fayl yaxlitligi va tiklanish

Kichik `USaveGame` obyekt [Unreal’ning xotiraga serializatsiya API si](https://dev.epicgames.com/documentation/unreal-engine/saving-and-loading-your-game-in-unreal-engine?lang=en-US) bilan yoziladi. Ustki qobiq format versiyasi, hajm va CRC ni tekshiradi. Semantik tekshiruv topshiriq bog‘liqliklari, noma’lum/ikki marta yozilgan ID, boshqa missiya/rejim/profil, noto‘g‘ri koordinata, NaN va rasm yo‘lining papkadan chiqishini rad etadi. 512 KiB dan katta fayl deserializatsiya qilinmaydi. Format v1; kelajakdagi mos kelmaydigan versiya avtomatik talqin qilinmaydi.

Avval vaqtinchalik fayl yozilib qayta tekshiriladi. Eski to‘g‘ri asosiy nusxa zaxiraga o‘tkaziladi; so‘ng Windows fayl almashtirish amali bilan asosiy fayl yangilanadi. Yozish yoki zaxira aylantirish xatosida oldingi asosiy saqlov qoladi. Buzilgan asosiy fayl yaxshi zaxira ustiga ko‘chirilmaydi. Yuklashda avval asosiy, keyin zaxira sinab ko‘riladi; tiklangan nusxa haqida xabar chiqadi. Bu mexanizm real elektr uzilishi yoki disk apparati nosozligi bilan sinovdan o‘tkazilgan degani emas.

Saqlash sinxron bajariladi; alohida jarayon testi yaratgan asosiy fayl 3 127 bayt. Boshlang‘ich render profili saqlash xarajatini ham o‘lchaydi; butun missiya bo‘yicha profillash hali navbatda.

## Tekshirish

`Tools/verify_apollo12.ps1 -Checkpoints` saqlash, xatodan tiklanish va alohida jarayonlar orasidagi davom ettirishni tekshiradi. `-Packaged` tayyor Windows paketidan foydalanadi. Test saqlovlari `Saved/AutomatedTests` ichiga yoziladi; oddiy regression/review tekshiruvlari o‘yinchining haqiqiy checkpointini o‘qimaydi yoki yozmaydi.

`Tools/profile_apollo12.ps1` render davomida haqiqiy checkpoint yozishni alohida test papkasida yoqadi. Boshlang‘ich marshrut profili butun missiya yoki Low preset o‘lchovi hisoblanmaydi.

2026-10-24 tekshiruvlari:

| Tekshiruv | Editor | Windows paket |
|---|---:|---:|
| Apollo / eski Moon / Mars / menyu / skafandr va input | 182 / 0 xato | 182 / 0 xato |
| Checkpoint, fayl xatosi, tasdiq, R va holat tiklash | 51 / 0 xato | 51 / 0 xato |
| Alohida jarayonga yozish | 2 / 0 xato | 2 / 0 xato |
| Yangi jarayondagi menyudan C bilan davom ettirish | 7 / 0 xato | 7 / 0 xato |
| Jami | **242 / 0 xato** | **242 / 0 xato** |

Paketdagi yozuvchi jarayon PID 8796, qayta ochib C bosgan jarayon PID 13124; editorda 18640 va 7612. O‘qish testi namunalar/ALSEP/tanlangan manzil, foto qaydlari, yashirilgan buyumlar, vaqt va masofani tekshiradi, keyingi namunani yig‘ib yangi checkpoint yozadi. Natijalar `Reports/package-apollo12-*-result.txt` va `Reports/editor-apollo12-*-result.txt`; toza qayta ochish jurnali `Reports/checkpoint-editor-resume-tests.log`.

Paketdagi haqiqiy input/collision bilan avtomatik yurish: bevosita yo‘l **1 237,29 m / 932,13 simulyatsiya soniyasi**, tarixiy yo‘l **1 344,67 m / 1 009,13 soniya**, ikkalasida ham 0 xato. Bu tezlashtirilgan, qat’iy 1/60 qadamli headless sinov: inson vaqti yoki grafik FPS o‘lchovi emas.

DX11, 1280×720, offscreen render, odatiy delta time va 60 FPS cap bilan 4 200 kadr olindi; dastlabki 600 kadr chiqarilib, 3 600 kadr / 60,01 soniya hisoblandi. O‘rtacha **59,99 FPS**, 1% low **55,74 FPS**, p95 **17,18 ms**, p99 **17,44 ms**, eng katta kadr **27,95 ms**. Jarayonning soniyada bir o‘lchangan eng katta working set qiymati taxminan **570 MiB**; bu umumiy RAM yoki VRAM emas. Oldingi 0.4.2 qisqa profili 60,00 / 56,15 FPS ko‘rsatgan; bu bitta mahalliy o‘lchovdagi farqni butun missiyaga tatbiq qilish mumkin emas.

Capture davomida 4 ta checkpoint muvaffaqiyatli yozildi: o‘rtacha **10,61 ms**, eng kattasi **11,25 ms**, 0 yozish xatosi. Ularning uchtasi warmup ichida, bittasi hisoblangan oraliqda. Bu Intrepid checkout qismi; ALSEP, fotosuratlar va Surveyor saqlovlarining grafik xarajati to‘liq o‘lchangan emas. Takrorlash ma’lumoti: `Reports/Performance/v043/summary.json`, `environment.json`, `capture.csv` va `capture.log`. Kelajakda katta inventar/fotolar saqlovga qo‘shilsa asinxron yozish qayta baholanadi.

154 ta ikki tilli yozuvning kalitlari va format argumentlari tekshirildi. `Content` va `SourceArt` dagi 357 fayl baseline SHA-256 qiymatlari bilan bir xil. Ushbu ish oldidan v0.4.2 source/config/tools va paket nusxasi `Backups/before-checkpoints-20261024-135347/manifest.json` bilan saqlandi; o‘zgarmagan katta assetlar oldingi baseline arxiviga bog‘langan.

21 tadan yangi PNG o‘zbekcha 1280×720 va inglizcha 1920×1080 o‘lchamda tekshirildi; fayllarning haqiqiy PNG o‘lchamlari ham tasdiqlandi. Yangi besh holat (davom ettirish, yangi ekspeditsiya tasdig‘i, pauzadagi reload, mashq tugagandagi Continue, zaxiradan tiklanish) ikkala tilda ko‘z bilan ko‘rib chiqildi. Yana bitta 1280×720 menyu rasmi C ko‘rsatmasini tekshiradi. Rasmlar `Reports/Visuals/v043-*` ichida. Ular avtomatik tayyorlangan holatlar; yangi o‘yinchi bilan tushunarlilik sinovi hali kerak.

Mahalliy yig‘ishda virtual xotira yetishmasligi va bir linker ichki xatosi kuzatildi. Modul PCH siz yig‘iladi; paket skripti editor va game ni alohida UBT jarayonlarida ketma-ket yig‘adi. Yakuniy build muvaffaqiyatli; xotira sabab to‘xtagan qayta ochish testi ham alohida qayta bajarilib o‘tdi. Bu xatolar gameplay tekshiruvining 0 xato natijasiga yashirilgan emas: dastlabki jurnallar `Reports/*memory-retry.log` va `Reports/checkpoint-package-link-retry.log` da qoldirilgan.
