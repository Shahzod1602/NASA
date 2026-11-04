#pragma once
#include "CoreMinimal.h"
#include "Templates/Function.h"

namespace LunarWater {
// A modern, remote evidence exercise; these are not Apollo 12 field measurements.
struct FDataset {
 bool Available=false;
 FString Error,Id,Title,Site,ObservationDate,CoordinateScope,Citation,SourceURL,DOI;
 double Latitude=0,Longitude=0,WaterPercent=0,UncertaintyPoints=0,ObservedWaterKg=0,ObservedWaterUncertaintyKg=0;
};
struct FState {
 int32 Step=0,Attempts=0;
 TArray<int32> Answers={INDEX_NONE,INDEX_NONE,INDEX_NONE};
 bool Recorded=false;
 FString Feedback;
};
// Cached once, from the bundled text product. No runtime network requests.
const FDataset& Dataset();
bool ParseDataset(const FString& Text,FDataset& Out,FString& Error);
bool IsValidState(const FState& State);
void Reset(FState& State);
FString Question(const FState& State);
TArray<FString> Choices(const FState& State);
bool RecordAnswer(FState& State,int32 Choice);
bool ReadyToCommit(const FState& State);
bool Commit(FState& State);
FString EvidenceText();
FString StateSummary(const FState& State);
FString ModelLimit();
void ValidateChecks(const UObject* Context,TFunctionRef<void(bool,const TCHAR*)> Check);
void RunChecks(const UObject* Context);
}