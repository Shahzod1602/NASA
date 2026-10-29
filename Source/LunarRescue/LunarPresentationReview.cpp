#include "LunarGame.h"
#include "LunarSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

void ALunarGameMode::ApolloPresentationReview(){
 // Synthetic scene fixtures for visual inspection only. No camera evidence is
 // captured, no participant took this route, and these numbers are not a run.
 static const TCHAR* Names[]={TEXT("intrepid-wide"),TEXT("intrepid-foot"),TEXT("regolith"),TEXT("tongs-lift"),TEXT("debrief-won"),TEXT("debrief-lost"),TEXT("result150"),TEXT("debrief150")};
 const FString Folder=FPaths::ProjectDir()/TEXT("Reports/ExpeditionPolish20261029");
 IFileManager::Get().MakeDirectory(*Folder,true);
 auto Fail=[&](const FString& Why){
  UE_LOG(LogTemp,Error,TEXT("PRESENTATION_REVIEW fixture failed: %s"),*Why);
  FFileHelper::SaveStringToFile(TEXT("Synthetic review failed: ")+Why,*(Folder/TEXT("presentation-review-error.txt")));
  FPlatformMisc::RequestExitWithStatus(false,1);
 };
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC||!P->Camera){Fail(TEXT("Missing review pawn, controller or camera"));return;}
 auto Later=[&](float Delay){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloPresentationReview,Delay,false);};
 if(MovementPhase>=UE_ARRAY_COUNT(Names)){
  bool Complete=!FScreenshotRequest::IsScreenshotRequested();
  for(const TCHAR* Name:Names)Complete&=IFileManager::Get().FileSize(*(Folder/(TEXT("presentation-")+FString(Name)+TEXT(".png"))))>24;
  if(!Complete){if(FPlatformTime::Seconds()<ApolloTestDeadline){Later(.1f);return;}Fail(TEXT("Screenshot requests did not finish within the fixture deadline"));return;}
  UE_LOG(LogTemp,Display,TEXT("PRESENTATION_REVIEW complete: eight synthetic scenes; not a playtest or performance capture"));
  FPlatformMisc::RequestExitWithStatus(false,0);return;
 }
 if(MovementPhase==0){
  ApolloTestDeadline=FPlatformTime::Seconds()+90;
  const FString Manifest=TEXT("{\n  \"synthetic_review_fixtures\": true,\n  \"human_playtest\": false,\n  \"performance_measurement\": false,\n  \"description\": \"Staged camera positions, cargo masks and elapsed/distance values for layout review. No in-game camera captures or user photographs are created.\"\n}\n");
  if(!FFileHelper::SaveStringToFile(Manifest,*(Folder/TEXT("presentation-fixtures.json")))){Fail(TEXT("Could not document synthetic fixtures"));return;}
  StartApollo(true);
 }
 P->ClosePanel();CancelApolloTool();Paused=false;Journal=false;RadioRemaining=0;ApolloSaveNoticeRemaining=0;
 P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();P->FastStride=false;
 P->ThirdPerson=false;P->Camera->bUsePawnControlRotation=true;P->Camera->SetRelativeLocation(FVector(0,0,68));P->Camera->SetRelativeRotation(FRotator::ZeroRotator);
 P->Camera->SetFieldOfView(85);
 auto View=[&](const FVector& Eye,const FVector& Target){P->SetActorLocation(Eye-FVector(0,0,68),false);PC->SetControlRotation((Target-Eye).Rotation());};
 auto ResultFixture=[&](bool Won){
  StartApollo(true);ApolloTimed=!Won;ApolloElapsed=Won?622.f:1800.f;ApolloDistance=Won?1238.f:790.f;
  Oxygen=Won?OxygenCapacity:0;ApolloInstruments=3;ApolloPhotos=Won?7:3;SamplesCollected=Won?7:1;
  ApolloParts=Won?7:0;SurveyorPhotographed=Won;ApolloDustPhotos=0;ApolloDustCompared=false;
  // Empty paths retain metadata-only illustrative entries, never user images.
  for(int32 I=0;I<(Won?4:2);++I)RecordApolloEvidence(I,TEXT(""));
  Stage=Won?ELunarStage::Won:ELunarStage::Lost;
  FailureReason=TEXT("Timed exercise ended. Synthetic partial-cargo review fixture.");
  RadioRemaining=0;ApolloSaveNoticeRemaining=0;
 };
 ULunarSettings::Get()->TextScale=MovementPhase>=6?1.5f:1.f;
 switch(MovementPhase){
 case 0:
 case 1:{
  Stage=ELunarStage::ApolloCheckout;AActor* Lander=nullptr;
  for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("apollo.reference-lander"))){Lander=*It;break;}
  if(!Lander){Fail(TEXT("Reference Intrepid model missing"));return;}
  const FBox WorldBounds=Lander->GetComponentsBoundingBox(true);
  if(!WorldBounds.IsValid){Fail(TEXT("Reference Intrepid world bounds invalid"));return;}
  if(MovementPhase==0){const FVector Center=WorldBounds.GetCenter();View(Center+FVector(-1300,-1050,300),Center+FVector(0,0,-60));}
  else{
   auto* Mesh=Lander->FindComponentByClass<UStaticMeshComponent>();
   const float Uniform=float(Lander->GetActorScale3D().X);
   if(!Mesh||!Mesh->GetStaticMesh()||Uniform<=0){Fail(TEXT("Reference foot geometry missing"));return;}
   const auto Bounds=Mesh->GetStaticMesh()->GetBounds();
   const FVector Datum(Bounds.Origin.X,Bounds.Origin.Y,Bounds.Origin.Z-Bounds.BoxExtent.Z);
   const FVector Foot=Lander->GetActorTransform().TransformPosition(Datum+FVector(397.53f,-5.19f,0)/Uniform);
   View(Foot+FVector(220,-160,110),Foot+FVector(0,0,15));P->PhotoCaptureFrames=2;
  }break;
 }
 case 2:{
  Stage=ELunarStage::ApolloCheckout;
  if(!SampleActors.IsValidIndex(0)||!SampleActors[0]){Fail(TEXT("Sample site missing for nearby regolith view"));return;}
  const FVector Site=SampleActors[0]->GetActorLocation();View(Site+FVector(-300,-240,175),Site+FVector(0,100,-10));break;
 }
 case 3:{
  Stage=ELunarStage::ApolloSamples;ApolloInstruments=3;ApolloPhotos=1;SamplesCollected=0;
  if(!SampleActors.IsValidIndex(0)||!SampleActors[0]){Fail(TEXT("Rock site missing for tongs fixture"));return;}
  auto* Rock=SampleActors[0].Get();Rock->SetActorHiddenInGame(false);
  P->SetActorLocation(Rock->GetActorLocation()+FVector(-170,0,50),false);
  FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);PC->SetControlRotation((Rock->GetActorLocation()-Eye).Rotation());
  if(!OpenApolloTool()||ApolloTool!=4){Fail(TEXT("Actual rock tongs interaction could not open"));return;}
  ApolloToolClick();
  if(ApolloToolPhase!=1||SamplesCollected||ApolloToolFeedbackRemaining<=0){Fail(TEXT("Actual lift did not produce uncommitted in-progress feedback"));return;}
  break;
 }
 case 4:ResultFixture(true);P->OpenEvidence();if(!P->ActivePanel){Fail(TEXT("Won debrief panel missing"));return;}break;
 case 5:ResultFixture(false);P->OpenEvidence();if(!P->ActivePanel){Fail(TEXT("Partial-cargo debrief panel missing"));return;}break;
 case 6:ResultFixture(true);break;
 case 7:ResultFixture(true);P->OpenEvidence();if(!P->ActivePanel){Fail(TEXT("Large-text debrief panel missing"));return;}break;
 }
 P->UpdateSuit(0);
 const FString Shot=Folder/(TEXT("presentation-")+FString(Names[MovementPhase])+TEXT(".png"));
 UE_LOG(LogTemp,Display,TEXT("PRESENTATION_REVIEW synthetic scene %d: %s"),MovementPhase,*Shot);
 // Request in the same invocation as the actual lift: the 1.6-second feedback
 // must still be present. Subsequent scene changes wait two seconds.
 FScreenshotRequest::RequestScreenshot(Shot,true,false);
 ++MovementPhase;Later(2.f);
}
