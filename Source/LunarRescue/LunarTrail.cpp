#include "LunarTrail.h"
#include "LunarGame.h"
#include "Kismet/GameplayStatics.h"

bool LunarTrail::MapUV(FVector P,FVector2D& UV){
 if(P.ContainsNaN()||!FMath::IsFinite(P.X)||!FMath::IsFinite(P.Y)||!FMath::IsFinite(P.Z))return false;
 UV=FVector2D((P.X+60025.366)/104800.,(P.Y+39983.14)/84800.);
 return UV.X>=0&&UV.X<=1&&UV.Y>=0&&UV.Y<=1;
}
void LunarTrail::Append(TArray<FApolloTrailPoint>& Points,FApolloTrailPoint Point,float& Spacing,bool& Simplified){
 if(Points.Num()>=Capacity){
  // Decimate globally only at capacity. Never join across a discarded gap.
  TArray<FApolloTrailPoint> Reduced;Reduced.Reserve(Capacity/2+1);
  bool Gap=false;
  for(int32 I=0;I<Points.Num();++I){Gap|=Points[I].BreakBefore;if((I%2)==0||I==Points.Num()-1){auto Kept=Points[I];Kept.BreakBefore|=Gap;Reduced.Add(Kept);Gap=false;}}
  Points=MoveTemp(Reduced);Spacing=FMath::Min(Spacing*2,2400.f);Simplified=true;
 }
 if(Points.IsEmpty())Point.BreakBefore=true;
 Points.Add(Point);
}
void ALunarGameMode::ResetApolloTrail(bool FromStart){
 ApolloTrail.Reset();ApolloTrailFromStart=FromStart;ApolloTrailSimplified=false;ApolloTrailSpacing=150;ApolloTrailPendingBreak=true;
 ApolloTrailLatestTime=0;
 if(auto* P=UGameplayStatics::GetPlayerPawn(this,0)){
  ApolloTrailLatest=P->GetActorLocation();FVector2D UV;
  if(LunarTrail::MapUV(ApolloTrailLatest,UV)){FApolloTrailPoint Point;Point.Position=ApolloTrailLatest;ApolloTrail.Add(Point);ApolloTrailPendingBreak=!FromStart;}
 }
}
void ALunarGameMode::RecordApolloTrail(FVector Position,float Delta,bool Accepted){
 FVector2D UV;
 if(!Apollo||!ApolloStarted||!IsActive()||Paused)return;
 if(!FMath::IsFinite(Delta)||Delta<=0)return;
 if(!Accepted||!LunarTrail::MapUV(Position,UV)){ApolloTrailPendingBreak=true;return;}
 ApolloTrailLatest=Position;ApolloTrailLatestTime=ApolloElapsed;
 if(ApolloTrail.IsEmpty()||ApolloTrailPendingBreak||FVector::Dist2D(Position,ApolloTrail.Last().Position)>=ApolloTrailSpacing){
  FApolloTrailPoint Point;Point.Position=Position;Point.Elapsed=ApolloElapsed;Point.BreakBefore=ApolloTrailPendingBreak;
  LunarTrail::Append(ApolloTrail,Point,ApolloTrailSpacing,ApolloTrailSimplified);ApolloTrailPendingBreak=false;
 }
}
TArray<FApolloTrailPoint> ALunarGameMode::ApolloTrailSnapshot() const {
 auto Result=ApolloTrail;FVector2D UV;
 if(!ApolloTrailPendingBreak&&LunarTrail::MapUV(ApolloTrailLatest,UV)&&!Result.IsEmpty()&&FVector::Dist2D(Result.Last().Position,ApolloTrailLatest)>.1f){
  FApolloTrailPoint Point;Point.Position=ApolloTrailLatest;Point.Elapsed=ApolloTrailLatestTime;Point.BreakBefore=false;
  float Spacing=ApolloTrailSpacing;bool Simplified=ApolloTrailSimplified;LunarTrail::Append(Result,Point,Spacing,Simplified);
 }
 return Result;
}
FString ALunarGameMode::ApolloTrailSummary() const {
 if(ApolloTrail.IsEmpty())return TEXT("No walking trace in this checkpoint. Recording begins when you resume moving.");
 return FString(ApolloTrailFromStart?TEXT("Recorded from this expedition's start."):TEXT("Partial trace: older checkpoint or earlier walking was not recorded."))+
 TEXT(" Gaps are not connected. The drawn line is sampled; distance above uses movement telemetry.")+
 (ApolloTrailSimplified?TEXT(" Display trace simplified to keep its memory bounded."):TEXT(""));
}
