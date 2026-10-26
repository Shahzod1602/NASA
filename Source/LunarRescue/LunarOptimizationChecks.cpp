#include "LunarGame.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "LunarSettings.h"
#include "LunarPanels.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/AudioComponent.h"

// Rendered integration checks: real screenshot pixels, asynchronous completion,
// an unwritable destination, retry, and bounded cache retention across GC.
void ALunarGameMode::ApolloOptimizationChecks(){
 auto Check=[&](bool Good,const TCHAR* Name){MovementReport+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++MovementFailures;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC||!FApp::CanEverRender()){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto AlbumContains=[&](const TCHAR* Text){bool Found=false;if(P->ActivePanel)P->ActivePanel->WidgetTree->ForEachWidget([&](UWidget* W){if(auto* L=Cast<UTextBlock>(W))Found|=L->GetText().ToString().Contains(Text);});return Found;};
 auto WaitForPhoto=[&](){
  if(!ApolloPhotoPending)return false;
  if(FPlatformTime::Seconds()>=ApolloTestDeadline){Check(false,TEXT("Photograph completed within 30 seconds"));MovementReport+=TEXT("RESULT: 1 failures\n");FFileHelper::SaveStringToFile(MovementReport,*ReportPath(TEXT("optimization-checks.txt")));FPlatformMisc::RequestExitWithStatus(false,1);return true;}
  FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloOptimizationChecks,.1f,false);return true;
 };
 auto Relative=[&](int I){return TEXT("Apollo12/Photos/")+ApolloRunId.ToString(EGuidFormats::Digits)+FString::Printf(TEXT("/optimization-%d.png"),I);};
 if(MovementPhase==0){
  StartApollo(true);Stage=ELunarStage::ApolloSamples;ApolloInstruments=3;RadioRemaining=0;
  P->GetCharacterMovement()->DisableMovement();P->SetActorLocation(SampleActors[0]->GetActorLocation()+FVector(-170,0,60));
  PC->SetControlRotation((SampleActors[0]->GetActorLocation()-P->Camera->GetComponentLocation()).Rotation());
  auto* Settings=ULunarSettings::Get();Settings->EffectsVolume=.4f;P->StampFootstep();
  Check(IsValid(P->StepAudio)&&FMath::IsNearlyEqual(P->StepAudio->VolumeMultiplier,.28f),TEXT("Actual footstep component respects suit volume"));
  Settings->EffectsVolume=0;P->Tick(0);
  Check(IsValid(P->StepAudio)&&P->StepAudio->VolumeMultiplier==0,TEXT("Changing suit volume mutes the current footstep"));
  Settings->RadioVolume=.3f;Apollo=false;Mars=true;Message(TEXT("NAV: test"));
  Check(IsValid(RadioAudio)&&FMath::IsNearlyEqual(RadioAudio->VolumeMultiplier,.3f),TEXT("Legacy radio starts at the configured volume"));
  Settings->RadioVolume=0;Paused=true;UpdateAudioPause();
  Check(IsValid(RadioAudio)&&RadioAudio->VolumeMultiplier==0,TEXT("Legacy radio volume changes while paused"));
  Paused=false;Apollo=true;Mars=false;Settings->Reset();UpdateAudioPause();
 }
 if(MovementPhase<8){
  const int I=MovementPhase/2;
  if(MovementPhase%2==0){
   Check(RequestApolloPhoto(I,Relative(I))&&ApolloPhotoPending,TEXT("A rendered photograph starts asynchronously"));
   ApolloTestDeadline=FPlatformTime::Seconds()+30;
   Check(ApolloEvidence.Num()==I&&!RequestApolloPhoto(I,Relative(I)),TEXT("Pending photograph is not awarded and cannot be overwritten"));
   if(I==0){const auto OldStage=Stage;const auto OldRun=ApolloRunId;Check(!RestoreApolloCheckpoint()&&Stage==OldStage&&ApolloRunId==OldRun&&ApolloPhotoPending&&ApolloSaveNotice==TEXT("CheckpointPhotoPending"),TEXT("Reload waits for pending photo completion without changing the run"));}
   P->OpenEvidence();if(I>0)P->ActivePanel->EvidenceIndex=I-1;P->ActivePanel->Rebuild();
   Check(Paused&&AlbumContains(TEXT("still being saved")),TEXT("Album opened during capture shows the pending state"));
  }else{
   if(WaitForPhoto())return;
   Check(ApolloEvidence.Num()==I+1&&IFileManager::Get().FileSize(*(FPaths::ProjectSavedDir()/Relative(I)))>100,TEXT("Evidence is committed only after its PNG exists"));
   Check(P->ActivePanel&&P->ActivePanel->Photo&&P->ActivePanel->EvidenceIndex==FMath::Max(0,I-1)&&!AlbumContains(TEXT("still being saved"))&&AlbumContains(*FString::Printf(TEXT("Evidence photographs: %d."),I+1))&&Paused,TEXT("Open album refreshes its count and photo while preserving selection and pause"));
   P->ClosePanel();
  }
 }else if(MovementPhase==8){
  auto* First=P->LoadEvidencePhoto(Relative(0));
  Check(First&&P->LoadEvidencePhoto(Relative(0))==First,TEXT("Repeated album navigation reuses the decoded texture"));
  P->LoadEvidencePhoto(Relative(1));P->LoadEvidencePhoto(Relative(2));P->LoadEvidencePhoto(Relative(0));P->LoadEvidencePhoto(Relative(3));
  Check(P->EvidencePhotoCache.Num()==3&&P->EvidencePhotoCache.Contains(Relative(0))&&!P->EvidencePhotoCache.Contains(Relative(1)),TEXT("Album retains at most three textures and evicts the least recently viewed"));
  CollectGarbage(RF_NoFlags);
  Check(IsValid(First)&&P->LoadEvidencePhoto(Relative(0))==First,TEXT("Cached photo remains valid after garbage collection"));
  Check(!P->LoadEvidencePhoto(TEXT("../outside.png"))&&!P->LoadEvidencePhoto(TEXT("Apollo12/Photos/another-run/photo.png")),TEXT("Album rejects paths outside the current mission"));
  IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir()/Relative(4)),true);
  Check(RequestApolloPhoto(4,Relative(4)),TEXT("An unwritable photo destination reaches the real asynchronous writer"));
  ApolloTestDeadline=FPlatformTime::Seconds()+30;P->OpenEvidence();
 }else if(MovementPhase==9){
  if(WaitForPhoto())return;
  Check(ApolloEvidence.Num()==4&&!ApolloDustPhotos,TEXT("Write failure leaves evidence and progress unchanged"));
  Check(AlbumContains(TEXT("Photo could not be saved"))&&!AlbumContains(TEXT("still being saved")),TEXT("An open album reports write failure without a stale saving banner"));P->ClosePanel();
  IFileManager::Get().DeleteDirectory(*(FPaths::ProjectSavedDir()/Relative(4)));
  Check(RequestApolloPhoto(4,Relative(4)),TEXT("Failed photograph can be retried"));
  ApolloTestDeadline=FPlatformTime::Seconds()+30;
 }else{
  if(WaitForPhoto())return;
  Check(ApolloEvidence.Num()==5&&ApolloDustPhotos==1,TEXT("Retry commits exactly one photo after successful writing"));
  MovementReport+=FString::Printf(TEXT("RESULT: %d failures\n"),MovementFailures);
  FFileHelper::SaveStringToFile(MovementReport,*ReportPath(TEXT("optimization-checks.txt")));
  UE_LOG(LogTemp,Display,TEXT("APOLLO_OPTIMIZATION\n%s"),*MovementReport);
  FPlatformMisc::RequestExitWithStatus(false,MovementFailures?1:0);return;
 }
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloOptimizationChecks,.35f,false);
}
