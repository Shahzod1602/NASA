#include "LunarCheckpoint.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

namespace ApolloTasks {
const TCHAR* const Checkout[]={TEXT("checkout.suit"),TEXT("checkout.tools"),TEXT("checkout.route")};
const TCHAR* const Instruments[]={TEXT("alsep.seismometer"),TEXT("alsep.central")};
const TCHAR* const Photos[]={TEXT("photo.12031"),TEXT("photo.12032"),TEXT("photo.12027")};
const TCHAR* const Samples[]={TEXT("sample.12031"),TEXT("sample.12032"),TEXT("sample.12027")};
const TCHAR* const Parts[]={TEXT("surveyor.camera"),TEXT("surveyor.scoop"),TEXT("surveyor.cable")};
const TCHAR* const Sites[]={TEXT("head.12031"),TEXT("bench.12032"),TEXT("sharp.12027")};
const TCHAR* const Arrival=TEXT("surveyor.reached");
const TCHAR* const SurveyorPhoto=TEXT("photo.surveyor3");
const TCHAR* const Stowed=TEXT("intrepid.cargo-stowed");
}

bool UApolloCheckpoint::Has(const TCHAR* Task) const {return CompletedTasks.Contains(FName(Task));}
int32 UApolloCheckpoint::Step() const {
 if(!Has(ApolloTasks::Checkout[2]))return 0;
 if(!Has(ApolloTasks::Instruments[1]))return 1;
 for(const TCHAR* Id:ApolloTasks::Samples)if(!Has(Id))return 2;
 if(!Has(ApolloTasks::Arrival))return 3;
 for(const TCHAR* Id:ApolloTasks::Parts)if(!Has(Id))return 4;
 return Has(ApolloTasks::Stowed)?6:5;
}
bool UApolloCheckpoint::Validate(FString& Reason) const {
 auto Reject=[&](const TCHAR* Why){Reason=Why;return false;};
 if(FormatVersion!=1||MissionId!=TEXT("apollo12-surveyor-legacy")||DataProfile!=TEXT("lroc-apollo12-v1.9")||(Mode!=TEXT("timed-exercise")&&Mode!=TEXT("historical-exploration")))return Reject(TEXT("incompatible"));
 if(!RunId.IsValid()||CompletedTasks.Num()>17||Evidence.Num()>4)return Reject(TEXT("invalid identifiers"));
 if(Position.ContainsNaN()||View.ContainsNaN()||!FMath::IsFinite(Position.X)||!FMath::IsFinite(Position.Y)||!FMath::IsFinite(Position.Z)||Position.X < -59000||Position.X > 44000||Position.Y < -39000||Position.Y > 44000||FMath::Abs(Position.Z)>50000)return Reject(TEXT("invalid position"));
 if(!FMath::IsFinite(Elapsed)||!FMath::IsFinite(Distance)||!FMath::IsFinite(Remaining)||!FMath::IsFinite(AntennaAngle)||Elapsed<0||Elapsed>(Mode==TEXT("timed-exercise")?1830.f:604800.f)||Distance<0||Distance>50000||Remaining<0||Remaining>1800||FMath::Abs(AntennaAngle)>180||FMath::Abs(View.Pitch)>360||FMath::Abs(View.Yaw)>360||FMath::Abs(View.Roll)>360)return Reject(TEXT("invalid telemetry"));
 TSet<FName> Allowed,Seen;
 for(const TCHAR* Id:ApolloTasks::Checkout)Allowed.Add(FName(Id));
 for(const TCHAR* Id:ApolloTasks::Instruments)Allowed.Add(FName(Id));
 for(const TCHAR* Id:ApolloTasks::Photos)Allowed.Add(FName(Id));
 for(const TCHAR* Id:ApolloTasks::Samples)Allowed.Add(FName(Id));
 for(const TCHAR* Id:ApolloTasks::Parts)Allowed.Add(FName(Id));
 Allowed.Add(FName(ApolloTasks::Arrival));Allowed.Add(FName(ApolloTasks::SurveyorPhoto));Allowed.Add(FName(ApolloTasks::Stowed));
 for(FName Id:CompletedTasks){if(!Allowed.Contains(Id)||Seen.Contains(Id))return Reject(TEXT("unknown or duplicate task"));Seen.Add(Id);}
 if((Has(ApolloTasks::Checkout[1])&&!Has(ApolloTasks::Checkout[0]))||(Has(ApolloTasks::Checkout[2])&&!Has(ApolloTasks::Checkout[1])))return Reject(TEXT("checkout prerequisite"));
 for(const TCHAR* Id:ApolloTasks::Instruments)if(Has(Id)&&!Has(ApolloTasks::Checkout[2]))return Reject(TEXT("deployment prerequisite"));
 if(Has(ApolloTasks::Instruments[1])&&!Has(ApolloTasks::Instruments[0]))return Reject(TEXT("seismometer prerequisite"));
 for(int I=0;I<3;++I){
  if(Has(ApolloTasks::Photos[I])&&!Has(ApolloTasks::Instruments[1]))return Reject(TEXT("sample prerequisite"));
  if(Has(ApolloTasks::Samples[I])&&!Has(ApolloTasks::Photos[I]))return Reject(TEXT("context photograph prerequisite"));
  if(Has(ApolloTasks::Arrival)&&!Has(ApolloTasks::Samples[I]))return Reject(TEXT("traverse prerequisite"));
  if(Has(ApolloTasks::Parts[I])&&!Has(ApolloTasks::SurveyorPhoto))return Reject(TEXT("hardware photograph prerequisite"));
  if(Has(ApolloTasks::Stowed)&&!Has(ApolloTasks::Parts[I]))return Reject(TEXT("cargo prerequisite"));
 }
 if(Has(ApolloTasks::SurveyorPhoto)&&!Has(ApolloTasks::Arrival))return Reject(TEXT("Surveyor arrival prerequisite"));
 if(Remaining<=0&&Step()!=6)return Reject(TEXT("expired checkpoint"));
 TSet<FName> PhotoIds;
 for(const FApolloEvidence& Item:Evidence){
  int Index=INDEX_NONE;for(int I=0;I<3;++I)if(Item.PhotoId==FName(ApolloTasks::Photos[I]))Index=I;
  const bool Surveyor=Item.PhotoId==FName(ApolloTasks::SurveyorPhoto);
  if((Index==INDEX_NONE&&!Surveyor)||PhotoIds.Contains(Item.PhotoId)||!CompletedTasks.Contains(Item.PhotoId))return Reject(TEXT("invalid evidence identifier"));
  if(Item.SiteId!=FName(Surveyor?TEXT("surveyor3"):ApolloTasks::Sites[Index]))return Reject(TEXT("evidence site mismatch"));
  if(!FMath::IsFinite(Item.CapturedAt)||Item.CapturedAt<0||Item.CapturedAt>Elapsed+.1f)return Reject(TEXT("invalid evidence time"));
  if(!Item.PhotoPath.IsEmpty()){
   const FString Prefix=FString(TEXT("Apollo12/Photos/"))+RunId.ToString(EGuidFormats::Digits)+TEXT("/");
   if(!Item.PhotoPath.StartsWith(Prefix)||Item.PhotoPath.Contains(TEXT(".."))||Item.PhotoPath.Contains(TEXT("\\"))||!Item.PhotoPath.EndsWith(TEXT(".png"))||Item.PhotoPath.Len()>180)return Reject(TEXT("unsafe evidence path"));
  }
  PhotoIds.Add(Item.PhotoId);
 }
 for(const TCHAR* Id:ApolloTasks::Photos)if(Has(Id)&&!PhotoIds.Contains(FName(Id)))return Reject(TEXT("missing evidence metadata"));
 if(Has(ApolloTasks::SurveyorPhoto)&&!PhotoIds.Contains(FName(ApolloTasks::SurveyorPhoto)))return Reject(TEXT("missing Surveyor metadata"));
 if(!SelectedSite.IsNone()){
  bool Known=false;for(int I=0;I<3;++I)Known|=SelectedSite==FName(ApolloTasks::Sites[I])||SelectedSite==FName(ApolloTasks::Parts[I]);
  if(!Known)return Reject(TEXT("unknown selection"));
 }
 Reason.Reset();return true;
}

namespace {
constexpr uint32 Magic=0x3150434c;
constexpr int32 MaxBytes=512*1024;
bool ReplaceFile(const FString& From,const FString& To){
#if PLATFORM_WINDOWS
 const FString Source=FPaths::ConvertRelativePathToFull(From),Target=FPaths::ConvertRelativePathToFull(To);
 return ::MoveFileExW(*Source,*Target,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
 return IFileManager::Get().Move(*To,*From,true,false,false,true);
#endif
}
UApolloCheckpoint* ReadFile(const FString& Path,EApolloSaveStatus& Status){
 Status=EApolloSaveStatus::Missing;
 const int64 Size=IFileManager::Get().FileSize(*Path);if(Size<0)return nullptr;
 Status=EApolloSaveStatus::Corrupt;if(Size<16||Size>MaxBytes)return nullptr;
 TArray<uint8> Bytes;if(!FFileHelper::LoadFileToArray(Bytes,*Path)||Bytes.Num()<Size)return nullptr;Bytes.SetNum(int32(Size),EAllowShrinking::No);
 FMemoryReader Reader(Bytes);uint32 Tag,Version,Crc;int32 PayloadSize;Reader<<Tag<<Version<<PayloadSize<<Crc;
 if(Reader.IsError()||Tag!=Magic)return nullptr;
 if(Version!=1){Status=EApolloSaveStatus::Incompatible;return nullptr;}
 if(PayloadSize<=0||PayloadSize!=Bytes.Num()-16||FCrc::MemCrc32(Bytes.GetData()+16,PayloadSize)!=Crc)return nullptr;
 TArray<uint8> Payload;Payload.Append(Bytes.GetData()+16,PayloadSize);
 auto* Save=Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Payload));if(!Save)return nullptr;
 FString Reason;if(!Save->Validate(Reason)){if(Reason==TEXT("incompatible"))Status=EApolloSaveStatus::Incompatible;return nullptr;}
 Status=EApolloSaveStatus::Ready;return Save;
}
}
FString ApolloCheckpointStorage::Primary(const FString& Directory){return Directory/TEXT("Apollo12.sav");}
FString ApolloCheckpointStorage::Backup(const FString& Directory){return Directory/TEXT("Apollo12.sav.bak");}
UApolloCheckpoint* ApolloCheckpointStorage::Read(const FString& Directory,EApolloSaveStatus& Status){
 auto* Save=ReadFile(Primary(Directory),Status);if(Save)return Save;
 const EApolloSaveStatus PrimaryStatus=Status;EApolloSaveStatus BackupStatus;
 Save=ReadFile(Backup(Directory),BackupStatus);
 if(Save){Status=EApolloSaveStatus::Recovered;return Save;}
 Status=PrimaryStatus==EApolloSaveStatus::Incompatible||BackupStatus==EApolloSaveStatus::Incompatible?EApolloSaveStatus::Incompatible:PrimaryStatus!=EApolloSaveStatus::Missing||BackupStatus!=EApolloSaveStatus::Missing?EApolloSaveStatus::Corrupt:EApolloSaveStatus::Missing;
 return nullptr;
}
bool ApolloCheckpointStorage::Write(const FString& Directory,UApolloCheckpoint* Snapshot){
 FString Reason;if(!Snapshot||!Snapshot->Validate(Reason))return false;
 TArray<uint8> Payload;if(!UGameplayStatics::SaveGameToMemory(Snapshot,Payload)||Payload.Num()>MaxBytes-16)return false;
 TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);uint32 Tag=Magic,Version=1,Crc=FCrc::MemCrc32(Payload.GetData(),Payload.Num());int32 Count=Payload.Num();Writer<<Tag<<Version<<Count<<Crc;Writer.Serialize(Payload.GetData(),Payload.Num());Writer.Close();
 if(!IFileManager::Get().DirectoryExists(*Directory)&&!IFileManager::Get().MakeDirectory(*Directory,true))return false;
 const FString Main=Primary(Directory),Old=Backup(Directory),Temp=Main+TEXT(".tmp"),OldTemp=Old+TEXT(".tmp");
 if(!FFileHelper::SaveArrayToFile(Bytes,*Temp))return false;
 EApolloSaveStatus Status;
 if(!ReadFile(Temp,Status))return false;
 const bool GoodPrimary=ReadFile(Main,Status)!=nullptr;
 // Never rotate a damaged primary over the last recoverable backup.
 if(GoodPrimary||(!IFileManager::Get().FileExists(*Old)&&IFileManager::Get().FileExists(*Main))){
  if(IFileManager::Get().Copy(*OldTemp,*Main,true,false,false)!=COPY_OK||!ReplaceFile(OldTemp,Old))return false;
 }
 return ReplaceFile(Temp,Main);
}
