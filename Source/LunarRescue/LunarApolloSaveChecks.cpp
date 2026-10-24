#include "LunarGame.h"
#include "LunarApolloText.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Crc.h"
#include "HAL/FileManager.h"
#include "Serialization/MemoryWriter.h"
#include "Algo/Reverse.h"
#include <limits>

namespace {
FString CrossReport;
bool CrossMenuVisited=false;
TArray<uint8> Raw(const FString& Path){TArray<uint8> Bytes;FFileHelper::LoadFileToArray(Bytes,*Path);const int64 Size=IFileManager::Get().FileSize(*Path);if(Size>=0&&Size<=Bytes.Num())Bytes.SetNum(int32(Size),EAllowShrinking::No);return Bytes;}
void RawSnapshot(const FString& Path,UApolloCheckpoint* Save){
 TArray<uint8> Payload,Bytes;UGameplayStatics::SaveGameToMemory(Save,Payload);FMemoryWriter W(Bytes);uint32 Magic=0x3150434c,Version=1,Crc=FCrc::MemCrc32(Payload.GetData(),Payload.Num());int32 Count=Payload.Num();W<<Magic<<Version<<Count<<Crc;W.Serialize(Payload.GetData(),Payload.Num());W.Close();FFileHelper::SaveArrayToFile(Bytes,*Path);
}
UApolloCheckpoint* Clone(UApolloCheckpoint* Save){TArray<uint8> Bytes;UGameplayStatics::SaveGameToMemory(Save,Bytes);return Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Bytes));}
}

void ALunarGameMode::ApolloSaveChecks(){
 ApolloToolsEnabled=false;ApolloTimed=true;
 int Fail=0;FString Report;
 const bool ReadTest=FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveReadTest")),WriteTest=FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveWriteTest"));
 const TCHAR* ReportName=ReadTest?TEXT("checkpoint-read-checks.txt"):WriteTest?TEXT("checkpoint-write-checks.txt"):TEXT("checkpoint-checks.txt");
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Fail;};
 auto Finish=[&](){Report=CrossReport+Report+FString::Printf(TEXT("RESULT: %d failures\n"),Fail);FFileHelper::SaveStringToFile(Report,*ReportPath(ReportName));UE_LOG(LogTemp,Display,TEXT("APOLLO_SAVE_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Fail?1:0);};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){Check(false,TEXT("Possessed astronaut exists"));Finish();return;}
 if(ReadTest&&Menu){
  Check(ApolloCachedCheckpoint&&ApolloCachedCheckpoint->Has(ApolloTasks::Samples[1]),TEXT("Fresh process discovers the saved mission in the menu"));
  if(!ApolloCachedCheckpoint){Finish();return;}
  CrossMenuVisited=true;CrossReport=Report;
  FTimerHandle Guard;GetWorldTimerManager().SetTimer(Guard,[this](){FFileHelper::SaveStringToFile(CrossReport+TEXT("FAIL C input did not travel to the saved mission\nRESULT: 1 failures\n"),*ReportPath(TEXT("checkpoint-read-checks.txt")));FPlatformMisc::RequestExitWithStatus(false,1);},10,false);
  PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::C,IE_Pressed,1));return;
 }
 const FString Dir=ApolloSaveDirectory(),Main=ApolloCheckpointStorage::Primary(Dir),Backup=ApolloCheckpointStorage::Backup(Dir);
 Check(Dir.Contains(TEXT("AutomatedTests")),TEXT("Save tests are isolated from player saves"));
 if(!Dir.Contains(TEXT("AutomatedTests"))){Finish();return;}
 auto Clean=[&](){for(const FString& Path:{Main,Backup,Main+TEXT(".tmp"),Backup+TEXT(".tmp")}){IFileManager::Get().Delete(*Path,false,true);IFileManager::Get().DeleteDirectory(*Path,false,false);}RefreshApolloCheckpoint();};
 auto Aim=[&](AActor* A){P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);P->GetCharacterMovement()->StopMovementImmediately();P->SetActorLocation(A->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Work=[&](){Interact();AdvanceInteraction(6);ReleaseInteract();};
 auto RoundTrip=[&](const TCHAR* Name){
  auto* Expected=CaptureApolloCheckpoint();const bool Written=SaveApolloCheckpoint();
  if(!Expected||!Written){Check(false,Name);return;}
  const ELunarStage ExpectedStage=Stage;const uint8 Samples=SamplesCollected,Photos=ApolloPhotos,Parts=ApolloParts,Instruments=ApolloInstruments;
  Stage=ELunarStage::ApolloCheckout;ApolloAction=0;SamplesCollected=0;ApolloPhotos=0;ApolloParts=0;ApolloInstruments=0;ApolloElapsed=0;Oxygen=1;ApolloDistance=0;
  const bool Loaded=RestoreApolloCheckpoint();
  Check(Loaded&&Stage==ExpectedStage&&SamplesCollected==Samples&&ApolloPhotos==Photos&&ApolloParts==Parts&&ApolloInstruments==Instruments&&ApolloRunId==Expected->RunId&&Oxygen==Expected->Remaining&&ApolloElapsed==Expected->Elapsed&&ApolloDistance==Expected->Distance&&!Holding&&!Paused,Name);
 };
 if(ReadTest){
  Check(CrossMenuVisited&&Apollo&&Stage==ELunarStage::ApolloSamples,TEXT("C input crosses maps and restores progress in a new process"));
  Check(SamplesCollected==2&&ApolloPhotos==2&&ApolloInstruments==3&&SelectedSample==2,TEXT("Named evidence, ALSEP and selected destination survive restart"));
  Check(ApolloEvidence.Num()==1&&ApolloEvidence[0].PhotoId==FName(ApolloTasks::Photos[1])&&SampleActors[1]->IsHidden()&&!SampleActors[0]->IsHidden(),TEXT("Evidence metadata and collected-object visibility are restored"));
  Check(Oxygen<=1635&&Oxygen>1628&&ApolloElapsed>=165&&ApolloElapsed<172&&FMath::IsNearlyEqual(ApolloDistance,285.f,.1f),TEXT("Exercise time and distance resume without resetting"));
  Aim(SampleActors[2]);Interact();Work();Check(SamplesCollected==6&&ApolloCachedCheckpoint&&ApolloCachedCheckpoint->Has(ApolloTasks::Samples[2]),TEXT("A resumed expedition can collect and checkpoint the next sample"));
  Finish();return;
 }
 Clean();StartApollo(true);
 if(WriteTest){
  Aim(ModuleActor);Work();Work();Work();Aim(ApolloInstrumentActors[0]);Work();Aim(ApolloInstrumentActors[1]);for(int I=0;I<5;++I)CycleApollo();Work();
  Aim(SampleActors[1]);Interact();Work();ApolloElapsed=165;Oxygen=1635;ApolloDistance=285;SelectedSample=2;
  Check(SaveApolloCheckpoint()&&ApolloCachedCheckpoint->Has(ApolloTasks::Samples[1])&&ApolloCachedCheckpoint->Evidence.Num()==1,TEXT("Named partial expedition is written for a separate process"));Finish();return;
 }
 Check(ApolloCachedCheckpoint&&ApolloCachedCheckpoint->CompletedTasks.IsEmpty(),TEXT("Starting an expedition creates an initial checkpoint"));
 const TArray<uint8> Initial=Raw(Main);Stage=ELunarStage::Briefing;StartApollo();Check(ApolloConfirmNew&&Raw(Main)==Initial,TEXT("New expedition confirmation leaves the existing checkpoint untouched"));
 P->PrimaryClick();Check(ApolloConfirmNew&&Raw(Main)==Initial,TEXT("An extra mouse click cannot accept the destructive new-run confirmation"));P->PauseMission();Check(!ApolloConfirmNew,TEXT("Escape cancels the new-run confirmation"));
 const FGuid PreviousRun=ApolloRunId;P->StartMission();P->StartMission();
 Check(!ApolloConfirmNew&&ApolloRunId!=PreviousRun&&ApolloCachedCheckpoint&&ApolloCachedCheckpoint->RunId==ApolloRunId,TEXT("Enter confirms a new run and replaces the checkpoint only after confirmation"));
 P->PauseMission();P->RestartMission();Check(!Paused&&Stage==ELunarStage::ApolloCheckout,TEXT("The pause R handler restores the checkpoint and resumes movement"));
 Stage=ELunarStage::ApolloCheckout;Aim(ModuleActor);Interact();AdvanceInteraction(1);
 Check(SaveApolloCheckpoint()&&RestoreApolloCheckpoint()&&ApolloAction==0&&!Holding,TEXT("Partial work is discarded rather than falsely completed on reload"));
 for(int I=0;I<3;++I){Aim(ModuleActor);Work();RoundTrip(TEXT("Checkout checkpoint restores committed work and telemetry"));}
 Aim(ApolloInstrumentActors[0]);Work();RoundTrip(TEXT("Seismometer checkpoint restores deployment"));
 Aim(ApolloInstrumentActors[1]);CycleApollo();SaveApolloCheckpoint();const float Bearing=ApolloAntennaAngle;RestoreApolloCheckpoint();Check(ApolloAntennaAngle==Bearing&&ApolloAction==1,TEXT("Partially aimed antenna survives checkpoint loading"));
 for(int I=0;I<4;++I)CycleApollo();Work();RoundTrip(TEXT("Verified antenna checkpoint opens science"));
 for(int I:{1,0,2}){Aim(SampleActors[I]);Interact();RoundTrip(TEXT("Context photo ID and site survive reload"));Aim(SampleActors[I]);Work();RoundTrip(TEXT("Collected sample remains collected after reload"));Check(SampleActors[I]->IsHidden(),TEXT("Loading does not respawn a collected sample"));}
 Aim(DroneActor);TickApollo(0);RoundTrip(TEXT("Surveyor arrival checkpoint restores the approach gate"));
 Aim(DroneActor);Interact();RoundTrip(TEXT("Surveyor context photograph survives reload"));
 for(int I:{2,0,1}){Aim(ApolloPartActors[I]);Work();RoundTrip(TEXT("Hardware retrieval checkpoint restores named parts"));Check(ApolloPartActors[I]->IsHidden(),TEXT("Loading does not respawn retrieved hardware"));}
 Aim(HomeActor);Work();RoundTrip(TEXT("Completed expedition reloads its result rather than restarting"));
 auto* Complete=CaptureApolloCheckpoint();Check(Complete&&Complete->CompletedTasks.Num()==17,TEXT("All seventeen committed work IDs describe the completed expedition"));
 if(!Complete){Finish();return;}
 auto* Shuffled=Clone(Complete);Algo::Reverse(Shuffled->CompletedTasks);Check(ApolloCheckpointStorage::Write(Dir,Shuffled)&&RestoreApolloCheckpoint()&&Stage==ELunarStage::Won,TEXT("Persistent work IDs are independent of their serialized ordering"));
 auto* Bad=Clone(Complete);Bad->FormatVersion=2;Check(!ApolloCheckpointStorage::Write(Dir,Bad),TEXT("Unsupported schema cannot overwrite a valid checkpoint"));
 Bad=Clone(Complete);Bad->CompletedTasks.Remove(FName(ApolloTasks::SurveyorPhoto));Check(!ApolloCheckpointStorage::Write(Dir,Bad),TEXT("Hardware without its prerequisite photograph is rejected"));
 Bad=Clone(Complete);Bad->CompletedTasks[0]=FName(TEXT("unknown.task"));Check(!ApolloCheckpointStorage::Write(Dir,Bad),TEXT("Unknown task identifiers are rejected"));
 Bad=Clone(Complete);Bad->Evidence[0].PhotoPath=TEXT("../outside.png");Check(!ApolloCheckpointStorage::Write(Dir,Bad),TEXT("Evidence paths cannot escape the run's photo folder"));
 Bad=Clone(Complete);Bad->Remaining=std::numeric_limits<float>::quiet_NaN();Check(!ApolloCheckpointStorage::Write(Dir,Bad),TEXT("Non-finite resource values are rejected"));
 Bad=Clone(Complete);Bad->Position.X=100000;Check(!ApolloCheckpointStorage::Write(Dir,Bad),TEXT("Out-of-area positions are rejected"));
 Bad=Clone(Complete);Bad->MissionId=TEXT("different-mission");RawSnapshot(Main,Bad);IFileManager::Get().Delete(*Backup,false,true);EApolloSaveStatus Status;
 Check(!ApolloCheckpointStorage::Read(Dir,Status)&&Status==EApolloSaveStatus::Incompatible,TEXT("A valid binary for another mission is reported as incompatible"));
 StartApollo(true);Aim(ModuleActor);Work();const TArray<uint8> GoodPrimary=Raw(Main);
 IFileManager::Get().MakeDirectory(*(Main+TEXT(".tmp")),false);Check(!SaveApolloCheckpoint()&&Raw(Main)==GoodPrimary,TEXT("Failed temporary write preserves the previous checkpoint"));IFileManager::Get().DeleteDirectory(*(Main+TEXT(".tmp")),false,false);
 // Backup rotation also has to succeed before the primary is replaced.
 IFileManager::Get().MakeDirectory(*(Backup+TEXT(".tmp")),false);Check(!SaveApolloCheckpoint()&&Raw(Main)==GoodPrimary,TEXT("Failed backup rotation preserves the primary"));IFileManager::Get().DeleteDirectory(*(Backup+TEXT(".tmp")),false,false);
 SaveApolloCheckpoint();TArray<uint8> Broken=Raw(Main);Broken[Broken.Num()-1]^=0x7f;FFileHelper::SaveArrayToFile(Broken,*Main);
 Check(RestoreApolloCheckpoint()&&ApolloSaveStatus==EApolloSaveStatus::Recovered,TEXT("Corrupt primary recovers the previous verified backup"));
 Check(SaveApolloCheckpoint(),TEXT("A recovered checkpoint can create a healthy primary again"));
 // Exercise the actual missing-photo load path with otherwise valid science progress.
 auto* Missing=Clone(Complete);Missing->Evidence[0].PhotoPath=FString(TEXT("Apollo12/Photos/"))+Missing->RunId.ToString(EGuidFormats::Digits)+TEXT("/not-present.png");
 Check(ApolloCheckpointStorage::Write(Dir,Missing)&&RestoreApolloCheckpoint()&&Stage==ELunarStage::Won&&ApolloSaveNotice==TEXT("CheckpointPhotosMissing"),TEXT("Missing photo files preserve progress and show a clear warning"));
 StartApollo(true);FVector LastGround;ApolloSafeGround(P->GetActorLocation(),LastGround);ApolloSafePosition=LastGround;ApolloHasSafePosition=true;P->GetCharacterMovement()->SetMovementMode(MOVE_Falling);P->SetActorLocation(LastGround+FVector(0,0,400));
 auto* Air=CaptureApolloCheckpoint();Check(Air&&FMath::Abs(Air->Position.Z-LastGround.Z)<20,TEXT("Saving in the air uses the last grounded location"));
 Check(SaveApolloCheckpoint()&&RestoreApolloCheckpoint()&&P->GetCharacterMovement()->IsMovingOnGround(),TEXT("Reload resets velocity, work and airborne state"));
 Broken={1,2,3};FFileHelper::SaveArrayToFile(Broken,*Main);FFileHelper::SaveArrayToFile(Broken,*Backup);
 Check(!ApolloCheckpointStorage::Read(Dir,Status)&&Status==EApolloSaveStatus::Corrupt,TEXT("Two damaged copies report corruption without loading invalid state"));
 IFileManager::Get().Delete(*Main,false,true);IFileManager::Get().Delete(*Backup,false,true);RawSnapshot(Main+TEXT(".tmp"),Complete);
 Check(!ApolloCheckpointStorage::Read(Dir,Status)&&Status==EApolloSaveStatus::Missing,TEXT("An uncommitted temporary file is never silently loaded"));
 TArray<uint8> Oversized;Oversized.Init(0,512*1024+1);FFileHelper::SaveArrayToFile(Oversized,*Main);
 Check(!ApolloCheckpointStorage::Read(Dir,Status)&&Status==EApolloSaveStatus::Corrupt,TEXT("Oversized files are rejected before deserialization"));
 Clean();Finish();
}
