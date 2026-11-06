#include "LunarApolloRegression.h"
#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarOperationsPanel.h"
#include "LunarDeparture.h"
#include "LunarSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

namespace LunarApolloRegression {
namespace {
// Process-local handover lets the real R/map-travel path verify a fresh map.
bool AwaitingRestart=false;
FString RestartReport;
int32 RestartFailures=0;
float RestartMetres=0,RestartSeconds=0;
class FRun : public TSharedFromThis<FRun> {
 TWeakObjectPtr<ALunarGameMode> Game;
 FString Report;
 int32 Failures=0,Phase=0,LastTool=0,LastAction=-1;
 ELunarStage LastStage=ELunarStage::Briefing;
 double PreviousTime=0,StepStart=0,ToolOpened=0,PauseStarted=0;
 float Seconds=0,PhysicalMetres=0,PausedOxygen=0,PausedElapsed=0,PausedCharge=0;
 FVector PreviousPosition=FVector::ZeroVector,FieldKitPosition=FVector::ZeroVector;
 bool HoldingW=false,TestedDistance=false,TestedPause=false,TestedLevel=false,TestedWeak=false,TestedLegacy=false,TestedBlocked=false;
 bool ArrowLeft=false,ArrowRight=false,ArrowUp=false,ArrowDown=false;
 int32 ReturnWaypoint=0;
 void Check(bool Good,const FString& Name){
  Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");
  if(!Good)++Failures;
  UE_LOG(LogTemp,Display,TEXT("APOLLO_INTERACTION %s %s"),Good?TEXT("PASS"):TEXT("FAIL"),*Name);
 }
 ALunarCharacter* Pilot()const{return Game.IsValid()?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Game.Get(),0)):nullptr;}
 APlayerController* Controller()const{auto* P=Pilot();return P?Cast<APlayerController>(P->GetController()):nullptr;}
 void Key(FKey K,bool Down){
  if(auto* PC=Controller())PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,Down?IE_Pressed:IE_Released,Down?1.f:0.f));
 }
 void Tap(FKey K){
  auto* PC=Controller();auto* P=Pilot();if(!PC||!P||!PC->PlayerInput||!P->InputComponent)return;
  Key(K,true);PC->PlayerInput->ProcessInputStack({P->InputComponent.Get()},.016f,false);
  Key(K,false);PC->PlayerInput->ProcessInputStack({P->InputComponent.Get()},.016f,false);
 }
 void Walk(bool Down){if(HoldingW!=Down){HoldingW=Down;Key(ULunarSettings::Get()->Key(TEXT("MoveForward")),Down);}}
 void Arrows(bool Left=false,bool Right=false,bool Up=false,bool Down=false){
  auto Set=[&](bool& Old,bool Next,FKey K){if(Old!=Next){Old=Next;Key(K,Next);}};
  Set(ArrowLeft,Left,EKeys::Left);Set(ArrowRight,Right,EKeys::Right);Set(ArrowUp,Up,EKeys::Up);Set(ArrowDown,Down,EKeys::Down);
 }
 void Aim(FVector At){auto* P=Pilot();auto* PC=Controller();if(!P||!PC)return;FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((At-Eye).Rotation());}
 void Finish(){
  Walk(false);Arrows();
  Report+=FString::Printf(TEXT("Measured collision-enabled walking: %.2f m; game seconds: %.2f. No SetActorLocation, actor relocation, speed override or direct successful task completion is used for the walkthrough.\nRESULT: %d failures\n"),PhysicalMetres,Seconds,Failures);
  if(auto* G=Game.Get()){const bool Saved=FFileHelper::SaveStringToFile(Report,*G->ReportPath(TEXT("apollo-interaction-regression.txt")));if(!Saved)++Failures;}
  UE_LOG(LogTemp,Display,TEXT("APOLLO_INTERACTION_REPORT\n%s"),*Report);
  FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
 }
 void Schedule(){if(auto* G=Game.Get()){FTimerHandle H;const auto Self=AsShared();G->GetWorldTimerManager().SetTimer(H,[Self](){Self->Tick();},.05f,false);}}
 void LegacyCheck(){
  auto* G=Game.Get();auto* C=G->FieldOperations.Get();const FString Field=C->ExportState();
  auto* Save=G->CaptureApolloCheckpoint();FString Why;
  Check(Save&&Save->Validate(Why)&&Save->Step()==1&&Save->Has(ApolloTasks::Instruments[0])&&!Save->Has(ApolloTasks::Instruments[1]),TEXT("Actual 1/2 ALSEP checkpoint records the committed instrument only"));
  if(Save){
   Save->FieldOperationsState.Reset();TArray<uint8> Bytes;
   const bool Serialized=UGameplayStatics::SaveGameToMemory(Save,Bytes);
   auto* Legacy=Serialized?Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Bytes)):nullptr;
   Check(Legacy&&Legacy->Validate(Why)&&Legacy->Step()==1&&Legacy->FieldOperationsState.IsEmpty(),TEXT("Legacy version-1 checkpoint without additive training data survives memory serialization"));
  }
  Check(C->ImportState(TEXT(""))&&!C->Enabled&&G->Stage==ELunarStage::ApolloDeploy&&G->ApolloInstruments==1,TEXT("Empty legacy field state disables optional extension without losing ALSEP progress"));
  Tap(ULunarSettings::Get()->Key(TEXT("Interact")));
  Check(G->ApolloTool==3&&G->ApolloInstruments==1,TEXT("Legacy resumed-state contract still accepts the real work-key antenna input"));
  G->CancelApolloTool();
  Check(C->ImportState(Field)&&C->Enabled&&C->Briefed&&G->ApolloInstruments==1,TEXT("Validated modern extension state restores without consuming an instrument action"));
  FString Fixture;
  if(FParse::Value(FCommandLine::Get(),TEXT("ApolloInteractionLegacyFixture="),Fixture)){
   TArray<uint8> Bytes;const bool Read=FFileHelper::LoadFileToArray(Bytes,*Fixture);
   auto* Old=Read?Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Bytes)):nullptr;
   Check(Old&&Old->Validate(Why)&&Old->FormatVersion==1&&Old->FieldOperationsState.IsEmpty(),TEXT("Read-only legacy binary fixture decodes and validates without additive field state"));
  }else Report+=TEXT("SKIP External old binary checkpoint fixture was not supplied; memory compatibility was checked, not a disk resume.\n");
  TestedLegacy=true;
 }
public:
 explicit FRun(ALunarGameMode* G):Game(G){}
 void Begin(){
  auto* G=Game.Get();auto* P=Pilot();auto* PC=Controller();auto* C=G?G->FieldOperations.Get():nullptr;
  Check(G&&G->Apollo&&P&&PC&&PC->PlayerInput&&P->InputComponent&&C&&C->Enabled&&ULunarSettings::IsAutomation()&&!G->ApolloSaveEnabled(),TEXT("Apollo automation uses standard enabled extension and disables owner checkpoint writes"));
  if(!G||!P||!PC||!PC->PlayerInput||!C||!C->Enabled||G->ApolloSaveEnabled()){Finish();return;}
  ULunarSettings::Get()->Reset();ULunarSettings::Get()->Apply(P,false);
  PreviousPosition=P->GetActorLocation();PreviousTime=G->GetWorld()->GetTimeSeconds();
  if(AwaitingRestart){
   Report=RestartReport;Failures=RestartFailures;PhysicalMetres=RestartMetres;Seconds=RestartSeconds;AwaitingRestart=false;
   Check(G->Stage==ELunarStage::Briefing&&G->ApolloInstruments==0&&G->ApolloAction==0&&!G->ApolloTool&&G->ApolloLevel.Equals(FVector2D(.4f,-.3f))&&FMath::IsNearlyEqual(G->ApolloAntennaAngle,-82.f),TEXT("Real R restart and map travel reset ALSEP masks bubble antenna and work context"));
   Check(C->Enabled&&!C->Prepared&&!C->Briefed&&C->PreflightMask==0&&C->ElapsedS==0&&C->WalkingDistanceM==0&&G->ApolloDistance==0&&G->ApolloElapsed==0,TEXT("Reloaded mission resets required training gates and both native and support telemetry"));
   Finish();return;
  }
  Check(G->Stage==ELunarStage::Briefing&&G->ApolloInstrumentActors.Num()==2,TEXT("Mission starts at briefing with both registered ALSEP sites"));
  Tap(EKeys::Enter);
  Check(C->Preparing&&P->ActivePanel&&G->Stage==ELunarStage::Briefing,TEXT("Real start input enters the required pre-EVA panel before any surface task"));
  Phase=1;Schedule();
 }
 void Tick(){
  auto* G=Game.Get();auto* P=Pilot();auto* PC=Controller();if(!G||!P||!PC)return;
  auto* C=G->FieldOperations.Get();if(!C){Check(false,TEXT("Field operations component remains available"));Finish();return;}
  const double Now=G->GetWorld()->GetTimeSeconds();const float Delta=float(FMath::Max(0.,Now-PreviousTime));PreviousTime=Now;Seconds+=Delta;
  const float M=FVector::Dist2D(P->GetActorLocation(),PreviousPosition)/100.f;
  if(!G->Paused&&M<2.f)PhysicalMetres+=M;PreviousPosition=P->GetActorLocation();
  if(Seconds>2200||G->Stage==ELunarStage::Lost){Check(false,FString::Printf(TEXT("Bounded physical walkthrough stalled at stage %d action %d: %s"),int32(G->Stage),G->ApolloAction,*G->Radio));Finish();return;}
  if(Phase==1){
   if(C->PendingCheck==INDEX_NONE&&C->PreflightStep()<4)Check(C->PerformAction(TEXT("Check"),C->PreflightStep()),TEXT("Owned preflight callback starts the next timed check"));
   if(C->Prepared){
    Check(C->PreflightMask==15&&C->PerformAction(TEXT("Launch")),TEXT("Four elapsed training checks allow the owned hatch callback"));
    // Presentation skip does not skip preflight, field work or physical travel.
    if(G->DepartureScene)G->DepartureScene->AdvancePresentation(ALunarDepartureScene::Duration);
    Phase=2;
   }
   Schedule();return;
  }
  if(Phase==2){
   if(!G->IsActive()){Schedule();return;}
   auto* Panel=Cast<ULunarOperationsPanel>(P->ActivePanel);
   Check(Panel&&Panel->View==EFieldPanelView::Briefing&&G->Paused&&!C->Briefed,TEXT("Committed surface mission waits for its mandatory resource briefing"));
   Check(C->PerformAction(TEXT("Difficulty"),1)&&C->PerformAction(TEXT("ConfirmBriefing"))&&C->Briefed&&!G->Paused&&!P->ActivePanel,TEXT("Actual briefing callbacks lock the Intermediate budget and release native work"));
   Phase=3;LastStage=G->Stage;LastAction=G->ApolloAction;StepStart=Now;
  }
  if(Phase==4){
   Tap(ULunarSettings::Get()->Key(TEXT("Interact")));Tap(EKeys::LeftMouseButton);
   if(Now-PauseStarted<.75){Schedule();return;}
   Check(G->Paused&&!G->ApolloTool&&G->ApolloInstruments==0&&G->Oxygen==PausedOxygen&&G->ApolloElapsed==PausedElapsed&&C->UtilityCharge==PausedCharge,TEXT("Paused real-input attempts cannot move ALSEP or consume native/support resources"));
   Tap(EKeys::Escape);Check(!G->Paused&&G->ApolloInstruments==0,TEXT("Real Escape resumes uncommitted ALSEP work"));
   Phase=3;TestedPause=true;Schedule();return;
  }
  if(G->Stage!=LastStage||G->ApolloAction!=LastAction){
   if(LastStage==ELunarStage::ApolloCheckout&&G->Stage==ELunarStage::ApolloDeploy){
    FieldKitPosition=P->GetActorLocation();C->WaypointIndex=0;
    const FString Prompt=G->ApolloPrompt();
    Check(!G->CanInteract()&&Prompt.Contains(TEXT("Main objective"))&&!Prompt.Contains(TEXT("Open level adjustment"))&&FVector::Dist2D(P->GetActorLocation(),G->ApolloInstrumentActors[0]->GetActorLocation())>10000,TEXT("Lander screenshot reproduction points to distant ALSEP instead of advertising a false E adjustment"));
    Tap(ULunarSettings::Get()->Key(TEXT("Interact")));
    Check(!G->ApolloTool&&G->ApolloInstruments==0&&!P->ActivePanel,TEXT("Real E at the lander cannot complete or enter distant ALSEP"));
    G->CompleteApolloInteraction(G->ModuleActor);
    Check(G->ApolloInstruments==0&&G->ApolloAction==0,TEXT("Unrelated field-kit callback cannot commit registered ALSEP work"));
    TestedDistance=true;
   }
   if(G->Stage==ELunarStage::ApolloSamples)Check(G->ApolloInstruments==3&&G->ApolloAction==2,TEXT("Two real instrument confirmations advance ALSEP 0/2 to 1/2 to 2/2 and unlock sampling"));
   if(G->Stage==ELunarStage::ApolloTraverse)Check(G->SamplesCollected==7&&G->ApolloPhotos==7,TEXT("All three actual photo/sample workflows unlock the Surveyor traverse"));
   if(G->Stage==ELunarStage::ApolloRecover)Check(G->ApolloParts==0&&!G->SurveyorPhotographed,TEXT("Physical Surveyor arrival opens recovery without granting evidence or hardware"));
   if(G->Stage==ELunarStage::ApolloReturn)Check(G->ApolloParts==7&&G->SurveyorPhotographed,TEXT("All three actual packed hardware workflows unlock the return"));
   LastStage=G->Stage;LastAction=G->ApolloAction;StepStart=Now;
  }
  if(Now-StepStart>520){Check(false,FString::Printf(TEXT("No progress within 520 game seconds at %s, pawn %s"),*G->ApolloSiteName(),*P->GetActorLocation().ToString()));Finish();return;}
  if(G->Stage==ELunarStage::Won){
   Walk(false);Arrows();
   Check(G->ApolloInstruments==3&&G->SamplesCollected==7&&G->ApolloParts==7&&G->ApolloEvidence.Num()==4&&PhysicalMetres>500&&TestedDistance&&TestedPause&&TestedLevel&&TestedLegacy&&TestedBlocked,TEXT("Full standard-extension Apollo mission completes with measured physical travel and original science cargo"));
   Check(C->Enabled&&C->Briefed&&!G->RecorderRecovered,TEXT("Optional support pickups and recorder are not required to complete the historical mission"));
   RestartReport=Report;RestartFailures=Failures;RestartMetres=PhysicalMetres;RestartSeconds=Seconds;AwaitingRestart=true;
   RestartReport+=FString::Printf(TEXT("Measured collision-enabled walkthrough before restart: %.2f m; %.2f game seconds. No teleport or direct successful task completion.\n"),PhysicalMetres,Seconds);
   Tap(EKeys::R);
   return; // A new map starts the same isolated flag and finishes the restart check.
  }
  AActor* T=G->Target();if(!T){Schedule();return;}
  if(G->ApolloTool){
   Walk(false);
   if(G->ApolloTool!=LastTool){LastTool=G->ApolloTool;ToolOpened=Now;}
   if(G->ApolloTool==2&&!TestedPause){
    Tap(EKeys::LeftMouseButton);Check(G->ApolloTool==2&&G->ApolloInstruments==0,TEXT("Uncentered actual bubble refuses confirmation"));
    Tap(EKeys::Escape);Check(G->Paused&&!G->ApolloTool&&G->ApolloInstruments==0,TEXT("Escape cancels unfinished adjustment and pauses without recording"));
    PausedOxygen=G->Oxygen;PausedElapsed=G->ApolloElapsed;PausedCharge=C->UtilityCharge;PauseStarted=Now;Phase=4;Schedule();return;
   }
   if(G->ApolloTool==2){
    const FVector2D L=G->ApolloLevel;
    Arrows(L.X>.025f,L.X<-.025f,L.Y<-.025f,L.Y>.025f);
    if(L.Size()>.055f){Schedule();return;}
    Arrows();
   }else if(G->ApolloTool==3){
    if(!TestedWeak){Tap(EKeys::LeftMouseButton);Check(G->ApolloTool==3&&G->ApolloInstruments==1,TEXT("Weak real Earth link cannot complete the second ALSEP task"));TestedWeak=true;}
    const float Error=FMath::FindDeltaAngleDegrees(G->ApolloAntennaAngle,-7.f);
    Arrows(Error< -1.f,Error>1.f);
    if(FMath::Abs(Error)>2.f){Schedule();return;}
    Arrows();
   }
   if(Now-ToolOpened>.25&&G->ApolloToolCooldown<=0&&!G->ApolloPhotoPending){
    const int Tool=G->ApolloTool;const uint8 Mask=G->ApolloInstruments;
    Tap(EKeys::LeftMouseButton);
    if(Tool==2&&Mask==0&&G->ApolloInstruments==1){Check(G->Stage==ELunarStage::ApolloDeploy&&G->ApolloAction==1,TEXT("Centered bubble via held arrow input commits exactly 1/2 ALSEP"));TestedLevel=true;}
   }
   Schedule();return;
  }
  LastTool=0;Arrows();
  FVector Destination=T->GetActorLocation();
  // Keep the real return path outside the lander's structural footprint.
  bool Transit=false;
  if(G->Stage==ELunarStage::ApolloReturn&&ReturnWaypoint<2){
   const FVector Waypoints[]={G->HomeActor->GetActorLocation()+FVector(1900,-1900,0),G->HomeActor->GetActorLocation()+FVector(-1200,-1500,0)};
   Destination=Waypoints[ReturnWaypoint];Transit=true;
   if(FVector::Dist2D(P->GetActorLocation(),Destination)<180){++ReturnWaypoint;Schedule();return;}
  }
  const FVector PhotoOffset=G->Stage==ELunarStage::ApolloRecover&&!G->SurveyorPhotographed?FVector(0,0,90):FVector::ZeroVector;
  Aim(Destination+(Transit?FVector::ZeroVector:PhotoOffset));
  if(!Transit&&G->Stage!=ELunarStage::ApolloTraverse&&G->CanInteract()){
   Walk(false);P->GetCharacterMovement()->StopMovementImmediately();
   if(G->Stage==ELunarStage::ApolloDeploy&&G->ApolloAction==0&&!TestedBlocked){
    Check(TestedDistance&&FVector::Dist2D(FieldKitPosition,P->GetActorLocation())>10000&&PhysicalMetres>100,TEXT("Physical W input reaches the actual NASA-positioned PSE from the lander without teleporting"));
    FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);
    auto* Obstacle=G->GetWorld()->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Obstacle);
    Obstacle->SetRootComponent(Box);Box->SetBoxExtent(FVector(12,90,120));Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);Box->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);Box->RegisterComponent();
    Obstacle->SetActorLocation((Eye+T->GetActorLocation())*.5f);
    Check(!G->CanInteract()&&G->ApolloPrompt().Contains(TEXT("Reach blocked")),TEXT("A real visibility obstacle suppresses the actionable ALSEP prompt"));
    Tap(ULunarSettings::Get()->Key(TEXT("Interact")));Check(!G->ApolloTool&&G->ApolloInstruments==0,TEXT("Actual E input cannot enter or award obstructed ALSEP work"));
    Obstacle->Destroy();TestedBlocked=true;
    auto* Settings=ULunarSettings::Get();Check(Settings->Rebind(TEXT("Interact"),EKeys::U),TEXT("A free alternate work key can be bound at the physical work site"));
    Settings->Apply(P,false);Tap(EKeys::E);Check(!G->ApolloTool,TEXT("Old E binding no longer opens the instrument after rebinding"));
    Tap(EKeys::U);Check(G->ApolloTool==2&&G->ApolloInstruments==0,TEXT("Rebound U input opens the real ALSEP tool without awarding progress"));G->CancelApolloTool();
    Check(Settings->Rebind(TEXT("Interact"),EKeys::E),TEXT("Default work binding restores after the input regression check"));Settings->Apply(P,false);
    Schedule();return;
   }
   if(G->Stage==ELunarStage::ApolloDeploy&&G->ApolloAction==1&&!TestedLegacy){LegacyCheck();Schedule();return;}
   const int Expected=G->Stage==ELunarStage::ApolloDeploy?(G->ApolloAction==0?2:3):0;
   Tap(ULunarSettings::Get()->Key(TEXT("Interact")));
   if(Expected)Check(G->ApolloTool==Expected&&!P->ActivePanel,TEXT("Standard-extension E input reaches the native ALSEP handler instead of an optional support panel"));
   Schedule();return;
  }
  Walk(true);
  if(P->GetVelocity().Size2D()<15&&Now-StepStart>3&&FMath::Fmod(Seconds,3)<Delta)Tap(ULunarSettings::Get()->Key(TEXT("Jump")));
  Schedule();
 }
};
}
void Start(ALunarGameMode* Game){if(Game){auto Run=MakeShared<FRun>(Game);Run->Begin();}}
}

