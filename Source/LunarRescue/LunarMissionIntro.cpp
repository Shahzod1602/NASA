#include "LunarMissionIntro.h"
#include "MoviePlayer.h"
#include "LunarSettings.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Widgets/SLeafWidget.h"
#include <atomic>

namespace {
struct FIntroProgress {std::atomic<bool> Finished{false};std::atomic<int32> SlateTicks{0},GameTicks{0};};
class SLunarIntroGuard : public SLeafWidget {
public:
 SLATE_BEGIN_ARGS(SLunarIntroGuard) {} SLATE_END_ARGS()
 void Construct(const FArguments&,TSharedRef<FIntroProgress,ESPMode::ThreadSafe> InState,IGameMoviePlayer* InPlayer,double InStart){State=InState;Player=InPlayer;Start=InStart;SetCanTick(true);SetVisibility(EVisibility::HitTestInvisible);}
 virtual FVector2D ComputeDesiredSize(float)const override{return FVector2D(1280,720);}
 virtual int32 OnPaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32 Layer,const FWidgetStyle&,bool)const override{return Layer;}
 virtual void Tick(const FGeometry&,double,float)override{
  ++State->SlateTicks;
  // Slate still ticks while the world is held. Stop on the game thread only;
  // the decoder completion delegate can run on a rendering thread.
  if(IsInGameThread()){
   ++State->GameTicks;
   if(State->Finished.load()||FPlatformTime::Seconds()-Start>40){Player->StopMovie();return;}
   // MoviePlayer's world-free wait otherwise busy-spins thousands of Slate
   // ticks per second. Pace the guard to at most 120 Hz, independently of the
   // unchanged 24 fps media clock. This is not a gameplay FPS measurement.
   const double Now=FPlatformTime::Seconds();if(LastPump>0&&Now-LastPump<1./120.)FPlatformProcess::Sleep(float(1./120.-(Now-LastPump)));LastPump=FPlatformTime::Seconds();
  }
 }
private:
 TSharedPtr<FIntroProgress,ESPMode::ThreadSafe> State;IGameMoviePlayer* Player=nullptr;double Start=0,LastPump=0;
};
}
bool LunarMissionIntro::Eligible(const FString& Map,const FString& Options,bool SelectedTravel){
 return SelectedTravel&&(Map==TEXT("MoonBase")||Map==TEXT("Apollo12"))&&!Options.Contains(TEXT("ResumeApollo"),ESearchCase::IgnoreCase)&&!Options.Contains(TEXT("Restart"),ESearchCase::IgnoreCase);
}
void LunarMissionIntro::Play(UWorld* World,const FString& Map){
 if(!World||!FApp::CanEverRender()||!IsMoviePlayerEnabled()||(ULunarSettings::IsAutomation()&&!FParse::Param(FCommandLine::Get(),TEXT("ReviewMissionIntro"))))return;
 const FString Clip=TEXT("MoonArrival");const FString Path=FPaths::ProjectContentDir()/TEXT("Movies")/(Clip+TEXT(".mp4"));
 if(IFileManager::Get().FileSize(*Path)<=0){UE_LOG(LogTemp,Display,TEXT("MISSION_INTRO missing optional MoonArrival.mp4; direct mission entry"));return;}
 const double WorldTime=World->GetTimeSeconds(),Start=FPlatformTime::Seconds();auto* Player=GetMoviePlayer();auto State=MakeShared<FIntroProgress,ESPMode::ThreadSafe>();
 FLoadingScreenAttributes Attr;Attr.MoviePaths.Add(Clip);Attr.PlaybackType=EMoviePlaybackType::MT_Normal;
 Attr.bAutoCompleteWhenLoadingCompletes=false;Attr.bWaitForManualStop=true;Attr.bMoviesAreSkippable=!FParse::Param(FCommandLine::Get(),TEXT("ReviewMissionIntro"));Attr.bAllowEngineTick=false;
 Attr.WidgetLoadingScreen=SNew(SLunarIntroGuard,State,Player,Start);
 const auto FinishedHandle=Player->OnMovieClipFinished().AddLambda([State](const FString& Name){State->Finished=true;UE_LOG(LogTemp,Display,TEXT("MISSION_INTRO clip_finished=%s"),*Name);});
 Player->SetupLoadingScreen(Attr);const bool Started=Player->PlayMovie();
 UE_LOG(LogTemp,Display,TEXT("MISSION_INTRO begin map=%s started=%d movie=%s original=854x480 engine_tick=0"),*Map,Started,*Player->GetMovieName());
 if(Started)Player->WaitForMovieToFinish(false);Player->OnMovieClipFinished().Remove(FinishedHandle);
 UE_LOG(LogTemp,Display,TEXT("MISSION_INTRO end elapsed=%.3f clip_finished=%d slate_ticks=%d game_ticks=%d world_time_held=%d"),FPlatformTime::Seconds()-Start,State->Finished.load(),State->SlateTicks.load(),State->GameTicks.load(),FMath::IsNearlyEqual(WorldTime,double(World->GetTimeSeconds()),.0001));
}
