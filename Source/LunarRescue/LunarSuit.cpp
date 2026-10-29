#include "LunarGame.h"
#include "LunarCargoFinale.h"
#include "LunarApolloText.h"
#include "LunarSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"
#include "InputKeyEventArgs.h"
#include "HAL/FileManager.h"
#include <limits>

namespace {
// Presentation filters use an exponential response; 30/60 Hz updates have the
// same response over equal time. A hitch cannot throw the pose through a stride.
float SuitMotionWeight(float Delta,float Rate){
 if(!FMath::IsFinite(Delta)||!FMath::IsFinite(Rate)||Delta<=0||Rate<=0)return 0;
 return 1.f-FMath::Exp(-Rate*FMath::Min(Delta,.1f));
}
// The mesh's X axis follows the limb. All assets are authored in centimetres.
void Limb(UStaticMeshComponent* C,FVector A,FVector B,float Length){
 C->SetRelativeLocation(A);C->SetRelativeRotation(FRotationMatrix::MakeFromX(B-A).Rotator());C->SetRelativeScale3D(FVector((B-A).Size()/Length,1,1));
}
FVector Joint(FVector A,FVector B,float Upper,float Lower,FVector Bend){
 FVector Axis=(B-A).GetSafeNormal();float D=FMath::Clamp((B-A).Size(),1.f,Upper+Lower-.1f);
 float Along=(Upper*Upper-Lower*Lower+D*D)/(2*D);
 FVector Side=(Bend-Axis*FVector::DotProduct(Bend,Axis)).GetSafeNormal();
 return A+Axis*Along+Side*FMath::Sqrt(FMath::Max(0.f,Upper*Upper-Along*Along));
}
void Hand(const TArray<TObjectPtr<UStaticMeshComponent>>& Parts,int Base,FVector Elbow,FVector Wrist,float Side,float Work,bool Upper=false,const FRotator* ToolFrame=nullptr){
 int N=Base;if(Upper)++N;
 Limb(Parts[N++],Elbow,Wrist,24);
 const FRotator R=ToolFrame?*ToolFrame:FRotator(8+Work*12,-Side*(7+Work*5),Side*7);const FTransform Frame(R,Wrist);
 Parts[N]->SetRelativeLocation(Wrist);Parts[N++]->SetRelativeRotation(R);
 Parts[N]->SetRelativeLocation(Frame.TransformPosition(FVector(1,0,0)));Parts[N++]->SetRelativeRotation(R);
 const float Lengths[]={.95f,1.f,.92f,.74f};
 for(int F=0;F<4;++F){
  auto* C=Parts[N++].Get();C->SetRelativeLocation(Frame.TransformPosition(FVector(9,(-3.15f+F*2.1f)*Side,.1f)));
  C->SetRelativeRotation((R.Quaternion()*FRotator(-9-Work*(F==0?24.f:34.f),(F-1.5f)*Side*(3-Work*2),0).Quaternion()).Rotator());C->SetRelativeScale3D(FVector(Lengths[F],1,1));
 }
 Parts[N]->SetRelativeLocation(Frame.TransformPosition(FVector(4,-Side*4,-1)));
 Parts[N]->SetRelativeRotation((R.Quaternion()*FRotator(-16-Work*18,-Side*(48-Work*19),Side*Work*14).Quaternion()).Rotator());Parts[N]->SetRelativeScale3D(FVector(.70f,1.1f,1.1f));
}
}

void ALunarCharacter::BuildGloves(){
 auto Make=[&](const TCHAR* Name,bool View)->UStaticMeshComponent*{
  auto* C=NewObject<UStaticMeshComponent>(this);C->SetupAttachment(View?static_cast<USceneComponent*>(Camera):GetCapsuleComponent());
  C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Suit/Meshes/%s.%s"),Name,Name)));
  C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(!View);C->bCastHiddenShadow=!View;C->SetOnlyOwnerSee(true);C->SetReceivesDecals(false);C->RegisterComponent();return C;
 };
 const TCHAR* Body[]={TEXT("SM_SuitTorso"),TEXT("SM_SuitPelvis"),TEXT("SM_SuitHelmet"),TEXT("SM_SuitPack"),TEXT("SM_SuitThigh"),TEXT("SM_SuitShin"),TEXT("SM_SuitBoot"),TEXT("SM_SuitThigh"),TEXT("SM_SuitShin"),TEXT("SM_SuitBoot")};
 for(auto* Name:Body)SuitBody.Add(Make(Name,false));
 for(int S=0;S<2;++S){
  SuitArms.Add(Make(TEXT("SM_SuitUpperArm"),false));
  const TCHAR* Names[]={TEXT("SM_SuitForearm"),TEXT("SM_SuitCuff"),TEXT("SM_SuitPalm"),TEXT("SM_SuitFinger"),TEXT("SM_SuitFinger"),TEXT("SM_SuitFinger"),TEXT("SM_SuitFinger"),TEXT("SM_SuitFinger")};
  for(auto* Name:Names){Gloves.Add(Make(Name,true));SuitArms.Add(Make(Name,false));}
 }
 bUseControllerRotationYaw=false;GetCharacterMovement()->bOrientRotationToMovement=false;
 auto* Label=NewObject<UTextRenderComponent>(this);Label->SetupAttachment(SuitBody[0]);Label->SetRelativeLocation(FVector(18.2f,0,25.2f));Label->SetText(FText::FromString(TEXT("APOLLO 12")));Label->SetWorldSize(1.6f);Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor(215,220,215));Label->RegisterComponent();
 if(auto* PC=Cast<APlayerController>(GetController()))if(PC->PlayerCameraManager){PC->PlayerCameraManager->ViewPitchMin=-85;PC->PlayerCameraManager->ViewPitchMax=70;}
 UpdateSuit(0);
 if(FParse::Param(FCommandLine::Get(),TEXT("SuitReview"))||FParse::Param(FCommandLine::Get(),TEXT("ExpeditionMotionReview"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitReview,5,false);}
 if(FParse::Param(FCommandLine::Get(),TEXT("SuitTest"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitChecks,4,false);}
}
void ALunarCharacter::BeginFastStride(){auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));if(ActivePanel||!G||G->Paused||G->ApolloTool)return;FastStride=ULunarSettings::Get()->ToggleSprint?!FastStride:true;}
void ALunarCharacter::EndFastStride(){if(!ULunarSettings::Get()->ToggleSprint)FastStride=false;}
void ALunarCharacter::BeginSuitOrbit(){if(!ALunarCargoFinale::Active(Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this))))SuitOrbit=true;}
void ALunarCharacter::EndSuitOrbit(){SuitOrbit=false;}
void ALunarCharacter::ToggleSuitView(){
 if(ALunarCargoFinale::Active(Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this))))return;
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));if(!G||G->Menu||G->Journal||G->ApolloTool||ActivePanel)return;
 ThirdPerson=!ThirdPerson;Camera->bUsePawnControlRotation=!ThirdPerson;Camera->SetFieldOfView(ULunarSettings::Get()->FieldOfView);
 if(!ThirdPerson){Camera->SetRelativeLocation(FVector(0,0,68));Camera->SetRelativeRotation(FRotator::ZeroRotator);}
 UpdateSuit(0);
}
void ALunarCharacter::GetActorEyesViewPoint(FVector& Location,FRotator& Rotation) const {
 // Keep interaction traces at the configured character eye height in both views.
 Super::GetActorEyesViewPoint(Location,Rotation);
}
void ALunarCharacter::UpdateSuit(float D){
 if(SuitBody.Num()!=10||Gloves.Num()!=16||SuitArms.Num()!=18)return;
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));const bool Active=G&&G->IsActive();const bool Paused=G&&G->Paused;
 auto* Move=GetCharacterMovement();
 if(G&&G->Apollo){Move->MaxWalkSpeed=FastStride&&!G->Holding?220.f:140.f;Move->MaxAcceleration=420;Move->BrakingDecelerationWalking=650;Move->GroundFriction=6;Move->AirControl=.12f;}
 // Keep normal pause ticks completely still. Explicit zero-delta pose calls
 // remain available to view switches, review fixtures and photography checks.
 if(Paused&&D>0){PoseSuitRig();return;}
 if(!FMath::IsFinite(D)||D<0)D=0;
 const bool HandMotion=ULunarSettings::Get()->CameraMotion;
 if(!Paused&&D>0){
  const float Speed=GetVelocity().Size2D();
  const float TurnAngle=FMath::Abs(FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,GetControlRotation().Yaw));
  const bool Turning=Active&&Speed<15&&TurnAngle>5&&!(ThirdPerson&&SuitOrbit);
  const float StrideSpeed=Active&&Move->IsMovingOnGround()?FMath::Max(Speed,Turning?FMath::Min(140.f,TurnAngle*6.f):0.f):0.f;
  SuitStrideSpeed=FMath::Lerp(SuitStrideSpeed,StrideSpeed,SuitMotionWeight(D,StrideSpeed<SuitStrideSpeed?10.f:7.f));
  if(SuitStrideSpeed<.15f)SuitStrideSpeed=0;
  GaitBlend=FMath::Clamp(SuitStrideSpeed/140.f,0.f,1.f);
  GaitPhase+=SuitStrideSpeed*FMath::Min(D,.1f)/(FastStride?160.f:128.f)*2*PI;
  // Bounded torso/hand follow-through is cosmetic; the capsule and camera
  // targets do not receive sway, lean or landing offsets.
  const float SignedTurn=FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,GetControlRotation().Yaw);
  const float TurnTarget=Active&&!(ThirdPerson&&SuitOrbit)?FMath::Clamp(SignedTurn/45.f,-1.f,1.f):0.f;
  SuitTurnLean=FMath::Lerp(SuitTurnLean,TurnTarget,SuitMotionWeight(D,9));
  const bool PhysicalTool=G&&(G->ApolloTool==1||G->ApolloTool==4||G->ApolloTool==5||G->ApolloTool==6);
  WorkBlend=FMath::Lerp(WorkBlend,G&&(G->Holding||PhysicalTool)?1.f:0.f,SuitMotionWeight(D,8));
  LandBlend=FMath::Lerp(LandBlend,0.f,SuitMotionWeight(D,5));
  if(!(ThirdPerson&&SuitOrbit))SetActorRotation(FQuat::Slerp(GetActorQuat(),FRotator(0,GetControlRotation().Yaw,0).Quaternion(),SuitMotionWeight(D,10)));
 }
 const bool Show=G&&!G->Menu&&(Active||G->Stage==ELunarStage::Briefing||G->Stage==ELunarStage::Won);
 for(const auto& C:SuitBody)C->SetVisibility(Show);SuitBody[2]->SetVisibility(Show&&ThirdPerson);
 SuitBody[0]->SetVisibility(Show&&ThirdPerson,true);SuitBody[3]->SetVisibility(Show&&ThirdPerson);
 SuitBody[1]->SetVisibility(Show&&ThirdPerson);
 for(const auto& C:SuitArms)C->SetVisibility(Show&&ThirdPerson);
 for(const auto& C:Gloves)C->SetVisibility(Show&&!ThirdPerson&&Camera->FieldOfView>60);
 const float Breath=FMath::Sin(SuitTime*1.5f)*.18f;
 const float LandingCompression=FMath::Clamp(LandBlend,0.f,2.2f)*1.6f;
 const float Crouch=GaitBlend*((FastStride?6.f:3.f)+FMath::Cos(GaitPhase*2)*(FastStride?1.8f:1.f))+LandingCompression;
 const float TorsoLean=FMath::Clamp(SuitTurnLean,-1.f,1.f)*1.6f;
 SuitBody[0]->SetRelativeLocation(FVector(0,-TorsoLean,Breath-Crouch));SuitBody[1]->SetRelativeLocation(FVector(0,0,-Crouch));
 SuitBody[2]->SetRelativeLocation(FVector(0,-TorsoLean*.75f,64+Breath-Crouch));SuitBody[3]->SetRelativeLocation(FVector(0,-TorsoLean,Breath-Crouch));
 if(ThirdPerson){
  FVector Target=GetActorLocation()+FVector(0,0,40);FRotator R=GetControlRotation();
  FVector Desired=Target+R.RotateVector(FVector(-265,42,55));FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
  if(GetWorld()->SweepSingleByChannel(Hit,Target,Desired,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(12),Q))Desired=Hit.Location;
  Camera->SetWorldLocation(D>0?FMath::VInterpTo(Camera->GetComponentLocation(),Desired,D,12):Desired);
  Camera->SetWorldRotation((Target+R.Vector()*70-Camera->GetComponentLocation()).Rotation());
 }
 else if(G&&G->ApolloTool>1&&G->ApolloWorkTarget){
  const FVector Target=(G->ApolloTool==4||G->ApolloTool==5||G->ApolloTool==6)?G->ApolloContactPoint():G->ApolloWorkTarget->GetActorLocation();
  const bool Kit=G->ApolloTool==7||G->ApolloTool==8;
  const float WorkDistance=Kit?125.f:G->ApolloTool==6?105.f:95.f;
  const FVector WorkDirection=G->ApolloTool==6&&G->SelectedSample==2?FVector(0,1,0):GetControlRotation().Vector().GetSafeNormal2D();
  FVector Desired=Target-WorkDirection*WorkDistance+FVector(0,0,Kit?70.f:G->ApolloTool==6?30.f:(G->ApolloTool==4||G->ApolloTool==5)?85.f:25.f);
  FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
  if(GetWorld()->SweepSingleByChannel(Hit,Target+FVector(0,0,30),Desired,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(6),Q))Desired=Hit.Location;
  Camera->SetWorldLocation(D>0?FMath::VInterpTo(Camera->GetComponentLocation(),Desired,D,10):Desired);
  Camera->SetWorldRotation((Target-Camera->GetComponentLocation()).Rotation());
 }
 // Camera first, tool contacts second, glove pose last: all use one view transform.
 FVector Direction=GetActorTransform().InverseTransformVectorNoScale(Paused?PausedVelocity:GetVelocity()).GetSafeNormal2D();
 if(D>0&&!Direction.IsNearlyZero()){
  const float CurrentAngle=FMath::RadiansToDegrees(FMath::Atan2(SuitMoveDirection.Y,SuitMoveDirection.X));
  const float DesiredAngle=FMath::RadiansToDegrees(FMath::Atan2(Direction.Y,Direction.X));
  const float BlendedAngle=FMath::DegreesToRadians(CurrentAngle+FMath::FindDeltaAngleDegrees(CurrentAngle,DesiredAngle)*SuitMotionWeight(D,12));
  SuitMoveDirection=FVector(FMath::Cos(BlendedAngle),FMath::Sin(BlendedAngle),0);
 }
 // Preserve the last walking direction during the short stop tail, while
 // stationary turns keep planted XY and lift the feet in place.
 if(GetVelocity().Size2D()>5||FMath::Abs(SuitTurnLean)<.08f)Direction=D>0?SuitMoveDirection:Direction;
 UpdateApolloWorkVisuals();
 for(int S=0;S<2;++S){
  const float Side=S?1.f:-1.f;const float Phase=GaitPhase+S*PI;const float Swing=FMath::Sin(Phase)*GaitBlend;
  const float Cycle=FMath::Fmod(Phase,2*PI)/(2*PI),HalfStride=FastStride?40.f:32.f;
  const float T=Cycle<.5f?Cycle*2:(Cycle-.5f)*2;
  const float Plant=Cycle<.5f?HalfStride*(-1+2*T*T*(3-2*T)):HalfStride*(1-2*T);
  FVector Hip(0,Side*12,-12-Crouch),Foot=FVector(Direction.X,Direction.Y,0)*Plant*GaitBlend+FVector(3,Side*12,-78);
  float Lift=Cycle<.5f?FMath::Sin(T*PI)*16*GaitBlend:0.f;
  if(Move->IsFalling()){Foot.X+=12;Foot.Z+=16;Lift=0;}
  // A planted foot stays in world space for the stance half of the cycle.
  const bool Stance=Cycle>=.5f&&Move->IsMovingOnGround()&&GaitBlend>.15f;
  if(Stance){if(!FootPlanted[S])PlantedFeet[S]=GetActorTransform().TransformPosition(Foot);const FVector Locked=GetActorTransform().InverseTransformPosition(PlantedFeet[S]);if(FVector::Dist2D(Locked,Foot)<65){Foot.X=Locked.X;Foot.Y=Locked.Y;}else PlantedFeet[S]=GetActorTransform().TransformPosition(Foot);}
  FootPlanted[S]=Stance;
  FRotator SoleRotation(Swing*7,0,0);
  if(!Move->IsFalling()){
   // Trace after locking stance XY: the boot must use the ground beneath its
   // planted position, not the moving animation target.
   const FVector W=GetActorTransform().TransformPosition(Foot);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
   if(GetWorld()->LineTraceSingleByChannel(Hit,W+FVector(0,0,65),W-FVector(0,0,90),ECC_Visibility,Q)&&Hit.ImpactNormal.Z>.65f){
    Foot.Z=FMath::Clamp(GetActorTransform().InverseTransformPosition(Hit.ImpactPoint).Z+11,-90.f,-65.f);
    const FVector LocalNormal=GetActorTransform().InverseTransformVectorNoScale(Hit.ImpactNormal).GetSafeNormal();
    SoleRotation=FRotationMatrix::MakeFromZX(LocalNormal,FVector::ForwardVector).Rotator();
    SoleRotation.Pitch=FMath::Clamp(SoleRotation.Pitch,-25.f,25.f)+(Stance?0.f:Swing*7);
    SoleRotation.Roll=FMath::Clamp(SoleRotation.Roll,-25.f,25.f);SoleRotation.Yaw=0;
   }
  }
  Foot.Z+=Lift;FVector Knee=Joint(Hip,Foot,36,34,FVector(1,0,0));int B=4+S*3;
  // The walking view uses an off-screen trouser extension: its proximal
  // closure stays behind the helmet camera instead of showing a sliced thigh.
  // Physical hips, knees, feet, collision and the third-person rig are unchanged.
  const FVector PresentedHip=!ThirdPerson&&G&&!G->ApolloTool&&Camera->FieldOfView>60?FVector(-44,Side*12,16-Crouch):Hip;
  Limb(SuitBody[B],PresentedHip,Knee,36);Limb(SuitBody[B+1],Knee,Foot,34);SuitBody[B+2]->SetRelativeLocation(Foot);SuitBody[B+2]->SetRelativeRotation(SoleRotation);
  FVector Shoulder(0,Side*25,40-Crouch),Wrist(12-Swing*8,Side*(28-WorkBlend*13),-10+WorkBlend*30-Crouch);
  Wrist.X+=WorkBlend*28;if(Move->IsFalling())Wrist.Z+=12;
  FVector Elbow=Joint(Shoulder,Wrist,28,24,FVector(0,Side,-.4f));Limb(SuitArms[S*9],Shoulder,Elbow,28);
  Hand(SuitArms,S*9,Elbow,Wrist,Side,WorkBlend,true);
  const float Sway=HandMotion?FMath::Sin(Phase)*1.1f*GaitBlend:0.f;
  FVector ViewElbow(8,Side*25,-28),ViewWrist(36+WorkBlend*3+Sway,Side*(20-WorkBlend*10),-15+FMath::Abs(Sway)*.35f-(HandMotion?LandBlend:0.f));
  if(G&&(G->ApolloTool==1||G->ApolloTool==4||G->ApolloTool==5||G->ApolloTool==6)){ViewElbow=FVector(7,Side*24,-28);ViewWrist=FVector(35,Side*10,-15);}
  if(G&&G->ApolloTool==4)ViewWrist=G->SelectedSample==1?(S?FVector(25,4,-11):FVector(42,-8,-18)):FVector(29,Side*8,-10);
  if(G&&G->ApolloTool==5)ViewWrist=FVector(38,Side*6,S?-20.f:-10.f);
  if(G&&G->ApolloTool==6)ViewWrist=FVector(30,Side*7,-16);
  if(WorkGripActive){ViewWrist=WorkGrip[S]-FVector(5,0,0);ViewElbow=FVector(8,Side*25,-32);}
  FRotator GripRotation;const FRotator* GripFrame=nullptr;
  if(WorkGripActive&&WorkTool&&WorkTool->IsVisible()&&(S==1||(G&&G->ApolloTool==1))){
   GripRotation=(WorkTool->GetRelativeRotation().Quaternion()*FRotator(8,-Side*5,Side*12).Quaternion()).Rotator();
   ViewWrist=WorkGrip[S]-GripRotation.RotateVector(FVector(5,0,0));GripFrame=&GripRotation;
  }
  // Tool-contact palms stay exactly on the tool. Free hands settle from the
  // previous component pose instead of snapping when a tool closes or a turn ends.
  if(!WorkGripActive){
   if(HandMotion)ViewWrist.Y-=TorsoLean*1.1f;
   FRotator FreeRotation(8+WorkBlend*12,-Side*(7+WorkBlend*5),Side*7+(HandMotion?SuitTurnLean*3:0));
   if(D>0){
    const float Response=SuitMotionWeight(D,14);
    ViewElbow=FMath::Lerp(Gloves[S*8]->GetRelativeLocation(),ViewElbow,Response);
    ViewWrist=FMath::Lerp(Gloves[S*8+1]->GetRelativeLocation(),ViewWrist,Response);
    FreeRotation=FQuat::Slerp(Gloves[S*8+1]->GetRelativeRotation().Quaternion(),FreeRotation.Quaternion(),Response).Rotator();
   }
   GripRotation=FreeRotation;GripFrame=&GripRotation;
  }
  Hand(Gloves,S*8,ViewElbow,ViewWrist,Side,WorkBlend,false,GripFrame);
 }
 PoseSuitRig();
}
void ALunarCharacter::SuitReview(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* PC=Cast<APlayerController>(GetController());if(!G||!PC)return;
 const int N=SuitReviewFrame++;
 if(FParse::Param(FCommandLine::Get(),TEXT("ExpeditionMotionReview"))){
  // A bounded synthetic motion sequence, not a mission run or a performance test.
  // Captured at 12 requested images/sec; the runner supplies fixed 60 Hz game time.
  static TWeakObjectPtr<AStaticMeshActor> Floor;
  static FString MotionCsv;
  auto Key=[&](FKey K,EInputEvent E){PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,E,E==IE_Released?0.f:1.f));};
  const FString Dir=FPaths::ProjectDir()/TEXT("Reports/ExpeditionMotion");
  if(N==0){
   IFileManager::Get().MakeDirectory(*Dir,true);
   MotionCsv=TEXT("frame,scene,actor_x_cm,actor_y_cm,actor_z_cm,left_stance,right_stance,left_foot_x_cm,left_foot_y_cm,left_foot_z_cm,right_foot_x_cm,right_foot_y_cm,right_foot_z_cm\n");
   G->StartMission();G->RadioRemaining=0;G->Journal=false;G->Paused=false;G->CancelApolloTool();
   auto* Fixture=GetWorld()->SpawnActor<AStaticMeshActor>();Floor=Fixture;
   Fixture->Tags.Add(TEXT("Expedition.SyntheticMotionFixture"));
   auto* FixtureSurface=Fixture->GetStaticMeshComponent();FixtureSurface->SetMobility(EComponentMobility::Movable);
   FixtureSurface->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
   FixtureSurface->SetCollisionEnabled(ECollisionEnabled::QueryOnly);FixtureSurface->SetCollisionResponseToAllChannels(ECR_Block);FixtureSurface->SetCanEverAffectNavigation(false);
   Fixture->SetActorLocation(FVector(0,0,9912));Fixture->SetActorScale3D(FVector(30,30,.02f));
   SetActorLocation(FVector(0,0,10002));SetActorRotation(FRotator::ZeroRotator);
   GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->SetMovementMode(MOVE_Walking);
   LastStepPosition=GetActorLocation();StepTravel=0;FootPlanted[0]=FootPlanted[1]=false;
   if(ThirdPerson)ToggleSuitView();PC->SetControlRotation(FRotator::ZeroRotator);UpdateSuit(0);
  }
  if(N==24)PC->SetControlRotation(FRotator(-78,0,0));
  if(N==36){ToggleSuitView();PC->SetControlRotation(FRotator(-12,0,0));Key(EKeys::W,IE_Pressed);}
  if(N==60&&Floor.IsValid())Floor->SetActorRotation(FRotator(0,0,12));
  if(N==72)PC->SetControlRotation(FRotator(-12,65,0));
  if(N==84){Key(EKeys::W,IE_Released);SuitOrbit=true;PC->SetControlRotation(FRotator(-10,GetActorRotation().Yaw+150,0));}
  if(N==90)Key(EKeys::SpaceBar,IE_Pressed);
  if(N==94)Key(EKeys::SpaceBar,IE_Released);
  if(N<120){
   const TCHAR* Scene=N<24?TEXT("hands"):N<36?TEXT("lookdown"):N<60?TEXT("flat-walk"):N<84?TEXT("slope-turn"):N<90?TEXT("stop"):TEXT("hop");
   const FVector A=GetActorLocation(),L=SuitBody[6]->GetComponentLocation(),R=SuitBody[9]->GetComponentLocation();
   MotionCsv+=FString::Printf(TEXT("%d,%s,%.4f,%.4f,%.4f,%d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n"),N,Scene,A.X,A.Y,A.Z,int(FootPlanted[0]),int(FootPlanted[1]),L.X,L.Y,L.Z,R.X,R.Y,R.Z);
   FScreenshotRequest::RequestScreenshot(Dir/FString::Printf(TEXT("motion-%03d.png"),N),true,false);
   FTimerHandle Next;GetWorldTimerManager().SetTimer(Next,this,&ALunarCharacter::SuitReview,1.f/12.f,false);return;
  }
  Key(EKeys::W,IE_Released);Key(EKeys::SpaceBar,IE_Released);
  FFileHelper::SaveStringToFile(MotionCsv,*(Dir/TEXT("motion.csv")));
  FFileHelper::SaveStringToFile(TEXT("{\"synthetic\":true,\"requested_frames\":120,\"capture_hz\":12,\"gameplay_fixed_hz\":60,\"fixture\":\"authored flat then 12-degree cross-slope floor\",\"limitations\":\"Visual motion review only; no human comfort or frame-performance claim. Separate fieldwork review covers real tool grips.\"}"),*(Dir/TEXT("manifest.json")));
  UE_LOG(LogTemp,Display,TEXT("EXPEDITION_MOTION_REVIEW 120 requested frames; synthetic flat/slope movement fixture"));
  FTimerHandle End;GetWorldTimerManager().SetTimer(End,[](){FPlatformMisc::RequestExitWithStatus(false,0);},2,false);return;
 }
 if(N==0){G->StartMission();G->RadioRemaining=0;PC->SetControlRotation(FRotator(0,0,0));}
 if(N==1&&HandRig)UE_LOG(LogTemp,Display,TEXT("SUIT_VIEW fov=%.2f wrist=%s hand=%s visible=%d bounds=%s extent=%s"),Camera->FieldOfView,*Gloves[1]->GetRelativeLocation().ToString(),*HandRig->GetBoneLocationByName(TEXT("hand_l"),EBoneSpaces::ComponentSpace).ToString(),HandRig->IsVisible(),*Camera->GetComponentTransform().InverseTransformPosition(HandRig->Bounds.Origin).ToString(),*HandRig->Bounds.BoxExtent.ToString());
 if(N==2)PC->SetControlRotation(FRotator(-78,0,0));
 if(N==4){ToggleSuitView();PC->SetControlRotation(FRotator(-10,25,0));}
 if(N==6){SuitOrbit=true;PC->SetControlRotation(FRotator(-8,GetActorRotation().Yaw+150,0));}
 if(N==8){SuitOrbit=false;ToggleSuitView();auto* T=G->Target();SetActorLocation(T->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator R;GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((T->GetActorLocation()-Eye).Rotation());G->Interact();}
 auto Key=[&](FKey K,EInputEvent E){PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,E,E==IE_Released?0.f:1.f));};
 if(N==10){G->CancelApolloTool();G->RadioRemaining=0;ToggleSuitView();PC->SetControlRotation(FRotator(-12,180,0));Key(EKeys::W,IE_Pressed);}
 if(N==12)Key(EKeys::LeftShift,IE_Pressed);
 if(N==14){Key(EKeys::LeftShift,IE_Released);Key(EKeys::W,IE_Released);Key(EKeys::SpaceBar,IE_Pressed);}
 if(N%2==1){const TCHAR* Names[]={TEXT("suit-hands.png"),TEXT("suit-body.png"),TEXT("suit-third-person.png"),TEXT("suit-front.png"),TEXT("suit-work.png"),TEXT("suit-walk.png"),TEXT("suit-stride.png"),TEXT("suit-hop.png")};FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports")/Names[N/2],true,false);}
 if(N>=16){FPlatformMisc::RequestExit(false);return;}FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitReview,1.4f,false);
}
void ALunarCharacter::SuitChecks(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* PC=Cast<APlayerController>(GetController());if(!G||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto Check=[&](bool Good,const TCHAR* Name){G->MovementReport+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++G->MovementFailures;};
 auto Key=[&](FKey K,EInputEvent E){PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,E,E==IE_Released?0.f:1.f));};
 const int N=SuitReviewFrame++;
 if(N==0){
  G->StartMission();G->MovementReport.Reset();G->MovementFailures=0;PC->SetControlRotation(FRotator(0,0,0));SetActorLocation(GetActorLocation()+FVector(0,-700,0));G->MovementOrigin=GetActorLocation();
  bool Assets=Gloves.Num()==16&&SuitBody.Num()==10&&SuitArms.Num()==18;
  for(const auto& C:Gloves)Assets&=C&&C->GetStaticMesh()!=nullptr;
  for(const auto& C:SuitBody)Assets&=C&&C->GetStaticMesh()!=nullptr;
  Check(Assets,TEXT("Authored suit, gloves, helmet, pack and boots loaded"));
  if(Assets){
   const FBox Sleeve=Gloves[0]->GetStaticMesh()->GetBoundingBox(),Thigh=SuitBody[4]->GetStaticMesh()->GetBoundingBox();
   Check(Sleeve.Min.X>-.7f&&Sleeve.Max.X<24.5f&&Sleeve.GetExtent().Y<7.f&&Sleeve.GetExtent().Z<7.1f,TEXT("Imported first-person sleeve retains bounded tapered garment dimensions"));
   Check(Thigh.Min.X>-.7f&&Thigh.Max.X<36.7f,TEXT("Imported look-down thigh has shallow closures instead of projecting round caps"));
  }
  Key(EKeys::W,IE_Pressed);
 }else if(N==1){
  Check(GetActorLocation().X-G->MovementOrigin.X>85&&GetVelocity().Size2D()>120,TEXT("Forward input accelerates to normal 1.4 m/s stride"));Key(EKeys::LeftShift,IE_Pressed);
 }else if(N==2){
  Check(GetVelocity().Size2D()>200&&FastStride,TEXT("Shift accelerates to bounded fast stride"));Key(EKeys::W,IE_Released);Key(EKeys::LeftShift,IE_Released);Key(EKeys::D,IE_Pressed);G->MovementOrigin=GetActorLocation();
 }else if(N==3){
  Check(GetActorLocation().Y-G->MovementOrigin.Y>75&&!FastStride,TEXT("Strafe follows input and releasing Shift restores walking"));Key(EKeys::D,IE_Released);Key(EKeys::SpaceBar,IE_Pressed);G->MovementOrigin=GetActorLocation();
 }else if(N==4){
  const float Rise=GetActorLocation().Z-G->MovementOrigin.Z;
  Check(Rise>25&&Rise<95&&GetCharacterMovement()->IsFalling()&&FMath::IsNearlyEqual(GetCharacterMovement()->JumpZVelocity,140.f),TEXT("Bounded EVA hop rises under lunar gravity"));Key(EKeys::SpaceBar,IE_Released);ToggleSuitView();
  Check(ThirdPerson&&SuitRig&&SuitRig->IsVisible()&&!HandRig->IsVisible()&&FVector::Dist(Camera->GetComponentLocation(),GetActorLocation())>120,TEXT("V exposes skinned suit and moves camera outside capsule"));
 }else if(N==5){
  G->MovementOrigin=FVector(GaitPhase,G->Oxygen,GetActorLocation().Z);PauseMission();
 }else if(N==6){
  Check(G->Paused&&GaitPhase==float(G->MovementOrigin.X)&&G->Oxygen==float(G->MovementOrigin.Y)&&FMath::IsNearlyEqual(GetActorLocation().Z,G->MovementOrigin.Z,.1),TEXT("Pause freezes gait, movement and EVA clock"));PauseMission();ToggleSuitView();
  Check(!ThirdPerson&&!SuitBody[2]->IsVisible()&&HandRig&&HandRig->IsVisible()&&Camera->GetRelativeLocation().Equals(FVector(0,0,68),.01),TEXT("V restores stable helmet camera and skinned first-person hands"));
 }else if(N==7){
  Check(GetCharacterMovement()->IsMovingOnGround(),TEXT("Hop returns safely to terrain"));
  Check(FootprintCount>2&&Footprints.Num()<=160,TEXT("Visible walking leaves bounded bootprints"));
  FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);FVector Kit=G->ModuleActor->GetActorLocation();
  Check(GetWorld()->SweepSingleByChannel(Hit,Kit+FVector(-200,0,40),Kit+FVector(0,0,40),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(34,88),Q)&&Hit.GetActor()==G->ModuleActor,TEXT("Field kit blocks capsule movement instead of clipping through suit"));
  Key(EKeys::Tab,IE_Pressed);
 }else if(N==8){
  Check(G->Journal&&G->Paused,TEXT("Tab input opens the field journal and pauses play"));Key(EKeys::Tab,IE_Released);Key(EKeys::Escape,IE_Pressed);
 }else if(N==9){
  Check(!G->Journal&&!G->Paused,TEXT("Escape input closes the journal and resumes play"));Key(EKeys::Escape,IE_Released);Key(EKeys::L,IE_Pressed);
 }else{
  Check(G->ApolloObjective().Contains(TEXT("PREPARE"))&&ApolloText::Get(TEXT("MenuHelp")).Contains(TEXT("Controls are introduced as you play.")),TEXT("Former L language shortcut leaves the game in English"));Key(EKeys::L,IE_Released);
  // Isolated collision fixture above the map; never modifies terrain or saves.
  auto* Floor=GetWorld()->SpawnActor<AStaticMeshActor>();auto* FixtureMesh=Floor->GetStaticMeshComponent();
  FixtureMesh->SetMobility(EComponentMobility::Movable);FixtureMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));FixtureMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);FixtureMesh->SetCollisionResponseToAllChannels(ECR_Block);
  Floor->SetActorLocation(FVector(0,0,9912));Floor->SetActorScale3D(FVector(4,4,.02f));
  const FTransform Before=GetActorTransform();SetActorLocation(FVector(0,0,10000));SetActorRotation(FRotator::ZeroRotator);GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->SetMovementMode(MOVE_Walking);GaitBlend=0;FootPlanted[0]=FootPlanted[1]=false;
  UpdateSuit(0);Check(SuitBody[6]->GetUpVector().Equals(FVector::UpVector,.01f)&&SuitBody[9]->GetUpVector().Equals(FVector::UpVector,.01f),TEXT("Boot soles stay level on a flat collision fixture"));
  Floor->SetActorRotation(FRotator(0,0,15));UpdateSuit(0);
  Check(FVector::DotProduct(SuitBody[6]->GetUpVector(),Floor->GetActorUpVector())>.995f&&FVector::DotProduct(SuitBody[9]->GetUpVector(),Floor->GetActorUpVector())>.995f,TEXT("Both boot soles align to a known 15-degree slope"));
  Floor->SetActorRotation(FRotator(12,0,15));UpdateSuit(0);
  Check(FVector::DotProduct(SuitBody[6]->GetUpVector(),Floor->GetActorUpVector())>.99f&&FVector::DotProduct(SuitBody[9]->GetUpVector(),Floor->GetActorUpVector())>.99f,TEXT("Boot alignment follows combined uphill and side slopes"));
  Floor->Destroy();SetActorTransform(Before);
  // Validate the rendered skeleton, not only the hidden static pose drivers.
  G->Holding=false;G->ApolloTool=0;WorkBlend=0;UpdateSuit(0);
  auto Bone=[&](const TCHAR* Name){return HandRig->GetBoneLocationByName(Name,EBoneSpaces::ComponentSpace);};
  Check(FMath::IsNearlyEqual(FVector::Dist(Bone(TEXT("finger1_r")),Bone(TEXT("tip1_r"))),4.f,.05f)&&FMath::IsNearlyEqual(FVector::Dist(Bone(TEXT("finger3_r")),Bone(TEXT("tip3_r"))),2.96f,.05f),TEXT("Rendered middle and little fingers retain different authored joint lengths"));
  HandRig->UpdateBounds();
  Check(HandRig->Bounds.GetBox().IsInside(HandRig->GetBoneLocationByName(TEXT("tip1_l"),EBoneSpaces::WorldSpace))&&HandRig->Bounds.GetBox().IsInside(HandRig->GetBoneLocationByName(TEXT("tip1_r"),EBoneSpaces::WorldSpace)),TEXT("Cooked viewmodel bounds contain both posed gloves"));
  const FRotator Relaxed=HandRig->GetBoneRotationByName(TEXT("tip1_r"),EBoneSpaces::ComponentSpace);
  G->Holding=true;WorkBlend=1;UpdateSuit(0);
  Check(!HandRig->GetBoneRotationByName(TEXT("tip1_r"),EBoneSpaces::ComponentSpace).Equals(Relaxed,10.f),TEXT("Gripping bends the visible distal finger instead of leaving an open hand"));
  G->Paused=true;const float FrozenGrip=WorkBlend;
  const FTransform FrozenCamera=Camera->GetComponentTransform();const FVector FrozenWrist=Gloves[9]->GetRelativeLocation();
  UpdateSuit(.5f);
  Check(WorkBlend==FrozenGrip,TEXT("Pause preserves the current glove grip pose"));
  Check(Camera->GetComponentTransform().Equals(FrozenCamera,.001f)&&Gloves[9]->GetRelativeLocation().Equals(FrozenWrist,.001f),TEXT("Pause freezes camera follow and the visible wrist pose"));
  G->Paused=false;G->Holding=false;WorkBlend=0;UpdateSuit(0);
  auto ResponseOverSecond=[](int Frames){float Value=0;for(int I=0;I<Frames;++I)Value=FMath::Lerp(Value,1.f,SuitMotionWeight(1.f/Frames,7));return Value;};
  Check(FMath::Abs(ResponseOverSecond(30)-ResponseOverSecond(60))<.0001f,TEXT("Presentation smoothing has matching 30 and 60 Hz responses"));
  Check(SuitMotionWeight(-1,7)==0&&SuitMotionWeight(std::numeric_limits<float>::infinity(),7)==0,TEXT("Invalid animation deltas cannot poison presentation filters"));
  const bool HadHandMotion=ULunarSettings::Get()->CameraMotion;ULunarSettings::Get()->CameraMotion=false;
  LandBlend=0;UpdateSuit(0);const FVector StableWrist=Gloves[9]->GetRelativeLocation();
  LandBlend=2.2f;UpdateSuit(0);
  Check(Gloves[9]->GetRelativeLocation().Equals(StableWrist,.001f),TEXT("Hand motion Off suppresses landing hand bounce"));
  ULunarSettings::Get()->CameraMotion=HadHandMotion;LandBlend=0;
  const FVector Stationary=GetActorLocation();GetCharacterMovement()->StopMovementImmediately();PC->SetControlRotation(GetActorRotation());
  SuitStrideSpeed=140;SuitTurnLean=0;GaitBlend=1;
  for(int I=0;I<10;++I)UpdateSuit(.1f);
  Check(SuitStrideSpeed<.2f&&GaitBlend<.002f&&GetActorLocation().Equals(Stationary,.001f),TEXT("Stopped gait settles without moving the gameplay capsule"));
  G->ApolloTool=0;UpdateSuit(0);Check(SuitBody[4]->GetRelativeLocation().X< -40&&!SuitBody[1]->IsVisible()&&!SuitBody[0]->IsVisible()&&!SuitBody[2]->IsVisible(),TEXT("Walking view keeps trouser closures behind the helmet camera without exposing torso caps"));
  G->ApolloTool=4;UpdateSuit(0);Check(!SuitBody[1]->IsVisible()&&FMath::Abs(SuitBody[4]->GetRelativeLocation().X)<1,TEXT("Close tool view restores physical hip drivers and keeps the waist hidden"));G->ApolloTool=0;
  PhotoCaptureFrames=2;UpdateSuit(0);Check(!HandRig->IsVisible(),TEXT("Evidence photography still excludes first-person gloves"));
  PhotoCaptureFrames=0;UpdateSuit(0);Check(HandRig->IsVisible(),TEXT("First-person gloves return after evidence photography"));

  G->MovementReport+=FString::Printf(TEXT("RESULT: %d failures\n"),G->MovementFailures);
  FFileHelper::SaveStringToFile(G->MovementReport,*(FPaths::ProjectDir()/TEXT("Reports/suit-checks.txt")));UE_LOG(LogTemp,Display,TEXT("SUIT_CHECKS\n%s"),*G->MovementReport);FPlatformMisc::RequestExitWithStatus(false,G->MovementFailures?1:0);return;
 }
 FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitChecks,1,false);
}
