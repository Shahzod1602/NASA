#include "LunarSurfaceVehicle.h"
#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarDeparture.h"
#include "LunarCargoFinale.h"
#include "LunarLoading.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"

namespace LunarSurfaceVehiclePrivate {
constexpr float RideHeight=92,WheelRadius=34,StepLimit=35;
const FVector HullExtent(130,93,78),Seat(20,0,35);
const FVector WheelOffset[4]={FVector(85,-84,0),FVector(85,84,0),FVector(-85,-84,0),FVector(-85,84,0)};
FQuat GroundRotation(FVector Normal,float Yaw){return FRotationMatrix::MakeFromZX(Normal,FRotator(0,Yaw,0).Vector()).ToQuat();}
ALunarCharacter* Pilot(const ALunarGameMode* G){return G?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0)):nullptr;}
}
ALunarSurfaceVehicle::ALunarSurfaceVehicle(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.bStartWithTickEnabled=false;
 Hull=CreateDefaultSubobject<UBoxComponent>(TEXT("TrainingVehicleHull"));SetRootComponent(Hull);Hull->SetBoxExtent(LunarSurfaceVehiclePrivate::HullExtent);
 Hull->SetMobility(EComponentMobility::Movable);Hull->SetCollisionProfileName(TEXT("BlockAllDynamic"));Hull->SetCollisionObjectType(ECC_Vehicle);Hull->SetGenerateOverlapEvents(false);Hull->SetCanEverAffectNavigation(false);
 FollowCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("SurfaceFollowCamera"));FollowCamera->SetupAttachment(Hull);FollowCamera->bUsePawnControlRotation=false;FollowCamera->SetRelativeLocation(FVector(-450,85,230));FollowCamera->SetFieldOfView(85);
 Tags.Add(TEXT("Lunar.TrainingVehicle"));
}
ALunarSurfaceVehicle* ALunarSurfaceVehicle::Build(ALunarGameMode* G){
 if(!G||G->Menu||!G->GetWorld()||!G->FieldOperations||!G->FieldOperations->Enabled)return nullptr;
 auto* C=G->FieldOperations.Get();if(IsValid(C->Vehicle)&&C->Vehicle->GetWorld()==G->GetWorld())return C->Vehicle;
 auto* V=G->GetWorld()->SpawnActor<ALunarSurfaceVehicle>();if(!V)return nullptr;
 V->SetOwner(G);if(!V->Initialize(G)){V->Destroy();return nullptr;}C->Vehicle=V;return V;
}
bool ALunarSurfaceVehicle::Initialize(ALunarGameMode* G){
 Mission=G;Operations=G?G->FieldOperations.Get():nullptr;if(!Mission||!Operations||!BuildVisuals())return false;
 auto* P=LunarSurfaceVehiclePrivate::Pilot(G);if(!P)return false;
 const FVector Anchor=IsValid(G->HomeActor)?G->HomeActor->GetActorLocation():P->GetActorLocation();const float BaseYaw=P->GetActorRotation().Yaw;
 const float FirstRadius=G->Apollo?1800.f:1100.f;
 for(float Radius:{FirstRadius,FirstRadius+350.f,FirstRadius+700.f})for(int32 Side=0;Side<8;++Side){
  const float A=FMath::DegreesToRadians(BaseYaw+Side*45.f);FGroundPose Ground;
  if(!SampleGround(Anchor+FVector(FMath::Cos(A)*Radius,FMath::Sin(A)*Radius,0),BaseYaw,Ground))continue;
  const FQuat Rotation=LunarSurfaceVehiclePrivate::GroundRotation(Ground.Normal,BaseYaw);
  if(!ClearHull(Ground.Centre,Rotation))continue;
  SetActorLocationAndRotation(Ground.Centre,Rotation,false,nullptr,ETeleportType::TeleportPhysics);PoseWheels(Ground,0);PoseCamera(0);AddTickPrerequisiteActor(G);return true;
 }
 return false;
}
UStaticMeshComponent* ALunarSurfaceVehicle::Shape(UStaticMesh* Mesh,FVector P,FVector Size,FLinearColor Color,FQuat Rotation){
 if(!Mesh||Visuals.Num()>=MaxVisualComponents)return nullptr;
 auto* C=NewObject<UStaticMeshComponent>(this);C->SetupAttachment(Hull);C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Mesh);
 C->SetRelativeLocation(P);C->SetRelativeScale3D(Size/100);C->SetRelativeRotation(Rotation);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetGenerateOverlapEvents(false);C->PrimaryComponentTick.bCanEverTick=false;
 if(auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))){auto* M=UMaterialInstanceDynamic::Create(Base,C);M->SetVectorParameterValue(TEXT("Color"),Color);C->SetMaterial(0,M);}
 C->RegisterComponent();Visuals.Add(C);return C;
}
bool ALunarSurfaceVehicle::BuildVisuals(){
 auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));auto* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));if(!Cube||!Cylinder)return false;
 const FLinearColor Paint(.64,.68,.65),Metal(.18,.23,.26),Dark(.035,.045,.05),Gold(.67,.42,.12);
 Shape(Cube,FVector::ZeroVector,FVector(260,154,54),Paint);Shape(Cube,FVector(0,0,31),FVector(225,145,8),Metal);
 Shape(Cube,FVector(80,0,43),FVector(95,145,35),Paint);Shape(Cube,FVector(-25,0,42),FVector(65,65,12),Dark);Shape(Cube,FVector(-60,0,68),FVector(15,65,65),Dark);Shape(Cube,FVector(44,0,70),FVector(16,130,50),Gold);
 for(int32 I=0;I<4;++I){const FVector P=LunarSurfaceVehiclePrivate::WheelOffset[I]+FVector(0,0,-58);auto* Wheel=Shape(Cylinder,P,FVector(68,68,20),Dark,FRotator(0,0,90).Quaternion());auto* Hub=Shape(Cylinder,P+FVector(0,I%2?11:-11,0),FVector(28,28,22),Metal,FRotator(0,0,90).Quaternion());Wheels.Add(Wheel);Hubs.Add(Hub);}
 for(float Side:{-1.f,1.f})Shape(Cube,FVector(-22,Side*62,75),FVector(130,6,6),Metal);
 return Wheels.Num()==4&&Hubs.Num()==4&&Visuals.Num()<=MaxVisualComponents;
}
bool ALunarSurfaceVehicle::GroundAt(FVector Point,FHitResult& Hit) const {
 if(!GetWorld()||Point.ContainsNaN())return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(TrainingRoverGround),true);Q.AddIgnoredActor(this);if(IsValid(Rider))Q.AddIgnoredActor(Rider);if(auto* P=LunarSurfaceVehiclePrivate::Pilot(Mission))Q.AddIgnoredActor(P);
 if(!GetWorld()->LineTraceSingleByChannel(Hit,Point+FVector(0,0,1600),Point-FVector(0,0,3200),ECC_Visibility,Q))return false;
 const auto* C=Cast<UStaticMeshComponent>(Hit.GetComponent());
 return C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName().Contains(TEXT("Terrain"))&&!Hit.ImpactPoint.ContainsNaN()&&!Hit.ImpactNormal.ContainsNaN()&&Hit.ImpactNormal.Z>0;
}
bool ALunarSurfaceVehicle::SampleGround(FVector Point,float Yaw,FGroundPose& Out) const {
 if(Point.ContainsNaN()||!FMath::IsFinite(Yaw)||!FMath::IsFinite(SlopeLimitDegrees)||SlopeLimitDegrees<=0||SlopeLimitDegrees>30)return false;
 FHitResult Centre;if(!GroundAt(Point,Centre))return false;FVector Normal=Centre.ImpactNormal;
 const float MinZ=FMath::Cos(FMath::DegreesToRadians(SlopeLimitDegrees));if(Normal.Z<MinZ)return false;
 const FQuat Heading=FRotator(0,Yaw,0).Quaternion();double MinHeight=Centre.ImpactPoint.Z,MaxHeight=MinHeight;
 for(int32 I=0;I<4;++I){FHitResult Hit;if(!GroundAt(Point+Heading.RotateVector(LunarSurfaceVehiclePrivate::WheelOffset[I]),Hit)||Hit.ImpactNormal.Z<MinZ)return false;
  Out.Wheel[I]=Hit.ImpactPoint;Normal+=Hit.ImpactNormal;MinHeight=FMath::Min(MinHeight,Hit.ImpactPoint.Z);MaxHeight=FMath::Max(MaxHeight,Hit.ImpactPoint.Z);
 }
 if(MaxHeight-MinHeight>90||!Normal.Normalize()||Normal.Z<MinZ)return false;
 for(const FVector& W:Out.Wheel){const double Expected=Centre.ImpactPoint.Z-(Normal.X*(W.X-Centre.ImpactPoint.X)+Normal.Y*(W.Y-Centre.ImpactPoint.Y))/Normal.Z;if(FMath::Abs(W.Z-Expected)>18)return false;}
 Out.Normal=Normal;Out.Centre=FVector(Point.X,Point.Y,Centre.ImpactPoint.Z+LunarSurfaceVehiclePrivate::RideHeight/Normal.Z);return !Out.Centre.ContainsNaN();
}
bool ALunarSurfaceVehicle::ClearHull(FVector P,FQuat R,float Padding) const {
 if(!GetWorld()||P.ContainsNaN()||R.ContainsNaN()||!FMath::IsFinite(Padding)||Padding<0)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(TrainingRoverHull),false);Q.AddIgnoredActor(this);if(IsValid(Rider))Q.AddIgnoredActor(Rider);if(auto* Pawn=LunarSurfaceVehiclePrivate::Pilot(Mission))Q.AddIgnoredActor(Pawn);
 return !GetWorld()->OverlapBlockingTestByChannel(P,R,ECC_Vehicle,FCollisionShape::MakeBox(LunarSurfaceVehiclePrivate::HullExtent+FVector(Padding,Padding,0)),Q);
}
bool ALunarSurfaceVehicle::IsOccupied() const {return Occupied&&IsValid(Rider);}
bool ALunarSurfaceVehicle::IsRiding(const ALunarGameMode* G){return G&&G->FieldOperations&&IsValid(G->FieldOperations->Vehicle)&&G->FieldOperations->Vehicle->IsOccupied();}
bool ALunarSurfaceVehicle::CanEnter() const {
 auto* P=LunarSurfaceVehiclePrivate::Pilot(Mission);
 if(!(IsValid(Mission)&&IsValid(Operations)&&Operations->ReadyForVehicle()&&P&&P->Camera&&!P->GetAttachParentActor()&&!P->ActivePanel&&!Mission->Holding&&!Mission->ApolloTool&&!Mission->ApolloPhotoPending&&!ALunarDepartureScene::Active(Mission)&&!ALunarCargoFinale::Active(Mission)&&!LunarLoading::IsBusy()&&FVector::Dist(P->GetActorLocation(),GetActorLocation())<=320))return false;
 const FVector Seat=GetActorTransform().TransformPosition(LunarSurfaceVehiclePrivate::Seat);const auto* Capsule=P->GetCapsuleComponent();const auto CapsuleShape=FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),Capsule->GetScaledCapsuleHalfHeight());
 FCollisionQueryParams Q(SCENE_QUERY_STAT(TrainingRoverMount),false);Q.AddIgnoredActor(this);Q.AddIgnoredActor(P);FHitResult Hit;
 return !GetWorld()->OverlapBlockingTestByChannel(Seat,FQuat::Identity,ECC_Pawn,CapsuleShape,Q)&&!GetWorld()->SweepSingleByChannel(Hit,P->GetActorLocation(),Seat,FQuat::Identity,ECC_Pawn,CapsuleShape,Q);
}
bool ALunarSurfaceVehicle::TryInteract(ALunarGameMode* G){
 if(!G||!G->FieldOperations||!IsValid(G->FieldOperations->Vehicle))return false;auto* V=G->FieldOperations->Vehicle.Get();
 if(V->IsOccupied()){V->Exit(false);return true;}if(!V->CanEnter())return false;
 if(!FMath::IsFinite(G->FieldOperations->UtilityCharge)||G->FieldOperations->UtilityCharge<=UE_SMALL_NUMBER){G->Message(TEXT("Rover charge is depleted. Continue on foot or return to the resource board."));return true;}
 V->Enter();return true;
}
bool ALunarSurfaceVehicle::Enter(){
 if(IsOccupied()||!CanEnter())return false;auto* P=LunarSurfaceVehiclePrivate::Pilot(Mission);auto* PC=Cast<APlayerController>(P->GetController());if(!PC||!P->GetCharacterMovement())return false;
 if(RecoveryPawn.IsValid()){RecoveryPawn->GetCapsuleComponent()->IgnoreActorWhenMoving(this,false);RecoveryPawn.Reset();}
 // Stow the magnifier before snapshotting the normal on-foot FOV.
 Operations->CancelScanner();
 Rider=P;PreviousViewTarget=PC->GetViewTarget();PreviousControl=PC->GetControlRotation();PreviousCamera=P->Camera->GetRelativeTransform();PreviousFov=P->Camera->FieldOfView;PreviousCameraControl=P->Camera->bUsePawnControlRotation;PreviousThird=P->ThirdPerson;
 auto* Move=P->GetCharacterMovement();PreviousMoveMode=uint8(Move->MovementMode);PreviousCustomMode=Move->CustomMovementMode;PreviousMoveTick=Move->IsComponentTickEnabled();PreviousVelocity=Move->Velocity;PreviousHidden=P->IsHidden();PreviousCollision=P->GetActorEnableCollision();
 Move->StopMovementImmediately();Move->DisableMovement();Move->SetComponentTickEnabled(false);P->SetActorEnableCollision(false);Hull->IgnoreActorWhenMoving(P,true);P->GetCapsuleComponent()->IgnoreActorWhenMoving(this,true);
 P->AttachToActor(this,FAttachmentTransformRules::KeepWorldTransform);P->SetActorRelativeLocation(LunarSurfaceVehiclePrivate::Seat);P->SetActorRelativeRotation(FRotator::ZeroRotator);P->SetActorHiddenInGame(true);
 Occupied=true;Throttle=Steering=SpeedCm=ActualSpeedMps=0;Notice.Empty();FollowCamera->SetFieldOfView(FMath::Clamp(PreviousFov,60.f,110.f));PoseCamera(0);PC->SetViewTarget(this);SetActorTickEnabled(true);
 Mission->Message(TEXT("TRAINING ROVER: movement keys drive and steer. {Interact}: stop and exit. This is a simplified fictional vehicle."));return true;
}
void ALunarSurfaceVehicle::SetForward(float V){Throttle=FMath::IsFinite(V)?FMath::Clamp(V,-1.f,1.f):0;}
void ALunarSurfaceVehicle::SetSteer(float V){Steering=FMath::IsFinite(V)?FMath::Clamp(V,-1.f,1.f):0;}
void ALunarSurfaceVehicle::Halt(const FString& Reason){SpeedCm=ActualSpeedMps=0;if(Notice!=Reason){Notice=Reason;if(IsValid(Mission)&&!Ending)Mission->Message(Reason);}}
void ALunarSurfaceVehicle::Drive(float D){
 const float Cost=Operations->DriveChargePerMeter,Charge=Operations->UtilityCharge;
 if(!FMath::IsFinite(Cost)||Cost<=0||!FMath::IsFinite(Charge)||Charge<=UE_SMALL_NUMBER){Halt(TEXT("Rover charge is depleted. {Interact}: exit and continue on foot."));return;}
 if(!FMath::IsFinite(ForwardLimitCm)||!FMath::IsFinite(ReverseLimitCm)||!FMath::IsFinite(AccelerationCm)||!FMath::IsFinite(BrakingCm)||!FMath::IsFinite(TurnRateDegrees)||AccelerationCm<=0||BrakingCm<=0){Halt(TEXT("Training rover settings are unavailable. Exit and continue on foot."));return;}
 const float Limit=Throttle<0?FMath::Clamp(ReverseLimitCm,0.f,350.f):FMath::Clamp(ForwardLimitCm,0.f,800.f);
 const float Wanted=Throttle*Limit;SpeedCm=FMath::FInterpConstantTo(SpeedCm,Wanted,D,FMath::Abs(Wanted)<FMath::Abs(SpeedCm)?BrakingCm:AccelerationCm);
 if(FMath::Abs(SpeedCm)<.1f){SpeedCm=ActualSpeedMps=0;return;}
 const FVector Before=GetActorLocation();const float OldYaw=GetActorRotation().Yaw;
 const float Turn=Steering*FMath::Clamp(TurnRateDegrees,0.f,90.f)*FMath::Clamp(FMath::Abs(SpeedCm)/140.f,0.f,1.f)*FMath::Sign(SpeedCm)*D;
 const float Yaw=FMath::UnwindDegrees(OldYaw+Turn);const FVector Heading=FRotator(0,Yaw,0).Vector();FGroundPose Ground;
 if(!SampleGround(Before+Heading*SpeedCm*D,Yaw,Ground)||FMath::Abs(Ground.Centre.Z-Before.Z)>LunarSurfaceVehiclePrivate::StepLimit){Halt(TEXT("Rover route blocked or ground unsupported. Choose flatter open terrain."));return;}
 float BudgetCm=FMath::Min(Charge/Cost*100.f,800.f*D);double Desired=FVector::Dist(Before,Ground.Centre);
 if(Desired>BudgetCm){const double Ratio=BudgetCm/FMath::Max(Desired,.001)*.95;if(!SampleGround(Before+Heading*SpeedCm*D*Ratio,Yaw,Ground)||FVector::Dist(Before,Ground.Centre)>BudgetCm){Halt(TEXT("Insufficient charge for this terrain step. Exit and continue on foot."));return;}}
 const FQuat Rotation=LunarSurfaceVehiclePrivate::GroundRotation(Ground.Normal,Yaw);
 const float TurnPadding=FMath::Abs(FMath::DegreesToRadians(Turn))*160.f;
 if(!ClearHull(Before,GetActorQuat())||(TurnPadding>.01f&&!ClearHull(Before,GetActorQuat(),TurnPadding))||!ClearHull(Ground.Centre,Rotation)){Halt(TEXT("Rover route blocked. Leave clearance around walls and equipment."));return;}
 FHitResult Hit;Hull->MoveComponent(Ground.Centre-Before,Rotation,true,&Hit);
 ActualSpeedMps=0;const FVector After=GetActorLocation();const float Meters=float(FVector::Dist(Before,After)*.01);
 if(FMath::IsFinite(Meters)&&Meters>UE_SMALL_NUMBER){ActualSpeedMps=Meters/D;TravelMeters+=Meters;Operations->RecordVehicleTravel(Meters,D);FGroundPose ActualGround;if(SampleGround(After,GetActorRotation().Yaw,ActualGround))PoseWheels(ActualGround,FVector::DotProduct(After-Before,Heading));Notice.Empty();}
 if(Hit.bBlockingHit||Hit.bStartPenetrating)Halt(TEXT("Rover stopped by an obstacle. Reverse or choose another route."));
 if(Operations->UtilityCharge<=UE_SMALL_NUMBER)SpeedCm=0;
}
void ALunarSurfaceVehicle::PoseWheels(const FGroundPose& Ground,float SignedCm){
 WheelPhase=FMath::Fmod(WheelPhase+SignedCm/LunarSurfaceVehiclePrivate::WheelRadius,2*PI);
 for(int32 I=0;I<4&&Wheels.IsValidIndex(I)&&Hubs.IsValidIndex(I);++I){if(!Wheels[I]||!Hubs[I])continue;
  const FQuat Local=FQuat(FVector::UpVector,I<2?FMath::DegreesToRadians(Steering*22):0)*FQuat(FVector::RightVector,WheelPhase)*FRotator(0,0,90).Quaternion();
  const FVector Centre=Ground.Wheel[I]+Ground.Normal*LunarSurfaceVehiclePrivate::WheelRadius;Wheels[I]->SetWorldLocation(Centre);Wheels[I]->SetRelativeRotation(Local);
  Hubs[I]->SetWorldLocation(Centre+GetActorRightVector()*(I%2?11.f:-11.f));Hubs[I]->SetRelativeRotation(Local);
 }
}
void ALunarSurfaceVehicle::PoseCamera(float D){
 const FVector Target=GetActorLocation()+GetActorUpVector()*60;
 FVector Desired=GetActorTransform().TransformPosition(FVector(-450,85,230));FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(TrainingRoverCamera),false);Q.AddIgnoredActor(this);if(IsValid(Rider))Q.AddIgnoredActor(Rider);
 if(GetWorld()->SweepSingleByChannel(Hit,Target,Desired,FQuat::Identity,ECC_Camera,FCollisionShape::MakeSphere(12),Q))Desired=Hit.Location;
 FollowCamera->SetWorldLocation(D>0?FMath::VInterpTo(FollowCamera->GetComponentLocation(),Desired,D,10):Desired);FollowCamera->SetWorldRotation((Target-FollowCamera->GetComponentLocation()).Rotation());
}
void ALunarSurfaceVehicle::Tick(float D){
 Super::Tick(D);
 if(!Occupied){if(RecoveryPawn.IsValid()){if(FVector::Dist2D(RecoveryPawn->GetActorLocation(),GetActorLocation())>240){RecoveryPawn->GetCapsuleComponent()->IgnoreActorWhenMoving(this,false);RecoveryPawn.Reset();SetActorTickEnabled(false);}}else SetActorTickEnabled(false);return;}
 if(!IsValid(Mission)||!IsValid(Operations)||!IsValid(Rider)){Exit(true);return;}
 if(Mission->Paused||Rider->ActivePanel||!Mission->IsActive()||LunarLoading::IsBusy()){Throttle=Steering=0;return;}
 if(!Operations->ReadyForVehicle()){Halt(TEXT("Rover driving is paused until field conditions are ready."));return;}
 if(!FMath::IsFinite(D)||D<=0)return;D=FMath::Min(D,.1f);
 Drive(D);if(IsValid(Rider)){Rider->SetActorRelativeLocation(LunarSurfaceVehiclePrivate::Seat);Rider->LastStepPosition=Rider->GetActorLocation();}PoseCamera(D);
}
bool ALunarSurfaceVehicle::GroundedExitPose(FVector& Grounded) const {
 auto* P=IsValid(Rider)?Rider.Get():LunarSurfaceVehiclePrivate::Pilot(Mission);if(!P||!P->GetCapsuleComponent()||!GetWorld())return false;
 const float Radius=P->GetCapsuleComponent()->GetScaledCapsuleRadius(),Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
 const FVector From=IsOccupied()?P->GetActorLocation():GetActorTransform().TransformPosition(LunarSurfaceVehiclePrivate::Seat);
 for(const FVector& Offset:{FVector(0,240,0),FVector(0,-240,0),FVector(-275,0,0),FVector(275,0,0)}){
  const FVector XY=GetActorLocation()+FRotator(0,GetActorRotation().Yaw,0).RotateVector(Offset);FHitResult Surface;if(!GroundAt(XY,Surface)||Surface.ImpactNormal.Z<FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(SlopeLimitDegrees,1.f,30.f))))continue;
  const FVector Candidate=Surface.ImpactPoint+FVector(0,0,Half+4+Radius*(1/Surface.ImpactNormal.Z-1));
  if(FMath::Abs(Candidate.Z-From.Z)>125)continue;
  FCollisionQueryParams Q(SCENE_QUERY_STAT(TrainingRoverExit),false);Q.AddIgnoredActor(this);Q.AddIgnoredActor(P);const auto CapsuleShape=FCollisionShape::MakeCapsule(Radius,Half);
  if(GetWorld()->OverlapBlockingTestByChannel(Candidate,FQuat::Identity,ECC_Pawn,CapsuleShape,Q))continue;
  FHitResult Block;if(GetWorld()->SweepSingleByChannel(Block,From,Candidate,FQuat::Identity,ECC_Pawn,CapsuleShape,Q))continue;
  Grounded=Candidate;return !Grounded.ContainsNaN();
 }
 return false;
}
void ALunarSurfaceVehicle::RestoreRider(const FVector* ExitPosition){
 auto* P=Rider.Get();if(!IsValid(P)){Occupied=false;Rider=nullptr;return;}auto* PC=Cast<APlayerController>(P->GetController());
 P->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);if(ExitPosition)P->SetActorLocation(*ExitPosition,false,nullptr,ETeleportType::None);
 P->SetActorRotation(FRotator(0,PreviousControl.Yaw,0));P->ThirdPerson=PreviousThird;P->Camera->bUsePawnControlRotation=PreviousCameraControl;P->Camera->SetRelativeTransform(PreviousCamera);P->Camera->SetFieldOfView(PreviousFov);
 P->SetActorHiddenInGame(PreviousHidden);P->SetActorEnableCollision(PreviousCollision);
 if(auto* Move=P->GetCharacterMovement()){Move->SetMovementMode(EMovementMode(PreviousMoveMode),PreviousCustomMode);Move->StopMovementImmediately();Move->SetComponentTickEnabled(PreviousMoveTick&&(!IsValid(Mission)||!Mission->Paused));}
 Hull->IgnoreActorWhenMoving(P,false);
 // A forced transition may have no external exit. Never teleport to the old
 // entry: temporarily ignore this hull until the on-foot pawn walks clear.
 if(ExitPosition||Ending)P->GetCapsuleComponent()->IgnoreActorWhenMoving(this,false);else RecoveryPawn=P;
 P->LastStepPosition=P->GetActorLocation();P->StepTravel=0;P->PausedVelocity=FVector::ZeroVector;
 // Dismount is a validated placement, not an on-foot route segment.
 if(IsValid(Mission)&&Mission->Apollo){Mission->ApolloLastPosition=P->GetActorLocation();Mission->ApolloTrailPendingBreak=true;}
 if(PC){PC->SetControlRotation(PreviousControl);if(PC->GetViewTarget()==this){AActor* View=PreviousViewTarget.Get();if(!IsValid(View))View=P;PC->SetViewTarget(View);}}
 Occupied=false;Rider=nullptr;Throttle=Steering=SpeedCm=ActualSpeedMps=0;SetActorTickEnabled(RecoveryPawn.IsValid()&&!Ending);
}
bool ALunarSurfaceVehicle::Exit(bool Force){
 if(!IsOccupied()){Occupied=false;SetActorTickEnabled(RecoveryPawn.IsValid());return true;}
 FVector Spot;SpeedCm=0;Throttle=Steering=0;const bool Found=GroundedExitPose(Spot);
 if(!Found&&!Force){Halt(TEXT("Exit blocked. Drive to clear ground before leaving the rover."));return false;}
 RestoreRider(Found?&Spot:nullptr);if(IsValid(Mission)&&!Ending)Mission->Message(TEXT("Rover parked. On-foot control restored."));return true;
}
void ALunarSurfaceVehicle::Abort(ALunarGameMode* G){if(G&&G->FieldOperations&&IsValid(G->FieldOperations->Vehicle))G->FieldOperations->Vehicle->Exit(true);}
bool ALunarSurfaceVehicle::RestoreParkedPose(const FTransform& Pose){
 if(IsOccupied()||Pose.ContainsNaN()||!Pose.GetScale3D().Equals(FVector::OneVector,.001)||!Pose.GetRotation().IsNormalized())return false;
 FGroundPose Ground;if(!SampleGround(Pose.GetLocation(),Pose.Rotator().Yaw,Ground)||FMath::Abs(Pose.GetLocation().Z-Ground.Centre.Z)>45||FMath::Abs(Pose.Rotator().Pitch)>35||FMath::Abs(Pose.Rotator().Roll)>35)return false;
 const FQuat Rotation=LunarSurfaceVehiclePrivate::GroundRotation(Ground.Normal,Pose.Rotator().Yaw);
 // Unlike driving with an attached rider, parking must include the restored
 // on-foot pawn in the actual world overlap query. Never enclose that pawn.
 FCollisionQueryParams Parking(SCENE_QUERY_STAT(TrainingRoverPark),false);Parking.AddIgnoredActor(this);
 if(GetWorld()->OverlapBlockingTestByChannel(Ground.Centre,Rotation,ECC_Vehicle,FCollisionShape::MakeBox(LunarSurfaceVehiclePrivate::HullExtent),Parking))return false;
 SetActorLocationAndRotation(Ground.Centre,Rotation,false,nullptr,ETeleportType::TeleportPhysics);SpeedCm=ActualSpeedMps=Throttle=Steering=0;PoseWheels(Ground,0);PoseCamera(0);return true;
}
FString ALunarSurfaceVehicle::InspectionText() const {
 return FString::Printf(TEXT("TRAINING ROVER / authored driving model\n%s\nFour-wheel ground checks, swept hull, %.1f m/s forward limit and %.0f degree gameplay slope limit. Movement keys drive/steer; use the work key to stop and exit. Charge pays for actual movement only. This is not a certified vehicle simulator.\nSession travel: %.1f m."),Mission&&Mission->Mars?TEXT("Fictional Mars surface vehicle; inspired by SEV research concepts."):Mission&&Mission->Apollo?TEXT("Fictional modern training addition. Apollo 12 did not use a lunar rover."):TEXT("Fictional SELENE training vehicle."),FMath::Clamp(ForwardLimitCm,0.f,800.f)*.01f,SlopeLimitDegrees,TravelMeters);
}
void ALunarSurfaceVehicle::EndPlay(const EEndPlayReason::Type Reason){
 Ending=true;Exit(true);if(RecoveryPawn.IsValid()){RecoveryPawn->GetCapsuleComponent()->IgnoreActorWhenMoving(this,false);RecoveryPawn.Reset();}
 if(IsValid(Operations)&&Operations->Vehicle==this)Operations->Vehicle=nullptr;Super::EndPlay(Reason);
}
