#include "LunarSeleneProps.h"
#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarOperationsPanel.h"
#include "LunarFlightHUD.h"
#include "LunarSurfaceVehicle.h"
#include "LunarDeparture.h"
#include "LunarSettings.h"
#include "LunarLoading.h"
#include "LunarMissionIntro.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void ALunarCharacter::OpenFieldOperations(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));if(!G||!G->FieldOperations||ALunarSurfaceVehicle::IsRiding(G))return;
 auto* C=G->FieldOperations.Get();if(C->Preparing){C->OpenPanel(EFieldPanelView::Preflight);return;}
 if(auto* Panel=Cast<ULunarOperationsPanel>(ActivePanel)){ClosePanel();return;}
 C->OpenPanel(C->Briefed?EFieldPanelView::Resources:EFieldPanelView::Briefing);
}
void ALunarCharacter::ToggleFieldScanner(){if(auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));G&&G->FieldOperations)G->FieldOperations->ToggleScanner();}
void ALunarCharacter::ReleasePrimary(){if(auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));G&&G->FieldOperations)G->FieldOperations->EndScan();}
void ALunarGameMode::ValidateFieldOperationsChecks(){
 FString Report;int32 Failed=0;auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=P?Cast<APlayerController>(P->GetController()):nullptr;auto* C=FieldOperations.Get();
 const bool Safe=ULunarSettings::IsAutomation()&&!ApolloSaveEnabled()&&UFieldOperationsComponent::IsNewAutomation();
 Check(Safe,TEXT("Field tests use isolated automation with owner checkpoint writes disabled"));
 Check(C&&C->Enabled&&P&&PC&&!Menu&&Stage==ELunarStage::Briefing,TEXT("Selected surface mission initializes field operations without starting mission"));
 if(Safe&&C&&C->Enabled&&P&&PC&&!Menu&&Stage==ELunarStage::Briefing){
  C->ValidateChecks(Check);
  {auto* Settings=ULunarSettings::Get();const auto BindingsBefore=Settings->Bindings;const FKey WorkBefore=Settings->Key(TEXT("Interact"));Check(!Settings->Rebind(TEXT("Interact"),EKeys::B)&&!Settings->Rebind(TEXT("Interact"),EKeys::G)&&Settings->Key(TEXT("Interact"))==WorkBefore,TEXT("Fixed board and scanner keys cannot also be rebound to a gameplay action"));Settings->Bindings.Add(TEXT("Interact"),EKeys::B);Check(Settings->Key(TEXT("Interact"))!=EKeys::B,TEXT("Legacy board-key binding falls back to its default"));Settings->Bindings.Add(TEXT("Interact"),EKeys::G);Check(Settings->Key(TEXT("Interact"))!=EKeys::G,TEXT("Legacy scanner-key binding falls back to its default"));Settings->Bindings=BindingsBefore;}
  Check(LunarMissionIntro::Eligible(TEXT("Apollo12"),TEXT(""),true)&&LunarMissionIntro::Eligible(TEXT("MoonBase"),TEXT(""),true)&&!LunarMissionIntro::Eligible(TEXT("MarsBase"),TEXT(""),true)&&!LunarMissionIntro::Eligible(TEXT("Apollo12"),TEXT("ResumeApollo=1"),true)&&!LunarMissionIntro::Eligible(TEXT("Apollo12"),TEXT(""),false),TEXT("Moon intro applies to selected new Moon missions only; resumes menu and Mars never reuse lunar footage"));
  P->FastStride=false;P->UpdateStrideSpeed();const float ExpectedWalk=Mars?320.f:140.f;const auto* Movement=P->GetCharacterMovement();Check(FMath::IsNearlyEqual(Movement->MaxWalkSpeed,ExpectedWalk)&& (Mars||FMath::IsNearlyEqual(Movement->JumpZVelocity,140.f)),TEXT("Destination movement uses deliberate Moon walk and hop settings while Mars retains its own tuning"));
  if(!Mars){
   TGuardValue<ELunarStage> JumpStage(Stage,Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule);TGuardValue<bool> JumpBriefed(C->Briefed,true);
   auto* Move=P->GetCharacterMovement();const FTransform BeforeJump=P->GetActorTransform();const FVector VelocityBefore=Move->Velocity;const EMovementMode ModeBefore=Move->MovementMode;
   P->ConsumeMovementInputVector();P->SetActorLocation(BeforeJump.GetLocation()+FVector(0,0,600),false,nullptr,ETeleportType::TeleportPhysics);Move->SetMovementMode(MOVE_Falling);Move->Velocity=FVector(120,20,100);
   P->Forward(-1);P->Right(1);Check(P->GetPendingMovementInputVector().IsNearlyZero()&&Move->AirControl==0&&Move->FallingLateralFriction==0&&Move->BrakingDecelerationFalling==0,TEXT("Moon movement controls contribute no airborne steering or artificial horizontal braking"));
   Move->TickComponent(.05f,LEVELTICK_All,nullptr);Check(FMath::IsNearlyEqual(Move->Velocity.X,120.f,.05f)&&FMath::IsNearlyEqual(Move->Velocity.Y,20.f,.05f)&&Move->Velocity.Z<100&&Move->IsFalling(),TEXT("Actual airborne movement advances under lunar gravity while preserving takeoff horizontal momentum"));
   P->SetActorTransform(BeforeJump,false,nullptr,ETeleportType::TeleportPhysics);Move->SetMovementMode(ModeBefore);Move->Velocity=VelocityBefore;P->LastStepPosition=P->GetActorLocation();Move->SetMovementMode(MOVE_Walking);P->Forward(1);Check(!P->GetPendingMovementInputVector().IsNearlyZero(),TEXT("Ground contact restores ordinary directional movement input"));P->ConsumeMovementInputVector();Move->SetMovementMode(ModeBefore);
  }
  if(!Apollo&&!Mars){TArray<UTextRenderComponent*> BoardText;if(C->ResourceBoard)C->ResourceBoard->GetComponents(BoardText);Check(C->ResourceBoard&&C->ResourceBoard->ActorHasTag(TEXT("Lunar.ServiceDock.NoInstructionBoard"))&&BoardText.IsEmpty(),TEXT("SELENE retains its low service anchor without the standing FIELD OPERATIONS instruction board"));}
  ApolloConfirmNew=false;DepartureAuthorizedNewRun=false;const float Before=Oxygen;
  const auto StableTarget=PC->GetViewTarget();const FTransform StableCamera=P->Camera->GetComponentTransform(),StablePawn=P->GetActorTransform();const float StableFov=P->Camera->FieldOfView;
  StartMission();auto* Panel=Cast<ULunarOperationsPanel>(P->ActivePanel);
  Check(!ALunarDepartureScene::Active(this)&&PC->GetViewTarget()==StableTarget&&P->Camera->GetComponentTransform().Equals(StableCamera,.001f),TEXT("Pre-EVA opens without spawning a cabin or taking over the player's camera"));
  Check(C->Preparing&&Panel&&Panel->View==EFieldPanelView::Preflight&&Stage==ELunarStage::Briefing,TEXT("Start enters dedicated pre-EVA checks rather than committing mission"));
  if(Panel){
   Check(C->CanUsePanel(Panel,true)&&!Paused,TEXT("Owned visible preflight panel accepts checks while mission clock remains briefing"));
   Check(!C->PerformAction(TEXT("Launch")),TEXT("Hatch cannot open before all four suit checks"));
   Check(!C->PerformAction(TEXT("Check"),1),TEXT("Preflight cannot skip the suit seal check"));
   for(int32 I=0;I<4;++I){
    Check(C->PerformAction(TEXT("Check"),I),TEXT("Real owned-panel callback begins next timed training check"));
    Check(!C->PerformAction(TEXT("Check"),I),TEXT("Repeated callback cannot award an in-progress check twice"));
    C->Advance(.4f);Check((C->PreflightMask&(1<<I))==0,TEXT("Partial check time does not mark completion"));
    Paused=true;const float Remaining=C->ActionRemaining;C->Advance(5);Check(C->ActionRemaining==Remaining&&Oxygen==Before,TEXT("Pause freezes pending suit action and oxygen"));Paused=false;
    C->Advance(.7f);Check((C->PreflightMask&(1<<I))!=0&&C->PendingCheck==INDEX_NONE,TEXT("Elapsed authored check timer completes exactly one step"));
    if(I<3)Check(C->PreflightCursor==I+1&&!C->PerformAction(TEXT("NextCheck")),TEXT("A completed check unlocks the next button directly without a Next action"));
   }
   Check(C->Prepared&&C->PreflightMask==15,TEXT("All four checks unlock hatch"));
   P->ClosePanel();const float SceneBefore=DepartureScene?DepartureScene->PresentationTime():0;C->Preparing=true;
   if(DepartureScene)DepartureScene->AdvancePresentation(20);ALunarDepartureScene::Skip(this);
   Check(Stage==ELunarStage::Briefing&&(!DepartureScene||DepartureScene->PresentationTime()==SceneBefore),TEXT("Closing checks and skip cannot bypass a pending EVA gate"));
   const auto SavedStatus=ApolloSaveStatus;if(Apollo){ApolloSaveStatus=EApolloSaveStatus::Ready;DepartureAuthorizedNewRun=true;}
   P->StartMission();Check(!Apollo||DepartureAuthorizedNewRun,TEXT("Reopening prepared pre-EVA preserves existing new-run authorization in the presence of a checkpoint"));ApolloSaveStatus=SavedStatus;Panel=Cast<ULunarOperationsPanel>(P->ActivePanel);Check(Panel&&Panel->View==EFieldPanelView::Preflight,TEXT("Enter reopens pending training checks"));
   Check(C->PerformAction(TEXT("Launch")),TEXT("Completed owned panel launches departure"));
   if(DepartureScene)DepartureScene->AdvancePresentation(ALunarDepartureScene::Duration);else CommitMissionStart();
   Panel=Cast<ULunarOperationsPanel>(P->ActivePanel);
   Check(IsActive()&&C->Prepared&&!C->Preparing&&C->Briefed&&!Panel&&!Paused,TEXT("Hatch handover starts surface play without an automatic resource modal"));
   Check(!ALunarDepartureScene::Active(this)&&PC->GetViewTarget()==StableTarget&&P->Camera->GetComponentTransform().Equals(StableCamera,.001f)&&P->GetActorTransform().Equals(StablePawn,.001f)&&FMath::IsNearlyEqual(P->Camera->FieldOfView,StableFov),TEXT("Completed checks enter gameplay immediately with identical pawn camera and FOV"));
   Check(C->Difficulty==1&&C->UtilityCapacity==100&&C->DataCapacityMB==1.2f,TEXT("Direct handover locks the balanced authored budget"));
   const float Locked=Oxygen;const auto StartedStage=Stage;
   Check(C->OpenPanel(EFieldPanelView::Briefing),TEXT("The optional locked resource briefing remains readable"));
   Check(!C->PerformAction(TEXT("Difficulty"),0)&&!C->PerformAction(TEXT("ConfirmBriefing"))&&Oxygen==Locked,TEXT("Reading the locked profile cannot refill resources or change difficulty"));P->ClosePanel();
   Check(C->OpenPanel(EFieldPanelView::Preflight)&&!C->PerformAction(TEXT("Launch"))&&Stage==StartedStage,TEXT("Reopening completed EVA checks cannot relaunch or reset an active mission"));P->ClosePanel();
  }
  const auto Points=C->MapPoints();Check(Points.Num()>=3&&Points.Num()<=32,TEXT("Map contains bounded actual world support positions"));
  AActor* NativeTarget=Target();const float Clock=Oxygen;
  if(C->OpenPanel(EFieldPanelView::Resources)){
    Check(C->PerformAction(TEXT("Waypoint"),0)&&Target()==NativeTarget,TEXT("Yellow-star support waypoint does not replace primary mission target"));
    const auto SelectedPoints=C->MapPoints();int32 MarkedCount=0,MainIndex=INDEX_NONE;bool FirstMarked=false;FVector SupportPosition=FVector::ZeroVector;for(const auto& Point:SelectedPoints){if(Point.Current)++MarkedCount;if(Point.Index==0){FirstMarked=Point.Current;SupportPosition=Point.Position;}if(Point.Label==TEXT("Main objective"))MainIndex=Point.Index;}
    Check(FirstMarked&&MarkedCount==1&&Target()==NativeTarget,TEXT("Selected support point is visibly marked without changing native interaction target"));
    {TGuardValue<bool> PauseGuard(Paused,false);TGuardValue<TObjectPtr<ULunarPanel>> PanelGuard(P->ActivePanel,nullptr);const auto SupportHUD=ULunarFlightHUD::ReadState(this,P);Check(SupportHUD.HasNavigation&&SupportHUD.NavigationName.StartsWith(TEXT("SUPPORT / "))&&SupportHUD.NavigationWorld.Equals(SupportPosition+FVector(0,0,70),.01)&&Target()==NativeTarget,TEXT("HUD support marker presents the selected real position without changing native work"));}
    Check(MainIndex!=INDEX_NONE&&C->PerformAction(TEXT("Waypoint"),MainIndex)&&Target()==NativeTarget,TEXT("Main-objective map entry restores mission navigation selection"));
    {TGuardValue<bool> PauseGuard(Paused,false);TGuardValue<TObjectPtr<ULunarPanel>> PanelGuard(P->ActivePanel,nullptr);const auto MainHUD=ULunarFlightHUD::ReadState(this,P);Check(MainHUD.HasNavigation&&!MainHUD.NavigationName.StartsWith(TEXT("SUPPORT / "))&&Target()==NativeTarget,TEXT("Main-objective selection restores the native HUD marker"));}
   const float Charge=C->UtilityCharge,Time=C->ElapsedS;Tick(3);Check(Oxygen==Clock&&C->UtilityCharge==Charge&&C->ElapsedS==Time,TEXT("Paused resource board freezes mission and shared field clocks"));
   auto* Old=Cast<ULunarOperationsPanel>(P->ActivePanel);P->ClosePanel();Check(!C->CanUsePanel(Old,true),TEXT("Closed panel cannot authorize a retained callback"));
  }
  // Explicit professional-budget fixture for the existing full-buffer and
  // payload-cap cases below. This is not a player profile-selection claim;
  // normal direct handover and its locked balanced budget were checked above.
  C->Difficulty=2;C->UtilityCapacity=80;C->UtilityCharge=FMath::Min(C->UtilityCharge,80.f);C->DataCapacityMB=.8f;
  OxygenCapacity=C->BaseMissionCapacity*.8f;Oxygen=FMath::Min(Oxygen,OxygenCapacity);if(Apollo)ApolloTimed=true;++C->Revision;
  // Actual authored world targets and camera line of sight, not a model-only scan fixture.
  if(C->SamplePackages.Num()==3&&IsValid(C->SamplePackages[0].Get())&&IsValid(C->SamplePackages[1].Get())&&IsValid(C->SamplePackages[2].Get())){
   Stage=Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule;C->Prepared=C->Briefed=true;C->VehicleInspected=false;C->ScannerUnlocked=false;
   Check(!C->ToggleScanner(),TEXT("Magnifier remains locked before a real vehicle inspection"));
   if(C->Vehicle){P->SetActorLocation(C->Vehicle->GetActorLocation()+FVector(-280,0,120));FVector Eye;FRotator A;P->GetActorEyesViewPoint(Eye,A);PC->SetControlRotation((C->Vehicle->GetActorLocation()-Eye).Rotation());
    if(C->OpenPanel(EFieldPanelView::Vehicle)){Check(C->PerformAction(TEXT("InspectVehicle"))&&C->ScannerUnlocked,TEXT("Owned nearby rover inspection unlocks optical magnifier"));P->ClosePanel();}
   }
   UStaticMeshComponent* Terrain=nullptr;const FString TerrainName=Apollo?TEXT("SM_Apollo12Terrain"):Mars?TEXT("SM_MarsTerrain"):TEXT("SM_LunarTerrain");
   for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){auto* M=It->GetStaticMeshComponent();if(M&&M->GetStaticMesh()&&M->GetStaticMesh()->GetName()==TerrainName){Terrain=M;break;}}
   auto AimPackage=[&](int32 I)->bool{
    auto* A=C->SamplePackages[I].Get();FVector Focus=A->GetActorLocation();TArray<UStaticMeshComponent*> Parts;A->GetComponents(Parts);for(auto* M:Parts)if(M&&M->ComponentHasTag(TEXT("work.focus"))){Focus=M->GetComponentLocation();break;}
    if(!Terrain)return false;
    for(float SightDistance:{220.f,240.f,340.f})for(int32 J=0;J<8;++J){const float Angle=PI*J/4;const FVector Probe=A->GetActorLocation()+FVector(-FMath::Cos(Angle)*SightDistance,FMath::Sin(Angle)*SightDistance,0);FHitResult Ground;FCollisionQueryParams T;T.bTraceComplex=true;
     if(!Terrain->LineTraceComponent(Ground,Probe+FVector(0,0,2000),Probe-FVector(0,0,2000),T)||Ground.ImpactNormal.Z<.75)continue;
     const float Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),Radius=P->GetCapsuleComponent()->GetScaledCapsuleRadius();const FVector Standing=Ground.ImpactPoint+FVector(0,0,(Half-Radius)+(Radius+2)/FMath::Max(.75f,Ground.ImpactNormal.Z));FCollisionQueryParams Q;Q.AddIgnoredActor(P);Q.AddIgnoredActor(Terrain->GetOwner());for(const auto& Support:C->SupportActors)if(Support)Q.AddIgnoredActor(Support.Get());
     if(GetWorld()->OverlapBlockingTestByChannel(Standing,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(P->GetCapsuleComponent()->GetScaledCapsuleRadius(),P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),Q))continue;
     P->SetActorLocation(Standing);const FRotator Aim=(Focus-P->Camera->GetComponentLocation()).Rotation();PC->SetControlRotation(Aim);P->Camera->SetWorldRotation(Aim);
     if(C->NearbyPackage(false)==I)return true;
    }return false;
   };
   C->UtilityCharge=20;C->DataStoredMB=0;Check(AimPackage(0),TEXT("First cartridge has a clear grounded observation approach on actual terrain"));
   Check(C->ToggleScanner()&&C->ScannerActive&&FMath::IsNearlyEqual(P->Camera->FieldOfView,35.f),TEXT("Unlocked magnifier sets real optical view on an authored cartridge"));
   const float ScanCharge=C->UtilityCharge;Check(C->BeginScan(),TEXT("Observed package begins real held inspection"));C->AdvanceScanner(.5f);C->EndScan();Check(C->ScannedMask==0&&C->UtilityCharge==ScanCharge,TEXT("Releasing incomplete inspection awards no record and spends no scan resource"));
   C->UtilityCharge=.79f;Check(!C->BeginScan()&&C->ScannedMask==0,TEXT("Insufficient shared battery rejects inspection atomically"));C->UtilityCharge=ScanCharge;
   C->DataStoredMB=C->DataCapacityMB;Check(!C->BeginScan()&&C->UtilityCharge==ScanCharge,TEXT("Full local storage rejects inspection without charge loss"));C->DataStoredMB=0;
   {const FVector Eye=P->Camera->GetComponentLocation(),Goal=C->SamplePackages[0]->GetActorLocation();auto* Wall=GetWorld()->SpawnActor<AActor>();auto* Box=Wall?NewObject<UBoxComponent>(Wall):nullptr;
    if(Box){Wall->SetRootComponent(Box);Box->SetMobility(EComponentMobility::Movable);Box->SetBoxExtent(FVector(12,60,90));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Wall->SetActorLocation((Eye+Goal)*.5f);}Check(Box&&!C->BeginScan()&&C->ScannedMask==0&&C->UtilityCharge==ScanCharge,TEXT("A real opaque world blocker prevents optical inspection without resource loss"));if(Wall)Wall->Destroy();}
   Check(C->BeginScan(),TEXT("Inspection can retry after resource constraints clear"));C->AdvanceScanner(1.3f);
   Check((C->ScannedMask&1)&&FMath::IsNearlyEqual(C->UtilityCharge,ScanCharge-.8f)&&FMath::IsNearlyEqual(C->DataStoredMB,.4f),TEXT("Completed observed hold records source label and exact shared scan costs once"));
   const uint8 HistoricSamples=SamplesCollected,HistoricParts=ApolloParts;Check(C->TryInteract()&&(C->CollectedMask&1)&&C->CargoMassKG==.5f,TEXT("Work key packs a scanned training cartridge into bounded real inventory"));
   Check(C->SamplePackages[0]->IsHidden()&&SamplesCollected==HistoricSamples&&ApolloParts==HistoricParts,TEXT("Packing hides only cartridge and never awards historical samples or hardware"));
   Check(AimPackage(2),TEXT("Third cartridge has a clear grounded observation approach on actual terrain"));Check(AimPackage(1),TEXT("Second cartridge has a clear grounded observation approach on actual terrain"));const bool SecondScan=C->BeginScan();Check(SecondScan,*FString::Printf(TEXT("Second cartridge can use the exact remaining 0.4 MB buffer / %s"),*C->Feedback));C->AdvanceScanner(1.3f);const int32 PackedBefore=C->CollectedMask;
   Check((C->ScannedMask&2)&&FMath::IsNearlyEqual(C->DataStoredMB,C->DataCapacityMB,1.e-6f)&&C->TryInteract()&&C->CollectedMask==PackedBefore&&C->CargoMassKG==C->MaxCargoKg(),TEXT("Professional payload cap prevents a second pickup while preserving its inspected label"));
   C->CancelScanner();Check(!C->ScannerActive&&FMath::IsNearlyEqual(P->Camera->FieldOfView,ULunarSettings::Get()->FieldOfView),TEXT("Stowing magnifier restores configured field of view"));
   if(IsValid(HomeActor)){P->SetActorLocation(HomeActor->GetActorLocation()+FVector(-100,0,90));if(C->OpenPanel(EFieldPanelView::Inventory)){Check(C->PerformAction(TEXT("Deposit"))&&C->DeliveredMask==C->CollectedMask&&C->CargoMassKG==0,TEXT("Base deposit clears carried training mass and retains delivery history"));P->ClosePanel();}}
  }else Check(false,TEXT("All three authored source-labelled cartridge targets exist on selected mission terrain"));
  if(IsValid(C->ResourceBoard.Get())){
   // Only the link fixture is seeded; actions, timers, native clocks and shared costs are real.
   const uint8 BeforeInstruments=ApolloInstruments;const bool BeforeReboot=StationRebootComplete;ApolloInstruments|=2;StationRebootComplete=true;
   P->SetActorLocation(C->ResourceBoard->GetActorLocation()+FVector(-100,0,90));C->UtilityCharge=20;const float BeforeCharge=C->UtilityCharge,BeforeOxygen=Oxygen;
   if(C->OpenPanel(EFieldPanelView::Resources)){
    Check(C->PerformAction(TEXT("Recharge"))&&!Paused&&!P->ActivePanel&&C->PendingCheck==4,TEXT("Powered nearby board begins actual recharge after closing its paused panel"));Tick(1);const float HeldAction=C->ActionRemaining,HeldCharge=C->UtilityCharge,HeldOxygen=Oxygen;Paused=true;Tick(10);Check(C->ActionRemaining==HeldAction&&C->UtilityCharge==HeldCharge&&Oxygen==HeldOxygen,TEXT("Pause freezes an in-flight recharge and native oxygen together"));Paused=false;Tick(2.01f);
    Check(C->PendingCheck==INDEX_NONE&&FMath::IsNearlyEqual(C->UtilityCharge,BeforeCharge+20-3.01f*C->IdleChargePerSecond,.001f)&&FMath::IsNearlyEqual(Oxygen,BeforeOxygen-3.01f,.002f),TEXT("Recharge adds 20 RU only after simulated mission time and native oxygen are spent"));
   }
   const float StoredBefore=C->DataStoredMB,SentBefore=C->DataTransmittedMB,TransmitCharge=C->UtilityCharge;
   if(C->OpenPanel(EFieldPanelView::Resources)){
    Check(C->PerformAction(TEXT("Transmit"))&&C->PendingCheck==5&&FMath::IsNearlyEqual(C->UtilityCharge,TransmitCharge-1,.001f),TEXT("Powered board downlink reserves its documented shared utility cost"));Tick(2.01f);Check(C->PendingCheck==INDEX_NONE&&C->DataStoredMB==0&&FMath::IsNearlyEqual(C->DataTransmittedMB,SentBefore+StoredBefore,.0001f),TEXT("Completed real downlink frees local storage and records the transmitted amount"));
   }
   ApolloInstruments=BeforeInstruments;StationRebootComplete=BeforeReboot;C->NativeScienceMask=Apollo?((ApolloPhotos?1:0)|((ApolloInstruments&1)?2:0)|((ApolloInstruments&2)?4:0)):(StationRebootComplete?1:0);
  }
  if(!Apollo&&IsValid(PowerActor)){
   Stage=ELunarStage::RestorePower;RepairStep=1;PowerSequenceActive=false;P->SetActorLocation(PowerActor->GetActorLocation()+FVector(-140,0,45));FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);PC->SetControlRotation((InteractionTarget()->GetActorLocation()-Eye).Rotation());
   Check(CanInteract(),TEXT("Rescue cable test begins within real native reach facing the power port"));
   Interact();Panel=Cast<ULunarOperationsPanel>(P->ActivePanel);Check(Panel&&Panel->View==EFieldPanelView::Wire&&RepairStep==1&&Paused,TEXT("Cable work opens matching panel without premature native progress"));
   if(Panel){
    Check(C->PerformAction(TEXT("WireIsolate")),TEXT("Paused owned wire UI validates physical reach and isolates toy circuit"));
    for(int32 I=0;I<C->WirePairs();++I){int32 Match=INDEX_NONE;for(int32 J=0;J<C->WirePairs();++J)if(C->WireLeftLabel(I)==C->WireRightLabel(J))Match=J;Check(Match!=INDEX_NONE&&C->PerformAction(TEXT("WireLeft"),I)&&C->PerformAction(TEXT("WireRight"),Match),TEXT("Matching text identifiers connect through real UI action authority"));}
    Check(C->WireComplete&&C->WirePendingCommit&&RepairStep==1&&Paused&&P->ActivePanel,TEXT("Matching every cable keeps the power sequence isolated until explicit verification"));
    Check(C->PerformAction(TEXT("WireVerify"))&&RepairStep==2&&!C->WirePendingCommit&&!Paused&&!P->ActivePanel&&!C->PerformAction(TEXT("WireVerify")),TEXT("Verify commits the cable exactly once and immediately closes its panel with confirmation"));
   }
  }
  if(P->ActivePanel)P->ClosePanel();
  LunarSeleneProps::ValidateChecks(this,Check);
  ALunarSurfaceVehicle::ValidateChecks(this,Check);
  if(Apollo){
   Stage=ELunarStage::ApolloCheckout;ApolloStarted=true;ApolloRunId=FGuid::NewGuid();
   const auto State=C->ExportState();auto* Save=CaptureApolloCheckpoint();Check(Save&&!State.IsEmpty()&&Save->FieldOperationsState==State,TEXT("Native Apollo checkpoint carries validated additive training state"));
   if(Save){TArray<uint8> Bytes;const bool Serialized=UGameplayStatics::SaveGameToMemory(Save,Bytes);auto* Loaded=Serialized?Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Bytes)):nullptr;Check(Loaded&&Loaded->FieldOperationsState==State&&Loaded->FormatVersion==1,TEXT("Actual SaveGame memory roundtrip retains additive state without version migration"));}
  }
  C->ResetNewRun();Check(C->PreflightMask==0&&!C->Briefed&&!C->VehicleInspected&&C->ScannedMask==0&&C->CollectedMask==0&&C->DeliveredMask==0&&C->VehicleDistanceM==0&&C->WalkingDistanceM==0,TEXT("Fresh run clears checks inventory scanner decisions and telemetry"));
 }
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);const FString Path=ReportPath(TEXT("field-operations-checks.txt"));
 if(!FFileHelper::SaveStringToFile(Report,*Path)){++Failed;UE_LOG(LogTemp,Error,TEXT("FIELD_OPERATIONS_CHECKS report write failed"));}UE_LOG(LogTemp,Display,TEXT("FIELD_OPERATIONS_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}
