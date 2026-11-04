#include "LunarFieldOperations.h"
#include "LunarCarryPresentation.h"
#include "LunarGame.h"
#include "LunarSurfaceVehicle.h"
#include "LunarSettings.h"
#include "LunarWaterStudy.h"
#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UnrealClient.h"

namespace {
constexpr float PackageRangeCM=450.f;
constexpr float PackageFacingDot=.94f;
constexpr float ScanChargeRU=.8f;
constexpr float ScanDataMB=.4f;
constexpr int32 PackageCount=3;

int32 RightCircuit(int32 Difficulty,int32 Right){
 static constexpr int32 Order[3][4]={{1,0,INDEX_NONE,INDEX_NONE},{2,0,1,INDEX_NONE},{2,0,3,1}};
 const int32 Level=FMath::Clamp(Difficulty,0,2),Count=Level+2;
 return Right>=0&&Right<Count?Order[Level][Right]:INDEX_NONE;
}
FString CircuitLabel(int32 Circuit){
 static const TCHAR* Labels[]={TEXT("A1 / POWER"),TEXT("B2 / RADIO"),TEXT("C3 / SENSOR"),TEXT("D4 / UTILITY")};
 return Circuit>=0&&Circuit<4?FString(Labels[Circuit]):FString();
}
bool CleanPhoto(const ALunarGameMode* G,const ALunarCharacter* P){
 return !G||!P||P->PhotoCaptureFrames>0||G->ApolloPhotoPending||
  (FScreenshotRequest::IsScreenshotRequested()&&!FScreenshotRequest::ShouldShowUI());
}
bool ScannerCanOperate(const UFieldOperationsComponent* Ops){
 if(!Ops||!Ops->Enabled||!Ops->ScannerUnlocked||!Ops->Prepared||!Ops->Briefed||!Ops->VehicleInspected)return false;
 const auto* G=Ops->Game();const auto* P=Ops->Pilot();
 return G&&P&&P->Camera&&G->IsActive()&&!G->Menu&&!G->Paused&&!G->Journal&&!P->ActivePanel&&
  !G->ApolloTool&&!G->Holding&&!LunarCarryPresentation::IsCellCarried(G)&&!Ops->Riding()&&!CleanPhoto(G,P)&&
  LunarWater::Dataset().Available;
}
bool PackageCanBeObserved(const UFieldOperationsComponent* Ops,int32 Index,FVector& Position,float& Dot,float& DistanceSquared){
 Position=FVector::ZeroVector;Dot=0;DistanceSquared=0;
 if(!Ops||Index<0||Index>=PackageCount||!Ops->SamplePackages.IsValidIndex(Index)||
  ((Ops->CollectedMask|Ops->DeliveredMask)&(1<<Index)))return false;
 const auto* P=Ops->Pilot();auto* Target=Ops->SamplePackages[Index].Get();
 if(!P||!P->Camera||!IsValid(Target)||Target->IsHidden()||!Ops->GetWorld()||Target->GetWorld()!=Ops->GetWorld())return false;
 if(!Target->ActorHasTag(FName(*FString::Printf(TEXT("Field.Package.%d"),Index))))return false;
 Position=Target->GetActorLocation();
 TArray<UStaticMeshComponent*> Parts;Target->GetComponents(Parts);
 for(const auto* Part:Parts)if(Part&&Part->ComponentHasTag(TEXT("work.focus"))){Position=Part->GetComponentLocation();break;}
 const FVector Eye=P->Camera->GetComponentLocation(),Offset=Position-Eye;
 if(Position.ContainsNaN()||Eye.ContainsNaN()||Offset.ContainsNaN())return false;
 DistanceSquared=Offset.SizeSquared();
 if(!FMath::IsFinite(DistanceSquared)||DistanceSquared>FMath::Square(PackageRangeCM)||DistanceSquared<=UE_SMALL_NUMBER)return false;
 const FVector Forward=P->Camera->GetForwardVector();
 if(Forward.ContainsNaN())return false;
 Dot=FVector::DotProduct(Forward,Offset.GetSafeNormal());
 if(!FMath::IsFinite(Dot)||Dot<PackageFacingDot)return false;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(FieldTrainingPackageSight),false,P);
 for(const auto& Support:Ops->SupportActors)if(IsValid(Support.Get()))Query.AddIgnoredActor(Support.Get());
 FHitResult Hit;
 if(Ops->GetWorld()->LineTraceSingleByChannel(Hit,Eye,Position,ECC_Visibility,Query)&&Hit.GetActor()!=Target)return false;
 return true;
}
bool ScanBufferAvailable(const UFieldOperationsComponent* Ops){
 if(!Ops||!FMath::IsFinite(Ops->UtilityCharge)||Ops->UtilityCharge<ScanChargeRU||
  !FMath::IsFinite(Ops->DataStoredMB)||Ops->DataStoredMB<0||!FMath::IsFinite(Ops->DataCapacityMB)||
  Ops->DataCapacityMB<Ops->DataStoredMB)return false;
 // Permit only representation-level rounding at an exact 0.4 MB boundary;
 // clamp the committed float to capacity rather than exceeding the buffer.
 const double Free=double(Ops->DataCapacityMB)-double(Ops->DataStoredMB);
 return Free>=double(ScanDataMB)||FMath::IsNearlyEqual(Free,double(ScanDataMB),1.e-6);
}
}

int32 UFieldOperationsComponent::WirePairs() const{return FMath::Clamp(Difficulty,0,2)+2;}
FString UFieldOperationsComponent::WireLeftLabel(int32 Index) const{return Index>=0&&Index<WirePairs()?CircuitLabel(Index):FString();}
FString UFieldOperationsComponent::WireRightLabel(int32 Index) const{return CircuitLabel(RightCircuit(Difficulty,Index));}
bool UFieldOperationsComponent::ConnectWire(int32 Right){
 const auto* G=Game();
 if(!Enabled||!Prepared||!Briefed||!G||G->Menu||!G->IsActive())return false;
 if(WireComplete){Feedback=TEXT("Training circuit already matched.");return false;}
 if(!WireIsolated){Feedback=TEXT("Isolate the training circuit before matching its terminals.");++Revision;return false;}
 if(WireLeft<0||WireLeft>=WirePairs()||Right<0||Right>=WirePairs()){
  Feedback=TEXT("Choose a labelled left terminal, then its matching right terminal.");++Revision;return false;
 }
 const int32 LeftBit=1<<WireLeft;
 if(WireMask&LeftBit){Feedback=TEXT("That terminal is already connected. Choose an unfinished terminal.");++Revision;return false;}
 if(RightCircuit(Difficulty,Right)!=WireLeft){
  WireMistakes=FMath::Clamp(WireMistakes,0,MAX_int32-1)+1;
  Feedback=FString::Printf(TEXT("Labels do not match: %s / %s. Completed connections are retained."),*WireLeftLabel(WireLeft),*WireRightLabel(Right));
  ++Revision;return false;
 }
 WireMask|=LeftBit;WireLeft=INDEX_NONE;
 const int32 CompleteMask=(1<<WirePairs())-1;
 WireComplete=(WireMask&CompleteMask)==CompleteMask;
 // The owner validates the active native mission stage before applying repair.
 // Matching the puzzle alone must never award main-mission progression here.
 WirePendingCommit=WireComplete;
 Feedback=WireComplete?TEXT("Training circuit matched. Confirm restoration to apply it to the current task."):
  TEXT("Labelled connection recorded. Choose the next unfinished terminal.");
 ++Revision;return true;
}

int32 UFieldOperationsComponent::NearbyPackage(bool RequireObserved) const{
 int32 Best=INDEX_NONE;float BestDot=-1,BestDistanceSquared=TNumericLimits<float>::Max();
 for(int32 I=0;I<PackageCount;++I){
  if(RequireObserved&&!(ScannedMask&(1<<I)))continue;
  FVector Position;float Dot=0,DistanceSquared=0;
  if(!PackageCanBeObserved(this,I,Position,Dot,DistanceSquared))continue;
  if(Dot>BestDot+.001f||(FMath::IsNearlyEqual(Dot,BestDot,.001f)&&DistanceSquared<BestDistanceSquared)){
   Best=I;BestDot=Dot;BestDistanceSquared=DistanceSquared;
  }
 }
 return Best;
}
bool UFieldOperationsComponent::ToggleScanner(){
 if(ScannerActive){CancelScanner();Feedback=TEXT("Inspection magnifier stowed.");++Revision;return true;}
 if(!ScannerCanOperate(this)){
  Feedback=!LunarWater::Dataset().Available?TEXT("Optional inspection unavailable: the local LCROSS source summary is missing or invalid."):
   TEXT("Complete EVA preparation, briefing and vehicle inspection; then stow other tools before using the magnifier.");
  ++Revision;return false;
 }
 auto* P=Pilot();
 if(P->ThirdPerson)P->ToggleSuitView();
 if(P->ThirdPerson){Feedback=TEXT("Use first-person view for the inspection magnifier.");++Revision;return false;}
 ScannerActive=true;Scanning=false;ScanProgress=0;SelectedPackage=INDEX_NONE;
 P->Camera->SetFieldOfView(35.f);
 Feedback=TEXT("Aim at a labelled training package and hold left click to inspect it. This does not detect local water.");
 RefreshWorldVisibility();++Revision;return true;
}
bool UFieldOperationsComponent::BeginScan(){
 if(!ScannerActive||!ScannerCanOperate(this)){if(ScannerActive)CancelScanner();return false;}
 if(Scanning)return false;
 if(!FMath::IsFinite(ScanDuration)||ScanDuration<=0){Feedback=TEXT("Inspection unavailable: invalid gameplay duration.");++Revision;return false;}
 const int32 Target=NearbyPackage(false);
 if(Target==INDEX_NONE){Feedback=TEXT("Face a visible training package within 4.5 m to inspect its label.");++Revision;return false;}
 SelectedPackage=Target;
 if(ScannedMask&(1<<Target)){
  Feedback=TEXT("This training label is already recorded. Use the displayed work binding to collect the package.");++Revision;return false;
 }
 if(!ScanBufferAvailable(this)){
  Feedback=TEXT("Inspection needs 0.8 utility RU and 0.4 MB of free game data storage. No resource was spent.");++Revision;return false;
 }
 Scanning=true;ScanProgress=0;
 Feedback=TEXT("Reading a simulated remote-delivery label — no in-situ composition measurement.");
 ++Revision;return true;
}
void UFieldOperationsComponent::EndScan(){Scanning=false;ScanProgress=0;}
void UFieldOperationsComponent::CancelScanner(){
 const bool OwnedZoom=ScannerActive;
 ScannerActive=false;EndScan();SelectedPackage=INDEX_NONE;
 if(OwnedZoom)if(auto* P=Pilot();P&&P->Camera){
  const float Setting=ULunarSettings::Get()->FieldOfView;
  P->Camera->SetFieldOfView(FMath::IsFinite(Setting)&&Setting>=5&&Setting<=170?Setting:85.f);
 }
 for(const auto& Visual:ScannerVisuals)if(Visual){Visual->SetVisibility(false,true);Visual->SetHiddenInGame(true,true);}
}
void UFieldOperationsComponent::AdvanceScanner(float Delta){
 if(!ScannerActive){RefreshWorldVisibility();return;}
 auto* P=Pilot();
 if(!ScannerCanOperate(this)||!P||P->ThirdPerson){CancelScanner();return;}
 // The magnifier owns this temporary FOV until stowed. No settings are saved.
 P->Camera->SetFieldOfView(35.f);RefreshWorldVisibility();
 if(!Scanning||!FMath::IsFinite(Delta)||Delta<=0)return;
 if(!FMath::IsFinite(ScanProgress)||ScanProgress<0||!FMath::IsFinite(ScanDuration)||ScanDuration<=0){EndScan();return;}
 FVector Position;float Dot=0,DistanceSquared=0;
 if(!PackageCanBeObserved(this,SelectedPackage,Position,Dot,DistanceSquared)){
  EndScan();Feedback=TEXT("Inspection interrupted: keep the package visible, in range and centred.");++Revision;return;
 }
 const int32 Bit=1<<SelectedPackage;
 if(ScannedMask&Bit){EndScan();return;}
 if(!ScanBufferAvailable(this)){
  EndScan();Feedback=TEXT("Inspection interrupted: insufficient utility RU or free data storage. No scan cost was applied.");++Revision;return;
 }
 ScanProgress=FMath::Min(ScanDuration,ScanProgress+Delta);
 if(ScanProgress<ScanDuration)return;
 // Commit a single authored label record atomically. Pickup/inventory is a
 // separate work action; the LCROSS publication is never modified or relabelled.
 if(!SpendCharge(ScanChargeRU)){EndScan();Feedback=TEXT("Inspection could not commit its utility cost.");++Revision;return;}
 DataStoredMB=FMath::Min(DataCapacityMB,DataStoredMB+ScanDataMB);ScannedMask|=Bit;Scanning=false;
 const auto& D=LunarWater::Dataset();
 Feedback=FString::Printf(TEXT("Training label recorded (+0.4 MB; 0.8 utility RU). Remote science reference: %s, %s. This package is not a local water detection."),*D.Site,*D.ObservationDate);
 ++Revision;
}
void UFieldOperationsComponent::RefreshWorldVisibility(){
 const auto* P=Pilot();
 for(const auto& A:SupportActors)if(IsValid(A.Get())){A->SetActorHiddenInGame(!Enabled);A->SetActorEnableCollision(Enabled);}
 if(IsValid(Vehicle.Get())){Vehicle->SetActorHiddenInGame(!Enabled);Vehicle->SetActorEnableCollision(Enabled);}
 const bool Show=ScannerActive&&ScannerCanOperate(this)&&P&&!P->ThirdPerson;
 for(const auto& Visual:ScannerVisuals)if(Visual){Visual->SetVisibility(Show,true);Visual->SetHiddenInGame(!Show,true);}
 for(int32 I=0;I<PackageCount&&I<SamplePackages.Num();++I)if(auto* Package=SamplePackages[I].Get();IsValid(Package)){
  const bool Removed=!Enabled||((CollectedMask|DeliveredMask)&(1<<I))!=0;
  Package->SetActorHiddenInGame(Removed);Package->SetActorEnableCollision(!Removed);
 }
}
