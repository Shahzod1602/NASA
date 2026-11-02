#include "LunarDiagnostics.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

void LunarDiagnostics::RunChecks(ALunarGameMode* Game){
 FString Report;int32 Failed=0;
 auto Check=[&](bool Good,const FString& Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto Row=[&](const FSnapshot& Snapshot,const TCHAR* Key)->const FRow*{return Snapshot.Rows.FindByPredicate([&](const FRow& Value){return Value.Key==FName(Key);});};
 auto Is=[&](const FSnapshot& Snapshot,const TCHAR* Key,EState State){const auto* Found=Row(Snapshot,Key);return Found&&Found->State==State;};
 Check(Game&&!Game->Menu,TEXT("Diagnostics checks have a playable mission context"));
 Check(!Build(nullptr).Available&&Build(nullptr).Rows.IsEmpty(),TEXT("Missing context produces no invented diagnostic records"));
 if(Game){
  // These checks only substitute transient state and references. No interaction,
  // checkpoint, actor transform, photograph or owner data is written.
  TGuardValue<bool> ApolloGuard(Game->Apollo,false),MarsGuard(Game->Mars,false),MenuGuard(Game->Menu,false),PausedGuard(Game->Paused,false);
  TGuardValue<ELunarStage> StageGuard(Game->Stage,ELunarStage::Briefing);
  TGuardValue<int32> RepairGuard(Game->RepairStep,0),ActionGuard(Game->ApolloAction,0),SelectedGuard(Game->SelectedSample,0),ToolGuard(Game->ApolloTool,0),PhaseGuard(Game->ApolloToolPhase,0);
  TGuardValue<bool> SequenceGuard(Game->PowerSequenceActive,false),OnlineGuard(Game->StationRebootComplete,false),SurveyorGuard(Game->SurveyorPhotographed,false),PendingGuard(Game->ApolloPhotoPending,false);
  TGuardValue<uint8> PhotosGuard(Game->ApolloPhotos,0),InstrumentsGuard(Game->ApolloInstruments,0),SamplesGuard(Game->SamplesCollected,0),PartsGuard(Game->ApolloParts,0);
  TGuardValue<TObjectPtr<AActor>> ModuleGuard(Game->ModuleActor,Game->ModuleActor),PowerGuard(Game->PowerActor,Game->PowerActor),DataGuard(Game->DataActor,Game->DataActor);
  Game->PowerActor=Game->ModuleActor;Game->DataActor=Game->ModuleActor;
  Game->Menu=true;Check(!Build(Game).Available,TEXT("Mission-selection menu has no equipment records"));Game->Menu=false;
  auto State=Build(Game);Check(State.Rows.Num()==5&&State.NextAction.Contains(TEXT("briefing"))&&Is(State,TEXT("station.startup"),EState::Pending),TEXT("SELENE briefing introduces five dependencies without claiming startup"));
  Game->Stage=ELunarStage::FindModule;State=Build(Game);
  Check(IsValid(Game->ModuleActor.Get())?Is(State,TEXT("cell.recovered"),EState::Ready):Is(State,TEXT("cell.recovered"),EState::Unavailable),TEXT("Cell recovery is ready only when its mission actor exists"));
  Game->Stage=ELunarStage::RestorePower;State=Build(Game);
  Check(Is(State,TEXT("cell.recovered"),EState::Complete)&&Is(State,TEXT("cell.seated"),EState::Ready)&&Is(State,TEXT("cable.connected"),EState::Blocked)&&State.NextAction.Contains(TEXT("insert")),TEXT("A recovered cell enables seating, while the cable remains blocked"));
  Game->RepairStep=1;State=Build(Game);
  Check(Is(State,TEXT("cell.seated"),EState::Complete)&&Is(State,TEXT("cable.connected"),EState::Ready)&&Is(State,TEXT("station.startup"),EState::Blocked)&&State.NextAction.Contains(TEXT("connect the power cable")),TEXT("Seating the cell enables the cable but does not imply station power"));
  Game->RepairStep=2;State=Build(Game);
  Check(Is(State,TEXT("cable.connected"),EState::Complete)&&Is(State,TEXT("station.startup"),EState::Ready)&&Is(State,TEXT("science.download"),EState::Blocked)&&State.NextAction.Contains(TEXT("command the station restart")),TEXT("A connected cable still requires a separate restart command"));
  Game->Stage=ELunarStage::CollectData;Game->PowerSequenceActive=true;State=Build(Game);
  Check(Is(State,TEXT("station.startup"),EState::InProgress)&&Is(State,TEXT("science.download"),EState::Blocked)&&State.NextAction.Contains(TEXT("startup finish")),TEXT("Entering CollectData does not falsely complete an unfinished station restart"));
  const auto BeforeStage=Game->Stage;const float BeforeOxygen=Game->Oxygen,BeforeElapsed=Game->ApolloElapsed,BeforeDistance=Game->ApolloDistance,BeforeSequence=Game->PowerSequenceSeconds,BeforeHold=Game->HoldProgress;
  Game->Paused=true;for(int32 I=0;I<10;++I)Build(Game);
  Check(Game->Stage==BeforeStage&&Game->Oxygen==BeforeOxygen&&Game->ApolloElapsed==BeforeElapsed&&Game->ApolloDistance==BeforeDistance&&Game->PowerSequenceSeconds==BeforeSequence&&Game->HoldProgress==BeforeHold&&Game->Paused&&Game->PowerSequenceActive,TEXT("Repeated paused inspection changes no stage, resource, telemetry, restart clock or hold progress"));
  Game->Paused=false;Game->PowerSequenceActive=false;Game->StationRebootComplete=true;State=Build(Game);
  Check(Is(State,TEXT("station.startup"),EState::Complete)&&Is(State,TEXT("science.download"),EState::Ready)&&State.NextAction.Contains(TEXT("download")),TEXT("Actual reboot completion enables the story-data terminal"));
  Game->Stage=ELunarStage::SurveySamples;State=Build(Game);
  Check(Is(State,TEXT("science.download"),EState::Complete)&&State.NextAction.Contains(TEXT("current mission objective")),TEXT("Moon post-download objectives remain separate from power diagnostics"));
  Game->Mars=true;Game->Stage=ELunarStage::AlignRelay;State=Build(Game);
  Check(State.Rows.Num()==6&&State.Title.Contains(TEXT("ARES"))&&Is(State,TEXT("relay.aligned"),EState::Ready)&&State.NextAction.Contains(TEXT("antenna")),TEXT("ARES uses its actual extra relay dependency after the download"));
  Game->Stage=ELunarStage::ReturnHome;State=Build(Game);Check(Is(State,TEXT("relay.aligned"),EState::Complete),TEXT("ReturnHome records completed Mars relay alignment"));
  Game->Stage=ELunarStage::Lost;State=Build(Game);
  Check(Is(State,TEXT("science.download"),EState::Unavailable)&&Is(State,TEXT("relay.aligned"),EState::Unavailable)&&State.NextAction.Contains(TEXT("Restart")),TEXT("Failure without separate retained flags never invents earlier download or relay completion"));
  Game->Stage=ELunarStage::RestorePower;Game->RepairStep=-1;State=Build(Game);
  Check(Is(State,TEXT("cell.seated"),EState::Unavailable)&&State.NextAction.Contains(TEXT("unavailable")),TEXT("Negative repair state is reported as unavailable"));
  Game->RepairStep=3;Check(Is(Build(Game),TEXT("cable.connected"),EState::Unavailable),TEXT("Out-of-range repair state never implies a connected cable"));
  Game->RepairStep=0;Game->StationRebootComplete=false;Game->Stage=ELunarStage::FindModule;Game->ModuleActor=nullptr;State=Build(Game);
  Check(Is(State,TEXT("cell.recovered"),EState::Unavailable)&&State.NextAction.Contains(TEXT("actor is unavailable")),TEXT("Missing cell actor has an explicit failure instead of a fictitious pickup"));
  Game->ModuleActor=ModuleGuard.GetOriginalValue();Game->Stage=ELunarStage::RestorePower;Game->PowerActor=nullptr;
  Check(Build(Game).NextAction.Contains(TEXT("power-port actor is unavailable")),TEXT("Missing station port is identified before suggesting an impossible repair"));
  Game->PowerActor=Game->ModuleActor;Game->DataActor=nullptr;Game->RepairStep=2;Game->Stage=ELunarStage::CollectData;Game->StationRebootComplete=true;
  Check(Build(Game).NextAction.Contains(TEXT("Science-terminal actor is unavailable")),TEXT("Missing science terminal never produces a download instruction"));
  Game->DataActor=Game->ModuleActor;

  Game->Apollo=true;Game->Mars=false;Game->Stage=ELunarStage::ApolloCheckout;Game->ApolloAction=1;Game->StationRebootComplete=false;Game->RepairStep=0;
  State=Build(Game);
  Check(State.Rows.Num()==7&&State.Title.Contains(TEXT("APOLLO 12"))&&!Row(State,TEXT("station.startup"))&&State.NextAction.Contains(TEXT("camera and tools")),TEXT("Apollo shows its actual checkout confirmations, not the rescue power-cell chain"));
  Game->Stage=ELunarStage::ApolloDeploy;Game->ApolloAction=0;State=Build(Game);
  Check(Is(State,TEXT("apollo.checkout"),EState::Complete)&&Is(State,TEXT("apollo.level"),EState::Ready)&&Is(State,TEXT("apollo.link"),EState::Blocked),TEXT("Apollo setup explains the existing seismometer-before-Earth-link sequence"));
  Game->ApolloInstruments=1;Game->ApolloAction=1;State=Build(Game);
  Check(Is(State,TEXT("apollo.level"),EState::Complete)&&Is(State,TEXT("apollo.link"),EState::Ready)&&State.NextAction.Contains(TEXT("strongest signal")),TEXT("Only a committed seismometer step unlocks the link checklist row"));
  Game->ApolloInstruments=3;Game->Stage=ELunarStage::ApolloSamples;Game->SelectedSample=2;State=Build(Game);
  Check(State.NextAction.Contains(TEXT("12027 core"))&&State.NextAction.Contains(TEXT("photograph"))&&Row(State,TEXT("apollo.samples"))->Detail.Contains(TEXT("packed samples 0 / 3")),TEXT("A core without a committed context photo still asks for that photo"));
  Game->ApolloTool=5;Game->ApolloToolPhase=6;State=Build(Game);
  Check(Row(State,TEXT("apollo.samples"))->Detail.Contains(TEXT("packed samples 0 / 3")),TEXT("A withdrawn visual core is not counted as packed cargo"));
  Game->ApolloPhotos=4;State=Build(Game);Check(State.NextAction.Contains(TEXT("seal or cap")),TEXT("A committed context photo enables the normal collection-and-packing explanation"));
  Game->ApolloPhotoPending=true;Check(Build(Game).NextAction.Contains(TEXT("finish saving")),TEXT("Pending asynchronous photos are never described as committed evidence"));Game->ApolloPhotoPending=false;
  Game->SamplesCollected=7;Game->Stage=ELunarStage::ApolloTraverse;State=Build(Game);
  Check(Is(State,TEXT("apollo.samples"),EState::Complete)&&State.NextAction.Contains(TEXT("Walk to the Surveyor")),TEXT("Completed sample cargo leads to the real traverse objective"));
  Game->Stage=ELunarStage::ApolloRecover;State=Build(Game);
  Check(Is(State,TEXT("apollo.surveyor.photo"),EState::Ready)&&Is(State,TEXT("apollo.hardware"),EState::Blocked)&&State.NextAction.Contains(TEXT("whole Surveyor")),TEXT("Surveyor hardware stays blocked before its own context photograph"));
  Game->SurveyorPhotographed=true;State=Build(Game);
  Check(Is(State,TEXT("apollo.surveyor.photo"),EState::Complete)&&Is(State,TEXT("apollo.hardware"),EState::Ready),TEXT("The committed Surveyor photo enables existing hardware work"));
  Game->ApolloParts=7;Game->Stage=ELunarStage::ApolloReturn;State=Build(Game);
  Check(Is(State,TEXT("apollo.return"),EState::Ready)&&State.NextAction.Contains(TEXT("cargo stowage")),TEXT("All real cargo and instrument masks are required before the return row is ready"));
  Game->ApolloParts=0;State=Build(Game);
  Check(Is(State,TEXT("apollo.return"),EState::Blocked)&&State.NextAction.Contains(TEXT("incomplete")),TEXT("Entering the return stage cannot substitute for missing cargo records"));
  Game->ApolloParts=7;Game->ApolloInstruments=0x83;State=Build(Game);
  Check(Is(State,TEXT("apollo.return"),EState::Blocked),TEXT("Extra unsupported instrument bits do not pass the actual exact-mask stowage rule"));
  Game->ApolloInstruments=3;Game->Stage=ELunarStage::Won;Check(Is(Build(Game),TEXT("apollo.return"),EState::Complete),TEXT("Only the won mission records completed cargo stowage"));
  Game->ApolloInstruments=0;Game->SamplesCollected=0;Game->ApolloParts=0;Game->ApolloPhotos=0;Game->SurveyorPhotographed=false;Game->Stage=ELunarStage::Briefing;
  Check(Is(Build(Game),TEXT("apollo.checkout"),EState::Pending)&&Is(Build(Game),TEXT("apollo.samples"),EState::Blocked),TEXT("Fresh mission state yields no stale diagnostic completion cache"));

  if(auto* PC=UGameplayStatics::GetPlayerController(Game,0)){
   auto* Panel=CreateWidget<ULunarPanel>(PC);Panel->TakeWidget();bool HasChecklist=false;
   TArray<UWidget*> Widgets;Panel->WidgetTree->GetAllWidgets(Widgets);for(auto* Widget:Widgets)if(auto* Label=Cast<UTextBlock>(Widget))HasChecklist|=Label->GetText().ToString()==TEXT("APOLLO 12 / FIELDWORK CHECKLIST");
   Check(HasChecklist,TEXT("The existing notebook builds the integrated read-only checklist"));
   const int32 Before=Panel->Body?Panel->Body->GetChildrenCount():0;Panel->Settings=true;BuildNotebook(Panel);
   Check(Panel->Body&&Panel->Body->GetChildrenCount()==Before,TEXT("Settings panels cannot append mission diagnostics"));
  }else Check(false,TEXT("A player controller is required for the notebook integration check"));
  Check(!State.ModelNote.IsEmpty()&&State.ModelNote.Contains(TEXT("authored"))&&State.ModelNote.Contains(TEXT("not hardware health")),TEXT("Apollo diagnostics preserve the distinction between recorded actions and measured engineering"));
 }
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);
 const FString Path=Game?Game->ReportPath(TEXT("equipment-diagnostics-checks.txt")):FPaths::ProjectDir()/TEXT("Reports/equipment-diagnostics-checks.txt");
 IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);FFileHelper::SaveStringToFile(Report,*Path);
 FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}
