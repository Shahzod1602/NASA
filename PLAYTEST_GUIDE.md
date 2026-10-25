# First-time player acceptance test

This is a ready-to-run protocol, **not completed human test evidence**. Recruit
5-8 people who have not played LunarRescue. Use the Windows build, default English
settings and a fresh test save. Keep each person's save and photographs separate
from the developer's and owner's existing progress.

1. Record hardware, resolution, graphics preset and build/version. Do not record
   names, faces or voices unless the participant has agreed to that recording.
2. Ask the player to start historical exploration. Give no directions for the
   first 90 seconds. Record time to first equipment check and any wrong action.
   At 60 seconds ask them to state their next action and destination. Record the
   answer without correcting it. Note confusion between opening and confirming.
3. Let them reach ALSEP, align the instruments, document and collect a sample.
   Observe whether they distinguish opening a tool from using it, discover cancel,
   and understand why a context photo is needed.
4. Ask them to change text size, sensitivity and one work key. Close settings and
   confirm the new key works. Reload the build and confirm settings persist.
   Ask them to hide/show the guide with H, then pause and open the how-to page
   with H. Confirm that its instructions use the rebound work key.
5. Complete the mission if available; verify that Surveyor components disappear
   only when packed, the album has the player's photographs, and the debrief has
   the actual cargo. Record completion time and any help given.
6. Ask the player what 12032 is, why Surveyor hardware was collected, and whether
   the game timer represents actual suit oxygen. These identify misleading copy.
7. Ask for motion discomfort, clipping, hand/tool grip problems, unreadable shadows
   and the single most confusing moment. Save anonymous findings with severity.

Acceptance targets: at least 4 of the first 5 players complete the initial action
without help; at least 4 of 5 distinguish photo, sample and hardware workflows;
no repeatable loss of committed progress or blocked mission. Fix repeated
confusion before treating the build as a final release candidate.

For v0.5.2 also require at least 4 of the first 5 players to name their next
action and destination at the 60-second question. Observe whether players can
follow the first/second fitting markers and recognize the sealed sample ID and
actual count without an explanation. Record wrong clicks and help requests.

| Participant code | First action seconds | Help needed | Finished | Completion minutes | Most confusing moment | Severity / follow-up |
|---|---:|---|---|---:|---|---|
| P01 | | | | | | |
| P02 | | | | | | |
| P03 | | | | | | |
| P04 | | | | | | |
| P05 | | | | | | |
| P06 | | | | | | |
| P07 | | | | | | |
| P08 | | | | | | |

For v0.6.0, use `Tools/start_playtest.ps1 -Participant P01 -Archive <release.zip>`
to extract an independent installation for each participant. Repeat with P02-P05;
the launcher refuses to overwrite an existing participant folder. These copies
have separate saves and photos and do not touch the owner's installation. Resume
an existing test through its own executable. Participant folders stay local.

Observe whether the moving sample, pouch label and two fitting contacts explain
the action. After the Surveyor photo, ask the player to find the optional coating
study without directions. Record discovery, hypothesis choice and whether they
understand that its illustrative panels are not exact historical measurements.
The optional study must not block primary cargo or mission completion.

No recruitment or human sessions have been completed by automation. Keep the
table empty until real participants supply observations. Do not count scripted
checks, renders or performance captures as human test results.
