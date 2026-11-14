#include "LunarFieldOperations.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "Camera/CameraComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "Misc/ScopeExit.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include <limits>

namespace {
FString SerializeFixture(const TSharedPtr<FJsonObject>& J){
 FString Text;if(J.IsValid())FJsonSerializer::Serialize(J.ToSharedRef(),TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>::Create(&Text));return Text;
}
TSharedPtr<FJsonObject> ParseFixture(const FString& Text){TSharedPtr<FJsonObject> J;FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),J);return J;}
}

void UFieldOperationsComponent::ValidateChecks(TFunctionRef<void(bool,const TCHAR*)> Check){
 auto* G=Game();auto* P=Pilot();
 Check(G&&!G->Menu&&P&&P->Camera,TEXT("Field model checks have a surface GameMode and pilot"));
 if(!G||G->Menu||!P||!P->Camera)return;
 // Isolate model/state checks from the live component and its vehicle, world
 // props, UI and checkpoint callbacks. No BuildWorld or panel is requested.
 auto* F=NewObject<UFieldOperationsComponent>(G);G->AddInstanceComponent(F);F->RegisterComponent();
 const auto PreviousOperations=G->FieldOperations;
 const ELunarStage PreviousStage=G->Stage;const bool PreviousPause=G->Paused,PreviousJournal=G->Journal,PreviousHolding=G->Holding;
 const float PreviousOxygen=G->Oxygen,PreviousCapacity=G->OxygenCapacity,PreviousFov=P->Camera->FieldOfView;
 const int32 PreviousRepair=G->RepairStep;TArray<TWeakObjectPtr<AActor>> FixtureActors;
 G->FieldOperations=F;
 ON_SCOPE_EXIT {
  F->CancelScanner();G->FieldOperations=PreviousOperations;G->Stage=PreviousStage;G->Paused=PreviousPause;G->Journal=PreviousJournal;G->Holding=PreviousHolding;
  G->Oxygen=PreviousOxygen;G->OxygenCapacity=PreviousCapacity;G->RepairStep=PreviousRepair;P->Camera->SetFieldOfView(PreviousFov);
  for(const auto& A:FixtureActors)if(A.IsValid())A->Destroy();G->RemoveInstanceComponent(F);F->DestroyComponent();
 };
 Check(F->Game()==G,TEXT("Unrendered fixture component is owned by the current mission"));
 if(F->Game()!=G)return;
 const float Base=FMath::IsFinite(PreviousCapacity)?FMath::Clamp(PreviousCapacity,1.f,1800.f):300.f;
 const ELunarStage ActiveStage=G->Apollo?ELunarStage::ApolloCheckout:ELunarStage::RestorePower;
 G->Paused=G->Journal=G->Holding=false;G->Stage=ELunarStage::Briefing;G->OxygenCapacity=Base;G->Oxygen=FMath::Min(73.f,Base);
 F->Enabled=F->Preparing=true;F->BaseMissionCapacity=Base;F->DataCapacityMB=4;
 const float PreflightOxygen=G->Oxygen;
 Check(!F->ContinuePreflight()&&F->PreflightCursor==0,TEXT("Next cannot skip an unfinished system"));
 Check(!F->BeginCheck(1)&&F->PendingCheck==INDEX_NONE&&F->PreflightMask==0,TEXT("EVA checks reject skipping the first unfinished system"));
 Check(F->BeginCheck(0)&&F->PendingCheck==0,TEXT("The first ordered EVA check starts without opening a viewport panel"));
 F->Advance(0);F->Advance(-.25f);F->Advance(std::numeric_limits<float>::quiet_NaN());
 Check(FMath::IsNearlyEqual(F->ActionRemaining,1.f)&&F->PreflightMask==0,TEXT("Zero negative and nonfinite preflight deltas cannot complete a check"));
 F->Advance(.4f);
 Check(FMath::IsNearlyEqual(F->ActionRemaining,.6f)&&F->PreflightMask==0&&FMath::IsNearlyEqual(F->UtilityCharge,100.f),TEXT("Partial EVA progress does not prematurely charge or commit a check"));
 const float PausedRemaining=F->ActionRemaining;G->Paused=true;F->Advance(2.f);
 Check(F->ActionRemaining==PausedRemaining&&F->PreflightMask==0&&G->Oxygen==PreflightOxygen,TEXT("Paused component advancement freezes the active EVA check and native resource"));
 G->Paused=false;F->Advance(.61f);
 Check(F->PreflightMask==1&&F->PendingCheck==INDEX_NONE&&!F->Prepared&&FMath::IsNearlyEqual(F->UtilityCharge,99.8f,.001f),TEXT("A completed EVA check commits one bit and one authored cost"));
 for(int32 Step=1;Step<4;++Step){
  Check(!F->BeginCheck(Step)&&F->ContinuePreflight()&&F->PreflightCursor==Step,TEXT("Next is required after a completed system and advances exactly one step"));
  Check(!F->BeginCheck(Step-1),*FString::Printf(TEXT("EVA check %d cannot repeat an already committed system"),Step+1));
  const bool Began=F->BeginCheck(Step);F->Advance(1.01f);
  Check(Began&&F->PreflightMask==((1<<(Step+1))-1)&&F->Prepared==(Step==3),*FString::Printf(TEXT("Ordered EVA system %d commits before the next unlock"),Step+1));
 }
 Check(F->Prepared&&F->PreflightMask==15&&FMath::IsNearlyEqual(F->UtilityCharge,99.2f,.001f)&&!F->BeginCheck(3),TEXT("Four completed checks unlock readiness once without duplicate spending"));
 Check(G->Stage==ELunarStage::Briefing&&G->Oxygen==PreflightOxygen&&G->OxygenCapacity==Base,TEXT("Component EVA completion does not start or alter the native mission"));

 F->UtilityCapacity=100;F->UtilityCharge=10;
 Check(F->SpendCharge(0)&&F->UtilityCharge==10,TEXT("A zero utility cost is an exact no-op"));
 Check(!F->SpendCharge(-1)&&!F->SpendCharge(std::numeric_limits<float>::quiet_NaN())&&!F->SpendCharge(std::numeric_limits<float>::infinity())&&F->UtilityCharge==10,TEXT("Negative NaN and infinite utility costs reject without mutation"));
 Check(F->SpendCharge(10)&&F->UtilityCharge==0&&!F->SpendCharge(.01f)&&F->UtilityCharge==0,TEXT("An exact remaining utility cost reaches zero and cannot overspend"));
 F->UtilityCharge=1;const bool Spent=F->SpendCharge(.8f);const float AfterSpend=F->UtilityCharge;
 Check(Spent&&FMath::IsNearlyEqual(AfterSpend,.2f,1.e-6f)&&!F->SpendCharge(.8f)&&F->UtilityCharge==AfterSpend,TEXT("A second unaffordable inspection cost leaves the remaining charge intact"));

 G->Stage=ActiveStage;G->RepairStep=1;F->Preparing=false;F->Briefed=true;F->UtilityCharge=10;
 F->VehicleDistanceM=F->VehicleTimeS=0;F->RecordVehicleTravel(10,2);
 Check(FMath::IsNearlyEqual(F->UtilityCharge,9.88f,.001f)&&F->VehicleDistanceM==10&&F->VehicleTimeS==2,TEXT("Recorded drive travel charges the configured RU per metre and records actual supplied units"));
 const float DriveCharge=F->UtilityCharge;
 F->RecordVehicleTravel(-1,1);F->RecordVehicleTravel(1,-1);F->RecordVehicleTravel(101,1);F->RecordVehicleTravel(1,2.1f);F->RecordVehicleTravel(std::numeric_limits<float>::quiet_NaN(),1);
 Check(F->UtilityCharge==DriveCharge&&F->VehicleDistanceM==10&&F->VehicleTimeS==2,TEXT("Invalid or discontinuous vehicle travel inputs leave charge and telemetry unchanged"));
 G->Paused=true;F->RecordVehicleTravel(10,1);G->Paused=false;F->RecordVehicleTravel(0,1);
 Check(F->UtilityCharge==DriveCharge&&F->VehicleDistanceM==10&&F->VehicleTimeS==2,TEXT("Paused or stationary drive records do not invent travel time"));
 F->UtilityCharge=10;F->ElapsedS=0;F->Advance(2);
 Check(F->ElapsedS==2&&FMath::IsNearlyEqual(F->UtilityCharge,9.97f,.001f),TEXT("Active briefed advancement uses the configured idle cost and seconds"));
 const float IdleCharge=F->UtilityCharge,Elapsed=F->ElapsedS;
 F->Advance(0);F->Advance(-1);F->Advance(std::numeric_limits<float>::quiet_NaN());G->Paused=true;F->Advance(3);G->Paused=false;G->Journal=true;F->Advance(3);G->Journal=false;
 Check(F->ElapsedS==Elapsed&&F->UtilityCharge==IdleCharge,TEXT("Invalid deltas pause and journal freeze field resource and elapsed state"));
 G->Stage=ELunarStage::Won;F->Advance(5);
 Check(F->ElapsedS==Elapsed&&F->UtilityCharge==IdleCharge,TEXT("A finished mission does not continue field resource consumption"));

 auto Ready=[&](int32 ProfileDifficulty){
  F->CancelScanner();F->Enabled=true;F->Preparing=false;F->Prepared=F->Briefed=F->VehicleInspected=F->ScannerUnlocked=true;F->PreflightMask=15;F->Difficulty=ProfileDifficulty;
  F->WireIsolated=F->WireComplete=F->WirePendingCommit=false;F->WireMask=F->WireMistakes=0;F->WireLeft=INDEX_NONE;
  F->ScannedMask=F->CollectedMask=F->DeliveredMask=F->NativeScienceMask=0;F->CargoMassKG=F->DataStoredMB=F->DataTransmittedMB=0;
  F->ElapsedS=F->VehicleDistanceM=F->VehicleTimeS=F->WalkingDistanceM=0;F->PendingCheck=INDEX_NONE;F->ActionRemaining=0;
  F->BaseMissionCapacity=Base;const float Capacities[]={150,100,80},Storage[]={4,1.2f,.8f},Factors[]={1.5f,1,.8f};
  F->UtilityCapacity=F->UtilityCharge=Capacities[ProfileDifficulty];F->DataCapacityMB=Storage[ProfileDifficulty];
  G->Stage=ActiveStage;G->Paused=G->Journal=G->Holding=false;G->RepairStep=1;G->OxygenCapacity=FMath::Min(1800.f,Base*Factors[ProfileDifficulty]);G->Oxygen=FMath::Min(73.f,G->OxygenCapacity);
 };
 static constexpr int32 RightOrder[3][4]={{1,0,-1,-1},{2,0,1,-1},{2,0,3,1}};
 const TCHAR* Codes[]={TEXT("A1"),TEXT("B2"),TEXT("C3"),TEXT("D4")};
 for(int32 ProfileDifficulty=0;ProfileDifficulty<3;++ProfileDifficulty){
  Ready(ProfileDifficulty);const int32 Pairs=ProfileDifficulty+2;bool Labels=F->WirePairs()==Pairs;
  for(int32 I=0;I<Pairs;++I)Labels=Labels&&F->WireLeftLabel(I).StartsWith(Codes[I])&&F->WireRightLabel(I).StartsWith(Codes[RightOrder[ProfileDifficulty][I]]);
  Check(Labels&&F->WireLeftLabel(-1).IsEmpty()&&F->WireRightLabel(Pairs).IsEmpty(),*FString::Printf(TEXT("Difficulty %d exposes unique labelled ports in the required permutation"),ProfileDifficulty));
  auto RightFor=[&](int32 Left)->int32{for(int32 I=0;I<Pairs;++I)if(RightOrder[ProfileDifficulty][I]==Left)return I;return INDEX_NONE;};
  F->WireLeft=0;Check(!F->ConnectWire(RightFor(0))&&F->WireMask==0&&F->WireMistakes==0,*FString::Printf(TEXT("Difficulty %d requires isolation before matching"),ProfileDifficulty));
  F->WireIsolated=true;Check(!F->ConnectWire((RightFor(0)+1)%Pairs)&&F->WireMask==0&&F->WireMistakes==1,*FString::Printf(TEXT("Difficulty %d records a wrong match without erasing connections"),ProfileDifficulty));
  Check(!F->ConnectWire(-1)&&!F->ConnectWire(Pairs)&&F->WireMistakes==1,*FString::Printf(TEXT("Difficulty %d rejects invalid port indices without a fabricated attempt"),ProfileDifficulty));
  for(int32 Left=0;Left<Pairs;++Left){
   F->WireLeft=Left;const bool Connected=F->ConnectWire(RightFor(Left));
   Check(Connected&&F->WireMask==((1<<(Left+1))-1)&&F->WireComplete==(Left==Pairs-1)&&F->WirePendingCommit==(Left==Pairs-1),
    *FString::Printf(TEXT("Difficulty %d connection %d updates only the required circuit bits"),ProfileDifficulty,Left+1));
   if(Left==0){F->WireLeft=0;Check(!F->ConnectWire(RightFor(0))&&F->WireMask==1&&F->WireMistakes==1,*FString::Printf(TEXT("Difficulty %d cannot commit the same terminal twice"),ProfileDifficulty));}
  }
  Check(G->Stage==ActiveStage&&G->RepairStep==1&&F->WireComplete&&F->WirePendingCommit&&!F->ConnectWire(0),*FString::Printf(TEXT("Difficulty %d completed puzzle awaits native-stage validation without advancing it"),ProfileDifficulty));
 }

 for(int32 ProfileDifficulty=0;ProfileDifficulty<3;++ProfileDifficulty){
  Ready(ProfileDifficulty);F->UtilityCharge-=1.6f;F->ScannedMask=F->CollectedMask=1;F->CargoMassKG=.5f;F->DataStoredMB=.4f;
  F->PendingCheck=5;F->ActionRemaining=.7f;F->WireLeft=0;F->WirePendingCommit=true;F->Scanning=true;F->ScanProgress=.6f;F->SelectedPackage=F->LastPackage=0;F->WaypointIndex=2;F->PackagePreviewSeconds=1;
  FString Reason;const FString Saved=F->ExportState();
  Check(F->ValidateState(Reason)&&!Saved.IsEmpty()&&FTCHARToUTF8(*Saved).Length()<=16384,*FString::Printf(TEXT("Difficulty %d exports a bounded valid committed checkpoint"),ProfileDifficulty));
  Check(!Saved.Contains(TEXT("pending_check"))&&!Saved.Contains(TEXT("scan_progress"))&&!Saved.Contains(TEXT("wire_left"))&&!Saved.Contains(TEXT("feedback")),*FString::Printf(TEXT("Difficulty %d does not persist transient UI scanner or action fields"),ProfileDifficulty));
  F->UtilityCharge=0;F->ScannedMask=F->CollectedMask=0;F->CargoMassKG=F->DataStoredMB=0;G->Oxygen=G->OxygenCapacity+10;
  const bool Loaded=F->ImportState(Saved);
  Check(Loaded&&F->ExportState()==Saved&&G->Oxygen==G->OxygenCapacity,*FString::Printf(TEXT("Difficulty %d restores committed resources and clamps native remaining to its validated capacity"),ProfileDifficulty));
  Check(F->PendingCheck==INDEX_NONE&&F->ActionRemaining==0&&F->WireLeft==INDEX_NONE&&!F->WirePendingCommit&&!F->ScannerActive&&!F->Scanning&&F->ScanProgress==0&&F->SelectedPackage==INDEX_NONE&&F->LastPackage==INDEX_NONE&&F->WaypointIndex==INDEX_NONE&&F->PackagePreviewSeconds==0,
   *FString::Printf(TEXT("Difficulty %d import clears transient scanner UI and operation state"),ProfileDifficulty));
 }
 Ready(1);F->ScannedMask=F->CollectedMask=1;F->CargoMassKG=.5f;F->DataStoredMB=.4f;
 const FString Baseline=F->ExportState();Check(!Baseline.IsEmpty(),TEXT("Transactional corruption fixtures begin from a valid serialized checkpoint"));if(Baseline.IsEmpty())return;
 F->PendingCheck=5;F->ActionRemaining=.7f;F->WireLeft=0;const float NativeOxygen=G->Oxygen,NativeCapacity=G->OxygenCapacity;
 auto RejectText=[&](const FString& Text,const TCHAR* Label){
  // Each adversarial input starts independently. A mistakenly accepted case
  // must fail itself without changing the meaning of every later rejection.
  const bool Seeded=F->ImportState(Baseline);
  F->PendingCheck=5;F->ActionRemaining=.7f;F->WireLeft=0;G->Oxygen=NativeOxygen;G->OxygenCapacity=NativeCapacity;
  const bool Rejected=!F->ImportState(Text);
  Check(Seeded&&Rejected&&F->ExportState()==Baseline&&G->Oxygen==NativeOxygen&&G->OxygenCapacity==NativeCapacity&&F->PendingCheck==5&&F->ActionRemaining==.7f&&F->WireLeft==0,Label);
 };
 auto RejectNumber=[&](const TCHAR* Key,double Number,const TCHAR* Label){auto J=ParseFixture(Baseline);J->SetNumberField(Key,Number);RejectText(SerializeFixture(J),Label);};
 auto RejectString=[&](const TCHAR* Key,const TCHAR* Text,const TCHAR* Label){auto J=ParseFixture(Baseline);J->SetStringField(Key,Text);RejectText(SerializeFixture(J),Label);};
 auto RejectBool=[&](const TCHAR* Key,bool Value,const TCHAR* Label){auto J=ParseFixture(Baseline);J->SetBoolField(Key,Value);RejectText(SerializeFixture(J),Label);};
 RejectText(TEXT("{"),TEXT("Malformed JSON rejects transactionally"));
 RejectText(FString::ChrN(17000,' '),TEXT("Oversized checkpoint text rejects transactionally"));
 RejectText(FString::ChrN(9000,TCHAR(0x03a9)),TEXT("The 16 KB limit counts UTF-8 bytes rather than only TCHAR length"));
 RejectText(Baseline.LeftChop(1)+TEXT(",\"extra\":[[[[0]]]]}"),TEXT("Deeply nested extra JSON rejects before live-state mutation"));
 RejectNumber(TEXT("schema"),2,TEXT("Unknown checkpoint schema rejects transactionally"));
 RejectString(TEXT("profile"),TEXT("unsupported-profile"),TEXT("An unsupported authored resource profile rejects transactionally"));
 RejectString(TEXT("science_id"),TEXT("not-lcross"),TEXT("A mismatched science-product identity rejects transactionally"));
 RejectString(TEXT("mission_id"),G->Apollo?TEXT("ares"):TEXT("apollo12"),TEXT("A checkpoint for another mission rejects transactionally"));
 {auto J=ParseFixture(Baseline);J->RemoveField(TEXT("utility_charge"));RejectText(SerializeFixture(J),TEXT("Missing required state rejects rather than applying partial defaults"));}
 RejectString(TEXT("utility_charge"),TEXT("100"),TEXT("A numeric string does not bypass resource type validation"));
 RejectString(TEXT("schema"),TEXT("1"),TEXT("A quoted schema number is rejected without coercion"));
 RejectString(TEXT("difficulty"),TEXT("1"),TEXT("A quoted difficulty number is rejected without coercion"));
 RejectString(TEXT("enabled"),TEXT("true"),TEXT("A string cannot masquerade as the extension Boolean flag"));
 RejectString(TEXT("prepared"),TEXT("true"),TEXT("A string cannot masquerade as readiness Boolean state"));
 RejectNumber(TEXT("enabled"),1,TEXT("A number cannot masquerade as the extension Boolean flag"));
 RejectNumber(TEXT("utility_charge"),-1,TEXT("Negative stored utility rejects transactionally"));
 RejectNumber(TEXT("utility_charge"),151,TEXT("Charge above the profile capacity rejects transactionally"));
 RejectNumber(TEXT("utility_capacity"),101,TEXT("A forged utility-capacity profile rejects transactionally"));
 RejectNumber(TEXT("data_capacity_mb"),4,TEXT("A forged data-capacity profile rejects transactionally"));
 RejectNumber(TEXT("data_stored_mb"),2,TEXT("Stored data above capacity rejects transactionally"));
 RejectNumber(TEXT("data_transmitted_mb"),-1,TEXT("Negative transmitted data rejects transactionally"));
 RejectNumber(TEXT("cargo_mass_kg"),1,TEXT("Cargo mass inconsistent with collected undelivered bits rejects transactionally"));
 RejectNumber(TEXT("scanned_mask"),0,TEXT("Collection without prior inspection rejects transactionally"));
 RejectNumber(TEXT("delivered_mask"),2,TEXT("Delivery outside the collected set rejects transactionally"));
 RejectNumber(TEXT("collected_mask"),8,TEXT("Package masks cannot contain an unknown fourth cartridge"));
 RejectNumber(TEXT("native_science_mask"),8,TEXT("Semantic native-science flags remain a three-bit set"));
 RejectNumber(TEXT("preflight_mask"),14,TEXT("Prepared state without four committed checks rejects transactionally"));
 RejectBool(TEXT("vehicle_inspected"),false,TEXT("Scanner unlock without vehicle inspection rejects transactionally"));
 RejectNumber(TEXT("difficulty"),3,TEXT("Unknown difficulty rejects transactionally"));
 RejectNumber(TEXT("difficulty"),1.5,TEXT("Fractional difficulty cannot truncate into a valid profile"));
 RejectBool(TEXT("wire_complete"),true,TEXT("Circuit completion without all matching bits rejects transactionally"));
 RejectNumber(TEXT("elapsed_s"),604801,TEXT("Unbounded elapsed telemetry rejects transactionally"));
 RejectNumber(TEXT("walking_distance_m"),-1,TEXT("Negative travel telemetry rejects transactionally"));
 RejectNumber(TEXT("mission_capacity"),1801,TEXT("Native checkpoint-compatible capacity is capped at 1800 gameplay seconds"));
 RejectNumber(TEXT("mission_capacity"),NativeCapacity*.9,TEXT("Difficulty and effective mission capacity must agree"));
 RejectNumber(TEXT("base_mission_capacity"),0,TEXT("Enabled operations cannot restore a missing base mission capacity"));
 {auto J=ParseFixture(Baseline);const auto BadPose=MakeShared<FJsonObject>();J->SetBoolField(TEXT("has_vehicle_pose"),true);J->SetObjectField(TEXT("vehicle_pose"),BadPose);
  Check(F->ImportState(SerializeFixture(J))&&F->ExportState()==Baseline,TEXT("An unusable auxiliary parked-pose record does not discard valid core training state"));}
 Ready(1);F->ScannedMask=7;F->CollectedMask=3;F->DeliveredMask=1;F->CargoMassKG=.5f;F->DataStoredMB=.4f;F->DataTransmittedMB=.8f;
 FString Reason;Check(F->ValidateState(Reason),TEXT("Delivered package history remains recorded while derived carried mass counts only undelivered cargo"));
 F->CargoMassKG=1;Check(!F->ValidateState(Reason),TEXT("Live-state validation catches derived-cargo drift before saving"));F->CargoMassKG=.5f;
 F->UtilityCharge=std::numeric_limits<float>::quiet_NaN();Check(!F->ValidateState(Reason)&&F->ExportState().IsEmpty(),TEXT("Nonfinite live resources cannot be exported as a checkpoint"));
 Ready(1);auto* Marker=G->GetWorld()->SpawnActor<AActor>();
 Check(IsValid(Marker),TEXT("A geometry-free fixture actor is available for hidden-extension state checks"));
 if(Marker){FixtureActors.Add(Marker);Marker->Tags.Add(TEXT("Field.Package.0"));F->SamplePackages.Add(Marker);F->SupportActors.Add(Marker);Marker->SetActorHiddenInGame(false);}
 const float LegacyOxygen=G->Oxygen,LegacyCapacity=G->OxygenCapacity;
 Check(F->ImportState(FString())&&!F->Enabled&&!F->Prepared&&F->ScannedMask==0&&F->CollectedMask==0&&G->Oxygen==LegacyOxygen&&G->OxygenCapacity==LegacyCapacity,
  TEXT("An empty legacy blob disables optional operations while preserving the native mission resource"));
 F->RefreshWorldVisibility();Check(Marker&&Marker->IsHidden(),TEXT("Disabled legacy operations keep optional package actors hidden after visibility refresh"));
 Check(F->ExportState().IsEmpty(),TEXT("A disabled legacy extension does not fabricate a modern progress blob"));
 UE_LOG(LogTemp,Display,TEXT("SKIP Field model fixtures do not exercise real vehicle meshes, terrain traversal, scanner sight rays or human playtesting; separate world/vehicle/rendered checks are required."));
}
