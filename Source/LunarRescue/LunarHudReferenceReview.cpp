#include "LunarGame.h"
#include "LunarFlightHUD.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "LunarDiagnostics.h"
#include "LunarEquipmentDetail.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace {
struct FFeedbackReview {
 FString Folder,Report;TArray<FString> Frames;int32 Step=0,Failed=0;bool Waiting=false;FDelegateHandle Shot;
};
using FFeedbackState=TSharedRef<FFeedbackReview>;
void NextFeedbackFrame(ALunarGameMode* G,FFeedbackState State);
void FinishFeedback(ALunarGameMode* G,FFeedbackState State){
 if(State->Shot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->Shot);
 ULunarSettings::Get()->TextScale=ULunarSettings::Get()->SubtitleScale=1.f;
 const FString Manifest=TEXT("{\"synthetic\":true,\"human_playtest\":false,\"performance_measurement\":false,\"description\":\"Scripted HUD and optional notebook scenarios. HUD and notebook viewport checks use actual arranged Slate geometry at the requested font scale; notebook body scrolling is allowed. Mission states and hold values are fixtures, not human session results. PNG freshness and dimensions are checked by the external runner.\",\"frames\":[\n")+FString::Join(State->Frames,TEXT(",\n"))+TEXT("\n]}\n");
 if(!FFileHelper::SaveStringToFile(Manifest,*(State->Folder/TEXT("hud-reference-manifest.json"))))++State->Failed;
 State->Report+=FString::Printf(TEXT("RESULT: %d failures\n"),State->Failed);
 if(!FFileHelper::SaveStringToFile(State->Report,*(State->Folder/TEXT("hud-reference-checks.txt"))))++State->Failed;
 UE_LOG(LogTemp,Display,TEXT("HUD_REFERENCE_REVIEW %s frames=%d\n%s"),State->Failed?TEXT("FAILED"):TEXT("COMPLETE"),State->Frames.Num(),*State->Report);
 FPlatformMisc::RequestExitWithStatus(false,State->Failed?1:0);
}
bool NotebookLayout(ULunarPanel* Panel,FString& Reason){
 if(!Panel||!Panel->Body||!Panel->WidgetTree){Reason=TEXT("Notebook missing");return false;}
 const auto* Scroll=Cast<UScrollBox>(Panel->Body->GetParent());if(!Scroll){Reason=TEXT("Notebook body is not scrollable");return false;}
 const auto& Root=Panel->GetCachedGeometry();const auto& View=Scroll->GetCachedGeometry();
 const FVector2D A=Root.AbsoluteToLocal(View.LocalToAbsolute(FVector2D::ZeroVector)),B=Root.AbsoluteToLocal(View.LocalToAbsolute(View.GetLocalSize())),Size=Root.GetLocalSize();
 if(Size.X<1||A.X<-.5||A.Y<-.5||B.X>Size.X+.5||B.Y>Size.Y+.5||B.X<=A.X||B.Y<=A.Y){Reason=TEXT("Notebook scroll viewport is outside its allocated screen");return false;}
 TArray<UWidget*> Widgets;Panel->WidgetTree->GetAllWidgets(Widgets);
 for(const auto* W:Widgets)if(const auto* Text=Cast<UTextBlock>(W)){
  bool InBody=false;for(const UWidget* Parent=Text;Parent;Parent=Parent->GetParent())if(Parent==Panel->Body){InBody=true;break;}if(!InBody)continue;
  const auto& Geo=Text->GetCachedGeometry();if(Geo.GetLocalSize().X<1)continue;
  const auto Left=View.AbsoluteToLocal(Geo.LocalToAbsolute(FVector2D::ZeroVector));const auto Right=View.AbsoluteToLocal(Geo.LocalToAbsolute(Geo.GetLocalSize()));
  if(Left.X<-.5||Right.X>View.GetLocalSize().X+.5||Geo.GetLocalSize().Y+.5<Text->GetDesiredSize().Y){Reason=TEXT("Notebook text overflows horizontally or clips its allocated height");return false;}
 }
 return true;
}
void NextFeedbackFrame(ALunarGameMode* G,FFeedbackState State){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));if(!P||!G->FlightHUD){++State->Failed;FinishFeedback(G,State);return;}
 if(P->ActivePanel)P->ClosePanel();G->Paused=false;G->Holding=false;G->HoldProgress=0;G->ApolloTool=0;G->ApolloDustMode=false;
 // Each fixture begins from one coherent mission baseline. Work/notebook states
 // seed their prerequisites explicitly; fixture flags do not represent playtests.
 G->Oxygen=G->OxygenCapacity;G->PowerSequenceActive=false;G->StationRebootComplete=false;G->RepairStep=0;
 if(G->Apollo){G->Stage=ELunarStage::ApolloCheckout;G->ApolloAction=0;G->ApolloInstruments=0;G->ApolloPhotos=0;G->SamplesCollected=0;G->ApolloParts=0;G->SurveyorPhotographed=false;G->ApolloDustPhotos=0;G->ApolloTimed=false;}
 else G->Stage=ELunarStage::FindModule;
 auto* S=ULunarSettings::Get();S->TextScale=S->SubtitleScale=1.f;S->TutorialHints=true;G->ApolloGuideEnabled=true;G->RadioRemaining=0;
 FString Name;bool Notebook=false;
 switch(State->Step++){
  case 0:Name=TEXT("hud-overview100");G->Message(G->Apollo?TEXT("NAV: Check the suit and prepare the field kit before leaving Intrepid."):TEXT("NAV: Recover the spare power cell beside the survey rover."));break;
  case 1:Name=TEXT("hud-overview150");S->TextScale=S->SubtitleScale=1.5f;G->Message(TEXT("SCRIPTED RADIO / Check the current objective, keep the return journey in mind, and complete each work action before moving to the next site."));break;
  case 2:Name=TEXT("hud-guide100");break;
  case 3:Name=TEXT("hud-guide150");S->TextScale=S->SubtitleScale=1.5f;break;
  case 4:
   Name=TEXT("hud-work150");S->TextScale=S->SubtitleScale=1.5f;
   if(G->Apollo){G->Stage=ELunarStage::ApolloDeploy;G->ApolloTool=2;G->ApolloToolPhase=0;G->ApolloLevel=FVector2D(.17f,-.12f);}
   else{G->Stage=ELunarStage::CollectData;G->RepairStep=2;G->StationRebootComplete=true;G->Holding=true;G->HoldProgress=G->HoldDuration()*.65f;}
   if(AActor* Subject=G->Apollo?(G->ApolloInstrumentActors.Num()?G->ApolloInstrumentActors[0].Get():nullptr):G->DataActor.Get()){P->SetActorLocation(Subject->GetActorLocation()+FVector(-160,0,50));FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);if(auto* PC=UGameplayStatics::GetPlayerController(G,0))PC->SetControlRotation((Subject->GetActorLocation()-Eye).Rotation());}
   break;
  case 5:Name=TEXT("hud-low-resource150");S->TextScale=S->SubtitleScale=1.5f;G->ApolloTimed=G->Apollo;G->Oxygen=15;break;
  case 6:Name=TEXT("equipment-notebook100");Notebook=true;break;
  case 7:Name=TEXT("equipment-notebook150");S->TextScale=S->SubtitleScale=1.5f;Notebook=true;break;
  case 8:
   if(!G->Apollo){FinishFeedback(G,State);return;}
   Name=TEXT("hud-optional-choice150");S->TextScale=S->SubtitleScale=1.5f;G->ApolloInstruments=3;G->ApolloPhotos=7;G->SamplesCollected=7;G->SurveyorPhotographed=true;G->ApolloDustPhotos=3;G->ApolloDustMode=true;G->ApolloDustCompared=false;G->ApolloTool=9;G->ApolloDustChoice=0;G->Stage=ELunarStage::ApolloRecover;break;
  default:FinishFeedback(G,State);return;
 }
 if(Notebook)P->OpenEvidence();else{P->SyncPauseMenu();P->UpdateSuit(0);G->FlightHUD->Refresh();}
 FTimerHandle Wait;G->GetWorldTimerManager().SetTimer(Wait,FTimerDelegate::CreateWeakLambda(G,[G,State,Name,Notebook](){
  if(State->Shot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(State->Shot);
  State->Waiting=true;State->Shot=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(G,[G,State,Name,Notebook](){
   if(!State->Waiting)return;State->Waiting=false;
   auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));FString Reason;
   const bool Layout=Notebook?Pilot&&LunarDiagnostics::Build(G).Available&&NotebookLayout(Pilot->ActivePanel,Reason):G->FlightHUD&&G->FlightHUD->ValidateLayout(Reason);
   const auto& View=G->FlightHUD->State();const bool Coherent=G->Apollo?(G->ApolloTool!=9||(G->SurveyorPhotographed&&G->ApolloDustPhotos==3&&G->SamplesCollected==7&&G->ApolloInstruments==3)):(!G->StationRebootComplete||G->RepairStep==2);
   const bool Truthful=Coherent&&(Notebook||View.Objective==ULunarFlightHUD::ReadState(G,Pilot).Objective);
   const bool Good=Layout&&Truthful;if(!Good)++State->Failed;
   State->Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+(Reason.IsEmpty()?FString():TEXT(" / ")+Reason)+TEXT("\n");
   State->Frames.Add(FString::Printf(TEXT("{\"name\":\"%s\",\"layout_passed\":%s,\"text_scale\":%.2f,\"subtitle_scale\":%.2f,\"hud_visible\":%s,\"notebook\":%s,\"hold_fraction\":%.4f}"),*Name,Good?TEXT("true"):TEXT("false"),ULunarSettings::Get()->TextScale,ULunarSettings::Get()->SubtitleScale,View.Visible?TEXT("true"):TEXT("false"),Notebook?TEXT("true"):TEXT("false"),View.HoldFraction));
   FTimerHandle Next;G->GetWorldTimerManager().SetTimer(Next,FTimerDelegate::CreateWeakLambda(G,[G,State](){NextFeedbackFrame(G,State);}),.25f,false);
  });
  FScreenshotRequest::RequestScreenshot(State->Folder/(Name+TEXT(".png")),true,false);
 }),.65f,false);
}
}
void ALunarGameMode::HUDReferenceReview(){
 auto State=MakeShared<FFeedbackReview>();State->Folder=FPaths::ProjectDir()/TEXT("Reports/Feedback20261102")/(Apollo?TEXT("hud-apollo"):Mars?TEXT("hud-mars"):TEXT("hud-selene"));IFileManager::Get().MakeDirectory(*State->Folder,true);
 if(Apollo)StartApollo(true);else StartMission();SetActorTickEnabled(false);
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0))){P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();}
 NextFeedbackFrame(this,State);
}
void ALunarGameMode::FeedbackAssetChecks(){
 FString Report;int32 Failed=0;LunarEquipmentDetail::AppendChecks(this,Report,Failed);Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);
 FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("feedback-assets-checks.txt")));UE_LOG(LogTemp,Display,TEXT("FEEDBACK_ASSETS_RESULT\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}