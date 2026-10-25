#include "LunarGame.h"
#include "LunarSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void ALunarGameMode::ApolloFieldworkChecks(){
 FString Report;int Failed=0;auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto Aim=[&](AActor* A){P->GetCharacterMovement()->StopMovementImmediately();P->SetActorLocation(A->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Click=[&](){TickApollo(.85f);P->PrimaryClick();};
 StartApollo(true);Stage=ELunarStage::ApolloSamples;ApolloInstruments=3;ApolloPhotos=1;Aim(SampleActors[0]);P->Interact();P->UpdateSuit(0);
 Check(ApolloTool==4&&P->WorkSourceMesh&&P->ToolJaws.Num()==2,TEXT("Rock work loads a source mesh and articulated tongs"));
 Check(P->WorkTool&&P->WorkTool->GetStaticMesh()&&P->ToolJaws[0]->GetStaticMesh(),TEXT("Preloaded tool base and both jaw assets are available"));
 const FVector Tip=P->WorkTool->GetComponentTransform().TransformPosition(FVector(22,0,-4));
 Check(Tip.Equals(P->WorkContact,.2f)&&P->WorkGripActive,TEXT("Tongs tip contacts the actual rock and drives hand grip anchors"));
 Check(P->Gloves[9]->GetRelativeLocation().Equals(P->WorkGrip[1]-FVector(5,0,0),.2f),TEXT("The current camera's grip and right wrist agree in the same frame"));
 Click();P->UpdateSuit(0);Check(!SamplesCollected&&!P->WorkSourceMesh->IsVisible()&&P->WorkPayload->IsVisible(),TEXT("Lift transfers the pending rock into the tongs without awarding cargo"));
 P->PrimaryClick();Check(!SamplesCollected&&ApolloToolPhase==1,TEXT("Rapid repeated input cannot skip the lift animation"));
 const FVector Before=P->WorkPayload->GetComponentLocation();TickApollo(.4f);P->UpdateSuit(0);const FVector During=P->WorkPayload->GetComponentLocation();
 Check(!Before.Equals(During,1),TEXT("Pending rock moves continuously from the source toward the pouch"));
 CancelApolloTool();P->UpdateSuit(0);Check(P->WorkSourceMesh->IsVisible()&&!P->WorkPayload->IsVisible()&&!SamplesCollected,TEXT("Cancel restores the original rock and removes pending payload"));
 P->Interact();Click();Click();P->UpdateSuit(0);Check(SamplesCollected==1&&SampleActors[0]->IsHidden()&&P->SamplePouch->IsVisible(),TEXT("Final seal commits the rock and leaves a physical labeled stow preview"));
 TickApollo(3);P->UpdateSuit(0);Check(!P->SamplePouch->IsVisible(),TEXT("Completed stow preview ends without retaining a floating pouch"));
 ApolloPhotos|=4;Aim(SampleActors[2]);P->Interact();Click();P->UpdateSuit(0);
 Check(ApolloTool==5&&P->WorkTool->IsVisible()&&!P->DrivenTube->IsVisible(),TEXT("Tube placement has one moving tube rather than a duplicate"));
 TickApollo(.8f);P->UpdateSuit(0);Check(P->DrivenTube->IsVisible()&&P->WorkTool->GetStaticMesh()==P->ApolloToolMeshes.FindRef(TEXT("SM_GeologyHammer")),TEXT("Placed tube remains at the surface while the hammer is held"));
 for(int I=0;I<5;++I)Click();Check(!(SamplesCollected&4)&&ApolloToolPhase==6,TEXT("Four hammer strokes and withdrawal still require the final cap"));
 Click();P->UpdateSuit(0);Check((SamplesCollected&4)&&!ApolloTool&&P->WorkGripActive,TEXT("Capping commits the core and keeps both grips on its stow preview"));
 Stage=ELunarStage::ApolloRecover;SamplesCollected=7;ApolloPhotos=7;SurveyorPhotographed=true;ApolloEvidence.Reset();for(int I=0;I<4;++I)RecordApolloEvidence(I,TEXT(""));
 Aim(ApolloPartActors[2]);P->Interact();P->UpdateSuit(0);
 Check(ApolloTool==6&&P->WorkTool->GetComponentTransform().TransformPosition(FVector(11,0,0)).Equals(ApolloContactPoint(),.2f),TEXT("Cutter tips meet the first physical cable fitting"));
 const FVector First=ApolloContactPoint();Click();Check(ApolloContactPoint().Equals(First,.2f),TEXT("Contact remains at the first fitting while the cutting motion plays"));
 TickApollo(.8f);Check(!ApolloContactPoint().Equals(First,1),TEXT("The tool advances to the second fitting only after its motion"));
 Click();Check(!ApolloParts,TEXT("Two released cable ends are not packed hardware"));Click();P->UpdateSuit(0);Check(ApolloParts==4&&P->WorkPayload->IsVisible(),TEXT("Packing transfers the actual hardware mesh into a visible carry preview"));
 Check(CanStudyDust()&&ApolloDustActors.Num()==2,TEXT("Optional coating study is available after the Surveyor context photo"));
 ToggleDustStudy();Check(ApolloDustMode,TEXT("K starts the optional investigation near Surveyor"));
 const uint8 Cargo=ApolloParts;const auto MainStage=Stage;
 for(int I=0;I<2;++I){Aim(ApolloDustActors[I]);P->Interact();Check(ApolloTool==1,TEXT("Each coating face opens a context camera"));PC->SetControlRotation(PC->GetControlRotation()+FRotator(0,30,0));Click();Check(!(ApolloDustPhotos&(1<<I)),TEXT("A misframed coating photograph cannot be awarded"));Aim(ApolloDustActors[I]);Click();Check(ApolloDustPhotos&(1<<I),TEXT("An aligned coating photograph creates its own evidence record"));}
 Check(ApolloEvidence.Num()==6&&ApolloParts==Cargo&&Stage==MainStage,TEXT("Two optional photographs do not change main cargo or mission stage"));
 Aim(ApolloDustActors[0]);P->Interact();Check(ApolloTool==9,TEXT("Both photographs unlock the evidence interpretation"));
 CycleApollo();Click();Check(!ApolloDustCompared&&ApolloTool==9,TEXT("An unsupported explanation is rejected without awarding the study"));
 CycleApollo();CycleApollo();Click();Check(ApolloDustCompared&&!ApolloDustMode&&!ApolloTool,TEXT("The exhaust interpretation records the study and returns to the main task"));
 auto* Save=CaptureApolloCheckpoint();FString Why;Check(Save&&Save->Validate(Why)&&Save->Evidence.Num()==6&&Save->Has(ApolloTasks::DustCompared),TEXT("Six-photo optional progress forms a valid backward-compatible checkpoint"));
 const FString Directory=FPaths::ProjectSavedDir()/TEXT("AutomatedTests/Fieldwork");
 Check(Save&&ApolloCheckpointStorage::Write(Directory,Save),TEXT("Optional evidence can be serialized through the real checkpoint writer"));EApolloSaveStatus Status;auto* Read=ApolloCheckpointStorage::Read(Directory,Status);
 Check(Read&&Read->Evidence.Num()==6&&Read->Has(ApolloTasks::DustCompared),TEXT("A fresh checkpoint read retains all six evidence records and the comparison"));
 if(Save){Save->CompletedTasks.Remove(FName(ApolloTasks::DustPhotos[1]));Check(!Save->Validate(Why),TEXT("Comparison without both prerequisite photos is rejected"));}
 FString Fixture;FParse::Value(FCommandLine::Get(),TEXT("FieldworkLegacyFixture="),Fixture);
 const FString Legacy=FPaths::ProjectSavedDir()/TEXT("AutomatedTests/FieldworkLegacy");IFileManager::Get().MakeDirectory(*Legacy,true);
 const bool Copied=!Fixture.IsEmpty()&&IFileManager::Get().Copy(*ApolloCheckpointStorage::Primary(Legacy),*Fixture)==COPY_OK;
 auto* Old=Copied?ApolloCheckpointStorage::Read(Legacy,Status):nullptr;
 Check(Old&&Old->Validate(Why)&&Old->Evidence.Num()==4&&!Old->Has(ApolloTasks::DustCompared),TEXT("The actual v0.5.2 binary checkpoint opens without requiring optional tasks"));
 auto* Settings=ULunarSettings::Get();Settings->Bindings.Add(TEXT("Route"),EKeys::K);Check(Settings->Key(TEXT("Route"))==EKeys::Q,TEXT("An old K route binding falls back safely to Q when K becomes the study key"));Settings->Reset();
 Check(!ULunarSettings::Get()->Rebind(TEXT("Interact"),EKeys::K),TEXT("The fixed study key cannot be rebound onto an action"));
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("fieldwork-checks.txt")));UE_LOG(LogTemp,Display,TEXT("APOLLO_FIELDWORK\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}

void ALunarGameMode::ApolloFieldworkReview(){
 // Keep the next camera position/tool stable until the previous write completes.
 if(ApolloPhotoPending){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloFieldworkReview,.1f,false);return;}
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 auto View=[&](AActor* A,FVector Offset){P->SetActorLocation(A->GetActorLocation()+Offset);FVector Eye;FRotator R;P->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((A->GetActorLocation()-Eye).Rotation());};
 auto Shot=[&](const TCHAR* Name){const FString Filename(Name);GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this,Filename](){FScreenshotRequest::RequestScreenshot(ReportPath(*Filename),true,false);}));};
 Paused=false;
 switch(MovementPhase){
 case 0:StartApollo(true);RadioRemaining=0;P->GetCharacterMovement()->DisableMovement();Stage=ELunarStage::ApolloSamples;ApolloPhotos=1;View(SampleActors[0],FVector(-170,0,50));OpenApolloTool();break;
 case 1:Shot(TEXT("fieldwork-contact.png"));break;
 case 2:ApolloToolClick();ApolloToolCooldown=.35f;P->UpdateSuit(0);break;
 case 3:Shot(TEXT("fieldwork-lift.png"));break;
 case 4:ApolloToolCooldown=0;P->UpdateSuit(0);break;
 case 5:UE_LOG(LogTemp,Display,TEXT("FIELDWORK_VIEW camera=%s scale=%s pouch=%s visible=%d mesh=%s tool=%s grip0=%s grip1=%s"),*P->Camera->GetComponentLocation().ToString(),*P->Camera->GetComponentScale().ToString(),*P->SamplePouch->GetComponentLocation().ToString(),P->SamplePouch->IsVisible(),*GetNameSafe(P->SamplePouch->GetStaticMesh()),*P->WorkTool->GetComponentLocation().ToString(),*P->WorkGrip[0].ToString(),*P->WorkGrip[1].ToString());Shot(TEXT("fieldwork-pouch.png"));break;
 case 6:CancelApolloTool();ApolloPhotos|=4;View(SampleActors[2],FVector(-170,0,50));OpenApolloTool();ApolloToolPhase=3;ApolloToolCooldown=.5f;break;
 case 7:Shot(TEXT("fieldwork-core.png"));break;
 case 8:CancelApolloTool();Stage=ELunarStage::ApolloRecover;SurveyorPhotographed=true;View(ApolloPartActors[2],FVector(-170,0,50));OpenApolloTool();break;
 case 9:Shot(TEXT("fieldwork-cable.png"));break;
 case 10:CancelApolloTool();View(DroneActor,FVector(-720,-430,140));break;
 case 11:Shot(TEXT("fieldwork-surveyor.png"));break;
 case 12:ToggleDustStudy();View(ApolloDustActors[0],FVector(-220,0,30));OpenApolloTool();break;
 case 13:Shot(TEXT("fieldwork-exposed.png"));break;
 case 14:CaptureDustPhoto();break;
 case 15:View(ApolloDustActors[1],FVector(-220,0,30));OpenApolloTool();Shot(TEXT("fieldwork-shielded.png"));break;
 case 16:CaptureDustPhoto();break;
 case 17:View(ApolloDustActors[0],FVector(-220,0,30));OpenApolloTool();RadioRemaining=0;Shot(TEXT("fieldwork-compare.png"));break;
 case 18:ApolloDustCompared=true;ULunarSettings::Get()->TextScale=1.5f;break;
 case 19:Shot(TEXT("fieldwork-large.png"));break;
 case 20:CancelApolloTool();ApolloDustMode=false;P->OpenEvidence();break;
 case 21:Shot(TEXT("fieldwork-album.png"));break;
 case 22:P->ClosePanel();ULunarSettings::Get()->TextScale=1;View(HomeActor,FVector(-1400,-700,180));break;
 case 23:Shot(TEXT("fieldwork-intrepid.png"));break;
 default:FPlatformMisc::RequestExitWithStatus(false,0);return;
 }
 P->UpdateSuit(0);const float Delay=MovementPhase==0?3.f:MovementPhase%2==0?.15f:1.5f;++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloFieldworkReview,Delay,false);
}
