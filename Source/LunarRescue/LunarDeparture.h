#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LunarDeparture.generated.h"

class ALunarGameMode;
class ALunarCharacter;
class APlayerController;
class UCameraComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

// A transient authored cabin vignette before mission state is committed.
// It never moves the player, changes a dataset/map or writes a checkpoint.
UCLASS()
class LUNARRESCUE_API ALunarDepartureScene : public AActor {
 GENERATED_BODY()
public:
 static constexpr float Duration=1.4f;
 static constexpr float SkipDelay=.3f;
 static constexpr int32 MaxVisualComponents=48;
 ALunarDepartureScene();
 static ALunarDepartureScene* Begin(ALunarGameMode* Mission);
 static bool Active(const ALunarGameMode* Mission);
 // Returns true while the scene owns the input, including the initial delay.
 static bool Skip(ALunarGameMode* Mission);
 static void Abort(ALunarGameMode* Mission);
 static void Reset(ALunarGameMode* Mission);
 static void ValidateChecks(ALunarGameMode* Mission,TFunctionRef<void(bool,const TCHAR*)> Check);
 virtual void Tick(float Delta) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void AdvancePresentation(float Delta);
 void Finish(bool Commit=true);
 FString Title() const;
 FString Caption() const;
 FString Guide() const;
 float Progress() const;
 float PresentationTime() const {return Elapsed;}
 float HatchFraction() const;
 int32 VisualCount() const {return Visuals.Num();}
 bool IsPlaying() const {return Playing;}
 bool HasCommitted() const {return Committed;}
private:
 bool Initialize(ALunarGameMode* Mission);
 bool BuildCabin();
 UStaticMeshComponent* Box(FVector Position,FVector Dimensions,int32 Material=0);
 UStaticMeshComponent* Rod(FVector From,FVector To,float Radius=2,int32 Material=2);
 UStaticMeshComponent* Shape(UStaticMesh* Mesh,FVector Position,FVector Dimensions,int32 Material,FQuat Rotation=FQuat::Identity);
 void Pose();
 void RestoreView();
 UPROPERTY() TObjectPtr<ALunarGameMode> Game;
 UPROPERTY() TObjectPtr<ALunarCharacter> Astronaut;
 UPROPERTY() TObjectPtr<APlayerController> Player;
 UPROPERTY() TObjectPtr<UCameraComponent> ShotCamera;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Visuals;
 UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> Materials;
 UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
 UPROPERTY() TObjectPtr<UStaticMesh> CylinderMesh;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> LeftDoor;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> RightDoor;
 TWeakObjectPtr<AActor> PreviousViewTarget;
 FVector FinalEye=FVector::ZeroVector,PreviousVelocity=FVector::ZeroVector;
 FRotator FinalAim=FRotator::ZeroRotator;
 float Elapsed=0,PreviousFOV=85;
 uint8 PreviousMoveMode=0,PreviousCustomMode=0;
 bool Playing=false,Committed=false,PreviousPawnHidden=false,PreviousMovementTick=true;
};
