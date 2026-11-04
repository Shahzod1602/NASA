#include "LunarWaterStudy.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void LunarWater::ValidateChecks(const UObject* Context,TFunctionRef<void(bool,const TCHAR*)> Check){
 Check(Context!=nullptr,TEXT("Water-study checks have an engine context"));
 FString Text;const bool Found=FFileHelper::LoadFileToString(Text,*(FPaths::ProjectContentDir()/TEXT("Science/LunarWater/study.json")));
 FDataset D;FString Error;Check(Found&&ParseDataset(Text,D,Error)&&D.Available&&Dataset().Available,TEXT("Bundled offline LCROSS publication record loads with its required provenance"));
 Check(D.Id==TEXT("lcross-cabeus-published-estimate-v1")&&D.ObservationDate==TEXT("2009-10-09")&&D.DOI==TEXT("10.1126/science.1186986"),TEXT("Remote dataset has its actual site-study identity, date and original publication"));
 Check(FMath::IsNearlyEqual(D.WaterPercent,5.6,1.e-8)&&FMath::IsNearlyEqual(D.UncertaintyPoints,2.9,1.e-8)&&FMath::IsNearlyEqual(D.WaterPercent-D.UncertaintyPoints,2.7,1.e-8)&&FMath::IsNearlyEqual(D.WaterPercent+D.UncertaintyPoints,8.5,1.e-8),TEXT("Percent and percentage-point arithmetic preserve the cited estimate and its reported uncertainty"));
 Check(D.ObservedWaterKg==155&&D.ObservedWaterUncertaintyKg==12&&D.CoordinateScope.Contains(TEXT("not the LCROSS impact point")),TEXT("FOV water quantity and approximate crater context are distinct quantities"));
 auto Reject=[&](const FString& Input,const TCHAR* Name){FDataset Invalid=D;FString Why;Check(!ParseDataset(Input,Invalid,Why)&&!Invalid.Available&&!Why.IsEmpty()&&!Invalid.Error.IsEmpty(),Name);};
 Reject(TEXT(""),TEXT("Missing source content disables only the optional dataset with an explicit error"));
 Reject(TEXT("{"),TEXT("Malformed JSON is not substituted with synthetic water evidence"));
 auto Change=[&](const TCHAR* Key,TSharedPtr<FJsonValue> Value){TSharedPtr<FJsonObject> Root;FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Root);if(!Root)return FString();Root->SetField(Key,Value);FString Changed;FJsonSerializer::Serialize(Root.ToSharedRef(),TJsonWriterFactory<>::Create(&Changed));return Changed;};
 Reject(Change(TEXT("nodata"),MakeShared<FJsonValueBoolean>(true)),TEXT("Explicit NoData is unavailable rather than an invented zero-water measurement"));
 Reject(Change(TEXT("water_mass_percent"),MakeShared<FJsonValueNull>()),TEXT("Null water abundance is not treated as a real numeric value"));
 Reject(Change(TEXT("water_units"),MakeShared<FJsonValueString>(TEXT("kg_per_litre"))),TEXT("Wrong physical units cannot enter the percent-by-mass activity"));
 Reject(Change(TEXT("uncertainty_kind"),MakeShared<FJsonValueString>(TEXT("relative_percent"))),TEXT("Relative percent cannot silently replace percentage-point uncertainty"));
 Reject(Change(TEXT("latitude_deg"),MakeShared<FJsonValueNumber>(-94.9)),TEXT("Out-of-range context coordinates are rejected"));
 Reject(Change(TEXT("water_uncertainty_points"),MakeShared<FJsonValueNumber>(-2.9)),TEXT("Negative uncertainty is rejected"));
 Reject(Change(TEXT("water_mass_percent"),MakeShared<FJsonValueNumber>(56.)),TEXT("A substituted abundance cannot silently reuse the fixed published exercise"));
 Reject(Change(TEXT("source_url"),MakeShared<FJsonValueString>(TEXT("https://example.invalid/fake-water"))),TEXT("An unrelated source cannot claim this version of the LCROSS study"));
 Reject(Change(TEXT("kind"),MakeShared<FJsonValueString>(TEXT("raw_spectrum"))),TEXT("Publication facts are never accepted as raw spectral measurements"));
 FState State;Check(IsValidState(State)&&State.Step==0&&!ReadyToCommit(State)&&!State.Recorded,TEXT("Fresh study has no prior decisions or committed report"));
 const FString Before=StateSummary(State);Check(!Commit(State)&&StateSummary(State)==Before,TEXT("A report cannot be committed before the evidence decisions"));
 Check(!RecordAnswer(State,8)&&State.Attempts==0&&State.Step==0,TEXT("Out-of-range answer does not create an attempt or advance the study"));
 Check(!RecordAnswer(State,0)&&State.Step==0&&State.Attempts==1&&!State.Feedback.IsEmpty(),TEXT("Choosing Apollo 12 for the remote Cabeus observation gives feedback without completing a step"));
 Check(RecordAnswer(State,1)&&State.Step==1&&!State.Recorded,TEXT("Identifying the real remote site advances only the optional study"));
 Check(!RecordAnswer(State,0)&&State.Step==1&&State.Feedback.Contains(TEXT("No Apollo sample bag")),TEXT("The illustrative 100 kg comparison cannot become a player measurement"));
 Check(RecordAnswer(State,2)&&State.Step==2,TEXT("Correct interpretation of mass fraction advances to uncertainty reasoning"));
 const auto RangeChoices=Choices(State);Check(RangeChoices.Num()==3&&RangeChoices[0].Contains(TEXT("2.7"))&&RangeChoices[0].Contains(TEXT("8.5")),TEXT("The range choice is computed from the bundled real published values"));
 Check(!RecordAnswer(State,1)&&State.Step==2&&State.Feedback.Contains(TEXT("percentage points")),TEXT("A relative-percent error remains an unfinished decision"));
 Check(RecordAnswer(State,0)&&State.Step==3&&ReadyToCommit(State)&&!State.Recorded,TEXT("Three correct decisions prepare but do not automatically commit a report"));
 Check(Commit(State)&&State.Recorded&&State.Attempts==6&&!ReadyToCommit(State),TEXT("Explicit local report commit uses the actual six answer attempts in this fixture"));
 const FString Committed=StateSummary(State);Check(Committed.Contains(TEXT("6 answer attempts"))&&Committed.Contains(TEXT("no local water detection")),TEXT("Study summary reports session decisions without claiming an in-situ discovery"));
 Check(!Commit(State)&&!RecordAnswer(State,1)&&State.Attempts==6,TEXT("Repeated commit and extra answers cannot duplicate a completed study"));
 FState Bad=State;Bad.Answers[1]=0;Check(!IsValidState(Bad)&&!ReadyToCommit(Bad),TEXT("Corrupted saved answers cannot imply completed evidence decisions"));
 Reset(State);Check(IsValidState(State)&&State.Step==0&&State.Attempts==0&&!State.Recorded&&State.Answers==TArray<int32>({INDEX_NONE,INDEX_NONE,INDEX_NONE})&&State.Feedback.IsEmpty(),TEXT("Restart/reset clears every local water-study answer and committed result"));
 const FString Evidence=EvidenceText(),Limits=ModelLimit();Check(Evidence.Contains(TEXT("instrument field of view"))&&Evidence.Contains(TEXT("estimated observable"))&&Evidence.Contains(TEXT("not a player sample"))&&Limits.Contains(TEXT("separate from the 1969"))&&Limits.Contains(TEXT("not a raw spectrum")),TEXT("Presented evidence keeps observation, inference, explanatory arithmetic and historical scope distinct"));
}
void LunarWater::RunChecks(const UObject* Context){
 FString Report;int32 Failures=0;
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failures;};
 ValidateChecks(Context,Check);
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);const FString Folder=FPaths::ProjectDir()/TEXT("Reports");IFileManager::Get().MakeDirectory(*Folder,true);FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("water-study-checks.txt")));UE_LOG(LogTemp,Display,TEXT("WATER_STUDY_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
}