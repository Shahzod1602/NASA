# Bosh menyu va Settings — 2026-10-29

Foydalanuvchining oltita UI referensi asosida mavjud o‘yinning frontend qismi yangilandi. Bu alohida gameplay yoki ilmiy dataset o‘zgarishi emas. Interface matnlari inglizcha; Mars fictional prototype sifatida ko‘rsatiladi.

## Interfeys

- Chap menyu: Continue expedition, Explore the Moon, Explore Mars, Settings, Exit. Continue faqat o‘qiladigan checkpoint mavjud bo‘lsa yoqiladi. Planet ustiga mouse olib borilganda fon va tavsif almashadi; bosilganda tegishli missiya ochiladi. 1/2 va F1 tugmalari saqlanadi.
- Moon/Mars uchun ikkita alohida badiiy fon. NASA fotosurati yoki surveyed terrain deb belgilanmaydi. Gameplay LROC relyefi o‘zgarmaydi.
- Settings: Video Options, Sound Options, Gameplay, Controls. Har tab ikki ustundan iborat; katta shrift yoki kichik ekran uchun ustun ichida scroll bor. Panel 16:9 bazadan ekran o‘lchamiga mos masshtablanadi.
- Eski fotoalbom, checkpoint va tool workflow o‘z holicha saqlanadi. Menu pointer input UMG tugmalariga o‘tdi; bo‘sh joy missionni boshlamaydi.

## Ishlaydigan sozlamalar

| Bo‘lim | Amaldagi boshqaruv |
| --- | --- |
| Video | Resolution, windowed/borderless/fullscreen, 50–100% render scale, 30/60/120/unlimited FPS, VSync, brightness, FOV, graphics preset, texture/shadow/view-distance quality, Off/FXAA/TAA, ambient occlusion, bloom, motion blur |
| Sound | Master, suit/tool va radio volume; subtitles; radio audio test |
| Gameplay | Interface/subtitle size, hold/toggle fast stride, hand motion |
| Controls | Mouse sensitivity, invert Y, smoothing; movement, work, jump, route, journal, view, sprint, zoom key rebinding |

Resolution yoki display mode o‘zgarishi 15 sekund ichida **Keep display** bilan tasdiqlanadi. Tasdiqlanmasa yoki panel yopilsa oldingi ekran rejimi qaytadi. Boshqa sozlamalar mavjud GameUserSettings configiga saqlanadi. Test/review rejimlari foydalanuvchi displayini almashtirmaydi va configini saqlamaydi.

Odatiy `Play Lunar Rescue.cmd` endi `-ResX/-ResY/-windowed` argumentlari bilan saqlangan ekran tanlovini bekor qilmaydi. AA, motion blur va FPS tanlovlari project defaultlardan ustun bo‘lgan game override orqali rendererga uzatiladi; explicit console override esa o‘z ustuvorligini saqlaydi.

Rasmdagi har bir yozuv mustaqil yangi subsystem degani emas. Music/ambient/UI uchun alohida treklar, audio device/spatial/dynamic-range boshqaruvi, crouch/head-bob va vaqt bo‘yicha autosave ushbu loyihada yo‘q; ular ishlaydigan soxta dropdown sifatida qo‘shilmadi. Buning o‘rniga panelda amaldagi chegaralar yozilgan. Checkpointlar bajarilgan ish tasdiqlanganda saqlanadi. Tutorial H, map J va kamera View key orqali mavjud boshqaruvda qoladi.

## Fayllar va takrorlash

`LunarFrontend.cpp`: main menu, settings layout/tablar, display confirmation, frontend checks/review. `LunarPanels.*`: mavjud settings rowlari va dropdown/key selectors. `LunarSettings.*`: config/backend. `LunarGame.cpp`, `LunarApolloHUD.cpp`, `LunarApolloTools.cpp`: menu, volume/subtitle va panel lifecycle ulanishlari.

`SourceArt/Frontend/` original PNG va credits; `/Game/Lunar/Frontend/` imported texturelar. `Tools/import_frontend.py` faqat texture yaratadi, maplarni tahrirlamaydi. `/Game/Lunar` mavjud AlwaysCook qoidasi yangi assetlarni ham qamrab oladi.

```powershell
& .\Tools\verify_frontend.ps1
& .\Tools\verify_frontend.ps1 -Review
& .\Tools\verify_frontend.ps1 -Review -Width 1920 -Height 1080
& .\Tools\verify_apollo12.ps1 -Polish -Regression
```

Frontend tests: asset yuklanishi, preview, yangi settinglarning serializatsiyasi, rebound key saqlanishi, automation display himoyasi, default reset, to‘rtta tab va panel yopilishi. Render review 7 ta ekran yaratadi, shu jumladan 150% text. Bular odamning mouse bilan qo‘lda playtesti o‘rnini to‘liq bosmaydi.

## Tekshiruv dalillari

Joriy ishning local log va screenshotlari: `Reports/Frontend20261028/`. Boshlang‘ich keng inventory: 422 save/config/foto fayli; backup: `Backups/Frontend20261028-player-files/`. Ulardan 82 tasi haqiqiy save, foto va GameUserSettings fayllari. Qolganlar orasida Unrealning eski crash-report configlari ham bor; engine ularni housekeeping paytida o‘chirishi mumkin. Barcha 422 fayl o‘zgarmadi degan da’vo qilinmaydi.

UE **5.8.3**, Windows 11, Ryzen 5 6600H, RTX 3050 Laptop GPU, 16 GB RAM. Yakuniy editor build muvaffaqiyatli. Frontend: **15 PASS, 0 failure**; renderer override tekshiruvi ham bor. Mavjud headless regressiya: Apollo 57, legacy Moon 50, Mars 54, menu travel 7, polish 63 — **231 PASS, 0 failure**. Bu NullRHI natijalari grafik performance o‘lchovi emas.

1280×720 va 1920×1080 uchun editor rejimida 7 tadan fresh DX11 screenshot yaratildi; matn wrap/kontrast muammolari tuzatildi. Yakuniy packaged build uchun 1280×720 formatda yana 7 ta screenshot olindi. Bular FPS yoki memory benchmark emas.

Windows native build, cook, stage va archive muvaffaqiyatli: `Reports/Frontend20261028/package.log`, ExitCode 0. Ishga tushirish: `Play Lunar Rescue.cmd` yoki `Dist/Apollo12/Windows/LunarRescue.exe`. Paketda frontend **15 PASS**, Apollo/legacy Moon/Mars/menu/polish regressiyasi **231 PASS**, jami **246 PASS, 0 failure**. Dalillar: `package-verify.log`, `package-review.log`, `package-regression.log` va `Reports/package-apollo12-{checks,legacy-moon,mars,menu,polish}-result.txt`. Boshqa oldingi reportlar ushbu ishda qayta bajarilgan deb hisoblanmaydi.

Computer-use orqali tayyor paketda F1, Video/Sound/Gameplay/Controls tablari, dropdown ochish va tanlovsiz yopish, Back, bosh menyudagi Settings va Exit tekshirildi. Bo‘sh fondagi click missiyani boshlamadi. Dastlabki avtomatlashtirilgan Settings bosishida kutilmaganda Continue ishga tushdi; keyingi ikki urinishda Settings to‘g‘ri ochildi. Sabab aniqlanmadi, tuzatilgan deb da’vo qilinmaydi; cold-start mouse tekshiruvini haqiqiy foydalanuvchi bilan takrorlash kerak. Display mode almashtirishning 15 soniyalik real monitor rollbacki qo‘lda tekshirilmagan. To‘liq inson playtesti o‘tkazilmagan.

Oddiy o‘yinni ochish engine configida render scale qiymatini yangiladi; sinovdan so‘ng oldingi GameUserSettings nusxasi backupdan tiklandi. Editorning shu ishda yangilangan per-project preferences fayli ham tiklandi. Yakunda 82 ta haqiqiy save/foto/GameUserSettings faylining SHA-256 hashlarida farq yo‘q. Remote push va yangi GitHub release bajarilmadi.
