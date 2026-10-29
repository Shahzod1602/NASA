#include "LunarGame.h"
#include "LunarSettings.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"

namespace {
constexpr int32 FootprintLimit=160;
constexpr float TeleportStepLimit=100.f;
bool IsRegolith(const UPrimitiveComponent* Component){
 const auto* Mesh=Cast<UStaticMeshComponent>(Component);
 return IsValid(Mesh)&&Mesh->GetStaticMesh()&&Mesh->GetStaticMesh()->GetName().Contains(TEXT("Terrain"));
}
FVector SurfaceForward(FVector Forward,FVector Normal){
 if(Forward.ContainsNaN()||Normal.ContainsNaN())return FVector::ZeroVector;
 return FVector::VectorPlaneProject(Forward,Normal.GetSafeNormal()).GetSafeNormal();
}
FRotator PrintRotation(FVector Forward,FVector Normal){return FRotationMatrix::MakeFromXZ(-Normal,SurfaceForward(Forward,Normal)).Rotator();}
}

void ALunarCharacter::ResetFootprints(){
 for(auto& Mark:Footprints)if(IsValid(Mark))Mark->DestroyComponent();
 Footprints.Reset();FootprintCount=0;LeftFoot=false;StepTravel=0;LastStepPosition=GetActorLocation();
}

void ALunarCharacter::StampFootstep(){
 auto* Game=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));
 auto* Movement=GetCharacterMovement();
 // Walking calls are spaced by travelled distance. This second guard also
 // rejects explicit calls during menus, pause, air movement or a teleport.
 if(!Game||!Game->IsActive()||Game->Paused||!Movement||!Movement->IsMovingOnGround()||
    GetActorLocation().ContainsNaN()||FVector::Dist2D(GetActorLocation(),LastStepPosition)>TeleportStepLimit)return;
 if(StepSounds.Num()==3&&StepSounds[FootprintCount%3])StepAudio=UGameplayStatics::SpawnSound2D(this,StepSounds[FootprintCount%3],.7f*(ULunarSettings::Get()->EffectsVolume*ULunarSettings::Get()->MasterVolume),FMath::FRandRange(.94f,1.06f));
 const auto* Floor=Movement->CurrentFloor.HitResult.GetComponent();
 // Never project a ground mark through equipment when standing on that prop.
 if(!Movement->CurrentFloor.IsWalkableFloor()||!IsRegolith(Floor))return;
 const bool NextLeft=!LeftFoot;
 const int32 FootIndex=NextLeft?6:9;
 FVector Centre=GetActorLocation()+GetActorRightVector()*(NextLeft?-12.f:12.f);
 FVector Forward=GetActorForwardVector();
 // Pose proxies are also used to drive the skeletal feet. Match that actual
 // sole position and heading instead of the free-look camera's yaw.
 if(SuitBody.IsValidIndex(FootIndex)&&IsValid(SuitBody[FootIndex])){
  const FVector Posed=SuitBody[FootIndex]->GetComponentLocation();
  if(!Posed.ContainsNaN()&&FVector::Dist2D(Posed,GetActorLocation())<80.f){Centre=Posed;Forward=SuitBody[FootIndex]->GetForwardVector();}
 }
 FHitResult Hit;FCollisionQueryParams Query;Query.AddIgnoredActor(this);Query.bTraceComplex=true;
 if(!GetWorld()->LineTraceSingleByChannel(Hit,Centre+FVector(0,0,40),Centre-FVector(0,0,100),ECC_Visibility,Query)||
    !IsRegolith(Hit.GetComponent())||Hit.GetComponent()!=Floor||Hit.ImpactPoint.ContainsNaN()||Hit.ImpactNormal.ContainsNaN()||
    Hit.ImpactNormal.Z<Movement->GetWalkableFloorZ()||SurfaceForward(Forward,Hit.ImpactNormal).IsNearlyZero())return;
 LeftFoot=NextLeft;
 if(!BootMaterial)return;
 const FRotator Rotation=PrintRotation(Forward,Hit.ImpactNormal);
 // The original procedural tread material remains unchanged. Recycle a fixed
 // pool instead of allocating/destroying one component at every later step.
 const int32 Slot=FootprintCount%FootprintLimit;
 UDecalComponent* Mark=Footprints.IsValidIndex(Slot)?Footprints[Slot].Get():nullptr;
 if(!IsValid(Mark)){
  Mark=NewObject<UDecalComponent>(this);Mark->SetDecalMaterial(BootMaterial);Mark->DecalSize=FVector(4,8,17);
  Mark->SetFadeScreenSize(.0001f);Mark->RegisterComponent();
  if(Footprints.IsValidIndex(Slot))Footprints[Slot]=Mark;else Footprints.Add(Mark);
 }
 Mark->SetWorldLocationAndRotation(Hit.ImpactPoint+Hit.ImpactNormal*.35f,Rotation);Mark->SetVisibility(true);
 ++FootprintCount;
 // Marks persist until replaced after 160 newer steps or an explicit mission
 // reset. No real-time fade can continue while the mission is paused.
}

void ALunarCharacter::AppendFootprintChecks(FString& Report,int32& Failed){
 auto Check=[&](bool Good,const TCHAR* Description){Report+=FString::Printf(TEXT("%s %s\n"),Good?TEXT("PASS"):TEXT("FAIL"),Description);if(!Good)++Failed;};
 auto* Game=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));
 auto* Movement=GetCharacterMovement();
 if(!Game||!Movement){Check(false,TEXT("Footprint fixture has a character and mission"));return;}
 const FVector Flat=SurfaceForward(FVector::ForwardVector,FVector::UpVector);
 Check(Flat.Equals(FVector::ForwardVector,.0001f),TEXT("Flat tread keeps the boot heading"));
 const FVector Normal=FVector(-.3f,.2f,1).GetSafeNormal();
 const FVector Projected=SurfaceForward(FVector::ForwardVector,Normal);
 const FRotationMatrix Matrix(PrintRotation(FVector::ForwardVector,Normal));
 Check(FMath::Abs(FVector::DotProduct(Projected,Normal))<.0001f&&FMath::Abs(Projected.Size()-1.f)<.0001f,TEXT("Slope tread heading is a unit tangent to the measured surface"));
 Check(Matrix.GetUnitAxis(EAxis::X).Equals(-Normal,.0001f)&&Matrix.GetUnitAxis(EAxis::Z).Equals(Projected,.0001f),TEXT("Slope decal projects down onto the surface with its long axis along the boot"));
 Check(SurfaceForward(FVector::UpVector,FVector::UpVector).IsNearlyZero(),TEXT("Degenerate vertical boot direction is rejected"));
 auto* Prop=NewObject<UStaticMeshComponent>(this);Prop->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 Check(!IsRegolith(Prop)&&!IsRegolith(nullptr),TEXT("Equipment shapes and missing components are not regolith"));
 const FTransform Before=GetActorTransform();const FVector BeforeLast=LastStepPosition;const float BeforeTravel=StepTravel;
 const auto BeforeMode=Movement->MovementMode;const auto BeforeFloor=Movement->CurrentFloor;
 const ELunarStage BeforeStage=Game->Stage;const bool BeforePause=Game->Paused;
 ResetFootprints();Game->Paused=false;Game->Stage=Game->Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule;
 Check(Footprints.IsEmpty()&&FootprintCount==0&&StepTravel==0,TEXT("Mission reset clears footprint components, counters and partial stride"));
 Movement->SetMovementMode(MOVE_Falling);StampFootstep();
 Check(Footprints.IsEmpty(),TEXT("Airborne calls cannot leave a footprint"));
 Movement->SetMovementMode(MOVE_Walking);Game->Paused=true;StampFootstep();
 Check(Footprints.IsEmpty(),TEXT("Paused calls cannot create footprints"));Game->Paused=false;
 // Use the actual imported regolith under the saved character location. This
 // is an integration check, not a claim about historical astronaut footprints.
 FHitResult Ground;FCollisionQueryParams Query;Query.AddIgnoredActor(this);Query.bTraceComplex=true;
 const bool OnTerrain=GetWorld()->LineTraceSingleByChannel(Ground,Before.GetLocation()+FVector(0,0,500),Before.GetLocation()-FVector(0,0,5000),ECC_Visibility,Query)&&IsRegolith(Ground.GetComponent());
 Check(OnTerrain,TEXT("Footprint integration fixture locates real imported terrain"));
 if(OnTerrain){
  SetActorLocation(Ground.ImpactPoint+FVector(0,0,90));Movement->SetMovementMode(MOVE_Walking);Movement->FindFloor(GetActorLocation(),Movement->CurrentFloor,false);
  LastStepPosition=GetActorLocation();UpdateSuit(0);StampFootstep();
  Check(FootprintCount==1&&Footprints.Num()==1,TEXT("A grounded posed boot stamps one regolith tread"));
  if(Footprints.Num()==1){
   const auto* Mark=Footprints[0].Get();const auto* Boot=SuitBody.IsValidIndex(6)?SuitBody[6].Get():nullptr;
   Check(Boot&&FVector::Dist2D(Mark->GetComponentLocation(),Boot->GetComponentLocation())<4,TEXT("The tread follows the posed left boot rather than camera yaw"));
   Check(FVector::DotProduct(Mark->GetForwardVector(),-Ground.ImpactNormal)>.99f,TEXT("The emitted tread projection follows imported terrain normal"));
  }
  const int32 Count=FootprintCount;LastStepPosition=GetActorLocation()+FVector(1000,0,0);StampFootstep();
  Check(FootprintCount==Count,TEXT("An explicit teleport cannot stamp a connecting footprint"));LastStepPosition=GetActorLocation();
  auto FloorBackup=Movement->CurrentFloor;Movement->CurrentFloor.HitResult=FHitResult(this,Prop,Ground.ImpactPoint,FVector::UpVector);StampFootstep();
  Check(FootprintCount==Count,TEXT("A grounded equipment contact cannot project a tread through to regolith"));
  Movement->CurrentFloor.Clear();StampFootstep();
  Check(FootprintCount==Count,TEXT("Missing regolith contact cannot create a footprint"));Movement->CurrentFloor=FloorBackup;
  auto* Original=Footprints.IsEmpty()?nullptr:Footprints[0].Get();
  for(int32 I=0;I<FootprintLimit+2;++I)StampFootstep();
  Check(Footprints.Num()==FootprintLimit&&FootprintCount>FootprintLimit,TEXT("A long walk retains at most 160 visible tread components"));
  Check(Footprints.Num()&&Footprints[0].Get()==Original,TEXT("Old footprint component is recycled rather than reallocated"));
  Game->Paused=true;const auto Frozen=Footprints.IsEmpty()?FTransform::Identity:Footprints.Last()->GetComponentTransform();const int32 FrozenCount=FootprintCount;StampFootstep();
  Check(Footprints.Num()>0&&FootprintCount==FrozenCount&&Footprints.Last()->GetComponentTransform().Equals(Frozen),TEXT("Paused trail preserves its count and rendered transform"));Game->Paused=false;
 }
 ResetFootprints();Check(Footprints.IsEmpty()&&FootprintCount==0&&!LeftFoot,TEXT("Restart removes the old trail and restarts left/right sequencing"));
 SetActorTransform(Before);Movement->SetMovementMode(BeforeMode);Movement->CurrentFloor=BeforeFloor;LastStepPosition=BeforeLast;StepTravel=BeforeTravel;Game->Stage=BeforeStage;Game->Paused=BeforePause;UpdateSuit(0);
}
