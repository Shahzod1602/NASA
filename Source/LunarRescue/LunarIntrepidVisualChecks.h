#pragma once
class ALunarGameMode;

// Isolated functional, staged review and normal-delta visual profiling drivers.
namespace LunarIntrepidVisualChecks {
void Start(ALunarGameMode* Game,bool Review,bool Profile=false);
}
