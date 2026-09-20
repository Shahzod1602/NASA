from pathlib import Path
p=Path(__file__).resolve().parents[1]/'Source/LunarRescue/LunarGame.cpp'
s=p.read_text()
translations={
"ALOQA UZILGAN. SELENE stansiyasidan javob yo'q.":"SIGNAL LOST. No response from SELENE research station.",
"Kislorod tugadi. Missiyani qayta boshlang.":"Oxygen depleted. Restart the mission to try again.",
"NAV: Sharqdagi krater yonida zaxira quvvat moduli bor. Belgiga boring.":"NAV: A spare power module is beside the eastern crater. Follow the marker.",
"01 / QUVVAT MODULINI TOPING":"01 / FIND THE POWER MODULE",
"02 / STANSIYA QUVVATINI TIKLANG":"02 / RESTORE STATION POWER",
"03 / ILMIY MA'LUMOTLARNI OLING":"03 / RECOVER THE SCIENCE DATA",
"04 / QO'NISH APPARATIGA QAYTING":"04 / RETURN TO THE LANDER",
"SELENE / QUTQARUV MISSIYASI":"SELENE / RESCUE MISSION",
"[ E ]  Quvvat modulini olish":"[ E ]  Pick up power module",
"[ E ]  Modulni o'rnatish":"[ E ]  Install power module",
"[ E ]  Tadqiqot natijalarini yuklash":"[ E ]  Download research data",
"[ E ]  Missiyani yakunlash":"[ E ]  Complete mission",
"MODUL OLINDI. Uni stansiyaning tashqi quvvat portiga o'rnating.":"MODULE SECURED. Install it in the station's external power port.",
"QUVVAT TIKLANDI. Terminaldan regolit harorati ma'lumotlarini oling.":"POWER RESTORED. Recover the regolith temperature records from the terminal.",
"MA'LUMOT SAQLANDI. Oy regolitining harorat kuzatuvlari olindi. Apparatga qayting.":"DATA RECOVERED. Regolith temperature records secured. Return to the lander.",
"ALOQA TIKLANDI. Ilmiy ma'lumotlar bazaga yetkazildi.":"SIGNAL RESTORED. Research data delivered safely to base.",
"NISHON  %.0f m   /   g = 1.62 m/s2":"TARGET  %.0f m   /   g = 1.62 m/s2",
"NISHON  >":"TARGET  >", "<  NISHON":"<  TARGET",
"WASD  Yurish     SICHQONCHA  Qarash     SPACE  Sakrash     E  Ishlatish     ESC  Pauza":"WASD / ARROWS  Move     MOUSE  Look     SPACE  Jump     E  Interact     ESC  Pause",
"MISSIYA PAUZADA":"MISSION PAUSED", "MISSIYA BAJARILDI":"MISSION COMPLETE",
"ALOQA YO'QOLDI":"SIGNAL LOST", "OYDAGI SO'NGGI SIGNAL":"THE LAST SIGNAL",
"SELENE stansiyasi javob bermayapti. Sizda 5 daqiqa kislorod bor.":"SELENE is offline. You have five minutes of oxygen.",
"Modulni toping. Quvvatni tiklang. Ilmiy ma'lumotni olib qayting.":"Find the module. Restore power. Recover the data and return.",
"Oy gravitatsiyasi: 1.62 m/s2. Sakrash sekinroq, tushish uzoqroq.":"Lunar gravity: 1.62 m/s2. Expect longer jumps and slower falls.",
"Hudud badiiy model; tadqiqot maqsadi - regolit haroratini kuzatish.":"Fictional terrain. Science objective: monitor regolith temperature.",
"[ ENTER ]  MISSIYANI BOSHLASH":"[ ENTER / CLICK ]  START MISSION",
"Kislorod hisoblagichi to'xtatildi.":"Movement and oxygen are paused.",
"[ ESC ]  DAVOM ETISH":"[ ESC / ENTER / CLICK ]  RESUME",
"Stansiya ishlamoqda. Tadqiqot natijalari xavfsiz yetkazildi.":"The station is online. Research data has been delivered safely.",
"Kislorod tugadi. Keyingi safar nishon belgilariga ergashing.":"Your oxygen ran out. Follow the objective markers on your next attempt.",
"Ilmiy maqsad: keskin harorat o'zgarishlarini kuzatish orqali":"Science goal: study extreme surface temperature changes",
"Maslahat: E tugmasini obyektga yaqinlashib, unga qarab bosing.":"Tip: move close to an object, look at it, then press E.",
"kelajakdagi Oy uskunalari uchun issiqlik himoyasini o'rganish.":"to improve thermal protection for future lunar equipment.",
"[ R ]  QAYTA O'YNASH":"[ R / ENTER ]  PLAY AGAIN",
}
for old,new in translations.items():
    assert old in s, old
    s=s.replace(old,new)
p.write_text(s)
print(f'Translated {len(translations)} game strings to English.')
