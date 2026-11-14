#pragma once
#include "CoreMinimal.h"

class ALunarGameMode;
namespace LunarSpaceTech {
// Read-only mission context; never interpreted as simulation coefficients.
FString ForMission(const ALunarGameMode* Game);
}
