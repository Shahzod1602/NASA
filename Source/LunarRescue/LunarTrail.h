#pragma once
#include "CoreMinimal.h"
#include "LunarCheckpoint.h"
namespace LunarTrail {
 constexpr int32 Capacity=1024;
 bool MapUV(FVector Position,FVector2D& UV);
 void Append(TArray<FApolloTrailPoint>& Points,FApolloTrailPoint Point,float& Spacing,bool& Simplified);
}
