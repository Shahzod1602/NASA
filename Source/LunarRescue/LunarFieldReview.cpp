#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarOperationsPanel.h"
#include "LunarDeparture.h"
#include "LunarSettings.h"
#include "LunarSurfaceVehicle.h"
#include "LunarFlightHUD.h"
#include "LunarWireBoard.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/ExpandableArea.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace {
struct FFieldReview {
 FString Folder,Report;
 TArray<TSharedPtr<FJsonValue>> Frames;
 int32 RoverView=0,Step=0,Failed=0,ShotNumber=0,PreflightPreview=0,WirePreview=0;
 bool Finished=false,Waiting=false,FeedChecks=false,AwaitProgress=false,AwaitReady=false;
 double LastGameTime=0,StartedGameTime=0;
 float OriginalText=1,OriginalSubtitle=1;
 FTimerHandle Pump;
 FDelegateHandle Screenshot;
};
using FFieldReviewState=TSharedRef<FFieldReview>;
void Next(ALunarGameMode* G,FFieldReviewState S);
void Capture(ALunarGameMode* G,FFieldReviewState S,const FString& Name,bool Seeded=false,bool ScrollEnd=false);
void Check(FFieldReviewState S,bool Good,const FString& What){S->Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+What+TEXT("\n");if(!Good)++S->Failed;}
ULunarOperationsPanel* Panel(ALunarGameMode* G){auto* P=G?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0)):nullptr;return P?Cast<ULunarOperationsPanel>(P->ActivePanel):nullptr;}
bool InPanelBody(const UWidget* W,const ULunarOperationsPanel* P){for(const UWidget* At=W;At;At=At->GetParent())if(At==P->OperationsBody.Get())return true;return false;}
ULunarOperationsButton* FindButton(ULunarOperationsPanel* P,const FString& Prefix){
 if(!P||!P->WidgetTree)return nullptr;TArray<UWidget*> Widgets;P->WidgetTree->GetAllWidgets(Widgets);
 for(auto* W:Widgets)if(auto* B=Cast<ULunarOperationsButton>(W);B&&InPanelBody(B,P))if(auto* T=Cast<UTextBlock>(B->GetContent());T&&T->GetText().ToString().StartsWith(Prefix))return B;return nullptr;
}
bool Click(ALunarGameMode* G,const FString& Prefix){auto* B=FindButton(Panel(G),Prefix);if(!B||!B->GetIsEnabled())return false;B->Activate();return true;}
const TCHAR* ViewKey(EFieldPanelView V){switch(V){case EFieldPanelView::Preflight:return TEXT("preflight");case EFieldPanelView::Briefing:return TEXT("briefing");case EFieldPanelView::Resources:return TEXT("resources");case EFieldPanelView::Vehicle:return TEXT("vehicle");case EFieldPanelView::Wire:return TEXT("wire");case EFieldPanelView::Inventory:return TEXT("inventory");default:return TEXT("sample");}}
void Finish(ALunarGameMode* G,FFieldReviewState S){
 if(S->Finished)return;S->Finished=true;
 if(S->Screenshot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(S->Screenshot);
 if(G)G->GetWorldTimerManager().ClearTimer(S->Pump);
 auto* Settings=ULunarSettings::Get();Settings->TextScale=S->OriginalText;Settings->SubtitleScale=S->OriginalSubtitle;
 auto Manifest=MakeShared<FJsonObject>();Manifest->SetNumberField(TEXT("schema"),1);Manifest->SetBoolField(TEXT("synthetic"),true);Manifest->SetBoolField(TEXT("human_playtest"),false);Manifest->SetBoolField(TEXT("performance_measurement"),false);
 Manifest->SetStringField(TEXT("description"),TEXT("Staged actual UMG operations panels. Preflight uses real button callbacks and component timer transitions driven by World::GetTimeSeconds deltas in a bounded review ticker, with a deliberate partial-progress freeze for its screenshot. Later mission phase, resource ledger, cable isolation and training cargo are explicitly seeded synthetic fixtures. World frames include an actual stationary occupied rover and eight added production mouse/V camera views, actual magnifier state aimed at a source-labelled cartridge, and a synthetic external sampler view. Their layout assertions validate the real flight HUD, not a nonexistent panel. No human mission completion, water detection, animation/performance result or driven-route result is claimed."));
 Manifest->SetStringField(TEXT("map"),G?UGameplayStatics::GetCurrentLevelName(G,true):FString());Manifest->SetArrayField(TEXT("frames"),S->Frames);Manifest->SetNumberField(TEXT("frame_count"),S->Frames.Num());
 FString Json;const auto Writer=TJsonWriterFactory<>::Create(&Json);const bool Serialized=FJsonSerializer::Serialize(Manifest,Writer);
 Check(S,S->Frames.Num()==(G&&G->Apollo?34:G&&G->Mars?36:41),TEXT("All preflight, panel, circuit-result and world frames were processed"));
 if(!Serialized||!FFileHelper::SaveStringToFile(Json,*(S->Folder/TEXT("field-operations-manifest.json"))))++S->Failed;
 S->Report+=FString::Printf(TEXT("RESULT: %d failures\n"),S->Failed);
 if(!FFileHelper::SaveStringToFile(S->Report,*(S->Folder/TEXT("field-operations-review-checks.txt"))))++S->Failed;
 UE_LOG(LogTemp,Display,TEXT("FIELD_OPERATIONS_REVIEW %s frames=%d folder=%s\n%s"),S->Failed?TEXT("FAILED"):TEXT("COMPLETE"),S->Frames.Num(),*S->Folder,*S->Report);
 FPlatformMisc::RequestExitWithStatus(false,S->Failed?1:0);
}
void GuardChecks(ALunarGameMode* G,FFieldReviewState S){
 auto* C=G->FieldOperations.Get();auto* P=C?C->Pilot():nullptr;auto* UI=Panel(G);if(!C||!P||!UI){Check(S,false,TEXT("Briefing guard fixture has an actual operations panel"));return;}
 auto* Old=FindButton(UI,TEXT("BEGINNER"));const int32 Difficulty=C->Difficulty,Revision=C->Revision;UI->Refresh(true);if(Old)Old->Activate();
 Check(S,Old&&C->Difficulty==Difficulty&&C->Revision==Revision,TEXT("Detached previous-generation difficulty callback cannot change the component"));
 auto* Fresh=FindButton(UI,TEXT("BEGINNER"));P->ActivePanel=nullptr;if(Fresh)Fresh->Activate();P->ActivePanel=UI;
 Check(S,Fresh&&C->Difficulty==Difficulty&&C->Revision==Revision,TEXT("An unowned panel callback cannot change difficulty or revision"));
 const auto Stage=G->Stage;G->Stage=ELunarStage::Won;if(Fresh)Fresh->Activate();G->Stage=Stage;
 Check(S,Fresh&&C->Difficulty==Difficulty&&C->Revision==Revision,TEXT("A frozen end-state panel callback cannot change difficulty or revision"));
 Check(S,Click(G,TEXT("PROFESSIONAL"))&&C->Difficulty==2,TEXT("A current owned difficulty button changes the real component selection"));
}
void Capture(ALunarGameMode* G,FFieldReviewState S,const FString& Name,bool Seeded,bool ScrollEnd){
 if(S->Finished)return;auto* C=G?G->FieldOperations.Get():nullptr;auto* UI=Panel(G);if(!G||!C||!UI){Check(S,false,Name+TEXT(" / actual panel missing"));Finish(G,S);return;}
 UI->Refresh(true);UI->ForceLayoutPrepass();if(UI->OperationsScroll){if(Name==TEXT("resources150")&&UI->OperationsMap)UI->OperationsScroll->ScrollWidgetIntoView(UI->OperationsMap.Get(),false,EDescendantScrollDestination::Center);else if(ScrollEnd)UI->OperationsScroll->ScrollToEnd();else UI->OperationsScroll->ScrollToStart();}
 FTimerHandle LayoutWait;G->GetWorldTimerManager().SetTimer(LayoutWait,FTimerDelegate::CreateWeakLambda(G,[G,S,Name,Seeded,ScrollEnd](){
  if(S->Finished)return;if(S->Screenshot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(S->Screenshot);S->Waiting=true;const int32 Number=++S->ShotNumber;
  S->Screenshot=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(G,[G,S,Name,Seeded,ScrollEnd,Number](){
   if(S->Finished||!S->Waiting||Number!=S->ShotNumber)return;S->Waiting=false;auto* C=G->FieldOperations.Get();auto* UI=Panel(G);auto* P=C?C->Pilot():nullptr;auto* PC=UGameplayStatics::GetPlayerController(G,0);int32 Width=0,Height=0;if(PC)PC->GetViewportSize(Width,Height);FString Reason;
   bool Good=UI&&C&&P&&PC&&Width>0&&Height>0&&C->CanUsePanel(UI,false)&&UI->ValidateLayout(Reason);
   if(C&&C->Preparing)Good=Good&&!G->Paused&&G->Stage==ELunarStage::Briefing;
   else Good=Good&&G->Paused&&G->IsActive();
   if(Name==TEXT("preflight-checking150"))Good=Good&&C&&C->PendingCheck==0&&C->ActionProgress()>0&&C->ActionProgress()<1;
   if(Name==TEXT("preflight-ready150"))Good=Good&&C&&C->Prepared&&C->PreflightMask==15&&C->PendingCheck==INDEX_NONE;
   Check(S,Good,Name+(Reason.IsEmpty()?FString():TEXT(" / ")+Reason));
   auto Frame=MakeShared<FJsonObject>();Frame->SetStringField(TEXT("name"),Name);Frame->SetStringField(TEXT("view"),UI?ViewKey(UI->View):TEXT("missing"));Frame->SetBoolField(TEXT("layout_passed"),Good);Frame->SetNumberField(TEXT("width"),Width);Frame->SetNumberField(TEXT("height"),Height);Frame->SetNumberField(TEXT("text_scale"),ULunarSettings::Get()->TextScale);Frame->SetBoolField(TEXT("paused"),G->Paused);Frame->SetBoolField(TEXT("keyboard_focus"),UI&&UI->HasKeyboardFocus());Frame->SetBoolField(TEXT("fixture_state_seeded"),Seeded);Frame->SetBoolField(TEXT("scrolled_to_sources"),ScrollEnd&&Name==TEXT("sample-sources150"));Frame->SetBoolField(TEXT("map_focused"),Name==TEXT("resources150"));Frame->SetNumberField(TEXT("map_marker_count"),UI&&UI->OperationsMap?UI->OperationsMap->MarkerCount():0);
   if(C){Frame->SetNumberField(TEXT("preflight_mask"),C->PreflightMask);Frame->SetNumberField(TEXT("pending_check"),C->PendingCheck);Frame->SetNumberField(TEXT("action_progress"),C->ActionProgress());Frame->SetBoolField(TEXT("briefed"),C->Briefed);Frame->SetBoolField(TEXT("vehicle_inspected"),C->VehicleInspected);Frame->SetNumberField(TEXT("wire_mask"),C->WireMask);Frame->SetNumberField(TEXT("wire_required"),C->WirePairs());Frame->SetStringField(TEXT("component_state"),C->ExportState());TArray<TSharedPtr<FJsonValue>> Markers;for(const auto& Point:C->MapPoints()){auto M=MakeShared<FJsonObject>();M->SetStringField(TEXT("label"),Point.Label);M->SetNumberField(TEXT("index"),Point.Index);M->SetNumberField(TEXT("x_cm"),Point.Position.X);M->SetNumberField(TEXT("y_cm"),Point.Position.Y);M->SetNumberField(TEXT("z_cm"),Point.Position.Z);M->SetBoolField(TEXT("completed"),Point.Completed);M->SetBoolField(TEXT("current"),Point.Current);Markers.Add(MakeShared<FJsonValueObject>(M));}Frame->SetArrayField(TEXT("actual_world_markers"),Markers);}
   S->Frames.Add(MakeShared<FJsonValueObject>(Frame));FTimerHandle NextWait;G->GetWorldTimerManager().SetTimer(NextWait,FTimerDelegate::CreateWeakLambda(G,[G,S](){Next(G,S);}),.15f,false);
  });
  FScreenshotRequest::RequestScreenshot(S->Folder/(Name+TEXT(".png")),true,false);
  FTimerHandle ShotTimeout;G->GetWorldTimerManager().SetTimer(ShotTimeout,FTimerDelegate::CreateWeakLambda(G,[G,S,Number](){if(!S->Finished&&S->Waiting&&S->ShotNumber==Number){Check(S,false,TEXT("Rendered screenshot callback timed out"));Finish(G,S);}}),12.f,false);
 }),.45f,false);
}
int32 VisibleMeshes(AActor* Actor){if(!IsValid(Actor)||Actor->IsHidden())return 0;TArray<UStaticMeshComponent*> Parts;Actor->GetComponents(Parts);int32 Count=0;for(auto* Part:Parts)if(Part&&Part->GetStaticMesh()&&Part->IsVisible())++Count;return Count;}
void CaptureWorld(ALunarGameMode* G,FFieldReviewState S,const FString& Name){
 auto* C=G->FieldOperations.Get();auto* P=C?C->Pilot():nullptr;if(!C||!P||!G->FlightHUD){Check(S,false,Name+TEXT(" / world fixture missing"));Finish(G,S);return;}
 if(Name!=TEXT("world-sampler-package")&&!Name.StartsWith(TEXT("world-selene-props-")))P->UpdateSuit(0);C->RefreshWorldVisibility();G->FlightHUD->Refresh();FTimerHandle Wait;
 G->GetWorldTimerManager().SetTimer(Wait,FTimerDelegate::CreateWeakLambda(G,[G,S,Name](){
  if(S->Finished)return;if(S->Screenshot.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(S->Screenshot);S->Waiting=true;const int32 Number=++S->ShotNumber;
  S->Screenshot=FScreenshotRequest::OnScreenshotRequestProcessed().AddWeakLambda(G,[G,S,Name,Number](){
   if(S->Finished||!S->Waiting||Number!=S->ShotNumber)return;S->Waiting=false;auto* C=G->FieldOperations.Get();auto* P=C?C->Pilot():nullptr;auto* PC=UGameplayStatics::GetPlayerController(G,0);int32 W=0,H=0;if(PC)PC->GetViewportSize(W,H);FString Reason;
   bool Good=C&&P&&PC&&W>0&&H>0&&!P->ActivePanel&&!G->Paused&&G->IsActive()&&G->FlightHUD&&G->FlightHUD->IsVisible()&&G->FlightHUD->ValidateLayout(Reason);
   int32 ScannerParts=0;if(C)for(const auto& Part:C->ScannerVisuals)if(Part&&Part->IsVisible())++ScannerParts;
   if(Name.StartsWith(TEXT("world-rover-")))Good=Good&&C&&C->Riding()&&PC&&PC->GetViewTarget()==C->Vehicle.Get()&&VisibleMeshes(C->Vehicle.Get())>0;
   if(Name==TEXT("world-magnifier-cartridge"))Good=Good&&C&&C->ScannerActive&&P&&P->Camera&&FMath::IsNearlyEqual(P->Camera->FieldOfView,35.f)&&C->NearbyPackage(false)==0&&ScannerParts>0;
   if(Name==TEXT("world-sampler-package"))Good=Good&&C&&VisibleMeshes(C->SampleRobot.Get())>0&&C->SamplePackages.IsValidIndex(0)&&VisibleMeshes(C->SamplePackages[0].Get())>0;
   Check(S,Good,Name+(Reason.IsEmpty()?FString():TEXT(" / ")+Reason));auto F=MakeShared<FJsonObject>();F->SetStringField(TEXT("name"),Name);F->SetStringField(TEXT("view"),TEXT("world"));F->SetBoolField(TEXT("world_no_panel"),true);F->SetStringField(TEXT("layout_scope"),TEXT("flight_hud"));F->SetBoolField(TEXT("panel_layout_tested"),false);F->SetBoolField(TEXT("layout_passed"),Good);F->SetBoolField(TEXT("fixture_state_seeded"),true);F->SetNumberField(TEXT("width"),W);F->SetNumberField(TEXT("height"),H);F->SetNumberField(TEXT("text_scale"),ULunarSettings::Get()->TextScale);F->SetBoolField(TEXT("paused"),G->Paused);F->SetBoolField(TEXT("vehicle_mounted"),C&&C->Riding());F->SetBoolField(TEXT("production_follow_camera"),Name.StartsWith(TEXT("world-rover-")));F->SetBoolField(TEXT("scanner_active"),C&&C->ScannerActive);F->SetNumberField(TEXT("visible_scanner_parts"),ScannerParts);F->SetNumberField(TEXT("visible_vehicle_meshes"),C?VisibleMeshes(C->Vehicle.Get()):0);F->SetNumberField(TEXT("visible_sampler_meshes"),C?VisibleMeshes(C->SampleRobot.Get()):0);F->SetBoolField(TEXT("performance_measurement"),false);F->SetBoolField(TEXT("human_playtest"),false);
   if(PC){FVector Eye;FRotator Aim;PC->GetPlayerViewPoint(Eye,Aim);F->SetNumberField(TEXT("camera_x_cm"),Eye.X);F->SetNumberField(TEXT("camera_y_cm"),Eye.Y);F->SetNumberField(TEXT("camera_z_cm"),Eye.Z);F->SetNumberField(TEXT("camera_pitch"),Aim.Pitch);F->SetNumberField(TEXT("camera_yaw"),Aim.Yaw);}
   S->Frames.Add(MakeShared<FJsonValueObject>(F));FTimerHandle NextWait;G->GetWorldTimerManager().SetTimer(NextWait,FTimerDelegate::CreateWeakLambda(G,[G,S](){Next(G,S);}),.15f,false);
  });FScreenshotRequest::RequestScreenshot(S->Folder/(Name+TEXT(".png")),true,false);
  FTimerHandle Timeout;G->GetWorldTimerManager().SetTimer(Timeout,FTimerDelegate::CreateWeakLambda(G,[G,S,Number](){if(!S->Finished&&S->Waiting&&S->ShotNumber==Number){Check(S,false,TEXT("World screenshot callback timed out"));Finish(G,S);}}),12.f,false);
 }),.5f,false);
}
void Next(ALunarGameMode* G,FFieldReviewState S){
 if(S->Finished)return;auto* C=G?G->FieldOperations.Get():nullptr;auto* P=C?C->Pilot():nullptr;if(!G||!C||!P){Check(S,false,TEXT("Review retains its mission component and astronaut"));Finish(G,S);return;}
 const int32 Index=S->Step++;auto* Settings=ULunarSettings::Get();
 if(Index==0){Settings->TextScale=Settings->SubtitleScale=1;Check(S,C->BeginPreflight(),TEXT("Actual preflight flow opens its owned unpaused operations panel"));Capture(G,S,TEXT("preflight100"));return;}
 if(Index==1){Settings->TextScale=Settings->SubtitleScale=1.5f;Capture(G,S,TEXT("preflight150"));return;}
 if(Index==2){
  // Capture the actual preflight tabs at both supported text scales. These
  // read-only views retain unfinished checks and never seed mission progress.
  if(S->PreflightPreview<6){
   const int32 Preview=S->PreflightPreview++;--S->Step;Settings->TextScale=Settings->SubtitleScale=Preview>=3?1.5f:1.f;
   if(Preview==0||Preview==4){Check(S,Click(G,TEXT("Map")),TEXT("Preflight map preview opens through its owned button"));Click(G,TEXT("Local mission map"));}
   if(Preview==1||Preview==5)Check(S,Click(G,TEXT("Planet overview")),TEXT("Preflight globe preview uses the retained offline context material"));
   if(Preview==2)Check(S,Click(G,TEXT("Inventory")),TEXT("Preflight backpack preview opens through its owned button"));
   const TCHAR* Names[]={TEXT("preflight-map100"),TEXT("preflight-globe100"),TEXT("preflight-inventory100"),TEXT("preflight-inventory150"),TEXT("preflight-map150"),TEXT("preflight-globe150")};
   Capture(G,S,Names[Preview]);return;
  }
  Check(S,Click(G,TEXT("Map"))&&Panel(G)->View==EFieldPanelView::Resources&&!C->CanUsePanel(Panel(G),true),TEXT("Preflight Map opens as a read-only page without starting gameplay"));
  Check(S,Click(G,TEXT("Zoom +"))&&Panel(G)->OperationsMap&&Panel(G)->OperationsMap->ZoomFactor()>1,TEXT("Map zoom button changes the actual map viewport"));
  Check(S,Click(G,TEXT("Reset map"))&&Panel(G)->OperationsMap&&Panel(G)->OperationsMap->ZoomFactor()==1,TEXT("Map reset restores a bounded overview"));
  if(auto* UI=Panel(G))UI->ForceLayoutPrepass();
  Check(S,Click(G,TEXT("Planet overview")),TEXT("Planet overview button uses the actual map page callback"));
  auto* Map=Panel(G)?Panel(G)->OperationsMap.Get():nullptr;auto* Globe=Map&&Map->WidgetTree?Cast<UImage>(Map->WidgetTree->FindWidget(TEXT("PlanetContext"))):nullptr;auto* GlobeMaterial=Globe?Globe->GetDynamicMaterial():nullptr;
  Check(S,Globe&&Globe->IsVisible()&&GlobeMaterial,TEXT("Offline NASA planet context uses a real retained material and texture"));
  if(Map&&GlobeMaterial){const float BeforeYaw=GlobeMaterial->K2_GetScalarParameterValue(TEXT("Yaw"));Map->PanBy(FVector2D(.1,.05));Check(S,GlobeMaterial->K2_GetScalarParameterValue(TEXT("Yaw"))!=BeforeYaw,TEXT("Planet drag math updates the actual sphere material rotation"));Map->PanBy(FVector2D(0,1));Check(S,FMath::Abs(GlobeMaterial->K2_GetScalarParameterValue(TEXT("Pitch")))>80,TEXT("Planet drag can rotate over a pole rather than stopping at the old vertical clamp"));Map->ResetView();}
  Check(S,Click(G,TEXT("Local mission map"))&&Globe&&!Globe->IsVisible(),TEXT("Local map can be restored without changing mission state"));
  Check(S,Click(G,TEXT("1. Lander"))&&C->WaypointIndex==0&&C->PreflightMask==0&&G->Stage==ELunarStage::Briefing,TEXT("Preflight location selection tracks a real point without completing checks or starting the mission"));
  Check(S,Click(G,TEXT("Inventory"))&&Panel(G)->View==EFieldPanelView::Inventory&&!C->PerformAction(TEXT("Deposit")),TEXT("Preflight Inventory cannot award or deposit cargo"));
  Check(S,Click(G,TEXT("Tools"))&&Click(G,TEXT("Optical magnifier"))&&!Click(G,TEXT("Equip magnifier"))&&C->CollectedMask==0,TEXT("Actual backpack filter and item selection preserve the scanner unlock and cargo guards"));
  Check(S,Click(G,TEXT("All")),TEXT("Backpack All filter restores the actual available item cards"));
  Check(S,Click(G,TEXT("EVA checks"))&&C->PreflightMask==0,TEXT("Returning to EVA checks preserves unfinished systems"));
  Check(S,!Click(G,TEXT("Next /"))&&!Click(G,TEXT("Checks complete /")),TEXT("Next and Open Hatch remain disabled before the required checks"));
  Check(S,Click(G,TEXT("1 / Inspect suit seals"))&&C->PendingCheck==0,TEXT("Actual first suit-check button starts the component timer"));S->FeedChecks=S->AwaitProgress=true;return;
 }
 if(Index==3){S->FeedChecks=S->AwaitReady=true;return;}
 if(Index==4){
  if(P->ActivePanel)P->ClosePanel();ALunarDepartureScene::Abort(G);C->Preparing=false;G->Stage=G->Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule;G->Paused=false;G->Journal=false;G->Holding=false;G->RadioRemaining=0;G->ApolloTool=0;
 }
 if(Index>=4&&Index<=15){
  if(Index==12&&!G->Apollo&&S->WirePreview<2){
   const int32 Preview=S->WirePreview++;--S->Step;
   if(Preview==0){for(int32 Left=0;Left<C->WirePairs();++Left)if(!(C->WireMask&(1<<Left))){int32 Right=INDEX_NONE;for(int32 J=0;J<C->WirePairs();++J)if(C->WireLeftLabel(Left)==C->WireRightLabel(J))Right=J;Check(S,C->PerformAction(TEXT("WireLeft"),Left)&&C->PerformAction(TEXT("WireRight"),Right),TEXT("Remaining reference cables match through production authority"));}Check(S,C->WireComplete&&C->WirePendingCommit&&G->RepairStep==1,TEXT("Fully drawn cables remain isolated before Verify"));Capture(G,S,TEXT("wire-ready-to-verify150"),true);}
   else{Check(S,Click(G,TEXT("Verify"))&&G->RepairStep==2&&!C->WirePendingCommit,TEXT("Actual Verify button applies exactly the native cable step"));Check(S,!P->ActivePanel&&!G->Paused,TEXT("Verified System Connect closes immediately and returns control"));CaptureWorld(G,S,TEXT("world-system-connected"));}
   return;
  }
  const auto View=EFieldPanelView((Index-4)/2+1);const bool Large=(Index&1)!=0;Settings->TextScale=Settings->SubtitleScale=Large?1.5f:1.f;
  if(Index==6){Check(S,Click(G,TEXT("Confirm resource profile"))&&C->Briefed,TEXT("Actual briefing confirmation locks the gameplay budget"));C->ElapsedS=83.5f;C->WalkingDistanceM=35;C->VehicleDistanceM=112;C->VehicleTimeS=22.4f;C->UtilityCharge=FMath::Max(0.f,C->UtilityCapacity-2.1f);C->DataStoredMB=FMath::Min(.4f,C->DataCapacityMB);++C->Revision;}
  if(Index==8&&C->Vehicle){P->SetActorLocation(C->Vehicle->GetActorLocation()+FVector(0,-180,120),false,nullptr,ETeleportType::TeleportPhysics);FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);if(auto* PC=UGameplayStatics::GetPlayerController(G,0))PC->SetControlRotation((C->Vehicle->GetActorLocation()-Eye).Rotation());}
  if(Index==10&&!G->Apollo){G->Stage=ELunarStage::RestorePower;G->RepairStep=1;C->WireIsolated=false;C->WireComplete=C->WirePendingCommit=false;C->WireMask=0;C->WireLeft=INDEX_NONE;++C->Revision;}
  if(Index==12){C->ScannedMask=C->CollectedMask=3;C->DeliveredMask=1;C->CargoMassKG=C->TrainingPackageMassKG;C->LastPackage=1;++C->Revision;}
  if(Index==14){C->ScannedMask=7;C->SelectedPackage=2;++C->Revision;}
  Check(S,C->OpenPanel(View),FString::Printf(TEXT("Actual %s page opens or configures the owned panel"),ViewKey(View)));
  if(auto* UI=Panel(G))UI->Refresh(true);
  if(Index==10&&!G->Apollo){bool Isolated=false;auto* PC=UGameplayStatics::GetPlayerController(G,0);if(G->PowerActor&&PC)for(const FVector Offset:{FVector(-220,0,75),FVector(0,-220,75),FVector(220,0,75),FVector(0,220,75)}){P->SetActorLocation(G->PowerActor->GetActorLocation()+Offset,false,nullptr,ETeleportType::TeleportPhysics);FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);PC->SetControlRotation((G->PowerActor->GetActorLocation()-Eye).Rotation());if(Click(G,TEXT("Isolate training circuit"))){Isolated=true;break;}}Check(S,Isolated,TEXT("The actual isolate button requires a nearby visible mission panel before cable input"));}
  if(Index==4)GuardChecks(G,S);
  if(Index==9)Check(S,Click(G,TEXT("Inspect vehicle systems"))&&C->VehicleInspected&&C->ScannerUnlocked,TEXT("Actual nearby vehicle-inspection button unlocks the scanner"));
  if(Index==11&&!G->Apollo){
   if(auto* UI=Panel(G)){TArray<UWidget*> Widgets;UI->WidgetTree->GetAllWidgets(Widgets);for(auto* W:Widgets)if(auto* Area=Cast<UExpandableArea>(W))Area->SetIsExpanded(true);UI->Refresh(true);Widgets.Reset();UI->WidgetTree->GetAllWidgets(Widgets);UExpandableArea* Retained=nullptr;for(auto* W:Widgets)if(auto* Area=Cast<UExpandableArea>(W))Retained=Area;Check(S,Retained&&Retained->GetIsExpanded(),TEXT("Keyboard/click alternatives remain expanded when cable state refreshes the page"));if(Retained)Retained->SetIsExpanded(false);}
   ULunarWireBoard* Board=nullptr;if(auto* UI=Panel(G)){TArray<UWidget*> Widgets;UI->WidgetTree->GetAllWidgets(Widgets);for(auto* W:Widgets)if(auto* Found=Cast<ULunarWireBoard>(W)){Board=Found;break;}}
   Check(S,Board&&Board->BeginCable(0)&&!Board->ReleaseCable(INDEX_NONE)&&C->WireMask==0,TEXT("Production cable drag can be cancelled outside a socket without awarding progress"));int32 Right=INDEX_NONE;for(int32 I=0;I<C->WirePairs();++I)if(C->WireRightLabel(I)==C->WireLeftLabel(0)){Right=I;break;}
   Check(S,Board&&Board->BeginCable(0)&&Board->ReleaseCable(Right)&&(C->WireMask&1)!=0&&G->RepairStep==1,TEXT("Production cable release records a matching signal without applying the mission repair before Verify"));
  }
  Capture(G,S,FString(ViewKey(View))+(Large?TEXT("150"):TEXT("100")),true);return;
 }
 if(Index==16){Settings->TextScale=Settings->SubtitleScale=1.5f;Check(S,Click(G,TEXT("Open Science & Sources")),TEXT("Actual source toggle opens its optional source section"));Capture(G,S,TEXT("sample-sources150"),true,true);return;}
 if(Index==17){
  if(P->ActivePanel)P->ClosePanel();G->Paused=G->Journal=G->Holding=false;Settings->TextScale=Settings->SubtitleScale=1;C->CancelScanner();P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();
  if(!C->Vehicle){Check(S,false,TEXT("World rover fixture requires the actual training vehicle"));Finish(G,S);return;}
  P->SetActorLocation(C->Vehicle->GetActorTransform().TransformPosition(FVector(-240,0,80)),false,nullptr,ETeleportType::TeleportPhysics);Check(S,ALunarSurfaceVehicle::TryInteract(G)&&C->Riding(),TEXT("World fixture mounts the actual stationary rover through production interaction"));
  // Preserve the camera and stationary tick that actual mounted gameplay uses.
  C->Vehicle->SetForward(0);C->Vehicle->SetSteer(0);C->Vehicle->SetActorTickEnabled(true);
  CaptureWorld(G,S,TEXT("world-rover-mounted"));return;
 }
 if(Index==18&&S->RoverView<8){
  const int32 View=S->RoverView++;--S->Step;auto* V=C->Vehicle.Get();if(!V||!C->Riding()){Check(S,false,TEXT("Rover orbit fixture retains its mounted vehicle"));Finish(G,S);return;}
  const TCHAR* Names[]={TEXT("world-rover-left"),TEXT("world-rover-front"),TEXT("world-rover-right"),TEXT("world-rover-above"),TEXT("world-rover-cockpit"),TEXT("world-rover-cockpit-controls"),TEXT("world-rover-cockpit-left"),TEXT("world-rover-follow-restored")};
  if(View<3)P->Turn(55.f);else if(View==3)P->Look(20.f);else if(View==4)P->ToggleSuitView();else if(View==5)P->Look(34.f);else if(View==6){P->Look(-34.f);P->Turn(-45.f);}else P->ToggleSuitView();
  CaptureWorld(G,S,Names[View]);return;
 }
 if(Index==18){
  ALunarSurfaceVehicle::Abort(G);auto* PC=UGameplayStatics::GetPlayerController(G,0);if(!PC||!C->SamplePackages.IsValidIndex(0)||!IsValid(C->SamplePackages[0])){Check(S,false,TEXT("Magnifier world fixture requires its actual labelled package"));Finish(G,S);return;}
  C->ScannedMask=C->CollectedMask=C->DeliveredMask=0;C->CargoMassKG=C->DataStoredMB=0;C->UtilityCharge=FMath::Min(20.f,C->UtilityCapacity);C->RefreshWorldVisibility();auto* Package=C->SamplePackages[0].Get();FVector Focus=Package->GetActorLocation();TArray<UStaticMeshComponent*> Parts;Package->GetComponents(Parts);for(auto* Part:Parts)if(Part&&Part->ComponentHasTag(TEXT("work.focus"))){Focus=Part->GetComponentLocation();break;}
  P->SetActorTickEnabled(false);P->GetCharacterMovement()->DisableMovement();P->ThirdPerson=false;P->SetActorHiddenInGame(false);P->Camera->bUsePawnControlRotation=true;P->Camera->SetRelativeLocation(FVector(0,0,68));PC->SetViewTarget(P);const FTransform PackageFrame=Package->GetActorTransform();Focus=PackageFrame.TransformPosition(FVector(-16,0,84));bool FrontAligned=false;
  for(float Degrees:{0.f,-25.f,25.f,-45.f,45.f}){const float Angle=FMath::DegreesToRadians(Degrees);const FVector Local(-300*FMath::Cos(Angle),-300*FMath::Sin(Angle),100);P->SetActorLocation(PackageFrame.TransformPosition(Local),false,nullptr,ETeleportType::TeleportPhysics);const FRotator Aim=(Focus-P->Camera->GetComponentLocation()).Rotation();PC->SetControlRotation(Aim);P->Camera->SetWorldRotation(Aim);if(C->NearbyPackage(false)==0){FrontAligned=true;break;}}
  Check(S,FrontAligned,TEXT("Local-front magnifier camera retains real package facing range and line-of-sight eligibility"));
  Check(S,C->ToggleScanner()&&C->ScannerActive&&C->NearbyPackage(false)==0,TEXT("World magnifier uses its actual unlock and observed-package eligibility"));CaptureWorld(G,S,TEXT("world-magnifier-cartridge"));return;
 }
 if(Index==19){
  C->CancelScanner();auto* PC=UGameplayStatics::GetPlayerController(G,0);if(!PC||!C->SamplePackages.IsValidIndex(0)||!IsValid(C->SamplePackages[0])){Check(S,false,TEXT("Sampler world fixture requires its authored package"));Finish(G,S);return;}
  const FTransform Frame=C->SamplePackages[0]->GetActorTransform();const FVector Focus=Frame.TransformPosition(FVector(0,0,55)),Eye=Frame.TransformPosition(FVector(-250,-210,160));P->SetActorHiddenInGame(true);P->Camera->bUsePawnControlRotation=false;P->Camera->SetWorldLocation(Eye);P->Camera->SetWorldRotation((Focus-Eye).Rotation());P->Camera->SetFieldOfView(70);PC->SetViewTarget(P);CaptureWorld(G,S,TEXT("world-sampler-package"));return;
 }
 if(Index>=20&&Index<25&&!G->Apollo&&!G->Mars){
  auto* PC=UGameplayStatics::GetPlayerController(G,0);AActor* FocusActor=Index<22?G->ModuleActor.Get():G->PowerActor.Get();if(!PC||!FocusActor){Check(S,false,TEXT("SELENE prop review anchors exist"));Finish(G,S);return;}
  G->Stage=Index<22?ELunarStage::FindModule:ELunarStage::RestorePower;G->RepairStep=0;G->PowerSequenceActive=false;G->StationRebootComplete=false;G->Paused=false;
  if(Index==22){G->Stage=ELunarStage::FindModule;P->Camera->bUsePawnControlRotation=true;P->SetActorLocation(G->ModuleActor->GetActorLocation()+FVector(-100,0,70),false,nullptr,ETeleportType::TeleportPhysics);P->Camera->SetRelativeLocation(FVector(0,0,68));const FRotator Aim=(G->ModuleActor->GetActorLocation()-P->Camera->GetComponentLocation()).Rotation();PC->SetControlRotation(Aim);P->Camera->SetWorldRotation(Aim);G->Interact();Check(S,G->Stage==ELunarStage::RestorePower&&G->ModuleActor->IsHidden(),TEXT("SELENE world review uses a real module pickup before the 0/3 station views"));}
  G->RadioRemaining=0;
  const FVector Focus=Index==24?FocusActor->GetActorLocation()+FVector(-30,0,30):Index<22?FocusActor->GetActorLocation()+FVector(200,100,100):FVector(700,-1000,700);
  const FVector Eye=Focus+(Index==24?FVector(-220,-140,140):Index<22?((Index&1)?FVector(-650,700,220):FVector(-850,-650,180)):((Index&1)?FVector(-2000,1600,700):FVector(-2000,-1600,700)));P->Camera->bUsePawnControlRotation=false;P->Camera->SetWorldLocation(Eye);P->Camera->SetWorldRotation((Focus-Eye).Rotation());P->Camera->SetFieldOfView(75);PC->SetViewTarget(P);
  CaptureWorld(G,S,FString::Printf(TEXT("world-selene-props-%d"),Index-20));return;
 }
 Finish(G,S);
}
void Pump(ALunarGameMode* G,FFieldReviewState S){
 if(S->Finished)return;const double Now=G->GetWorld()->GetTimeSeconds(),Delta=FMath::Clamp(Now-S->LastGameTime,0.,2.);S->LastGameTime=Now;if(Now-S->StartedGameTime>100){Check(S,false,TEXT("Bounded rendered review exceeded its world-time deadline"));Finish(G,S);return;}
 auto* C=G->FieldOperations.Get();if(!C){Check(S,false,TEXT("Operations component disappeared"));Finish(G,S);return;}
 if(S->FeedChecks)C->Advance(float(Delta));
 if(S->AwaitProgress&&C->PendingCheck==0&&C->ActionProgress()>=.35f){
  if(auto* UI=Panel(G)){UI->Refresh();const int32 Before=UI->PageGeneration();auto* Progress=UI->CheckProgress.Get();C->Advance(.01f);UI->Refresh();Check(S,UI->PageGeneration()==Before&&UI->CheckProgress==Progress&&Progress&&FMath::IsNearlyEqual(Progress->GetPercent(),C->ActionProgress(),.001f),TEXT("Progress updates in place without rebuilding the page or replacing its widgets"));}
  S->FeedChecks=S->AwaitProgress=false;Capture(G,S,TEXT("preflight-checking150"));return;
 }
 if(S->AwaitReady){
  if(C->Prepared){S->FeedChecks=S->AwaitReady=false;Check(S,C->PreflightMask==15&&C->PendingCheck==INDEX_NONE,TEXT("Four ordered real check callbacks complete through component game-time deltas"));Capture(G,S,TEXT("preflight-ready150"));return;}
  if(C->PendingCheck==INDEX_NONE){if(auto* UI=Panel(G))UI->Refresh(true);const int32 CheckIndex=C->PreflightStep();Check(S,!FindButton(Panel(G),TEXT("Next /"))&&C->PreflightCursor==CheckIndex,TEXT("Next completed system is directly clickable and no Next button exists"));Check(S,Click(G,FString::Printf(TEXT("%d /"),CheckIndex+1))&&C->PendingCheck==CheckIndex,FString::Printf(TEXT("Actual ordered suit-check button %d starts its timer"),CheckIndex+1));}
 }
}
}

void ALunarGameMode::ValidateFieldOperationsReview(){
 auto S=MakeShared<FFieldReview>();S->Folder=FPaths::ProjectDir()/TEXT("Reports/FieldOperations20261104")/(Apollo?TEXT("review-apollo"):Mars?TEXT("review-mars"):TEXT("review-selene"));IFileManager::Get().MakeDirectory(*S->Folder,true);
 auto* Settings=ULunarSettings::Get();S->OriginalText=Settings->TextScale;S->OriginalSubtitle=Settings->SubtitleScale;S->StartedGameTime=S->LastGameTime=GetWorld()->GetTimeSeconds();auto* C=FieldOperations.Get();auto* P=C?C->Pilot():nullptr;
 Check(S,!Menu&&C&&C->Enabled&&P,TEXT("Rendered field review has an enabled extension in a playable mission world"));if(Menu||!C||!C->Enabled||!P){Finish(this,S);return;}
 if(P->ActivePanel)P->ClosePanel();ALunarDepartureScene::Abort(this);Stage=ELunarStage::Briefing;Paused=Journal=Holding=false;C->ResetNewRun(false);SetActorTickEnabled(false);P->SetActorTickEnabled(false);P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();
 GetWorldTimerManager().SetTimer(S->Pump,FTimerDelegate::CreateWeakLambda(this,[this,S](){Pump(this,S);}),.05f,true);Next(this,S);
}
