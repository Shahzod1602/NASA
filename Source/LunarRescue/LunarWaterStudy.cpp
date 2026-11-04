#include "LunarWaterStudy.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace {
constexpr int32 Correct[3]={1,2,0};
bool Number(const TSharedPtr<FJsonObject>& Root,const TCHAR* Key,double& Out){return Root->TryGetNumberField(Key,Out)&&FMath::IsFinite(Out);}
bool String(const TSharedPtr<FJsonObject>& Root,const TCHAR* Key,FString& Out){return Root->TryGetStringField(Key,Out)&&!Out.IsEmpty();}
}
bool LunarWater::ParseDataset(const FString& Text,FDataset& Out,FString& Error){
 Out=FDataset();Error.Empty();
 auto Fail=[&](const TCHAR* Why){Error=Why;Out=FDataset();Out.Error=Error;return false;};
 if(Text.IsEmpty()||Text.Len()>65536)return Fail(TEXT("Optional water study unavailable: bundled source record is empty or oversized."));
 TSharedPtr<FJsonObject> Root;const auto Reader=TJsonReaderFactory<>::Create(Text);
 if(!FJsonSerializer::Deserialize(Reader,Root)||!Root.IsValid())return Fail(TEXT("Optional water study unavailable: source JSON could not be read."));
 double Schema=0;FString Kind,Units,UncertaintyKind;bool NoData=false;
 if(!Number(Root,TEXT("schema"),Schema)||Schema!=1||!String(Root,TEXT("kind"),Kind)||Kind!=TEXT("published_science_summary")||!Root->TryGetBoolField(TEXT("nodata"),NoData)||NoData)return Fail(TEXT("Optional water study unavailable: unsupported or missing published data."));
 FDataset D;
 if(!String(Root,TEXT("dataset_id"),D.Id)||D.Id!=TEXT("lcross-cabeus-published-estimate-v1")||!String(Root,TEXT("title"),D.Title)||!String(Root,TEXT("site"),D.Site)||!String(Root,TEXT("observation_date"),D.ObservationDate)||!String(Root,TEXT("coordinate_scope"),D.CoordinateScope)||!String(Root,TEXT("citation"),D.Citation)||!String(Root,TEXT("source_url"),D.SourceURL)||!String(Root,TEXT("doi"),D.DOI))return Fail(TEXT("Optional water study unavailable: source provenance is incomplete."));
 if(!String(Root,TEXT("water_units"),Units)||Units!=TEXT("percent_by_mass")||!String(Root,TEXT("uncertainty_kind"),UncertaintyKind)||UncertaintyKind!=TEXT("reported_plus_minus_percentage_points"))return Fail(TEXT("Optional water study unavailable: units or uncertainty convention are unsupported."));
 if(!Number(Root,TEXT("latitude_deg"),D.Latitude)||!Number(Root,TEXT("longitude_deg"),D.Longitude)||FMath::Abs(D.Latitude)>90||FMath::Abs(D.Longitude)>180)return Fail(TEXT("Optional water study unavailable: crater context coordinates are invalid."));
 if(!Number(Root,TEXT("water_mass_percent"),D.WaterPercent)||!Number(Root,TEXT("water_uncertainty_points"),D.UncertaintyPoints)||D.UncertaintyPoints<0||D.WaterPercent-D.UncertaintyPoints<0||D.WaterPercent+D.UncertaintyPoints>100)return Fail(TEXT("Optional water study unavailable: water estimate or uncertainty is invalid."));
 if(!Number(Root,TEXT("observed_water_kg"),D.ObservedWaterKg)||!Number(Root,TEXT("observed_water_uncertainty_kg"),D.ObservedWaterUncertaintyKg)||D.ObservedWaterKg<=0||D.ObservedWaterUncertaintyKg<0||D.ObservedWaterUncertaintyKg>D.ObservedWaterKg)return Fail(TEXT("Optional water study unavailable: observed-plume quantity is invalid."));
 // This bounded exercise is tied to the cited 2010 result, not a generic dataset
 // editor. A stale or substituted number must not silently disagree with its quiz.
 if(D.ObservationDate!=TEXT("2009-10-09")||D.DOI!=TEXT("10.1126/science.1186986")||D.SourceURL!=TEXT("https://pubmed.ncbi.nlm.nih.gov/20966242/")||!D.Site.Contains(TEXT("Cabeus"))||!D.CoordinateScope.Contains(TEXT("not the LCROSS impact point"))||!FMath::IsNearlyEqual(D.Latitude,-84.9,1.e-8)||!FMath::IsNearlyEqual(D.Longitude,-35.5,1.e-8)||!FMath::IsNearlyEqual(D.WaterPercent,5.6,1.e-8)||!FMath::IsNearlyEqual(D.UncertaintyPoints,2.9,1.e-8)||!FMath::IsNearlyEqual(D.ObservedWaterKg,155.,1.e-8)||!FMath::IsNearlyEqual(D.ObservedWaterUncertaintyKg,12.,1.e-8))return Fail(TEXT("Optional water study unavailable: bundled values do not match this cited exercise version."));
 D.Available=true;Out=MoveTemp(D);return true;
}
const LunarWater::FDataset& LunarWater::Dataset(){
 static const FDataset Cached=[](){FDataset D;FString Text,Error;const FString Path=FPaths::ProjectContentDir()/TEXT("Science/LunarWater/study.json");if(!FFileHelper::LoadFileToString(Text,*Path)){D.Error=TEXT("Optional water study unavailable: bundled LCROSS source record is missing. Main mission remains playable.");return D;}ParseDataset(Text,D,Error);return D;}();
 return Cached;
}
bool LunarWater::IsValidState(const FState& State){
 if(State.Step<0||State.Step>3||State.Attempts<State.Step||State.Answers.Num()!=3||(State.Recorded&&State.Step!=3))return false;
 for(int32 I=0;I<3;++I){if(State.Answers[I]<INDEX_NONE||State.Answers[I]>2)return false;if(I<State.Step&&State.Answers[I]!=Correct[I])return false;if(I>State.Step&&State.Answers[I]!=INDEX_NONE)return false;}
 return true;
}
void LunarWater::Reset(FState& State){State=FState();}
FString LunarWater::Question(const FState& State){
 if(!Dataset().Available)return Dataset().Error;if(!IsValidState(State))return TEXT("Study state is unavailable. Reset this optional exercise.");
 if(State.Recorded)return TEXT("REMOTE STUDY RECORDED");
 switch(State.Step){case 0:return TEXT("1 / 3  Which site does this water evidence describe?");case 1:return TEXT("2 / 3  What does the published mass fraction mean?");case 2:return TEXT("3 / 3  What is 5.6 +/- 2.9 percentage points?");default:return TEXT("Review the source limits, then record your remote-study report.");}
}
TArray<FString> LunarWater::Choices(const FState& State){
 if(!Dataset().Available||!IsValidState(State)||State.Recorded)return {};
 switch(State.Step){case 0:return {TEXT("An Apollo 12 sample collected in Oceanus Procellarum in 1969."),TEXT("The 2009 LCROSS impact plume from Cabeus near the lunar south pole."),TEXT("An ARES sample collected on Mars.")};
 case 1:return {TEXT("A measured 5.6 kg of water inside an Apollo 12 sample bag."),TEXT("Exactly 5.6 percent of the entire Moon is water."),TEXT("An estimated 5.6 kg per 100 kg of site regolith in an illustrative comparison; the result has uncertainty.")};
 case 2:{const auto& D=Dataset();return {FString::Printf(TEXT("%.1f to %.1f percent by mass by arithmetic; this does not guarantee water elsewhere."),D.WaterPercent-D.UncertaintyPoints,D.WaterPercent+D.UncertaintyPoints),TEXT("5.44 to 5.76 percent, treating 2.9 as a relative percent."),TEXT("Exactly 5.6 percent with no uncertainty.")};}default:return {};}
}
bool LunarWater::RecordAnswer(FState& State,int32 Choice){
 if(!Dataset().Available){State.Feedback=Dataset().Error;return false;}
 if(!IsValidState(State)||State.Recorded||State.Step>=3||Choice<0||Choice>=3)return false;
 State.Attempts=FMath::Min(State.Attempts,MAX_int32-1)+1;State.Answers[State.Step]=Choice;
 if(Choice!=Correct[State.Step]){State.Feedback=State.Step==0?TEXT("This modern remote observation concerns Cabeus, not the Apollo 12 landing site or Mars."):State.Step==1?TEXT("Mass percent describes a site-specific inferred ratio. No Apollo sample bag or whole-Moon inventory was measured here."):TEXT("The +/- term is 2.9 percentage points: subtract and add 2.9. Do not turn it into a relative 2.9 percent.");return false;}
 ++State.Step;State.Feedback=State.Step==1?TEXT("Site identified: remote Cabeus observation, 9 October 2009."):State.Step==2?TEXT("The 100 kg comparison explains the published ratio; it is not a sample measured by this player."):TEXT("2.7 to 8.5 percent is the arithmetic expression of the reported +/- value, not a guaranteed deposit or an independently calculated confidence interval.");return true;
}
bool LunarWater::ReadyToCommit(const FState& State){return Dataset().Available&&IsValidState(State)&&State.Step==3&&!State.Recorded;}
bool LunarWater::Commit(FState& State){if(!ReadyToCommit(State))return false;State.Recorded=true;State.Feedback=TEXT("Optional remote-study report recorded locally. Historical cargo and mission resources are unchanged.");return true;}
FString LunarWater::EvidenceText(){
 const auto& D=Dataset();if(!D.Available)return D.Error;
 return FString::Printf(TEXT("REMOTE EVIDENCE / CABEUS / 2009\nNear-infrared absorption and ultraviolet OH emission supported water in LCROSS ejecta. The published analysis estimated %.0f +/- %.0f kg of water vapour plus ice at maximum within the instrument field of view. From the estimated observable excavated regolith, the authors inferred %.1f +/- %.1f percent water by mass at the impact site.\n100 kg illustration: %.1f +/- %.1f kg of water. Arithmetic range: %.1f to %.1f percent. This is not a player sample, a guaranteed deposit, or a result for Apollo 12."),D.ObservedWaterKg,D.ObservedWaterUncertaintyKg,D.WaterPercent,D.UncertaintyPoints,D.WaterPercent,D.UncertaintyPoints,D.WaterPercent-D.UncertaintyPoints,D.WaterPercent+D.UncertaintyPoints);
}
FString LunarWater::StateSummary(const FState& State){
 if(!Dataset().Available)return Dataset().Error;if(!IsValidState(State))return TEXT("Optional LCROSS study: invalid saved state; reset available.");
 if(State.Recorded)return FString::Printf(TEXT("Optional LCROSS report recorded / Cabeus 2009 / 3 evidence decisions / %d answer attempts. Published result reviewed; no local water detection or cargo award."),State.Attempts);
 return FString::Printf(TEXT("Optional LCROSS remote study / %d of 3 evidence decisions / report not recorded."),State.Step);
}
FString LunarWater::ModelLimit(){return TEXT("Modern remote-study exercise, separate from the 1969 Apollo 12 expedition. Published estimates are derived from real LCROSS observations and analysis, not a raw spectrum generated here. The reported +/- is preserved without assigning a new confidence level. Crater context coordinates are approximate and are not an impact-point survey. No water is detected at the player's location; no drilling, extraction, oxygen or historical cargo is awarded. This exercise does not establish the distribution or recoverability of lunar water.");}