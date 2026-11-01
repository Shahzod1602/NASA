#pragma once
#include "CoreMinimal.h"

// Bounded authored presentation math. None of these functions changes collision,
// terrain, mission progress, oxygen, task durations or scientific measurements.
namespace LunarPovMotion {
inline float Ease(float V){if(!FMath::IsFinite(V))return 0;V=FMath::Clamp(V,0.f,1.f);return V*V*V*(V*(V*6-15)+10);}
inline float Ratio(float Numerator,float Denominator){return FMath::IsFinite(Numerator)&&FMath::IsFinite(Denominator)&&Denominator>0?FMath::Clamp(Numerator/Denominator,0.f,1.f):0;}
inline float HandRest(float Pitch,bool TaskHands){return TaskHands||!FMath::IsFinite(Pitch)?0:Ease((-FMath::UnwindDegrees(Pitch)-28.f)/30.f);}
inline bool HandsVisible(float Pitch,bool TaskHands){return TaskHands||HandRest(Pitch,false)<.985f;}
inline FVector ScoopLift(FVector Contact,FVector Carry,float Progress){
 const float T=Ease(Progress);
 // Dig under the surface, then lift with a small inward arc. Ground values and
 // the source sample remain unchanged; this is a tool/payload presentation.
 const float Dig=FMath::Sin(FMath::Clamp(Progress/.28f,0.f,1.f)*PI)*(Progress<.28f?1.f:0.f);
 return FMath::Lerp(Contact,Carry,T)+FVector(0,0,12.f*FMath::Sin(T*PI)-2.8f*Dig);
}
inline FVector CargoApproach(float Progress){
 const float T=Ease(Progress),R=1-T;
 return FVector(-28,-78,78)*(R*R)+FVector(-10,-16,48)*(2*R*T);
}
}
