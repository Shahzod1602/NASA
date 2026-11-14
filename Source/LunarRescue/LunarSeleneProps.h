#pragma once
#include "CoreMinimal.h"
class ALunarGameMode;
namespace LunarSeleneProps {
 void Build(ALunarGameMode* Game);
 void ModulePickedUp(ALunarGameMode* Game);
 void ValidateChecks(ALunarGameMode* Game,TFunctionRef<void(bool,const TCHAR*)> Check);
}
