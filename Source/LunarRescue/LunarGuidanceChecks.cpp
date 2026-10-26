#include "LunarGame.h"
#include "LunarSettings.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UnrealClient.h"

void ALunarGameMode::ApolloGuidanceChecks(){
 FString Report;int Failed=0;
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 StartApollo(true);P->GetCharacterMovement()->DisableMovement();RadioRemaining=0;
 auto Aim=[&](AActor* A){P->SetActorLocation(A->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Click=[&](){TickApollo(.85f);P->PrimaryClick();};
 Check(ApolloTutorial().Contains(TEXT("1 / 4"))&&!ApolloFooter().Contains(TEXT("I:")),TEXT("First instruction teaches looking; album is introduced after evidence"));
 P->ToggleApolloGuide();Check(ApolloTutorial().IsEmpty()&&ApolloAction==0,TEXT("Skipping the guide never advances equipment or cargo"));
 P->ToggleApolloGuide();Check(!ApolloTutorial().IsEmpty(),TEXT("The guide can be shown again without restarting"));
 ApolloLookLearned=true;Check(ApolloTutorial().Contains(TEXT("2 / 4")),TEXT("Walking appears after the look instruction"));
 auto* S=ULunarSettings::Get();S->Rebind(TEXT("MoveForward"),EKeys::Z);S->Rebind(TEXT("Interact"),EKeys::B);
 Check(ApolloTutorial().Contains(TEXT("Z"))&&!S->Rebind(TEXT("Jump"),EKeys::H),TEXT("Guide displays rebound movement and reserves the help key"));
 const FString BeforeHintRadio=Radio;const int BeforeHintTranscript=ApolloTranscript.Num();
 P->SetActorLocation(ModuleActor->GetActorLocation()+FVector(-800,0,50));P->Interact();
 Check(ApolloInputHint().Contains(TEXT("Move closer"))&&ApolloInputHint().Contains(TEXT("3 m"))&&!ApolloTool&&ApolloAction==0,TEXT("Distant work input explains the real reach without opening or awarding work"));
 Check(Radio==BeforeHintRadio&&ApolloTranscript.Num()==BeforeHintTranscript,TEXT("Input help never replaces mission radio or appends transcript entries"));
 TickApollo(.5f);const float HintTime=ApolloInputHintRemaining;P->PrimaryClick();P->Interact();
 Check(ApolloInputHintRemaining==HintTime,TEXT("Repeated clicks and work input do not restart the hint timer"));
 Paused=true;P->PrimaryClick();P->Interact();TickApollo(1);
 Check(ApolloInputHint().IsEmpty()&&ApolloInputHintRemaining==HintTime,TEXT("Pause hides and freezes input help"));Paused=false;
 P->OpenSettings();P->PrimaryClick();P->Interact();Check(ApolloInputHint().IsEmpty()&&ApolloInputHintRemaining==HintTime,TEXT("Settings input does not create mission hints"));P->ClosePanel();
 ApolloMoveLearned=true;Aim(ModuleActor);PC->SetControlRotation(FRotator(0,180,0));
 Check(ApolloTutorial().Contains(TEXT("Look at")),TEXT("A nearby player facing away is told to look at the kit"));
 Check(ApolloInputHint().Contains(TEXT("Face the marked")),TEXT("Input help updates from distance to facing as the player approaches"));
 Aim(ModuleActor);Check(ApolloTutorial().Contains(TEXT("Press B")),TEXT("Opening instruction uses the current work binding"));
 Check(ApolloInputHint().Contains(TEXT("[ B ]")),TEXT("Input help switches to the rebound work key once the site is reachable"));
 S->Rebind(TEXT("Interact"),EKeys::N);Check(ApolloInputHint().Contains(TEXT("[ N ]")),TEXT("An active hint follows a changed binding without another click"));S->Rebind(TEXT("Interact"),EKeys::B);
 FVector Eye;FRotator AimRotation;P->GetActorEyesViewPoint(Eye,AimRotation);
 auto* Obstacle=GetWorld()->SpawnActor<AActor>();auto* Block=NewObject<UBoxComponent>(Obstacle);
 Obstacle->SetRootComponent(Block);Block->SetBoxExtent(FVector(12,90,120));Block->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Block->SetCollisionResponseToAllChannels(ECR_Ignore);Block->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);Block->RegisterComponent();Obstacle->SetActorLocation((Eye+ModuleActor->GetActorLocation())*.5f);
 P->Interact();Check(ApolloInputHint().Contains(TEXT("blocks your reach"))&&!ApolloTool,TEXT("A real visibility obstacle explains blocked reach rather than claiming the site is usable"));Obstacle->Destroy();
 ApolloPhotoPending=true;P->Interact();Check(ApolloInputHint().Contains(TEXT("Photo is saving"))&&!ApolloTool,TEXT("Work input during a pending photo explains why collection must wait"));ApolloPhotoPending=false;
 P->ToggleApolloGuide();P->PrimaryClick();P->Interact();
 // The reachable work key remains functional when optional hints are hidden.
 Check(ApolloInputHint().IsEmpty()&&ApolloInputHintRemaining==0&&ApolloTool==8,TEXT("Hidden guidance stays hidden while valid work remains available"));CancelApolloTool();P->ToggleApolloGuide();
 Stage=ELunarStage::ApolloTraverse;P->Interact();Check(ApolloInputHint().Contains(TEXT("no button is needed"))&&!ApolloTool,TEXT("Traverse input explains that reaching Surveyor progresses the route"));
 Stage=ELunarStage::ApolloRecover;SurveyorPhotographed=false;P->SetActorLocation(DroneActor->GetActorLocation()+FVector(-800,0,50));
 Check(ApolloInputHint().Contains(TEXT("7 m")),TEXT("Surveyor context photography uses its actual seven metre opening reach"));
 Stage=ELunarStage::Lost;Check(ApolloInputHint().IsEmpty(),TEXT("Terminal states suppress active mission hints"));
 StartApollo(true);Check(ApolloInputHintRemaining==0,TEXT("A new mission clears transient input guidance"));Aim(ModuleActor);ApolloLookLearned=true;ApolloMoveLearned=true;
 P->PrimaryClick();Check(!ApolloTool&&ApolloAction==0&&ApolloInputHint().Contains(TEXT("[ B ]")),TEXT("Primary click teaches the work key without starting or completing a tool"));
 TickApollo(6.1f);Check(ApolloInputHint().IsEmpty()&&ApolloInputHintRemaining==0,TEXT("Input help expires and returns the normal instruction"));
 P->PrimaryClick();
 P->Interact();Check(ApolloTool==8&&ApolloAction==0,TEXT("Opening the first check awards no completed work"));
 Check(ApolloInputHintRemaining==0,TEXT("Opening the tool clears the previous input hint"));
 Check(ApolloToolHelp().Contains(TEXT("Left click")),TEXT("The opened check teaches its distinct confirmation action"));
 P->PauseMission();Check(!ApolloTool&&ApolloAction==0&&!Paused,TEXT("Cancelling the first check does not complete it or pause"));
 for(int N=0;N<3;++N){P->Interact();Click();}
 Check(Stage==ELunarStage::ApolloDeploy&&ApolloConfirmation.Contains(TEXT("3 / 3")),TEXT("All three actual confirmations finish onboarding"));
 Stage=ELunarStage::ApolloSamples;SelectedSample=0;SamplesCollected=0;ApolloPhotos=2;Aim(SampleActors[1]);
 Check(ApolloGuidanceTarget()==SampleActors[1]&&ApolloSiteName().Contains(TEXT("BENCH"))&&ApolloPrompt().Contains(TEXT("soil"))&&ApolloInstruction().Contains(TEXT("Scoop")),TEXT("Prompt, destination and action describe the aimed soil site rather than the selected rock"));
 P->Interact();Click();P->UpdateSuit(0);
 Check(ApolloTool==4&&ApolloToolPhase==1&&!SamplesCollected&&P->SamplePouch&&P->SamplePouch->IsVisible(),TEXT("A pending soil pouch is visible before sealing and is not cargo"));
 CancelApolloTool();P->UpdateSuit(0);Check(!SamplesCollected&&!P->SamplePouch->IsVisible(),TEXT("Cancelling hides the pending pouch and keeps cargo unchanged"));
 P->Interact();Click();Click();P->UpdateSuit(0);
 Check(SamplesCollected==2&&ApolloConfirmation.Contains(TEXT("12032"))&&ApolloConfirmation.Contains(TEXT("1 / 3"))&&P->SamplePouch->IsVisible(),TEXT("Final seal adds the correct labeled cargo, confirmation and brief physical preview"));
 const float Remaining=ApolloConfirmationRemaining,Preview=ApolloPackedPreviewRemaining;Paused=true;TickApollo(1);
 Check(ApolloConfirmationRemaining==Remaining&&ApolloPackedPreviewRemaining==Preview,TEXT("Pausing freezes success feedback and the stow preview"));Paused=false;
 TickApollo(6);P->UpdateSuit(0);Check(!P->SamplePouch->IsVisible()&&ApolloConfirmationRemaining==0,TEXT("Temporary stow feedback ends without altering cargo"));
 Check(ApolloGuidanceTarget()!=SampleActors[1],TEXT("Completed sample no longer receives navigation guidance"));
 ApolloEvidence.AddDefaulted();Check(ApolloFooter().Contains(TEXT("I:")),TEXT("Album access is disclosed once evidence exists"));
 Stage=ELunarStage::ApolloRecover;SurveyorPhotographed=true;ApolloParts=0;SelectedSample=1;Aim(ApolloPartActors[0]);P->Interact();
 FVector First,Second;FString Label;
 Check(ApolloWorkCue(First,Label)&&Label.Contains(TEXT("FIRST")),TEXT("Hardware guide points to its first physical fitting"));
 Click();TickApollo(.8f);Check(ApolloWorkCue(Second,Label)&&Label.Contains(TEXT("SECOND"))&&!First.Equals(Second,1),TEXT("The next action moves the marker to the second physical fitting"));
 CancelApolloTool();Check(ApolloParts==0&&!ApolloPartActors[0]->IsHidden(),TEXT("Aborted guided removal keeps the hardware on Surveyor"));
 Paused=true;P->ToggleApolloGuide();Check(Journal&&JournalPage==4,TEXT("The paused game provides a revisitable how-to page"));CycleApollo();Check(JournalPage==0,TEXT("Journal cycles from help back to the route"));
 S->Reset();S->Apply(P,false);
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("guidance-checks.txt")));
 UE_LOG(LogTemp,Display,TEXT("APOLLO_GUIDANCE\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}

void ALunarGameMode::ApolloGuidanceReview(){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 P->GetCharacterMovement()->DisableMovement();
 auto View=[&](AActor* A,FVector Offset){P->SetActorLocation(A->GetActorLocation()+Offset);FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Shot=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(ReportPath(Name),true,false);};
 switch(MovementPhase){
 case 0:StartApollo(true);RadioRemaining=0;ApolloSaveNoticeRemaining=0;View(ModuleActor,FVector(-550,-120,100));ApolloLookLearned=false;ApolloTutorialAim=PC->GetControlRotation();break;
 case 1:Shot(TEXT("guide-look.png"));break;
 case 2:ApolloLookLearned=true;ApolloMoveLearned=false;ApolloTutorialOrigin=P->GetActorLocation();break;
 case 3:Shot(TEXT("guide-walk.png"));break;
 case 4:View(ModuleActor,FVector(-170,0,50));ApolloLookLearned=true;ApolloMoveLearned=true;OpenApolloTool();break;
 case 5:Shot(TEXT("guide-first-check.png"));break;
 case 6:CancelApolloTool();Stage=ELunarStage::ApolloSamples;SelectedSample=0;ApolloPhotos=0;View(SampleActors[0],FVector(-170,0,50));RadioRemaining=0;break;
 case 7:Shot(TEXT("guide-rock.png"));break;
 case 8:ApolloPhotos=2;View(SampleActors[1],FVector(-170,0,50));OpenApolloTool();ApolloToolPhase=1;break;
 case 9:Shot(TEXT("guide-pouch.png"));break;
 case 10:CancelApolloTool();CompleteApolloInteraction(SampleActors[1]);RadioRemaining=0;break;
 case 11:ApolloPackedPreviewRemaining=1.8f;P->UpdateSuit(0);Shot(TEXT("guide-sealed.png"));break;
 case 12:Stage=ELunarStage::ApolloRecover;SurveyorPhotographed=true;ApolloParts=0;SelectedSample=0;ApolloConfirmationRemaining=0;ApolloPackedPreviewRemaining=0;RadioRemaining=0;View(ApolloPartActors[0],FVector(-170,0,50));OpenApolloTool();break;
 case 13:Shot(TEXT("guide-fitting-first.png"));break;
 case 14:ApolloToolPhase=1;break;
 case 15:Shot(TEXT("guide-fitting-second.png"));break;
 case 16:CancelApolloTool();Journal=true;Paused=true;JournalPage=4;break;
 case 17:Shot(TEXT("guide-help.png"));break;
 case 18:Journal=false;Paused=false;Stage=ELunarStage::ApolloSamples;ULunarSettings::Get()->TextScale=1.5f;ULunarSettings::Get()->SubtitleScale=1.5f;View(SampleActors[0],FVector(-170,0,50));Message(TEXT("Photograph this sample in place, then collect and seal it. The field map shows your next destination."));break;
 case 19:Shot(TEXT("guide-large-text.png"));break;
 case 20:Journal=true;Paused=true;JournalPage=4;break;
 case 21:Shot(TEXT("guide-help-large.png"));break;
 case 22:JournalPage=0;break;
 case 23:Shot(TEXT("guide-map-large.png"));break;
 case 24:Journal=false;Paused=false;StartApollo(true);RadioRemaining=0;ApolloSaveNoticeRemaining=0;ULunarSettings::Get()->TextScale=1;ULunarSettings::Get()->SubtitleScale=1;View(ModuleActor,FVector(-800,0,50));P->Interact();break;
 case 25:Shot(TEXT("guide-hint-distance.png"));break;
 case 26:View(ModuleActor,FVector(-170,0,50));PC->SetControlRotation(FRotator(0,180,0));ApolloInputHintRemaining=0;P->Interact();break;
 case 27:Shot(TEXT("guide-hint-facing.png"));break;
 case 28:View(ModuleActor,FVector(-170,0,50));ULunarSettings::Get()->Rebind(TEXT("Interact"),EKeys::B);ApolloInputHintRemaining=0;P->PrimaryClick();break;
 case 29:Shot(TEXT("guide-hint-binding.png"));break;
 case 30:ULunarSettings::Get()->TextScale=1.5f;ULunarSettings::Get()->SubtitleScale=1.5f;View(ModuleActor,FVector(-800,0,50));ApolloInputHintRemaining=0;P->Interact();Message(TEXT("Begin beside Intrepid. Follow the gold marker to the field kit. Your guide shows the controls."));break;
 case 31:Shot(TEXT("guide-hint-large.png"));break;
 case 32:Stage=ELunarStage::ApolloRecover;SurveyorPhotographed=false;View(DroneActor,FVector(-900,0,50));ApolloInputHintRemaining=0;P->Interact();break;
 case 33:Shot(TEXT("guide-hint-surveyor.png"));break;
 default:FPlatformMisc::RequestExitWithStatus(false,0);return;
 }
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloGuidanceReview,1.5f,false);
}
