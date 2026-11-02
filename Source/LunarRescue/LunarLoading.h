#pragma once
#include "CoreMinimal.h"

class SWidget;
class UTexture2D;
class UWorld;

// One local travel at a time. Options (including ResumeApollo) survive unchanged.
namespace LunarLoading {
void Initialize();
void Shutdown();
bool Open(const UObject* Context, FName Map, const FString& Options = FString());
bool IsBusy();
bool Completed(const FString& Map, bool RequireRendered);
// Staged layout review uses the exact paint widget, not actual map travel.
TSharedRef<SWidget> MakeTypographyPreview(UTexture2D* Texture,const FString& Map,float TextScale,int32 Percent);
void RunTypographyChecks(const UObject* Context);
void RunTypographyReview(UWorld* World);
}
