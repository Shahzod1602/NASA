#include "LunarGame.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "InputKeyEventArgs.h"
#include "UnrealClient.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"
#include "Engine/StaticMesh.h"
#include "HAL/PlatformMemory.h"

ALunarCharacter::ALunarCharacter() {
 PrimaryActorTick.bCanEverTick=true;
 GetCapsuleComponent()->InitCapsuleSize(34,88);
 Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("HelmetCamera"));
 Camera->SetupAttachment(GetCapsuleComponent()); Camera->SetRelativeLocation(FVector(0,0,68));
 Camera->bUsePawnControlRotation=true; Camera->FieldOfView=85;
 GetCharacterMovement()->MaxWalkSpeed=320;
 GetCharacterMovement()->JumpZVelocity=230;
 GetCharacterMovement()->AirControl=0.25;
 GetCharacterMovement()->GravityScale=1;
 GetCharacterMovement()->BrakingDecelerationWalking=700;
 GetCharacterMovement()->MaxStepHeight=45;
}
static ALunarGameMode* Mission(const UObject* O) { return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(O)); }
void ALunarCharacter::BeginPlay() {
 Super::BeginPlay(); LastStepPosition=GetActorLocation();
 BootMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Immersion/Materials/M_Bootprint.M_Bootprint"));
 for(int I=0;I<3;++I) StepSounds.Add(LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Immersion/Audio/SuitStep%d.SuitStep%d"),I,I)));
 auto* Breath=LoadObject<USoundBase>(nullptr,TEXT("/Game/Lunar/Immersion/Audio/SuitBreath.SuitBreath"));
 if(Breath) { BreathAudio=UGameplayStatics::CreateSound2D(this,Breath,.5f,1,0,nullptr,false,false); }
 BuildGloves();
 CarriedModule=NewObject<UStaticMeshComponent>(this);
 CarriedModule->SetupAttachment(Camera);
 CarriedModule->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell")));
 CarriedModule->SetRelativeLocation(FVector(52,0,-33));CarriedModule->SetRelativeScale3D(FVector(.23));
 CarriedModule->SetCollisionEnabled(ECollisionEnabled::NoCollision);CarriedModule->SetCastShadow(false);
 CarriedModule->SetOnlyOwnerSee(true);CarriedModule->RegisterComponent();CarriedModule->SetVisibility(false);
}
void ALunarCharacter::BuildGloves() {
 auto* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
 const TCHAR* Names[]={TEXT("M_SuitCloth"),TEXT("M_SuitJoint"),TEXT("M_SuitTrim")};
 UMaterialInterface* Mats[3];
 for(int I=0;I<3;++I) Mats[I]=LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Immersion/Materials/%s.%s"),Names[I],Names[I]));
 auto Part=[&](FVector Location,FVector Scale,int Mat) {
  Location.X+=5; Location.Y*=.9f; Location.Z-=2; Scale*=.76f;
  auto* C=NewObject<UStaticMeshComponent>(this);C->SetupAttachment(Camera);C->SetStaticMesh(Sphere);C->SetMaterial(0,Mats[Mat]);
  C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(false);C->SetOnlyOwnerSee(true);C->SetReceivesDecals(false);
  C->SetRelativeLocation(Location);C->SetRelativeScale3D(Scale);C->RegisterComponent();Gloves.Add(C);GloveRest.Add(Location);
 };
 for(float Side:{-1.f,1.f}) {
  Part(FVector(22,Side*22,-23),FVector(.30,.105,.12),0);
  Part(FVector(32,Side*21,-22),FVector(.065,.12,.125),1);
  Part(FVector(34,Side*21,-22),FVector(.025,.123,.126),2);
  Part(FVector(39,Side*20,-21),FVector(.13,.11,.085),0);
  for(int F=0;F<4;++F) Part(FVector(44.5,Side*(16.5+F*2.1),-20.5-F*.2),FVector(.075,.022,.032),0);
  Part(FVector(39,Side*14.8,-23),FVector(.075,.033,.037),1);
 }
}
void ALunarCharacter::Tick(float D) {
 Super::Tick(D);auto* G=Mission(this);const bool Active=G&&G->IsActive()&&!G->Paused;
 if(BreathAudio) {
  if(Active) { if(!BreathAudio->IsPlaying()) BreathAudio->Play();BreathAudio->SetPaused(false);BreathAudio->SetPitchMultiplier(G->Oxygen<60?1.18f:1.f); }
  else BreathAudio->SetPaused(true);
 }
 if(StepAudio)StepAudio->SetPaused(!Active);
 const FVector Here=GetActorLocation();float Travel=FVector::Dist2D(Here,LastStepPosition);LastStepPosition=Here;
 if(Active) {
  SuitTime+=D;
  if(GetCharacterMovement()->IsMovingOnGround()&&Travel<100) { StepTravel+=Travel;if(StepTravel>=125) {StepTravel=0;StampFootstep();} }
  if(GrainLife>0) {
   GrainLife-=D;
   for(int I=0;I<LandingGrains.Num();++I) {
    auto* Grain=LandingGrains[I].Get();if(!Grain||!Grain->IsVisible())continue;
    FVector Before=Grain->GetComponentLocation();GrainVelocity[I].Z-=162.f*D;
    FVector After=Before+GrainVelocity[I]*D;FHitResult Surface;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
    if(GrainLife<=0||GetWorld()->LineTraceSingleByChannel(Surface,Before,After,ECC_Visibility,Q))Grain->SetVisibility(false);
    else Grain->SetWorldLocation(After);
   }
  }
 }
 if(CarriedModule)CarriedModule->SetVisibility(G&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==0&&Camera->FieldOfView>60);
 // A few millimetres of hand sway; camera and aiming remain stable.
 for(int I=0;I<Gloves.Num();++I) if(auto* C=Gloves[I].Get()) {
  C->SetVisibility(G&&G->IsActive()&&Camera->FieldOfView>60);
  float Sway=FMath::Sin(SuitTime*4.5f+(GloveRest[I].Y>0?PI:0))*.4f*FMath::Clamp(GetVelocity().Size2D()/320.f,0.f,1.f);
  const bool Carry=G&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==0;
  const float Reach=G&&G->Holding?FMath::Clamp(G->HoldProgress/.3f,0.f,1.f):0.f;
  C->SetRelativeLocation(GloveRest[I]+FVector(Reach*7,Carry?-GloveRest[I].Y*.15f:0,Sway+(Carry?4.f:Reach*3)));
 }
}
void ALunarCharacter::StampFootstep() {
 LeftFoot=!LeftFoot;
 if(StepSounds.Num()==3&&StepSounds[FootprintCount%3]) StepAudio=UGameplayStatics::SpawnSound2D(this,StepSounds[FootprintCount%3],.7f,FMath::FRandRange(.94f,1.06f));
 const FRotator Yaw(0,GetControlRotation().Yaw,0);
 FVector P=GetActorLocation()+FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y)*(LeftFoot?-12.f:12.f);
 FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);Q.bTraceComplex=true;
 if(!GetWorld()->LineTraceSingleByChannel(Hit,P,P-FVector(0,0,180),ECC_Visibility,Q)||!BootMaterial)return;
 auto* SurfaceMesh=Cast<UStaticMeshComponent>(Hit.GetComponent());
 if(!SurfaceMesh||!SurfaceMesh->GetStaticMesh()||!SurfaceMesh->GetStaticMesh()->GetName().Contains(TEXT("LunarTerrain")))return;
 const FVector Forward=FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
 const FRotator Rotation=FRotationMatrix::MakeFromXZ(-Hit.ImpactNormal,Forward).Rotator();
 auto* Mark=UGameplayStatics::SpawnDecalAtLocation(this,BootMaterial,FVector(5,8,17),Hit.ImpactPoint+Hit.ImpactNormal*.7f,Rotation,0);
 if(Mark) { Mark->SetFadeScreenSize(.0001f);Footprints.Add(Mark);++FootprintCount; }
 if(Footprints.Num()>160) {if(Footprints[0])Footprints[0]->DestroyComponent();Footprints.RemoveAt(0);}
}
void ALunarCharacter::Landed(const FHitResult& Hit) {
 Super::Landed(Hit);auto* G=Mission(this);if(!G||!G->IsActive()||G->Paused)return;
 StampFootstep();StepTravel=0;
 auto* Ground=Cast<UStaticMeshComponent>(Hit.GetComponent());
 if(!Ground||!Ground->GetStaticMesh()||!Ground->GetStaticMesh()->GetName().Contains(TEXT("LunarTerrain")))return;
 if(LandingGrains.IsEmpty()) for(int I=0;I<16;++I) {
  auto* Grain=NewObject<UStaticMeshComponent>(this);
  Grain->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));
  Grain->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/V3/Materials/M_LunarRegolith.M_LunarRegolith")));
  Grain->SetCollisionEnabled(ECollisionEnabled::NoCollision);Grain->SetCastShadow(false);Grain->SetReceivesDecals(false);Grain->RegisterComponent();
  LandingGrains.Add(Grain);GrainVelocity.Add(FVector::ZeroVector);
 }
 GrainLife=1.4f;
 for(int I=0;I<LandingGrains.Num();++I) {
  float Angle=I*2*PI/LandingGrains.Num();FVector Radial(FMath::Cos(Angle),FMath::Sin(Angle),0);
  LandingGrains[I]->SetWorldLocation(Hit.ImpactPoint+Radial*12+FVector(0,0,3));
  LandingGrains[I]->SetWorldScale3D(FVector(FMath::FRandRange(.012f,.025f)));LandingGrains[I]->SetVisibility(true);
  GrainVelocity[I]=Radial*FMath::FRandRange(35.f,65.f)+FVector(0,0,FMath::FRandRange(40.f,75.f));
 }
}
void ALunarCharacter::SetupPlayerInputComponent(UInputComponent* I) {
 Super::SetupPlayerInputComponent(I);
 I->BindAxis("Forward",this,&ALunarCharacter::Forward); I->BindAxis("Right",this,&ALunarCharacter::Right);
 I->BindAxis("Turn",this,&ALunarCharacter::Turn); I->BindAxis("Look",this,&ALunarCharacter::Look);
 I->BindAction("Jump",IE_Pressed,this,&ALunarCharacter::Leap);
 I->BindAction("Jump",IE_Released,this,&ACharacter::StopJumping);
 I->BindAction("Interact",IE_Pressed,this,&ALunarCharacter::Interact);
 I->BindAction("Interact",IE_Released,this,&ALunarCharacter::ReleaseInteract);
 I->BindAction("Route",IE_Pressed,this,&ALunarCharacter::ToggleRoute);
 I->BindAction("Start",IE_Pressed,this,&ALunarCharacter::StartMission);
 I->BindAction("Restart",IE_Pressed,this,&ALunarCharacter::RestartMission);
 I->BindAction("Pause",IE_Pressed,this,&ALunarCharacter::PauseMission);
 I->BindAction("Zoom",IE_Pressed,this,&ALunarCharacter::ZoomVisor);
}
void ALunarCharacter::Forward(float V) { auto* G=Mission(this); if(G&&G->IsActive()&&!G->Paused) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V); }
void ALunarCharacter::Right(float V) { auto* G=Mission(this); if(G&&G->IsActive()&&!G->Paused) AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V); }
void ALunarCharacter::Turn(float V) { auto* G=Mission(this); if(G&&!G->Paused) AddControllerYawInput(V*0.65f*Camera->FieldOfView/85.f); }
void ALunarCharacter::Look(float V) { auto* G=Mission(this); if(G&&!G->Paused) AddControllerPitchInput(V*0.65f*Camera->FieldOfView/85.f); }
void ALunarCharacter::ZoomVisor() { auto* G=Mission(this);if(G&&G->IsActive()&&!G->Paused) Camera->SetFieldOfView(Camera->FieldOfView<60?85.f:18.f); }
void ALunarCharacter::Leap() { auto* G=Mission(this); if(G&&G->IsActive()&&!G->Paused) Jump(); }
void ALunarCharacter::Interact() { if(auto* G=Mission(this)) G->Interact(); }
void ALunarCharacter::ReleaseInteract() {if(auto* G=Mission(this))G->ReleaseInteract();}
void ALunarCharacter::ToggleRoute() {if(auto* G=Mission(this))G->ToggleRoute();}
void ALunarCharacter::StartMission() {
 if(auto* G=Mission(this)) {
  if(G->Paused) PauseMission();
  else if(G->Stage==ELunarStage::Won||G->Stage==ELunarStage::Lost) RestartMission();
  else G->StartMission();
 }
}
// Keep ACharacter::Restart intact: possession calls it to enable walking physics.
void ALunarCharacter::RestartMission() { auto* G=Mission(this); if(G&&(G->Stage==ELunarStage::Won||G->Stage==ELunarStage::Lost)) UGameplayStatics::OpenLevel(this,FName("MoonBase")); }
void ALunarCharacter::PauseMission() {
 if(auto* G=Mission(this)) {
  if(!G->IsActive())return;
  auto* Move=GetCharacterMovement();
  G->Paused=!G->Paused; G->ReleaseInteract();
  G->UpdateAudioPause();
  if(G->Paused) { PausedVelocity=Move->Velocity; Move->StopMovementImmediately();Move->SetComponentTickEnabled(false); }
  else { Move->SetComponentTickEnabled(true);Move->Velocity=PausedVelocity; }
 }
}

ALunarGameMode::ALunarGameMode() { DefaultPawnClass=ALunarCharacter::StaticClass(); HUDClass=ALunarHUD::StaticClass(); PrimaryActorTick.bCanEverTick=true; }
void ALunarGameMode::BeginPlay() {
 Super::BeginPlay(); Oxygen=OxygenCapacity;
 PowerBulbs.SetNum(4);PowerLights.SetNum(4);
 for(TActorIterator<AActor> It(GetWorld());It;++It) {
  if(It->ActorHasTag("Module")) ModuleActor=*It;
  if(It->ActorHasTag("Power")) PowerActor=*It;
  if(It->ActorHasTag("Data")) DataActor=*It;
  if(It->ActorHasTag("Home")) HomeActor=*It;
  if(It->ActorHasTag("PowerAntenna")) {PowerAntenna=*It;AntennaStartRotation=It->GetActorRotation();if(auto* Root=It->GetRootComponent())Root->SetMobility(EComponentMobility::Movable);}
  if(It->ActorHasTag("PowerScreen")) {PowerScreen=*It;It->SetActorHiddenInGame(true);}
  if(It->ActorHasTag("PowerWarningBulb")) PowerWarningBulb=*It;
  if(It->ActorHasTag("PowerWarningLight")) {TArray<UPointLightComponent*> Lights;It->GetComponents(Lights);if(Lights.Num())PowerWarningLight=Lights[0];}
  if(It->ActorHasTag("StationLight")) {TArray<UPointLightComponent*> Lights;It->GetComponents(Lights);for(auto* L:Lights)L->SetIntensity(0);}
  for(int I=0;I<4;++I) {
   if(It->ActorHasTag(FName(*FString::Printf(TEXT("PowerBulb%d"),I)))) {PowerBulbs[I]=*It;It->SetActorHiddenInGame(true);}
   if(It->ActorHasTag(FName(*FString::Printf(TEXT("PowerLight%d"),I)))) {TArray<UPointLightComponent*> Lights;It->GetComponents(Lights);if(Lights.Num()){PowerLights[I]=Lights[0];Lights[0]->SetIntensity(0);Lights[0]->SetAttenuationRadius(650);}}
  }
 }
 if(auto* PC=UGameplayStatics::GetPlayerController(this,0)) { PC->SetInputMode(FInputModeGameOnly()); PC->bShowMouseCursor=false; }
 BuildStoryProps();
 WalkthroughEnabled=FParse::Param(FCommandLine::Get(),TEXT("LunarWalkthroughTest"));
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarLedgeTest"))) {FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::LedgeCheck,3,false);}
 Message("SIGNAL LOST. SELENE, do you copy? There should be someone here.");
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarStoryReview"))) {FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::StoryReview,8,false);}
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarTest"))) { FTimerHandle H; GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::RunChecks,2,false); }
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarMovementTest"))) { FTimerHandle H; GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::MovementCheck,2,false); }
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarReview"))) { FTimerHandle H; GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ReviewVisuals,10,false); }
 if(FParse::Param(FCommandLine::Get(),TEXT("LunarPowerReview"))) { FTimerHandle H; GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::PowerReview,10,false); }
}
bool ALunarGameMode::IsActive() const { return Stage>=ELunarStage::FindModule&&Stage<=ELunarStage::ReturnHome; }
void ALunarGameMode::Tick(float D) {
 Super::Tick(D); AdvanceRadio(D); UpdateAudioPause(); AdvanceTime(D); AdvancePowerSequence(D); AdvanceInteraction(D);
 if(WalkthroughEnabled)WalkthroughTick(D);
}
void ALunarGameMode::AdvanceRadio(float D) {
 if(!Paused)RadioRemaining=FMath::Max(0.f,RadioRemaining-FMath::Max(0.f,D));
}
void ALunarGameMode::UpdateAudioPause() {
 if(IsValid(RadioAudio))RadioAudio->SetPaused(Paused);
 PowerSounds.RemoveAll([](const TObjectPtr<UAudioComponent>& C){return !IsValid(C.Get());});
 for(const auto& C:PowerSounds)C->SetPaused(Paused);
}
UAudioComponent* ALunarGameMode::PlayPowerSound(const TCHAR* Name,float Volume) {
 auto* Sound=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Immersion/Audio/%s.%s"),Name,Name));
 auto* C=Sound?UGameplayStatics::SpawnSound2D(this,Sound,Volume):nullptr;
 if(C){PowerSounds.Add(C);C->SetPaused(Paused);}return C;
}
void ALunarGameMode::BeginPowerSequence() {
 if(PowerSequenceActive||StationRebootComplete)return;
 PowerSequenceSeconds=0;PowerSequenceActive=true;
 Message("LINK ESTABLISHED. Station restart in progress.");
 PlayPowerSound(TEXT("PowerContact"),.55f);PowerAudio=PlayPowerSound(TEXT("PowerSequence"),.27f);
}
void ALunarGameMode::AdvancePowerSequence(float D) {
 if(Paused||!IsActive())return;
 if(PowerWarningLight&&!StationRebootComplete) {
  const float Fade=PowerSequenceActive?1-FMath::Clamp(PowerSequenceSeconds/4.2f,0.f,1.f):1.f;
  PowerWarningLight->SetIntensity(180.f*Fade*(.55f+.45f*FMath::Sin(GetWorld()->GetTimeSeconds()*5.5f)));
 }
 if(!PowerSequenceActive)return;
 PowerSequenceSeconds+=FMath::Max(0.f,D);
 for(int I=0;I<4;++I) {
  const float P=FMath::Clamp((PowerSequenceSeconds-(.45f+I*.65f))/.42f,0.f,1.f);
  if(PowerLights[I])PowerLights[I]->SetIntensity(180.f*P);
  if(PowerBulbs[I])PowerBulbs[I]->SetActorHiddenInGame(P<.1f);
 }
 if(PowerAntenna) {
  const float P=FMath::Clamp((PowerSequenceSeconds-1.35f)/2.45f,0.f,1.f);
  const float Smooth=P*P*(3.f-2.f*P);
  PowerAntenna->SetActorRotation(AntennaStartRotation+FRotator(-34.f*Smooth,68.f*Smooth,0));
 }
 if(PowerSequenceSeconds<4.2f)return;
 PowerSequenceActive=false;StationRebootComplete=true;
 if(PowerScreen)PowerScreen->SetActorHiddenInGame(false);
 if(PowerWarningBulb)PowerWarningBulb->SetActorHiddenInGame(true);
 if(PowerWarningLight)PowerWarningLight->SetIntensity(0);
 for(TActorIterator<AActor> It(GetWorld());It;++It) if(It->ActorHasTag("StationLight")) {TArray<UPointLightComponent*> Lights;It->GetComponents(Lights);for(auto* L:Lights){L->SetIntensity(300);L->SetAttenuationRadius(650);}}
 if(PowerAudio)PowerAudio->Stop();
 PowerAudio=PlayPowerSound(TEXT("PowerReady"),.4f);
 Message("POWER RESTORED. Crew log: The rover did not fail. We shut it down.");
}
void ALunarGameMode::AdvanceTime(float D) {
 if(!IsActive()||Paused) return;
 Oxygen=FMath::Max(0.f,Oxygen-FMath::Max(0.f,D));
 if(Oxygen>0&&Oxygen<60&&!LowOxygenWarned) {LowOxygenWarned=true;Message("CAUTION. One minute of oxygen. Return to the lander.");}
 auto* P=UGameplayStatics::GetPlayerPawn(this,0);
 if(Oxygen<=0||(P&&P->GetActorLocation().Z < -1500)) {
  FailureReason=Oxygen<=0?TEXT("Your oxygen ran out. Follow the objective markers next time."):TEXT("You fell beyond the mission area. Stay on the lunar surface.");
  Stage=ELunarStage::Lost;ReleaseInteract();Message(FailureReason);
 }
}
void ALunarGameMode::StartMission() { if(Stage==ELunarStage::Briefing) { Stage=ELunarStage::FindModule; Message("NAV: Find the rover power cell. SELENE has stopped responding."); } }
AActor* ALunarGameMode::Target() const {
 if(PowerSequenceActive)return nullptr;
 switch(Stage) { case ELunarStage::FindModule:return ModuleActor; case ELunarStage::RestorePower:return PowerActor; case ELunarStage::CollectData:return DataActor; case ELunarStage::ReturnHome:return TrackRecorder&&!RecorderRecovered?RecorderActor.Get():HomeActor.Get(); default:return nullptr; }
}
FString ALunarGameMode::Objective() const {
 if(PowerSequenceActive)return "03 / STATION STARTING UP";
 if(Stage==ELunarStage::ReturnHome&&TrackRecorder&&!RecorderRecovered)return "OPTIONAL / RECOVER THE CREW RECORDER";
 switch(Stage) { case ELunarStage::FindModule:return "01 / FIND THE POWER MODULE"; case ELunarStage::RestorePower:return "02 / RESTORE STATION POWER"; case ELunarStage::CollectData:return "03 / RECOVER THE SCIENCE DATA"; case ELunarStage::ReturnHome:return "04 / RETURN TO THE LANDER"; default:return "SELENE / RESCUE MISSION"; }
}
FString ALunarGameMode::Prompt() const {
 if(PowerSequenceActive)return "";
 switch(Stage) { case ELunarStage::FindModule:return "[ E ]  Pick up power module"; case ELunarStage::RestorePower:return RepairStep==0?"[ E ]  Insert power cell":RepairStep==1?"[ E ]  Connect power cable":"[ HOLD E ]  Restart station"; case ELunarStage::CollectData:return "[ HOLD E ]  Download science data"; case ELunarStage::ReturnHome:return InteractionTarget()==RecorderActor&&!RecorderRecovered?"[ HOLD E ]  Read crew recorder":"[ E ]  Complete mission"; default:return ""; }
}
bool ALunarGameMode::CanInteract() const {
 return CanInteractWith(InteractionTarget());
}
AActor* ALunarGameMode::InteractionTarget() const {
 // Q selects navigation only. Nearby usable objectives remain interactive.
 if(Stage==ELunarStage::ReturnHome) {
  if(CanInteractWith(HomeActor))return HomeActor;
  if(!RecorderRecovered&&CanInteractWith(RecorderActor))return RecorderActor;
 }
 return Target();
}
bool ALunarGameMode::CanInteractWith(AActor* T) const {
 if(PowerSequenceActive)return false;
 auto* P=UGameplayStatics::GetPlayerPawn(this,0);
 if(!IsValid(T)||!P||Paused||!IsActive()||FVector::Dist(P->GetActorLocation(),T->GetActorLocation())>300) return false;
 if(T==RecorderActor&&P->GetActorLocation().Z<T->GetActorLocation().Z+65.f)return false;
 FVector Eye; FRotator Rot; P->GetActorEyesViewPoint(Eye,Rot);
 if(FVector::DotProduct(Rot.Vector(),(T->GetActorLocation()-Eye).GetSafeNormal())<0.35) return false;
 FHitResult Hit; FCollisionQueryParams Q; Q.AddIgnoredActor(P); Q.AddIgnoredActor(T); Q.AddIgnoredActor(this);
 return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,T->GetActorLocation(),ECC_Visibility,Q);
}
void ALunarGameMode::Message(const FString& T) {
 Radio=T; RadioRemaining=10;
 FString Name=T.StartsWith(TEXT("SIGNAL LOST"))?TEXT("StoryIntro"):T.StartsWith(TEXT("NAV:"))?TEXT("StoryStart"):T.StartsWith(TEXT("MODULE"))?TEXT("Module"):T.StartsWith(TEXT("POWER RESTORED"))?TEXT("StoryPower"):T.StartsWith(TEXT("DATA RECOVERED"))?TEXT("StoryData"):T.StartsWith(TEXT("CREW LOG"))?TEXT("StoryTruth"):T.StartsWith(TEXT("SIGNAL RESTORED"))?TEXT("StoryWin"):T.StartsWith(TEXT("CAUTION"))?TEXT("StoryWarning"):TEXT("");
 if(RadioAudio)RadioAudio->Stop();
 if(!Name.IsEmpty()) {
  if(RadioAudio)RadioAudio->Stop();
  auto* Sound=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Immersion/Audio/Radio_%s.Radio_%s"),*Name,*Name));
  if(Sound){RadioRemaining=FMath::Max(10.f,Sound->GetDuration()+.5f);RadioAudio=UGameplayStatics::SpawnSound2D(this,Sound,.8f);}
 }
}
void ALunarGameMode::BuildStoryProps() {
 auto Make=[&](const TCHAR* Mesh,const TCHAR* Mat,FVector Pos,FVector Scale) {
  auto* Visual=GetWorld()->SpawnActor<AActor>();
  auto* C=NewObject<UStaticMeshComponent>(Visual);Visual->SetRootComponent(C);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Mesh));
  if(Mat)C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Mat));
  C->SetMobility(EComponentMobility::Movable);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  C->RegisterComponent();C->SetWorldLocation(Pos);C->SetWorldScale3D(Scale);return C;
 };
 if(PowerActor) {
  const FVector Port=PowerActor->GetActorLocation();
  InstalledModule=Make(TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell"),nullptr,Port+FVector(-69,0,-45),FVector(.5));
  InstalledModule->SetVisibility(false);
  const FVector Start=Port+FVector(-77,0,-25), End=Port+FVector(-57,28,35);
  PowerCable=Make(TEXT("/Engine/BasicShapes/Sphere.Sphere"),TEXT("/Game/Lunar/Materials/M_Graphite.M_Graphite"),Start,FVector(.085));
  auto Point=[&](float T){const float U=1-T;return U*U*U*Start+3*U*U*T*(Start+FVector(-28,40,-20))+3*U*T*T*(End+FVector(-30,30,-15))+T*T*T*End;};
  for(int I=0;I<18;++I) {
   const FVector A=Point(I/18.f),B=Point((I+1)/18.f);
   auto* Segment=Make(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),TEXT("/Game/Lunar/Materials/M_ThermalFoil.M_ThermalFoil"),(A+B)*.5f,FVector(.026,.026,FVector::Distance(A,B)/100.f+.008f));
   Segment->SetWorldRotation(FRotationMatrix::MakeFromZ(B-A).Rotator());
   Segment->AttachToComponent(PowerCable,FAttachmentTransformRules::KeepWorldTransform);
  }
  auto* Connector=Make(TEXT("/Engine/BasicShapes/Sphere.Sphere"),TEXT("/Game/Lunar/Materials/M_Graphite.M_Graphite"),End,FVector(.085));
  Connector->AttachToComponent(PowerCable,FAttachmentTransformRules::KeepWorldTransform);
  PowerCable->SetVisibility(false,true);
 }
 // The optional recorder sits on a low survey ledge: a lunar jump reaches its top.
 FVector Site(6200,0,0);FHitResult Ground;FCollisionQueryParams Q;Q.bTraceComplex=true;
 for(TActorIterator<AActor> It(GetWorld());It;++It) {
  auto* Mesh=It->FindComponentByClass<UStaticMeshComponent>();
  if(!Mesh||!Mesh->GetStaticMesh()||!Mesh->GetStaticMesh()->GetName().Contains(TEXT("LunarTerrain")))Q.AddIgnoredActor(*It);
 }
 if(GetWorld()->LineTraceSingleByChannel(Ground,Site+FVector(0,0,2500),Site-FVector(0,0,1400),ECC_Visibility,Q))Site.Z=Ground.ImpactPoint.Z;
 auto* Ledge=GetWorld()->SpawnActor<AActor>();
 auto* Rock=NewObject<UStaticMeshComponent>(Ledge);Ledge->SetRootComponent(Rock);
 auto* LedgeMesh=FParse::Param(FCommandLine::Get(),TEXT("LunarMissingLedgeMesh"))?nullptr:LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_0.SM_Basalt_0"));
 if(!LedgeMesh||LedgeMesh->GetBoundingBox().GetSize().GetMin()<=KINDA_SMALL_NUMBER) {
  UE_LOG(LogTemp,Warning,TEXT("Lunar ledge mesh unavailable or degenerate; using safe fallback."));
  LedgeMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
 }
 if(!LedgeMesh){UE_LOG(LogTemp,Error,TEXT("No ledge mesh available; optional recorder disabled."));Ledge->Destroy();return;}
 Rock->SetStaticMesh(LedgeMesh);
 Rock->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/V3/Materials/M_LunarRegolith.M_LunarRegolith")));
 const FBox Bounds=Rock->GetStaticMesh()->GetBoundingBox();
 const FVector RockScale=FVector(400,400,140)/Bounds.GetSize();
 Rock->SetCollisionProfileName(TEXT("BlockAll"));Rock->RegisterComponent();
 Ledge->SetActorLocation(Site+FVector(0,0,-50-Bounds.Min.Z*RockScale.Z));Rock->SetWorldScale3D(RockScale);
 RecorderActor=GetWorld()->SpawnActor<AActor>();auto* Recorder=NewObject<UStaticMeshComponent>(RecorderActor);RecorderActor->SetRootComponent(Recorder);
 Recorder->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Imported/ScienceTerminal/ScienceTerminal.ScienceTerminal")));
 Recorder->SetCollisionEnabled(ECollisionEnabled::NoCollision);Recorder->RegisterComponent();RecorderActor->SetActorLocation(Site+FVector(0,0,95));Recorder->SetWorldScale3D(FVector(.42));
 Make(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),TEXT("/Game/Lunar/Materials/M_Signal.M_Signal"),Site+FVector(110,0,165),FVector(.055,.055,1.5));
}
float ALunarGameMode::HoldDuration() const {
 if(Stage==ELunarStage::RestorePower&&RepairStep==2)return 2.f;
 if(Stage==ELunarStage::CollectData)return 3.f;
 if(Stage==ELunarStage::ReturnHome&&InteractionTarget()==RecorderActor&&!RecorderRecovered)return 2.f;
 return 0;
}
void ALunarGameMode::ReleaseInteract() {Holding=false;HoldProgress=0;}
void ALunarGameMode::ToggleRoute() {
 if(Stage!=ELunarStage::ReturnHome||Paused||RecorderRecovered||!RecorderActor)return;
 ReleaseInteract();TrackRecorder=!TrackRecorder;
 Message(TrackRecorder?"NAVIGATION: Crew recorder selected. Jump onto the survey ledge. Q returns to the lander route.":"NAVIGATION: Lander selected. The crew recorder is optional.");
}
void ALunarGameMode::AdvanceInteraction(float D) {
 if(Paused)return;
 if(!Holding)return;
 if(!CanInteract()||HoldDuration()<=0) {ReleaseInteract();return;}
 HoldProgress+=FMath::Max(0.f,D);
 if(HoldProgress<HoldDuration())return;
 ReleaseInteract();
 if(Stage==ELunarStage::RestorePower) {Stage=ELunarStage::CollectData;BeginPowerSequence();}
 else if(Stage==ELunarStage::CollectData) {Stage=ELunarStage::ReturnHome;Message("DATA RECOVERED. A crew recorder is on the survey ledge. Q selects the optional route.");}
 else if(Stage==ELunarStage::ReturnHome&&!RecorderRecovered) {
  RecorderRecovered=true;TrackRecorder=false;
  Message("CREW LOG: Cooling failed. We cut rover power to save the samples. Crew safe at the relay.");
 }
}
void ALunarGameMode::Interact() {
 if(!CanInteract()||Holding)return;
 if(HoldDuration()>0) {Holding=true;HoldProgress=0;return;}
 switch(Stage) {
 case ELunarStage::FindModule: ModuleActor->SetActorHiddenInGame(true);ModuleActor->SetActorEnableCollision(false);Stage=ELunarStage::RestorePower;Message("MODULE SECURED. Install it in the station's external power port.");break;
 case ELunarStage::RestorePower:
  if(RepairStep==0) {RepairStep=1;if(InstalledModule)InstalledModule->SetVisibility(true);Message("CELL SEATED. Connect the orange power cable.");}
  else if(RepairStep==1) {RepairStep=2;if(PowerCable)PowerCable->SetVisibility(true,true);Message("CABLE CONNECTED. Hold E to restart the station.");}
  PlayPowerSound(TEXT("PowerContact"),.5f);
  break;
 case ELunarStage::ReturnHome:Stage=ELunarStage::Won;Message("SIGNAL RESTORED. We have your signal. Bring yourself home.");break;
 default:break;
 }
}

void ALunarHUD::Label(const FString& T,float X,float Y,float S,FLinearColor C) { DrawText(T,C,X,Y,GEngine->GetLargeFont(),S*2.3f,false); }
void ALunarHUD::Panel(float X,float Y,float W,float H,float A) { DrawRect(FLinearColor(0.008,0.019,0.032,A),X,Y,W,H); }
void ALunarHUD::DrawHUD() {
 Super::DrawHUD(); auto* G=Mission(this); if(!G||!Canvas) return;
 float W=Canvas->SizeX,H=Canvas->SizeY; float S=FMath::Min(W/1280.f,H/720.f);
 FLinearColor White(.87,.94,1),Muted(.46,.63,.73),Cyan(.12,.86,.92),Orange(1,.53,.17);
 // Thin curved helmet seal around the periphery, behind all readable HUD elements.
 if(G->IsActive()) for(int I=0;I<128;++I) {
  float A=2*PI*I/128.f,B=2*PI*(I+1)/128.f;
  float X=W*.5f+FMath::Cos(A)*W*.525f,Y=H*.5f+FMath::Sin(A)*H*.55f;
  float X2=W*.5f+FMath::Cos(B)*W*.525f,Y2=H*.5f+FMath::Sin(B)*H*.55f;
  const FLinearColor Rim=G->Oxygen<60?FLinearColor(.11,.012,.008,.85):FLinearColor(.009,.013,.018,.92);
  DrawLine(X,Y,X2,Y2,Rim,12*S);
 }
 Panel(0,0,W,90*S,.9); DrawRect(Cyan,32*S,26*S,3*S,36*S);
 Label("LUNAR / RESCUE",48*S,22*S,.8*S,White); Label("SELENE EXPEDITION  /  01",48*S,51*S,.4*S,Muted);
 FString Time=FString::Printf(TEXT("%02d:%02d"),FMath::CeilToInt(G->Oxygen)/60,FMath::CeilToInt(G->Oxygen)%60);
 Label("O2  "+Time,W-220*S,24*S,.9*S,G->Oxygen<60?Orange:Cyan);
 DrawRect(FLinearColor(.1,.18,.23),W-220*S,65*S,182*S,4*S);
 DrawRect(G->Oxygen<60?Orange:Cyan,W-220*S,65*S,182*S*FMath::Clamp(G->Oxygen/G->OxygenCapacity,0.f,1.f),4*S);
 if(G->IsActive()) {
  Panel(24*S,110*S,520*S,70*S,.8); Label(G->Objective(),42*S,126*S,.55*S,White);
  if(G->PowerSequenceActive) {
   Panel(24*S,191*S,520*S,62*S,.83);
   Label(FString::Printf(TEXT("STATION RESTART  /  %02d%%"),FMath::Clamp(FMath::RoundToInt(G->PowerSequenceSeconds/4.2f*100),0,99)),42*S,201*S,.48*S,Cyan);
   DrawRect(FLinearColor(.09,.18,.23),42*S,232*S,473*S,5*S);
   DrawRect(Cyan,42*S,232*S,473*S*FMath::Clamp(G->PowerSequenceSeconds/4.2f,0.f,1.f),5*S);
  } else if(G->StationRebootComplete&&G->Stage==ELunarStage::CollectData) {
   Panel(24*S,191*S,520*S,62*S,.83);
   Label("SCIENCE DATA AVAILABLE",42*S,201*S,.48*S,Cyan);
   Label("READ THE REGOLITH TEMPERATURE RECORDS",42*S,230*S,.35*S,Muted);
  }
  if(G->Stage==ELunarStage::RestorePower&&!G->PowerSequenceActive) {
   Panel(24*S,191*S,520*S,62*S,.83);
   Label(FString::Printf(TEXT("REPAIR %d / 3  -  %s"),G->RepairStep+1,G->RepairStep==0?TEXT("INSERT CELL"):G->RepairStep==1?TEXT("CONNECT CABLE"):TEXT("HOLD TO RESTART")),42*S,203*S,.46*S,Cyan);
   Label("Move close to the power port and look at it.",42*S,231*S,.37*S,Muted);
  }
  if(G->Stage==ELunarStage::ReturnHome) {
   Panel(24*S,191*S,570*S,75*S,.83);
   Label(G->RecorderRecovered?"CREW LOG RECOVERED / CREW SAFE":G->TrackRecorder?"[ Q ]  SWITCH BACK TO LANDER":"[ Q ]  OPTIONAL: INVESTIGATE CREW RECORDER",42*S,203*S,.43*S,Cyan);
   if(!G->RecorderRecovered)Label("Extra travel uses oxygen. Returning now still completes the mission.",42*S,233*S,.35*S,Muted);
  }
  if(G->Holding) {
   const float Progress=FMath::Clamp(G->HoldProgress/G->HoldDuration(),0.f,1.f);
   Panel(W/2-220*S,H-243*S,440*S,28*S,.9);
   DrawRect(Cyan,W/2-210*S,H-234*S,420*S*Progress,8*S);
  }
  auto* T=G->Target(); auto* P=UGameplayStatics::GetPlayerPawn(this,0);
  if(T&&P) {
   float M=FVector::Dist(P->GetActorLocation(),T->GetActorLocation())/100;
   Label(FString::Printf(TEXT("TARGET  %.0f m   /   g = 1.62 m/s2"),M),42*S,153*S,.42*S,Muted);
   FVector2D Pos; auto* PC=GetOwningPlayerController();
   bool Front=FVector::DotProduct(PC->GetControlRotation().Vector(),T->GetActorLocation()-P->GetActorLocation())>0;
   if(Front&&PC->ProjectWorldLocationToScreen(T->GetActorLocation()+FVector(0,0,120),Pos)) {
    Pos.X=FMath::Clamp(Pos.X,60*S,W-100*S); Pos.Y=FMath::Clamp(Pos.Y,210*S,H-160*S);
    DrawRect(Cyan,Pos.X-5*S,Pos.Y-5*S,10*S,10*S); Label(FString::Printf(TEXT("%.0f m"),M),Pos.X+15*S,Pos.Y-8*S,.48*S,White);
   } else {
    float Side=FVector::DotProduct(FRotationMatrix(FRotator(0,PC->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),T->GetActorLocation()-P->GetActorLocation());
    Label(Side>0?"TARGET  >":"<  TARGET",Side>0?W-180*S:30*S,H*.5,.65*S,Cyan);
   }
  }
  DrawLine(W/2-7*S,H/2,W/2+7*S,H/2,FLinearColor(1,1,1,.65),1); DrawLine(W/2,H/2-7*S,W/2,H/2+7*S,FLinearColor(1,1,1,.65),1);
  if(G->CanInteract()) { Panel(W/2-240*S,H-205*S,480*S,50*S); Label(G->Prompt(),W/2-218*S,H-191*S,.6*S,Cyan); }
  if(G->RadioRemaining>0) { Panel(24*S,H-138*S,W-48*S,62*S,.88); Label("RADIO / "+G->Radio,40*S,H-118*S,.44*S,White); }
 }
 Panel(0,H-48*S,W,48*S,.9); Label("WASD / ARROWS  Move     MOUSE  Look     SPACE  Jump     E  Interact     F  Zoom     Q  Route     ESC  Pause",32*S,H-31*S,.43*S,Muted);
 if(G->Stage==ELunarStage::Briefing||G->Stage==ELunarStage::Won||G->Stage==ELunarStage::Lost||G->Paused) {
  Panel(0,90*S,W,H-138*S,.6); float X=W/2-365*S,Y=H/2-180*S;
  Panel(X,Y,730*S,360*S,.96); DrawRect(Cyan,X,Y,730*S,3*S);
  FString Title=G->Paused?"MISSION PAUSED":G->Stage==ELunarStage::Won?(G->RecorderRecovered?"THE TRUTH RECOVERED":"MISSION COMPLETE"):G->Stage==ELunarStage::Lost?"SIGNAL LOST":"THE LAST SIGNAL";
  Label("LUNAR RESCUE  /  MISSION 01",X+36*S,Y+30*S,.44*S,Cyan);
  Label(Title,X+36*S,Y+69*S,.95*S,White);
  if(G->Stage==ELunarStage::Briefing) {
   Label("SELENE is offline. You have five minutes of oxygen.",X+36*S,Y+132*S,.49*S,White);
   Label("Find the module. Restore power. Recover the data and return.",X+36*S,Y+164*S,.49*S,White);
   Label("Lunar gravity: 1.62 m/s2. Expect longer jumps and slower falls.",X+36*S,Y+212*S,.43*S,Muted);
   Label("Fictional terrain. Science objective: monitor regolith temperature.",X+36*S,Y+238*S,.4*S,Muted);
   Label("[ ENTER / CLICK ]  START MISSION",X+36*S,Y+298*S,.65*S,Cyan);
  } else if(G->Paused) { Label("Movement and oxygen are paused.",X+36*S,Y+150*S,.6*S,White); Label("[ ESC / ENTER / CLICK ]  RESUME",X+36*S,Y+280*S,.65*S,Cyan); }
  else {
   Label(G->Stage==ELunarStage::Won?FString("We have your signal. Bring yourself home."):G->FailureReason,X+36*S,Y+145*S,.5*S,White);
   Label(G->Stage==ELunarStage::Won?FString::Printf(TEXT("O2 REMAINING  %02d:%02d   /   POWER ONLINE   /   SCIENCE SECURED"),FMath::CeilToInt(G->Oxygen)/60,FMath::CeilToInt(G->Oxygen)%60):FString("Tip: hold E during restart and data download."),X+36*S,Y+195*S,.42*S,Muted);
   if(G->Stage==ELunarStage::Won) Label(G->RecorderRecovered?"LOGS 2/2 - Crew evacuated safely. Samples saved by emergency shutdown.":"LOGS 1/2 - The crew recorder remains on the survey ledge.",X+36*S,Y+224*S,.4*S,Muted);
   Label("[ R / ENTER ]  PLAY AGAIN",X+36*S,Y+290*S,.65*S,Cyan);
  }
 }
}

void ALunarGameMode::RunChecks() {
 int Fail=0; FString Report;
 auto Check=[&](bool B,const TCHAR* N){ Report+=FString(B?TEXT("PASS "):TEXT("FAIL "))+N+TEXT("\n"); if(!B) ++Fail; };
 Check(ModuleActor&&PowerActor&&DataActor&&HomeActor,TEXT("All four mission actors loaded"));
 Check(RecorderActor&&InstalledModule&&PowerCable,TEXT("Story recorder and repair visuals spawned"));
 Check(InstalledModule&&!InstalledModule->GetOwner()->IsHidden()&&PowerCable&&!PowerCable->GetOwner()->IsHidden(),TEXT("Repair visuals belong to visible world actors"));
 bool StoryAudioReady=true;
 for(const TCHAR* Name:{TEXT("StoryIntro"),TEXT("StoryStart"),TEXT("StoryPower"),TEXT("StoryData"),TEXT("StoryTruth"),TEXT("StoryWin"),TEXT("StoryWarning")})StoryAudioReady&=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Immersion/Audio/Radio_%s.Radio_%s"),Name,Name))!=nullptr;
 Check(StoryAudioReady,TEXT("Seven English story radio clips loaded"));
 auto* P=UGameplayStatics::GetPlayerPawn(this,0); auto* PC=UGameplayStatics::GetPlayerController(this,0);
 Check(P&&PC,TEXT("Player spawned and possessed"));
 Check(FMath::IsNearlyEqual(GetWorld()->GetGravityZ(),-162.f,0.1f),TEXT("Lunar gravity is -162 cm/s2"));
 AdvanceTime(10); Check(Oxygen==OxygenCapacity,TEXT("Briefing does not consume oxygen"));
 StartMission(); Check(Stage==ELunarStage::FindModule,TEXT("Start enters module search"));
 const float RadioSaved=RadioRemaining;Paused=true;AdvanceRadio(30);
 Check(FMath::IsNearlyEqual(RadioRemaining,RadioSaved),TEXT("Long pause preserves radio subtitle"));
 Paused=false;AdvanceRadio(1);Check(FMath::IsNearlyEqual(RadioRemaining,RadioSaved-1),TEXT("Radio subtitle countdown resumes"));
 Interact(); Check(Stage==ELunarStage::FindModule,TEXT("Cannot interact remotely"));
 Paused=true; AdvanceTime(20); Check(Oxygen==OxygenCapacity,TEXT("Pause stops oxygen")); Paused=false;
 AdvanceTime(5); Check(FMath::IsNearlyEqual(Oxygen,OxygenCapacity-5),TEXT("Oxygen decrements by elapsed seconds"));
 if(P&&PC&&ModuleActor&&PowerActor&&DataActor&&HomeActor) {
  const ELunarStage Expected[]={ELunarStage::RestorePower,ELunarStage::CollectData,ELunarStage::ReturnHome,ELunarStage::Won};
  for(int I=0;I<4;++I) {
   auto* T=Target(); if(!T) {Check(false,TEXT("Target unavailable"));break;}
   FVector L=T->GetActorLocation()+FVector(-180,0,40); P->SetActorLocation(L,false,nullptr,ETeleportType::TeleportPhysics);
   FVector Eye; FRotator Rot; P->GetActorEyesViewPoint(Eye,Rot); PC->SetControlRotation((T->GetActorLocation()-Eye).Rotation());
   if(!CanInteract()) {
    FVector E; FRotator R; P->GetActorEyesViewPoint(E,R); FHitResult Hit; FCollisionQueryParams Q; Q.AddIgnoredActor(P);Q.AddIgnoredActor(T);
    GetWorld()->LineTraceSingleByChannel(Hit,E,T->GetActorLocation(),ECC_Visibility,Q);
    Report+=FString::Printf(TEXT("DETAIL stage=%d distance=%.1f dot=%.3f hit=%s eye=%s target=%s\n"),int(Stage),FVector::Dist(P->GetActorLocation(),T->GetActorLocation()),FVector::DotProduct(R.Vector(),(T->GetActorLocation()-E).GetSafeNormal()),Hit.GetActor()?*Hit.GetActor()->GetName():TEXT("none"),*E.ToString(),*T->GetActorLocation().ToString());
   }
   Check(CanInteract(),*FString::Printf(TEXT("Target %d is reachable and visible"),I)); Interact();
   if(I==0)Check(ModuleActor->IsHidden()&&RepairStep==0,TEXT("Power cell picked up without enabling station"));
   if(I==1) {
    Check(RepairStep==1&&InstalledModule&&InstalledModule->IsVisible(),TEXT("Power cell visibly inserted"));
    Interact();Check(RepairStep==2&&PowerCable&&PowerCable->IsVisible(),TEXT("Cable connected as separate repair step"));
    Interact();AdvanceInteraction(.8f);ReleaseInteract();Check(HoldProgress==0&&!PowerSequenceActive,TEXT("Released hold resets restart progress"));
    Interact();Paused=true;AdvanceInteraction(5);Check(HoldProgress==0,TEXT("Paused hold does not advance"));Paused=false;
    AdvanceInteraction(2.1f);
   }
   if(I==2) {
    AdvanceInteraction(1);PC->SetControlRotation(FRotator(0,180,0));AdvanceInteraction(1);Check(!Holding&&Stage==ELunarStage::CollectData,TEXT("Looking away cancels download"));
    PC->SetControlRotation((T->GetActorLocation()-Eye).Rotation());Interact();AdvanceInteraction(3.1f);
   }
   Check(Stage==Expected[I],*FString::Printf(TEXT("Mission transition %d"),I));
   if(I==1) {
    Check(PowerSequenceActive&&!StationRebootComplete,TEXT("Station restart begins after module installation"));
    if(!FParse::Param(FCommandLine::Get(),TEXT("nosound"))) {
     Paused=true;UpdateAudioPause();bool AllPaused=PowerSounds.Num()>=2;
     for(const auto& C:PowerSounds)AllPaused&=IsValid(C.Get())&&C->bIsPaused;
     Check(AllPaused,TEXT("Pause freezes every power sound including contact"));
     Paused=false;UpdateAudioPause();bool AllResumed=PowerSounds.Num()>=2;
     for(const auto& C:PowerSounds)AllResumed&=IsValid(C.Get())&&!C->bIsPaused;
     Check(AllResumed,TEXT("Resume restores every power sound"));
    }
    Check(!CanInteract()&&Target()==nullptr,TEXT("Science terminal waits for completed restart"));
    const float SavedSequence=PowerSequenceSeconds;
    Paused=true;AdvancePowerSequence(2);Check(FMath::IsNearlyEqual(PowerSequenceSeconds,SavedSequence),TEXT("Station restart freezes during pause"));Paused=false;
    AdvancePowerSequence(4.3f);
    Check(StationRebootComplete&&!PowerSequenceActive,TEXT("Station restart completes once"));
    Check(PowerScreen&&!PowerScreen->IsHidden(),TEXT("Science terminal screen powers on"));
    Check(PowerAntenna&&!PowerAntenna->GetActorRotation().Equals(AntennaStartRotation,.1f),TEXT("Antenna points toward the relay"));
    bool LampsOn=true;for(const auto& Lamp:PowerLights)LampsOn&=Lamp&&Lamp->Intensity>100;
    Check(LampsOn,TEXT("Station roof lights are energized"));
    AdvancePowerSequence(10);Check(StationRebootComplete&&!PowerSequenceActive,TEXT("Station restart does not replay"));
   }
  }
  Check(!RecorderRecovered&&Stage==ELunarStage::Won,TEXT("Optional recorder may be skipped for normal victory"));
  Stage=ELunarStage::ReturnHome;TrackRecorder=true;
  Check(Target()==RecorderActor&&InteractionTarget()==HomeActor&&Prompt().Contains(TEXT("Complete mission")),TEXT("Lander prompt works while recorder marker is selected"));
  Interact();Check(Stage==ELunarStage::Won&&!RecorderRecovered,TEXT("Lander completes mission without changing Q route"));TrackRecorder=false;
  Stage=ELunarStage::ReturnHome;ToggleRoute();Check(Target()==RecorderActor,TEXT("Q selects optional recorder"));
  ToggleRoute();Check(Target()==HomeActor,TEXT("Optional detour can be abandoned"));ToggleRoute();
  if(RecorderActor) {
   P->SetActorLocation(RecorderActor->GetActorLocation()+FVector(-260,0,-7),false,nullptr,ETeleportType::TeleportPhysics);
   Check(!CanInteract(),TEXT("Recorder requires reaching the survey ledge"));
   P->SetActorLocation(RecorderActor->GetActorLocation()+FVector(-100,0,100),false,nullptr,ETeleportType::TeleportPhysics);
   FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((RecorderActor->GetActorLocation()-Eye).Rotation());
   TrackRecorder=false;Check(CanInteract()&&InteractionTarget()==RecorderActor,TEXT("Recorder usable even with lander marker selected"));
   Check(CanInteract(),TEXT("Recorder reachable from survey ledge"));Interact();AdvanceInteraction(2.1f);
   Check(RecorderRecovered&&!TrackRecorder&&Target()==HomeActor,TEXT("Recorder reveals crew fate and restores home marker"));
   ToggleRoute();Check(!TrackRecorder,TEXT("Recovered recorder cannot be collected twice"));
   P->SetActorLocation(HomeActor->GetActorLocation()+FVector(-180,0,40),false,nullptr,ETeleportType::TeleportPhysics);
   P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((HomeActor->GetActorLocation()-Eye).Rotation());Interact();
   Check(Stage==ELunarStage::Won&&RecorderRecovered,TEXT("Full story ending completes at lander"));
  }
  float Saved=Oxygen; AdvanceTime(500); Check(Stage==ELunarStage::Won&&Oxygen==Saved,TEXT("Victory freezes oxygen"));
  Stage=ELunarStage::FindModule; Oxygen=1; AdvanceTime(2); Check(Stage==ELunarStage::Lost&&Oxygen==0,TEXT("Oxygen depletion causes failure"));
  Check(FailureReason.Contains(TEXT("oxygen")),TEXT("Oxygen loss has correct reason"));
  Stage=ELunarStage::FindModule;Oxygen=100;P->SetActorLocation(FVector(0,0,-1600),false,nullptr,ETeleportType::TeleportPhysics);AdvanceTime(0);
  Check(Stage==ELunarStage::Lost&&Oxygen==100&&FailureReason.Contains(TEXT("fell"))&&!FailureReason.Contains(TEXT("oxygen")),TEXT("Out-of-bounds fall has distinct reason with oxygen remaining"));
 }
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Fail);
 FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectDir()/TEXT("Reports/gameplay-checks.txt")));
 UE_LOG(LogTemp,Display,TEXT("LUNAR_CHECKS %s"),*Report);
 FPlatformMisc::RequestExitWithStatus(false,Fail?1:0);
}

void ALunarGameMode::MovementCheck() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC) { FFileHelper::SaveStringToFile(TEXT("FAIL no possessed astronaut"),*(FPaths::ProjectDir()/TEXT("Reports/movement-checks.txt"))); FPlatformMisc::RequestExitWithStatus(false,1);return; }
 const FKey Keys[]={EKeys::W,EKeys::D,EKeys::S,EKeys::A};
 auto Input=[&](FKey Key,EInputEvent Event) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,Event,Event==IE_Released?0.f:1.f)); };
 if(MovementPhase==0) {
  StartMission(); PC->SetControlRotation(FRotator(0,0,0)); MovementOrigin=P->GetActorLocation();
  MovementReport=FString::Printf(TEXT("Spawn: %s, pawn=%s, movement mode=%d\n"),*MovementOrigin.ToString(),*P->GetClass()->GetName(),int(P->GetCharacterMovement()->MovementMode));
  Input(Keys[0],IE_Pressed);
 } else if(MovementPhase<=4) {
  Input(Keys[MovementPhase-1],IE_Released);
  FVector Delta=P->GetActorLocation()-MovementOrigin;
  float Along=MovementPhase==1?Delta.X:MovementPhase==2?Delta.Y:MovementPhase==3?-Delta.X:-Delta.Y;
  bool Good=Along>100;
  if(!Good)++MovementFailures;
  MovementReport+=FString::Printf(TEXT("%s %s held 1.5 seconds: %.1f cm along intended direction, delta=%s velocity=%s mode=%d\n"),Good?TEXT("PASS"):TEXT("FAIL"),*Keys[MovementPhase-1].ToString(),Along,*Delta.ToString(),*P->GetVelocity().ToString(),int(P->GetCharacterMovement()->MovementMode));
  if(!Good) {
   FHitResult Hit; FCollisionQueryParams Q; Q.AddIgnoredActor(P);
   const FVector Dirs[]={FVector(1,0,0),FVector(0,1,0),FVector(-1,0,0),FVector(0,-1,0)};
   GetWorld()->SweepSingleByChannel(Hit,P->GetActorLocation(),P->GetActorLocation()+Dirs[MovementPhase-1]*100,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(34,86),Q);
   MovementReport+=FString::Printf(TEXT("Blocker=%s label=%s input=%s\n"),Hit.GetActor()?*Hit.GetActor()->GetName():TEXT("none"),Hit.GetActor()?*Hit.GetActor()->GetActorNameOrLabel():TEXT("none"),*P->GetLastMovementInputVector().ToString());
  }
  P->GetCharacterMovement()->StopMovementImmediately(); MovementOrigin=P->GetActorLocation();
  if(MovementPhase<4) Input(Keys[MovementPhase],IE_Pressed);
  else Input(EKeys::SpaceBar,IE_Pressed);
 } else if(MovementPhase==5) {
  Input(EKeys::SpaceBar,IE_Released); float Height=P->GetActorLocation().Z-MovementOrigin.Z;
  bool Good=Height>50;if(!Good)++MovementFailures;
  MovementReport+=FString::Printf(TEXT("%s Space jump: %.1f cm rise\n"),Good?TEXT("PASS"):TEXT("FAIL"),Height);
  bool Prints=P->FootprintCount>=8&&P->Footprints.Num()<=160;
  bool Assets=P->BootMaterial&&P->StepSounds.Num()==3&&P->StepSounds[0]&&P->StepSounds[1]&&P->StepSounds[2]&&P->Gloves.Num()==18;
  if(!Prints)++MovementFailures;if(!Assets)++MovementFailures;
  MovementReport+=FString::Printf(TEXT("%s Grounded walking leaves bootprints: %d\n%s Suit meshes, decal and step sounds loaded\n"),Prints?TEXT("PASS"):TEXT("FAIL"),P->FootprintCount,Assets?TEXT("PASS"):TEXT("FAIL"));
 } else if(MovementPhase==6) {
  bool Landed=P->GetCharacterMovement()->IsMovingOnGround()&&P->LandingGrains.Num()==16;
  if(!Landed)++MovementFailures;
  MovementReport+=FString::Printf(TEXT("%s Jump landing emits bounded grain particles\n"),Landed?TEXT("PASS"):TEXT("FAIL"));
  P->PauseMission();MovementOrigin=FVector(P->GrainLife,P->FootprintCount,Oxygen);
 } else {
  bool Frozen=Paused&&FMath::IsNearlyEqual(P->GrainLife,float(MovementOrigin.X))&&P->FootprintCount==int(MovementOrigin.Y)&&FMath::IsNearlyEqual(Oxygen,float(MovementOrigin.Z));
  if(!Frozen)++MovementFailures;
  MovementReport+=FString::Printf(TEXT("%s Pause freezes footprints, particles and oxygen\nRESULT: %d failures\n"),Frozen?TEXT("PASS"):TEXT("FAIL"),MovementFailures);
  P->PauseMission();
  FFileHelper::SaveStringToFile(MovementReport,*(FPaths::ProjectDir()/TEXT("Reports/movement-checks.txt")));
  FPlatformMisc::RequestExitWithStatus(false,MovementFailures?1:0);return;
 }
 ++MovementPhase;
 FTimerHandle H; GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::MovementCheck,1.5,false);
}

void ALunarGameMode::ReviewVisuals() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 Stage=ELunarStage::FindModule;Oxygen=300;
 if(MovementPhase==0) {P->SetActorLocation(FVector(-2450,300,90));PC->SetControlRotation(FRotator(12,18,0));PC->SetIgnoreLookInput(true);PC->SetIgnoreMoveInput(true);P->Camera->SetFieldOfView(85);}
 if(MovementPhase==1) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/lunar-surface.png"),true,false);
 if(MovementPhase==2) {P->SetActorLocation(FVector(3000,550,100));PC->SetControlRotation(FRotator(-6,42,0));}
 if(MovementPhase==3) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/lunar-crater.png"),true,false);
 if(MovementPhase==4) {P->SetActorLocation(FVector(-2450,300,90));PC->SetControlRotation(FRotator(27,18,0));P->Camera->SetFieldOfView(18);}
 if(MovementPhase==5) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/lunar-earth.png"),true,false);
 if(MovementPhase==6) {
  PC->SetControlRotation(FRotator(0,0,0));
  for(int I=0;I<6;++I) {P->SetActorLocation(FVector(-2290+I*90,300,95));P->StampFootstep();}
  P->SetActorLocation(FVector(-2450,300,95));PC->SetControlRotation(FRotator(-40,0,0));P->Camera->SetFieldOfView(85);
 }
 if(MovementPhase==7)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/lunar-suit-footprints.png"),true,false);
 if(MovementPhase==8) {FPlatformMisc::RequestExitWithStatus(false,0);return;}
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ReviewVisuals,3,false);
}

void ALunarGameMode::PowerReview() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 if(MovementPhase==0) {
  Stage=ELunarStage::RestorePower;Oxygen=300;
  P->SetActorLocation(FVector(-430,-2050,95));PC->SetControlRotation(FRotator(7,34,0));
  PC->SetIgnoreLookInput(true);PC->SetIgnoreMoveInput(true);P->Camera->SetFieldOfView(85);
 }
 if(MovementPhase==1)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/power-before.png"),true,false);
 if(MovementPhase==2){Stage=ELunarStage::CollectData;BeginPowerSequence();}
 if(MovementPhase==3)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/power-sequence.png"),true,false);
 if(MovementPhase==4)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/power-restored.png"),true,false);
 if(MovementPhase==5){FPlatformMisc::RequestExitWithStatus(false,0);return;}
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::PowerReview,3,false);
}

void ALunarGameMode::StoryReview() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 Oxygen=240;
 if(MovementPhase==0) {Stage=ELunarStage::RestorePower;RepairStep=0;P->SetActorLocation(FVector(-320,-1000,95));PC->SetControlRotation(FRotator(-10,0,0));PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true);}
 if(MovementPhase==1)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-carry.png"),true,false);
 if(MovementPhase==2) {RepairStep=2;InstalledModule->SetVisibility(true);PowerCable->SetVisibility(true,true);}
 if(MovementPhase==3)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-repair.png"),true,false);
 if(MovementPhase==4) {Stage=ELunarStage::ReturnHome;TrackRecorder=true;P->SetActorLocation(RecorderActor->GetActorLocation()+FVector(-550,0,180));PC->SetControlRotation(FRotator(-10,0,0));Message("DATA RECOVERED. A crew recorder is on the survey ledge. Q selects the optional route.");}
 if(MovementPhase==5)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-recorder.png"),true,false);
 if(MovementPhase==6) {Stage=ELunarStage::Won;RecorderRecovered=true;}
 if(MovementPhase==7)FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/TEXT("Reports/story-ending.png"),true,false);
 if(MovementPhase==8){FPlatformMisc::RequestExitWithStatus(false,0);return;}
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::StoryReview,3,false);
}

void ALunarGameMode::LedgeCheck() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC||!RecorderActor){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto Input=[&](FKey Key,EInputEvent Event){PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,Event,Event==IE_Released?0.f:1.f));};
 if(MovementPhase==0) {
  Stage=ELunarStage::ReturnHome;TrackRecorder=true;
  FVector Approach=RecorderActor->GetActorLocation()+FVector(-310,0,0);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(P);Q.bTraceComplex=true;
  if(GetWorld()->LineTraceSingleByChannel(Hit,Approach+FVector(0,0,1000),Approach-FVector(0,0,1000),ECC_Visibility,Q))Approach.Z=Hit.ImpactPoint.Z+90;
  P->SetActorLocation(Approach,false,nullptr,ETeleportType::TeleportPhysics);P->GetCharacterMovement()->StopMovementImmediately();PC->SetControlRotation(FRotator(0,0,0));
 } else if(MovementPhase==1) {Input(EKeys::SpaceBar,IE_Pressed);Input(EKeys::W,IE_Pressed);}
 else if(MovementPhase==2) {Input(EKeys::W,IE_Released);Input(EKeys::SpaceBar,IE_Released);Input(EKeys::S,IE_Pressed);}
 else if(MovementPhase==3) {Input(EKeys::S,IE_Released);}
 else if(MovementPhase==4) {
  FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((RecorderActor->GetActorLocation()-Eye).Rotation());
  const bool Grounded=P->GetCharacterMovement()->IsMovingOnGround();const bool Reach=CanInteract();Interact();AdvanceInteraction(2.1f);
  FString R=FString::Printf(TEXT("%s Real Space + W jump lands on survey ledge\n%s Recorder is visible and usable after landing\n%s Optional log recovered\nPlayer: %s\nRecorder: %s\nRESULT: %d failures\n"),Grounded?TEXT("PASS"):TEXT("FAIL"),Reach?TEXT("PASS"):TEXT("FAIL"),RecorderRecovered?TEXT("PASS"):TEXT("FAIL"),*P->GetActorLocation().ToString(),*RecorderActor->GetActorLocation().ToString(),int(!Grounded)+int(!Reach)+int(!RecorderRecovered));
  FFileHelper::SaveStringToFile(R,*(FPaths::ProjectDir()/TEXT("Reports/ledge-checks.txt")));FPlatformMisc::RequestExitWithStatus(false,Grounded&&Reach&&RecorderRecovered?0:1);return;
 }
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::LedgeCheck,1,false);
}

// End-to-end traversal uses the character movement component, normal collision,
// real mission time and held interactions. No teleport or direct stage changes.
void ALunarGameMode::WalkthroughTick(float D) {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC||GetWorld()->GetTimeSeconds()<3)return;
 struct FStop {float X,Y;int Action;};
 const FStop Route[]={
  {-2450,650,0},{3700,650,0},{3780,-500,0},{0,0,1},
  {3700,650,0},{-450,650,0},{-450,-1000,0},{-270,-1000,0},{0,0,2},
  {-380,-1500,0},{-260,-1500,0},{0,0,3},
  {-450,-1500,0},{-450,650,0},{5890,650,0},{5890,0,0},{0,0,4},{0,0,5},
  {5890,650,0},{-2450,650,0},{-2450,-300,0},{0,0,6}
 };
 const double Now=FPlatformTime::Seconds();
 if(WalkLastFrame==0){StartMission();WalkReport=TEXT("Traversal starts at actual player spawn; no teleports.\n");}
 else WalkFrameTimes.Add(float((Now-WalkLastFrame)*1000));
 WalkLastFrame=Now;WalkSeconds+=D;WalkStepSeconds+=D;
 auto Finish=[&](bool Good,const FString& Reason) {
  WalkthroughEnabled=false;ReleaseInteract();P->StopJumping();
  WalkFrameTimes.Sort();double Sum=0;for(float V:WalkFrameTimes)Sum+=V;
  const float Mean=WalkFrameTimes.Num()?Sum/WalkFrameTimes.Num():0;
  const float P95=WalkFrameTimes.Num()?WalkFrameTimes[FMath::Min(WalkFrameTimes.Num()-1,int(WalkFrameTimes.Num()*.95f))]:0;
  const auto Memory=FPlatformMemory::GetStats();
  WalkReport+=FString::Printf(TEXT("%s %s\nMission seconds: %.2f; oxygen remaining: %.2f\nFrames: %d; mean frame ms: %.2f; p95 frame ms: %.2f; mean FPS: %.2f\nPeak process physical memory MiB: %.1f\nRenderer: %s\nRESULT: %d failures\n"),Good?TEXT("PASS"):TEXT("FAIL"),*Reason,WalkSeconds,Oxygen,WalkFrameTimes.Num(),Mean,P95,Mean>0?1000/Mean:0,Memory.PeakUsedPhysical/1048576.0,FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))?TEXT("NullRHI - no GPU performance claim"):TEXT("Graphics enabled"),Good?0:1);
  FFileHelper::SaveStringToFile(WalkReport,*(FPaths::ProjectDir()/TEXT("Reports/walkthrough-checks.txt")));
  UE_LOG(LogTemp,Display,TEXT("LUNAR_WALKTHROUGH %s"),*WalkReport);
  FPlatformMisc::RequestExitWithStatus(false,Good?0:1);
 };
 if(Stage==ELunarStage::Lost||WalkSeconds>295||WalkStepSeconds>45) {
  Finish(false,FString::Printf(TEXT("Route step %d at %s: %s"),WalkStep,*P->GetActorLocation().ToString(),*FailureReason));return;
 }
 if(WalkStep>=UE_ARRAY_COUNT(Route)){Finish(Stage==ELunarStage::Won&&RecorderRecovered,TEXT("Full walked rescue including optional recorder and lander return"));return;}
 const auto& Stop=Route[WalkStep];bool Done=false;
 if(Stop.Action==0) {
  FVector Direction=FVector(Stop.X,Stop.Y,P->GetActorLocation().Z)-P->GetActorLocation();
  const float Distance=Direction.Size2D();
  if(Distance<18)Done=true;
  else {PC->SetControlRotation(Direction.Rotation());P->Forward(FMath::Clamp(Distance/100.f,.15f,1.f));}
 } else if(Stop.Action==4) {
  PC->SetControlRotation(FRotator(0,0,0));
  if(WalkStepSeconds<1){P->Jump();P->Forward(1);}
  else if(WalkStepSeconds<2){P->StopJumping();P->Forward(-1);}
  else Done=WalkStepSeconds>3&&P->GetCharacterMovement()->IsMovingOnGround();
 } else {
  AActor* T=Stop.Action==1?ModuleActor.Get():Stop.Action==2?PowerActor.Get():Stop.Action==3?DataActor.Get():Stop.Action==5?RecorderActor.Get():HomeActor.Get();
  if(!T){Finish(false,TEXT("Missing mission target"));return;}
  FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((T->GetActorLocation()-Eye).Rotation());
  if(Stop.Action==6)TrackRecorder=true; // Landing must not depend on marker selection.
  if(!Holding)Interact();
  Done=Stop.Action==1?Stage==ELunarStage::RestorePower:Stop.Action==2?StationRebootComplete:Stop.Action==3?Stage==ELunarStage::ReturnHome:Stop.Action==5?RecorderRecovered:Stage==ELunarStage::Won;
 }
 if(Done){WalkReport+=FString::Printf(TEXT("PASS route step %d at %.2fs (%s)\n"),WalkStep,WalkSeconds,*P->GetActorLocation().ToString());UE_LOG(LogTemp,Display,TEXT("Walk step %d complete at %.2fs"),WalkStep,WalkSeconds);++WalkStep;WalkStepSeconds=0;}
}
