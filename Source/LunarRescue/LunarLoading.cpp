#include "UObject/Package.h"
#include "LunarLoading.h"
#include "LunarMissionIntro.h"
#include "LunarLoadingTypography.h"
#include "LunarSettings.h"
#include "MoviePlayer.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Styling/CoreStyle.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformProcess.h"
#include <atomic>

namespace {
struct FLoadingProgress {
 // Targets represent completed preparation stages, not package bytes.
 std::atomic<int32> Target{0}, Display{0}, ZeroFrames{0}, ReadyFrames{0};
 std::atomic<bool> Monotonic{true};
};

class SLunarLoading : public SLeafWidget {
public:
 SLATE_BEGIN_ARGS(SLunarLoading):_StartupScreen(false) {} SLATE_ARGUMENT(bool,StartupScreen) SLATE_END_ARGS()
 void Construct(const FArguments& Args, UTexture2D* Texture, const FString& Map,
                TSharedRef<FLoadingProgress, ESPMode::ThreadSafe> InProgress,float TextScale,bool IsPreview=false) {
  Progress=InProgress;Preview=IsPreview;Startup=Args._StartupScreen;Layout=LunarLoading::BuildTypography(Map,TextScale);
  Art.SetResourceObject(Texture);Art.ImageSize=FVector2D(1280,720);Art.DrawAs=ESlateBrushDrawType::Image;
  White=FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
  SetCanTick(!Preview);
 }
 virtual FVector2D ComputeDesiredSize(float) const override {return FVector2D(1280,720);}
 virtual void Tick(const FGeometry&,double,float Delta) override {
  if(Preview)return;
  const int32 Before=Progress->Display.load();
  // Give the first zero state at least three Slate frames, including warm loads.
  if(Ticks++>=3)Value=Startup?float(Progress->Target.load()):FMath::Min(float(Progress->Target.load()),Value+FMath::Clamp(Delta,0.f,.1f)*70.f);
  const int32 Next=FMath::Clamp(FMath::FloorToInt(Value+.001f),0,100);
  if(Next<Before)Progress->Monotonic=false;
  Progress->Display=Next;
  Invalidate(EInvalidateWidgetReason::Paint);
 }
 virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect& Clip,
  FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle&,bool) const override {
  auto Box=[&](FVector2D At,FVector2D Size,const FSlateBrush* Brush,FLinearColor Color){
   FSlateDrawElement::MakeBox(Out,Layer++,G.ToPaintGeometry(Size,FSlateLayoutTransform(At)),Brush,ESlateDrawEffect::None,Color);
  };
  if(Startup){
   Box(FVector2D::ZeroVector,FVector2D(1280,720),White,FLinearColor::Black);
   if(Art.GetResourceObject())Box(FVector2D(0,26),FVector2D(1280,668),&Art,FLinearColor::White);
   else {FSlateDrawElement::MakeText(Out,Layer++,G.ToPaintGeometry(FVector2D(640,85),FSlateLayoutTransform(FVector2D(500,295))),TEXT("AstroKid"),FCoreStyle::GetDefaultFontStyle("Regular",44),ESlateDrawEffect::None,FLinearColor::White);FSlateDrawElement::MakeText(Out,Layer++,G.ToPaintGeometry(FVector2D(700,60),FSlateLayoutTransform(FVector2D(452,372))),TEXT("E X P E D I T I O N S"),FCoreStyle::GetDefaultFontStyle("Regular",19),ESlateDrawEffect::None,FLinearColor(.68f,.79f,.85f));}
   // Replace only the reference's baked spinner with a live Slate indicator.
   Box(FVector2D(1136,606),FVector2D(64,64),White,FLinearColor::Black);
   TArray<FVector2D> Ring;const double Phase=FPlatformTime::Seconds()*3.;
   for(int32 I=0;I<=40;++I){const double A=Phase+I*(PI*1.5/40);Ring.Add(FVector2D(1168,634)+FVector2D(FMath::Cos(A),FMath::Sin(A))*13);}
   FSlateDrawElement::MakeLines(Out,Layer++,G.ToPaintGeometry(),Ring,ESlateDrawEffect::None,FLinearColor(.92f,.97f,1),true,3);
   if(Progress->Display.load()==0)++Progress->ZeroFrames;if(Progress->Display.load()==100)++Progress->ReadyFrames;
   return Layer;
  }
  const FLinearColor Accent(.025f,.85f,.9f);
  Box(FVector2D::ZeroVector,FVector2D(1280,720),White,FLinearColor(.002f,.006f,.012f));
  if(Art.GetResourceObject())Box(FVector2D::ZeroVector,FVector2D(1280,720),&Art,FLinearColor::White);
  Box(FVector2D::ZeroVector,FVector2D(Layout.HeaderWidth,Layout.HeaderHeight),White,FLinearColor(0,.004f,.008f,.72f));
  Box(FVector2D(0,Layout.FooterTop),FVector2D(1280,720-Layout.FooterTop),White,FLinearColor(0,.004f,.008f,.88f));
  const int32 Percent=Progress->Display.load();
  if(Percent==0)++Progress->ZeroFrames;
  if(Percent==100)++Progress->ReadyFrames;
  TArray<FVector2D> Ring,Arc;const FVector2D Center=Layout.RingCenter;
  for(int32 I=0;I<=64;++I){const float A=-HALF_PI+2*PI*I/64.f;const FVector2D P=Center+FVector2D(FMath::Cos(A),FMath::Sin(A))*Layout.RingRadius;Ring.Add(P);if(I<=Percent*.64f)Arc.Add(P);}
  FSlateDrawElement::MakeLines(Out,Layer++,G.ToPaintGeometry(),Ring,ESlateDrawEffect::None,FLinearColor(.15f,.22f,.27f),true,3);
  if(Arc.Num()>1)FSlateDrawElement::MakeLines(Out,Layer++,G.ToPaintGeometry(),Arc,ESlateDrawEffect::None,Accent,true,3);
  for(const auto& Line:Layout.Lines){
   FString Message=Line.Text;
   if(Line.Role==TEXT("percent"))Message=FString::Printf(TEXT("%d %%"),Percent);
   else if(Line.Role==TEXT("status"))Message=Percent==100?TEXT("READY TO EXPLORE"):Progress->Target.load()<70?TEXT("PREPARING DESTINATION"):TEXT("TERRAIN & EQUIPMENT");
   FSlateDrawElement::MakeText(Out,Layer++,G.ToPaintGeometry(Line.Extent,FSlateLayoutTransform(Line.Position)),Message,Line.Font,ESlateDrawEffect::None,Line.Color);
  }
  return Layer;
 }
private:
 TSharedPtr<FLoadingProgress,ESPMode::ThreadSafe> Progress;
 FSlateBrush Art;const FSlateBrush* White=nullptr;
 LunarLoading::FTypographyLayout Layout;bool Preview=false,Startup=false;float Value=0;int32 Ticks=0;
};

class FLoadingController {
public:
 void Initialize(){
  Initialized=true;
  Exit=FCoreDelegates::OnEnginePreExit.AddRaw(this,&FLoadingController::Shutdown);
  Pre=FCoreUObjectDelegates::PreLoadMap.AddRaw(this,&FLoadingController::PreLoad);
  Post=FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this,&FLoadingController::PostLoad);
 }
 void Shutdown(){
  if(!Initialized)return;Initialized=false;
  FCoreDelegates::OnEnginePreExit.Remove(Exit);
  FCoreUObjectDelegates::PreLoadMap.Remove(Pre);FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(Post);
  if(GEngine)GEngine->OnTravelFailure().Remove(Failure);
  if(IntroPlaying&&GetMoviePlayer())GetMoviePlayer()->StopMovie();IntroPlaying=false;
  End(false);
  for(auto* Texture:Textures)if(Texture)Texture->RemoveFromRoot();Textures.Empty();
 }
 bool Open(const UObject* Context,FName Map,const FString& Options){
  if(Busy||Pending||IntroPlaying||!Context)return false;
  const FString Package=Map.ToString();
  if(!Known(FPackageName::GetShortName(Package))||!FPackageName::DoesPackageExist(Package)){UE_LOG(LogTemp,Warning,TEXT("MISSION_LOADING rejected missing map %s"),*Package);return false;}
  if(GEngine&&!Failure.IsValid())Failure=GEngine->OnTravelFailure().AddRaw(this,&FLoadingController::TravelFailed);
  // OpenLevel queues travel for the engine. Start MoviePlayer only inside
  // PreLoadMap; starting it here would stall that queued travel at 15%.
  IntroPending=LunarMissionIntro::Eligible(FPackageName::GetShortName(Package),Options,UGameplayStatics::GetCurrentLevelName(Context,true)==TEXT("MissionSelect"));Pending=true;
  UGameplayStatics::OpenLevel(Context,Map,true,Options);
  return true;
 }
 bool Completed(const FString& Map,bool RequireRendered) const {
  return !Busy&&!Pending&&!IntroPlaying&&LastMap==Map&&LastSucceeded&&LastTimeHeld&&(!RequireRendered||LastRendered);
 }
 bool Busy=false,Pending=false,IntroPlaying=false;
private:
 bool Known(const FString& Map) const {return Map==TEXT("Apollo12")||Map==TEXT("MoonBase")||Map==TEXT("MarsBase")||Map==TEXT("MissionSelect");}
 void Begin(const FString& Package){
  if(Busy)return;
  const FString Map=FPackageName::GetShortName(Package);if(!Known(Map))return;
  Startup=Map==TEXT("MissionSelect")&&!StartupSeen;StartupSeen=true;Busy=true;Destination=Map;Start=FPlatformTime::Seconds();Progress=MakeShared<FLoadingProgress,ESPMode::ThreadSafe>();Rendered=false;
  if(FApp::CanEverRender()&&IsMoviePlayerEnabled()&&FSlateApplication::IsInitialized()){
   const TCHAR* Name=Map==TEXT("MarsBase")?TEXT("Mars"):TEXT("Moon");
   auto* Texture=Startup&&!FPackageName::DoesPackageExist(TEXT("/Game/Lunar/Frontend/Startup20261114/T_AstroKid"))?nullptr:LoadObject<UTexture2D>(nullptr,Startup?TEXT("/Game/Lunar/Frontend/Startup20261114/T_AstroKid.T_AstroKid"):*FString::Printf(TEXT("/Game/Lunar/Frontend/T_Loading%s.T_Loading%s"),Name,Name));
   if(Texture&&!Textures.Contains(Texture)){Texture->AddToRoot();Textures.Add(Texture);}
   Screen=SNew(SBox).WidthOverride(1280).HeightOverride(720)[SNew(SLunarLoading,Texture,Map,Progress.ToSharedRef(),LunarLoading::LoadingTextScale(ULunarSettings::Get()->TextScale),false).StartupScreen(Startup)];
   FLoadingScreenAttributes Attr;
   Attr.WidgetLoadingScreen=SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[Screen.ToSharedRef()];
   Attr.bAutoCompleteWhenLoadingCompletes=false;Attr.bWaitForManualStop=true;Attr.bMoviesAreSkippable=false;
   Attr.bAllowEngineTick=false;
   GetMoviePlayer()->SetupLoadingScreen(Attr);Rendered=GetMoviePlayer()->PlayMovie();
  }
  Progress->Target=15;
  UE_LOG(LogTemp,Display,TEXT("MISSION_LOADING begin %s rendered=%d"),*Destination,Rendered);
 }
 void PreLoad(const FString& Package){
  const FString Map=FPackageName::GetShortName(Package);
  // Initial launch is not a selected mission. All later local travel uses Open().
  if(!Busy&&(Pending||Map!=TEXT("MissionSelect")||!StartupSeen))Begin(Package);
  if(Busy&&Map==Destination)Progress->Target=70;
 }
 void PostLoad(UWorld* World){
  if(!Busy)return;
  if(!World||FPackageName::GetShortName(World->GetOutermost()->GetName())!=Destination){End(false);return;}
  const double WorldTime=World->GetTimeSeconds();
  Progress->Target=100;
  if(Rendered){
   // The Slate loading thread keeps painting while this game thread is blocked.
   // Hold both endpoints, without advancing the world/timer underneath the screen.
   const double Deadline=FPlatformTime::Seconds()+4;
   while(FPlatformTime::Seconds()<Deadline&&(FPlatformTime::Seconds()-Start<(Startup?.35:2.5)||Progress->ReadyFrames.load()<(Startup?2:15)))FPlatformProcess::Sleep(.01f);
  }
  LastTimeHeld=FMath::IsNearlyEqual(WorldTime,double(World->GetTimeSeconds()),.0001);
  if(Startup)UE_LOG(LogTemp,Display,TEXT("ASTROKID_STARTUP ready wall_seconds=%.3f zero_frames=%d ready_frames=%d held_world=%d"),FPlatformTime::Seconds()-Start,Progress->ZeroFrames.load(),Progress->ReadyFrames.load(),LastTimeHeld);
  const bool PlayIntro=IntroPending;const FString IntroMap=Destination;End(true);
  if(PlayIntro){IntroPlaying=true;if(IsMoviePlayerEnabled())GetMoviePlayer()->WaitForMovieToFinish(false);LunarMissionIntro::Play(World,IntroMap);IntroPlaying=false;}
 }
 void TravelFailed(UWorld*,ETravelFailure::Type,const FString&){End(false);}
 void End(bool Succeeded){
  if(!Busy){Pending=false;return;}
  LastMap=Destination;LastSucceeded=Succeeded;
  LastRendered=Rendered&&Progress->Monotonic.load()&&Progress->ZeroFrames.load()>=1&&Progress->ReadyFrames.load()>=(Startup?2:15);
  UE_LOG(LogTemp,Display,TEXT("MISSION_LOADING end %s success=%d rendered=%d zero_frames=%d ready_frames=%d progress=%d monotonic=%d"),*Destination,Succeeded,Rendered,Progress->ZeroFrames.load(),Progress->ReadyFrames.load(),Progress->Display.load(),Progress->Monotonic.load());
  if(Rendered&&GetMoviePlayer())GetMoviePlayer()->StopMovie();
  Screen.Reset();Progress.Reset();Busy=false;Pending=false;Rendered=false;IntroPending=false;
 }
 FDelegateHandle Pre,Post,Failure,Exit;bool Initialized=false,IntroPending=false,Startup=false,StartupSeen=false;
 TArray<UTexture2D*> Textures;TSharedPtr<SWidget> Screen;
 TSharedPtr<FLoadingProgress,ESPMode::ThreadSafe> Progress;
 FString Destination,LastMap;double Start=0;bool Rendered=false,LastSucceeded=false,LastRendered=false,LastTimeHeld=true;
};
FLoadingController LunarLoadingController;
}

void LunarLoading::Initialize(){LunarLoadingController.Initialize();}
void LunarLoading::Shutdown(){LunarLoadingController.Shutdown();}
bool LunarLoading::Open(const UObject* Context,FName Map,const FString& Options){return LunarLoadingController.Open(Context,Map,Options);}
bool LunarLoading::IsBusy(){return LunarLoadingController.Busy||LunarLoadingController.Pending||LunarLoadingController.IntroPlaying;}
bool LunarLoading::Completed(const FString& Map,bool Rendered){return LunarLoadingController.Completed(Map,Rendered);}

TSharedRef<SWidget> LunarLoading::MakeTypographyPreview(UTexture2D* Texture,const FString& Map,float TextScale,int32 Percent){
 auto Progress=MakeShared<FLoadingProgress,ESPMode::ThreadSafe>();Progress->Target=Percent<=0?15:100;Progress->Display=FMath::Clamp(Percent,0,100);
 return SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1280).HeightOverride(720)[SNew(SLunarLoading,Texture,Map,Progress,TextScale,true)]];
}
