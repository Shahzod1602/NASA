#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarPauseMenu.h"
#include "LunarMissionCard.h"
#include "LunarFlightHUD.h"
#include "LunarLearning.h"
#include "LunarSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Input/Events.h"

namespace {
struct FNasaUIReviewState {
 FString Folder;TArray<FString> Frames,PointerClicks;int32 Step=0;bool Waiting=false,CapturePaused=false,GameTickBefore=true;FDelegateHandle ScreenshotHandle;
};
using FUIReviewState=TSharedRef<FNasaUIReviewState>;
void PrepareUIReview(ALunarGameMode* G,FUIReviewState State);
void ScheduleUI(ALunarGameMode* G,float Delay,TFunction<void()> Action){FTimerHandle Timer;G->GetWorldTimerManager().SetTimer(Timer,FTimerDelegate::CreateWeakLambda(G,[Action=MoveTemp(Action)](){Action();}),Delay,false);}
void EndUIReview(ALunarGameMode* G,FUIReviewState State,bool Good){
 if(State->ScreenshotHandle.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->ScreenshotHandle);
 ULunarSettings::Get()->TextScale=1.f;G->SetActorTickEnabled(State->GameTickBefore);
 const FString Manifest=TEXT("{\"synthetic\":true,\"human_playtest\":false,\"performance_measurement\":false,\"description\":\"Scripted intro, HUD, pause, settings and field-lesson presentation. Menu intro samples set explicit model phases; menu hover visuals use scripted UButton hover delegates after hit testing (not physical pointer hover); pause clicks use routed Slate events; other mission scenes hold gameplay between screenshots. Lesson choices are scripted UI fixtures, not human learning outcomes. GameMode actor ticking is held while world timers and UMG presentation continue.\",\"pointer_clicks\":[\n")+FString::Join(State->PointerClicks,TEXT(",\n"))+TEXT("\n],\"frames\":[\n")+FString::Join(State->Frames,TEXT(",\n"))+TEXT("\n]}\n");
 Good=FFileHelper::SaveStringToFile(Manifest,*(State->Folder/TEXT("ui-manifest.json")))&&Good;
 UE_LOG(LogTemp,Display,TEXT("NASA_UI_REVIEW %s frames=%d"),Good?TEXT("COMPLETE"):TEXT("FAILED"),State->Frames.Num());FPlatformMisc::RequestExitWithStatus(false,Good?0:1);
}
bool MoveToCard(ULunarMissionCard* Card){
 if(!Card||!Card->Button||!FSlateApplication::IsInitialized())return false;auto& Slate=FSlateApplication::Get();const FGeometry& Geometry=Card->GetCachedGeometry();
 const FVector2D Position=Geometry.LocalToAbsolute(Geometry.GetLocalSize()*.5f);FWidgetPath Path=Slate.LocateWindowUnderMouse(Position,Slate.GetTopLevelWindows());
 const auto Cached=Card->Button->GetCachedWidget();if(!Path.IsValid()||!Cached.IsValid()||!Path.ContainsWidget(Cached.Get()))return false;
 // Offscreen native mouse polling can immediately leave a routed hover.
 // Stage the existing button delegate after verifying its real hit path.
 Card->Button->OnHovered.Broadcast();return true;
}
bool ClickPauseWidget(ALunarGameMode* G,FUIReviewState State,UWidget* Widget,const TCHAR* Action){
 if(!Widget||!FSlateApplication::IsInitialized())return false;
 auto& Slate=FSlateApplication::Get();const auto& Geometry=Widget->GetCachedGeometry();
 const FVector2D Position=Geometry.LocalToAbsolute(Geometry.GetLocalSize()*.5f);FWidgetPath Path=Slate.LocateWindowUnderMouse(Position,Slate.GetTopLevelWindows());const auto Cached=Widget->GetCachedWidget();
 const bool Hit=Path.IsValid()&&Cached.IsValid()&&Path.ContainsWidget(Cached.Get());bool Down=false,Up=false;
 if(Hit){
  TSet<FKey> Pressed,Released;Pressed.Add(EKeys::LeftMouseButton);
  const FPointerEvent Move(0,FSlateApplication::CursorPointerIndex,Position,Position,Released,EKeys::Invalid,0.f,FModifierKeysState());Slate.RoutePointerMoveEvent(Path,Move,false);
  const FPointerEvent Press(0,FSlateApplication::CursorPointerIndex,Position,Position,Pressed,EKeys::LeftMouseButton,0.f,FModifierKeysState());Down=Slate.RoutePointerDownEvent(Path,Press).IsEventHandled();
  const FPointerEvent Release(0,FSlateApplication::CursorPointerIndex,Position,Position,Released,EKeys::LeftMouseButton,0.f,FModifierKeysState());Up=Slate.RoutePointerUpEvent(Path,Release).IsEventHandled();
 }
 const auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));
 State->PointerClicks.Add(FString::Printf(TEXT("{\"action\":\"%s\",\"button_in_hit_path\":%s,\"down_handled\":%s,\"up_handled\":%s,\"paused_after\":%s,\"settings_after\":%s,\"confirmation_after\":%s}"),Action,Hit?TEXT("true"):TEXT("false"),Down?TEXT("true"):TEXT("false"),Up?TEXT("true"):TEXT("false"),G->Paused?TEXT("true"):TEXT("false"),P&&P->ActivePanel&&P->ActivePanel->Settings?TEXT("true"):TEXT("false"),P&&P->PauseMenu&&P->PauseMenu->IsConfirmingRestart()?TEXT("true"):TEXT("false")));
 UE_LOG(LogTemp,Display,TEXT("NASA_UI_POINTER action=%s hit=%d down=%d up=%d"),Action,Hit,Down,Up);
 return Hit&&Down&&Up;
}
void RequestUIShot(ALunarGameMode* G,FUIReviewState State,FString Name){
 if(State->ScreenshotHandle.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->ScreenshotHandle);
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!P){EndUIReview(G,State,false);return;}
 State->Waiting=true;
 State->ScreenshotHandle=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(G,[G,State,Name](){
  if(!State->Waiting)return;State->Waiting=false;
  auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!Pilot)return;
  auto* Main=G->MainMenu.Get();auto* Pause=Pilot->PauseMenu.Get();auto* Panel=Pilot->ActivePanel.Get();const auto HUD=ULunarFlightHUD::ReadState(G,Pilot);
  State->Frames.Add(FString::Printf(TEXT("{\"name\":\"%s\",\"paused\":%s,\"settings_open\":%s,\"field_lessons_open\":%s,\"lesson_reviewed\":%d,\"lesson_matching_choices\":%d,\"pause_actions\":%d,\"restart_confirmation\":%s,\"text_scale\":%.2f,\"hud_visible\":%s,\"hold_fraction\":%.4f,\"intro_elapsed\":%.4f,\"intro_image_alpha\":%.4f,\"intro_layer_alpha\":%.4f}"),
   *Name,G->Paused?TEXT("true"):TEXT("false"),Panel&&Panel->Settings?TEXT("true"):TEXT("false"),Panel&&Panel->ShowFieldLessons?TEXT("true"):TEXT("false"),LunarLearning::Reviewed(G),LunarLearning::Correct(G),Pause?Pause->ActionCards.Num():0,Pause&&Pause->IsConfirmingRestart()?TEXT("true"):TEXT("false"),ULunarSettings::Get()->TextScale,HUD.Visible?TEXT("true"):TEXT("false"),HUD.HoldFraction,Main?Main->IntroState.Elapsed:0,Main?Main->IntroState.ImageAlpha():0,Main?Main->IntroState.LayerAlpha():0));
  const bool HoverFixture=Name==TEXT("home-settings-hover")||Name==TEXT("home-exit-hover")||Name==TEXT("home-mars-hover");
  State->Frames.Last().LeftChopInline(1);State->Frames.Last()+=FString::Printf(TEXT(",\"menu_hover_fixture\":%s}"),HoverFixture?TEXT("true"):TEXT("false"));
  if(!G->Menu)G->Paused=true;
  ScheduleUI(G,.5f,[G,State](){PrepareUIReview(G,State);});
 });
 FScreenshotRequest::RequestScreenshot(State->Folder/(Name+TEXT(".png")),true,false);
}
void ShotUI(ALunarGameMode* G,FUIReviewState State,FString Name){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!P){EndUIReview(G,State,false);return;}
 if(!G->Menu){G->Paused=State->CapturePaused;P->SyncPauseMenu();P->UpdateSuit(0);if(G->FlightHUD)G->FlightHUD->Refresh();}
 const float FadeDelay=!G->Menu&&!State->CapturePaused?.25f:.001f;
 ScheduleUI(G,FadeDelay,[G,State,Name](){RequestUIShot(G,State,Name);});
}
void PrepareUIReview(ALunarGameMode* G,FUIReviewState State){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));auto* PC=UGameplayStatics::GetPlayerController(G,0);if(!P||!PC){EndUIReview(G,State,false);return;}FString Name;float Delay=1.f;
 if(G->Menu){
  auto* Main=G->MainMenu.Get();if(!Main||Main->HomeCards.Num()!=3||Main->ExtraHomeCards.Num()!=2){EndUIReview(G,State,false);return;}
  switch(State->Step++){
   case 0:Main->BeginIntro(true);Main->AdvanceIntro(.20f);Name=TEXT("intro-fade-in");Delay=.001f;break;
   case 1:Main->BeginIntro(true);Main->AdvanceIntro(.90f);Name=TEXT("intro-illustration");Delay=.001f;break;
   case 2:Main->BeginIntro(true);Main->AdvanceIntro(1.90f);Name=TEXT("intro-menu-reveal");Delay=.001f;break;
   case 3:Main->AdvanceIntro(10);Name=TEXT("home-five-glass-actions");break;
   case 4:if(!MoveToCard(Main->ExtraHomeCards[0])){EndUIReview(G,State,false);return;}Name=TEXT("home-settings-hover");Delay=.35f;break;
   case 5:Main->ExtraHomeCards[0]->Button->OnUnhovered.Broadcast();if(!MoveToCard(Main->ExtraHomeCards[1])){EndUIReview(G,State,false);return;}Name=TEXT("home-exit-hover");Delay=.35f;break;
   case 6:Main->ExtraHomeCards[1]->Button->OnUnhovered.Broadcast();if(!MoveToCard(Main->HomeCards[2])){EndUIReview(G,State,false);return;}Name=TEXT("home-mars-hover");Delay=.35f;break;
   default:EndUIReview(G,State,true);return;
  }
 }else{
  switch(State->Step++){
   case 0:
    G->Paused=false;if(G->Apollo)G->StartApollo(true);else G->StartMission();G->BuildMissionInterface();Name=TEXT("mission-compact-hud");State->CapturePaused=false;break;
   case 1:{
    G->Paused=false;P->SyncPauseMenu();G->RadioRemaining=0;
    AActor* Target=nullptr;
    if(G->Apollo){G->Stage=ELunarStage::ApolloSamples;G->ApolloPhotos=1;Target=G->SampleActors.Num()?G->SampleActors[0].Get():nullptr;Name=TEXT("apollo-photographed-sample-guide");}
    else{G->Stage=ELunarStage::CollectData;G->StationRebootComplete=true;G->Oxygen=301;Target=G->DataActor;Name=TEXT("terminal-actual-hold");}
    if(Target){P->SetActorLocation(Target->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator Rotation;P->GetActorEyesViewPoint(Eye,Rotation);PC->SetControlRotation((Target->GetActorLocation()-Eye).Rotation());}
    if(!G->Apollo){P->Interact();G->AdvanceInteraction(1.95f);if(!G->Holding){EndUIReview(G,State,false);return;}}
    State->CapturePaused=false;break;
   }
   case 2:G->Paused=false;P->SyncPauseMenu();P->PauseMission();Name=TEXT("pause-five-actions");State->CapturePaused=true;break;
   case 3:
    if(!P->PauseMenu||P->PauseMenu->ActionCards.Num()!=5||!ClickPauseWidget(G,State,P->PauseMenu->ActionCards[2]->Button,TEXT("pause-restart"))||!P->PauseMenu->IsConfirmingRestart()){EndUIReview(G,State,false);return;}Name=TEXT("pause-restart-confirmation");State->CapturePaused=true;break;
   case 4:
    if(!P->PauseMenu||!ClickPauseWidget(G,State,P->PauseMenu->CancelButton,TEXT("pause-cancel-restart"))||P->PauseMenu->IsConfirmingRestart()){EndUIReview(G,State,false);return;}
    // Give Slate a frame to rebuild the hit grid after collapsing the modal.
    ScheduleUI(G,.1f,[G,State](){PrepareUIReview(G,State);});return;
   case 5:
    if(!P->PauseMenu||!ClickPauseWidget(G,State,P->PauseMenu->ActionCards[1]->Button,TEXT("pause-settings"))||!P->ActivePanel||!P->ActivePanel->Settings){EndUIReview(G,State,false);return;}Name=TEXT("pause-settings");State->CapturePaused=true;break;
   case 6:if(P->ActivePanel)P->ClosePanel();P->SyncPauseMenu();Name=TEXT("pause-settings-return");State->CapturePaused=true;break;
   case 7:
    if(!P->PauseMenu||!ClickPauseWidget(G,State,P->PauseMenu->ActionCards[0]->Button,TEXT("pause-resume"))||G->Paused){EndUIReview(G,State,false);return;}
    P->OpenEvidence();if(!P->ActivePanel){EndUIReview(G,State,false);return;}P->ActivePanel->ToggleFieldLessons();Name=TEXT("field-lesson-question");State->CapturePaused=true;break;
   case 8:
    if(P->ActivePanel){if(LunarLearning::CorrectAnswer(G,0)==0)P->ActivePanel->AnswerFieldLessonA();else P->ActivePanel->AnswerFieldLessonB();}Name=TEXT("field-lesson-explanation");State->CapturePaused=true;break;
   case 9:
    if(P->ActivePanel){P->ActivePanel->NextFieldLesson();if(LunarLearning::CorrectAnswer(G,1)==0)P->ActivePanel->AnswerFieldLessonB();else P->ActivePanel->AnswerFieldLessonA();}Name=TEXT("field-lesson-alternative");State->CapturePaused=true;break;
   case 10:ULunarSettings::Get()->TextScale=1.5f;if(P->ActivePanel)P->ActivePanel->Rebuild();Name=TEXT("field-lesson-text150");State->CapturePaused=true;break;
   case 11:if(P->ActivePanel)P->ClosePanel();if(!G->Paused)P->PauseMission();P->SyncPauseMenu();Name=TEXT("pause-text150");State->CapturePaused=true;break;
   case 12:if(P->ActivePanel)P->ClosePanel();if(P->PauseMenu)P->PauseMenu->Resume();ULunarSettings::Get()->TextScale=1;G->Paused=false;G->Stage=ELunarStage::Lost;G->FailureReason=TEXT("Scripted UI review / session result fixture");G->RadioRemaining=0;Name=TEXT("debrief-session-lessons");State->CapturePaused=false;break;
   case 13:if(!G->Apollo){EndUIReview(G,State,true);return;}ULunarSettings::Get()->TextScale=1.5f;G->Stage=ELunarStage::Won;G->SamplesCollected=G->ApolloParts=G->ApolloPhotos=7;G->ApolloInstruments=3;G->SurveyorPhotographed=true;Name=TEXT("debrief-win-lessons-text150");State->CapturePaused=false;break;
   default:EndUIReview(G,State,true);return;
  }
  P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();P->WorkBlend=G->Holding?1.f:0.f;P->UpdateSuit(0);G->Paused=true;P->SyncPauseMenu();if(G->FlightHUD)G->FlightHUD->Refresh();
 }
 ScheduleUI(G,Delay,[G,State,Name](){ShotUI(G,State,Name);});
}
}
void ALunarGameMode::NasaUIReview(){
 auto State=MakeShared<FNasaUIReviewState>();State->Folder=FPaths::ProjectDir()/TEXT("Reports/NasaTasks20261101")/(Menu?TEXT("ui-intro"):Apollo?TEXT("ui-apollo"):Mars?TEXT("ui-mars"):TEXT("ui-selene"));IFileManager::Get().MakeDirectory(*State->Folder,true);
 State->GameTickBefore=IsActorTickEnabled();SetActorTickEnabled(false);
 ULunarSettings::Get()->TextScale=1.f;PrepareUIReview(this,State);
}
