# Intrepid visual pass — science and art provenance

Date: 2026-11-11. Status: provenance documented; revised appearance has packaged 720p/1080p structural checks, representative direct image review and scoped static captures. An independent read-only check confirms all 154 original Content/SourceArt payloads retain their size/SHA-256; the separate final all-run receipt confirms 869 original owner/config/map files have their original bytes and modification times. No new measurements are claimed. The distinction below concerns the new local visual pass over the existing Apollo 12 scene.

## What is measured, derived and authored

| Element | Source and role | Scientific limit |
| --- | --- | --- |
| Apollo ground elevations | Existing LROC stereo-derived `NAC_DTM_APOLLO12`, version 1.9, dataset `LRO-L-LROC-5-RDR-V1.0`; 2 m post spacing. The imported mesh controls terrain collision and worksite heights. | A 2 m source grid does not measure centimeter-scale grains, every stone, footprints or equipment contacts. Interpolation does not add measurements. |
| Imported local mesh and coordinate conversion | Existing pipeline and manifest: UE X east, Y south, Z up; 100 Unreal units = 1 m, with no vertical or horizontal exaggeration. | The new pass does not reproject, resample, regenerate or replace the elevation source. |
| Regolith bitmap | Existing `T_RegolithAuthored.png`, generated with an AI image tool for the project on 2026-10-25. | Authored appearance, not a NASA surface photograph, surveyed albedo or measured normal map. No new image generation is part of this pass. |
| Fine surface shading | New multiscale color/normal shader variation using the retained bitmap and filtered procedural noise. | Visual relief only; it creates no measured small-scale terrain or collision detail. |
| Rock shapes and scatter | Six retained project-authored procedural basalt meshes, deterministically arranged over actual terrain hits. | Shapes, sizes, clustering and 25% bound burial are art choices, not an Apollo rock catalog, sample distribution or geology result. |
| Intrepid source geometry | Existing NASA / Michael D. Carbajal Apollo Lunar Module model, already imported into the project. | A reference model with project presentation adaptations; the new materials and added folds are not measurements of the flown vehicle. |
| Foil, metal and blanket folds | New project-authored material graphs and 12 small fold instances in existing batches; the original ladder is retained without added rung strips. | Material coefficients and detail locations are tuned for readability, not laboratory optical properties or exact historical construction. |
| Local contact shading | Bounded AO intensity/radius and a smooth spatial blend. | Rendering approximation; it is not a lunar illumination solver or a simulation of dust mechanics. |

The existing height-product label and processing records remain authoritative for the source product: [dataset manifest](../../SourceArt/Apollo12/manifest.json), [PDS label](../../SourceArt/Apollo12/Raw/NAC_DTM_APOLLO12.LBL), and [project science document](../../SCIENCE_AND_SOURCES.md). The underlying stereo imagery was acquired in 2010; the retained product was created in 2022. It is not a contemporaneous 1969 ground scan. This pass does not import a new LOLA raster or claim an improvement in source terrain resolution.

## Source image identity

The retained art bitmap's source SHA-256 is `3c1b4fc0e37a24e53a626434ae5d6e5751f299491ffd7138af34e77c3293ddab` (1,254 × 1,254 pixels, as recorded in the existing source manifest). Its prior intended art tiling scale is 1.5 m; the new shader adds other authored frequencies. Neither pixel density nor shader repetition is a measurement of lunar grain size.

Original image provenance is recorded in [Realism credits](../../SourceArt/Apollo12/Realism/CREDITS.txt) and [material source manifest](../../SourceArt/Apollo12/Realism/material-source.json). The script records the retained file identity, and the separate 154/154 original-payload comparison confirms the original Content/SourceArt inputs remain byte-identical. This read-only identity check performs no restoration and is distinct from automation's owner save/config backups. See [verification index](verification-index.json) for the receipt scope.

## Lighting context and artistic limits

NASA describes the Moon's weak atmospheric scattering and the strong difference between directly lit regions and shadows. This supports keeping an airless, high-contrast lunar presentation; it does not validate this game's numeric AO, roughness, exposure or fill settings. See [NASA: how light looks different on the Moon](https://www.nasa.gov/centers-and-facilities/ames/the-dark-side-of-the-crater-how-light-looks-different-on-the-moon-and-what-nasa-is-doing-about-it/) (article dated 2017-07-26; checked 2026-11-11).

The new pass changes local surface and contact appearance without adding a sun, sky, fill light or exposure override. Existing artistic lighting remains part of the baseline. Shadows and highlights must be reviewed for readable shapes and restrained material response, not described as photometrically validated Apollo illumination.

Epic's [detail-texturing documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/adding-detail-textures-to-unreal-engine-materials) explains adding apparent close-range detail through layered color/normal information. It supports the rendering technique, not its scientific authenticity. The page currently resolves to the latest documentation; the actual project engine/version and successful material compilation must be recorded by the build report.

## Credits and reuse

LROC elevations retain the established NASA/GSFC/Arizona State University and LROC science-team credit. Apollo Lunar Module geometry retains NASA / Michael D. Carbajal attribution and its [official source](https://science.nasa.gov/3d-resources/apollo-lunar-module/). Existing imported image-bearing materials are preserved rather than replaced by the new foil finish. The c Editor views show the retained ladder and readable lit foil; the existing white ascent cover is still coarse/faceted. This is a bounded presentation improvement, not a newly reconstructed or photorealistically certified lander. Staged capture hides HUD and text-rendered world labels, so it is not a review of all ordinary gameplay presentation.

The retained basalt geometry is project-authored procedural art; its provenance is in [Lunar V3 credits](../../SourceArt/LunarV3/CREDITS.md). Existing model/texture credits, including any separately licensed third-party material, remain in [external-model credits](../../SourceArt/ExternalModels/CREDITS.md). Only the existing meshes/bitmap and four newly authored materials are needed for this pass; no new third-party model or commercial generation service is introduced.

The new material code, arrangement rules, cosmetic detail and documentation were developed with OpenAI Codex assistance. The prior regolith bitmap's AI authorship remains disclosed. NASA media usage follows the existing [NASA images/media guidance](https://www.nasa.gov/nasa-brand-center/images-and-media/); third-party terms remain separate. This work does not choose or change the project's code license or apply a code license to third-party assets. See [project credits](../../CREDITS.md) for the unresolved project-code licensing decision.

These are educational game visuals. They do not establish NASA collaboration/endorsement, professional astronaut training, physiological accuracy, measured route safety or Space Apps eligibility. Mars keeps its separate fictional presentation; lunar appearance should not be presented as a Mars surface model.
