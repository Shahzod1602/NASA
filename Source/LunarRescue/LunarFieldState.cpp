#include "LunarFieldOperations.h"
#include "LunarGame.h"
#include "LunarSurfaceVehicle.h"
#include "LunarWaterStudy.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace {
constexpr int32 StateSchema=1,MaximumStateBytes=16*1024;
constexpr TCHAR StateProfile[]=TEXT("field-operations-training-v1");
constexpr TCHAR ScienceProduct[]=TEXT("lcross-cabeus-published-estimate-v1");
constexpr float MaximumMissionCapacity=1800.f;
struct FOperationsSnapshot {
 bool Enabled=false,Prepared=false,Briefed=false,VehicleInspected=false,ScannerUnlocked=false;
 bool WireIsolated=false,WireComplete=false;
 int32 Difficulty=1,PreflightMask=0,WireMask=0,WireMistakes=0,ScannedMask=0,CollectedMask=0,DeliveredMask=0,NativeScienceMask=0;
 float UtilityCharge=100,UtilityCapacity=100,CargoMassKG=0,DataStoredMB=0,DataTransmittedMB=0,DataCapacityMB=4;
 float ElapsedS=0,VehicleDistanceM=0,VehicleTimeS=0,WalkingDistanceM=0,BaseMissionCapacity=0,MissionCapacity=0;
 bool HasVehiclePose=false,DiscardedVehiclePose=false;
 FTransform VehiclePose=FTransform::Identity;
};
FString MissionIdentity(const ALunarGameMode* G){return !G||G->Menu?FString():G->Apollo?TEXT("apollo12"):G->Mars?TEXT("ares"):TEXT("selene");}
int32 Bits3(int32 Mask){return (Mask&1?1:0)+(Mask&2?1:0)+(Mask&4?1:0);}
bool Bounded(float Value,float Min,float Max){return FMath::IsFinite(Value)&&Value>=Min&&Value<=Max;}
bool EqualProfile(float A,float B){return FMath::IsNearlyEqual(A,B,.0001f);}
FOperationsSnapshot CaptureOperations(const UFieldOperationsComponent& O){
 FOperationsSnapshot S;
 S.Enabled=O.Enabled;S.Prepared=O.Prepared;S.Briefed=O.Briefed;S.VehicleInspected=O.VehicleInspected;S.ScannerUnlocked=O.ScannerUnlocked;
 S.WireIsolated=O.WireIsolated;S.WireComplete=O.WireComplete;S.Difficulty=O.Difficulty;S.PreflightMask=O.PreflightMask;
 S.WireMask=O.WireMask;S.WireMistakes=O.WireMistakes;S.ScannedMask=O.ScannedMask;S.CollectedMask=O.CollectedMask;
 S.DeliveredMask=O.DeliveredMask;S.NativeScienceMask=O.NativeScienceMask;S.UtilityCharge=O.UtilityCharge;S.UtilityCapacity=O.UtilityCapacity;
 S.CargoMassKG=O.CargoMassKG;S.DataStoredMB=O.DataStoredMB;S.DataTransmittedMB=O.DataTransmittedMB;S.DataCapacityMB=O.DataCapacityMB;
 S.ElapsedS=O.ElapsedS;S.VehicleDistanceM=O.VehicleDistanceM;S.VehicleTimeS=O.VehicleTimeS;S.WalkingDistanceM=O.WalkingDistanceM;
 S.BaseMissionCapacity=O.BaseMissionCapacity;S.MissionCapacity=O.Game()?O.Game()->OxygenCapacity:0;
 if(IsValid(O.Vehicle.Get())){S.HasVehiclePose=true;S.VehiclePose=O.Vehicle->GetActorTransform();}
 return S;
}
bool ValidSnapshot(const FOperationsSnapshot& S,FString& Reason){
 Reason.Reset();auto Fail=[&](const TCHAR* Message){Reason=Message;return false;};
 if(S.Difficulty<0||S.Difficulty>2)return Fail(TEXT("Invalid difficulty profile."));
 const int32 FullWire=(1<<(S.Difficulty+2))-1;
 if(S.PreflightMask<0||S.PreflightMask>15||S.WireMask<0||(S.WireMask&~FullWire)||S.WireMistakes<0||S.WireMistakes>1000000)
  return Fail(TEXT("Invalid readiness or circuit state."));
 for(int32 Mask:{S.ScannedMask,S.CollectedMask,S.DeliveredMask,S.NativeScienceMask})if(Mask<0||Mask>7)return Fail(TEXT("Invalid package or science mask."));
 if((S.DeliveredMask&~S.CollectedMask)||(S.CollectedMask&~S.ScannedMask))return Fail(TEXT("Delivery must be collected and collection must be inspected."));
 if(S.Prepared!=(S.PreflightMask==15)||S.Briefed&&!S.Prepared||S.VehicleInspected&&!S.Briefed||
  S.ScannerUnlocked&&(!S.Briefed||!S.VehicleInspected))return Fail(TEXT("Readiness prerequisites do not match."));
 if(S.WireComplete!=(S.WireMask==FullWire)||S.WireMask&&!S.WireIsolated||S.WireMask&&!S.Briefed)
  return Fail(TEXT("Circuit completion does not match its isolated labelled connections."));
 if(S.ScannedMask&&!S.ScannerUnlocked)return Fail(TEXT("Recorded training labels require the inspection unlock."));
 static constexpr float Utility[3]={150,100,80},Data[3]={4,1.2f,.8f},Cargo[3]={2,1,.5f},MissionFactors[3]={1.5f,1,.8f};
 const float ExpectedUtility=S.Briefed?Utility[S.Difficulty]:100;
 const float ExpectedData=S.Briefed?Data[S.Difficulty]:4;
 if(!Bounded(S.UtilityCapacity,1,150)||!EqualProfile(S.UtilityCapacity,ExpectedUtility)||!Bounded(S.UtilityCharge,0,S.UtilityCapacity)||
  !Bounded(S.DataCapacityMB,.1f,4)||!EqualProfile(S.DataCapacityMB,ExpectedData)||!Bounded(S.DataStoredMB,0,S.DataCapacityMB)||
  !Bounded(S.DataTransmittedMB,0,4096))return Fail(TEXT("Resource values do not fit their authored profile."));
 const float Carried=.5f*Bits3(S.CollectedMask&~S.DeliveredMask);
 if(!Bounded(S.CargoMassKG,0,Cargo[S.Difficulty])||!EqualProfile(S.CargoMassKG,Carried))return Fail(TEXT("Cargo mass does not match undelivered training packages."));
 if(!Bounded(S.ElapsedS,0,604800)||!Bounded(S.VehicleTimeS,0,604800)||!Bounded(S.VehicleDistanceM,0,1000000)||
  !Bounded(S.WalkingDistanceM,0,1000000))return Fail(TEXT("Invalid field telemetry."));
 if(!Bounded(S.MissionCapacity,1,MaximumMissionCapacity)||!Bounded(S.BaseMissionCapacity,S.Enabled?1.f:0.f,MaximumMissionCapacity))
  return Fail(TEXT("Invalid gameplay mission-resource capacity."));
 if(S.Enabled){
  const float ExpectedMission=S.Briefed?FMath::Min(MaximumMissionCapacity,S.BaseMissionCapacity*MissionFactors[S.Difficulty]):S.BaseMissionCapacity;
  if(!EqualProfile(S.MissionCapacity,ExpectedMission))return Fail(TEXT("Mission-resource capacity does not match its authored difficulty profile."));
 }
 return true;
}
bool SafeJsonShape(const FString& Text){
 if(Text.Len()>MaximumStateBytes||FTCHARToUTF8(*Text).Length()>MaximumStateBytes)return false;
 bool Quoted=false,Escaped=false;int32 Depth=0;
 for(TCHAR C:Text){
  if(!C)return false;
  if(Quoted){if(Escaped)Escaped=false;else if(C=='\\')Escaped=true;else if(C=='"')Quoted=false;continue;}
  if(C=='"'){Quoted=true;continue;}
  if(C=='{'||C=='['){if(++Depth>4)return false;}else if(C=='}'||C==']'){if(--Depth<0)return false;}
 }
 return !Quoted&&Depth==0;
}
// Unreal's string values implement TryGetNumber/TryGetBool conversions. Schema
// fields must have their actual JSON type before any conversion is attempted.
bool ReadNumber(const TSharedPtr<FJsonObject>& J,const TCHAR* Key,double& Out){
 const auto Value=J->TryGetField(Key);return Value.IsValid()&&Value->Type==EJson::Number&&Value->TryGetNumber(Out);
}
bool ReadBool(const TSharedPtr<FJsonObject>& J,const TCHAR* Key,bool& Out){
 const auto Value=J->TryGetField(Key);return Value.IsValid()&&Value->Type==EJson::Boolean&&Value->TryGetBool(Out);
}
bool ReadFloat(const TSharedPtr<FJsonObject>& J,const TCHAR* Key,float& Out,float Min,float Max){
 double N=0;if(!ReadNumber(J,Key,N)||!FMath::IsFinite(N)||N<double(Min)||N>double(Max))return false;
 Out=float(N);return Bounded(Out,Min,Max);
}
bool ReadInt(const TSharedPtr<FJsonObject>& J,const TCHAR* Key,int32& Out,int32 Min,int32 Max){
 double N=0;if(!ReadNumber(J,Key,N)||!FMath::IsFinite(N)||N<Min||N>Max||FMath::FloorToDouble(N)!=N)return false;
 Out=int32(N);return true;
}
bool ReadCoordinates(const TSharedPtr<FJsonObject>& J,const TCHAR* Key,int32 Count,TArray<double>& Out,double Limit){
 const TArray<TSharedPtr<FJsonValue>>* Values=nullptr;
 if(!J->TryGetArrayField(Key,Values)||!Values||Values->Num()!=Count)return false;
 Out.Reset();for(const auto& Value:*Values){double Number=0;if(!Value.IsValid()||Value->Type!=EJson::Number||!Value->TryGetNumber(Number)||!FMath::IsFinite(Number)||FMath::Abs(Number)>Limit)return false;Out.Add(Number);}
 return true;
}
bool ReadVehicle(const TSharedPtr<FJsonObject>& J,FOperationsSnapshot& S){
 bool HasPose=false;if(!ReadBool(J,TEXT("has_vehicle_pose"),HasPose))return false;
 if(!HasPose)return true;
 const TSharedPtr<FJsonObject>* Parked=nullptr;TArray<double> P,Q,Scale;
 // An unusable auxiliary pose does not invalidate historical mission progress.
 if(!J->TryGetObjectField(TEXT("vehicle_pose"),Parked)||!Parked||!Parked->IsValid()||
  !ReadCoordinates(*Parked,TEXT("position_cm"),3,P,10000000)||!ReadCoordinates(*Parked,TEXT("rotation_xyzw"),4,Q,1.001)||
  !ReadCoordinates(*Parked,TEXT("scale"),3,Scale,2)){
  S.DiscardedVehiclePose=true;return true;
 }
 FQuat Rotation(Q[0],Q[1],Q[2],Q[3]);const FVector Scaling(Scale[0],Scale[1],Scale[2]);
 if(!Rotation.IsNormalized()||!Scaling.Equals(FVector::OneVector,.0001)){
  S.DiscardedVehiclePose=true;return true;
 }
 S.VehiclePose=FTransform(Rotation,FVector(P[0],P[1],P[2]),Scaling);S.HasVehiclePose=!S.VehiclePose.ContainsNaN();
 S.DiscardedVehiclePose=!S.HasVehiclePose;return true;
}
bool ReadSnapshot(const TSharedPtr<FJsonObject>& J,FOperationsSnapshot& S){
 if(!ReadBool(J,TEXT("enabled"),S.Enabled)||!ReadBool(J,TEXT("prepared"),S.Prepared)||
  !ReadBool(J,TEXT("briefed"),S.Briefed)||!ReadBool(J,TEXT("vehicle_inspected"),S.VehicleInspected)||
  !ReadBool(J,TEXT("scanner_unlocked"),S.ScannerUnlocked)||!ReadBool(J,TEXT("wire_isolated"),S.WireIsolated)||
  !ReadBool(J,TEXT("wire_complete"),S.WireComplete))return false;
 struct FIntField {const TCHAR* Key;int32* Value;int32 Min,Max;};
 for(const FIntField& F:{FIntField{TEXT("difficulty"),&S.Difficulty,0,2},{TEXT("preflight_mask"),&S.PreflightMask,0,15},
  {TEXT("wire_mask"),&S.WireMask,0,15},{TEXT("wire_mistakes"),&S.WireMistakes,0,1000000},{TEXT("scanned_mask"),&S.ScannedMask,0,7},
  {TEXT("collected_mask"),&S.CollectedMask,0,7},{TEXT("delivered_mask"),&S.DeliveredMask,0,7},{TEXT("native_science_mask"),&S.NativeScienceMask,0,7}})
  if(!ReadInt(J,F.Key,*F.Value,F.Min,F.Max))return false;
 struct FFloatField {const TCHAR* Key;float* Value;float Min,Max;};
 for(const FFloatField& F:{FFloatField{TEXT("utility_charge"),&S.UtilityCharge,0,150},{TEXT("utility_capacity"),&S.UtilityCapacity,1,150},
  {TEXT("cargo_mass_kg"),&S.CargoMassKG,0,2},{TEXT("data_stored_mb"),&S.DataStoredMB,0,4},{TEXT("data_transmitted_mb"),&S.DataTransmittedMB,0,4096},
  {TEXT("data_capacity_mb"),&S.DataCapacityMB,.1f,4},{TEXT("elapsed_s"),&S.ElapsedS,0,604800},{TEXT("vehicle_distance_m"),&S.VehicleDistanceM,0,1000000},
  {TEXT("vehicle_time_s"),&S.VehicleTimeS,0,604800},{TEXT("walking_distance_m"),&S.WalkingDistanceM,0,1000000},
  {TEXT("base_mission_capacity"),&S.BaseMissionCapacity,0,MaximumMissionCapacity},{TEXT("mission_capacity"),&S.MissionCapacity,1,MaximumMissionCapacity}})
  if(!ReadFloat(J,F.Key,*F.Value,F.Min,F.Max))return false;
 return ReadVehicle(J,S);
}
void ApplySnapshot(UFieldOperationsComponent& O,const FOperationsSnapshot& S){
 O.Enabled=S.Enabled;O.Prepared=S.Prepared;O.Briefed=S.Briefed;O.VehicleInspected=S.VehicleInspected;O.ScannerUnlocked=S.ScannerUnlocked;
 O.WireIsolated=S.WireIsolated;O.WireComplete=S.WireComplete;O.Difficulty=S.Difficulty;O.PreflightMask=S.PreflightMask;O.PreflightCursor=S.Prepared?3:0;O.WireMask=S.WireMask;
 O.WireMistakes=S.WireMistakes;O.ScannedMask=S.ScannedMask;O.CollectedMask=S.CollectedMask;O.DeliveredMask=S.DeliveredMask;O.NativeScienceMask=S.NativeScienceMask;
 O.UtilityCharge=S.UtilityCharge;O.UtilityCapacity=S.UtilityCapacity;O.CargoMassKG=.5f*Bits3(S.CollectedMask&~S.DeliveredMask);
 O.DataStoredMB=S.DataStoredMB;O.DataTransmittedMB=S.DataTransmittedMB;O.DataCapacityMB=S.DataCapacityMB;
 O.ElapsedS=S.ElapsedS;O.VehicleDistanceM=S.VehicleDistanceM;O.VehicleTimeS=S.VehicleTimeS;O.WalkingDistanceM=S.WalkingDistanceM;O.BaseMissionCapacity=S.BaseMissionCapacity;
 O.Preparing=false;O.PendingCheck=INDEX_NONE;O.ActionRemaining=0;O.ActionDuration=1;O.WireLeft=INDEX_NONE;O.WirePendingCommit=false;
 O.ScannerActive=false;O.Scanning=false;O.ScanProgress=0;O.ScanDuration=1.2f;O.SelectedPackage=INDEX_NONE;O.LastPackage=INDEX_NONE;
 O.PackagePreviewSeconds=0;O.WaypointIndex=INDEX_NONE;O.HasWalkPosition=false;O.PreviousWalkPosition=FVector::ZeroVector;O.Feedback.Reset();++O.Revision;
}
void ExtensionVisible(UFieldOperationsComponent& O,bool Visible){
 for(const auto& A:O.SupportActors)if(IsValid(A.Get())){A->SetActorHiddenInGame(!Visible);A->SetActorEnableCollision(Visible);}
 for(AActor* A:{O.ResourceBoard.Get(),O.SampleRobot.Get(),static_cast<AActor*>(O.Vehicle.Get())})if(IsValid(A)){A->SetActorHiddenInGame(!Visible);A->SetActorEnableCollision(Visible);}
 for(const auto& A:O.SamplePackages)if(IsValid(A.Get())){A->SetActorHiddenInGame(!Visible);A->SetActorEnableCollision(Visible);}
 if(Visible)O.RefreshWorldVisibility();
}
void AddVector(const TSharedRef<FJsonObject>& J,const TCHAR* Key,std::initializer_list<double> Numbers){
 TArray<TSharedPtr<FJsonValue>> Values;for(double Number:Numbers)Values.Add(MakeShared<FJsonValueNumber>(Number));J->SetArrayField(Key,Values);
}
}

bool UFieldOperationsComponent::ValidateState(FString& Reason) const{
 if(MissionIdentity(Game()).IsEmpty()){Reason=TEXT("Field operations require a surface mission.");return false;}
 if(!Bounded(IdleChargePerSecond,0,1)||!Bounded(DriveChargePerMeter,0,1)||!EqualProfile(TrainingPackageMassKG,.5f)){
  Reason=TEXT("Invalid authored field-operations tuning.");return false;
 }
 return ValidSnapshot(CaptureOperations(*this),Reason);
}
FString UFieldOperationsComponent::ExportState() const{
 if(!Enabled)return FString();
 FString Reason;if(!ValidateState(Reason)){UE_LOG(LogTemp,Warning,TEXT("Field operations export skipped: %s"),*Reason);return FString();}
 const FOperationsSnapshot S=CaptureOperations(*this);const auto J=MakeShared<FJsonObject>();
 J->SetNumberField(TEXT("schema"),StateSchema);J->SetStringField(TEXT("profile"),StateProfile);J->SetStringField(TEXT("science_id"),ScienceProduct);
 J->SetStringField(TEXT("mission_id"),MissionIdentity(Game()));
 J->SetBoolField(TEXT("enabled"),S.Enabled);J->SetBoolField(TEXT("prepared"),S.Prepared);J->SetBoolField(TEXT("briefed"),S.Briefed);
 J->SetBoolField(TEXT("vehicle_inspected"),S.VehicleInspected);J->SetBoolField(TEXT("scanner_unlocked"),S.ScannerUnlocked);
 J->SetBoolField(TEXT("wire_isolated"),S.WireIsolated);J->SetBoolField(TEXT("wire_complete"),S.WireComplete);
 struct FNumberField {const TCHAR* Key;double Value;};
 for(const FNumberField& F:{FNumberField{TEXT("difficulty"),double(S.Difficulty)},{TEXT("preflight_mask"),double(S.PreflightMask)},{TEXT("wire_mask"),double(S.WireMask)},
  {TEXT("wire_mistakes"),double(S.WireMistakes)},{TEXT("scanned_mask"),double(S.ScannedMask)},{TEXT("collected_mask"),double(S.CollectedMask)},{TEXT("delivered_mask"),double(S.DeliveredMask)},
  {TEXT("native_science_mask"),double(S.NativeScienceMask)},{TEXT("utility_charge"),S.UtilityCharge},{TEXT("utility_capacity"),S.UtilityCapacity},
  {TEXT("cargo_mass_kg"),S.CargoMassKG},{TEXT("data_stored_mb"),S.DataStoredMB},{TEXT("data_transmitted_mb"),S.DataTransmittedMB},
  {TEXT("data_capacity_mb"),S.DataCapacityMB},{TEXT("elapsed_s"),S.ElapsedS},{TEXT("vehicle_distance_m"),S.VehicleDistanceM},
  {TEXT("vehicle_time_s"),S.VehicleTimeS},{TEXT("walking_distance_m"),S.WalkingDistanceM},{TEXT("base_mission_capacity"),S.BaseMissionCapacity},
  {TEXT("mission_capacity"),S.MissionCapacity}})J->SetNumberField(F.Key,F.Value);
 bool SavePose=S.HasVehiclePose&&!S.VehiclePose.ContainsNaN()&&S.VehiclePose.GetRotation().IsNormalized()&&S.VehiclePose.GetScale3D().Equals(FVector::OneVector,.0001);
 const FVector P=S.VehiclePose.GetLocation();if(P.GetAbsMax()>10000000)SavePose=false;
 J->SetBoolField(TEXT("has_vehicle_pose"),SavePose);
 if(SavePose){
  const auto Parked=MakeShared<FJsonObject>();const FQuat Q=S.VehiclePose.GetRotation();const FVector Scale=S.VehiclePose.GetScale3D();
  AddVector(Parked,TEXT("position_cm"),{P.X,P.Y,P.Z});AddVector(Parked,TEXT("rotation_xyzw"),{Q.X,Q.Y,Q.Z,Q.W});
  AddVector(Parked,TEXT("scale"),{Scale.X,Scale.Y,Scale.Z});J->SetObjectField(TEXT("vehicle_pose"),Parked);
 }
 FString Json;const auto Writer=TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json);
 if(!FJsonSerializer::Serialize(J,Writer)||!SafeJsonShape(Json))return FString();return Json;
}
bool UFieldOperationsComponent::ImportState(const FString& Json){
 if(!SafeJsonShape(Json))return false;
 if(Json.TrimStartAndEnd().IsEmpty()){
  // A legacy checkpoint is valid without this optional extension. Never alter
  // historical cargo, primary objectives or the main resource on this path.
  CancelScanner();ALunarSurfaceVehicle::Abort(Game());FOperationsSnapshot Empty;
  Empty.BaseMissionCapacity=Game()?Game()->OxygenCapacity:0;Empty.MissionCapacity=Empty.BaseMissionCapacity;
  ApplySnapshot(*this,Empty);ExtensionVisible(*this,false);return true;
 }
 const FString Mission=MissionIdentity(Game());if(Mission.IsEmpty())return false;
 TSharedPtr<FJsonObject> J;if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),J)||!J.IsValid())return false;
 int32 Schema=0;FString Profile,Science,MissionId;
 if(!ReadInt(J,TEXT("schema"),Schema,StateSchema,StateSchema)||!J->TryGetStringField(TEXT("profile"),Profile)||Profile!=StateProfile||
  !J->TryGetStringField(TEXT("science_id"),Science)||Science!=ScienceProduct||!J->TryGetStringField(TEXT("mission_id"),MissionId)||MissionId!=Mission)return false;
 const auto& Dataset=LunarWater::Dataset();if(Dataset.Available&&Dataset.Id!=ScienceProduct)return false;
 FOperationsSnapshot Parsed;FString Reason;
 if(!ReadSnapshot(J,Parsed)||!ValidSnapshot(Parsed,Reason))return false;
 if(!Bounded(IdleChargePerSecond,0,1)||!Bounded(DriveChargePerMeter,0,1)||!EqualProfile(TrainingPackageMassKG,.5f))return false;
 // No live field, camera, actor or resource is changed until the complete core
 // snapshot has passed validation. Invalid optional vehicle poses keep spawn.
 CancelScanner();ALunarSurfaceVehicle::Abort(Game());ApplySnapshot(*this,Parsed);
 if(auto* G=Game();G&&Parsed.Enabled){
  G->OxygenCapacity=Parsed.MissionCapacity;
  G->Oxygen=FMath::IsFinite(G->Oxygen)?FMath::Clamp(G->Oxygen,0.f,G->OxygenCapacity):0.f;
 }
 ExtensionVisible(*this,Enabled);
 if(Parsed.DiscardedVehiclePose)UE_LOG(LogTemp,Warning,TEXT("Field operations: ignored an invalid optional parked pose; retaining the current vehicle spawn."));
 if(Enabled&&Parsed.HasVehiclePose&&IsValid(Vehicle.Get())&&!Vehicle->RestoreParkedPose(Parsed.VehiclePose))
  UE_LOG(LogTemp,Warning,TEXT("Field operations: parked pose is not usable on this terrain; retaining the current vehicle spawn."));
 return true;
}
