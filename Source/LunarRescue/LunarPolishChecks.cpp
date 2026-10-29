#include "LunarGame.h"
#include "LunarCargoFinale.h"
#include "LunarSettings.h"
#include "LunarPanels.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "InputKeyEventArgs.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

void ALunarGameMode::ApolloPolishChecks(){
 FString& Report=MovementReport;int& Failed=MovementFailures;auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto Aim=[&](AActor* A){P->GetCharacterMovement()->StopMovementImmediately();P->SetActorLocation(A->GetActorLocation()+FVector(-170,0,50),false,nullptr,ETeleportType::TeleportPhysics);FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Click=[&](){TickApollo(.85f);P->PrimaryClick();};
 if(MovementPhase==0){
 Check(!ApolloTimed&&ApolloToolsEnabled,TEXT("Default product mode is untimed exploration with distinct tools"));
 StartApollo(true);P->GetCharacterMovement()->DisableMovement();AdvanceTime(2000);Check(IsActive()&&Oxygen==1800,TEXT("Historical exploration does not time out after 30 minutes"));
 Aim(ModuleActor);P->Interact();Check(ApolloTool==8&&ApolloAction==0,TEXT("E opens checkout without committing it"));P->PauseMission();Check(!ApolloTool&&!Paused&&ApolloAction==0,TEXT("Escape cancels a tool before pausing"));
 for(int I=0;I<3;++I){P->Interact();Click();}
 Check(Stage==ELunarStage::ApolloDeploy,TEXT("Equipment checks advance through actual primary-action input"));
 Aim(ApolloInstrumentActors[0]);P->Interact();Click();Check(ApolloTool==2&&ApolloInstruments==0,TEXT("Unlevel seismometer cannot be committed"));
 const FVector2D Old=ApolloLevel;Paused=true;ApolloToolAxis(1,1);Check(ApolloLevel.Equals(Old),TEXT("Paused tool axes do not change instrument state"));Paused=false;
 ApolloToolAxis(-ApolloLevel.X/.035f,-ApolloLevel.Y/.035f);Click();Check(ApolloInstruments==1&&ApolloAction==1,TEXT("Centered bubble commits the seismometer"));
 Aim(ApolloInstrumentActors[1]);P->Interact();Click();Check(ApolloTool==3&&ApolloInstruments==1,TEXT("Weak Earth signal prevents antenna confirmation"));ApolloToolAxis(75/.6f,0);Click();Check(Stage==ELunarStage::ApolloSamples,TEXT("Mouse alignment and confirmation unlock samples"));
 }else if(MovementPhase<=6){
  const int Order[]={1,0,2};const int I=Order[(MovementPhase-1)/2];
  if(MovementPhase%2==1){
  Aim(SampleActors[I]);P->Interact();Check(ApolloTool==1&&!(ApolloPhotos&(1<<I)),TEXT("Camera opens without awarding photograph metadata"));
  PC->SetControlRotation(PC->GetControlRotation()+FRotator(0,20,0));Click();Check(!(ApolloPhotos&(1<<I)),TEXT("Shutter outside the framing guide does not award context evidence"));
  Aim(SampleActors[I]);Click();ApolloTestDeadline=FPlatformTime::Seconds()+30;
  }else{
  if(ApolloPhotoPending&&FPlatformTime::Seconds()<ApolloTestDeadline){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloPolishChecks,.1f,false);return;}
  Check(!ApolloPhotoPending&&(ApolloPhotos&(1<<I))&&!ApolloTool,TEXT("Framed photograph creates exactly one evidence record"));
  P->Interact();Check(ApolloTool==(I==2?5:4),TEXT("Rock/soil and drive core use different tool contexts"));
  Click();CancelApolloTool();Check(!(SamplesCollected&(1<<I)),TEXT("Cancelling partial sampling never adds cargo"));
  P->Interact();const int Needed=I==2?7:2;
  for(int N=0;N<Needed-1;++N)Click();Check(!(SamplesCollected&(1<<I)),TEXT("Sample is pending until the final seal/cap action"));
  Click();Check((SamplesCollected&(1<<I))&&!ApolloTool,TEXT("Final cap or seal commits sample with its photograph"));
  }
 }else if(MovementPhase==7){
 Check(ApolloEvidence.Num()==3&&Stage==ELunarStage::ApolloTraverse,TEXT("Three distinct sample workflows complete the science traverse"));
 Aim(DroneActor);TickApollo(0);P->Interact();FVector CloseEye;FRotator CloseAim;P->GetActorEyesViewPoint(CloseEye,CloseAim);PC->SetControlRotation((DroneActor->GetActorLocation()+FVector(0,0,90)-CloseEye).Rotation());Click();Check(!SurveyorPhotographed,TEXT("Close-up of only part of Surveyor is not a context photograph"));
 P->SetActorLocation(DroneActor->GetActorLocation()+FVector(-600,0,100));FVector PhotoEye;FRotator PhotoAim;P->GetActorEyesViewPoint(PhotoEye,PhotoAim);PC->SetControlRotation((DroneActor->GetActorLocation()+FVector(0,0,90)-PhotoEye).Rotation());Click();ApolloTestDeadline=FPlatformTime::Seconds()+30;
 }else{
 if(ApolloPhotoPending&&FPlatformTime::Seconds()<ApolloTestDeadline){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloPolishChecks,.1f,false);return;}
 Check(!ApolloPhotoPending&&SurveyorPhotographed&&ApolloEvidence.Num()==4,TEXT("Framed whole-Surveyor photograph precedes all hardware work"));
 for(int I:{2,0,1}){Aim(ApolloPartActors[I]);P->Interact();Click();CancelApolloTool();Check(!(ApolloParts&(1<<I))&&!ApolloPartActors[I]->IsHidden(),TEXT("Cancelled component removal leaves original hardware visible"));P->Interact();Click();Click();Check(!(ApolloParts&(1<<I)),TEXT("Released fittings alone do not count as packed hardware"));Click();Check((ApolloParts&(1<<I))&&ApolloPartActors[I]->IsHidden(),TEXT("Packed Surveyor component disappears from the original location"));}
 Check(Stage==ELunarStage::ApolloReturn&&ApolloParts==7,TEXT("All real hardware workflows unlock return"));Aim(HomeActor);P->Interact();Click();Check(Stage==ELunarStage::Won,TEXT("Manifest confirmation stows actual cargo and finishes mission"));
 auto* Save=CaptureApolloCheckpoint();FString Why;Check(Save&&Save->Mode==TEXT("historical-exploration")&&Save->Validate(Why),TEXT("Untimed checkpoint uses a validated explicit mode"));
 if(Save){Save->Elapsed=4000;Check(Save->Validate(Why),TEXT("Historical mode accepts elapsed time beyond exercise duration"));Save->Mode=TEXT("timed-exercise");Check(!Save->Validate(Why),TEXT("Timed save rejects impossible exercise telemetry"));}
 auto* S=ULunarSettings::Get();Check(!S->Rebind(TEXT("Interact"),EKeys::Escape)&&!S->Rebind(TEXT("Interact"),EKeys::W),TEXT("Rebinding rejects reserved and duplicate keys"));Check(S->Rebind(TEXT("Interact"),EKeys::B)&&S->Key(TEXT("Interact"))==EKeys::B,TEXT("A free key can be assigned to work"));
 // This subtest starts a new synthetic phase after completing the expedition.
 ALunarCargoFinale::Reset(this);Stage=ELunarStage::ApolloCheckout;ApolloAction=0;S->Apply(P,false);PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::B,IE_Pressed,1.f));PC->PlayerInput->ProcessInputStack({P->InputComponent.Get()},.016f,false);Check(ApolloTool==8,TEXT("Rebound B input opens the actual work tool"));PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::B,IE_Released,0.f));PC->PlayerInput->ProcessInputStack({P->InputComponent.Get()},.016f,false);CancelApolloTool();
 const FString SettingsPath=FPaths::ProjectSavedDir()/TEXT("AutomatedTests/PolishSettings.ini");IFileManager::Get().MakeDirectory(*FPaths::GetPath(SettingsPath),true);S->Sensitivity=1.17f;S->SaveConfig(CPF_Config,*SettingsPath);
 auto* Reload=NewObject<ULunarSettings>();Reload->LoadConfig(nullptr,*SettingsPath);Check(FMath::IsNearlyEqual(Reload->Sensitivity,1.17f)&&Reload->Key(TEXT("Interact"))==EKeys::B,TEXT("Settings and rebound keys survive independent config serialization"));
 S->Reset();S->Apply(P,false);
 Stage=ELunarStage::ApolloCheckout;P->OpenSettings();Check(P->ActivePanel&&Paused,TEXT("Settings panel pauses active gameplay"));P->ClosePanel();Check(!P->ActivePanel&&!Paused,TEXT("Closing settings restores the previous running state"));
 P->PauseMission();P->OpenEvidence();Check(P->ActivePanel&&Paused,TEXT("Evidence album opens from an already paused game"));P->ClosePanel();Check(Paused,TEXT("Closing album preserves an existing pause"));P->PauseMission();
 Check(P->SuitRig&&P->HandRig&&P->SuitRig->GetNumBones()>=36,TEXT("Full suit and first-person hands load real skinned skeletons"));
 ApolloTimed=true;
 for(int Tool:{8,1,5,6}){
  Stage=ELunarStage::ApolloCheckout;Paused=false;Journal=false;ApolloTool=Tool;ApolloToolPhase=1;Oxygen=.5;
  AdvanceTime(1);
  Check(Stage==ELunarStage::Lost&&!ApolloTool&&!ApolloDustMode&&!Journal,TEXT("Time expiry closes unfinished work and reveals the result"));
 }
 Check(S->Rebind(TEXT("Journal"),EKeys::L)&&S->ControlText(TEXT("{Journal} / J Map"))==TEXT("L / J Map"),TEXT("Pause instructions resolve the rebound journal key"));
 S->Rebind(TEXT("Interact"),EKeys::B);S->Rebind(TEXT("Route"),EKeys::N);
 Apollo=false;Stage=ELunarStage::SurveySamples;
 Check(Prompt().Contains(TEXT("B"))&&!Prompt().Contains(TEXT("HOLD E")),TEXT("Legacy work prompt follows the rebound interaction key"));
 Apollo=true;S->Reset();S->Apply(P,false);
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("polish-checks.txt")));UE_LOG(LogTemp,Display,TEXT("APOLLO_POLISH\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);return;
 }
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloPolishChecks,.1f,false);
}
