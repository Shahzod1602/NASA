# Ilmiy ma’lumot va manbalar

Holat: **2026-09-21; dataset baholashi, integratsiya emas**. LunarRescue hozir authored Moon/Mars rescue prototipi. NASA tasdiqlagan simulyator yoki professional astronavt tayyorlash vositasi emas. Science terminaldagi yozuvlar syujet; real tadqiqot natijasi emas.

## Nomzod: LOLA janubiy qutb grid

[NASA LOLA](https://science.nasa.gov/mission/lro/lola/) asbob haqida; [PDS Geosciences](https://pds-geosciences.wustl.edu/missions/lro/lola.htm) arxiv va mahsulotlarga kirish beradi. Quyidagi qiymatlar bevosita [PDS mahsulot labeli](https://pds-geosciences.wustl.edu/lro/lro-l-lola-3-rdr-v1/lrolol_1xxx/data/lola_gdr/polar/img/ldem_875s_20m.lbl)dan tekshirildi:

| Maydon | Qiymat |
| --- | --- |
| Dataset / product | `LRO-L-LOLA-4-GDR-V1.0`; `LDEM_875S_20M.IMG`, version V2.0 |
| Producer | LRO LOLA Team, Goddard Space Flight Center; David E. Smith |
| Source observations | 2009-07-13 – 2017-02-02; product created 2017-06-15 |
| Qamrov | Planetocentric latitude −90…−87.5°; label longitude bounds N/A. Aniq missiya ROI hali tanlanmagan. |
| Grid | 7584 × 7584; 20 m/pixel, qutbda haqiqiy; bu o‘lchov aniqligi kafolati emas |
| Format/hajm | Raw IMG, signed little-endian int16; 7584 × 7584 × 2 = **115,034,112 bytes**, taxminan 109.7 MiB |
| Balandlik | `elevation_m = DN * 0.5`; reference sphere radius 1,737,400 m |
| Radius | `planetary_radius_m = elevation_m + 1737400`; Unreal Z sifatida radius ishlatilmaydi |
| Proyeksiya | Spherical polar stereographic, center −90°, longitude 0°, east-positive |
| Koordinata reference | Body-fixed rotating, Mean Earth/Polar Axis of DE421; uchala radius 1737.4 km |
| Registration | Pixel registration; line/sample projection offset 3791.5; indekslar 1…7584 |
| NoData | Labelda explicit sentinel ko‘rsatilmagan. **Tasdiqlanmagan**; barcha int16 qiymatlarni valid deb qabul qilish mumkin emas. |
| Olish holati | Faqat LBL 2026-09-21 olindi. IMG olinmagan; raster checksum va qayta ishlangan ROI mavjud emas. |

Bu allaqachon gridded/interpolated mahsulot: label GMT blockmedian/surface ishlovini qayd etadi. Har bir 20 m piksel mustaqil lazer o‘lchovi emas. Unrealda 0.5 m mesh yaratish yangi o‘lchangan detal bermaydi. Slope aniqligi ham grid va ishlovga bog‘liq. `LDEC` count mahsulotini `LDEM` elevation bilan almashtirmaslik kerak.

Foydalanish uchun [NASA Science data license](https://science.data.nasa.gov/about/license) umumiy siyosati va aynan mahsulotdagi qo‘shimcha shartlar tekshiriladi. Labelda maxsus cheklov ko‘rinmadi; catalog/redistribution tekshiruvi yakunlanmagan. NASA nomi va ochiq data mahsulotga NASA tasdig‘ini bermaydi. Credit: NASA/GSFC, LRO LOLA Science Team; distribution: NASA PDS Geosciences Node. Joriy SourceArt asset litsenziyalari bundan alohida.

## Ruxsatli bosqich uchun pipeline spetsifikatsiyasi — hali bajarilmagan

1. Rasmiy IMG, label va projection catalogni olish; original URL, UTC download date, byte count va SHA-256 saqlash. NoData talqini ochilmaguncha importni bloklash.
2. Endian/type/dimensions/range tekshiruvi; explicit validity mask. Nol balandlikni NoData deb taxmin qilmaslik. Yetishmagan faylda aniq xato; NASA deb belgilangan sun’iy fallback bo‘lmasin.
3. Projection catalog bilan pixel-center/half-pixel konvensiyasini tekshirish; kamida uchta nazorat nuqtasi va lon/lat round-trip. Qutb meridian singularity va Y yo‘nalishini sinash; Earth EPSG:4326 ni qo‘llamaslik.
4. Buzilmagan real relyefda yurishga mos kichik ROI va ikki yo‘lni topish. 20 m grid joriy taxminan yuz metrlik sahnada kam namuna beradi: hudud kengayishi yoki boshqa rasmiy mahsulot kerak bo‘lishi mumkin. Scale yoki balandlikni yashirin o‘zgartirmaslik.
5. Lokal metrik origin, east/north/up o‘qlari, Z origin va georeference manifestda; Unreal 1 m = 100 cm. Mesh uchun `Z_cm = (elevation_m - origin_elevation_m) * 100`. Landscape quantization tanlansa decoded min/max xatosini tekshirish.
6. Valid qo‘shnilar bilan offline slope: `atan(sqrt((dz/dx)^2 + (dz/dy)^2))` radians → degrees; dx/dy metrlarda, map-scale distortion hisobga olinadi. NoData yonidagi natija invalid. Flat va ma’lum plane testlari shart.
7. Author-created baza, tosh va mission obyektlarini alohida qatlamda joylash; yo‘l qilish uchun ilmiy balandliklarni tekislamaslik. Collision va CharacterMovement orqali ikkala yo‘lni o‘tish.
8. Ixcham cooked mesh/height, slope samples, route points va metadata lokal packagega kiritiladi. Runtime internet va har-frame geoprocessing talab qilinmaydi. Raw full grid runtimega kiritish shart emas.

Final manifest quyidagilarni majburiy saqlaydi: product ID/version, URLs, download UTC, raw/derived SHA-256, ROI polygon, CRS/reference sphere, pixel registration, units/grid spacing, NoData/mask policy, origin/axis mapping, processing software/version/commands, interpolation va quantization, credit/terms. Hozir bunday final manifest mavjud emas.

## O‘yinda ma’lumotning rejalashtirilgan aniq roli

Haqiqiy grid balandlik/collisionni, undan hisoblangan slope esa A/B yo‘llarning profili va taqqoslashini boshqaradi. Planner va science terminal aynan shu cached samplesdan foydalanadi. Telemetry haqiqiy harakat vaqtini va masofani o‘lchaydi. Hozir bu bog‘lanishlar **amalga oshirilmagan**; mavjud terrain sun’iy.

## Fakt, model va badiiy qatlam

| Qatlam | Izoh va dalil |
| --- | --- |
| Moon gravity | O‘yinda 1.62 m/s²; [NASA Moon Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html). Lokal gravitatsiya maydoni simulyatsiya qilinmaydi. |
| Mars gravity | O‘yinda 3.71 m/s² doimiy gameplay qiymati; [NASA Mars Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/marsfact.html)dagi fizik qiymatlar bilan solishtirish mumkin; spatial model emas. |
| Atmosfera | Oyda juda siyrak exosphere; Marsda siyrak atmosfera bor. [Moon facts](https://science.nasa.gov/moon/facts/), [Mars facts](https://science.nasa.gov/mars/facts/). Ikki sayyora muhiti tenglashtirilmaydi. |
| Audio | Vakuum orqali odatiy mexanik tovush tarqalmaydi: [NASA electromagnetic waves](https://science.nasa.gov/ems/02_anatomy/). Radio, nafas va suit-conducted qadamlar badiiy audio; fizik akustika hisoblanmaydi. Mars audiosi ham o‘lchangan atmosfera akustikasi emas. |
| Resource | 300/480 s — o‘yin balansi. Litrlarda O2 yoki haqiqiy skafandr ishlash vaqti emas. Hozir har active sekundga 1 budget sekund ketadi. |
| Movement | Tezlik, jump, hold vaqtlar va kelajak slope koeffitsiyentlari gameplay tuning; fiziologik aniqlik da’vosi yo‘q. Slope “astronaut-safe” degan mezon emas. |
| Authored content | SELENE/ARES, rescue voqeasi, science records, joylashtirilgan baza/toshlar; [credits](CREDITS.md). |

Science & Sources oynasi kelajakda uch bo‘limni aniq ajratadi: “Measured & derived data”, “Simplified gameplay rules”, “Authored story & objects”. Asosiy HUD qisqa qoladi.
