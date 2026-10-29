#include "LunarGame.h"

namespace {
int32 DebriefTaskBits(uint8 Mask){int32 Count=0;for(int32 I=0;I<3;++I)Count+=(Mask&(1<<I))?1:0;return Count;}
FString DebriefBudgetClock(float Value,float Capacity){
 const float Limit=FMath::IsFinite(Capacity)&&Capacity>0?Capacity:1800.f;
 const int32 Seconds=FMath::FloorToInt(FMath::IsFinite(Value)?FMath::Clamp(Value,0.f,Limit):0.f);
 return FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60);
}
}

TArray<FString> ALunarGameMode::ApolloDebriefLines() const {
 TArray<FString> Lines;
 Lines.Add(FString::Printf(TEXT("Recorded work: %d/3 sample sites documented, %d/3 samples packed and %d/3 Surveyor components packed."),DebriefTaskBits(ApolloPhotos),DebriefTaskBits(SamplesCollected),DebriefTaskBits(ApolloParts)));
 if(ApolloDustCompared)Lines.Add(TEXT("Optional investigation: the exposed/shielded coating comparison was recorded. Its interpretation is in the evidence album."));
 else if(ApolloDustPhotos)Lines.Add(TEXT("Optional investigation: coating photographs were recorded, but the comparison was not completed."));
 else Lines.Add(TEXT("Optional investigation: no coating comparison was recorded. It is not required to return the primary cargo."));
 Lines.Add(ApolloTimed?TEXT("Timed exercise budget remaining: ")+DebriefBudgetClock(Oxygen,OxygenCapacity)+TEXT(". This is a gameplay time budget, not suit endurance."):TEXT("Historical exploration had no exercise deadline. The mission clock and recorded horizontal distance describe this attempt."));
 Lines.Add(TEXT("NASA data in play: LROC elevations shape the ground and work-site heights. Lander detail, small fragments and tool motions are authored presentation."));
 return Lines;
}
FString ALunarGameMode::ApolloRetryAdvice() const {
 const bool CargoComplete=DebriefTaskBits(SamplesCollected)==3&&DebriefTaskBits(ApolloParts)==3;
 if(!CargoComplete)return TEXT("Next attempt: document and pack the required samples and hardware before choosing optional work. Check the cargo list at each site.");
 if(ApolloTimed&&FMath::IsFinite(Oxygen)&&Oxygen<180.f)return TEXT("Next attempt: keep a larger exercise time margin for the return to Intrepid; prioritize required cargo before optional work.");
 if(!ApolloDustCompared)return TEXT("Next attempt: after documenting Surveyor, try the optional exposed/shielded coating comparison before returning to Intrepid.");
 return ApolloTimed?TEXT("Next attempt: repeat with the same settings and compare your recorded time, distance and completed evidence."):TEXT("Next attempt: try the timed exercise mode and compare your recorded time, distance and completed evidence.");
}
