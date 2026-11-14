#include "Misc/PackageName.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarPauseMenu.h"
#include "LunarMissionCard.h"
#include "LunarFlightHUD.h"
#include "LunarIntro.h"
#include "LunarLoading.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "TimerManager.h"
#include <limits>

namespace {
struct FNasaUIResult {
 FString Report;int32 Failed=0;
 void Check(bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;}
};
using FUIResult=TSharedRef<FNasaUIResult>;
void FinishUI(ALunarGameMode* G,FUIResult Result,const TCHAR* Name){
 Result->Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Result->Failed);FFileHelper::SaveStringToFile(Result->Report,*G->ReportPath(Name));UE_LOG(LogTemp,Display,TEXT("NASA_UI_RESULT %s\n%s"),Name,*Result->Report);FPlatformMisc::RequestExitWithStatus(false,Result->Failed?1:0);
}
FString CheckpointSnapshot(){
 const FString Folder=FPaths::ProjectSavedDir()/TEXT("SaveGames/Apollo12");TArray<FString> Files;IFileManager::Get().FindFilesRecursive(Files,*Folder,TEXT("*"),true,false,false);Files.Sort();FString Result;
 for(const auto& File:Files)Result+=File+TEXT(" ")+LexToString(FMD5Hash::HashFile(*File))+TEXT("\n");return Result;
}
void AimUI(ALunarGameMode* G,ALunarCharacter* P,AActor* Target){
 if(!Target)return;P->SetActorLocation(Target->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator View;P->GetActorEyesViewPoint(Eye,View);if(auto* PC=UGameplayStatics::GetPlayerController(G,0))PC->SetControlRotation((Target->GetActorLocation()-Eye).Rotation());
}
}

void ALunarGameMode::NasaIntroChecks(){
 auto Result=MakeShared<FNasaUIResult>();auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* Main=MainMenu.Get();
 Result->Check(Menu&&P&&Main&&ULunarSettings::IsAutomation(),TEXT("Intro test has an isolated home menu"));if(!Menu||!P||!Main||!ULunarSettings::IsAutomation()){FinishUI(this,Result,TEXT("nasa-intro-checks.txt"));return;}
 FLunarIntroState Model;Model.Begin();Result->Check(Model.Active&&Model.Elapsed==0&&Model.ImageAlpha()==1&&Model.LayerAlpha()==1,TEXT("Intro begins with its fully visible illustration, avoiding a second black flash"));
 Model.Advance(-1);Model.Advance(std::numeric_limits<float>::quiet_NaN());Model.Advance(std::numeric_limits<float>::infinity());Result->Check(Model.Elapsed==0&&Model.Active,TEXT("Invalid intro deltas cannot advance or poison the presentation"));
 Model.Advance(FLunarIntroState::FadeIn);Result->Check(FMath::IsNearlyEqual(Model.ImageAlpha(),1.f)&&!Model.RevealingMenu(),TEXT("Illustration remains visible before the bounded menu reveal"));
 Model.Advance(FLunarIntroState::Hold);Result->Check(Model.RevealingMenu()&&FMath::IsNearlyEqual(Model.LayerAlpha(),1.f),TEXT("Menu reveal begins only after the illustration hold"));
 Model.Advance(FLunarIntroState::FadeOut);Result->Check(!Model.Active&&FMath::IsNearlyEqual(Model.Elapsed,FLunarIntroState::Duration)&&FMath::IsNearlyZero(Model.LayerAlpha(),.0001f),TEXT("Intro reaches its bounded end without leaving a blocking black layer"));
 Model.Advance(100);Result->Check(FMath::IsNearlyEqual(Model.Elapsed,FLunarIntroState::Duration),TEXT("Finished intro cannot accumulate more presentation time"));
 Main->BeginIntro(true);auto* Wordmark=Main->WidgetTree?Cast<UTextBlock>(Main->WidgetTree->FindWidget(TEXT("PublicStartupWordmark"))):nullptr;
 const bool LocalArt=Main->IntroImage&&Main->IntroImage->GetBrush().GetResourceObject()&&Main->IntroImage->GetBrush().GetResourceObject()->GetName()==TEXT("T_AstroKid")&&Main->IntroImage->GetRenderOpacity()==1;
 const bool PublicText=Wordmark&&Wordmark->GetParent()==Main->IntroLayer&&Wordmark->IsVisible()&&Wordmark->GetText().ToString()==TEXT("AstroKid");
 Result->Check(Main->IsIntroActive()&&Main->IntroLayer&&(LocalArt||PublicText),TEXT("Startup owns a visible original text wordmark or an explicitly supplied local illustration"));
 if(FParse::Param(FCommandLine::Get(),TEXT("PublicDistributionTest"))){
  Result->Check(PublicText&&!LocalArt&&!FPackageName::DoesPackageExist(TEXT("/Game/Lunar/Frontend/Startup20261114/T_AstroKid")),TEXT("Public package contains original startup text and no unconfirmed logo texture"));
  Result->Check(!FPackageName::DoesPackageExist(TEXT("/Game/Lunar/Frontend/Startup20261114/T_InventoryReference")),TEXT("Public package contains no unconfirmed inventory-reference texture"));
  Result->Check(IFileManager::Get().FileSize(*(FPaths::ProjectContentDir()/TEXT("Movies/MoonArrival.mp4")))<0,TEXT("Public package contains no unconfirmed optional movie"));
 }
 const auto SavedCheckpoint=ApolloCachedCheckpoint;ApolloCachedCheckpoint=NewObject<UApolloCheckpoint>(this);
 P->ChooseMoon();P->ChooseMars();P->StartMission();Main->ExploreMoon();Main->ExploreMars();Main->ConfirmSelection();Main->Continue();P->ContinueMission();Main->Settings();P->OpenSettings();
 Result->Check(Menu&&!Main->MissionSelection&&!LunarLoading::IsBusy()&&!P->ActivePanel,TEXT("Intro blocks destination, Enter, Continue and settings entry points"));ApolloCachedCheckpoint=SavedCheckpoint;
 if(Result->Failed){FinishUI(this,Result,TEXT("nasa-intro-checks.txt"));return;}
 FTimerHandle Timer;GetWorldTimerManager().SetTimer(Timer,FTimerDelegate::CreateWeakLambda(this,[this,Result](){
  auto* Actual=MainMenu.Get();Result->Check(Actual&&!Actual->IsIntroActive()&&Actual->HomePage&&Actual->HomePage->IsVisible()&&Actual->IntroLayer&&Actual->IntroLayer->GetVisibility()==ESlateVisibility::Collapsed,TEXT("Real menu tick automatically completes startup and reveals the home page"));
  Result->Check(Actual&&Actual->HomeCards.Num()==3&&Actual->ExtraHomeCards.Num()==2,TEXT("All five home actions use the shared glass-card presentation"));
  auto* Clone=CreateWidget<ULunarMainMenu>(UGameplayStatics::GetPlayerController(this,0));if(Clone)Clone->TakeWidget();
  Result->Check(Clone&&!Clone->IsIntroActive(),TEXT("Rebuilding a menu in the same process does not replay the launch intro"));
  FinishUI(this,Result,TEXT("nasa-intro-checks.txt"));
 }),2.8f,false);
}

void ALunarGameMode::NasaUIChecks(){
 auto Result=MakeShared<FNasaUIResult>();auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 Result->Check(P&&!Menu&&ULunarSettings::IsAutomation()&&(!Apollo||!ApolloSaveEnabled()),TEXT("UI tests run in an isolated mission without normal checkpoint writes"));
 if(!P||Menu||!ULunarSettings::IsAutomation()||(Apollo&&ApolloSaveEnabled())){FinishUI(this,Result,TEXT("nasa-ui-checks.txt"));return;}
 const FString SaveBefore=CheckpointSnapshot();if(Apollo)StartApollo(true);else StartMission();BuildMissionInterface();
 Result->Check(FlightHUD&&FlightHUD->IsInViewport(),TEXT("The active mission owns the new flight interface"));
 auto State=ULunarFlightHUD::ReadState(this,P);Result->Check(State.Visible&&!State.Objective.IsEmpty()&&!State.ResourceTime.IsEmpty()&&!State.WorkKey.IsEmpty(),TEXT("Flight HUD reads current objective, resource clock and configured work key"));
 if(Apollo){
  Result->Check(!State.BoundedResource&&!State.Holding&&State.HoldFraction==0,TEXT("Untimed Apollo does not claim a suit deadline or invent a hold action"));
  Stage=ELunarStage::ApolloSamples;SamplesCollected=5;State=ULunarFlightHUD::ReadState(this,P);Result->Check(State.Completed==2&&State.Total==3,TEXT("Sample counter reflects only actually collected mask entries"));
  Stage=ELunarStage::ApolloRecover;ApolloParts=3;State=ULunarFlightHUD::ReadState(this,P);Result->Check(State.Completed==2&&State.Total==3,TEXT("Hardware counter uses the actual three-part manifest"));
  Stage=ELunarStage::ApolloCheckout;SamplesCollected=ApolloParts=0;
 }else{
  Stage=ELunarStage::ReturnHome;Result->Check(ULunarFlightHUD::ReadState(this,P).Stage.Contains(Mars?TEXT("05 / 05"):TEXT("07 / 07")),TEXT("Rescue return uses the actual destination mission stage count"));
  if(!Mars){Stage=ELunarStage::SurveySamples;Result->Check(ULunarFlightHUD::ReadState(this,P).Stage.Contains(TEXT("04 / 07")),TEXT("SELENE sampling follows data recovery as stage four of seven"));Stage=ELunarStage::ActivateBeacon;const int32 OldLocks=BeaconLocks;BeaconLocks=2;const auto BeaconState=ULunarFlightHUD::ReadState(this,P);Result->Check(BeaconState.Stage.Contains(TEXT("06 / 07"))&&BeaconState.Completed==2&&BeaconState.Total==3,TEXT("SELENE beacon reports its actual sixth stage and locked channels"));BeaconLocks=OldLocks;}
  Stage=ELunarStage::CollectData;StationRebootComplete=true;AimUI(this,P,DataActor);P->Interact();AdvanceInteraction(.9f);
  State=ULunarFlightHUD::ReadState(this,P);Result->Check(Holding&&State.Holding&&FMath::IsNearlyEqual(State.HoldFraction,HoldProgress/HoldDuration(),.001f),TEXT("Actual terminal hold drives the HUD ring from the gameplay clock"));
  Result->Check(State.HoldTitle.Contains(TEXT("SCIENCE"))&&State.NavigationName==TEXT("SCIENCE TERMINAL")&&!State.ShowGuide,TEXT("Holding presents the terminal action without duplicating the guide"));
  HoldProgress=-1;Result->Check(ULunarFlightHUD::HoldFraction(this)==0,TEXT("Negative hold progress renders an empty ring"));HoldProgress=99;Result->Check(ULunarFlightHUD::HoldFraction(this)==1,TEXT("Excess hold progress is bounded to a full ring"));
  HoldProgress=std::numeric_limits<float>::quiet_NaN();Result->Check(ULunarFlightHUD::HoldFraction(this)==0,TEXT("Non-finite hold progress cannot corrupt the ring"));HoldProgress=.9f;
  ReleaseInteract();Result->Check(ULunarFlightHUD::HoldFraction(this)==0&&!ULunarFlightHUD::ReadState(this,P).Holding,TEXT("Released work clears both gameplay hold and HUD progress"));
 }
 if(Apollo)AimUI(this,P,ModuleActor);
 auto* HintSettings=ULunarSettings::Get();const bool OldHints=HintSettings->TutorialHints,OldGuide=ApolloGuideEnabled;HintSettings->TutorialHints=false;ApolloGuideEnabled=false;
 const auto Minimal=ULunarFlightHUD::ReadState(this,P);Result->Check(CanInteract()&&!Minimal.ShowGuide&&!Minimal.InteractionPrompt.IsEmpty(),TEXT("Removing the field-guide card retains an actionable prompt beside the primary objective"));
 HintSettings->TutorialHints=true;ApolloGuideEnabled=true;const auto NoCard=ULunarFlightHUD::ReadState(this,P);
 Result->Check(!NoCard.ShowGuide&&!NoCard.InteractionPrompt.IsEmpty()&&!NoCard.Objective.IsEmpty()&&!NoCard.ResourceTime.IsEmpty(),TEXT("Tutorial hints do not restore the redundant field-guide card or hide mission resources"));
 HintSettings->TutorialHints=OldHints;ApolloGuideEnabled=OldGuide;
 auto* Settings=ULunarSettings::Get();const auto OldBindings=Settings->Bindings;Settings->Bindings.Add(TEXT("Interact"),EKeys::U);Result->Check(ULunarFlightHUD::ReadState(this,P).WorkKey==EKeys::U.GetDisplayName().ToString(),TEXT("Hold and work labels reflect a rebound work key"));Settings->Bindings=OldBindings;
 const float OldOxygen=Oxygen;Oxygen=-2;Result->Check(ULunarFlightHUD::ReadState(this,P).ResourceFraction==0,TEXT("Negative resource is clamped in the HUD"));Oxygen=std::numeric_limits<float>::quiet_NaN();Result->Check(ULunarFlightHUD::ReadState(this,P).ResourceFraction==0,TEXT("Non-finite resource is displayed safely"));Oxygen=OldOxygen;
 P->PauseMission();P->SyncPauseMenu();auto* Pause=P->PauseMenu.Get();
 Result->Check(Paused&&Pause&&Pause->IsInViewport()&&Pause->IsVisible()&&Pause->ActionCards.Num()==5,TEXT("Pause opens five explicit mission actions"));
 if(!Paused||!Pause){FinishUI(this,Result,TEXT("nasa-ui-checks.txt"));return;}
 bool ButtonsReady=true;for(const auto& Card:Pause->ActionCards)ButtonsReady&=Card&&Card->Button;Result->Check(ButtonsReady,TEXT("Each pause action has a live glass-card button"));
 Pause->NotifyOpened();Pause->AdvancePresentation(.12f);Result->Check(Pause->EntryBlend()>0&&Pause->EntryBlend()<1,TEXT("Pause interface fades in while gameplay is held"));Pause->AdvancePresentation(10);const float FullBlend=Pause->EntryBlend();Pause->AdvancePresentation(-1);Pause->AdvancePresentation(std::numeric_limits<float>::quiet_NaN());Result->Check(FullBlend==1&&Pause->EntryBlend()==1,TEXT("Pause entry reaches its bounded end and rejects invalid deltas"));
 if(!Apollo){PowerSequenceActive=true;PowerSequenceSeconds=.6f;}
 const float Clock=ApolloElapsed,Budget=Oxygen,Hold=HoldProgress,Power=PowerSequenceSeconds,Gait=P->GaitPhase;const FTransform Body=P->GetActorTransform();
 AdvanceTime(20);AdvanceInteraction(20);AdvancePowerSequence(20);AdvanceExpedition(20);if(Apollo)TickApollo(20);P->UpdateSuit(20);
 Result->Check(ApolloElapsed==Clock&&Oxygen==Budget&&HoldProgress==Hold&&PowerSequenceSeconds==Power&&P->GaitPhase==Gait,TEXT("Paused updates freeze mission resource, work, power sequence and gait"));
 FTimerHandle Timer;GetWorldTimerManager().SetTimer(Timer,FTimerDelegate::CreateWeakLambda(this,[this,P,Result,SaveBefore,Clock,Budget,Hold,Power,Gait,Body](){
  Result->Check(Paused&&ApolloElapsed==Clock&&Oxygen==Budget&&HoldProgress==Hold&&PowerSequenceSeconds==Power&&P->GaitPhase==Gait&&P->GetActorTransform().Equals(Body,.01f),TEXT("Real world ticks also leave the paused mission and astronaut unchanged"));PowerSequenceActive=false;
  P->PrimaryClick();Result->Check(Paused&&!LunarLoading::IsBusy(),TEXT("A background click cannot resume or activate a paused mission"));
  auto* Panel=P->PauseMenu.Get();if(!Panel){Result->Check(false,TEXT("Pause actions remain available"));FinishUI(this,Result,TEXT("nasa-ui-checks.txt"));return;}
  Panel->PreviewAction(1);P->StartMission();Result->Check(Paused&&P->ActivePanel&&P->ActivePanel->Settings,TEXT("Enter's mission action opens the selected Settings item instead of resuming"));
  if(P->ActivePanel)P->ClosePanel();P->SyncPauseMenu();Result->Check(Paused&&P->PauseMenu&&P->PauseMenu->IsInViewport()&&P->PauseMenu->IsVisible(),TEXT("Closing Settings returns to the paused menu"));
  Panel=P->PauseMenu.Get();if(Panel){Panel->Restart();Result->Check(Panel->IsConfirmingRestart()&&!LunarLoading::IsBusy(),TEXT("Restart first presents confirmation without changing worlds"));P->PauseMission();Result->Check(Paused&&!Panel->IsConfirmingRestart(),TEXT("Escape dismisses restart confirmation while keeping the mission paused"));}
  P->PauseMission();Result->Check(!Paused&&(!P->PauseMenu||P->PauseMenu->GetVisibility()==ESlateVisibility::Collapsed),TEXT("A second Escape resumes the mission and removes the pause widget"));
  const auto Before=Stage;const float Resource=Oxygen;P->RestartCurrentMission();Result->Check(!LunarLoading::IsBusy()&&Stage==Before&&Oxygen==Resource,TEXT("Full restart is guarded when the mission is not paused"));
  Result->Check(CheckpointSnapshot()==SaveBefore,TEXT("UI interaction tests do not mutate existing Apollo checkpoint files"));FinishUI(this,Result,TEXT("nasa-ui-checks.txt"));
 }),.45f,false);
}

void ALunarGameMode::NasaRestartChecks(){
 static int32 Phase=0;static FString MapBefore,SaveBefore;static FUIResult Result=MakeShared<FNasaUIResult>();
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));const FString Map=UGameplayStatics::GetCurrentLevelName(this,true);
 Result->Check(P&&!Menu&&ULunarSettings::IsAutomation()&&(!Apollo||!ApolloSaveEnabled()),TEXT("Full restart test has a mission and normal saves are disabled"));if(!P||Menu||!ULunarSettings::IsAutomation()||(Apollo&&ApolloSaveEnabled())){FinishUI(this,Result,TEXT("nasa-restart-checks.txt"));return;}
 if(Phase==0){
  MapBefore=Map;SaveBefore=CheckpointSnapshot();if(Apollo)StartApollo(true);else StartMission();Oxygen=93;FieldLessonAnswers={0,1,1};
  if(Apollo){ApolloElapsed=123;ApolloDistance=321;SamplesCollected=1;ApolloPhotos=3;ApolloParts=1;FApolloTrailPoint Point;Point.Position=P->GetActorLocation();Point.Elapsed=123;Point.BreakBefore=true;ApolloTrail.Add(Point);}
  else{Stage=ELunarStage::RestorePower;RepairStep=1;RecorderRecovered=true;SamplesCollected=1;}
  P->PauseMission();P->SyncPauseMenu();auto* Pause=P->PauseMenu.Get();Result->Check(Paused&&Pause,TEXT("Restart starts from the actual pause interface"));if(!Pause){FinishUI(this,Result,TEXT("nasa-restart-checks.txt"));return;}
  Pause->Restart();Result->Check(Pause->IsConfirmingRestart()&&!LunarLoading::IsBusy()&&Oxygen==93,TEXT("Restart confirmation preserves the current run until confirmed"));
  if(Result->Failed){FinishUI(this,Result,TEXT("nasa-restart-checks.txt"));return;}
  Phase=1;Pause->ConfirmRestart();Result->Check(LunarLoading::IsBusy(),TEXT("Confirmed restart queues the current map through the real loading path"));if(!LunarLoading::IsBusy())FinishUI(this,Result,TEXT("nasa-restart-checks.txt"));return;
 }
 Result->Check(Map==MapBefore&&LunarLoading::Completed(Map,false),TEXT("Restart returns to the same actual destination and completes loading"));
 Result->Check(Stage==ELunarStage::Briefing&&!Paused&&FMath::IsNearlyEqual(Oxygen,OxygenCapacity),TEXT("New world opens at briefing with a full mission resource"));
 Result->Check(SamplesCollected==0&&ApolloPhotos==0&&ApolloParts==0&&ApolloInstruments==0&&RepairStep==0&&!RecorderRecovered&&!Holding&&!PowerSequenceActive,TEXT("Restart clears required, optional and unfinished mission work"));
 Result->Check(ApolloElapsed==0&&ApolloDistance==0&&ApolloTrail.IsEmpty(),TEXT("Restart clears local timer, distance and route telemetry"));
 Result->Check(FieldLessonAnswers.Num()==3&&FieldLessonAnswers[0]==-1&&FieldLessonAnswers[1]==-1&&FieldLessonAnswers[2]==-1,TEXT("Restart resets all three local learning answers"));
 Result->Check(!P->PauseMenu||P->PauseMenu->GetVisibility()==ESlateVisibility::Collapsed,TEXT("Fresh briefing has no stale pause menu"));
 Result->Check(CheckpointSnapshot()==SaveBefore,TEXT("Full restart leaves existing saved Apollo checkpoints byte-identical"));FinishUI(this,Result,TEXT("nasa-restart-checks.txt"));
}
