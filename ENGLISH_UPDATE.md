# LunarRescue v0.4.4 - English edition

Menus, briefing, objectives, work prompts, tutorial, radio messages, journal, map labels, results and checkpoint notices are English. Existing Moon/Mars interfaces and synthetic radio narration were already English. Packaged player documentation is English too.

The former L language switch and hint are removed. A previous Language=uz preference or -ApolloLanguage=uz argument cannot change the interface. One offline English String Table provides 153 stable entries. No translation download is required.

Controls: WASD/arrows move, mouse looks, E interacts/holds to work, Q selects a destination or turns the antenna, Tab/J opens the paused journal, Esc pauses, C continues a save, and R reloads its checkpoint while paused. V changes camera, Alt orbits in following view, Shift holds a faster stride, F zooms and Space hops.

Checkpoint format v1, work/site identifiers and evidence data are unchanged. Existing saves remain compatible. Starting a new run still asks before replacing progress. Repackaging backs up player saves, photos and settings.

The input regression checks that pressing the former L shortcut leaves English active. Tools/verify_apollo12.ps1 -LegacyLanguage also passes the obsolete Uzbek launcher argument to exercise compatibility. Both editor and packaged regression runs passed 242 checks each with zero failures, including the obsolete launcher argument and former L input. Reports: Reports/english-validation.json, english-editor-tests.log and english-package-tests.log.

Older milestone documents preserve historical results; former bilingual behavior is not the current UI. See [Apollo 12](APOLLO12.md), [saving](CHECKPOINT_UPDATE.md) and [suit controls](SUIT_UPDATE.md).

Twenty-one English UI/world fixtures were rendered at 1280x720, including briefing, all stages, journal pages, ending and save dialogs. PNG dimensions are checked by the verifier. A separate mission-selection fixture covers the English menu and Continue hint. Screenshots are prepared states, not new-player usability evidence.

All 357 Content/SourceArt SHA-256 values match the baseline. No scientific data, suit meshes or mission coordinates changed. The pre-change source/config/tool backup is Backups/before-english-20261024-210241.
