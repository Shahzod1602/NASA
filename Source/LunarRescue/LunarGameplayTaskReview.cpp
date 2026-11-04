#include "Components/StaticMeshComponent.h"
#include "LunarGame.h"
#include "LunarDeparture.h"
#include "LunarCarryPresentation.h"
#include "LunarCrew.h"
#include "LunarPanels.h"
#include "LunarFlightHUD.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace {
struct FTaskReview {FString Folder,Report;TArray<FString> Frames;int32 Step=0,Failed=0,CellVariant=0;bool Waiting=false,HandoverCaptured=false;FDelegateHandle Shot;};
using FTaskState=TSharedRef<FTaskReview>;
void Next(ALunarGameMode* G,FTaskState S);
void Finish(ALunarGameMode* G,FTaskState S){
 if(S->Shot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(S->Shot);
 ULunarSettings::Get()->TextScale=ULunarSettings::Get()->SubtitleScale=1;
 const FString Manifest=TEXT("{\"synthetic\":true,\"human_playtest\":false,\"performance_measurement\":false,\"description\":\"Scripted rendered gameplay fixtures. Carry states use retained source meshes and committed fixture flags; crew views and notebook panels are actual world/UI representations. Departure times are explicitly advanced snapshots, not a measured human walkthrough.\",\"frames\":[\n")+FString::Join(S->Frames,TEXT(",\n"))+TEXT("\n]}\n");
 if(!FFileHelper::SaveStringToFile(Manifest,*(S->Folder/TEXT("gameplay-manifest.json"))))++S->Failed;
 S->Report+=FString::Printf(TEXT("RESULT: %d failures\n"),S->Failed);
 if(!FFileHelper::SaveStringToFile(S->Report,*(S->Folder/TEXT("gameplay-review-checks.txt"))))++S->Failed;
 UE_LOG(LogTemp,Display,TEXT("GAMEPLAY_TASKS_REVIEW %s frames=%d\n%s"),S->Failed?TEXT("FAILED"):TEXT("COMPLETE"),S->Frames.Num(),*S->Report);
 FPlatformMisc::RequestExitWithStatus(false,S->Failed?1:0);
}
bool NotebookLayout(ULunarPanel* P,FString& Why){
 if(!P||!P->Body||!P->WidgetTree){Why=TEXT("Notebook missing");return false;}
 auto* Scroll=Cast<UScrollBox>(P->Body->GetParent());if(!Scroll){Why=TEXT("Notebook is not scrollable");return false;}
 const auto& Root=P->GetCachedGeometry();const auto& View=Scroll->GetCachedGeometry();const auto Size=Root.GetLocalSize();
 const auto A=Root.AbsoluteToLocal(View.LocalToAbsolute(FVector2D::ZeroVector)),B=Root.AbsoluteToLocal(View.LocalToAbsolute(View.GetLocalSize()));
 if(Size.X<1||A.X<-.5||A.Y<-.5||B.X>Size.X+.5||B.Y>Size.Y+.5||B.X<=A.X||B.Y<=A.Y){Why=TEXT("Notebook viewport outside allocated screen");return false;}
 TArray<UWidget*> Widgets;P->WidgetTree->GetAllWidgets(Widgets);
 for(auto* W:Widgets)if(auto* T=Cast<UTextBlock>(W)){
  bool InBody=false;for(const UWidget* Parent=T;Parent;Parent=Parent->GetParent())if(Parent==P->Body){InBody=true;break;}if(!InBody)continue;
  const auto& Geo=T->GetCachedGeometry();if(Geo.GetLocalSize().X<1)continue;
  const auto Left=View.AbsoluteToLocal(Geo.LocalToAbsolute(FVector2D::ZeroVector)),Right=View.AbsoluteToLocal(Geo.LocalToAbsolute(Geo.GetLocalSize()));
  if(Left.X<-.5||Right.X>View.GetLocalSize().X+.5||Geo.GetLocalSize().Y+.5<T->GetDesiredSize().Y){Why=TEXT("Notebook text clips horizontally or vertically");return false;}
 }
 return true;
}
void Next(ALunarGameMode* G,FTaskState S){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));auto* PC=UGameplayStatics::GetPlayerController(G,0);
 if(!P||!PC||!G->FlightHUD){++S->Failed;Finish(G,S);return;}
 if(P->ActivePanel)P->ClosePanel();G->Paused=false;G->Journal=false;G->Holding=false;G->RadioRemaining=0;G->ApolloTool=0;G->ApolloPhotoPending=false;P->PhotoCaptureFrames=0;
 auto* Settings=ULunarSettings::Get();Settings->TextScale=Settings->SubtitleScale=1;
 const int32 Index=S->Step++;FString Name;bool Panel=false,Cabin=false,Carry=false,Handover=false,HiddenControl=false;int32 CrewIndex=INDEX_NONE;
 if(Index<4){
  Cabin=true;if(Index==0){G->Stage=ELunarStage::Briefing;ALunarDepartureScene::Reset(G);auto* Scene=ALunarDepartureScene::Begin(G);if(Scene)Scene->SetActorTickEnabled(false);Name=TEXT("departure-cabin");}
  auto* Scene=G->DepartureScene.Get();if(!Scene){++S->Failed;Finish(G,S);return;}
  if(Index==1){Settings->TextScale=Settings->SubtitleScale=1.5f;Scene->AdvancePresentation(4.4f);Name=TEXT("departure-hatch");}
  if(Index==2){G->Paused=true;Scene->AdvancePresentation(2);Name=TEXT("departure-paused");}
  if(Index==3){Scene->AdvancePresentation(4.3f);Name=TEXT("departure-near-surface");}
 }else if(Index==4&&!S->HandoverCaptured){
  Handover=true;S->HandoverCaptured=true;--S->Step;Name=TEXT("departure-handover");if(auto* Scene=G->DepartureScene.Get())Scene->AdvancePresentation(ALunarDepartureScene::Duration);P->UpdateSuit(0);LunarCarryPresentation::Update(P);
 }else{
  if(Index>=4&&Index<8){Carry=true;if(!LunarCarryPresentation::ApplyReviewFixture(G,Index-4,Name)){++S->Failed;Finish(G,S);return;}P->WorkBlend=1;P->UpdateSuit(0);LunarCarryPresentation::ApplyReviewFixture(G,Index-4,Name);if(!G->Apollo&&Index==5){const int32 Variant=S->CellVariant++;if(Variant<3)--S->Step;if(Variant>=2)Settings->TextScale=Settings->SubtitleScale=1.5f;if(Variant==1){Name=TEXT("cell-fp45-hidden-control");HiddenControl=true;}else if(Variant==2)Name=TEXT("cell-fp45-text150");else if(Variant==3){Name=TEXT("cell-fp45-text150-hidden-control");HiddenControl=true;}if(HiddenControl)P->CarriedModule->SetVisibility(false);}}
  else if(Index==8||Index==9){
   CrewIndex=Index-8;Name=Index==8?TEXT("crew-pip-model"):TEXT("crew-mira-communicator");
   if(!G->CrewActors.IsValidIndex(CrewIndex)||!IsValid(G->CrewActors[CrewIndex])){++S->Failed;Finish(G,S);return;}
   auto* A=G->CrewActors[CrewIndex].Get();const FVector Focus=A->GetActorLocation()+A->GetActorUpVector()*(CrewIndex?120:67);
   P->ThirdPerson=false;P->Camera->bUsePawnControlRotation=false;P->Camera->SetFieldOfView(70);P->Camera->SetWorldLocation(Focus+A->GetActorForwardVector()*-240+FVector(0,0,45));P->Camera->SetWorldRotation((Focus-P->Camera->GetComponentLocation()).Rotation());
   G->ApolloPackedPreviewRemaining=G->ApolloPackedPartRemaining=0;G->Stage=G->Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule;LunarCarryPresentation::Update(P);
  }else if(Index==10||Index==11){
   Name=Index==10?TEXT("crew-notebook100"):TEXT("crew-notebook150");if(Index==11)Settings->TextScale=Settings->SubtitleScale=1.5f;P->OpenCrewNotebook();Panel=true;
  }else if(!G->Mars&&Index>=12&&Index<=16){
   Panel=true;LunarWater::Reset(G->WaterStudy);if(Index==12)Name=TEXT("water-first-decision");
   if(Index==13){Name=TEXT("water-feedback");LunarWater::RecordAnswer(G->WaterStudy,0);}
   if(Index>=14){LunarWater::RecordAnswer(G->WaterStudy,1);LunarWater::RecordAnswer(G->WaterStudy,2);LunarWater::RecordAnswer(G->WaterStudy,0);Name=TEXT("water-ready");}
   if(Index>=15){LunarWater::Commit(G->WaterStudy);Name=Index==15?TEXT("water-recorded"):TEXT("water-sources150");}
   if(Index==16)Settings->TextScale=Settings->SubtitleScale=1.5f;
   P->OpenWaterNotebook();if(Index==16&&P->ActivePanel){P->ActivePanel->ShowWaterSources=true;P->ActivePanel->Rebuild();if(auto* Scroll=Cast<UScrollBox>(P->ActivePanel->Body->GetParent()))Scroll->ScrollToEnd();}
  }else{Finish(G,S);return;}
 }
 if(Panel)P->UpdateSuit(0);P->SyncPauseMenu();G->FlightHUD->Refresh();
 FTimerHandle Wait;G->GetWorldTimerManager().SetTimer(Wait,FTimerDelegate::CreateWeakLambda(G,[G,S,Name,Panel,Cabin,Carry,Handover,HiddenControl,CrewIndex](){
  if(S->Shot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(S->Shot);S->Waiting=true;
  S->Shot=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(G,[G,S,Name,Panel,Cabin,Carry,Handover,HiddenControl,CrewIndex](){
   if(!S->Waiting)return;S->Waiting=false;auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));auto* PC=UGameplayStatics::GetPlayerController(G,0);FString Reason;
   int32 W=0,H=0;if(PC)PC->GetViewportSize(W,H);bool Good=Pilot&&PC&&W>0&&H>0;
   if(Panel)Good=Good&&Pilot->ActivePanel&&G->Paused&&NotebookLayout(Pilot->ActivePanel,Reason);
   if(Cabin)Good=Good&&ALunarDepartureScene::Active(G)&&G->Stage==ELunarStage::Briefing&&PC->GetViewTarget()==G->DepartureScene;
   if(Handover)Good=Good&&G->IsActive()&&!ALunarDepartureScene::Active(G)&&PC->GetViewTarget()==Pilot;
   if(Carry){const bool Expected=!HiddenControl&&Name!=TEXT("cell-seated")&&Name!=TEXT("apollo-stowed");const auto Shown=[](const UStaticMeshComponent* Component){return IsValid(Component)&&Component->IsVisible();};const bool Visible=Pilot&&(Shown(Pilot->CarriedModule)||Shown(Pilot->BodyCarryItem)||Shown(Pilot->WorkPayload)||Shown(Pilot->WorkTool)||Shown(Pilot->SamplePouch));Good=Good&&Visible==Expected;}
   if(!Good)++S->Failed;S->Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+(Reason.IsEmpty()?FString():TEXT(" / ")+Reason)+TEXT("\n");
   const auto* Scene=G->DepartureScene.Get();FBox2D CellRegion(ForceInit);bool RegionValid=false;if(Carry&&!G->Apollo&&Pilot&&Pilot->CarriedModule&&Pilot->CarriedModule->GetStaticMesh()&&Pilot->Camera->FieldOfView<=60)RegionValid=LunarCarryPresentation::ProjectViewBounds(Pilot->CarriedModule->GetStaticMesh()->GetBoundingBox(),Pilot->CarriedModule->GetComponentTransform().GetRelativeTransform(Pilot->Camera->GetComponentTransform()),Pilot->Camera->FieldOfView,float(W)/H,CellRegion);const FVector2D RegionMin=RegionValid?CellRegion.Min:FVector2D::ZeroVector,RegionMax=RegionValid?CellRegion.Max:FVector2D::ZeroVector;S->Frames.Add(FString::Printf(TEXT("{\"name\":\"%s\",\"layout_passed\":%s,\"notebook\":%s,\"text_scale\":%.2f,\"paused\":%s,\"departure_time\":%.3f,\"carry_fixture\":%s,\"crew_model\":%d,\"water_step\":%d,\"water_attempts\":%d,\"water_recorded\":%s,\"fov\":%.3f,\"cell_region\":[%.6f,%.6f,%.6f,%.6f],\"cell_hidden_control\":%s}"),*Name,Good?TEXT("true"):TEXT("false"),Panel?TEXT("true"):TEXT("false"),ULunarSettings::Get()->TextScale,G->Paused?TEXT("true"):TEXT("false"),Scene?Scene->PresentationTime():-1,Carry?TEXT("true"):TEXT("false"),CrewIndex,G->WaterStudy.Step,G->WaterStudy.Attempts,G->WaterStudy.Recorded?TEXT("true"):TEXT("false"),Pilot->Camera->FieldOfView,RegionMin.X,RegionMin.Y,RegionMax.X,RegionMax.Y,HiddenControl?TEXT("true"):TEXT("false")));
   FTimerHandle T;G->GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(G,[G,S](){Next(G,S);}),.2f,false);
  });FScreenshotRequest::RequestScreenshot(S->Folder/(Name+TEXT(".png")),true,false);
 }),.7f,false);
}
}
void ALunarGameMode::GameplayTaskReview(){
 auto S=MakeShared<FTaskReview>();S->Folder=FPaths::ProjectDir()/TEXT("Reports/Gameplay20261103")/(Apollo?TEXT("review-apollo"):Mars?TEXT("review-mars"):TEXT("review-selene"));IFileManager::Get().MakeDirectory(*S->Folder,true);
 SetActorTickEnabled(false);auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(P){P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();P->SetActorTickEnabled(false);}Next(this,S);
}
