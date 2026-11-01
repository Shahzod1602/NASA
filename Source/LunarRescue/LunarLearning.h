#pragma once
#include "CoreMinimal.h"
class ALunarGameMode;

namespace LunarLearning {
 constexpr int32 Count=3;
 FString Question(const ALunarGameMode* Game,int32 Index);
 FString Choice(const ALunarGameMode* Game,int32 Index,int32 Answer);
 FString Explanation(const ALunarGameMode* Game,int32 Index,int32 Answer=INDEX_NONE);
 FString Source(const ALunarGameMode* Game,int32 Index);
 int32 CorrectAnswer(const ALunarGameMode* Game,int32 Index);
 int32 Reviewed(const ALunarGameMode* Game);
 int32 Correct(const ALunarGameMode* Game);
 FString SessionSummary(const ALunarGameMode* Game);
 bool RecordAnswer(ALunarGameMode* Game,int32 Index,int32 Answer);
}
