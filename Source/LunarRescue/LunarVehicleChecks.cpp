#include "LunarSurfaceVehicle.h"
#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarPanels.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include <limits>

namespace LunarVehicleCheckPrivate {
AActor* Wall(UWorld* World,FVector P,FQuat R,FVector Extent){
 auto* A=World?World->SpawnActor<AActor>():nullptr;if(!A)return nullptr;
 auto* Box=NewObject<UBoxComponent>(A);A->SetRootComponent(Box);Box->SetMobility(EComponentMobility::Movable);Box->SetBoxExtent(Extent);Box->SetCollisionProfileName(TEXT("BlockAll"));Box->SetCollisionObjectType(ECC_WorldStatic);Box->RegisterComponent();A->SetActorLocationAndRotation(P,R);return A;
}
}
void ALunarSurfaceVehicle::ValidateChecks(ALunarGameMode* G,TFunctionRef<void(bool,const TCHAR*)> Check){
 auto* C=G?G->FieldOperations.Get():nullptr;auto* P=G?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0)):nullptr;auto* PC=P?Cast<APlayerController>(P->GetController()):nullptr;
 Check(G&&C&&P&&PC&&UFieldOperationsComponent::IsNewAutomation(),TEXT("Vehicle checks require isolated field-operations automation and a possessed pawn"));
 if(!G||!C||!P||!PC||!UFieldOperationsComponent::IsNewAutomation())return;
 if(IsRiding(G)){Check(false,TEXT("Vehicle tests begin on foot without disturbing an existing driver"));return;}
 const FTransform PawnBefore=P->GetActorTransform();const FRotator ControlBefore=PC->GetControlRotation();const bool HiddenBefore=P->IsHidden(),CollisionBefore=P->GetActorEnableCollision(),ThirdBefore=P->ThirdPerson;const float FovBefore=P->Camera->FieldOfView;
 const auto ModeBefore=P->GetCharacterMovement()->MovementMode;const bool TickBefore=P->GetCharacterMovement()->IsComponentTickEnabled();const float Oxygen=G->Oxygen;const uint8 Samples=G->SamplesCollected,Parts=G->ApolloParts;
 TGuardValue<bool> Enabled(C->Enabled,true),Preparing(C->Preparing,false),Prepared(C->Prepared,true),Briefed(C->Briefed,true),Inspected(C->VehicleInspected,false),Scanner(C->ScannerUnlocked,false),Paused(G->Paused,false),Menu(G->Menu,false);
 TGuardValue<ELunarStage> Stage(G->Stage,G->Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule);
 // Keep the isolated battery internally valid even after a Professional (80 RU) briefing.
 TGuardValue<float> Capacity(C->UtilityCapacity,100),Charge(C->UtilityCharge,100),Meters(C->VehicleDistanceM,C->VehicleDistanceM),Seconds(C->VehicleTimeS,C->VehicleTimeS);
 TGuardValue<int32> Tool(G->ApolloTool,0);TGuardValue<bool> Hold(G->Holding,false),PendingPhoto(G->ApolloPhotoPending,false);
 TGuardValue<TObjectPtr<ULunarPanel>> Panel(P->ActivePanel,nullptr);
 auto* V=Build(G);Check(V!=nullptr,TEXT("A bounded training rover finds real supported terrain near the mission base"));if(!V)return;
 const FTransform Parked=V->GetActorTransform();Check(Build(G)==V,TEXT("Repeated vehicle construction reuses one existing rover"));
 Check(V->Visuals.Num()<=MaxVisualComponents&&V->Wheels.Num()==4&&(V->MuseumRover?V->Hubs.IsEmpty():V->Hubs.Num()==4)&&!V->IsActorTickEnabled(),TEXT("Parked rover has four proper wheels and bounded visuals with no idle Tick"));
 Check(G->Mars?!V->MuseumRover:V->MuseumRover&&FMath::IsNearlyEqual(V->TireRadiusCm,35.816f,.01f),TEXT("Moon loads the credited museum rover with measured source-wheel pivots; Mars retains its own vehicle"));
 bool BodyContained=!V->Visuals.IsEmpty()&&V->Visuals[0]&&V->Visuals[0]->GetStaticMesh();
 if(BodyContained){const FBox Bounds=V->Visuals[0]->GetStaticMesh()->GetBoundingBox();const FVector Extent=V->Hull->GetUnscaledBoxExtent();for(int32 Corner=0;Corner<8;++Corner){const FVector Local=V->Visuals[0]->GetRelativeTransform().TransformPosition(FVector(Corner&1?Bounds.Max.X:Bounds.Min.X,Corner&2?Bounds.Max.Y:Bounds.Min.Y,Corner&4?Bounds.Max.Z:Bounds.Min.Z));BodyContained&=FMath::Abs(Local.X)<=Extent.X+.1&&FMath::Abs(Local.Y)<=Extent.Y+.1&&FMath::Abs(Local.Z)<=Extent.Z+.1;}}
 Check(BodyContained,TEXT("The rendered rover body fits inside its swept collision hull using actual imported mesh bounds"));
 Check(!V->RestoreParkedPose(FTransform(FVector(1.e9,1.e9,1.e9)))&&V->GetActorTransform().Equals(Parked),TEXT("Missing or out-of-map ground rejects parked state without moving the rover"));
 const float NaN=std::numeric_limits<float>::quiet_NaN();
 V->SetForward(NaN);V->SetSteer(std::numeric_limits<float>::infinity());
 Check(V->Throttle==0&&V->Steering==0&&!V->RestoreParkedPose(FTransform(Parked.GetRotation(),Parked.GetLocation(),FVector(2))),TEXT("Nonfinite controls are neutralized and rescaled parked transforms are rejected"));
 Check(V->RestoreParkedPose(Parked),TEXT("A validated parked transform restores the unoccupied rover without moving the pawn"));
 Check(P->GetActorTransform().Equals(PawnBefore),TEXT("Restoring rover parking never teleports the player"));
 FVector Entry;if(!V->GroundedExitPose(Entry)){Check(false,TEXT("A real terrain-tested on-foot approach exists beside the rover"));return;}
 P->SetActorLocation(Entry,false,nullptr,ETeleportType::TeleportPhysics);
 FGroundPose PawnGround;const bool PawnSupported=V->SampleGround(Entry,Parked.Rotator().Yaw,PawnGround);
 Check(PawnSupported,TEXT("Parking-overlap fixture uses a supported standing capsule on actual terrain"));
 if(PawnSupported){
  // A successful grounded restore may resample its exact pose; rejection must
  // preserve the immediately preceding live transforms, not the older spawn.
  const FTransform BeforeRejectedPark=V->GetActorTransform(),PawnBeforeRejectedPark=P->GetActorTransform();
  const bool RejectedPawnOverlap=!V->RestoreParkedPose(FTransform(FRotationMatrix::MakeFromZX(PawnGround.Normal,FRotator(0,Parked.Rotator().Yaw,0).Vector()).ToQuat(),PawnGround.Centre));
  const bool NoParkingMutation=V->GetActorTransform().Equals(BeforeRejectedPark,0)&&P->GetActorTransform().Equals(PawnBeforeRejectedPark,0);
  if(!RejectedPawnOverlap||!NoParkingMutation)UE_LOG(LogTemp,Warning,TEXT("VEHICLE_PARK_CHECK rejected=%d unchanged=%d pawn_collision=%d capsule_query=%d capsule_vehicle_response=%d rover_before=%s rover_after=%s pawn_before=%s pawn_after=%s"),RejectedPawnOverlap,NoParkingMutation,P->GetActorEnableCollision(),P->GetCapsuleComponent()->IsQueryCollisionEnabled(),int32(P->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Vehicle)),*BeforeRejectedPark.ToHumanReadableString(),*V->GetActorTransform().ToHumanReadableString(),*PawnBeforeRejectedPark.ToHumanReadableString(),*P->GetActorTransform().ToHumanReadableString());
  Check(RejectedPawnOverlap&&NoParkingMutation,TEXT("A terrain-valid parked pose overlapping the on-foot player is rejected without moving either actor"));
 }
 Check(!V->CanEnter(),TEXT("Uninspected training rover cannot be driven"));
 PC->SetControlRotation((V->GetActorLocation()-P->Camera->GetComponentLocation()).Rotation());const bool WasScannerUnlocked=C->ScannerUnlocked;C->UtilityCharge=0;G->Interact();Check(!IsRiding(G)&&!C->VehicleInspected&&C->ScannerUnlocked==WasScannerUnlocked&&!P->ActivePanel,TEXT("Depleted-charge work-key boarding preserves readiness and never leaves a Vehicle Menu open"));C->UtilityCharge=100;const FString RadioBeforeBoarding=G->Radio;G->Interact();
 Check(C->VehicleInspected&&C->ScannerUnlocked&&!P->ActivePanel&&G->Radio==RadioBeforeBoarding,TEXT("Actual work-key boarding enters directly without Vehicle Menu or an extra instruction radio panel"));
 Check(IsRiding(G)&&PC->GetPawn()==P&&PC->GetViewTarget()==V&&P->IsHidden()&&P->GetCharacterMovement()->MovementMode==MOVE_None,TEXT("Mounting keeps the existing possessed pawn in a hidden fixed seat and changes only the view target"));
 if(V->IsOccupied()){
  Check(V->Driver&&V->Driver->IsVisible()&&V->Driver->GetSkinnedAsset()&&!V->Driver->IsComponentTickEnabled(),TEXT("Occupied rover renders a seated skinned astronaut without a second possessed pawn or visual Tick"));
  if(V->Driver){const FVector Hip=V->Driver->GetBoneLocationByName(TEXT("pelvis"),EBoneSpaces::WorldSpace),Knee=V->Driver->GetBoneLocationByName(TEXT("calf_l"),EBoneSpaces::WorldSpace);const FVector A=V->GetActorTransform().InverseTransformPosition(Hip),B=V->GetActorTransform().InverseTransformPosition(Knee);Check(B.X>A.X+25&&FMath::Abs(B.Z-A.Z)<20,TEXT("Rendered driver thighs extend forward in a seated pose rather than standing through the seat"));}
  const FTransform OrbitStart=V->FollowCamera->GetComponentTransform();const float ResourceBeforeLook=C->UtilityCharge;
  P->Turn(60);V->PoseCamera(0);Check(!V->FollowCamera->GetComponentTransform().Equals(OrbitStart)&&PC->GetViewTarget()==V&&C->UtilityCharge==ResourceBeforeLook,TEXT("Real mouse-yaw handler orbits the rover without changing drive state or charging RU"));
  P->ToggleSuitView();Check(V->CockpitView&&P->ThirdPerson==ThirdBefore&&PC->GetPawn()==P,TEXT("Real V handler switches to cockpit eyes while retaining the on-foot view preference and possession"));
  Check(V->Driver&&V->Driver->GetBoneTransformByName(TEXT("head"),EBoneSpaces::ComponentSpace).GetScale3D().IsNearlyZero(.001f),TEXT("Actual cockpit driver helmet is collapsed outside the eye view rather than occluding the camera"));
  const FTransform Cockpit=V->FollowCamera->GetComponentTransform();P->Look(12);V->PoseCamera(0);Check(!V->FollowCamera->GetComponentTransform().Equals(Cockpit),TEXT("Cockpit mouse pitch looks around from fixed driver eye height"));
  const FTransform FrozenLook=V->FollowCamera->GetComponentTransform();G->Paused=true;P->Turn(20);P->Look(20);P->ToggleSuitView();V->PoseCamera(0);Check(V->CockpitView&&V->FollowCamera->GetComponentTransform().Equals(FrozenLook),TEXT("Pause rejects rover mouse and V input without camera drift"));G->Paused=false;
  P->ToggleSuitView();Check(V->Driver&&V->Driver->GetBoneTransformByName(TEXT("head"),EBoneSpaces::ComponentSpace).GetScale3D().Equals(FVector::OneVector,.001f),TEXT("Returning to follow view restores the rendered driver helmet"));V->ViewYaw=0;V->ViewPitch=-20;V->PoseCamera(0);
  Check(!V->RestoreParkedPose(Parked),TEXT("Parked-state restore cannot relocate an occupied rover"));
  const FVector SeatBefore=P->GetRootComponent()->GetRelativeLocation();
  const float BeforeMeters=C->VehicleDistanceM,BeforeCharge=C->UtilityCharge,Phase=V->WheelPhase;const FVector Start=V->GetActorLocation();const FVector Forward=V->GetActorForwardVector();
  auto* Barrier=LunarVehicleCheckPrivate::Wall(G->GetWorld(),Start+Forward*220,V->GetActorQuat(),FVector(14,250,180));
  Check(Barrier!=nullptr,TEXT("A real blocking-wall fixture exists for swept driving"));
  const FTransform BodyPose=V->Visuals[0]->GetRelativeTransform();
  V->SetForward(1);for(int32 I=0;I<20;++I)V->Tick(.1f);
  const float Travel=C->VehicleDistanceM-BeforeMeters;const float Along=FVector::DotProduct(V->GetActorLocation()-Start,Forward);
  Check(Travel>0&&Along>0&&Along<90,TEXT("Rover moves continuously on terrain and stops before a real blocking wall"));
  const bool ExactDriveCost=FMath::IsNearlyEqual(BeforeCharge-C->UtilityCharge,Travel*C->DriveChargePerMeter,.0005f),WheelsMoved=V->WheelPhase!=Phase;
  if(!ExactDriveCost||!WheelsMoved)UE_LOG(LogTemp,Warning,TEXT("VEHICLE_DRIVE_CHECK capacity=%.9g before_charge=%.9g after_charge=%.9g travel_m=%.9g coefficient=%.9g expected_cost=%.9g actual_cost=%.9g wheel_before=%.9g wheel_after=%.9g"),C->UtilityCapacity,BeforeCharge,C->UtilityCharge,Travel,C->DriveChargePerMeter,Travel*C->DriveChargePerMeter,BeforeCharge-C->UtilityCharge,Phase,V->WheelPhase);
  Check(ExactDriveCost&&WheelsMoved,TEXT("Only actual swept travel consumes shared RU and rotates the wheels"));
  Check(V->Visuals[0]->GetRelativeTransform().Equals(BodyPose)&&V->WheelPhase!=Phase,TEXT("Stationary chassis and mudguards retain their local pose while tire phase advances"));
  Check(P->GetAttachParentActor()==V&&P->GetRootComponent()->GetRelativeLocation().Equals(SeatBefore,.01f),TEXT("The original pawn remains at the moving seat without controller possession changes"));
  if(Barrier)Barrier->Destroy();
  const FTransform Frozen=V->GetActorTransform(),Camera=V->FollowCamera->GetComponentTransform();const float FrozenCharge=C->UtilityCharge,FrozenMeters=C->VehicleDistanceM;
  G->Paused=true;V->SetForward(1);V->Tick(1);
  Check(V->GetActorTransform().Equals(Frozen)&&V->FollowCamera->GetComponentTransform().Equals(Camera)&&C->UtilityCharge==FrozenCharge&&C->VehicleDistanceM==FrozenMeters,TEXT("Pause freezes rover position camera resources and driving telemetry"));G->Paused=false;
  P->ActivePanel=NewObject<ULunarPanel>(P);V->Tick(.5f);
  Check(V->GetActorTransform().Equals(Frozen)&&C->UtilityCharge==FrozenCharge,TEXT("An open notebook cannot keep the rover driving in the background"));P->ActivePanel=nullptr;
  C->UtilityCharge=0;V->SetForward(1);V->Tick(.1f);
  Check(V->GetActorTransform().Equals(Frozen)&&V->GetSpeedMps()==0&&C->UtilityCharge==0,TEXT("Zero charge stops the rover with no negative resource or free travel"));
  auto* Cage=LunarVehicleCheckPrivate::Wall(G->GetWorld(),V->GetActorLocation(),FQuat::Identity,FVector(420,420,240));
  Check(Cage&&!V->Exit(false)&&V->IsOccupied()&&PC->GetViewTarget()==V,TEXT("Blocked capsule exits keep the rider inside instead of placing them through a wall"));
  if(Cage)Cage->Destroy();
  Check(V->Exit(false)&&!IsRiding(G)&&PC->GetPawn()==P&&PC->GetViewTarget()==P&&P->IsHidden()==HiddenBefore&&P->GetActorEnableCollision()==CollisionBefore,TEXT("The driver can exit on foot at zero charge with original ownership visibility and collision restored"));
  Check(!V->Driver||!V->Driver->IsVisible(),TEXT("Dismount hides the seated visual without leaking an extra astronaut"));
  Check(P->ThirdPerson==ThirdBefore&&FMath::IsNearlyEqual(P->Camera->FieldOfView,FovBefore)&&P->GetCharacterMovement()->MovementMode==ModeBefore,TEXT("Dismount restores previous view mode field of view and movement mode"));
  if(G->Apollo){
   const float NativeDistance=G->ApolloDistance,NativeElapsed=G->ApolloElapsed,NativeOxygen=G->Oxygen;
   G->TickApollo(0);
   Check(G->ApolloDistance==NativeDistance&&G->ApolloElapsed==NativeElapsed&&G->Oxygen==NativeOxygen&&G->ApolloLastPosition.Equals(P->GetActorLocation(),.01f)&&G->ApolloTrailPendingBreak,TEXT("A real rover dismount is not credited as native walking or connected to the historical trail and a zero-delta tick changes no elapsed time or oxygen"));
  }
 }
 C->UtilityCharge=100;V->RestoreParkedPose(Parked);
 const bool CleanupApproach=V->GroundedExitPose(Entry);Check(CleanupApproach,TEXT("EndPlay fixture has a validated approach beside the parked rover"));
 if(CleanupApproach){P->SetActorLocation(Entry,false,nullptr,ETeleportType::TeleportPhysics);const bool CleanupMounted=V->Enter();Check(CleanupMounted,TEXT("EndPlay fixture starts with an actual mounted rider"));if(CleanupMounted){V->Destroy();Check(!IsRiding(G)&&P->IsHidden()==HiddenBefore&&PC->GetViewTarget()==P&&PC->GetPawn()==P,TEXT("Vehicle EndPlay restores the original pawn and view without a hidden-rider leak"));}}
 Abort(G);
 auto* Rebuilt=Build(G);if(Rebuilt)Rebuilt->RestoreParkedPose(Parked);
 P->SetActorLocationAndRotation(PawnBefore.GetLocation(),PawnBefore.GetRotation(),false,nullptr,ETeleportType::TeleportPhysics);PC->SetControlRotation(ControlBefore);P->LastStepPosition=P->GetActorLocation();P->GetCharacterMovement()->SetComponentTickEnabled(TickBefore);
 Check(G->Oxygen==Oxygen&&G->SamplesCollected==Samples&&G->ApolloParts==Parts,TEXT("Driving checks never award primary science cargo or alter the legacy oxygen model"));
}
