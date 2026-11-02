#include "UObject/Package.h"
#include "LunarLoading.h"
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
 SLATE_BEGIN_ARGS(SLunarLoading) {} SLATE_END_ARGS()
 void Construct(const FArguments&, UTexture2D* Texture, const FString& Map,
                TSharedRef<FLoadingProgress, ESPMode::ThreadSafe> InProgress,float TextScale,bool IsPreview=false) {
  Progress=InProgress;Preview=IsPreview;Layout=LunarLoading::BuildTypography(Map,TextScale);
  Art.SetResourceObject(Texture);Art.ImageSize=FVector2D(1280,720);Art.DrawAs=ESlateBrushDrawType::Image;
  White=FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
  SetCanTick(!Preview);
 }
 virtual FVector2D ComputeDesiredSize(float) const override {return FVector2D(1280,720);}
 virtual void Tick(const FGeometry&,double,float Delta) override {
  if(Preview)return;
  const int32 Before=Progress->Display.load();
  // Give the first zero state at least three Slate frames, including warm loads.
  if(Ticks++>=3)Value=FMath::Min(float(Progress->Target.load()),Value+FMath::Clamp(Delta,0.f,.1f)*70.f);
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
 LunarLoading::FTypographyLayout Layout;bool Preview=false;float Value=0;int32 Ticks=0;
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
  End(false);
  for(auto* Texture:Textures)if(Texture)Texture->RemoveFromRoot();Textures.Empty();
 }
 bool Open(const UObject* Context,FName Map,const FString& Options){
  if(Busy||Pending||!Context)return false;
  const FString Package=Map.ToString();
  if(!Known(FPackageName::GetShortName(Package))||!FPackageName::DoesPackageExist(Package)){UE_LOG(LogTemp,Warning,TEXT("MISSION_LOADING rejected missing map %s"),*Package);return false;}
  if(GEngine&&!Failure.IsValid())Failure=GEngine->OnTravelFailure().AddRaw(this,&FLoadingController::TravelFailed);
  // OpenLevel queues travel for the engine. Start MoviePlayer only inside
  // PreLoadMap; starting it here would stall that queued travel at 15%.
  Pending=true;
  UGameplayStatics::OpenLevel(Context,Map,true,Options);
  return true;
 }
 bool Completed(const FString& Map,bool RequireRendered) const {
  return !Busy&&!Pending&&LastMap==Map&&LastSucceeded&&LastTimeHeld&&(!RequireRendered||LastRendered);
 }
 bool Busy=false,Pending=false;
private:
 bool Known(const FString& Map) const {return Map==TEXT("Apollo12")||Map==TEXT("MoonBase")||Map==TEXT("MarsBase")||Map==TEXT("MissionSelect");}
 void Begin(const FString& Package){
  if(Busy)return;
  const FString Map=FPackageName::GetShortName(Package);if(!Known(Map))return;
  Busy=true;Destination=Map;Start=FPlatformTime::Seconds();Progress=MakeShared<FLoadingProgress,ESPMode::ThreadSafe>();Rendered=false;
  if(FApp::CanEverRender()&&IsMoviePlayerEnabled()&&FSlateApplication::IsInitialized()){
   const TCHAR* Name=Map==TEXT("MarsBase")?TEXT("Mars"):TEXT("Moon");
   auto* Texture=LoadObject<UTexture2D>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Frontend/T_Loading%s.T_Loading%s"),Name,Name));
   if(Texture&&!Textures.Contains(Texture)){Texture->AddToRoot();Textures.Add(Texture);}
   Screen=SNew(SBox).WidthOverride(1280).HeightOverride(720)[SNew(SLunarLoading,Texture,Map,Progress.ToSharedRef(),LunarLoading::LoadingTextScale(ULunarSettings::Get()->TextScale))];
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
  if(!Busy&&(Pending||Map!=TEXT("MissionSelect")))Begin(Package);
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
   while(FPlatformTime::Seconds()<Deadline&&(FPlatformTime::Seconds()-Start<2.5||Progress->ReadyFrames.load()<15))FPlatformProcess::Sleep(.01f);
  }
  LastTimeHeld=FMath::IsNearlyEqual(WorldTime,double(World->GetTimeSeconds()),.0001);
  End(true);
 }
 void TravelFailed(UWorld*,ETravelFailure::Type,const FString&){End(false);}
 void End(bool Succeeded){
  if(!Busy){Pending=false;return;}
  LastMap=Destination;LastSucceeded=Succeeded;
  LastRendered=Rendered&&Progress->Monotonic.load()&&Progress->ZeroFrames.load()>=1&&Progress->ReadyFrames.load()>=15;
  UE_LOG(LogTemp,Display,TEXT("MISSION_LOADING end %s success=%d rendered=%d zero_frames=%d ready_frames=%d progress=%d monotonic=%d"),*Destination,Succeeded,Rendered,Progress->ZeroFrames.load(),Progress->ReadyFrames.load(),Progress->Display.load(),Progress->Monotonic.load());
  if(Rendered&&GetMoviePlayer())GetMoviePlayer()->StopMovie();
  Screen.Reset();Progress.Reset();Busy=false;Pending=false;Rendered=false;
 }
 FDelegateHandle Pre,Post,Failure,Exit;bool Initialized=false;
 TArray<UTexture2D*> Textures;TSharedPtr<SWidget> Screen;
 TSharedPtr<FLoadingProgress,ESPMode::ThreadSafe> Progress;
 FString Destination,LastMap;double Start=0;bool Rendered=false,LastSucceeded=false,LastRendered=false,LastTimeHeld=true;
};
FLoadingController LunarLoadingController;
}

void LunarLoading::Initialize(){LunarLoadingController.Initialize();}
void LunarLoading::Shutdown(){LunarLoadingController.Shutdown();}
bool LunarLoading::Open(const UObject* Context,FName Map,const FString& Options){return LunarLoadingController.Open(Context,Map,Options);}
bool LunarLoading::IsBusy(){return LunarLoadingController.Busy||LunarLoadingController.Pending;}
bool LunarLoading::Completed(const FString& Map,bool Rendered){return LunarLoadingController.Completed(Map,Rendered);}

TSharedRef<SWidget> LunarLoading::MakeTypographyPreview(UTexture2D* Texture,const FString& Map,float TextScale,int32 Percent){
 auto Progress=MakeShared<FLoadingProgress,ESPMode::ThreadSafe>();Progress->Target=Percent<=0?15:100;Progress->Display=FMath::Clamp(Percent,0,100);
 return SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1280).HeightOverride(720)[SNew(SLunarLoading,Texture,Map,Progress,TextScale,true)]];
}
