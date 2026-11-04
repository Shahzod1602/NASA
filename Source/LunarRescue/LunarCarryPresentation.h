#pragma once
#include "CoreMinimal.h"

class ALunarCharacter;
class ALunarGameMode;

// Cosmetic carry proxies only. Mission commits, resource costs and the existing
// 2.2 second Apollo stow clocks remain owned by gameplay.
namespace LunarCarryPresentation {
bool FitViewBounds(const FBox& SourceBounds,float HorizontalFov,float Aspect,FTransform& Relative);
bool ProjectViewBounds(const FBox& SourceBounds,const FTransform& Relative,float HorizontalFov,float Aspect,FBox2D& Normalized);
bool ShouldShow(const ALunarCharacter* Player,const ALunarGameMode* Game,bool FirstPerson,bool SuppressPhoto=false);
bool IsCellCarried(const ALunarGameMode* Game);
bool CellGrip(const ALunarCharacter* Player,float Side,FVector& CameraSpacePoint);
void Build(ALunarCharacter* Player);
void Update(ALunarCharacter* Player,bool SuppressPhoto=false);
void ValidateChecks(ALunarGameMode* Game,TFunctionRef<void(bool,const TCHAR*)> Check);
void RunChecks(ALunarGameMode* Game);
int32 ReviewFixtureCount();
bool ApplyReviewFixture(ALunarGameMode* Game,int32 Index,FString& Name);
}
