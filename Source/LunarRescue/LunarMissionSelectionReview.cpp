#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarMissionCard.h"
#include "LunarSettings.h"
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
// This harness stages only menu presentation. It never launches a mission,
// accepts a checkpoint, alters cargo, or writes an in-game photograph.
struct FMissionSelectionReviewState {
 FString Folder;
 TArray<FString> Frames;
 TArray<FString> PointerClicks;
 FDelegateHandle ScreenshotHandle;
 bool WaitingForScreenshot=false;
 int32 Step=0;
 double SelectedAt=0;
};
using FReviewState=TSharedRef<FMissionSelectionReviewState>;
void ReviewStep(ALunarGameMode* Owner,FReviewState State);
void Schedule(ALunarGameMode* Owner,FReviewState State,float Delay){
 FTimerHandle Handle;
 Owner->GetWorldTimerManager().SetTimer(Handle,FTimerDelegate::CreateWeakLambda(Owner,[Owner,State](){ReviewStep(Owner,State);}),Delay,false);
}
void Finish(ALunarGameMode* Owner,FReviewState State,bool Success){
 if(State->ScreenshotHandle.IsValid()){FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->ScreenshotHandle);State->ScreenshotHandle.Reset();}
 ULunarSettings::Get()->TextScale=1.f;
 if(Success){
  const FString Manifest=TEXT("{\n\"synthetic\":true,\"human_playtest\":false,\"performance_measurement\":false,\n")
   TEXT("\"description\":\"Staged menu states and naturally ticking selection transitions. No mission was launched and no checkpoint was written. Values are sampled when the screenshot request is processed; timer intervals are visual samples, not a frame-time or performance benchmark.\",\n")
   TEXT("\"pointer_clicks\":[\n")+FString::Join(State->PointerClicks,TEXT(",\n"))+TEXT("\n],\n\"frames\":[\n")+FString::Join(State->Frames,TEXT(",\n"))+TEXT("\n]}\n");
  Success=FFileHelper::SaveStringToFile(Manifest,*(State->Folder/TEXT("review-manifest.json")));
 }
 UE_LOG(LogTemp,Display,TEXT("MISSION_SELECTION_REVIEW %s"),Success?TEXT("COMPLETE"):TEXT("FAILED"));
 FPlatformMisc::RequestExitWithStatus(false,Success?0:1);
}
void Capture(ALunarGameMode* Owner,FReviewState State,const TCHAR* Name,float NextDelay=1.f){
 const FString FrameName=Name;
 const double RequestedAt=Owner->GetWorld()->GetTimeSeconds();
 // Waiting for request completion prevents a slow render from replacing an
 // uncaptured screenshot request. The next one-shot timer starts afterward.
 // Remove the previous callback between frames, never from inside its own
 // invocation: removing it there destroys the lambda's captured shared state.
 if(State->ScreenshotHandle.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->ScreenshotHandle);
 State->WaitingForScreenshot=true;
 State->ScreenshotHandle=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(Owner,[Owner,State,FrameName,RequestedAt,NextDelay](){
  if(!State->WaitingForScreenshot)return;
  State->WaitingForScreenshot=false;
  auto* Main=Owner->MainMenu.Get();
  if(!Main||Main->MissionCards.Num()!=3||Main->HomeCards.Num()!=3){Finish(Owner,State,false);return;}
  const double Now=Owner->GetWorld()->GetTimeSeconds();
  State->Frames.Add(FString::Printf(TEXT("{\"name\":\"%s\",\"mission_selection\":%s,\"selected_mission\":%d,\"text_scale\":%.2f,\"requested_world_seconds\":%.6f,\"processed_world_seconds\":%.6f,\"since_selection_seconds\":%.6f,\"mission_blends\":[%.6f,%.6f,%.6f],\"home_blends\":[%.6f,%.6f,%.6f]}"),
   *FrameName,Main->MissionSelection?TEXT("true"):TEXT("false"),Main->SelectedMission,ULunarSettings::Get()->TextScale,RequestedAt,Now,State->SelectedAt>0?Now-State->SelectedAt:0,
   Main->MissionCards[0]->SelectionBlend(),Main->MissionCards[1]->SelectionBlend(),Main->MissionCards[2]->SelectionBlend(),Main->HomeCards[0]->SelectionBlend(),Main->HomeCards[1]->SelectionBlend(),Main->HomeCards[2]->SelectionBlend()));
  Schedule(Owner,State,NextDelay);
 });
 FScreenshotRequest::RequestScreenshot(State->Folder/(FrameName+TEXT(".png")),true,false);
}
bool ClickMissionDescription(ALunarGameMode* Owner,FReviewState State,int32 Index){
 auto* Main=Owner->MainMenu.Get();
 if(!Main||!Main->MissionCards.IsValidIndex(Index)||!FSlateApplication::IsInitialized())return false;
 auto* Card=Main->MissionCards[Index].Get();
 if(!Card||!Card->Button)return false;
 auto& Slate=FSlateApplication::Get();
 const FGeometry& Geometry=Card->GetCachedGeometry();
 const FVector2D Size=Geometry.GetLocalSize();
 // The description region contains a ScrollBox. Hit-test its actual rendered
 // location, rather than broadcasting OnClicked or constructing a button path.
 const FVector2D Position=Geometry.LocalToAbsolute(FVector2D(Size.X*.5f,Size.Y*.56f));
 FWidgetPath Path=Slate.LocateWindowUnderMouse(Position,Slate.GetTopLevelWindows());
 const auto ButtonWidget=Card->Button->GetCachedWidget();
 const bool CardHit=Size.X>0&&Size.Y>0&&Path.IsValid()&&ButtonWidget.IsValid()&&Path.ContainsWidget(ButtonWidget.Get());
 bool DownHandled=false,UpHandled=false;
 FString Leaf=Path.IsValid()?Path.GetLastWidget()->GetTypeAsString():TEXT("None");
 if(CardHit){
  TSet<FKey> Pressed;Pressed.Add(EKeys::LeftMouseButton);TSet<FKey> Released;
  const FPointerEvent Down(0,FSlateApplication::CursorPointerIndex,Position,Position,Pressed,EKeys::LeftMouseButton,0.f,FModifierKeysState());
  const FPointerEvent Up(0,FSlateApplication::CursorPointerIndex,Position,Position,Released,EKeys::LeftMouseButton,0.f,FModifierKeysState());
  const FPointerEvent Move(0,FSlateApplication::CursorPointerIndex,Position,Position,Released,EKeys::Invalid,0.f,FModifierKeysState());
  // A real click first enters the widget. Routing down/up alone omits the
  // hover path that SButton uses to accept a release inside its bounds.
  Slate.RoutePointerMoveEvent(Path,Move,false);
  DownHandled=Slate.RoutePointerDownEvent(Path,Down).IsEventHandled();
  UpHandled=Slate.RoutePointerUpEvent(Path,Up).IsEventHandled();
  const FVector2D Away=Main->GetCachedGeometry().LocalToAbsolute(FVector2D(20,20));
  FWidgetPath AwayPath=Slate.LocateWindowUnderMouse(Away,Slate.GetTopLevelWindows());
  const FPointerEvent Leave(0,FSlateApplication::CursorPointerIndex,Away,Position,Released,EKeys::Invalid,0.f,FModifierKeysState());
  Slate.RoutePointerMoveEvent(AwayPath,Leave,false);
 }
 const bool Selected=CardHit&&DownHandled&&UpHandled&&Main->SelectedMission==Index;
 State->PointerClicks.Add(FString::Printf(TEXT("{\"mission_index\":%d,\"region\":\"description_scroll_content\",\"route\":\"LocateWindowUnderMouse, RoutePointerMoveEvent, RoutePointerDownEvent/RoutePointerUpEvent, pointer exit\",\"leaf_widget\":\"%s\",\"button_in_hit_path\":%s,\"down_handled\":%s,\"up_handled\":%s,\"selected\":%s,\"absolute_position\":[%.3f,%.3f]}"),Index,*Leaf,CardHit?TEXT("true"):TEXT("false"),DownHandled?TEXT("true"):TEXT("false"),UpHandled?TEXT("true"):TEXT("false"),Selected?TEXT("true"):TEXT("false"),Position.X,Position.Y));
 UE_LOG(LogTemp,Display,TEXT("MISSION_SELECTION_POINTER index=%d hit=%d down=%d up=%d selected=%d leaf=%s"),Index,CardHit,DownHandled,UpHandled,Selected,*Leaf);
 return Selected;
}
void ReviewStep(ALunarGameMode* Owner,FReviewState State){
 auto* Main=Owner->MainMenu.Get();
 if(!Owner->Menu||!Main||Main->MissionCards.Num()!=3||Main->HomeCards.Num()!=3){Finish(Owner,State,false);return;}
 switch(State->Step++){
  case 0:Capture(Owner,State,TEXT("home"));break;
  case 1:Main->ExploreMoon();Schedule(Owner,State,1.f);break;
  case 2:Capture(Owner,State,TEXT("moon-unselected"));break;
  case 3:if(!ClickMissionDescription(Owner,State,0)){Finish(Owner,State,false);return;}State->SelectedAt=Owner->GetWorld()->GetTimeSeconds();Capture(Owner,State,TEXT("transition-00"),.06f);break;
  case 4:Capture(Owner,State,TEXT("transition-01"),.06f);break;
  case 5:Capture(Owner,State,TEXT("transition-02"),.06f);break;
  case 6:Capture(Owner,State,TEXT("transition-03"),.06f);break;
  case 7:Capture(Owner,State,TEXT("transition-04"),.12f);break;
  case 8:Capture(Owner,State,TEXT("transition-05"));break;
  case 9:Capture(Owner,State,TEXT("moon-apollo-selected"));break;
  case 10:if(!ClickMissionDescription(Owner,State,1)){Finish(Owner,State,false);return;}State->SelectedAt=Owner->GetWorld()->GetTimeSeconds();Schedule(Owner,State,1.f);break;
  case 11:Capture(Owner,State,TEXT("moon-selene-selected"));break;
  case 12:Main->Back();Main->ExploreMars();State->SelectedAt=0;Schedule(Owner,State,1.f);break;
  case 13:Capture(Owner,State,TEXT("mars-unselected"));break;
  case 14:if(!ClickMissionDescription(Owner,State,2)){Finish(Owner,State,false);return;}State->SelectedAt=Owner->GetWorld()->GetTimeSeconds();Schedule(Owner,State,1.f);break;
  case 15:Capture(Owner,State,TEXT("mars-ares-selected"));break;
  case 16:Main->Back();Main->ExploreMoon();Main->SelectMission(0);State->SelectedAt=Owner->GetWorld()->GetTimeSeconds();ULunarSettings::Get()->TextScale=1.5f;Schedule(Owner,State,1.f);break;
  case 17:Capture(Owner,State,TEXT("moon150-selected"));break;
  default:Finish(Owner,State,true);break;
 }
}
}
void ALunarGameMode::MissionSelectionReview(){
 auto State=MakeShared<FMissionSelectionReviewState>();
 State->Folder=FPaths::ProjectDir()/TEXT("Reports/MissionSelection20261031/review");
 IFileManager::Get().MakeDirectory(*State->Folder,true);
 ULunarSettings::Get()->TextScale=1.f;
 ReviewStep(this,State);
}
