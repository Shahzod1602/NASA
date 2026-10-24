# Changelog

## 0.5.0 - 2026-10-25

- Added a 36-bone skinned suit and hands, articulated fingers, terrain-aware stance and tool poses, with three mesh LODs.
- Added distinct camera, ALSEP leveling, antenna, tongs, soil scoop, hammer-driven core and Surveyor recovery actions with cancellable work and committed cargo.
- Added English UMG settings with key rebinding and accessibility controls, a real-photo evidence album, radio transcript and cargo debrief.
- Made historical exploration untimed by default; retained the separate timed exercise and existing checkpoint compatibility.
- Improved work-site details, collision and shadow readability; added seven original synthetic English radio messages and documented NASA source corrections.
- Passed 300 Editor and 303 packaged checks, repeated 129 packaged checks after the photo fix, and completed a rendered full-mission profile. See POLISH_UPDATE.md for measurements and limitations; the real first-time-player study remains pending.

## 0.4.4 - 2026-10-24

- Made all player-facing game text English, including the menu, Apollo journal, task guidance, radio, endings and checkpoint notices.
- Removed the Uzbek text table, L language switch and language hint; old settings/launcher arguments cannot restore another language.
- Updated packaged help to English while preserving mission and checkpoint identifiers.
- Replaced language-switch checks with English-only input and legacy-launcher regressions.

## 0.4.3 - 2026-10-24

- Added versioned Apollo checkpoints using stable work/site IDs, evidence metadata, exercise telemetry and a grounded player position.
- Added C Continue from the menu/briefing/timeout, R checkpoint reload while paused, and explicit confirmation before a new expedition replaces saved progress.
- Added verified temporary writes, CRC and semantic validation, previous-copy recovery, and separate photograph folders per expedition.
- Added isolated save fault tests and a two-process menu-to-mission resume check; packaging now backs up player saves and photos.

## 0.4.2 - 2026-10-24

- Replaced the Apollo HUD's simultaneous mission checklist with one readable objective, next action and science purpose; moved all six tasks into the journal.
- Added 139 stable bilingual string-table entries for Apollo and mission selection, Uzbek defaults, L language switching and a saved language preference.
- Added contextual look/walk/work guidance and Tab as a journal shortcut, preserving J and Escape behavior.
- Removed mandatory traversal of twelve hidden waypoints. Surveyor arrival now depends on reaching its work area; the historical route remains visible and separately testable.
- Expanded input/progression and visual review checks, and added reproducible rendered frame-time captures with explicit warmup exclusion.

## 0.4.1 - 2026-10-24

- Replaced sphere-based hands with authored suit sleeves, wrist rings, padded palms and individually posed fingers; added a visible full suit, boots, helmet and pack.
- Added travel-driven gait, terrain-aware leg posing, blended work reach, Shift fast stride and V camera switching with Alt orbit.
- Added named destination markers and immediate next-action instructions to the Apollo HUD.
- Added reproducible suit mesh/material authoring and import, and explicit cook inclusion for standalone builds.

## 0.4.0 — 2026-10-24

- Added Apollo 12: Surveyor's Legacy as the Moon's menu-selected expedition, grounded in LROC terrain and mapped EVA work sites.
- Added a six-stage surface route: Intrepid checkout, ALSEP deployment, documented samples, mapped Surveyor approach, hardware recovery, and return.
- Added a paused field journal with sources, map, traverse, sample identifiers and adaptations; preserved MoonBase, Mars and mission-selection maps in the package.
- Added NASA/LROC source manifest, credits, data-preparation/import scripts, and a standalone Apollo 12 package workflow.

## 0.3.0 — 2026-10-24

- Expanded Moon to seven linked objectives with three sample sites, NOMAD-7 memory recovery and a three-channel Earthlink timing challenge.
- Added eight-minute oxygen, a one-time robot reserve, sample navigation, expedition checklist and ending ranks.
- Preserved existing Blueprint enum values, Mars progression and optional crew recorder.
- Added progression, pause, duplicate-reward and out-of-order checks plus a full movement-based expedition walkthrough.


## Chiqarilmagan — mavjud menyu maintenance

- Planet tanlashda sarlavha, footer va kartalar orasidagi tasodifiy click missiyani boshlashi tuzatildi.
- Mouse va Enter alohida boshqariladi; HUD va click ayni karta chegaralarini ishlatadi.
- Bu oldingi auditdan keyingi source o‘zgarishi. Mavjud v0.2.0 package yangilanmagan; [maintenance qaydi](MAINTENANCE_FOLLOWUP.md).

## Chiqarilmagan — 2026-09-21 audit va tayyorgarlik hujjatlari

- Texnik audit, ilmiy manbalar, tanlov holati, kechiktirilgan reja, test hisoboti, credits, playtest protokoli va shartli inglizcha taqdimot qo‘shildi.
- README'da oxygen capacity runtime override ko‘rsatmasi va gameplay resource birliklari aniqlashtirildi.
- Mavjud build va packaged regressiya/traversal tekshiruvlari qayta bajarildi; dalil va chegaralar TEST_REPORT.md'da.
- Gameplay, map, dataset integratsiyasi yoki reliz o‘zgarmadi. Yangi challenge implementatsiyasi 2026 qoida va foydalanuvchi shartiga binoan kechiktirildi.

## v0.2.0 — mavjud reliz, 2026-09-21

- Moon/Mars tanlash, ARES ekspeditsiyasi, orbital relay vazifasi va standalone Windows build.
- Mavjud implementatsiya va avvalgi tekshiruvlar: [MARS_UPDATE](MARS_UPDATE.md), [AUDIT_FIXES](AUDIT_FIXES.md).
- Ushbu reliz 2026 Space Apps hakatonidan oldin yaratilgan; qayta ishlatishga ruxsat tasdiqlanmagan.
