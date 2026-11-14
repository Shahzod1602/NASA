#pragma once
#include "CoreMinimal.h"
class UWorld;
namespace LunarMissionIntro {
// Local optional clips under Content/Movies; absence never blocks mission entry.
bool Eligible(const FString& Map,const FString& Options,bool SelectedTravel);
void Play(UWorld* World,const FString& Map);
}
