#pragma once
#include "CoreMinimal.h"
class ALunarGameMode;

// Authored cosmetic equipment only. Call after mission actors exist; safe to repeat.
namespace LunarEquipmentDetail {
 constexpr int32 MaxComponents=128;
 constexpr int32 MaxInstances=1400;
 void Build(ALunarGameMode* Game);
 void AppendChecks(ALunarGameMode* Game,FString& Report,int32& Failed);
}
