#pragma once
#include "CoreMinimal.h"
class ALunarGameMode;
class ULunarPanel;

// Two explicitly fictional guides; their advice never performs mission work.
namespace LunarCrew {
 constexpr int32 Count=2;
 constexpr int32 MaxComponents=40;
 void Build(ALunarGameMode* Game);
 int32 Nearby(const ALunarGameMode* Game);
 FString Prompt(const ALunarGameMode* Game);
 bool Talk(ALunarGameMode* Game,int32 Index=INDEX_NONE);
 void BuildNotebook(ULunarPanel* Panel);
 void ValidateChecks(ALunarGameMode* Game,TFunctionRef<void(bool,const TCHAR*)> Check);
}
