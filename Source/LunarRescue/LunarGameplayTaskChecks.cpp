#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "LunarDeparture.h"
#include "LunarCarryPresentation.h"
#include "LunarCrew.h"
#include "LunarWaterStudy.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

namespace LunarGameplayCheckPrivate {
bool SameStudy(const LunarWater::FState& A,const LunarWater::FState& B){
 return A.Step==B.Step&&A.Attempts==B.Attempts&&A.Recorded==B.Recorded&&A.Answers==B.Answers&&A.Feedback==B.Feedback;
}
}

void ALunarGameMode::GameplayTaskChecks(){
 FString Report;int32 Failed=0;
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 const bool Safe=ULunarSettings::IsAutomation()&&!ApolloSaveEnabled();
 Check(Safe,TEXT("Combined gameplay checks run in automation with owner checkpoint writes disabled"));
 Check(!Menu&&Stage==ELunarStage::Briefing&&P&&PC,TEXT("Combined checks begin with a fresh mission briefing and possessed astronaut"));
 if(Safe&&!Menu&&Stage==ELunarStage::Briefing&&P&&PC){
  // Natural completion is separate from the detailed skip/abort suite below.
  // Direct actor advancement and GameMode Tick exercise the real freeze path.
  const float BeforeOxygen=Oxygen,BeforeElapsed=ApolloElapsed,BeforeRadio=RadioRemaining;
  const FTransform BeforePawn=P->GetActorTransform();const uint8 BeforeSamples=SamplesCollected,BeforeParts=ApolloParts;
  auto* Natural=ALunarDepartureScene::Begin(this);
  Check(Natural&&ALunarDepartureScene::Active(this),TEXT("A real transient departure starts before mission state is committed"));
  if(Natural){
   Tick(.6f);Check(Oxygen==BeforeOxygen&&ApolloElapsed==BeforeElapsed&&RadioRemaining==BeforeRadio&&Stage==ELunarStage::Briefing,TEXT("GameMode Tick freezes mission resources time and old radio during departure"));
   const float SceneTime=Natural->PresentationTime();Paused=true;Natural->AdvancePresentation(2);Tick(2);
   Check(Natural->PresentationTime()==SceneTime&&Oxygen==BeforeOxygen&&ApolloElapsed==BeforeElapsed&&RadioRemaining==BeforeRadio,TEXT("Paused departure freezes both its actor clock and the old mission clocks"));Paused=false;
   Natural->AdvancePresentation(ALunarDepartureScene::Duration);
   Check(!ALunarDepartureScene::Active(this)&&IsActive()&&PC->GetViewTarget()==P,TEXT("Natural departure hands control back and commits the mission once"));
   Check(P->GetActorTransform().Equals(BeforePawn,.001f)&&SamplesCollected==BeforeSamples&&ApolloParts==BeforeParts,TEXT("Departure does not teleport the astronaut or award science cargo"));
  }
  ALunarDepartureScene::Abort(this);Stage=ELunarStage::Briefing;Paused=false;Journal=false;ApolloConfirmNew=false;DepartureAuthorizedNewRun=false;
  LunarCarryPresentation::ValidateChecks(this,Check);
  LunarCrew::ValidateChecks(this,Check);
  LunarWater::ValidateChecks(this,Check);
  if(P->ActivePanel)P->ClosePanel();
  // Exercise the actual fresh-run reset/commit boundary with only transient
  // optional progress. Automated StartApollo cannot write an owner save.
  LunarWater::FState Finished;LunarWater::RecordAnswer(Finished,1);LunarWater::RecordAnswer(Finished,2);LunarWater::RecordAnswer(Finished,0);LunarWater::Commit(Finished);
  WaterStudy=Finished;CrewTalkCount=4;CrewSpeaker=1;Stage=ELunarStage::Briefing;Paused=false;Journal=false;DepartureAuthorizedNewRun=true;
  CommitMissionStart();
  Check(IsActive()&&WaterStudy.Step==0&&WaterStudy.Attempts==0&&!WaterStudy.Recorded&&CrewTalkCount==0&&CrewSpeaker==0,TEXT("Fresh mission commit resets optional water work and crew conversation counters"));
  const float Budget=Oxygen,Elapsed=ApolloElapsed;const uint8 CargoSamples=SamplesCollected,CargoParts=ApolloParts,Setup=ApolloInstruments;const ELunarStage MainStage=Stage;
  if(Mars){
   P->OpenWaterNotebook();
   Check(!P->ActivePanel&&WaterStudy.Step==0&&!WaterStudy.Recorded,TEXT("ARES cannot open the remote lunar exercise as a local Mars task"));
  }else{
   P->OpenWaterNotebook();auto* Panel=P->ActivePanel.Get();
   Check(Panel&&Panel->ShowWaterStudy&&Panel->IsInViewport()&&Paused,TEXT("The actual water notebook opens as the owned paused mission panel"));
   if(Panel){
    const auto Fresh=WaterStudy;Panel->SetVisibility(ESlateVisibility::Collapsed);Panel->WaterAnswerB();
    Check(LunarGameplayCheckPrivate::SameStudy(WaterStudy,Fresh),TEXT("A hidden owned research panel cannot submit a water decision"));Panel->SetVisibility(ESlateVisibility::Visible);
    auto* ForeignFresh=CreateWidget<ULunarPanel>(PC);if(ForeignFresh){ForeignFresh->ShowWaterStudy=true;ForeignFresh->WaterAnswerB();}
    Check(ForeignFresh&&LunarGameplayCheckPrivate::SameStudy(WaterStudy,Fresh),TEXT("An unowned panel cannot advance a mutable fresh research decision"));
    const FKeyEvent Repeated(EKeys::Two,FModifierKeysState(),0,true,0,0);
    Panel->NativeOnKeyDown(Panel->GetCachedGeometry(),Repeated);
    Check(LunarGameplayCheckPrivate::SameStudy(WaterStudy,Fresh),TEXT("A repeated research answer key does not skip a decision"));
    Tick(2.5f);Check(Oxygen==Budget&&ApolloElapsed==Elapsed&&Stage==MainStage,TEXT("Reading the water panel pauses mission time and gameplay resource use"));
    Panel->WaterAnswerA();Check(WaterStudy.Step==0&&WaterStudy.Attempts==1&&!WaterStudy.Recorded&&!WaterStudy.Feedback.IsEmpty(),TEXT("The wrong first notebook answer produces feedback without granting progress"));
    Panel->WaterAnswerB();Check(WaterStudy.Step==1&&WaterStudy.Attempts==2&&!WaterStudy.Recorded,TEXT("The correct remote-site answer advances exactly one optional decision"));
    Panel->WaterAnswerC();Check(WaterStudy.Step==2&&WaterStudy.Attempts==3&&!WaterStudy.Recorded,TEXT("The mass-fraction answer advances to uncertainty reasoning"));
    Panel->WaterAnswerA();Check(WaterStudy.Step==3&&WaterStudy.Attempts==4&&LunarWater::ReadyToCommit(WaterStudy)&&!WaterStudy.Recorded,TEXT("Finishing the three decisions still requires an explicit report commit"));
    Panel->CommitWaterReport();Check(WaterStudy.Recorded&&LunarWater::IsValidState(WaterStudy),TEXT("The actual notebook commit records only a valid completed research report"));
    const auto Recorded=WaterStudy;Panel->CommitWaterReport();Panel->ResetWaterReport();
    Check(LunarGameplayCheckPrivate::SameStudy(WaterStudy,Recorded),TEXT("Duplicate commit and reset callbacks cannot rewrite an already recorded report"));
    auto* Foreign=CreateWidget<ULunarPanel>(PC);Check(Foreign!=nullptr,TEXT("An unowned panel fixture can exercise callback ownership"));
    if(Foreign){Foreign->ShowWaterStudy=true;Foreign->WaterAnswerB();Foreign->CommitWaterReport();Foreign->ResetWaterReport();Foreign->ToggleWaterSources();}
    Check(LunarGameplayCheckPrivate::SameStudy(WaterStudy,Recorded),TEXT("Callbacks from an unowned panel cannot change the active research record"));
    P->ClosePanel();WaterStudy=Fresh;Panel->WaterAnswerB();
    Check(LunarGameplayCheckPrivate::SameStudy(WaterStudy,Fresh),TEXT("A stale closed panel cannot advance a mutable fresh decision"));WaterStudy=Recorded;
    Panel->WaterAnswerA();Panel->CommitWaterReport();Panel->ResetWaterReport();Panel->ToggleWaterSources();
    Check(!P->ActivePanel&&!Panel->IsInViewport()&&LunarGameplayCheckPrivate::SameStudy(WaterStudy,Recorded),TEXT("Callbacks from the closed stale notebook are harmless"));
    Check(Oxygen==Budget&&ApolloElapsed==Elapsed&&SamplesCollected==CargoSamples&&ApolloParts==CargoParts&&ApolloInstruments==Setup&&Stage==MainStage,TEXT("The complete optional UI exercise awards no primary cargo setup time or resource"));
   }
   if(Apollo&&WaterStudy.Recorded){
    auto* Save=CaptureApolloCheckpoint();const auto& Data=LunarWater::Dataset();
    Check(Save&&Save->WaterDataId==FName(*Data.Id)&&Save->WaterAnswers==WaterStudy.Answers&&Save->WaterStep==3&&Save->WaterAttempts==4&&Save->WaterRecorded,TEXT("Apollo capture includes valid optional water decisions and their exact dataset identity"));
    TArray<uint8> Memory;const bool Serialized=Save&&UGameplayStatics::SaveGameToMemory(Save,Memory);
    auto* Loaded=Serialized?Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Memory)):nullptr;
    Check(Loaded&&Loaded->WaterDataId==Save->WaterDataId&&Loaded->WaterAnswers==Save->WaterAnswers&&Loaded->WaterStep==Save->WaterStep&&Loaded->WaterAttempts==Save->WaterAttempts&&Loaded->WaterRecorded,TEXT("A real memory-only SaveGame roundtrip retains the optional research record"));
    if(Loaded){
     FString Reason;const auto Tasks=Loaded->CompletedTasks;Loaded->WaterDataId=TEXT("unrelated-study");Loaded->WaterAnswers={99};Loaded->WaterStep=999;Loaded->WaterRecorded=true;
     Check(Loaded->Validate(Reason)&&Loaded->CompletedTasks==Tasks,TEXT("Invalid optional research does not invalidate or rewrite valid historical cargo"));
     Loaded->WaterDataId=NAME_None;Loaded->WaterAnswers.Reset();Loaded->WaterStep=Loaded->WaterAttempts=0;Loaded->WaterRecorded=false;Reason.Reset();
     Check(Loaded->FormatVersion==1&&Loaded->Validate(Reason)&&Loaded->CompletedTasks==Tasks,TEXT("Version-one cargo remains valid when additive water fields are absent"));
    }
   }
  }
  if(P->ActivePanel)P->ClosePanel();
  CrewTalkCount=3;CrewSpeaker=1;Stage=ELunarStage::Briefing;Paused=false;Journal=false;DepartureAuthorizedNewRun=true;CommitMissionStart();
  Check(WaterStudy.Step==0&&WaterStudy.Attempts==0&&!WaterStudy.Recorded&&CrewTalkCount==0&&CrewSpeaker==0,TEXT("A later new run clears the recorded optional study and conversation count"));
  // This detailed suite deliberately commits through Skip and ends active.
  // Keep it last so that its own fresh-briefing invariants remain meaningful.
  ALunarDepartureScene::Abort(this);Stage=ELunarStage::Briefing;Paused=false;Journal=false;ApolloConfirmNew=false;DepartureAuthorizedNewRun=false;
  ALunarDepartureScene::ValidateChecks(this,Check);
 }
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);
 if(!FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("gameplay-tasks-checks.txt")))){++Failed;UE_LOG(LogTemp,Error,TEXT("GAMEPLAY_TASKS_CHECKS report write failed"));}
 UE_LOG(LogTemp,Display,TEXT("GAMEPLAY_TASKS_CHECKS\n%s"),*Report);
 FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}
