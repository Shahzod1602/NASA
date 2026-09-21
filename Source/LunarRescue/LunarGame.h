#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "LunarGame.generated.h"

UENUM(BlueprintType)
enum class ELunarStage : uint8 { Briefing, FindModule, RestorePower, CollectData, ReturnHome, Won, Lost };

UCLASS(Blueprintable)
class LUNARRESCUE_API ALunarCharacter : public ACharacter {
 GENERATED_BODY()
public:
 ALunarCharacter();
 virtual void BeginPlay() override;
 virtual void Tick(float Delta) override;
 virtual void Landed(const FHitResult& Hit) override;
 void StampFootstep();
 void BuildGloves();
 int32 FootprintCount=0;
 float StepTravel=0, SuitTime=0;
 bool LeftFoot=false;
 FVector LastStepPosition=FVector::ZeroVector;
 UPROPERTY() TObjectPtr<class UAudioComponent> BreathAudio;
 UPROPERTY() TObjectPtr<class UAudioComponent> StepAudio;
 UPROPERTY() TObjectPtr<class UMaterialInterface> BootMaterial;
 UPROPERTY() TArray<TObjectPtr<class USoundBase>> StepSounds;
 UPROPERTY() TArray<TObjectPtr<class UDecalComponent>> Footprints;
 UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> Gloves;
 TArray<FVector> GloveRest;
 UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> LandingGrains;
 TArray<FVector> GrainVelocity;
 float GrainLife=0;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 void Forward(float V); void Right(float V); void Turn(float V); void Look(float V);
 void Leap(); void Interact(); void StartMission(); void RestartMission(); void PauseMission();
 void ZoomVisor();
 void ReleaseInteract(); void ToggleRoute();
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> CarriedModule;
 FVector PausedVelocity=FVector::ZeroVector;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<class UCameraComponent> Camera;
};

UCLASS(Blueprintable)
class LUNARRESCUE_API ALunarGameMode : public AGameModeBase {
 GENERATED_BODY()
public:
 ALunarGameMode();
 virtual void BeginPlay() override;
 virtual void Tick(float Delta) override;
 UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Mission") float OxygenCapacity=300.f;
 UPROPERTY(BlueprintReadOnly, Category="Mission") float Oxygen=300.f;
 UPROPERTY(BlueprintReadOnly, Category="Mission") ELunarStage Stage=ELunarStage::Briefing;
 UPROPERTY(BlueprintReadOnly, Category="Mission") bool Paused=false;
 UPROPERTY(BlueprintReadOnly, Category="Mission") FString Radio;
 UPROPERTY(BlueprintReadOnly, Category="Mission") float RadioRemaining=0;
 UPROPERTY(BlueprintReadOnly, Category="Mission") FString FailureReason;
 void AdvanceRadio(float Delta);
 void UpdateAudioPause();
 class UAudioComponent* PlayPowerSound(const TCHAR* Name,float Volume);
 UPROPERTY() TArray<TObjectPtr<class UAudioComponent>> PowerSounds;
 UPROPERTY() TObjectPtr<AActor> ModuleActor;
 UPROPERTY() TObjectPtr<AActor> PowerActor;
 UPROPERTY() TObjectPtr<AActor> DataActor;
 UPROPERTY() TObjectPtr<AActor> HomeActor;
 UPROPERTY() TObjectPtr<class UAudioComponent> RadioAudio;
 UPROPERTY() TObjectPtr<class UAudioComponent> PowerAudio;
 UPROPERTY() TObjectPtr<AActor> PowerAntenna;
 UPROPERTY() TObjectPtr<AActor> PowerScreen;
 UPROPERTY() TObjectPtr<AActor> PowerWarningBulb;
 UPROPERTY() TObjectPtr<class UPointLightComponent> PowerWarningLight;
 UPROPERTY() TArray<TObjectPtr<AActor>> PowerBulbs;
 UPROPERTY() TArray<TObjectPtr<class UPointLightComponent>> PowerLights;
 FRotator AntennaStartRotation=FRotator::ZeroRotator;
 float PowerSequenceSeconds=0;
 bool PowerSequenceActive=false;
 bool StationRebootComplete=false;
 void BeginPowerSequence();
 void AdvancePowerSequence(float Delta);
 void PowerReview();
 void StoryReview();
 void LedgeCheck();
 void WalkthroughTick(float Delta);
 bool WalkthroughEnabled=false;
 int32 WalkStep=0;
 float WalkSeconds=0, WalkStepSeconds=0;
 double WalkLastFrame=0;
 TArray<float> WalkFrameTimes;
 FString WalkReport;
 void BuildStoryProps();
 void ReleaseInteract();
 void ToggleRoute();
 void AdvanceInteraction(float Delta);
 float HoldDuration() const;
 int32 RepairStep=0;
 bool Holding=false, TrackRecorder=false, RecorderRecovered=false, LowOxygenWarned=false;
 float HoldProgress=0;
 UPROPERTY() TObjectPtr<AActor> RecorderActor;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> InstalledModule;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> PowerCable;
 UFUNCTION(BlueprintCallable) void StartMission();
 UFUNCTION(BlueprintCallable) void Interact();
 UFUNCTION(BlueprintCallable) void AdvanceTime(float Delta);
 UFUNCTION(BlueprintPure) bool IsActive() const;
 UFUNCTION(BlueprintPure) AActor* Target() const;
 UFUNCTION(BlueprintPure) FString Objective() const;
 UFUNCTION(BlueprintPure) FString Prompt() const;
 bool CanInteract() const;
 bool CanInteractWith(AActor* Actor) const;
 AActor* InteractionTarget() const;
 void Message(const FString& Text);
 void RunChecks();
 void MovementCheck();
 void ReviewVisuals();
 int32 MovementPhase=0;
 FVector MovementOrigin=FVector::ZeroVector;
 FString MovementReport;
 int32 MovementFailures=0;
};

UCLASS()
class LUNARRESCUE_API ALunarHUD : public AHUD {
 GENERATED_BODY()
public:
 virtual void DrawHUD() override;
 void Label(const FString& Text,float X,float Y,float Size,FLinearColor Color);
 void Panel(float X,float Y,float W,float H,float Alpha=0.85f);
};
