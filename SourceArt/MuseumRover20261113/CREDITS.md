# Mondfahrzeug (Lunar Rover)

- Author: **Deutsches Museum | Digital** — https://sketchfab.com/deutsches-museum
- Original: https://sketchfab.com/3d-models/mondfahrzeug-lunar-rover-acb3f8645a69493cb211a62b40d66aab
- License: **Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)** — https://creativecommons.org/licenses/by-sa/4.0/
- Supplied file: `Raw/mondfahrzeug_lunar_rover.glb`, acquired locally from the authorized game-feedback attachment; source attribution and license verified against embedded metadata and the public Sketchfab API.
- Original SHA-256: `f416182a410f98e562d50661ae408affa613e67cfd1e9af72e82a1c6d66fb19b`.

Changes made with OpenAI Codex assistance: baked node transforms; separated the source wheel-material geometry into four disjoint wheel quadrants; assigned wheel-centred pivots; retained source geometry, UVs and materials; capped texture dimensions at 2048 pixels; generated Unreal static-mesh LODs and connected the visual parts to the existing authored vehicle controller. See `adaptation.json` and `Tools/prepare_museum_rover.py` for reproducible details. The original unmodified file is retained.

**These adapted model files and the corresponding imported model assets remain under CC BY-SA 4.0.** This notice does not replace the project's code license or license unrelated third-party assets. Credits and adaptation information must accompany redistributed versions of this model. The model author does not endorse this game.

Gameplay boundary: this is an authored modern training deployment using a museum model. Apollo 12 did not use a lunar rover. The driving controller is a bounded kinematic game model, not a validated vehicle dynamics or astronaut training simulation. Source-wheel dimensions describe this digital model, not certified flight hardware measurements.
