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

## 0.6.1 photo and animation optimization

Mission screenshots retain the engine's capture/readback path, then copy pixels
into ImageWriteQueue for background PNG encoding and disk writes. Evidence and
its checkpoint are committed after a successful write. A failed write leaves
progress unchanged and permits retry. Screenshot listeners keep a local strong
reference while unbinding; job references are thread-safe across the writer.

The work camera now updates before tool contacts and glove poses. Normal frames
perform one work-visual update. Album navigation retains the three most recently
viewed textures on the character, checks the current run's path prefix, and
evicts older entries. First-time decoding still happens when a photo is opened.

The final Windows runtime passed 363 functional checks plus 20 rendered photo
checks, including failed-write/retry, listener cleanup across successive photos,
cache eviction and retention through garbage collection. Thirty-two rendered
views were checked. The visual-review driver waits for photo completion before
moving the camera or opening the next tool.

On the development laptop, 1920x1080 opening-route captures used DX11, a 60 FPS
cap, no audio, 4,200 frames and 600 excluded startup frames:

| Preset | Mean FPS | 1% low FPS | p99 frame ms | Largest frame ms |
|---|---:|---:|---:|---:|
| Low | 60.00 | 56.90 | 17.31 | 19.71 |
| Medium | 60.00 | 56.75 | 17.35 | 20.58 |

These are short rendered-offscreen route samples. Each preset has one capture;
background workload was not controlled. Existing graphical presets and suit
LODs are retained. Other hardware and on-screen presentation need separate
measurements. The real five-person study remains pending; the protocol now
records wrong clicks, help requests, photo pauses and motion discomfort.

Build notes: the constrained Windows host required native compilation/linking
outside the managed build process, using its unchanged generated response
files. A bounded incremental cook completed all 712 packages without errors.
The editor cook configuration limits preloading and save queues and performs
more frequent garbage collection; these settings do not alter runtime graphics.

The audio-enabled full-mission comparison used 1280x720 Medium, DX11 and a
60 FPS cap, excluding 600 startup frames in each capture:

| Version | Mean FPS | 1% low FPS | p99 frame ms | Largest frame ms |
|---|---:|---:|---:|---:|
| 0.6.0 | 59.93 | 53.87 | 16.69 | 169.85 |
| 0.6.1 | 59.99 | 59.41 | 16.68 | 27.45 |

The final mission covered 1,238.03 m in 576.72 seconds, returned all samples and
hardware, and saved four actual context photographs. Peak engine-reported
memory was 647.47 MiB. Copy/enqueue time was 0.156-0.230 ms per photo; request
to pixels was 17.140-23.101 ms, including the wait for rendering. PNG writing
completed in the background after 87.471-154.399 ms including callback dispatch.

This is one full capture per build on the same development laptop with audio
enabled and normal simulation steps. Background workload was not controlled.
The average is constrained by the 60 FPS cap; the largest-frame difference is
observed evidence, not a general speedup guarantee. GPU readback remains on the
engine screenshot path. Twenty pre-existing save/photo/settings files were SHA256-verified
unchanged after testing, including the owner's settings, mission and photos.
