#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LunarCheckpoint.generated.h"

// Persistent identifiers describe work, never actor pointers or enum/array indices.
namespace ApolloTasks {
 extern const TCHAR* const Checkout[3];
 extern const TCHAR* const Instruments[2];
 extern const TCHAR* const Photos[3];
 extern const TCHAR* const Samples[3];
 extern const TCHAR* const Parts[3];
 extern const TCHAR* const Sites[3];
 extern const TCHAR* const Arrival;
 extern const TCHAR* const SurveyorPhoto;
 extern const TCHAR* const Stowed;
}

USTRUCT()
struct FApolloEvidence {
 GENERATED_BODY()
 UPROPERTY() FName PhotoId;
 UPROPERTY() FName SiteId;
 // Relative to ProjectSavedDir; empty in non-rendering automation.
 UPROPERTY() FString PhotoPath;
 UPROPERTY() float CapturedAt=0;
};

UCLASS()
class LUNARRESCUE_API UApolloCheckpoint : public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY() int32 FormatVersion=1;
 UPROPERTY() FName MissionId=TEXT("apollo12-surveyor-legacy");
 UPROPERTY() FName DataProfile=TEXT("lroc-apollo12-v1.9");
 UPROPERTY() FName Mode=TEXT("timed-exercise");
 UPROPERTY() FGuid RunId;
 UPROPERTY() int64 SavedAtUtc=0;
 UPROPERTY() TArray<FName> CompletedTasks;
 UPROPERTY() TArray<FApolloEvidence> Evidence;
 UPROPERTY() FVector Position=FVector::ZeroVector;
 UPROPERTY() FRotator View=FRotator::ZeroRotator;
 UPROPERTY() float Elapsed=0;
 UPROPERTY() float Distance=0;
 UPROPERTY() float Remaining=1800;
 UPROPERTY() float AntennaAngle=-82;
 UPROPERTY() FName SelectedSite;
 UPROPERTY() bool LookLearned=false;
 UPROPERTY() bool MoveLearned=false;
 bool Has(const TCHAR* Task) const;
 int32 Step() const;
 bool Validate(FString& Reason) const;
};

enum class EApolloSaveStatus : uint8 { Missing, Ready, Recovered, Incompatible, Corrupt };

namespace ApolloCheckpointStorage {
 UApolloCheckpoint* Read(const FString& Directory,EApolloSaveStatus& Status);
 bool Write(const FString& Directory,UApolloCheckpoint* Snapshot);
 FString Primary(const FString& Directory);
 FString Backup(const FString& Directory);
}
