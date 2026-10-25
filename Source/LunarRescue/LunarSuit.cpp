#include "LunarGame.h"
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

namespace {
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
void Hand(const TArray<TObjectPtr<UStaticMeshComponent>>& Parts,int Base,FVector Elbow,FVector Wrist,float Side,float Work,bool Upper=false){
 int N=Base;if(Upper)++N;
 Limb(Parts[N++],Elbow,Wrist,24);
 const FRotator R(8+Work*12,-Side*(7+Work*5),Side*7);const FTransform Frame(R,Wrist);
 Parts[N]->SetRelativeLocation(Wrist);Parts[N++]->SetRelativeRotation(R);
 Parts[N]->SetRelativeLocation(Frame.TransformPosition(FVector(1,0,0)));Parts[N++]->SetRelativeRotation(R);
 const float Lengths[]={.95f,1.f,.94f,.76f};
 for(int F=0;F<4;++F){
  auto* C=Parts[N++].Get();C->SetRelativeLocation(Frame.TransformPosition(FVector(9,(-3.15f+F*2.1f)*Side,.1f)));
  C->SetRelativeRotation((R.Quaternion()*FRotator(-5-Work*28,(F-1.5f)*Side*3,0).Quaternion()).Rotator());C->SetRelativeScale3D(FVector(Lengths[F],1,1));
 }
 Parts[N]->SetRelativeLocation(Frame.TransformPosition(FVector(4,-Side*4,-1)));
 Parts[N]->SetRelativeRotation((R.Quaternion()*FRotator(-18-Work*22,-Side*52,0).Quaternion()).Rotator());Parts[N]->SetRelativeScale3D(FVector(.76f,1.1f,1.1f));
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
 if(FParse::Param(FCommandLine::Get(),TEXT("SuitReview"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitReview,5,false);}
 if(FParse::Param(FCommandLine::Get(),TEXT("SuitTest"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitChecks,4,false);}
}
void ALunarCharacter::BeginFastStride(){auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));if(ActivePanel||!G||G->Paused||G->ApolloTool)return;FastStride=ULunarSettings::Get()->ToggleSprint?!FastStride:true;}
void ALunarCharacter::EndFastStride(){if(!ULunarSettings::Get()->ToggleSprint)FastStride=false;}
void ALunarCharacter::BeginSuitOrbit(){SuitOrbit=true;}
void ALunarCharacter::EndSuitOrbit(){SuitOrbit=false;}
void ALunarCharacter::ToggleSuitView(){
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
 if(!Paused){
  const float Speed=GetVelocity().Size2D();
  const float TurnAngle=FMath::Abs(FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,GetControlRotation().Yaw));
  const bool Turning=Active&&Speed<15&&TurnAngle>5&&!(ThirdPerson&&SuitOrbit);
  const float StrideSpeed=FMath::Max(Speed,Turning?FMath::Min(140.f,TurnAngle*6.f):0.f);
  GaitBlend=FMath::FInterpTo(GaitBlend,Move->IsMovingOnGround()?FMath::Clamp(StrideSpeed/140.f,0.f,1.f):0.f,D,7);
  GaitPhase+=StrideSpeed*D/(FastStride?160.f:128.f)*2*PI;
  const bool PhysicalTool=G&&(G->ApolloTool==1||G->ApolloTool==4||G->ApolloTool==5||G->ApolloTool==6);
  WorkBlend=FMath::FInterpTo(WorkBlend,G&&(G->Holding||PhysicalTool)?1.f:0.f,D,8);LandBlend=FMath::FInterpTo(LandBlend,0.f,D,5);
  if(!(ThirdPerson&&SuitOrbit))SetActorRotation(FMath::RInterpTo(GetActorRotation(),FRotator(0,GetControlRotation().Yaw,0),D,10));
 }
 const bool Show=G&&!G->Menu&&(Active||G->Stage==ELunarStage::Briefing||G->Stage==ELunarStage::Won);
 for(const auto& C:SuitBody)C->SetVisibility(Show);SuitBody[2]->SetVisibility(Show&&ThirdPerson);
 SuitBody[0]->SetVisibility(Show&&ThirdPerson,true);SuitBody[3]->SetVisibility(Show&&ThirdPerson);
 SuitBody[1]->SetVisibility(Show&&ThirdPerson);
 for(const auto& C:SuitArms)C->SetVisibility(Show&&ThirdPerson);
 for(const auto& C:Gloves)C->SetVisibility(Show&&!ThirdPerson&&Camera->FieldOfView>60);
 const float Breath=FMath::Sin(SuitTime*1.5f)*.18f;
 const float Crouch=GaitBlend*((FastStride?6.f:3.f)+FMath::Cos(GaitPhase*2)*(FastStride?1.8f:1.f));
 SuitBody[0]->SetRelativeLocation(FVector(0,0,Breath-Crouch));SuitBody[1]->SetRelativeLocation(FVector(0,0,-Crouch));
 SuitBody[2]->SetRelativeLocation(FVector(0,0,64+Breath-Crouch));SuitBody[3]->SetRelativeLocation(FVector(0,0,Breath-Crouch));
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
 UpdateApolloWorkVisuals();
 for(int S=0;S<2;++S){
  const float Side=S?1.f:-1.f;const float Phase=GaitPhase+S*PI;const float Swing=FMath::Sin(Phase)*GaitBlend;
  const float Cycle=FMath::Fmod(Phase,2*PI)/(2*PI),HalfStride=FastStride?40.f:32.f;
  const float T=Cycle<.5f?Cycle*2:(Cycle-.5f)*2;
  const float Plant=Cycle<.5f?HalfStride*(-1+2*T*T*(3-2*T)):HalfStride*(1-2*T);
  FVector Hip(0,Side*12,-12-Crouch),Foot=FVector(Direction.X,Direction.Y,0)*Plant*GaitBlend+FVector(3,Side*12,-78);
  float Lift=Cycle<.5f?FMath::Sin(T*PI)*16*GaitBlend:0.f;
  if(Move->IsFalling()){Foot.X+=12;Foot.Z+=16;Lift=0;}
  else {FVector W=GetActorTransform().TransformPosition(Foot);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
   if(GetWorld()->LineTraceSingleByChannel(Hit,W+FVector(0,0,65),W-FVector(0,0,90),ECC_Visibility,Q))Foot.Z=FMath::Clamp(GetActorTransform().InverseTransformPosition(Hit.ImpactPoint).Z+11,-90.f,-65.f);
  }
  // A planted foot stays in world space for the stance half of the cycle.
  const bool Stance=Cycle>=.5f&&Move->IsMovingOnGround()&&GaitBlend>.15f;
  if(Stance){if(!FootPlanted[S])PlantedFeet[S]=GetActorTransform().TransformPosition(Foot);const FVector Locked=GetActorTransform().InverseTransformPosition(PlantedFeet[S]);if(FVector::Dist2D(Locked,Foot)<65){Foot.X=Locked.X;Foot.Y=Locked.Y;}else PlantedFeet[S]=GetActorTransform().TransformPosition(Foot);}
  FootPlanted[S]=Stance;
  Foot.Z+=Lift;FVector Knee=Joint(Hip,Foot,36,34,FVector(1,0,0));int B=4+S*3;
  Limb(SuitBody[B],Hip,Knee,36);Limb(SuitBody[B+1],Knee,Foot,34);SuitBody[B+2]->SetRelativeLocation(Foot);SuitBody[B+2]->SetRelativeRotation(FRotator(Swing*7,0,0));
  FVector Shoulder(0,Side*25,40-Crouch),Wrist(12-Swing*8,Side*(28-WorkBlend*13),-10+WorkBlend*30-Crouch);
  Wrist.X+=WorkBlend*28;if(Move->IsFalling())Wrist.Z+=12;
  FVector Elbow=Joint(Shoulder,Wrist,28,24,FVector(0,Side,-.4f));Limb(SuitArms[S*9],Shoulder,Elbow,28);
  Hand(SuitArms,S*9,Elbow,Wrist,Side,WorkBlend,true);
  const float Sway=ULunarSettings::Get()->CameraMotion?FMath::Sin(Phase)*1.1f*GaitBlend:0.f;
  FVector ViewElbow(8,Side*25,-32),ViewWrist(36+WorkBlend*3+Sway,Side*(22-WorkBlend*12),-30+WorkBlend*15+FMath::Abs(Sway)*.35f-LandBlend);
  if(G&&(G->ApolloTool==1||G->ApolloTool==4||G->ApolloTool==5||G->ApolloTool==6)){ViewElbow=FVector(7,Side*24,-28);ViewWrist=FVector(35,Side*10,-15);}
  if(G&&G->ApolloTool==4)ViewWrist=G->SelectedSample==1?(S?FVector(25,4,-11):FVector(42,-8,-18)):FVector(29,Side*8,-10);
  if(G&&G->ApolloTool==5)ViewWrist=FVector(38,Side*6,S?-20.f:-10.f);
  if(G&&G->ApolloTool==6)ViewWrist=FVector(30,Side*7,-16);
  if(WorkGripActive){ViewWrist=WorkGrip[S]-FVector(5,0,0);ViewElbow=FVector(8,Side*25,-32);}
  Hand(Gloves,S*8,ViewElbow,ViewWrist,Side,WorkBlend);
 }
 PoseSuitRig();
}
void ALunarCharacter::SuitReview(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* PC=Cast<APlayerController>(GetController());if(!G||!PC)return;
 const int N=SuitReviewFrame++;
 if(N==0){G->StartMission();G->RadioRemaining=0;PC->SetControlRotation(FRotator(0,0,0));}
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
  Check(Assets,TEXT("Authored suit, gloves, helmet, pack and boots loaded"));Key(EKeys::W,IE_Pressed);
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
  G->MovementReport+=FString::Printf(TEXT("RESULT: %d failures\n"),G->MovementFailures);
  FFileHelper::SaveStringToFile(G->MovementReport,*(FPaths::ProjectDir()/TEXT("Reports/suit-checks.txt")));UE_LOG(LogTemp,Display,TEXT("SUIT_CHECKS\n%s"),*G->MovementReport);FPlatformMisc::RequestExitWithStatus(false,G->MovementFailures?1:0);return;
 }
 FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarCharacter::SuitChecks,1,false);
}
