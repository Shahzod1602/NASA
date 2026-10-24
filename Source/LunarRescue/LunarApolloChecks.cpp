#include "LunarGame.h"
#include "LunarApolloText.h"
#include "LunarSettings.h"
#include "ApolloData.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMemory.h"
#include "UnrealClient.h"

void ALunarGameMode::ApolloChecks(){
 // Preserve legacy completion invariants; ApolloPolishTest exercises the actual tool contexts.
 ApolloToolsEnabled=false;ApolloTimed=true;
 int32 Fail=0;FString Report;
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Fail;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 Check(Apollo&&P&&PC&&SampleActors.Num()==3&&ApolloInstrumentActors.Num()==2&&ApolloPartActors.Num()==3&&ApolloRouteActors.Num()==12,TEXT("Apollo map and complete expedition props loaded"));
 if(!P||!PC||SampleActors.Num()!=3||ApolloPartActors.Num()!=3){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 Check(ApolloMapTexture!=nullptr,TEXT("NASA orbital field map is packaged"));
 Check(FMath::IsNearlyEqual(GetWorld()->GetGravityZ(),-162.f,.01f),TEXT("Lunar gravity 1.62 m/s2"));
 Check(FMath::IsNearlyEqual(P->GetCharacterMovement()->MaxWalkSpeed,140.f),TEXT("Route walk speed uses metres rather than compressed geography"));
 auto Terrain=[&](FVector Expected){FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;Q.AddIgnoredActor(P);return GetWorld()->LineTraceSingleByChannel(Hit,Expected+FVector(0,0,5000),Expected-FVector(0,0,5000),ECC_Visibility,Q)&&FMath::Abs(Hit.ImpactPoint.Z-Expected.Z)<2.f;};
 Check(Terrain(ApolloData::Home),TEXT("Imported terrain elevation matches Intrepid source point"));
 Check(Terrain(ApolloData::Surveyor),TEXT("Imported terrain elevation matches Surveyor source point"));
 for(auto V:ApolloData::Samples)Check(Terrain(V),TEXT("Sample collision height matches source DTM triangle"));
 Check(FVector::Dist2D(ApolloData::Home,ApolloData::Surveyor)>15000&&FVector::Dist2D(ApolloData::Home,ApolloData::Surveyor)<16500,TEXT("Mapped LM-Surveyor separation is preserved at full scale"));
 Check(Oxygen==1800&&Stage==ELunarStage::Briefing,TEXT("Briefing starts with 30-minute exercise budget"));
 AdvanceTime(100);Check(Oxygen==1800,TEXT("Briefing does not consume time"));
 StartMission();Check(Stage==ELunarStage::ApolloCheckout,TEXT("Expedition starts with checkout"));
 Check(ApolloObjective()==TEXT("1 / 6   PREPARE FOR THE EXPEDITION")&&ApolloText::Get(TEXT("CheckpointContinue"))==TEXT("C  Continue expedition"),TEXT("Apollo objectives and checkpoint controls use English"));
 Check(!ApolloTutorial().IsEmpty()&&!ApolloLookLearned&&!ApolloMoveLearned,TEXT("Checkout starts with nonblocking look and movement guidance"));
 PC->SetControlRotation(ApolloTutorialAim+FRotator(0,30,0));Paused=true;TickApollo(0);Check(!ApolloLookLearned,TEXT("Paused tutorial does not advance"));Paused=false;TickApollo(0);
 Check(ApolloLookLearned&&!ApolloMoveLearned,TEXT("Looking around advances guidance without requiring a key"));
 P->SetActorLocation(ApolloTutorialOrigin+FVector(0,180,0));TickApollo(0);Check(ApolloMoveLearned,TEXT("Walking away from the start introduces the work instruction"));
 auto Aim=[&](AActor* A){P->GetCharacterMovement()->StopMovementImmediately();P->SetActorLocation(A->GetActorLocation()+FVector(-170,0,50),false,nullptr,ETeleportType::TeleportPhysics);FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Work=[&](){Interact();AdvanceInteraction(6);ReleaseInteract();};
 Aim(DroneActor);Work();Check(Stage==ELunarStage::ApolloCheckout&&!SurveyorPhotographed,TEXT("Cannot bypass checkout at Surveyor"));
 Aim(ModuleActor);Interact();AdvanceInteraction(1);ReleaseInteract();Check(ApolloAction==0,TEXT("Interrupted hold cannot finish checkout"));
 Interact();AdvanceInteraction(.5);Paused=true;AdvanceTime(50);AdvanceInteraction(50);Check(HoldProgress==.5f&&Oxygen==1800,TEXT("Pause freezes work and mission budget"));Paused=false;ReleaseInteract();
 Work();Work();Work();Check(Stage==ELunarStage::ApolloDeploy&&ApolloAction==0,TEXT("Three checkout actions unlock ALSEP"));
 Aim(ApolloInstrumentActors[0]);Work();Check(ApolloInstruments==1&&ApolloAction==1,TEXT("Seismometer deploys before central station"));
 Aim(ApolloInstrumentActors[1]);Work();Check(Stage==ELunarStage::ApolloDeploy&&ApolloInstruments==1&&!Holding,TEXT("Antenna rejects wrong Earth bearing"));
 for(int I=0;I<5;++I)CycleApollo();Work();Check(Stage==ELunarStage::ApolloSamples&&ApolloInstruments==3,TEXT("Aimed antenna and verified link unlock samples"));
 P->ToggleJournal();const float Budget=Oxygen;AdvanceTime(40);AdvanceInteraction(40);const int Page=JournalPage;CycleApollo();Check(Paused&&Journal&&Oxygen==Budget&&JournalPage==(Page+1)%4,TEXT("Journal pauses play and Q changes history page"));P->ToggleJournal();Check(!Paused&&!Journal,TEXT("Closing journal resumes play"));
 AActor* Before=Target();CycleApollo();Check(Target()!=Before,TEXT("Q cycles sample navigation"));
 for(int I:{1,0,2}){
  Aim(SampleActors[I]);Check(CanInteract(),TEXT("Sample can be selected by proximity independently of marker"));
  Interact();Check((ApolloPhotos&(1<<I))&&!(SamplesCollected&(1<<I)),TEXT("First sample action records context without collecting"));
  Interact();AdvanceInteraction(1);PC->SetControlRotation(FRotator(0,90,0));AdvanceInteraction(.1f);Check(!Holding&&!(SamplesCollected&(1<<I)),TEXT("Looking away cancels collection"));
  Aim(SampleActors[I]);Work();Check(SamplesCollected&(1<<I),TEXT("Photographed sample is sealed once"));
  const uint8 Mask=SamplesCollected;Work();Check(SamplesCollected==Mask,TEXT("Collected sample cannot be counted twice"));
 }
 Check(Stage==ELunarStage::ApolloTraverse&&ApolloPhotos==7,TEXT("All documented samples unlock historical approach"));
 Check(Target()==DroneActor,TEXT("Approach navigates to Surveyor rather than a hidden waypoint sequence"));
 Aim(ApolloRouteActors[0]);TickApollo(0);Check(Stage==ELunarStage::ApolloTraverse&&!SurveyorPhotographed,TEXT("Historical route remains optional and does not award Surveyor evidence"));
 Aim(DroneActor);TickApollo(0);Check(Stage==ELunarStage::ApolloRecover&&ApolloRouteIndex==0,TEXT("Arriving directly at Surveyor unlocks recovery without visiting twelve markers"));
 Aim(DroneActor);Interact();Check(SurveyorPhotographed&&ApolloParts==0,TEXT("Surveyor must be documented before removal"));
 const float SavedBudget=Oxygen;
 for(int I:{2,0,1}){Aim(ApolloPartActors[I]);Work();Check(ApolloParts&(1<<I),TEXT("Surveyor hardware can be packed in any order"));}
 Check(Stage==ELunarStage::ApolloReturn&&ApolloParts==7,TEXT("Camera scoop and cable unlock return"));
 Check(Oxygen==SavedBudget,TEXT("Hardware recovery provides no fictional oxygen reward"));
 Aim(HomeActor);Work();Check(Stage==ELunarStage::Won,TEXT("All science and hardware stowed completes expedition"));
 const float CompleteBudget=Oxygen;AdvanceTime(30);Check(Oxygen==CompleteBudget,TEXT("Completed expedition freezes the budget"));
 Stage=ELunarStage::ApolloCheckout;Oxygen=50;AdvanceTime(-10);Check(Oxygen==50,TEXT("Negative time cannot increase budget"));
 Oxygen=.5;AdvanceTime(1);Check(Stage==ELunarStage::Lost&&FailureReason==ApolloText::Get(TEXT("TimeEnded")),TEXT("Timeout ends the exercise without claiming astronaut death"));
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Fail);FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("apollo12-checks.txt")));
 UE_LOG(LogTemp,Display,TEXT("APOLLO12_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Fail?1:0);
}

void ALunarGameMode::ApolloReview(){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();Oxygen=1500;
 auto View=[&](AActor* A,FVector Offset=FVector(-450,-320,180)){P->SetActorLocation(A->GetActorLocation()+Offset);FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()+FVector(0,0,45)-Eye).Rotation());};
 auto Shot=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(ReportPath(Name),true,false);};
 switch(MovementPhase){
 case 0:Stage=ELunarStage::Briefing;View(HomeActor,FVector(-800,-700,260));break;
 case 1:Shot(TEXT("apollo12-briefing.png"));break;
 case 2:Stage=ELunarStage::ApolloCheckout;View(HomeActor,FVector(-600,-700,160));break;
 case 3:Shot(TEXT("apollo12-intrepid.png"));break;
 case 4:Stage=ELunarStage::ApolloDeploy;ApolloAction=1;View(ApolloInstrumentActors[1]);break;
 case 5:Shot(TEXT("apollo12-alsep.png"));break;
 case 6:Stage=ELunarStage::ApolloSamples;SelectedSample=0;View(SampleActors[0],FVector(-250,-140,40));break;
 case 7:Shot(TEXT("apollo12-sample.png"));break;
 case 8:Stage=ELunarStage::ApolloRecover;SamplesCollected=7;ApolloPhotos=7;ApolloInstruments=3;View(DroneActor,FVector(-550,-460,200));break;
 case 9:Shot(TEXT("apollo12-surveyor.png"));break;
 case 10:Journal=true;Paused=true;JournalPage=0;break;
 case 11:Shot(TEXT("apollo12-map.png"));break;
 case 12:JournalPage=3;break;
 case 13:Shot(TEXT("apollo12-sources.png"));break;
 case 14:Journal=false;Paused=false;Stage=ELunarStage::Won;SamplesCollected=7;ApolloParts=7;ApolloInstruments=3;ApolloElapsed=1224;ApolloDistance=1470;break;
 case 15:Shot(TEXT("apollo12-result.png"));break;
 case 16:Stage=ELunarStage::ApolloTraverse;ApolloParts=0;SurveyorPhotographed=false;View(DroneActor,FVector(-2400,-1600,280));break;
 case 17:Shot(TEXT("apollo12-approach.png"));break;
 case 18:Stage=ELunarStage::ApolloRecover;SurveyorPhotographed=true;SelectedSample=2;View(ApolloPartActors[2],FVector(-170,0,50));break;
 case 19:Shot(TEXT("apollo12-hardware.png"));break;
 case 20:Stage=ELunarStage::ApolloReturn;View(HomeActor,FVector(-170,0,50));break;
 case 21:Shot(TEXT("apollo12-return.png"));break;
 case 22:Journal=true;Paused=true;JournalPage=1;break;
 case 23:Shot(TEXT("apollo12-history.png"));break;
 case 24:JournalPage=2;break;
 case 25:Shot(TEXT("apollo12-evidence.png"));break;
 case 26:Journal=false;Paused=false;Stage=ELunarStage::ApolloCheckout;ApolloAction=0;ApolloLookLearned=true;ApolloMoveLearned=false;View(HomeActor,FVector(-600,-700,160));ApolloTutorialOrigin=P->GetActorLocation();break;
 case 27:Shot(TEXT("apollo12-tutorial.png"));break;
 case 28:ApolloMoveLearned=true;View(HomeActor,FVector(-170,0,50));break;
 case 29:Shot(TEXT("apollo12-work-prompt.png"));break;
 case 30:Holding=true;HoldProgress=1;HeldTarget=HomeActor;Shot(TEXT("apollo12-work-progress.png"));break;
 case 31:
  ReleaseInteract();ApolloStarted=true;ApolloRunId=FGuid::NewGuid();ApolloEvidence.Reset();ApolloElapsed=300;ApolloDistance=285;
  Stage=ELunarStage::ApolloSamples;ApolloAction=2;ApolloInstruments=3;SamplesCollected=2;ApolloPhotos=2;ApolloParts=0;SurveyorPhotographed=false;SelectedSample=2;
  RecordApolloEvidence(1,TEXT(""));P->GetCharacterMovement()->SetMovementMode(MOVE_Walking);View(HomeActor,FVector(-600,-700,160));
  ApolloCachedCheckpoint=CaptureApolloCheckpoint();ApolloSaveStatus=EApolloSaveStatus::Ready;Stage=ELunarStage::Briefing;break;
 case 32:Shot(TEXT("apollo12-continue.png"));break;
 case 33:ApolloConfirmNew=true;break;
 case 34:Shot(TEXT("apollo12-new-confirm.png"));break;
 case 35:ApolloConfirmNew=false;Stage=ELunarStage::ApolloSamples;Paused=true;break;
 case 36:Shot(TEXT("apollo12-reload.png"));break;
 case 37:Paused=false;Stage=ELunarStage::Lost;FailureReason=ApolloText::Get(TEXT("TimeEnded"));break;
 case 38:Shot(TEXT("apollo12-retry-checkpoint.png"));break;
 case 39:Stage=ELunarStage::ApolloSamples;ApolloSaveNotice=TEXT("CheckpointRecovered");ApolloSaveNoticeRemaining=20;break;
 case 40:Shot(TEXT("apollo12-recovered.png"));break;
 case 41:Stage=ELunarStage::ApolloDeploy;Paused=false;ApolloAction=0;ApolloSaveNoticeRemaining=0;View(ApolloInstrumentActors[0],FVector(-170,0,50));OpenApolloTool();break;
 case 42:Shot(TEXT("apollo12-tool-level.png"));break;
 case 43:CancelApolloTool();ApolloAction=1;View(ApolloInstrumentActors[1],FVector(-170,0,50));OpenApolloTool();break;
 case 44:Shot(TEXT("apollo12-tool-antenna.png"));break;
 case 45:CancelApolloTool();Stage=ELunarStage::ApolloSamples;ApolloPhotos=7;SamplesCollected=0;View(SampleActors[2],FVector(-170,0,50));OpenApolloTool();ApolloToolPhase=3;break;
 case 46:Shot(TEXT("apollo12-tool-core.png"));break;
 case 47:CancelApolloTool();ApolloPhotos=0;View(SampleActors[0],FVector(-170,0,50));OpenApolloTool();break;
 case 48:Shot(TEXT("apollo12-tool-camera.png"));break;
 case 49:CancelApolloTool();P->OpenSettings();break;
 case 50:Shot(TEXT("apollo12-settings.png"));break;
 case 51:P->ClosePanel();P->OpenEvidence();break;
 case 52:Shot(TEXT("apollo12-album.png"));break;
 case 53:P->ClosePanel();ULunarSettings::Get()->TextScale=1.5f;ULunarSettings::Get()->SubtitleScale=1.f;Stage=ELunarStage::ApolloSamples;Paused=false;Journal=false;RadioRemaining=0;break;
 case 54:Shot(TEXT("apollo12-text150.png"));break;
 case 55:ULunarSettings::Get()->Reset();break;
 default:FPlatformMisc::RequestExitWithStatus(false,0);return;
 }
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloReview,2,false);
}

void ALunarGameMode::ApolloWalkTick(float D){
 // Use real character input, collision and resource time. No teleports or completion shortcuts.
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!P||!PC)return;
 WalkSeconds+=D;WalkStepSeconds+=D;
 if(Stage==ELunarStage::Briefing){if(WalkSeconds>2){StartMission();WalkStepSeconds=0;}return;}
 if(Stage==ELunarStage::Won||Stage==ELunarStage::Lost||WalkSeconds>1700||WalkStepSeconds>500){
  const bool Good=Stage==ELunarStage::Won&&SamplesCollected==7&&ApolloParts==7&&ApolloInstruments==3;
  FString Report=FString::Printf(TEXT("%s real movement Apollo 12 expedition\nElapsed %.2f s\nWalked %.2f m\nRemaining exercise time %.2f s\nSamples %d/3; hardware mask %d; ALSEP mask %d\nPeak memory %.2f MiB\nRESULT: %d failures\n"),Good?TEXT("PASS"):TEXT("FAIL"),ApolloElapsed,ApolloDistance,Oxygen,SampleCount(),ApolloParts,ApolloInstruments,FPlatformMemory::GetStats().PeakUsedPhysical/1048576.0,Good?0:1);
  FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("apollo12-walkthrough.txt")));UE_LOG(LogTemp,Display,TEXT("APOLLO12_WALK %s"),*Report);ApolloWalking=false;
  if(FParse::Param(FCommandLine::Get(),TEXT("ApolloFullProfile"))){PC->ConsoleCommand(TEXT("csvprofile stop"),false);FTimerHandle End;GetWorldTimerManager().SetTimer(End,[Good](){FPlatformMisc::RequestExitWithStatus(false,Good?0:1);},8,false);}else FPlatformMisc::RequestExitWithStatus(false,Good?0:1);return;
 }
 auto* T=Target();if(!T)return;
 if(Stage==ELunarStage::ApolloTraverse&&FParse::Param(FCommandLine::Get(),TEXT("ApolloHistoricalRoute"))){
  if(ApolloRouteActors.IsValidIndex(ApolloRouteIndex)&&FVector::Dist2D(P->GetActorLocation(),ApolloRouteActors[ApolloRouteIndex]->GetActorLocation())<450)++ApolloRouteIndex;
  if(ApolloRouteActors.IsValidIndex(ApolloRouteIndex))T=ApolloRouteActors[ApolloRouteIndex];
 }
 const int Key=1000*ApolloStep()+100*ApolloAction+10*ApolloRouteIndex+SelectedSample;
 if(Key!=WalkStep){WalkStep=Key;WalkStepSeconds=0;UE_LOG(LogTemp,Display,TEXT("APOLLO12_WALK step %d %s target %s"),WalkStep,*Objective(),*T->GetActorLocation().ToString());}
 FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);const FVector PhotoOffset=ApolloToolsEnabled&&Stage==ELunarStage::ApolloRecover&&!SurveyorPhotographed?FVector(0,0,90):FVector::ZeroVector;PC->SetControlRotation((T->GetActorLocation()+PhotoOffset-Eye).Rotation());
 float Dist=FVector::Dist2D(P->GetActorLocation(),T->GetActorLocation());
 if(Stage!=ELunarStage::ApolloTraverse&&CanInteract()){
  P->GetCharacterMovement()->StopMovementImmediately();
  if(ApolloToolsEnabled){
   if(!ApolloTool)Interact();
   if(ApolloTool==2)ApolloToolAxis(-ApolloLevel.X/.035f,-ApolloLevel.Y/.035f);
   if(ApolloTool==3)ApolloToolAxis(FMath::FindDeltaAngleDegrees(ApolloAntennaAngle,-7.f)/.6f,0);
   if(ApolloTool)P->PrimaryClick();return;
  }
  if(Stage==ELunarStage::ApolloDeploy&&ApolloAction==1&&FMath::Abs(FMath::FindDeltaAngleDegrees(ApolloAntennaAngle,-7.f))>8){CycleApollo();return;}
  if(!Holding)Interact();return;
 }
 if(Dist>100){if(FParse::Param(FCommandLine::Get(),TEXT("ApolloProfileFastStride")))P->BeginFastStride();P->Forward(1);if(P->GetVelocity().Size2D()<15&&WalkStepSeconds>3&&FMath::Fmod(WalkSeconds,3)<D)P->Leap();}
}
