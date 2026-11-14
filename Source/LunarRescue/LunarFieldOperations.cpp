#include "LunarFieldOperations.h"
#include "LunarGame.h"
#include "LunarOperationsPanel.h"
#include "LunarSurfaceVehicle.h"
#include "LunarDeparture.h"
#include "LunarCargoFinale.h"
#include "LunarSettings.h"
#include "LunarLoading.h"
#include "LunarWaterStudy.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace {
bool Near(const ALunarCharacter* P,const AActor* A,float Range=420){return P&&IsValid(A)&&FVector::Dist(P->GetActorLocation(),A->GetActorLocation())<=Range;}
bool Visible(const ALunarCharacter* P,const AActor* A,float Range){
 if(!Near(P,A,Range)||!P->GetWorld())return false;FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);
 if(FVector::DotProduct(Aim.Vector(),(A->GetActorLocation()-Eye).GetSafeNormal())<.35)return false;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(FieldSupportView),false);Q.AddIgnoredActor(P);Q.AddIgnoredActor(A);
 return !P->GetWorld()->LineTraceSingleByChannel(Hit,Eye,A->GetActorLocation(),ECC_Visibility,Q);
}
int32 Bits(int32 Mask){int32 Count=0;for(int32 I=0;I<3;++I)Count+=(Mask>>I)&1;return Count;}
const TCHAR* DifficultyName(int32 D){return D==0?TEXT("Beginner"):D==2?TEXT("Professional"):TEXT("Intermediate");}
}
UFieldOperationsComponent::UFieldOperationsComponent(){PrimaryComponentTick.bCanEverTick=false;}
ALunarGameMode* UFieldOperationsComponent::Game()const{return Cast<ALunarGameMode>(GetOwner());}
ALunarCharacter* UFieldOperationsComponent::Pilot()const{return Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));}
bool UFieldOperationsComponent::IsNewAutomation(){const TCHAR* C=FCommandLine::Get();return FParse::Param(C,TEXT("FieldOperationsTest"))||FParse::Param(C,TEXT("FieldOperationsReview"))||FParse::Param(C,TEXT("FieldOperationsProfile"))||FParse::Param(C,TEXT("ApolloInteractionRegressionTest"))||FParse::Param(C,TEXT("StrideRegressionTest"));}
void UFieldOperationsComponent::Initialize(ALunarGameMode* Owner){
 if(!Owner||Owner!=GetOwner())return;Enabled=!Owner->Menu&&(!ULunarSettings::IsAutomation()||IsNewAutomation())&&!FParse::Param(FCommandLine::Get(),TEXT("NoFieldOperations"));
 if(!Enabled)return;BuildWorld();
 if(FParse::Param(FCommandLine::Get(),TEXT("FieldOperationsProfile"))){PreflightMask=15;Prepared=Briefed=VehicleInspected=ScannerUnlocked=true;}
}
void UFieldOperationsComponent::ResetNewRun(bool PreservePrepared){
 if(auto* G=Game();G&&FMath::IsFinite(BaseMissionCapacity)&&BaseMissionCapacity>0)G->OxygenCapacity=BaseMissionCapacity;
 ALunarSurfaceVehicle::Abort(Game());CancelScanner();const bool WasReady=PreservePrepared&&Prepared;const bool WasEnabled=Enabled;
 Preparing=false;Prepared=WasReady;PreflightMask=WasReady?15:0;PreflightCursor=WasReady?3:0;Briefed=VehicleInspected=ScannerUnlocked=false;Difficulty=1;
 WireIsolated=WireComplete=WirePendingCommit=false;WireMask=WireMistakes=0;WireLeft=INDEX_NONE;
 CollectedMask=DeliveredMask=ScannedMask=NativeScienceMask=0;SelectedPackage=LastPackage=WaypointIndex=PendingCheck=INDEX_NONE;
 UtilityCapacity=100;UtilityCharge=WasReady?99.2f:100.f;CargoMassKG=DataStoredMB=DataTransmittedMB=ElapsedS=VehicleDistanceM=VehicleTimeS=WalkingDistanceM=0;DataCapacityMB=4.f;
 if(Vehicle&&HasVehicleHomePose)Vehicle->RestoreParkedPose(VehicleHomePose);
 BaseMissionCapacity=0;ActionRemaining=ScanProgress=PackagePreviewSeconds=0;Feedback.Reset();HasWalkPosition=false;Enabled=WasEnabled;++Revision;RefreshWorldVisibility();
}
bool UFieldOperationsComponent::PreflightRequired()const{auto* G=Game();return Enabled&&G&&!G->Menu&&G->Stage==ELunarStage::Briefing&&!Prepared;}
bool UFieldOperationsComponent::BeginPreflight(){
 auto* G=Game();if(!PreflightRequired()||G->Paused)return false;
 if(!Preparing){ResetNewRun(false);Preparing=true;BuildWorld();}
 OpenPanel(EFieldPanelView::Preflight);return true;
}
bool UFieldOperationsComponent::GateMissionStart(){if(!PreflightRequired())return false;BeginPreflight();return true;}
void UFieldOperationsComponent::OnMissionCommitted(){
 auto* G=Game();if(!Enabled||!G||!G->IsActive())return;Preparing=false;BaseMissionCapacity=G->OxygenCapacity;BuildWorld();
 if(FParse::Param(FCommandLine::Get(),TEXT("FieldOperationsProfile"))){PreflightMask=15;Prepared=Briefed=VehicleInspected=ScannerUnlocked=true;DataCapacityMB=1.2f;return;}
 // Start surface play directly after the hatch. The former mandatory budget
 // modal was the extra UI reported in the Nov 13 feedback. Keep the balanced
 // authored budget and the optional board available without a surprise pause.
 if(!Briefed){Difficulty=1;UtilityCapacity=100;DataCapacityMB=1.2f;Briefed=true;++Revision;}
}
void UFieldOperationsComponent::OnPanelClosed(){++Revision;}
bool UFieldOperationsComponent::CanOpenView(EFieldPanelView View)const{
 const auto* G=Game();if(!Enabled||!G||G->Menu||LunarLoading::IsBusy())return false;
 if(View==EFieldPanelView::Preflight)return Preparing||Prepared;
 if(G->Stage==ELunarStage::Briefing)return Preparing&&(View==EFieldPanelView::Resources||View==EFieldPanelView::Inventory);
 return !ALunarDepartureScene::Active(G)&&!ALunarCargoFinale::Active(G);
}
bool UFieldOperationsComponent::OpenPanel(EFieldPanelView View){
 auto* G=Game();auto* P=Pilot();auto* PC=P?Cast<APlayerController>(P->GetController()):nullptr;if(!P||!PC||!CanOpenView(View))return false;
 if(P->ActivePanel){if(auto* Existing=Cast<ULunarOperationsPanel>(P->ActivePanel)){Existing->Configure(View);return true;}return false;}
 CancelScanner();G->ReleaseInteract();G->CancelApolloTool();P->PanelWasPaused=G->Paused;
 if(G->IsActive()&&!G->Paused)P->PauseMission();
 auto* Panel=CreateWidget<ULunarOperationsPanel>(PC);if(!Panel)return false;Panel->View=View;P->ActivePanel=Panel;Panel->AddToViewport(80);Panel->Configure(View);
 FInputModeGameAndUI Input;Input.SetHideCursorDuringCapture(false);Input.SetWidgetToFocus(Panel->TakeWidget());PC->SetInputMode(Input);PC->bShowMouseCursor=true;Panel->SetKeyboardFocus();return true;
}
bool UFieldOperationsComponent::CanUsePanel(const ULunarOperationsPanel* Panel,bool Mutating)const{
 const auto* G=Game();const auto* P=Pilot();if(!Enabled||!G||!P||!Panel||P->ActivePanel!=Panel||!Panel->IsInViewport()||!Panel->IsVisible()||G->Menu||LunarLoading::IsBusy())return false;
 if(G->Stage==ELunarStage::Briefing&&Preparing)return !G->Paused&&(!Mutating||Panel->View==EFieldPanelView::Preflight);
 if(ALunarDepartureScene::Active(G)||ALunarCargoFinale::Active(G))return false;
 return Mutating?G->IsActive()&&G->Paused:(!G->IsActive()||G->Paused);
}
int32 UFieldOperationsComponent::PreflightStep()const{for(int32 I=0;I<4;++I)if(!(PreflightMask&(1<<I)))return I;return 4;}
float UFieldOperationsComponent::ActionProgress()const{return ActionDuration>0?FMath::Clamp(1-ActionRemaining/ActionDuration,0.f,1.f):0;}
bool UFieldOperationsComponent::BeginCheck(int32 Index){
 auto* G=Game();if(!Preparing||!G||G->Stage!=ELunarStage::Briefing||G->Paused||Index!=PreflightStep()||Index!=PreflightCursor||Index<0||Index>=4||PendingCheck!=INDEX_NONE)return false;
 PendingCheck=Index;ActionDuration=ActionRemaining=1.f;Feedback=TEXT("Running an authored training check. This does not measure a real suit.");++Revision;return true;
}
bool UFieldOperationsComponent::ContinuePreflight(){
 auto* G=Game();if(!Preparing||!G||G->Stage!=ELunarStage::Briefing||G->Paused||PendingCheck!=INDEX_NONE||PreflightCursor<0||PreflightCursor>=3||!(PreflightMask&(1<<PreflightCursor)))return false;
 ++PreflightCursor;Feedback=TEXT("Next system ready. Start its training check when you are ready.");++Revision;return true;
}
bool UFieldOperationsComponent::SpendCharge(float Cost){
 if(!FMath::IsFinite(Cost)||Cost<0||!FMath::IsFinite(UtilityCapacity)||UtilityCapacity<0||!FMath::IsFinite(UtilityCharge)||UtilityCharge<0||UtilityCharge>UtilityCapacity||UtilityCharge<Cost)return false;UtilityCharge=FMath::Clamp(UtilityCharge-Cost,0.f,UtilityCapacity);return true;
}
bool UFieldOperationsComponent::ReadyForVehicle()const{
 auto* G=Game();auto* P=Pilot();return Enabled&&Prepared&&Briefed&&VehicleInspected&&G&&G->IsActive()&&!G->Paused&&P&&!P->ActivePanel&&!G->Journal&&!ALunarDepartureScene::Active(G)&&!ALunarCargoFinale::Active(G);
}
bool UFieldOperationsComponent::Riding()const{return ALunarSurfaceVehicle::IsRiding(Game());}
void UFieldOperationsComponent::RecordVehicleTravel(float Meters,float Seconds){
 auto* G=Game();if(!Enabled||!G||!G->IsActive()||G->Paused||!FMath::IsFinite(Meters)||!FMath::IsFinite(Seconds)||Meters<0||Seconds<0||Meters>100||Seconds>2)return;
 const float Cost=Meters*DriveChargePerMeter;if(!FMath::IsFinite(Cost)||Cost<0)return;UtilityCharge=FMath::Clamp(UtilityCharge-Cost,0.f,UtilityCapacity);VehicleDistanceM+=Meters;if(Meters>.001f)VehicleTimeS+=Seconds;
}
void UFieldOperationsComponent::Advance(float Delta){
 auto* G=Game();auto* P=Pilot();if(!Enabled||!G||!P||!FMath::IsFinite(Delta)||Delta<=0)return;
 if(G->Paused||G->Journal||LunarLoading::IsBusy()){CancelScanner();RefreshWorldVisibility();return;}
 if(Preparing&&G->Stage==ELunarStage::Briefing){
  if(PendingCheck>=0&&PendingCheck<4&&ActionRemaining>0){ActionRemaining=FMath::Max(0.f,ActionRemaining-Delta);if(ActionRemaining==0){PreflightMask|=1<<PendingCheck;PendingCheck=INDEX_NONE;PreflightCursor=FMath::Min(PreflightStep(),3);SpendCharge(.2f);Prepared=PreflightMask==15;Feedback=Prepared?TEXT("Four training checks complete. Open the hatch to begin surface operations."):TEXT("Training check recorded. Continue with the next suit control.");++Revision;}}
  return;
 }
 if(!G->IsActive()){CancelScanner();return;}
 if(!Briefed){CancelScanner();HasWalkPosition=false;return;}
 if(Briefed){ElapsedS+=Delta;UtilityCharge=FMath::Max(0.f,UtilityCharge-IdleChargePerSecond*Delta);}
 if(!Riding()){
  const auto Here=P->GetActorLocation();if(HasWalkPosition&&P->GetCharacterMovement()->IsMovingOnGround()){const float M=float(FVector::Dist2D(Here,PreviousWalkPosition)/100);if(FMath::IsFinite(M)&&M<=20)WalkingDistanceM+=M;}PreviousWalkPosition=Here;HasWalkPosition=true;
 }else HasWalkPosition=false;
 PackagePreviewSeconds=FMath::Max(0.f,PackagePreviewSeconds-Delta);
 if(PendingCheck>=4&&ActionRemaining>0){
  if(!Near(P,ResourceBoard)||Riding()||P->ActivePanel){PendingCheck=INDEX_NONE;ActionRemaining=0;Feedback=TEXT("Operation cancelled: stay beside the resource board.");++Revision;}
  else {ActionRemaining=FMath::Max(0.f,ActionRemaining-Delta);if(ActionRemaining==0){if(PendingCheck==4){UtilityCharge=FMath::Min(UtilityCapacity,UtilityCharge+20);Feedback=TEXT("20 gameplay charge units transferred; elapsed time and oxygen continued normally.");}else{DataTransmittedMB+=DataStoredMB;DataStoredMB=0;Feedback=TEXT("Simulated study packets transmitted; local storage is available again.");}PendingCheck=INDEX_NONE;++Revision;}}
 }
 const int32 Native=G->Apollo?((G->ApolloPhotos?1:0)|((G->ApolloInstruments&1)?2:0)|((G->ApolloInstruments&2)?4:0)):(G->StationRebootComplete?1:0);
 if(Native!=NativeScienceMask){NativeScienceMask=Native;++Revision;}
 AdvanceScanner(Delta);RefreshWorldVisibility();
}
bool UFieldOperationsComponent::PerformAction(FName Action,int32 Argument){
 auto* G=Game();auto* P=Pilot();auto* Panel=P?Cast<ULunarOperationsPanel>(P->ActivePanel):nullptr;if(!CanUsePanel(Panel,Action!=TEXT("Waypoint")))return false;
 if(Action==TEXT("Check"))return Panel->View==EFieldPanelView::Preflight&&BeginCheck(Argument);
 if(Action==TEXT("UseWorkTool")||Action==TEXT("UseScanner")){
  if(Panel->View!=EFieldPanelView::Inventory||Preparing||!G->IsActive()||P->PanelWasPaused||Riding())return false;
  if(Action==TEXT("UseWorkTool")){if(!G->Apollo||!G->CanInteract())return false;P->ClosePanel();return G->OpenApolloTool();}
  if(!ScannerUnlocked)return false;P->ClosePanel();return ToggleScanner();
 }
 if(Action==TEXT("NextCheck"))return Panel->View==EFieldPanelView::Preflight&&ContinuePreflight();
 if(Action==TEXT("Launch")){
  if(Panel->View!=EFieldPanelView::Preflight||!Preparing||G->Stage!=ELunarStage::Briefing||!Prepared||PendingCheck!=INDEX_NONE)return false;Preparing=false;P->ClosePanel();G->CommitMissionStart();++Revision;return true;
 }
 if(Action==TEXT("Difficulty")){
  if(Panel->View!=EFieldPanelView::Briefing||Briefed||Argument<0||Argument>2)return false;Difficulty=Argument;Feedback=TEXT("Game difficulty selected. Review the resource budget, then confirm once.");++Revision;return true;
 }
 if(Action==TEXT("ConfirmBriefing")){
  if(Panel->View!=EFieldPanelView::Briefing||Briefed||!Prepared)return false;
  const float Multipliers[]={1.5f,1.f,.8f};const float Capacities[]={150.f,100.f,80.f};const float Storage[]={4.f,1.2f,.8f};
  const float Spent=FMath::Max(0.f,100-UtilityCharge);UtilityCapacity=Capacities[Difficulty];UtilityCharge=FMath::Max(0.f,UtilityCapacity-Spent);DataCapacityMB=Storage[Difficulty];
  BaseMissionCapacity=G->OxygenCapacity;G->OxygenCapacity=FMath::Min(1800.f,BaseMissionCapacity*Multipliers[Difficulty]);G->Oxygen=G->OxygenCapacity;if(G->Apollo)G->ApolloTimed=Difficulty!=0;
  Briefed=true;Feedback=TEXT("Resource budget locked for this attempt. Inspect the training rover to unlock the magnifier.");P->ClosePanel();G->Message(Feedback);++Revision;return true;
 }
 if(Action==TEXT("InspectVehicle")){
  if(Panel->View!=EFieldPanelView::Vehicle||!Prepared||!Briefed||!Vehicle||!Visible(P,Vehicle,550))return false;
  VehicleInspected=ScannerUnlocked=true;Feedback=TEXT("Training rover checks recorded. G toggles the optical magnifier; hold left click on a labelled training cartridge, then use E to collect it.");++Revision;return true;
 }
 if(Action==TEXT("Ride")){
  if(P->PanelWasPaused){Feedback=TEXT("Close the board and resume your mission before boarding the rover.");++Revision;return false;}
  if(!VehicleInspected||!Vehicle||!Visible(P,Vehicle,320)){Feedback=TEXT("Move within 3.2 m of the training rover before boarding.");++Revision;return false;}P->ClosePanel();return ALunarSurfaceVehicle::TryInteract(G);
 }
 if(Action==TEXT("WireIsolate")){
  if(Panel->View!=EFieldPanelView::Wire||G->Apollo||G->Stage!=ELunarStage::RestorePower||G->RepairStep!=1||!G->IsActive()||!Visible(P,G->InteractionTarget(),G->InteractionReach(G->InteractionTarget())))return false;
  WireIsolated=true;Feedback=TEXT("Toy circuit isolated. Match port identifiers; this is not a real electrical-repair procedure.");++Revision;return true;
 }
 if(Action==TEXT("WireLeft")){
  if(Panel->View!=EFieldPanelView::Wire||G->Apollo||G->Stage!=ELunarStage::RestorePower||G->RepairStep!=1||!Visible(P,G->InteractionTarget(),G->InteractionReach(G->InteractionTarget()))||!WireIsolated||Argument<0||Argument>=WirePairs()||(WireMask&(1<<Argument)))return false;WireLeft=Argument;++Revision;return true;
 }
 if(Action==TEXT("WireRight")){
  return Panel->View==EFieldPanelView::Wire&&!G->Apollo&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==1&&Visible(P,G->InteractionTarget(),G->InteractionReach(G->InteractionTarget()))&&ConnectWire(Argument);
 }
 if(Action==TEXT("WireVerify")){
  if(Panel->View!=EFieldPanelView::Wire||!WireComplete||!WirePendingCommit||G->Apollo||G->Stage!=ELunarStage::RestorePower||G->RepairStep!=1||P->PanelWasPaused||!Visible(P,G->InteractionTarget(),G->InteractionReach(G->InteractionTarget())))return false;
  P->ClosePanel();if(!G->CanInteract())return false;G->Interact();if(G->RepairStep!=2)return false;
  WirePendingCommit=false;Feedback=TEXT("Cable verified. Restart the station with the normal work control to restore power.");++Revision;P->ClosePanel();G->Message(Feedback);return true;
 }
 if(Action==TEXT("WireReset")){
  if(Panel->View!=EFieldPanelView::Wire||WireComplete&&!WirePendingCommit)return false;WireMask=0;WireLeft=INDEX_NONE;WireIsolated=WireComplete=WirePendingCommit=false;Feedback=TEXT("Uncommitted toy connections reset.");++Revision;return true;
 }
 if(Action==TEXT("Waypoint")){
  if(Panel->View!=EFieldPanelView::Resources&&Panel->View!=EFieldPanelView::Briefing||!Preparing&&!G->IsActive())return false;
  const auto Points=MapPoints();if(!Points.ContainsByPredicate([&](const auto& Point){return Point.Index==Argument;}))return false;WaypointIndex=Argument;Feedback=TEXT("Yellow-star waypoint selected. The primary mission target is unchanged.");++Revision;return true;
 }
  if(Action==TEXT("Deposit")){
   if(!Briefed){Feedback=TEXT("Confirm the resource briefing before delivering training cargo.");++Revision;return false;}
   if(!Near(P,G->HomeActor,600)){Feedback=TEXT("Return within 6 m of the lander to deposit carried training cartridges.");++Revision;return false;}
   if(CollectedMask==DeliveredMask){Feedback=TEXT("No carried training cartridges are waiting for delivery.");++Revision;return false;}
   DeliveredMask=CollectedMask;CargoMassKG=0;PackagePreviewSeconds=0;Feedback=TEXT("Simulated remote-delivery cartridges deposited. No historical Apollo sample or local water discovery is awarded.");++Revision;if(G->Apollo)G->SaveApolloCheckpoint();return true;
  }
 if(Action==TEXT("Recharge")||Action==TEXT("Transmit")){
  if(P->PanelWasPaused){Feedback=TEXT("Close the board and resume your mission before starting a timed field operation.");++Revision;return false;}
  const bool Link=G->Apollo?(G->ApolloInstruments&2)!=0:G->StationRebootComplete;
   if(!Briefed){Feedback=TEXT("Confirm the resource briefing before starting a timed field operation.");++Revision;return false;}
   if(!Link){Feedback=TEXT("Downlink and recharge are offline. Restore station power or deploy the ALSEP link first.");++Revision;return false;}
   if(!Near(P,ResourceBoard)){Feedback=TEXT("Move beside the resource board before starting recharge or downlink.");++Revision;return false;}
   if(Riding()){Feedback=TEXT("Park and exit the training rover before using the resource board.");++Revision;return false;}
   if(PendingCheck!=INDEX_NONE){Feedback=TEXT("A field operation is already pending. Finish or cancel it before starting another.");++Revision;return false;}
   if(!FMath::IsFinite(UtilityCharge)||!FMath::IsFinite(UtilityCapacity)){Feedback=TEXT("The utility ledger is unavailable. No timed operation was started.");++Revision;return false;}
   if(Action==TEXT("Recharge")&&UtilityCharge>=UtilityCapacity-.001f){Feedback=TEXT("Utility charge is already full. No recharge time is needed.");++Revision;return false;}
   if(Action==TEXT("Transmit")){
    if(!FMath::IsFinite(DataStoredMB)||DataStoredMB<=0){Feedback=TEXT("The local data buffer is empty or unavailable. No utility charge was spent.");++Revision;return false;}
    if(!SpendCharge(1.f)){Feedback=TEXT("Downlink needs 1 utility RU. Recharge at the resource board first.");++Revision;return false;}
   }
  PendingCheck=Action==TEXT("Recharge")?4:5;ActionDuration=ActionRemaining=Action==TEXT("Recharge")?3.f:2.f;P->ClosePanel();Feedback=TEXT("Stay at the board while the operation runs. Mission time and oxygen continue.");G->Message(Feedback);++Revision;return true;
 }
 return false;
}
bool UFieldOperationsComponent::TryInteract(){
 auto* G=Game();auto* P=Pilot();if(!Enabled||!G||!P||G->Paused||!G->IsActive()||P->ActivePanel)return false;
 if(Riding())return ALunarSurfaceVehicle::TryInteract(G);
 if(G->CanInteract()){CancelScanner();return false;}
 if(Visible(P,ResourceBoard,360))return OpenPanel(Briefed?EFieldPanelView::Resources:EFieldPanelView::Briefing);
 if(Vehicle&&Visible(P,Vehicle,320)){const bool WasInspected=VehicleInspected,WasUnlocked=ScannerUnlocked;VehicleInspected=ScannerUnlocked=true;const bool Handled=ALunarSurfaceVehicle::TryInteract(G);if(Riding()){++Revision;return true;}VehicleInspected=WasInspected;ScannerUnlocked=WasUnlocked;return Handled;}
 const int32 Package=NearbyPackage(true);if(Package!=INDEX_NONE){
  if(CollectedMask&(1<<Package))return false;if(CargoMassKG+TrainingPackageMassKG>MaxCargoKg()+.001f){Feedback=TEXT("Training payload capacity reached. Return to the lander and deposit existing cartridges first.");G->Message(Feedback);++Revision;return true;}CollectedMask|=1<<Package;CargoMassKG=Bits(CollectedMask&~DeliveredMask)*TrainingPackageMassKG;LastPackage=Package;PackagePreviewSeconds=2.2f;
  if(SamplePackages.IsValidIndex(Package))SamplePackages[Package]->SetActorHiddenInGame(true);Feedback=TEXT("SIMULATED cartridge packed. Its remote LCROSS label is educational metadata, not a water detection here.");G->Message(Feedback);++Revision;if(G->Apollo)G->SaveApolloCheckpoint();return true;
 }
 return false;
}
float UFieldOperationsComponent::MaxCargoKg()const{const float Limits[]={2.f,1.f,.5f};return Limits[FMath::Clamp(Difficulty,0,2)];}
FString UFieldOperationsComponent::Prompt()const{
 auto* G=Game();auto* P=Pilot();if(!Enabled||!G||!P||!G->IsActive()||G->Paused||P->ActivePanel)return FString();
 const FString Key=ULunarSettings::Get()->Key(TEXT("Interact")).GetDisplayName().ToString();if(Riding())return Key+TEXT("  Exit training rover");if(G->CanInteract())return FString();
 if(Visible(P,ResourceBoard,360))return Key+TEXT("  Open resource board");if(Vehicle&&Visible(P,Vehicle,500))return Key+(VehicleInspected?(Near(P,Vehicle,320)?TEXT("  Drive training rover"):TEXT("  Move closer to board (3.2 m)")):TEXT("  Inspect training rover"));
 const int32 Package=NearbyPackage(false);if(Package!=INDEX_NONE)return ScannedMask&(1<<Package)?Key+TEXT("  Pack simulated cartridge"):ScannerUnlocked?TEXT("G  Magnifier / hold left click to inspect cartridge"):TEXT("Inspect the training rover to unlock the magnifier");return FString();
}
void UFieldOperationsComponent::Shutdown(){CancelScanner();ALunarSurfaceVehicle::Abort(Game());Preparing=false;PendingCheck=INDEX_NONE;ActionRemaining=0;}
FString UFieldOperationsComponent::Title(EFieldPanelView V)const{
 if(Preparing&&V==EFieldPanelView::Resources)return TEXT("PRE-EVA / NAVIGATION");
 if(V==EFieldPanelView::Inventory)return TEXT("ASTRONAUT BACKPACK / INVENTORY");
 switch(V){case EFieldPanelView::Preflight:return TEXT("PRE-EVA / TRAINING CHECKS");case EFieldPanelView::Briefing:return TEXT("SURFACE MISSION / RESOURCE BRIEFING");case EFieldPanelView::Vehicle:return TEXT("TRAINING ROVER / INSPECTION");case EFieldPanelView::Wire:return TEXT("POWER INTERFACE / MATCH THE PORTS");case EFieldPanelView::Inventory:return TEXT("TRAINING CARGO / DELIVERY LOG");case EFieldPanelView::Sample:return TEXT("REMOTE WATER / SOURCE LABEL");default:return TEXT("FIELD OPERATIONS / RESOURCE BOARD");}
}
FString UFieldOperationsComponent::Instructions(EFieldPanelView V)const{
 if(V==EFieldPanelView::Preflight)return TEXT("Select each unlocked system check. Completion unlocks the next row. Finish all four to open the hatch.");
 if(Preparing&&V==EFieldPanelView::Resources)return TEXT("Choose a local waypoint or inspect the planet overview. Mission time has not started.");
 if(V==EFieldPanelView::Inventory)return TEXT("Inspect your equipment and collected items. Tools keep their normal mission requirements.");
 if(V==EFieldPanelView::Briefing)return TEXT("Choose a gameplay difficulty, inspect the forecast and confirm once. Budgets use authored tuning; Professional is a game label, not astronaut certification. Budget/finance is deferred.");
 if(V==EFieldPanelView::Wire&&Game()&&Game()->Apollo)return TEXT("Cable matching belongs to the SELENE and ARES rescue missions. Historical Apollo ALSEP deployment is separate.");
 if(V==EFieldPanelView::Wire)return TEXT("Isolate the toy circuit, select a left port, then the right port with the same identifier. Text and symbols accompany colour. Only the full match connects the mission cable; close/retry preserves committed work.");
 if(V==EFieldPanelView::Vehicle)return TEXT("Inspect utility power and controls to unlock the optical magnifier. W/S drive, A/D steer, E exits on safe ground. Driving saves travel time but consumes the same utility battery used by inspection. The vehicle is a fictional modern support exercise.");
 return TEXT("B opens the board. Yellow stars mark current world locations; selecting one highlights it and shows a SUPPORT navigation marker while primary mission work stays unchanged. Select Main objective to restore the normal mission marker. G toggles the magnifier; hold left click on a labelled cartridge, then E to pack it. Return to base to deposit.");
}
TArray<FString> UFieldOperationsComponent::ResourceLines()const{
 auto* G=Game();TArray<FString> R;if(!G)return R;
 R.Add(FString::Printf(TEXT("Difficulty / %s%s"),DifficultyName(Difficulty),Briefed?TEXT(" / locked"):TEXT(" / not confirmed")));
 R.Add(IsValid(G->HomeActor.Get())?TEXT("Lander / PRESENT in this mission / return and training-cargo deposit point"):TEXT("Lander / UNAVAILABLE / world actor missing"));
 if(!Briefed){const int32 D=FMath::Clamp(Difficulty,0,2);const float Charge[]={150,100,80},Storage[]={4,1.2f,.8f},Factor[]={1.5f,1,.8f};const float Base=BaseMissionCapacity>0?BaseMissionCapacity:G->OxygenCapacity;
  R.Add(FString::Printf(TEXT("PROPOSED BUDGET / %.0f RU / %.1f MB / %.1f kg / %.0f gameplay oxygen seconds%s. Applied only when confirmed."),Charge[D],Storage[D],MaxCargoKg(),FMath::Min(1800.f,Base*Factor[D]),G->Apollo&&D==0?TEXT(" / untimed exploration"):TEXT("")));}

 R.Add(FString::Printf(TEXT("Oxygen / %.0f of %.0f gameplay seconds%s"),G->Oxygen,G->OxygenCapacity,G->Apollo&&!G->ApolloTimed?TEXT(" / untimed exploration"):TEXT("")));
 R.Add(FString::Printf(TEXT("Mission elapsed / %.1f s / walking %.1f m / rover %.1f m (%.1f s)"),ElapsedS,WalkingDistanceM,VehicleDistanceM,VehicleTimeS));
 R.Add(FString::Printf(TEXT("Utility charge / %.1f of %.1f RU / idle %.3f RU/s / drive %.3f RU/m"),UtilityCharge,UtilityCapacity,IdleChargePerSecond,DriveChargePerMeter));
 R.Add(FString::Printf(TEXT("Training cargo / %.2f of %.2f kg (authored mass) / %d carried / %d delivered"),CargoMassKG,MaxCargoKg(),Bits(CollectedMask&~DeliveredMask),Bits(DeliveredMask)));
 R.Add(FString::Printf(TEXT("Simulated data / %.2f of %.2f MB local / %.2f MB transmitted"),DataStoredMB,DataCapacityMB,DataTransmittedMB));
 const bool Link=G->Apollo?(G->ApolloInstruments&2)!=0:G->StationRebootComplete;
 R.Add(Link?TEXT("Communications / mission power or ALSEP link ready; board downlink available"):TEXT("Communications / offline; restore mission power or deploy the ALSEP link"));
 R.Add(Prepared?TEXT("Suit / four authored checks recorded; no real pressure measurement"):TEXT("Suit / pre-EVA checks pending"));
 R.Add(Vehicle?Vehicle->InspectionText():TEXT("Training rover / unavailable; main mission remains playable on foot"));
 if(Vehicle){const float D=float(FVector::Dist2D(Pilot()?Pilot()->GetActorLocation():FVector::ZeroVector,G->Target()?G->Target()->GetActorLocation():FVector::ZeroVector)/100);auto* P=Pilot();const float Walk=P?P->GetCharacterMovement()->MaxWalkSpeed/100:3.f;R.Add(FString::Printf(TEXT("Direct target forecast / %.0f m / walk %.0f s / rover %.0f s + %.1f RU. Straight-line estimate ignores obstacles and grades."),D,D/FMath::Max(.1f,Walk),D/FMath::Max(.1f,FMath::Clamp(Vehicle->ForwardLimitCm,0.f,800.f)/100.f),D*DriveChargePerMeter));}
 return R;
}
TArray<FString> UFieldOperationsComponent::InventoryLines()const{
 TArray<FString> R;for(int32 I=0;I<3;++I)R.Add(FString::Printf(TEXT("TRAINING-%02d / %s / %.2f kg authored cartridge / remote Cabeus 2009 label"),I+1,DeliveredMask&(1<<I)?TEXT("DELIVERED"):CollectedMask&(1<<I)?TEXT("PACKED"):ScannedMask&(1<<I)?TEXT("INSPECTED"):TEXT("PENDING"),TrainingPackageMassKG));
 R.Add(TEXT("These are authored remote-delivery teaching props. No drilling, water extraction, actual water mass or historical Apollo cargo is generated."));return R;
}
FString UFieldOperationsComponent::SourcesText()const{return TEXT("MODERN TRAINING EXTENSION / Fictional rover deployment, utility costs, cartridge mass and storage sizes are gameplay tuning. Apollo 12 did not use a lunar rover; its historical tasks remain separate. The Mars SEV-inspired vehicle is a fictional deployment of a prototype concept. Remote-water labels use the bundled LCROSS publication record, not a local measurement.\n\n")+LunarWater::EvidenceText()+TEXT("\n\n")+LunarWater::ModelLimit()+TEXT("\nNASA EVA context: https://www.nasa.gov/missions/station/what-is-a-spacewalk/\nLRV history: https://www.nasa.gov/history/alsj/alsj-LRVdocs.html\nSEV concept: https://science.nasa.gov/3d-resources/space-exploration-vehicle/\nPlanet overview: NASA/LROC CGI Moon Kit and NASA/JPL-Caltech Viking Mars colour imagery; visual context only, not terrain elevations. Fictional NOMAD-7 uses NASA/JPL Perseverance visual reference: Perseverance is a Mars mission and did not visit the Moon.\nPerseverance model: https://science.nasa.gov/3d-resources/mars-2020-perseverance-rover/\nMoon vehicle model: Deutsches Museum | Digital, CC BY-SA 4.0.\nAstronaut: LasquetiSpice, CC BY 4.0; adapted gameplay rig.\nSee SCIENCE_AND_SOURCES and SourceArt credits for source limits, modifications and links.");}
FString UFieldOperationsComponent::Summary()const{return FString::Printf(TEXT("Modern training extension / %s / rover %.1f m and %.1f s / charge %.1f RU / %d of 3 simulated cartridges delivered / %d wire mistakes. Historical cargo is recorded separately."),DifficultyName(Difficulty),VehicleDistanceM,VehicleTimeS,UtilityCharge,Bits(DeliveredMask),WireMistakes);}
FString UFieldOperationsComponent::HudHint()const{
 if(!Enabled)return FString();if(Preparing)return TEXT("PRE-EVA / Enter: open pending training checks");if(!Briefed)return TEXT("RESOURCE BRIEFING / B: choose and lock a gameplay budget");
 if(PendingCheck>=4)return FString::Printf(TEXT("%s / %.1f s / stay at the resource board"),PendingCheck==4?TEXT("CHARGING"):TEXT("DOWNLINK"),ActionRemaining);
 if(ScannerActive){if(UtilityCharge<.8f)return TEXT("OPTICAL MAGNIFIER / LOW UTILITY / recharge at the resource board / G: stow");if(DataCapacityMB-DataStoredMB<.4f-1.e-6f)return TEXT("OPTICAL MAGNIFIER / DATA BUFFER FULL / downlink at the resource board / G: stow");return FString::Printf(TEXT("OPTICAL MAGNIFIER / hold left click: inspect / %.0f%% / G: stow / training label, not a water measurement"),ScanDuration>0?ScanProgress/ScanDuration*100:0);}
 if(Riding())return FString::Printf(TEXT("TRAINING ROVER / W S drive / A D steer / E exit / charge %.1f RU"),UtilityCharge);
 if(CargoMassKG>=MaxCargoKg()-.001f&&NearbyPackage(true)!=INDEX_NONE)return TEXT("PAYLOAD FULL / return to the lander and deposit training cartridges before packing another");
 return FString::Printf(TEXT("FIELD SUPPORT / B board / G magnifier%s / charge %.1f RU / simulated cargo %d/3"),ScannerUnlocked?TEXT(""):TEXT(" (inspect rover to unlock)"),UtilityCharge,Bits(CollectedMask));
}
TArray<FFieldMapPoint> UFieldOperationsComponent::MapPoints()const{
 TArray<FFieldMapPoint> R;auto* G=Game();if(!G)return R;auto Add=[&](const FString& Label,AActor* A,bool Done=false){if(!IsValid(A)||A->GetActorLocation().ContainsNaN()||R.Num()>=32)return;FFieldMapPoint M;M.Index=R.Num();M.Label=Label;M.Position=A->GetActorLocation();M.Completed=Done;M.Current=WaypointIndex>=0?WaypointIndex==M.Index:A==G->Target();R.Add(M);};
 Add(TEXT("Lander / deposit"),G->HomeActor);Add(TEXT("Resource board"),ResourceBoard);Add(TEXT("Training rover"),Vehicle);Add(TEXT("Main objective"),G->Target());
 if(G->PowerActor!=G->Target())Add(TEXT("Power station"),G->PowerActor,G->StationRebootComplete);
 if(G->DataActor!=G->Target())Add(TEXT("Science terminal"),G->DataActor);
 if(G->ModuleActor!=G->Target())Add(TEXT("Power module"),G->ModuleActor,G->RepairStep>0);
 if(G->RelayActor!=G->Target())Add(TEXT("Orbital relay"),G->RelayActor);
 for(int32 I=0;I<G->SampleActors.Num();++I)Add(FString::Printf(TEXT("Mission sample %d"),I+1),G->SampleActors[I],(G->SamplesCollected&(1<<I))!=0);
 for(int32 I=0;I<SamplePackages.Num();++I)Add(FString::Printf(TEXT("Training cartridge %d"),I+1),SamplePackages[I],(DeliveredMask&(1<<I))!=0);return R;
}
