#include "LunarLoading.h"
#include "LunarLoadingTypography.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UObject/StrongObjectPtr.h"
#include "UnrealClient.h"
#include "Widgets/SWidget.h"

namespace {
struct FLoadingPreviewCase {const TCHAR* Map;const TCHAR* Name;float Scale;int32 Percent;};
const FLoadingPreviewCase LoadingCases[]={
 {TEXT("Apollo12"),TEXT("apollo"),1,85},{TEXT("Apollo12"),TEXT("apollo150"),1.5f,85},
 {TEXT("MoonBase"),TEXT("selene"),1,85},{TEXT("MoonBase"),TEXT("selene150"),1.5f,85},
 {TEXT("MarsBase"),TEXT("mars"),1,85},{TEXT("MarsBase"),TEXT("mars150"),1.5f,85},
 {TEXT("MissionSelect"),TEXT("menu"),1,85},{TEXT("MissionSelect"),TEXT("menu150"),1.5f,85},
 {TEXT("Apollo12"),TEXT("apollo150-zero"),1.5f,0},{TEXT("Apollo12"),TEXT("apollo150-ready"),1.5f,100}
};
struct FLoadingPreviewState {
 TWeakObjectPtr<UWorld> World;TWeakObjectPtr<UGameViewportClient> Viewport;
 TSharedPtr<SWidget> Screen;TArray<TStrongObjectPtr<UTexture2D>> Textures;
 FString Folder;TArray<FString> Frames;int32 Index=0;bool Waiting=false;FDelegateHandle Processed;
};
using FPreviewState=TSharedRef<FLoadingPreviewState>;
void NextLoadingPreview(FPreviewState State);
void LoadingPreviewLater(FPreviewState State,float Delay,TFunction<void()> Action){
 if(auto* World=State->World.Get()){FTimerHandle Timer;World->GetTimerManager().SetTimer(Timer,FTimerDelegate::CreateWeakLambda(World,[Action=MoveTemp(Action)](){Action();}),Delay,false);}
}
void EndLoadingPreview(FPreviewState State,bool Good){
 if(State->Processed.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->Processed);
 if(auto* Viewport=State->Viewport.Get();Viewport&&State->Screen.IsValid())Viewport->RemoveViewportWidgetContent(State->Screen.ToSharedRef());State->Screen.Reset();
 const FString Json=TEXT("{\"synthetic\":true,\"human_playtest\":false,\"performance_measurement\":false,\"actual_travel\":false,\"description\":\"Staged typography snapshots use the exact MoviePlayer paint widget and fixed display values. They do not measure real map preparation, runtime progress timing or world hold; those remain covered by LoadingRenderedTest.\",\"frames\":[\n")+FString::Join(State->Frames,TEXT(",\n"))+TEXT("\n]}\n");
 Good=FFileHelper::SaveStringToFile(Json,*(State->Folder/TEXT("layout-manifest.json")))&&Good;
 UE_LOG(LogTemp,Display,TEXT("LOADING_TYPOGRAPHY_REVIEW %s frames=%d"),Good?TEXT("COMPLETE"):TEXT("FAILED"),State->Frames.Num());FPlatformMisc::RequestExitWithStatus(false,Good?0:1);
}
void NextLoadingPreview(FPreviewState State){
 if(State->Processed.IsValid()){FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->Processed);State->Processed.Reset();}
 auto* Viewport=State->Viewport.Get();auto* World=State->World.Get();if(!Viewport||!World){EndLoadingPreview(State,false);return;}
 if(State->Screen.IsValid()){Viewport->RemoveViewportWidgetContent(State->Screen.ToSharedRef());State->Screen.Reset();}
 if(State->Index>=UE_ARRAY_COUNT(LoadingCases)){EndLoadingPreview(State,true);return;}
 const auto Case=LoadingCases[State->Index++];const bool Mars=FString(Case.Map)==TEXT("MarsBase");
 auto* Texture=LoadObject<UTexture2D>(nullptr,Mars?TEXT("/Game/Lunar/Frontend/T_LoadingMars.T_LoadingMars"):TEXT("/Game/Lunar/Frontend/T_LoadingMoon.T_LoadingMoon"));if(!Texture){EndLoadingPreview(State,false);return;}
 State->Textures.Add(TStrongObjectPtr<UTexture2D>(Texture));State->Screen=LunarLoading::MakeTypographyPreview(Texture,Case.Map,Case.Scale,Case.Percent);Viewport->AddViewportWidgetContent(State->Screen.ToSharedRef(),1000);
 LoadingPreviewLater(State,.3f,[State,Case](){
  auto* CurrentWorld=State->World.Get();if(!CurrentWorld){EndLoadingPreview(State,false);return;}
  State->Waiting=true;State->Processed=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(CurrentWorld,[State,Case](){
   if(!State->Waiting)return;State->Waiting=false;const auto Layout=LunarLoading::BuildTypography(Case.Map,Case.Scale);
   State->Frames.Add(FString::Printf(TEXT("{\"name\":\"%s\",\"map\":\"%s\",\"text_scale\":%.2f,\"percent\":%d,\"footer_top\":%.3f,\"measured_line_count\":%d,\"slate_measured\":%s}"),Case.Name,Case.Map,Case.Scale,Case.Percent,Layout.FooterTop,Layout.Lines.Num(),Layout.SlateMeasured?TEXT("true"):TEXT("false")));
   LoadingPreviewLater(State,.15f,[State](){NextLoadingPreview(State);});
  });
  FScreenshotRequest::RequestScreenshot(State->Folder/(FString(Case.Name)+TEXT(".png")),true,false);
 });
}
}
void LunarLoading::RunTypographyReview(UWorld* World){
 if(!World||!GEngine||!GEngine->GameViewport){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto State=MakeShared<FLoadingPreviewState>();State->World=World;State->Viewport=GEngine->GameViewport.Get();State->Folder=FPaths::ProjectDir()/TEXT("Reports/LoadingTypography20261102/staged");IFileManager::Get().MakeDirectory(*State->Folder,true);NextLoadingPreview(State);
}
