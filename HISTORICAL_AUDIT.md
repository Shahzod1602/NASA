# Apollo 12 historical and asset audit

Reviewed 25 October 2026 for the v0.5 work. The game is an educational adaptation,
not a training simulator or a complete reconstruction of the two real EVAs.

| Game claim or asset | Evidence and treatment |
|---|---|
| Apollo 12, November 1969; Conrad and Bean on the surface, Gordon in orbit | [NASA mission account](https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/). The single player combines selected surface work from both EVAs. |
| Photograph Surveyor before taking its camera, scoop and cable | Same NASA mission account describes documentation and these returned items. The game selects three items; it does not claim they were the entire returned hardware inventory. |
| Head / 12031; Bench / 12032; Sharp / 12027 | [NASA preliminary science report](https://www.nasa.gov/wp-content/uploads/static/history/alsj/a12/as12psr.pdf), [C. Meyer / 12031](https://www.lpi.usra.edu/lunar/samples/atlas/compendium/12031.pdf) and [12032](https://www.lpi.usra.edu/lunar/samples/atlas/compendium/12032.pdf). 12031 is basalt; 12032 is fines. Bench now depicts and collects soil. Existing LROC mapping is retained. |
| Sharp core 12027 | [NASA TM X-58066](https://ntrs.nasa.gov/citations/19720008095) identifies the Sharp core and its recovered 17.4 cm length. [Smithsonian core assembly](https://www.si.edu/object/core-tube-assembly-apollo%3Anasm_A19720827000) describes hammer insertion and capping; metadata is referenced, no museum image is copied. The visible tool length and click count are authored gameplay. |
| ALSEP deployment, leveling and radio link | [Apollo 12 crew ALSEP handbook](https://www.lpi.usra.edu/lunar/ALSEP/pdf/31111000673218.pdf). The bubble and signal controls condense a much longer deployment. The -7 degree bearing is a simplified scene target, not an ephemeris solution. |
| Landing site and route | `SourceArt/Apollo12/manifest.json` retains raw source URLs, SHA256, crop and coordinate transformation. Modern LROC data is explicitly identified as modern orbital mapping. A 2 m grid does not imply 2 m absolute accuracy. Small rocks, tools, models and their collision are authored art. |
| Intrepid model | NASA / Michael D. Carbajal; retained attribution in `SourceArt/ExternalModels/CREDITS.md`. Model is a visual approximation; source-derived work-site positions are independent of its mesh. |
| Suit, hands, rig, tools, Surveyor and ALSEP details | Original project-authored approximations. [NASA suit history](https://www.nasa.gov/history/50-years-ago-certifying-apollo-spacesuits/) identifies A7L for Apollo 7-14, distinguishing it from later A7LB suits. GLB, OBJ, JSON and generation scripts are included. No new third-party character, museum scan or motion capture was added. |
| Radio | Original English text with local Microsoft Zira synthesis. It is not NASA archival audio or an astronaut impersonation. On-screen subtitles use the same source text; suit/tool sounds remain synthetic. |
| Time | Untimed historical exploration is the default. A separate 30-minute exercise is explicitly an exercise; it is not suit oxygen endurance. Old timed checkpoints retain their mode. |
| Ending | Cargo and photographs come from the actual session. The historical return to Earth is context, not an extra playable launch or flight simulation. |

NASA imagery/media guidance was reviewed at
[NASA images and media](https://www.nasa.gov/nasa-brand-center/images-and-media/).
Existing third-party assets retain their individual credits; NASA provenance is
not used to relicense them. The game does not claim NASA endorsement.

Source references are paraphrased. No new archival video, full document text,
NASA voice recording, or trademark-based endorsement was added in this update.
