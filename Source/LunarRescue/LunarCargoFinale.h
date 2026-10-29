#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LunarCargoFinale.generated.h"

class ALunarGameMode;
class APlayerController;
class ALunarCharacter;
class UCameraComponent;
class USceneComponent;

// Authored stowage vignette after completion has already been committed.
// It never awards cargo, edits the mission clock or changes the pawn's camera.
UCLASS()
class LUNARRESCUE_API ALunarCargoFinale : public AActor {
 GENERATED_BODY()
public:
 ALunarCargoFinale();
 static ALunarCargoFinale* Begin(ALunarGameMode* Mission);
 static bool Active(const ALunarGameMode* Mission);
 static bool Skip(ALunarGameMode* Mission);
 static void Reset(ALunarGameMode* Mission);
 static void ValidateChecks(ALunarGameMode* Mission,TFunctionRef<void(bool,const TCHAR*)> Check);
 virtual void Tick(float Delta) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void AdvancePresentation(float Delta);
 void Finish();
 FString Caption() const;
 float Progress() const;
 int32 CargoCount() const {return CargoRoots.Num();}
 float PresentationTime() const {return Elapsed;}
 bool IsPlaying() const {return Playing;}
private:
 bool Initialize(ALunarGameMode* Mission);
 void BuildCargo();
 void PoseCargo();
 void RestoreView();
 UPROPERTY() TObjectPtr<ALunarGameMode> Game;
 UPROPERTY() TObjectPtr<APlayerController> Player;
 UPROPERTY() TObjectPtr<ALunarCharacter> Astronaut;
 UPROPERTY() TObjectPtr<UCameraComponent> ShotCamera;
 UPROPERTY() TArray<TObjectPtr<USceneComponent>> CargoRoots;
 TArray<FVector> CargoTargets;
 TArray<FString> CargoLabels;
 TWeakObjectPtr<AActor> PreviousViewTarget;
 uint8 SampleMask=0,PartMask=0;
 float Elapsed=0;
 bool Playing=false,PreviousPawnHidden=false;
};
