#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LunarSurfaceVehicle.generated.h"

class ALunarGameMode;
class ALunarCharacter;
class UFieldOperationsComponent;
class UBoxComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UPoseableMeshComponent;

UCLASS()
class LUNARRESCUE_API ALunarSurfaceVehicle : public AActor {
 GENERATED_BODY()
public:
 static constexpr int32 MaxVisualComponents=16;
 ALunarSurfaceVehicle();
 static ALunarSurfaceVehicle* Build(ALunarGameMode* Game);
 static bool IsRiding(const ALunarGameMode* Game);
 static bool TryInteract(ALunarGameMode* Game);
 static void Abort(ALunarGameMode* Game);
 void SetForward(float Value);
 void SetSteer(float Value);
 void RotateView(float Yaw,float Pitch);
 void ToggleView();
 bool IsCockpitView() const {return CockpitView;}
 float GetSpeedMps() const {return ActualSpeedMps;}
 float DistanceM() const {return TravelMeters;}
 bool IsOccupied() const;
 FString InspectionText() const;
 bool RestoreParkedPose(const FTransform& Pose);
 bool GroundedExitPose(FVector& Grounded) const;
 static void ValidateChecks(ALunarGameMode* Game,TFunctionRef<void(bool,const TCHAR*)> Check);
 virtual void Tick(float Delta) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditAnywhere,Category="Authored training movement") float ForwardLimitCm=450;
 UPROPERTY(EditAnywhere,Category="Authored training movement") float ReverseLimitCm=170;
 UPROPERTY(EditAnywhere,Category="Authored training movement") float AccelerationCm=140;
 UPROPERTY(EditAnywhere,Category="Authored training movement") float BrakingCm=240;
 UPROPERTY(EditAnywhere,Category="Authored training movement") float TurnRateDegrees=55;
 UPROPERTY(EditAnywhere,Category="Authored training movement") float SlopeLimitDegrees=18;
private:
 struct FGroundPose {
  FVector Centre=FVector::ZeroVector,Normal=FVector::UpVector;
  FVector Wheel[4];
 };
 UPROPERTY() TObjectPtr<UBoxComponent> Hull;
 UPROPERTY() TObjectPtr<UCameraComponent> FollowCamera;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Visuals;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Wheels;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Hubs;
 UPROPERTY() TObjectPtr<UPoseableMeshComponent> Driver;
 UPROPERTY() TObjectPtr<ALunarGameMode> Mission;
 UPROPERTY() TObjectPtr<UFieldOperationsComponent> Operations;
 UPROPERTY() TObjectPtr<ALunarCharacter> Rider;
 TWeakObjectPtr<AActor> PreviousViewTarget;
 TWeakObjectPtr<ALunarCharacter> RecoveryPawn;
 FTransform PreviousCamera=FTransform::Identity;
 FRotator PreviousControl=FRotator::ZeroRotator;
 FVector PreviousVelocity=FVector::ZeroVector;
 float PreviousFov=85,Throttle=0,Steering=0,SpeedCm=0,ActualSpeedMps=0,WheelPhase=0,TravelMeters=0;
 uint8 PreviousMoveMode=0,PreviousCustomMode=0;
 bool PreviousHidden=false,PreviousCollision=true,PreviousMoveTick=true,PreviousThird=false,PreviousCameraControl=true,Occupied=false,Ending=false;
 bool MuseumRover=false,CockpitView=false;
 float ViewYaw=0,ViewPitch=-20;
 void PoseDriver();
 float TireRadiusCm=34.f;
 FVector ContactOffset[4]={FVector(85,-84,0),FVector(85,84,0),FVector(-85,-84,0),FVector(-85,84,0)};
 FString Notice;
 bool Initialize(ALunarGameMode* Game);
 bool BuildVisuals();
 UStaticMeshComponent* Shape(UStaticMesh* Mesh,FVector Position,FVector Size,FLinearColor Color,FQuat Rotation=FQuat::Identity);
 bool GroundAt(FVector Point,FHitResult& Hit) const;
 bool SampleGround(FVector Point,float Yaw,FGroundPose& Out) const;
 bool ClearHull(FVector Position,FQuat Rotation,float Padding=0) const;
 bool CanEnter() const;
 bool Enter();
 bool Exit(bool Force);
 void RestoreRider(const FVector* ExitPosition);
 void Drive(float Delta);
 void PoseWheels(const FGroundPose& Ground,float SignedDistanceCm);
 void PoseCamera(float Delta);
 void Halt(const FString& Reason);
};
