# Source and derivative credits

Source model: **NASA / Michael D. Carbajal**, Apollo Lunar Module.
The official [NASA model page](https://science.nasa.gov/3d-resources/apollo-lunar-module/)
identifies this source and provides the original GLB. This attribution was
checked on 2026-11-11. The bundled raw GLB is an unchanged copy of the existing
locally supplied source; the bundled OBJ is the existing project conversion,
not a newly authored NASA model.

A separate HTTP check on 2026-11-11 verified the retained GLB against the
[official NASA GLB](https://assets.science.nasa.gov/content/dam/science/cds/3d/resources/model/apollo-lunar-module/Apollo%20Lunar%20Module.glb):
both contain 716,840 bytes with the SHA-256 listed below. Identity verification
does not certify the model as a measured reconstruction of the flown vehicle.

NASA's [Images and Media Usage Guidelines](https://www.nasa.gov/nasa-brand-center/images-and-media/),
checked on 2026-11-11, describe educational and informational use of NASA media,
including polygon and texture data used for 3D models, with source acknowledgment
and without implying NASA endorsement. NASA names, identifiers and logos have
separate usage restrictions; this package does not claim that every identifier
or third-party contribution is unrestricted or public domain.

LunarRescue does not claim NASA sponsorship, approval, certification or a
partnership. This reference model and its authored normals are artistic source
content, not measured Apollo 12 cabin geometry or lunar topography. Existing
repository licensing and third-party credits are unchanged.

Project-derived work on 2026-11-11: exact source conversion reproduction,
cabin-only authored normal extraction, computed source-frame baking into a new
mesh duplicate, explicitly reported degenerate-tangent rendering repairs,
authored cover-material shading, mapping validation and integration support.
The 12 cabin fallback bases and outside zero/parallel tangent repairs are
technical coordinate frames, not GLB-authored tangents or scientific measurements.
OpenAI Codex assisted with source analysis, code, validation fixtures and
documentation. No AI-generated replacement model or new texture was added for
this normal-recovery package. NumPy and `trimesh` are processing dependencies;
their software is not redistributed in this directory.

Source SHA-256 values:

- `Raw/Apollo Lunar Module.glb`:
  `379101dfcee399267addf771709107c35e826b0b9e5233272f123e4c8a585c4e`
- `Raw/ApolloLander.obj`:
  `7b034359fbfa5c42cd576e129e36e29a9272c178b5b99d95de0ed260c843b8d8`

See [README.md](README.md) and `source-normal-recovery-report.json` for the
exact geometry scope, processing steps, scientific limits and verification.
