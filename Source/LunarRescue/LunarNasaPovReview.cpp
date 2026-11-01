#include "LunarGame.h"
#include "LunarCargoFinale.h"
#include "LunarFlightHUD.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace {
struct FPovReview {
 FString Folder;TArray<FString> Frames;int32 Step=0;
 bool Mars=false,Waiting=false,GameTickBefore=true;FDelegateHandle ScreenshotHandle;
};
using FPovState=TSharedRef<FPovReview>;
void Prepare(ALunarGameMode* G,FPovState State);
void Later(ALunarGameMode* G,FPovState State,float Delay,TFunction<void()> Action){
 FTimerHandle Timer;G->GetWorldTimerManager().SetTimer(Timer,FTimerDelegate::CreateWeakLambda(G,[Action=MoveTemp(Action)](){Action();}),Delay,false);
}
void End(ALunarGameMode* G,FPovState State,bool Good){
 if(State->ScreenshotHandle.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->ScreenshotHandle);
 ALunarCargoFinale::Reset(G);G->Paused=false;G->SetActorTickEnabled(State->GameTickBefore);
 const FString Json=TEXT("{\"synthetic\":true,\"human_playtest\":false,\"performance_measurement\":false,\"planet\":\"")+FString(State->Mars?TEXT("Mars"):TEXT("Moon"))+TEXT("\",\"description\":\"Staged actual-world POV/work states. Gameplay is held during preparation and exposed for a single screenshot frame. GameMode ticking is held while world timers and UMG fade continue. No human task completion, recorded science result or FPS is inferred.\",\"frames\":[\n")+FString::Join(State->Frames,TEXT(",\n"))+TEXT("\n]}\n");
 Good=FFileHelper::SaveStringToFile(Json,*(State->Folder/TEXT("pov-manifest.json")))&&Good;
 UE_LOG(LogTemp,Display,TEXT("NASA_POV_REVIEW %s frames=%d"),Good?TEXT("COMPLETE"):TEXT("FAILED"),State->Frames.Num());FPlatformMisc::RequestExitWithStatus(false,Good?0:1);
}
void AimAt(ALunarGameMode* G,ALunarCharacter* P,AActor* Target){
 if(!Target)return;
 FVector Position=Target->GetActorLocation()+FVector(-180,0,0);FHitResult Ground;FCollisionQueryParams Query;Query.AddIgnoredActor(P);
 if(G->GetWorld()->LineTraceSingleByChannel(Ground,Position+FVector(0,0,1000),Position-FVector(0,0,1500),ECC_Visibility,Query))Position.Z=Ground.ImpactPoint.Z+P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
 P->SetActorLocation(Position);P->LastStepPosition=Position;FVector Eye;FRotator Rotation;P->GetActorEyesViewPoint(Eye,Rotation);
 if(auto* PC=UGameplayStatics::GetPlayerController(G,0))PC->SetControlRotation((Target->GetActorLocation()-Eye).Rotation());
}
void RequestPovShot(ALunarGameMode* G,FPovState State,FString Name){
 if(State->ScreenshotHandle.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->ScreenshotHandle);
 State->Waiting=true;
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!P){End(G,State,false);return;}
 State->ScreenshotHandle=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(G,[G,State,Name](){
  if(!State->Waiting)return;State->Waiting=false;G->Paused=true;
  auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!Pilot)return;
  bool BodyVisible=false;for(int32 I=4;I<10;++I)BodyVisible|=Pilot->SuitBody[I]->IsVisible();
  State->Frames.Add(FString::Printf(TEXT("{\"name\":\"%s\",\"player_fov\":%.2f,\"camera_pitch\":%.2f,\"hands_visible\":%s,\"leg_proxies_visible\":%s,\"tool\":%d,\"phase\":%d,\"selected_sample\":%d,\"holding\":%s,\"hold_progress\":%.4f,\"stage\":%d}"),*Name,Pilot->Camera->FieldOfView,Pilot->Camera->GetComponentRotation().Pitch,Pilot->HandRig&&Pilot->HandRig->IsVisible()?TEXT("true"):TEXT("false"),BodyVisible?TEXT("true"):TEXT("false"),G->ApolloTool,G->ApolloToolPhase,G->SelectedSample,G->Holding?TEXT("true"):TEXT("false"),G->HoldProgress,int32(G->Stage)));
  Later(G,State,.1f,[G,State](){Prepare(G,State);});
 });
 FScreenshotRequest::RequestScreenshot(State->Folder/(Name+TEXT(".png")),true,false);
}
void Capture(ALunarGameMode* G,FPovState State,FString Name){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!P){End(G,State,false);return;}
 G->Paused=false;P->SyncPauseMenu();P->UpdateSuit(0);if(G->FlightHUD)G->FlightHUD->Refresh();
 Later(G,State,.25f,[G,State,Name](){RequestPovShot(G,State,Name);});
}
void Prepare(ALunarGameMode* G,FPovState State){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));auto* PC=UGameplayStatics::GetPlayerController(G,0);if(!P||!PC){End(G,State,false);return;}
 G->Paused=false;FString Name;
 if(State->Mars){
  switch(State->Step++){
   case 0:G->StartMission();G->Stage=ELunarStage::RestorePower;G->RepairStep=0;AimAt(G,P,G->PowerActor);P->CarriedModule->SetVisibility(true);Name=TEXT("mars-cell-carry");break;
   case 1:G->Stage=ELunarStage::RestorePower;G->RepairStep=2;G->Holding=true;G->HoldProgress=.85f;G->HeldTarget=G->PowerActor;AimAt(G,P,G->PowerActor);P->CarriedModule->SetVisibility(false);Name=TEXT("mars-station-restart");break;
   case 2:G->Stage=ELunarStage::CollectData;G->StationRebootComplete=true;G->HoldProgress=1.95f;G->HeldTarget=G->DataActor;G->Oxygen=301;AimAt(G,P,G->DataActor);Name=TEXT("mars-science-download");break;
   case 3:G->Stage=ELunarStage::AlignRelay;G->HoldProgress=2.3f;G->HeldTarget=G->RelayActor;AimAt(G,P,G->RelayActor);Name=TEXT("mars-relay-alignment");break;
   case 4:G->Stage=ELunarStage::ReturnHome;G->TrackRecorder=true;G->HoldProgress=.8f;G->HeldTarget=G->RecorderActor;AimAt(G,P,G->RecorderActor);Name=TEXT("mars-recorder");break;
   case 5:G->ReleaseInteract();G->Stage=ELunarStage::FindModule;PC->SetControlRotation(FRotator(-75,0,0));P->Camera->SetWorldRotation(PC->GetControlRotation());Name=TEXT("mars-free-lookdown");break;
   default:End(G,State,true);return;
  }
 }else{
  switch(State->Step++){
   case 0:G->StartApollo(true);P->GetCharacterMovement()->DisableMovement();P->Camera->SetFieldOfView(70);PC->SetControlRotation(FRotator(-75,0,0));P->Camera->SetWorldRotation(PC->GetControlRotation());Name=TEXT("moon-lookdown-fov70");break;
   case 1:P->Camera->SetFieldOfView(85);Name=TEXT("moon-lookdown-fov85");break;
   case 2:P->Camera->SetFieldOfView(110);Name=TEXT("moon-lookdown-fov110");break;
   case 3:P->Camera->SetFieldOfView(85);PC->SetControlRotation(FRotator::ZeroRotator);P->Camera->SetWorldRotation(PC->GetControlRotation());Name=TEXT("moon-free-hands");break;
   case 4:{
    G->Stage=ELunarStage::ApolloSamples;G->ApolloPhotos=2;AimAt(G,P,G->SampleActors[1]);
    if(!G->OpenApolloTool()){End(G,State,false);return;}G->ApolloToolClick();G->ApolloToolCooldown=0;Name=TEXT("soil-step2-fov85");break;
   }
   case 5:P->Camera->SetFieldOfView(70);Name=TEXT("soil-step2-fov70");break;
   case 6:P->Camera->SetFieldOfView(110);Name=TEXT("soil-step2-fov110");break;
   case 7:P->Camera->SetFieldOfView(85);G->ApolloToolCooldown=.55f;Name=TEXT("soil-dig-motion");break;
   case 8:G->ApolloToolCooldown=.25f;Name=TEXT("soil-lift-motion");break;
   case 9:G->CancelApolloTool();G->ApolloPackedSample=1;G->ApolloPackedPreviewRemaining=1.95f;Name=TEXT("soil-deposit");break;
   case 10:G->ApolloPackedPreviewRemaining=1.2f;Name=TEXT("soil-seal");break;
   case 11:G->ApolloPackedPreviewRemaining=.25f;Name=TEXT("soil-lower");break;
   case 12:G->ApolloPackedPreviewRemaining=0;G->Stage=ELunarStage::Won;G->SamplesCollected=G->ApolloParts=7;ALunarCargoFinale::Begin(G);if(G->CargoFinale)G->CargoFinale->AdvancePresentation(.65f);Name=TEXT("cargo-approach");break;
   case 13:if(G->CargoFinale)G->CargoFinale->AdvancePresentation(2.3f);Name=TEXT("cargo-middle");break;
   case 14:if(G->CargoFinale)G->CargoFinale->AdvancePresentation(3.1f);Name=TEXT("cargo-settled");break;
   default:End(G,State,true);return;
  }
 }
 if(G->CargoFinale)G->CargoFinale->SetActorTickEnabled(false);P->GetCharacterMovement()->DisableMovement();
 G->RadioRemaining=0;P->WorkBlend=G->Holding||G->ApolloTool||G->ApolloPackedPreviewRemaining>0||(!G->Apollo&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==0)?1.f:0.f;
 P->UpdateSuit(0);G->Paused=true;
 Later(G,State,1.f,[G,State,Name](){Capture(G,State,Name);});
}
}
void ALunarGameMode::NasaPovReview(){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!P||(!Apollo&&!Mars)){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto State=MakeShared<FPovReview>();State->Mars=Mars;State->Folder=FPaths::ProjectDir()/TEXT("Reports/NasaTasks20261101")/(Mars?TEXT("pov-mars"):TEXT("pov-moon"));IFileManager::Get().MakeDirectory(*State->Folder,true);
 State->GameTickBefore=IsActorTickEnabled();SetActorTickEnabled(false);
 P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();P->ThirdPerson=false;P->Camera->bUsePawnControlRotation=true;P->Camera->SetFieldOfView(85);Prepare(this,State);
}
