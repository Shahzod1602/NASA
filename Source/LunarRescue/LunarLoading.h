#pragma once
#include "CoreMinimal.h"

// One local travel at a time. Options (including ResumeApollo) survive unchanged.
namespace LunarLoading {
void Initialize();
void Shutdown();
bool Open(const UObject* Context, FName Map, const FString& Options = FString());
bool IsBusy();
bool Completed(const FString& Map, bool RequireRendered);
}
