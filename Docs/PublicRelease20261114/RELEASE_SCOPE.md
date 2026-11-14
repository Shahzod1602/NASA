# v0.9.2 public feedback preview

This preview contains the Moon/Mars feedback fixes and verified museum-rover adaptations from the 14 November development batch. It is distinct from any competition submission. No claim of competition eligibility, NASA endorsement or institutional partnership is made. Existing dates/history are preserved; the public-distribution fallback changes were made after the private candidate, not backdated into it.

## Public content boundary

The optional `MoonArrival.mp4`, AstroKid reference JPEG, inventory-reference JPEG and the two imported reference-art textures are excluded from Git and this public Windows package while their original sources/redistribution details remain unconfirmed. The full partner montage is also excluded. Startup uses an original live text wordmark/spinner; backpack entries use original text symbols when local reference art is absent. Item quantities, guards and gameplay remain unchanged. Missing optional video proceeds directly into the selected mission.

The verified museum-rover adaptations remain CC BY-SA 4.0 with attribution and reproducible adaptation/source metadata. That license is not applied to unrelated code or assets. Existing planetary data and model credits remain bundled. No new NASA elevation data was created by these feedback changes.

## Private-data boundary

The release archive excludes all Saved directories, player checkpoints/photos/settings, Reports/test outputs, local backups, environment files, debug symbols and unverified media. The previously validated reference-art build and the owner's original files remain in local backups and are not uploaded. The ZIP opens with Windows/LunarRescue.exe; Unreal Editor is not required to play.

## Functional scope

- Deliberate lunar movement, retained takeoff momentum and a reachable authored recorder ledge; Mars tuning retained.
- Seated rover driver, mouse orbit, cockpit/follow switching, stationary mudguards and immediate quiet boarding.
- Cell grip handles, removed SELENE instruction board, collision for static Lunokhod, grounded mast and mounted status lamps.
- Direct sequential EVA checks, prompt verified-cable dismissal, interactive offline planet context and live backpack state.
- Mission-specific loading with held mission clocks and a graceful missing-optional-media path.

Human playtests and occupied-driving performance remain unmeasured. UI layouts are adapted to actual mission state and existing styling; they are not certified pixel-perfect reproductions of supplied references. See PUBLIC_TEST_REPORT.md for checks on this exact public package, rather than inheriting the earlier private candidate's receipts.

## Reproducing the public-content check

After building, cooking the four existing maps and running `Tools/package_apollo12.ps1 -StageOnly`, run `Tools/verify_nasa_tasks.ps1 -Packaged -Intro -PublicDistribution`. This verifies the live text startup plus the absence of both unconfirmed texture packages and the optional movie in the packaged game. The flag is intended for the public distribution, where these optional inputs are deliberately absent.
