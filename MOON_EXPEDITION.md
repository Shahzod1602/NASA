# Moon: The Lost Expedition

An eight-minute playable Moon expedition with seven linked objectives. In-game text remains English.

1. Recover the rover power cell.
2. Insert it, connect the cable and restart SELENE.
3. Download the station research, revealing the field sample sites.
4. Recover regolith, basalt and impact-glass canisters in any order. Hold E for 2.5 seconds at each; Q changes navigation. Looking away, releasing E or changing targets cancels progress.
5. Locate the wheeled NOMAD-7 survey robot. Hold E for four seconds to save its memory and transfer its emergency oxygen cartridge (+60 seconds, limited to suit capacity). The robot stays at its site; this is memory recovery, not an escort mission.
6. Activate Earthlink: approach and face the beacon, then tap E inside the green timing window for each of three channels. Mistimed taps do not remove already locked channels. Pausing freezes the signal.
7. Return to the lander. The crew recorder remains an optional lunar-jump detour, selected with Q.

The HUD shows the expedition checklist, sample count, active target and beacon synchronization. The ending displays samples, recovered memory, signal locks and a rank: RESCUER for completing the main expedition, EXPLORER with the crew recorder, or PATHFINDER with the recorder and at least two minutes of oxygen left.

The sites, robot and beacon use existing project materials and runtime geometry. No map regeneration or new third-party assets are needed. Existing enum values retain their numeric indices for Blueprint compatibility. Mars keeps its original expedition. Terrain and mission science are fictional; the oxygen budget is a game mechanic. New radio cues are text with existing suit sounds, not newly recorded speech.

## Verification

`Tools/verify_expedition.ps1 -Walkthrough` runs Moon progression and edge cases, Mars regression, menu travel, then a full Moon route using normal character movement, collision and mission time. `-Graphics` renders the checks; `-Review` captures the new HUD and mission props; `-Packaged` tests the standalone binary. Reports are written to `Reports`.

Editor and standalone DX11 progression, edge cases, menu travel and full movement-based traversal passed. Five rendered screens were visually reviewed. See [measured results](Reports/MOON_EXPEDITION_VALIDATION.md).
