#include "LunarPhotos.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"
#include "ImageWriteQueue.h"
#include "ImageWriteTask.h"
#include "Modules/ModuleManager.h"
#include "UnrealClient.h"

namespace {
IImageWriteQueue* Queue=nullptr;
struct FCapture {
 FString Filename;
 TFunction<void(bool)> Completed;
 FDelegateHandle PixelsHandle,ProcessedHandle;
 double Started=0;
 bool Received=false;
 bool Finished=false;
 void Finish(bool Good){if(!Finished){Finished=true;Completed(Good);}}
};
TSharedPtr<FCapture,ESPMode::ThreadSafe> Capture;
}

void LunarPhotos::Initialize(){
 Queue=&FModuleManager::LoadModuleChecked<IImageWriteQueueModule>(TEXT("ImageWriteQueue")).GetWriteQueue();
}

bool LunarPhotos::Request(const FString& Filename,TFunction<void(bool)> Completed){
 // Do not replace another screenshot, bind over another consumer, or silently
 // fall back to synchronous encoding when screenshot delegates are disabled.
 const auto* Enabled=IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenshotDelegate"));
 if(!Queue||Capture||FScreenshotRequest::IsScreenshotRequested()||UGameViewportClient::OnScreenshotCaptured().IsBound()||(Enabled&&Enabled->GetInt()==0))return false;
 auto Job=MakeShared<FCapture,ESPMode::ThreadSafe>();Job->Filename=Filename;Job->Completed=MoveTemp(Completed);Job->Started=FPlatformTime::Seconds();Capture=Job;
 Job->PixelsHandle=UGameViewportClient::OnScreenshotCaptured().AddLambda([Job](int32 Width,int32 Height,const TArray<FColor>& Colors){
  Job->Received=true;
  const double PixelsAt=FPlatformTime::Seconds();
  if(Width<=0||Height<=0||int64(Width)*Height!=Colors.Num()){Job->Finish(false);return;}
  TArray64<FColor> Pixels;Pixels.Append(Colors.GetData(),Colors.Num());
  auto Task=MakeUnique<FImageWriteTask>();Task->Filename=Job->Filename;Task->Format=EImageFormat::PNG;
  Task->PixelData=MakeUnique<TImagePixelData<FColor>>(FIntPoint(Width,Height),MoveTemp(Pixels));Task->AddPreProcessorToSetAlphaOpaque();
  Task->OnCompleted=[Job,PixelsAt](bool Good){
   UE_LOG(LogTemp,Display,TEXT("APOLLO_PHOTO write %s pixels_to_completion_ms=%.3f file=%s"),Good?TEXT("OK"):TEXT("FAILED"),(FPlatformTime::Seconds()-PixelsAt)*1000,*Job->Filename);
   Job->Finish(Good);
  };
  // Never wait for queue capacity on the game thread.
  auto Result=Queue->Enqueue(MoveTemp(Task),false);
  UE_LOG(LogTemp,Display,TEXT("APOLLO_PHOTO request_to_pixels_ms=%.3f copy_submit_ms=%.3f size=%dx%d"),(PixelsAt-Job->Started)*1000,(FPlatformTime::Seconds()-PixelsAt)*1000,Width,Height);
  if(!Result.IsValid())Job->Finish(false);
 });
 Job->ProcessedHandle=FScreenshotRequest::OnScreenshotRequestProcessed().AddLambda([Job](){
  // Remove destroys this delegate's closure immediately in optimized builds.
  // Keep a stack-owned reference before unbinding; never read its captures after.
  const auto FinishedJob=Job;
  UGameViewportClient::OnScreenshotCaptured().Remove(FinishedJob->PixelsHandle);
  FScreenshotRequest::OnScreenshotRequestProcessed().Remove(FinishedJob->ProcessedHandle);
  Capture.Reset();
  if(!FinishedJob->Received)FinishedJob->Finish(false);
 });
 FScreenshotRequest::RequestScreenshot(Filename,false,false);
 return true;
}

bool ALunarGameMode::RequestApolloPhoto(int32 Index,const FString& Relative){
 if(ApolloPhotoPending||Index<0||Index>5)return false;
 ApolloPhotoFailed=false;
 const TWeakObjectPtr<ALunarGameMode> PhotoGame(this);const FGuid Run=ApolloRunId;
 auto Complete=[PhotoGame,Run,Index,Relative](bool Good){
  auto* G=PhotoGame.Get();if(!G||G->ApolloRunId!=Run)return;
  G->ApolloPhotoPending=false;
  G->ApolloPhotoFailed=!Good;
  auto RefreshAlbum=[G](){if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));P&&P->ActivePanel&&!P->ActivePanel->Settings){P->ActivePanel->Rebuild();P->ActivePanel->SetKeyboardFocus();}};
  if(!Good){G->ConfirmApolloWork(TEXT("Photo could not be saved. Open the camera to retry."));RefreshAlbum();return;}
  if(Index<3)G->ApolloPhotos|=1<<Index;
  else if(Index==3){G->SurveyorPhotographed=true;G->SelectedSample=0;}
  else G->ApolloDustPhotos|=1<<(Index-4);
  G->RecordApolloEvidence(Index,Relative);
  G->ConfirmApolloWork(TEXT("Photo saved. I: view your photograph."));G->SaveApolloCheckpoint();RefreshAlbum();
 };
 if(Relative.IsEmpty()){Complete(true);return true;} // Metadata-only headless checks.
 const FString Filename=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/Relative);
 IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true);
 ApolloPhotoPending=true;
 if(!LunarPhotos::Request(Filename,Complete)){Complete(false);return false;}
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0))){P->PhotoCaptureFrames=2;if(P->HandRig)P->HandRig->SetVisibility(false);if(P->WorkTool)P->WorkTool->SetVisibility(false);}
 if(auto* PC=UGameplayStatics::GetPlayerController(this,0);PC&&PC->PlayerCameraManager)PC->PlayerCameraManager->SetGameCameraCutThisFrame();
 ConfirmApolloWork(TEXT("Saving photograph..."));return true;
}
