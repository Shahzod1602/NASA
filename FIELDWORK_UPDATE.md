# Apollo 12 fieldwork update — v0.6.0

Tools now have source contact points and animated tongs/cutter jaws. Pending rock
and soil move toward a labeled soft pouch; committing the seal deposits and
stows them. A placed core tube remains on the surface while the hammer works.
Surveyor fittings stay targeted until the release motion ends. Cancel restores
uncommitted rock; released hardware is cargo only after packing.

The cloth helmet hood and visor use a rounded rectangular outline. Surveyor has
an open triangular equipment frame, harness detail and panel dividers. The
existing NASA/Carbajal Intrepid mesh is normalized to roughly 6.98 metres high.
These remain simplified project-authored approximations, not engineering models.
Visual reference: [NASA AS12-48-7136](https://images-assets.nasa.gov/image/as12-48-7136/as12-48-7136~medium.jpg).

After its context photograph, K opens a separate optional Surveyor coating
investigation. Two actual in-game photographs support a selectable explanation;
the recorded interpretation follows [NASA 19720019093](https://ntrs.nasa.gov/citations/19720019093).
The panels' colors, shape and placement are illustrative. Surface discoloration
also involved radiation and contamination; see [NASA 19710024418](https://ntrs.nasa.gov/citations/19710024418).
No atmospheric wind or rain mechanism is presented as correct lunar physics.

Optional tasks retain checkpoint format 1. Old four-photo saves need no optional
progress; new six-photo saves require both records before the interpretation.
An unchanged automated v0.5.2 binary fixture exercises the real save reader.
The completion screen reports the actual number of evidence records.

The five-person study is pending real participants. PLAYTEST_GUIDE.md and the
isolated participant launcher are prepared; automated checks are not human data.
Performance evidence uses the same 1280x720 Medium, DX11, 60 FPS cap, audio-enabled
full automated mission before and after. Rendered offscreen results do not prove
on-screen comfort or hitch-free behavior on other hardware.

First-person forearm sections follow the posed wrists while the glove fingers
retain their weighted rig. The ground-work camera is raised to keep carried
samples above the terrain, and tools approach from the lower right so the
working tip stays visible. Photographs suppress hands, tools and the HUD.
K is reserved for the study; an older K action binding falls back to its default.

Rebuild source art with prepare_suit.py, prepare_suit_rig.py and
prepare_apollo_tools.py. After building the editor, Tools/import_fieldwork.ps1
imports static geometry, skins and final materials in separate editor processes
to release memory between stages. On this constrained machine a cold header
parse may need the package helper default 1024 MB managed heap; after it has
completed, -ManagedHeapMB 256 leaves more memory available for the native linker.

## Final validation

The final Win64 package passed 362 checks across Apollo, the older Moon/Mars
missions, menus, the full walk, suit input, work tools, guidance, optional science
and cross-process checkpoints. Thirty-two fresh rendered review states were
verified at 1280x720. The original owner save, five photos and settings match
their starting SHA256 hashes; the distribution ZIP excludes local player data.

| Audio-enabled full mission | Mean FPS | 1% low FPS | p99 frame ms | Largest frame ms |
|---|---:|---:|---:|---:|
| 0.5.2 | 59.92 | 52.80 | 16.6921 | 169.93 |
| 0.6.0 | 59.93 | 53.87 | 16.6885 | 169.85 |

These are single offscreen captures on this PC, Medium/DX11/1280x720 with a
60 FPS cap and 600 startup frames excluded. Normal simulation steps cover the
whole primary route. The small changes do not establish a general speedup.
Photo readback/PNG capture still causes a brief pause of about 0.17 seconds;
this build is not hitch-free. No human study or on-screen comfort result is
implied. Full captures and environment metadata remain in local Reports.
