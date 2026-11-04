#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LunarFieldOperations.generated.h"
class ALunarGameMode; class ALunarCharacter; class ALunarSurfaceVehicle; class ULunarOperationsPanel; class UStaticMeshComponent;

enum class EFieldPanelView : uint8 { Preflight, Briefing, Resources, Vehicle, Wire, Inventory, Sample };
struct FFieldMapPoint {FString Label;FVector Position=FVector::ZeroVector;bool Completed=false,Current=false;int32 Index=0;};

// Independent modern training extension. All costs are authored gameplay units;
// no in-situ water detection, real EVA procedure or historical rover deployment.
UCLASS()
class LUNARRESCUE_API UFieldOperationsComponent : public UActorComponent {
 GENERATED_BODY()
public:
 UFieldOperationsComponent();
 UPROPERTY() TObjectPtr<ALunarSurfaceVehicle> Vehicle;
 UPROPERTY() TArray<TObjectPtr<AActor>> SupportActors;
 UPROPERTY() TObjectPtr<AActor> ResourceBoard;
 UPROPERTY() TObjectPtr<AActor> SampleRobot;
 UPROPERTY() TArray<TObjectPtr<AActor>> SamplePackages;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> ScannerVisuals;
 bool Enabled=false,Preparing=false,Prepared=false,Briefed=false,VehicleInspected=false,ScannerUnlocked=false;
 bool WireIsolated=false,WireComplete=false,WirePendingCommit=false,ScannerActive=false,Scanning=false;
 int32 Difficulty=1,PreflightMask=0,WireMask=0,WireLeft=INDEX_NONE,WireMistakes=0;
 int32 CollectedMask=0,DeliveredMask=0,ScannedMask=0,SelectedPackage=INDEX_NONE,LastPackage=INDEX_NONE;
 int32 Revision=0,PendingCheck=INDEX_NONE,WaypointIndex=INDEX_NONE,NativeScienceMask=0;
 float UtilityCharge=100,UtilityCapacity=100,CargoMassKG=0,DataStoredMB=0,DataTransmittedMB=0,DataCapacityMB=4;
 float ElapsedS=0,VehicleDistanceM=0,VehicleTimeS=0,WalkingDistanceM=0;
 float ActionRemaining=0,ActionDuration=1.0f,ScanProgress=0,ScanDuration=1.2f,PackagePreviewSeconds=0;
 float BaseMissionCapacity=0;
 UPROPERTY(EditAnywhere,Category="Gameplay model") float IdleChargePerSecond=.015f;
 UPROPERTY(EditAnywhere,Category="Gameplay model") float DriveChargePerMeter=.012f;
 UPROPERTY(EditAnywhere,Category="Gameplay model") float TrainingPackageMassKG=.5f;
 FString Feedback;
 FVector PreviousWalkPosition=FVector::ZeroVector;
 bool HasWalkPosition=false;
 FTransform VehicleHomePose=FTransform::Identity;bool HasVehicleHomePose=false;
 ALunarGameMode* Game() const;
 ALunarCharacter* Pilot() const;
 void Initialize(ALunarGameMode* Owner);
 void BuildWorld();
 void ResetNewRun(bool PreservePrepared=false);
 bool BeginPreflight();
 bool GateMissionStart();
 bool PreflightRequired() const;
 void OnMissionCommitted();
 void OnPanelClosed();
 void Advance(float Delta);
 void AdvanceScanner(float Delta);
 float MaxCargoKg() const;
 void Shutdown();
 bool OpenPanel(EFieldPanelView View);
 bool CanUsePanel(const ULunarOperationsPanel* Panel,bool Mutating=true) const;
 bool CanOpenView(EFieldPanelView View) const;
 bool PerformAction(FName Action,int32 Argument=0);
 bool BeginCheck(int32 Index);
 int32 PreflightStep() const;
 float ActionProgress() const;
 int32 WirePairs() const;
 FString WireLeftLabel(int32 Index) const;
 FString WireRightLabel(int32 Index) const;
 bool ConnectWire(int32 Right);
 bool ReadyForVehicle() const;
 bool SpendCharge(float Cost);
 void RecordVehicleTravel(float Meters,float Seconds);
 bool Riding() const;
 bool TryInteract();
 FString Prompt() const;
 bool ToggleScanner();
 bool BeginScan();
 void EndScan();
 void CancelScanner();
 int32 NearbyPackage(bool RequireObserved=false) const;
 void RefreshWorldVisibility();
 FString Title(EFieldPanelView View) const;
 FString Instructions(EFieldPanelView View) const;
 TArray<FString> ResourceLines() const;
 TArray<FString> InventoryLines() const;
 FString SourcesText() const;
 FString Summary() const;
 FString HudHint() const;
 TArray<FFieldMapPoint> MapPoints() const;
 FString ExportState() const;
 bool ImportState(const FString& Json);
 bool ValidateState(FString& Reason) const;
 static bool IsNewAutomation();
 void ValidateChecks(TFunctionRef<void(bool,const TCHAR*)> Check);
};