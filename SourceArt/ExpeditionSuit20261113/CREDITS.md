# Animated Floating Astronaut in Space Suit Loop

- Author: **LasquetiSpice**.
- Original: https://sketchfab.com/3d-models/animated-floating-astronaut-in-space-suit-loop-e2c4b146e58141e4b87917456a9970b1
- License: **Creative Commons Attribution 4.0 International (CC BY 4.0)** — https://creativecommons.org/licenses/by/4.0/
- Original SHA-256: `6008551be3e7f9668b73d2cfb12c1c7499db6e432303210f753791873f5c861d`.
- The unchanged supplied file, including its floating animation, is retained in `Raw`.

Changes made with OpenAI Codex assistance: authored bind-pose and weight adaptation from the supplied 91-joint skeleton to the existing 36-bone game controller; separate body and first-person hand derivatives; texture size reduction to 2048 pixels; Unreal import and LOD generation. Torso/accessory branches follow anatomical torso anchors instead of the global root. Geometry, base-colour UVs and normal UVs are retained. Legacy specular/glossiness is converted to core metallic/roughness after a rendered colour failure: authored metallic is zero and roughness is `max(0.12, 1 - source glossiness)`, including the source texture alpha. This is a visual approximation rather than measured physical fabric properties. See `adaptation.json` and `Tools/prepare_expedition_suit.py`.

The floating loop is **not** used as a ground walking animation. Ground movement uses the game's procedural posing and character collision controller. Animation quality and grip fit require actual rendered validation. This is a visual game asset; it is not a verified historical Apollo pressure-suit model or a NASA-approved training model. The author does not endorse the game.

Arm segments are adapted along their length so each source segment endpoint meets the next legacy bind pivot. This prevents the original longer forearm sleeve from extending over the glove; inverse-transpose normal correction accompanies the nonuniform adaptation. Original source topology and all 58,312 body triangles remain in the derivative.

14 November 2026 palm correction: alignment now uses the wrist-to-middle-finger axis and the index-to-little-finger spread to retain palm/finger roll. The earlier direction-only adaptation rotated the palm sideways by about 84 degrees in the orthogonal palm frame. Palm width is fitted to the target MCP spread, preserving depth, with a small authored shear for source/target axis differences. Finger frames share the palm's transverse reference. These are visual rig adaptations, not anatomical or pressure-suit measurements. Source topology, UVs, source files and existing licence remain unchanged.
