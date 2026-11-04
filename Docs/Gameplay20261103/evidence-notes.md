# Independent final Windows evidence count

Executable SHA-256: 5171c1b0a6142dfa4c35e373d731d441ce88eb8d7a41f768779bbeaded52f2f0

The final queue is complete: 16 suites, no suite failure. Counted exact anchored PASS lines from 55 copied C++ reports; checked each report zero-failure terminator, freshness and SHA-256. Per-suite summaries agree with the raw report counts where available. Current packaged executable hash independently matches the queue.

- C++ assertion executions: 2400 PASS / 0 FAIL. This is an execution count, not unique test definitions.
- New four suites: 500 NullRHI functional + 536 rendered functional + 55 + 55 staged layout assertions = 1,146.
- Other 12 suites: 1,254 exact PASS executions independently recounted, not imported from a previous version.
- Staged PNG artifacts: 160 (110 gameplay + 50 HUD), counted separately from the 160 layout assertions already inside the 2,400 C++ executions.
- Paired carried-cell contribution checks: 8 PASS, counted separately; fractions 0.9089360090874669–0.988631221719457 against threshold 0.12. Pixel and manifest SHA bindings verified.
- Nine identical-report-content groups reflect repeated executions across contexts. They are listed, not deduplicated into invented unique test counts.

The 36 rendered-only assertions are reported per mission below; they require actual viewport projection and do not signify missing mission cases in NullRHI.

Reports/Gameplay20261103/package-test-1280x720-20261103-075915-379/apollo-checks.txt -> Reports/Gameplay20261103/package-rendered-test-1280x720-20261103-075935-294/apollo-checks.txt
- PASS Engine projects every actual cell corner inside viewport at 45 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 45 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 45 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV 65 pitch

Reports/Gameplay20261103/package-test-1280x720-20261103-075915-379/mars-checks.txt -> Reports/Gameplay20261103/package-rendered-test-1280x720-20261103-075935-294/mars-checks.txt
- PASS Engine projects every actual cell corner inside viewport at 45 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 45 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 45 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV 65 pitch

Reports/Gameplay20261103/package-test-1280x720-20261103-075915-379/selene-checks.txt -> Reports/Gameplay20261103/package-rendered-test-1280x720-20261103-075935-294/selene-checks.txt
- PASS Engine projects every actual cell corner inside viewport at 45 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 45 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 45 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 60 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 85 FOV 65 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV -75 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV 0 pitch
- PASS Engine projects every actual cell corner inside viewport at 110 FOV 65 pitch

Staged captures do not establish human usability, anatomical correctness, learning gains or FPS. The pixel metric measures rendered contribution, not semantic segmentation. Network-disconnected operation, real participants and separate performance profiling are outside this queue. Legacy suites lacking their own executable digest have queue/current-package binding rather than a retroactively invented per-process digest.

The JSON uses repository-relative artifact references and contains no save GUIDs, private conversations or personal absolute paths.
