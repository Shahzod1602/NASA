# Astronaut movement and suit update - 0.4.1

The Apollo 12 chapter now has an authored A7L-inspired suit with fitted sleeves, blue wrist locking rings, padded articulated gloves, torso, helmet, life-support pack, legs and overboots. These are original illustrative meshes and procedural animation, not a scanned historical suit or motion-captured astronaut.

- WASD moves; Shift holds a faster 2.2 m/s stride on Apollo 12, compared with the normal 1.4 m/s walk. Acceleration and braking are gradual. Space performs the existing low-gravity hop.
- V switches between the stable helmet view and a following camera. Hold Alt and move the mouse in the following view to inspect the suit from another angle.
- Looking down in helmet view reveals the suit and boots. First-person sleeves and fingers have a separate near-camera pose to keep the work area readable. Finger curl and hand reach blend into work actions.
- The lower body uses two-link leg posing with terrain height checks; arm and leg motion follows actual travel rather than playing a walk cycle while stationary. Camera collision keeps the following view out of the terrain.
- Pausing freezes the exercise clock, locomotion and suit animation. The journal retains the same pause behavior.
- Markers now name their work site, and each Apollo stage has an immediate instruction explaining where to go and which key to use. Antenna bearing remains visible while aiming.

Source meshes and authoring credits: `SourceArt/Suit/`. Rebuild with `python Tools/prepare_suit.py`, then import through Unreal's Python commandlet with `Tools/import_suit.py`. `Source/LunarRescue/LunarSuit.cpp` drives the suit and view modes. `/Game/Lunar/Suit` is always cooked so the dynamically loaded meshes are present in the standalone package.

Validation uses `-SuitTest` for actual key-driven walk/fast stride/strafe/hop, pause and view checks; `-SuitReview` captures hands, downward body view, third-person suit and work pose. Apollo progression and the existing Moon/Mars regression cases remain available in `Tools/verify_apollo12.ps1`.
