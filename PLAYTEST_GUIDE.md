# Novice expedition playtest protocol

This is a prepared protocol, not completed human test evidence. Run it with **3–5 people who have not played LunarRescue**. Do not fill observations from scripted checks, screenshots or automated walkthroughs. This document does not recruit participants or perform sessions.

Use one blank copy of `Tools/fixtures/expedition-playtest-form.md` per actual session, identified only as P01–P05. Keep raw forms locally outside version control until an anonymous summary is reviewed. Do not collect names, email addresses, account identifiers, exact ages, faces, voices or desktop recordings. In-game evidence photographs are game content; do not include the participant's desktop or files.

## Setup and consent — 3 minutes

Record build/version or commit, CPU/GPU/RAM, resolution, graphics preset, input device, UI text size and mission mode. Use the same build/default controls unless accessibility needs a setting change; record the change without a medical explanation.

Create an independent installation for each participant:

```powershell
./Tools/start_playtest.ps1 -Participant P01 -Archive '<release.zip>'
```

For the current local packaged build, prepare an isolated installation without
publishing a release or launching the game:

```powershell
./Tools/start_playtest.ps1 -Participant P01 -PackageRoot 'Dist/Apollo12/Windows' -PrepareOnly
```

Use either `-Archive` or `-PackageRoot`, never both. `-PrepareOnly` copies the
runtime and credits while excluding owner saves/photos (`Saved`), test reports,
debug symbols, logs, CSVs and staging manifests. Start that participant's copied
`LunarRescue.exe` manually when ready. Omitting `-PrepareOnly` keeps the normal
interactive launch behaviour. The local `preparation.json` records the source
path, Git base commit and dirty state; it does not certify a committed release.
An existing `BUILD_INFO.json` is copied unchanged as source metadata, not newly
verified release evidence.


Repeat for P02–P05. The launcher refuses to replace an existing participant folder. Resume through that folder's executable. Keep the owner's installation, checkpoint and photographs untouched.

Read this statement; record only accepted/declined:

> We are testing whether the game explains its tasks and ideas clearly. Taking part is optional. You can pause or stop at any time, skip questions or ask us to omit a note. We will keep anonymous task observations, game timing and comments without personal identifiers. We will not record your face, voice or desktop.

If declined, stop and collect no answers. Explain that there are no participant grades and that confused moments help improve the game.

## Before play — three questions, 3 minutes

Record answers as given without teaching or correcting them:

1. If two paths reach the same destination, what besides distance could make one take more time or be harder to cross?
2. If a mission has a limited time budget, how would you decide whether to do an optional science task before returning?
3. In a space exploration game, how could you tell a measured terrain value from an illustrated scene or a rule chosen by the designer?

## Play — up to 30 minutes, with optional continuation

Start the wall-clock timer when the participant is told to choose the Moon mission. Record loading separately from active play. Ask them to read the loading guide, enter historical exploration and attempt the expedition: check equipment, deploy ALSEP, document and collect three samples, recover Surveyor hardware and return cargo to Intrepid. The optional coating study is their choice. The initial round uses untimed historical mode; do not introduce a fictitious oxygen deadline. Record a later timed exercise as a separate attempt.

Give no coaching for the first 90 seconds unless there is a crash, inaccessible control or a request to stop. Record the first successful action, ineffective work clicks, help requests and what the participant says they are trying to do. Observe whether they distinguish opening a tool, using it, documenting a sample, collecting it and packing hardware.

After 90 seconds, give the smallest help necessary when requested: first point to the objective/guide, next point to its control, finally demonstrate one action if still blocked. Record exact help, its time and the outcome. Do not count assisted completion as unassisted. Identify confusing places by mission stage/site and relative mission time, not an identifying desktop path.

Record milestones: first equipment check, ALSEP complete, first/third sample, Surveyor arrival, required hardware complete and cargo stowed. Record pauses, loading, active mission time and wall-clock time separately. At the 30-minute observation limit, record **not completed within the observation window**, the current stage and help; do not call it mission failure unless the game actually failed the mission. Continue only if the participant wants to and record extra time separately. Stop immediately if requested or motion discomfort prevents comfortable play.

At the result or stopping point, let the participant inspect cargo, photographs, debrief and Science/Sources information if available. Check that they match observed committed work. Mark absent planned features **not available**, not user errors. After the timed observation, optionally test text-size changes, a rebound work key and a checkpoint reload as separate tasks; exclude their time from mission completion.

## After play — three questions, 5 minutes

Record answers before explaining correct information:

1. Using terrain you saw, explain when the shortest path might not be easiest or fastest. What did the game show, and what are you assuming?
2. What would you change with a shorter exercise budget? Would you keep or skip the optional task, and why?
3. Name one part based on measured planetary data and one authored or simplified part. Where did you find that distinction?

Invite comments on the single most confusing moment, loading instructions, hand/tool contact, shadows, text readability and motion comfort. Do not suggest a complaint or answer. Comments and comfort ratings may be left blank.

## Presentation follow-up tasks — after the uncoached round

These checks extend the current protocol; they do not replace the three paired learning questions. Run them after the main observation, on the same isolated installation, and record assistance separately. If a feature is missing, mark it unavailable. Do not prime the participant with these tasks during the first 90 seconds.

1. Ask the participant to open the map and explain which line shows their own recorded movement and which is the historical reference. Ask them to locate a photograph or sample on the map. Record whether they can do this without help; do not infer that any detour was caused by a slope.
2. Ask them to walk a short curve on regolith, look back and describe what the boot marks mean. Record visibility, alternating placement and any apparent marks on equipment. Explain afterward that the tread marks are authored presentation and are capped/replaced; they are not a complete persistent historical record.
3. Invite a short walk, turn, stop and tool action in their preferred view. Record glove proportions, hand/tool connection, foot sliding, clipping and any discomfort in their own words. They may skip this task or stop at any time. A screenshot is not evidence that motion was comfortable.
4. At actual cargo completion, observe whether the closing scene agrees with their committed samples/hardware. Ask whether they understand how to skip it, and allow them to skip. Record whether pause holds the scene and whether the normal debrief remains accessible. Do not fabricate a completed mission to count this as participant completion.

The existing first-action target is 60–90 seconds, with at least four of five new players finding it without coaching. If fewer than five sessions occur, report the numerator and denominator instead of claiming that target passed. These are design acceptance targets, not observed results. Trail understanding, finale skipping and suit comfort have no invented success values.

## Review after 3–5 real sessions

Score each pre/post answer locally: 0 = absent/unsupported or mistaken; 1 = partly correct without a useful explanation; 2 = relevant explanation and concrete example. For question 1, look for a terrain/obstacle factor and separation of observation from assumed causation. For question 2, look for balancing required work, optional work and return budget. For question 3, check the example against the build's actual sources: elevation data can be measured; the spacecraft illustration and exercise deadline are authored/simplified. Do not present the deadline as measured suit endurance. Preserve original answers beside scores.

Report the number of real sessions, individual paired scores, completed missions, assisted/unassisted milestones, observation limits and repeated confusion. With 3–5 volunteers these are usability observations, not proof of educational effectiveness or a representative population study. A small score increase alone does not establish learning. Prioritize repeated blockers/progress loss, then misleading explanations, then visual/readability problems. Keep all result cells blank until an actual session supplies evidence.
