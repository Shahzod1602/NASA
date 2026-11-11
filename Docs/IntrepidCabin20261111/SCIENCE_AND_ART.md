# Cabin source, science and art boundaries

Date: 2026-11-11. Source recovery, the separate 46-check mesh build, material
construction, packaged/runtime comparisons and scoped static captures are
verified. These are educational presentation results, not certification of
scientific fidelity, photorealism or professional training.

| Element | Actual role | Limit |
| --- | --- | --- |
| Retained Apollo Lunar Module | NASA / Michael D. Carbajal reference model already used by LunarRescue | Artistic model; not a measured reconstruction of every LM-6 Intrepid panel |
| Recovered cabin normals | Explicit normals from the original GLB, transformed through the verified original conversion | Restores source shading choices; does not add geometry, improve the silhouette or guarantee perfect source normals |
| Cabin derivative | Separate mesh with computed source frames baked into its editable data, then authored cabin normals applied | Original geometry/UVs/materials and other attributes remain exact; raw non-cabin frame attributes intentionally change |
| Technical tangent reconstruction | 12 cabin axis fallbacks, plus 836 zero/parallel outside tangent instances on 570 faces across five original slots | Rendering bases only; not GLB-authored tangents, repaired UVs, material weave measurements or new model geometry |
| Two neutral material tints | Original GLB RGB factors 0.498039216 and 0.678431392 | Source art factors, not calibrated lunar radiometry |
| Rough cover and stitching | Newly authored shader: roughness 0.78, specular 0.18, metallic 0; subtle local-space color variation | Gameplay presentation tuning, not measured fabric/thermal-blanket optical properties or exact historical seams |
| Existing Apollo ground | Established LROC dataset, terrain mesh, collision and worksite coordinates | Cabin work imports no new elevation data and adds no centimeter-scale measurements |

Authored GLB normal recovery is limited to 1,237 triangles / 3,711 corners in
`polySurfac.002` and `polySurfac.002_1`. Baking the displayed source frames into
the new mesh affects its full editable frame table, with the explicitly recorded
outside tangent exceptions above. The two shared white material slots also
contain retained trim and other small model parts, so their shader finish has a
wider art scope than the normal patch. Material sharing must not be described as
editing only the two cabin objects.

The accepted CPU-render-buffer checks establish exact triangle geometry, Unreal
UVs, material assignment and winding. Outside normals and valid tangents differ
by no more than one 8-bit component step plus float epsilon; W/sign values are
exact. Reconstructed degenerate tangents have a separate finite/unit/orthogonal
policy. This is not bitwise outside-frame identity or proof of identical GPU
pixels, shadows or material response. The original source asset and render data
remain unchanged.

Stitch spacing is 80 source-local centimeters, approximately 110 world
centimeters at the current 698 cm lander-height normalization. This is a visual
pattern, not recovered fabrication dimensions. Shader color variation and
roughness add no collision surface, deformation, WPO, physics or new lighting.
No new image, texture, external model, paid asset or AI-generated model is used.

The [official NASA model page](https://science.nasa.gov/3d-resources/apollo-lunar-module/)
credits NASA/Michael D. Carbajal. NASA's [Images and Media Usage Guidelines](https://www.nasa.gov/nasa-brand-center/images-and-media/)
cover educational/informational use of 3D polygon and texture media with source
credit and without implied endorsement. Both pages were checked on 2026-11-11.
A separate HTTP identity check verified the retained 716,840-byte GLB against
the official download, with SHA-256
`379101dfcee399267addf771709107c35e826b0b9e5233272f123e4c8a585c4e`.
NASA identifiers/logos and separately credited third-party works retain their
own restrictions. Existing project licensing is unchanged.

Source hashes, raw copies, reproducible processing and detailed credits are in
[the local source package](../../SourceArt/IntrepidCabin20261111/README.md) and
[its credits](../../SourceArt/IntrepidCabin20261111/CREDITS.md). Codex assisted
with analysis, implementation, synthetic fixtures and documentation. NASA
endorsement, professional astronaut training and Space Apps eligibility are not
established by this presentation change. Mars retains its separate environment.

## Verification scope

The [verification index](verification-index.json) binds final packaged evidence
to executable `59d4f795b1141694741264370efe9b79dad66eaa9b4487270ffe7f9517fc358c`.
It separates 1,763 passing packaged assertion executions across 31 reports from
182 preliminary Editor assertions, 46 asset-builder checks and nine offline
fixtures. Repeated executions are not unique tests. Editor views precede a
source/protocol annotation refresh; their selected-input fingerprint is not
asserted equal to the final package. The normal payload itself remained
byte-identical.

Final preservation confirms 915 protected originals (906 Saved, five Config,
four maps), 32 original asset/source identities and the earlier 154 science/art
payloads. Eight isolated/config file records required three byte and eight time
restorations; 80 true owner save/photo files stayed unchanged without
restoration. Owner filenames/inventories are omitted from public evidence.
Prior surface-pass documents and accepted receipts also remain unchanged.

Representative images support smoother source shading, with a still-faceted
silhouette and subtle cloth effect. Bright/low-contrast white areas remain;
small LDR pixel samples do not diagnose global HDR clipping. The one capped,
static camera pair does not prove a causal performance gain, universal FPS or
human usability. Material-sharing and technical tangent exceptions remain the
explicit art/rendering scopes described above.
