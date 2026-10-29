#include "LunarGame.h"
#include "LunarCargoFinale.h"
#include "LunarApolloText.h"
#include "LunarSettings.h"
#include "LunarLoading.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace {
bool SaveTest(){return FParse::Param(FCommandLine::Get(),TEXT("LoadingTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveWriteTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveReadTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveMenuReview"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveProfile"));}
bool Automation(){
 if(ULunarSettings::IsAutomation())return true;
 const TCHAR* Flags[]={TEXT("ApolloTest"),TEXT("ApolloReview"),TEXT("ApolloWalkTest"),TEXT("SuitTest"),TEXT("SuitReview"),TEXT("LunarTest"),TEXT("LunarMenuTest"),TEXT("LunarReview"),TEXT("LunarMovementTest"),TEXT("LunarWalkthroughTest"),TEXT("LunarExpeditionReview")};
 for(const TCHAR* Flag:Flags)if(FParse::Param(FCommandLine::Get(),Flag))return true;
 return SaveTest();
}
}

FString ALunarGameMode::ApolloSaveDirectory() const {
 if(FParse::Param(FCommandLine::Get(),TEXT("LoadingTest")))return FPaths::ProjectSavedDir()/TEXT("AutomatedTests/Loading");
 if(SaveTest())return FPaths::ProjectSavedDir()/TEXT("AutomatedTests")/(FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveProfile"))?TEXT("ApolloProfile"):FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveTest"))?TEXT("ApolloCheckpoint"):TEXT("ApolloCrossProcess"));
 return FPaths::ProjectSavedDir()/TEXT("SaveGames/Apollo12");
}
bool ALunarGameMode::ApolloSaveEnabled() const {return !Automation()||SaveTest();}
void ALunarGameMode::RefreshApolloCheckpoint(){
 ApolloCachedCheckpoint=nullptr;ApolloSaveStatus=EApolloSaveStatus::Missing;
 if(ApolloSaveEnabled()&&!FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveProfile")))ApolloCachedCheckpoint=ApolloCheckpointStorage::Read(ApolloSaveDirectory(),ApolloSaveStatus);
}
bool ALunarGameMode::ApolloSafeGround(FVector Position,FVector& Grounded) const {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!P||Position.ContainsNaN()||Position.X < -59000||Position.X > 44000||Position.Y < -39000||Position.Y > 44000)return false;
 FCollisionQueryParams Query;Query.bTraceComplex=true;Query.AddIgnoredActor(P);
 TArray<FHitResult> Hits;GetWorld()->LineTraceMultiByChannel(Hits,FVector(Position.X,Position.Y,5000),FVector(Position.X,Position.Y,-10000),ECC_Visibility,Query);
 for(const FHitResult& Hit:Hits){
  auto* Mesh=Cast<UStaticMeshComponent>(Hit.GetComponent());
  if(!Mesh||!Mesh->GetStaticMesh()||!Mesh->GetStaticMesh()->GetName().Contains(TEXT("Terrain")))continue;
  if(Hit.ImpactNormal.Z<P->GetCharacterMovement()->GetWalkableFloorZ())return false;
  const float Radius=P->GetCapsuleComponent()->GetScaledCapsuleRadius(),Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  Grounded=Hit.ImpactPoint+FVector(0,0,Half+3+Radius*(1/Hit.ImpactNormal.Z-1));
  return GetWorld()->FindTeleportSpot(P,Grounded,FRotator(0,P->GetActorRotation().Yaw,0));
 }
 return false;
}
void ALunarGameMode::RecordApolloEvidence(int32 Index,const FString& RelativePath){
 const FName Id=Index<3?FName(ApolloTasks::Photos[Index]):Index==3?FName(ApolloTasks::SurveyorPhoto):FName(ApolloTasks::DustPhotos[FMath::Clamp(Index-4,0,1)]);
 auto* Item=ApolloEvidence.FindByPredicate([&](const FApolloEvidence& E){return E.PhotoId==Id;});
 if(!Item){Item=&ApolloEvidence.AddDefaulted_GetRef();Item->PhotoId=Id;}
 Item->SiteId=Index<3?FName(ApolloTasks::Sites[Index]):Index==3?FName(TEXT("surveyor3")):FName(Index==4?TEXT("surveyor.exposed"):TEXT("surveyor.shielded"));Item->PhotoPath=RelativePath;Item->CapturedAt=ApolloElapsed;
}
UApolloCheckpoint* ALunarGameMode::CaptureApolloCheckpoint(){
 if(!Apollo||!ApolloStarted||(!IsActive()&&Stage!=ELunarStage::Won))return nullptr;
 FVector Safe;auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 const bool Grounded=P&&P->GetCharacterMovement()->IsMovingOnGround();
 if(!ApolloSafeGround(Grounded?P->GetActorLocation():ApolloHasSafePosition?ApolloSafePosition:ApolloLastPosition,Safe))return nullptr;
 auto* Save=Cast<UApolloCheckpoint>(UGameplayStatics::CreateSaveGameObject(UApolloCheckpoint::StaticClass()));
 Save->RunId=ApolloRunId;Save->SavedAtUtc=FDateTime::UtcNow().GetTicks();Save->Position=Safe;
 Save->Mode=ApolloTimed?TEXT("timed-exercise"):TEXT("historical-exploration");
 Save->View=(P?P->GetControlRotation():ApolloSafeView).GetNormalized();Save->Elapsed=ApolloElapsed;Save->Distance=ApolloDistance;Save->Remaining=Oxygen;Save->AntennaAngle=ApolloAntennaAngle;
 Save->LookLearned=ApolloLookLearned;Save->MoveLearned=ApolloMoveLearned;Save->Evidence=ApolloEvidence;Save->Trail=ApolloTrailSnapshot();Save->TrailFromStart=ApolloTrailFromStart;Save->TrailSimplified=ApolloTrailSimplified||ApolloTrail.Num()>=1024;
 auto Add=[&](const TCHAR* Task){Save->CompletedTasks.Add(FName(Task));};
 const int32 CheckoutCount=Stage==ELunarStage::ApolloCheckout?FMath::Clamp(ApolloAction,0,3):3;
 for(int I=0;I<CheckoutCount;++I)Add(ApolloTasks::Checkout[I]);
 for(int I=0;I<2;++I)if(ApolloInstruments&(1<<I))Add(ApolloTasks::Instruments[I]);
 for(int I=0;I<3;++I){if(ApolloPhotos&(1<<I))Add(ApolloTasks::Photos[I]);if(SamplesCollected&(1<<I))Add(ApolloTasks::Samples[I]);if(ApolloParts&(1<<I))Add(ApolloTasks::Parts[I]);}
 if(Stage==ELunarStage::ApolloRecover||Stage==ELunarStage::ApolloReturn||Stage==ELunarStage::Won)Add(ApolloTasks::Arrival);
 if(SurveyorPhotographed)Add(ApolloTasks::SurveyorPhoto);
 for(int I=0;I<2;++I)if(ApolloDustPhotos&(1<<I))Add(ApolloTasks::DustPhotos[I]);
 if(ApolloDustCompared)Add(ApolloTasks::DustCompared);
 if(Stage==ELunarStage::Won)Add(ApolloTasks::Stowed);
 if(Stage==ELunarStage::ApolloSamples)Save->SelectedSite=FName(ApolloTasks::Sites[FMath::Clamp(SelectedSample,0,2)]);
 else if(Stage==ELunarStage::ApolloRecover&&SurveyorPhotographed)Save->SelectedSite=FName(ApolloTasks::Parts[FMath::Clamp(SelectedSample,0,2)]);
 return Save;
}
bool ALunarGameMode::SaveApolloCheckpoint(){
 if(!ApolloSaveEnabled())return true;
 const double StartedAt=FPlatformTime::Seconds();
 auto* Save=CaptureApolloCheckpoint();
 const bool Good=Save&&ApolloCheckpointStorage::Write(ApolloSaveDirectory(),Save);
 ApolloSaveError=!Good;ApolloSaveNotice=Good?TEXT("CheckpointSaved"):TEXT("CheckpointWriteFailed");ApolloSaveNoticeRemaining=Good?6:30;
 if(Good){ApolloCachedCheckpoint=Save;ApolloSaveStatus=EApolloSaveStatus::Ready;}
 else UE_LOG(LogTemp,Warning,TEXT("APOLLO_CHECKPOINT: write rejected or failed; previous checkpoint retained"));
 UE_LOG(LogTemp,Display,TEXT("APOLLO_CHECKPOINT write %s %.3f ms"),Good?TEXT("OK"):TEXT("FAILED"),(FPlatformTime::Seconds()-StartedAt)*1000);
 return Good;
}
bool ALunarGameMode::RestoreApolloCheckpoint(){
 if(!Apollo)return false;
 // A completion from this run must not overwrite a checkpoint just restored.
 if(ApolloPhotoPending){ApolloSaveNotice=TEXT("CheckpointPhotoPending");ApolloSaveNoticeRemaining=6;ApolloSaveError=false;return false;}
 RefreshApolloCheckpoint();auto* Save=ApolloCachedCheckpoint.Get();if(!Save){ApolloSaveError=true;ApolloSaveNotice=TEXT("CheckpointLoadFailed");ApolloSaveNoticeRemaining=30;return false;}
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);FVector Ground;
 if(!P||!PC||SampleActors.Num()!=3||ApolloPartActors.Num()!=3||!ApolloSafeGround(Save->Position,Ground)){
  ApolloSaveError=true;ApolloSaveNotice=TEXT("CheckpointLoadFailed");ApolloSaveNoticeRemaining=30;return false;
 }
 ALunarCargoFinale::Reset(this);P->ResetFootprints();
 // Validate position before changing progress. In-progress work is never restored.
 ApolloPhotoFailed=false;ApolloToolFeedback.Reset();ApolloToolFeedbackRemaining=0;
 CancelApolloTool();ApolloInputHintRemaining=0;ApolloConfirmationRemaining=0;ApolloPackedPreviewRemaining=0;ApolloPackedPartRemaining=0;ApolloDustMode=false;Paused=false;Journal=false;ApolloConfirmNew=false;ApolloStarted=true;
 ApolloGuideEnabled=ULunarSettings::Get()->TutorialHints;
 ApolloTimed=Save->Mode==TEXT("timed-exercise");
 ApolloRunId=Save->RunId;ApolloEvidence=Save->Evidence;ApolloTrail=Save->Trail;ApolloTrailFromStart=Save->TrailFromStart;ApolloTrailSimplified=Save->TrailSimplified;ApolloTrailSpacing=ApolloTrailSimplified?300.f:150.f;ApolloTrailPendingBreak=true;ApolloTrailLatest=Ground;ApolloTrailLatestTime=Save->Elapsed;ApolloElapsed=Save->Elapsed;ApolloDistance=Save->Distance;Oxygen=Save->Remaining;
 ApolloAntennaAngle=Save->AntennaAngle;ApolloLookLearned=Save->LookLearned;ApolloMoveLearned=Save->MoveLearned;ApolloRouteIndex=0;
 SamplesCollected=0;ApolloPhotos=0;ApolloParts=0;ApolloInstruments=0;SelectedSample=0;
 for(int I=0;I<3;++I){
  if(Save->Has(ApolloTasks::Samples[I]))SamplesCollected|=1<<I;
  if(Save->Has(ApolloTasks::Photos[I]))ApolloPhotos|=1<<I;
  if(Save->Has(ApolloTasks::Parts[I]))ApolloParts|=1<<I;
  SampleActors[I]->SetActorHiddenInGame((SamplesCollected&(1<<I))!=0);ApolloPartActors[I]->SetActorHiddenInGame((ApolloParts&(1<<I))!=0);
  if(Save->SelectedSite==FName(ApolloTasks::Sites[I])||Save->SelectedSite==FName(ApolloTasks::Parts[I]))SelectedSample=I;
 }
 for(int I=0;I<2;++I)if(Save->Has(ApolloTasks::Instruments[I]))ApolloInstruments|=1<<I;
 SurveyorPhotographed=Save->Has(ApolloTasks::SurveyorPhoto);
 ApolloDustPhotos=0;for(int I=0;I<2;++I)if(Save->Has(ApolloTasks::DustPhotos[I]))ApolloDustPhotos|=1<<I;ApolloDustCompared=Save->Has(ApolloTasks::DustCompared);
 const ELunarStage Stages[]={ELunarStage::ApolloCheckout,ELunarStage::ApolloDeploy,ELunarStage::ApolloSamples,ELunarStage::ApolloTraverse,ELunarStage::ApolloRecover,ELunarStage::ApolloReturn,ELunarStage::Won};
 Stage=Stages[Save->Step()];ApolloAction=Stage==ELunarStage::ApolloCheckout?int32(Save->Has(ApolloTasks::Checkout[0]))+int32(Save->Has(ApolloTasks::Checkout[1])):Stage==ELunarStage::ApolloDeploy?int32(Save->Has(ApolloTasks::Instruments[0])):2;
 if(ApolloAntenna)ApolloAntenna->SetRelativeRotation(FRotator(-65,ApolloAntennaAngle,0));
 P->FastStride=false;P->SuitOrbit=false;if(P->ThirdPerson)P->ToggleSuitView();P->Camera->SetFieldOfView(ULunarSettings::Get()->FieldOfView);
 P->GetCharacterMovement()->SetComponentTickEnabled(true);P->GetCharacterMovement()->StopMovementImmediately();
 P->SetActorLocation(Ground,false,nullptr,ETeleportType::TeleportPhysics);P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);PC->SetControlRotation(Save->View);
 P->LastStepPosition=Ground;P->PausedVelocity=FVector::ZeroVector;P->StepTravel=0;P->GaitBlend=0;P->WorkBlend=0;P->SuitStrideSpeed=P->SuitTurnLean=0;P->SuitMoveDirection=FVector::ForwardVector;
 ApolloLastPosition=Ground;ApolloSafePosition=Ground;ApolloSafeView=Save->View;ApolloHasSafePosition=true;ApolloTutorialOrigin=Ground;ApolloTutorialAim=Save->View;
 LowOxygenWarned=Oxygen<180;FailureReason.Reset();UpdateAudioPause();
 int Missing=0;for(const auto& E:ApolloEvidence)if(!E.PhotoPath.IsEmpty()&&!IFileManager::Get().FileExists(*(FPaths::ProjectSavedDir()/E.PhotoPath)))++Missing;
 ApolloSaveError=false;ApolloSaveNotice=ApolloSaveStatus==EApolloSaveStatus::Recovered?TEXT("CheckpointRecovered"):Missing?TEXT("CheckpointPhotosMissing"):TEXT("CheckpointLoaded");ApolloSaveNoticeRemaining=Missing||ApolloSaveStatus==EApolloSaveStatus::Recovered?20:6;
 Message(ApolloInstruction());return true;
}
void ALunarGameMode::ContinueApollo(){
 if(!ApolloCachedCheckpoint)return;
 if(Menu){LunarLoading::Open(this,TEXT("/Game/Lunar/Maps/Apollo12"),TEXT("ResumeApollo=1"));return;}
 if(Apollo&&(Stage==ELunarStage::Briefing||Stage==ELunarStage::Lost))RestoreApolloCheckpoint();
}
