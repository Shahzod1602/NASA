#include "LunarGame.h"
#include "LunarTrail.h"
#include "LunarCargoFinale.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "ApolloData.h"
#include "LunarApolloText.h"
#include "Engine/World.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include <limits>

void ALunarGameMode::ApolloExperienceChecks(){
 FString Report;int32 Failed=0;
 auto Check=[&](bool Good,const TCHAR* Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+FString(Name)+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 StartApollo(true);P->GetCharacterMovement()->DisableMovement();
 FVector2D UV;
 Check(LunarTrail::MapUV(FVector(-60025.366,-39983.14,0),UV)&&UV.IsNearlyZero(.00001),TEXT("Map northwest corner follows imported world-centimetre extent"));
 Check(LunarTrail::MapUV(FVector(44774.634,44816.86,0),UV)&&UV.Equals(FVector2D(1,1),.00001),TEXT("Map southeast corner preserves horizontal metres and orientation"));
 Check(!LunarTrail::MapUV(FVector(50000,0,0),UV)&&!LunarTrail::MapUV(FVector(std::numeric_limits<double>::quiet_NaN(),0,0),UV),TEXT("Outside and non-finite positions are rejected rather than edge-clamped"));
 Check(ApolloTrail.Num()==1&&ApolloTrailFromStart&&ApolloTrail[0].Elapsed==0,TEXT("New mission starts a fresh local trace at time zero"));
 const FVector Origin=ApolloTrail[0].Position;
 ApolloElapsed=1;RecordApolloTrail(Origin+FVector(200,0,0),1,true);
 Check(ApolloTrail.Num()==2&&!ApolloTrail.Last().BreakBefore,TEXT("Accepted ordinary movement extends the current trace"));
 const int32 Count=ApolloTrail.Num();const float Time=ApolloElapsed;
 Paused=true;RecordApolloTrail(Origin+FVector(400,0,0),10,true);TickApollo(10);
 Check(ApolloTrail.Num()==Count&&ApolloElapsed==Time,TEXT("Pause preserves walking trace and mission timer"));Paused=false;
 RecordApolloTrail(Origin+FVector(500,0,0),0,true);RecordApolloTrail(Origin+FVector(500,0,0),-1,true);
 Check(ApolloTrail.Num()==Count,TEXT("Zero and negative deltas cannot manufacture a walking trace"));
 RecordApolloTrail(Origin+FVector(10000,0,0),1,false);ApolloElapsed=2;RecordApolloTrail(Origin+FVector(10200,0,0),1,true);
 Check(ApolloTrail.Last().BreakBefore,TEXT("Teleport discontinuities are never drawn as walked segments"));
 ApolloElapsed=3;RecordApolloTrail(Origin+FVector(10240,0,0),1,true);
 Check(ApolloTrailSnapshot().Num()==ApolloTrail.Num()+1,TEXT("The latest accepted endpoint is retained even below display spacing"));
 auto* Save=CaptureApolloCheckpoint();FString Reason;
 Check(Save&&Save->Validate(Reason)&&Save->Trail.Num()==ApolloTrailSnapshot().Num(),TEXT("Checkpoint includes a valid bounded trace snapshot"));
 if(Save){
  TArray<uint8> Bytes;const bool Written=UGameplayStatics::SaveGameToMemory(Save,Bytes);auto* Round=Written?Cast<UApolloCheckpoint>(UGameplayStatics::LoadGameFromMemory(Bytes)):nullptr;
  Check(Round&&Round->Validate(Reason)&&Round->Trail.Num()==Save->Trail.Num()&&Round->Trail.Last().BreakBefore==Save->Trail.Last().BreakBefore&&Round->TrailFromStart,TEXT("Trace positions, boundaries and coverage survive actual save serialization"));
  Save->Trail.Reset();Save->TrailFromStart=false;Check(Save->Validate(Reason),TEXT("Legacy checkpoint with no optional trace stays valid"));
  FApolloTrailPoint Invalid;Invalid.Position=FVector(0,0,0);Invalid.Elapsed=Save->Elapsed+2;Save->Trail.Add(Invalid);
  Check(!Save->Validate(Reason),TEXT("A checkpoint cannot contain future trace time"));Save->Trail[0].Elapsed=0;Save->Trail[0].Position.X=std::numeric_limits<double>::infinity();
  Check(!Save->Validate(Reason),TEXT("Non-finite checkpoint trace coordinates are rejected"));Save->Trail.Reset();Save->Trail.SetNum(1025);
  Check(!Save->Validate(Reason),TEXT("Checkpoint trace capacity is enforced"));
 }
 TArray<FApolloTrailPoint> Bounded;float Spacing=150;bool Simplified=false;
 for(int32 I=0;I<2100;++I){FApolloTrailPoint Point;Point.Position=FVector(I%200*100,I/200*100,0);Point.Elapsed=I;Point.BreakBefore=I==0||I==333;LunarTrail::Append(Bounded,Point,Spacing,Simplified);}
 Check(Bounded.Num()<=1024&&Simplified&&Spacing>150&&Bounded[0].Elapsed==0&&Bounded.Last().Elapsed==2099,TEXT("Long traces decimate within capacity while preserving first and latest endpoints"));
 bool GapKept=false;for(const auto& Point:Bounded)if(Point.BreakBefore&&Point.Elapsed>=333&&Point.Elapsed<345)GapKept=true;
 Check(GapKept,TEXT("Trace simplification carries discontinuities forward without inventing a connection"));
 ResetApolloTrail(false);Check(!ApolloTrailFromStart&&ApolloTrailSummary().Contains(TEXT("Partial trace")),TEXT("Old or resumed partial coverage is explained honestly"));
 StartApollo(true);Check(ApolloTrail.Num()==1&&!ApolloTrailSimplified&&ApolloTrailFromStart&&P->FootprintCount==0,TEXT("Restart clears prior trace, simplification and tread state"));
 ALunarCargoFinale::ValidateChecks(this,Check);
 StartApollo(true);Stage=ELunarStage::ApolloReturn;SamplesCollected=ApolloParts=7;ApolloInstruments=3;CompleteApolloInteraction(HomeActor);
 Check(Stage==ELunarStage::Won&&ALunarCargoFinale::Active(this),TEXT("Actual return completion commits win before starting stowage"));
 P->PauseMission();P->StartMission();Check(!Paused&&ALunarCargoFinale::Active(this),TEXT("Enter resumes paused finale without skipping it"));
 P->PauseMission();P->PrimaryClick();Check(!Paused&&ALunarCargoFinale::Active(this),TEXT("Primary click resumes paused finale without skipping it"));
 const bool OriginalThird=P->ThirdPerson;P->ToggleSuitView();Check(P->ThirdPerson==OriginalThird,TEXT("Camera-view input cannot alter the preserved player view during stowage"));
 P->PauseMission();const float Clock=ApolloElapsed,Remaining=Oxygen;auto* Scene=CargoFinale.Get();const float SceneTime=Scene?Scene->PresentationTime():0;TickApollo(4);if(Scene)Scene->AdvancePresentation(4);
 Check(Paused&&Scene&&Scene->PresentationTime()==SceneTime&&Clock==ApolloElapsed&&Remaining==Oxygen,TEXT("Normal pause input freezes finale and completed telemetry"));
 P->PauseMission();const FRotator Aim=PC->GetControlRotation();P->Turn(3);P->Look(3);
 Check(!Paused&&PC->GetControlRotation().Equals(Aim),TEXT("Finale resume works and look input cannot change saved player aim"));
 P->Interact();Check(!ALunarCargoFinale::Active(this)&&Stage==ELunarStage::Won,TEXT("Rebound interaction path skips directly to completed results"));
 StartApollo(true);P->AppendFootprintChecks(Report,Failed);
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("experience-checks.txt")));UE_LOG(LogTemp,Display,TEXT("EXPEDITION_EXPERIENCE_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}

void ALunarGameMode::ApolloExperienceReview(){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 const FString Folder=FPaths::ProjectDir()/TEXT("Reports/ExpeditionExperience20261029");IFileManager::Get().MakeDirectory(*Folder,true);
 FFileHelper::SaveStringToFile(TEXT("{\"synthetic\":true,\"human_playtest\":false,\"description\":\"Staged trace/cargo for layout and presentation review only.\"}"),*(Folder/TEXT("review-fixtures.json")));
 StartApollo(true);P->GetCharacterMovement()->DisableMovement();ApolloElapsed=800;ApolloDistance=1237;ApolloPhotos=7;SamplesCollected=7;ApolloParts=7;SurveyorPhotographed=true;
 ApolloTrail.Reset();FVector Path[]={ApolloData::Check,ApolloData::Instruments[0],ApolloData::Samples[0],ApolloData::Samples[1],ApolloData::Samples[2],ApolloData::Surveyor,ApolloData::Home};
 for(int32 I=0;I<7;++I){FApolloTrailPoint Point;Point.Position=Path[I];Point.Elapsed=I*100;Point.BreakBefore=I==0;ApolloTrail.Add(Point);}ApolloTrailPendingBreak=true;
 auto* Settings=ULunarSettings::Get();Settings->TextScale=1;
 FTimerHandle Sequence;GetWorldTimerManager().SetTimer(Sequence,FTimerDelegate::CreateWeakLambda(this,[this,P,PC,Folder,Step=0]()mutable{
  auto Shot=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(Folder/TEXT("experience-")+Name+TEXT(".png"),true,false);};
  switch(Step++){
   case 0:Stage=ELunarStage::ApolloReturn;Journal=true;Paused=true;JournalPage=0;break;
   case 1:Shot(TEXT("journal-trace"));break;
   case 2:Journal=false;Paused=false;P->OpenEvidence();if(P->ActivePanel)P->ActivePanel->ToggleExpeditionMap();break;
   case 3:Shot(TEXT("my-expedition"));break;
   case 4:ULunarSettings::Get()->TextScale=1.5f;if(P->ActivePanel)P->ActivePanel->Rebuild();break;
   case 5:Shot(TEXT("map150"));break;
   case 6:P->ClosePanel();ULunarSettings::Get()->TextScale=1;Stage=ELunarStage::Won;Paused=false;Message(ApolloText::Get(TEXT("MissionDone")));ALunarCargoFinale::Begin(this);if(CargoFinale)CargoFinale->AdvancePresentation(5.6f);Paused=true;break;
   case 7:Paused=false;Shot(TEXT("cargo-finale"));break;
   case 8:ALunarCargoFinale::Skip(this);Shot(TEXT("completed"));break;
   case 9:ALunarCargoFinale::Reset(this);SamplesCollected=5;ApolloParts=2;ALunarCargoFinale::Begin(this);if(CargoFinale)CargoFinale->AdvancePresentation(4);break;
   case 10:Shot(TEXT("partial-cargo"));break;
   case 11:{
    ALunarCargoFinale::Reset(this);Stage=ELunarStage::ApolloCheckout;Paused=false;P->ResetFootprints();
    for(int32 I=0;I<24;++I){const FVector XY(-1300-I*48.f,-900-180*FMath::Sin(I*.13f),0);FVector Ground;
     if(ApolloSafeGround(XY,Ground)){P->SetActorLocation(Ground);P->SetActorRotation(FRotator(0,180,0));P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);P->GetCharacterMovement()->FindFloor(Ground,P->GetCharacterMovement()->CurrentFloor,false);P->LastStepPosition=Ground;P->UpdateSuit(0);P->StampFootstep();}
    }
    P->GetCharacterMovement()->DisableMovement();P->SetActorLocation(FVector(-1200,-650,260));PC->SetControlRotation((FVector(-1900,-1050,30)-FVector(-1200,-650,328)).Rotation());Paused=true;
    P->HandRig->SetVisibility(false,true);P->SuitRig->SetVisibility(false,true);for(auto& Arm:P->SuitArms)if(Arm)Arm->SetVisibility(false);for(auto& Body:P->SuitBody)if(Body)Body->SetVisibility(false);RadioRemaining=0;break;
   }
   case 12:FScreenshotRequest::RequestScreenshot(Folder/TEXT("experience-boot-treads.png"),false,false);break;
   case 13:FPlatformMisc::RequestExitWithStatus(false,0);break;
  }
 }),1.2f,true);
}
