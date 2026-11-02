# Loading typography - 2 November 2026

Local, unreleased presentation update. The existing Moon/Mars illustrations and
loading controller remain; mission text is smaller and measured before playback.
No binary asset or planetary dataset was reimported by this typography pass.

## Implemented layout

- Reduce the destination, mission-title and guide-text hierarchy while reserving
  a separate corner column for the ring and preparation percentage.
- Wrap the guide with actual Slate font measurements. Compute the progress
  column from measured text heights, keeping the status, percentage, label and
  caption separated at 100-150% text. The default footer leaves more artwork
  visible; it expands when larger text needs room.
- Uniformly fit a logical 1280x720 canvas to the viewport. Snapshot the selected
  text scale before MoviePlayer starts; the loading thread does not read
  settings UObjects or remeasure fonts every frame.
- Retain the preparation-stage label and fictional-spacecraft/not-to-scale
  disclosure. The percentage is preparation progress, not bytes transferred.
- Summarize the real SELENE sequence, including samples, NOMAD-7, Earthlink and
  return. This adds no task, resource or completion reward.

The existing controller still owns its 0/15/70/100 preparation targets, smooth
monotonic display, visible 0/100 endpoints, duplicate-travel guard, Continue
options and world-time hold while transitioning. The typography helper controls
presentation only.

## Final packaged validation

Editor/Game compilation, full cook and staging succeeded for final game SHA-256:
`b034765110ad30f5a60ffcf1e874ddac679002b16c74f0e9f72bea54017a9fb4`.
This validation follows the later SELENE rover and cosmetic grounding repairs.

| Final-executable evidence | Observed result |
| --- | --- |
| LoadingTypographyTest | 55 exact PASS lines; zero failures |
| LoadingTypographyReview at 1280x720 and 1920x1080 | Ten fresh staged PNGs at each resolution; 20 total |
| Actual rendered loading regression | 76 exact PASS lines; zero failures |

The font suite checks all four destinations at 100/150%, measured safe bounds,
text/ring overlap, progress gutter, all displayed percentages, stage captions
and source disclosures. The staged review uses the same MoviePlayer paint widget
but declares synthetic UI-layout fixtures; those images alone do not establish
actual travel, world hold, human usability or FPS. The separate real travel run
checks visible 0/100, monotonic progress, destination flow and held world time.

These 55 and 76 outcomes are included in the final full-queue receipt's 1,254
PASS executions, not added again. The receipt separately records 110 staged PNGs
across all reviewed UI families and six routed Slate pointer outcomes.
[Executable-specific evidence](Docs/Feedback2026-11-02/evidence.json) includes the
55-pass typography source report/summary and every queued wrapper result.
The earlier 2ed/e738 builds also passed their own suites. Their outcomes remain
under the earlier hashes, not additional unique tests or evidence transferred
to certify the final executable. All 17 queue runs and the separate typography
suite were rerun on b034 after the later contact fixes.

The initial measured 150% progress-column overlap was corrected before final
validation by placing the column from actual text heights. Static inspection,
PowerShell parsing and whitespace checks also ran. Automated layout/freshness
checks are distinct from independent visual inspection. See
[TEST_REPORT](TEST_REPORT.md) for the completed matched-condition performance
runs and 264/264 unchanged protected-file comparison. A single run per scene
is not evidence of whole-mission sustained FPS. The separate equipment review's latest solar-junction view
remains inconclusive; it does not affect the loading-layout assertion count.
No human-learning result is claimed.

## Reproduction

```powershell
./Tools/verify_loading_typography.ps1 -Packaged
./Tools/verify_loading_typography.ps1 -Packaged -Review -Width 1280 -Height 720
./Tools/verify_loading_typography.ps1 -Packaged -Review -Width 1920 -Height 1080
./Tools/verify_loading.ps1 -Packaged -Graphics
```

The real-travel command is required in addition to staged typography review.
Source illustrations, NASA terrain and third-party credits retain their existing
provenance; no NASA approval, new scientific measurement or private reference
correspondence is added to the repository by this update.
