# Lunar Rescue — The Last Signal

The five-minute mission now includes a fictional crew mystery and an optional ending.

1. Pick up the rover power cell with **E**. It is visible in the astronaut's hands.
2. At the external power port, press **E** to insert it, then **E** to connect the cable.
3. Hold **E** for two seconds to restart the station. Wait for the station lights and antenna.
4. Hold **E** for three seconds at the science terminal to recover the data.
5. Return to the lander, or press **Q** to track the optional crew recorder. Press **Q** again to return to the lander route.
6. Jump onto the survey ledge and hold **E** to recover the recorder. The home marker returns automatically.

Releasing E, moving out of range, or looking away cancels an unfinished download or restart. Pausing cancels the held interaction and freezes oxygen and the station restart sequence. The recorder detour consumes the same oxygen budget; it is optional for victory.

The final screen reports remaining oxygen, restored power, secured science, and recovered logs. Recovering the optional recorder reveals that the crew shut down the rover to preserve samples after cooling failed, then evacuated to the relay.

All dialogue and interface text are English. Seven locally synthesized radio clips are included. Terrain and story are fictional; there are no new claims of authentic lunar measurements.

Implementation: `Source/LunarRescue/LunarGame.cpp` and `.h`, `Config/DefaultInput.ini`, and `Content/Lunar/Immersion/Audio/Radio_Story*.uasset`. The recorder, survey ledge, installed cell and cable are spawned at runtime; the saved map is preserved. Original source/config backups are in `Backups/BeforeStory`.

The hand pose is a simple procedural mesh pose, not a fully rigged astronaut animation. The visible cable is a simplified connector. This remains an Unreal Editor game build, not a packaged distribution.

Validation: the Editor build succeeds; 43 mission integration checks and three physical ledge traversal checks pass. The traversal check uses simulated Space/W/S input, then recovers the recorder after landing. Mission tests use teleports between objectives, so they do not establish a complete walked playthrough time. See `gameplay-checks.txt`, `ledge-checks.txt` and `story-build.log`.

The first graphics review failed due to Windows memory exhaustion. A subsequent review succeeded with a 384 MB texture streaming pool and shadow quality 2. `Play Lunar Rescue - Low Memory.cmd` provides these settings without changing the default project graphics configuration.
