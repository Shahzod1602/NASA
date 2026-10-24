#include "LunarGame.h"
#include "LunarApolloText.h"
#include "ApolloData.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace {
UStaticMeshComponent* Part(AActor* A,const TCHAR* Shape,const TCHAR* Material,FVector P,FVector Scale,FRotator R=FRotator::ZeroRotator) {
 auto* C=NewObject<UStaticMeshComponent>(A);C->SetupAttachment(A->GetRootComponent());
 C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
 C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Materials/%s.%s"),Material,Material)));
 C->SetMobility(EComponentMobility::Movable);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 C->SetRelativeLocation(P);C->SetRelativeScale3D(Scale);C->SetRelativeRotation(R);C->RegisterComponent();return C;
}
void Strut(AActor* A,FVector From,FVector To,float Radius=3) {
 const FVector D=To-From;
 Part(A,TEXT("Cylinder"),TEXT("M_Ceramic"),(From+To)*.5,FVector(Radius/50,Radius/50,D.Size()/100),FRotationMatrix::MakeFromZ(D).Rotator());
}
void Sign(AActor* A,const FString& Text,FVector P,float Size=15) {
 auto* C=NewObject<UTextRenderComponent>(A);C->SetupAttachment(A->GetRootComponent());C->SetText(FText::FromString(Text));
 C->SetWorldSize(Size);C->SetTextRenderColor(FColor(210,220,210));C->SetRelativeLocation(P);C->SetRelativeRotation(FRotator(0,180,0));C->SetHorizontalAlignment(EHTA_Center);C->RegisterComponent();
}
int32 IndexOf(const TArray<TObjectPtr<AActor>>& Actors,AActor* A) {return Actors.IndexOfByPredicate([A](const TObjectPtr<AActor>& B){return B.Get()==A;});}
int32 Bits(uint8 Mask){int32 N=0;for(int32 I=0;I<8;++I)if(Mask&(1<<I))++N;return N;}
}

void ALunarGameMode::BuildApollo() {
 OxygenCapacity=1800;Oxygen=OxygenCapacity;
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(PC){PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;}
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0))){P->GetCharacterMovement()->MaxWalkSpeed=140;P->GetCharacterMovement()->JumpZVelocity=190;ApolloLastPosition=P->GetActorLocation();}
 ApolloMapTexture=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Apollo12/Textures/T_Apollo12_Orbital.T_Apollo12_Orbital"));
 auto Site=[&](FVector P){
  auto* A=GetWorld()->SpawnActor<AActor>();auto* Root=NewObject<USceneComponent>(A);A->SetRootComponent(Root);Root->RegisterComponent();
  // All coordinates are dataset-derived centimetres; ground trace checks imported collision.
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;
  if(GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,5000),P-FVector(0,0,5000),ECC_Visibility,Q))P.Z=Hit.ImpactPoint.Z;
  A->SetActorLocation(P+FVector(0,0,65));return A;
 };
 ModuleActor=Site(ApolloData::Check);HomeActor=ModuleActor;
 ModuleActor->Tags.Add(TEXT("intrepid.field-kit"));
 auto* KitBody=Part(ModuleActor,TEXT("Cube"),TEXT("M_Ceramic"),FVector(0,0,-12),FVector(.95,.7,.45));
 KitBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);KitBody->SetCollisionResponseToAllChannels(ECR_Ignore);KitBody->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
 Part(ModuleActor,TEXT("Cube"),TEXT("M_ThermalFoil"),FVector(0,0,14),FVector(.75,.54,.08));
 for(float Y:{-26.f,26.f})Strut(ModuleActor,FVector(-25,Y,-33),FVector(-32,Y,-65),2);
 Sign(ModuleActor,TEXT("INTREPID / FIELD KIT"),FVector(-50,0,38),4);
 for(int32 I=0;I<2;++I){
  auto* A=Site(ApolloData::Instruments[I]);A->Tags.Add(FName(ApolloTasks::Instruments[I]));ApolloInstrumentActors.Add(A);
  if(I==0){
   Part(A,TEXT("Cylinder"),TEXT("M_ThermalFoil"),FVector(0,0,-40),FVector(.6,.6,.35));
   Part(A,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector(0,0,-21),FVector(.7,.7,.05));
   for(int J=0;J<3;++J){float T=J*2*PI/3;Strut(A,FVector(0,0,-45),FVector(50*FMath::Cos(T),50*FMath::Sin(T),-64),1.5);}
   Sign(A,TEXT("ALSEP / PSE"),FVector(-45,0,12),12);
  }else{
   Part(A,TEXT("Cube"),TEXT("M_ThermalFoil"),FVector(0,0,-35),FVector(.8,.65,.5));
   Part(A,TEXT("Cube"),TEXT("M_Ceramic"),FVector(0,0,-8),FVector(.95,.8,.06));
   Strut(A,FVector(0,0,-8),FVector(0,0,85),2);
   ApolloAntenna=Part(A,TEXT("Cube"),TEXT("M_Ceramic"),FVector(0,0,90),FVector(.45,.18,.22),FRotator(-65,ApolloAntennaAngle,0));
   Sign(A,TEXT("ALSEP / CENTRAL STATION"),FVector(-50,0,40),12);
  }
 }
 for(int32 I=0;I<3;++I){
  auto* A=Site(ApolloData::Samples[I]);A->Tags.Add(FName(ApolloTasks::Sites[I]));SampleActors.Add(A);
  auto* Rock=Part(A,TEXT("Sphere"),TEXT("M_Graphite"),FVector(0,0,-55),FVector(.25,.2,.16));
  if(auto* M=LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_%d.SM_Basalt_%d"),I,I)))Rock->SetStaticMesh(M);
  if(I==2)Part(A,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector(0,0,-35),FVector(.04,.04,.6));
  // Small virtual gnomon identifies the work area; it is not a permanent historical flag.
  for(int J=0;J<3;++J){float T=J*2*PI/3;Strut(A,FVector(35,0,-30),FVector(35+22*FMath::Cos(T),22*FMath::Sin(T),-64),.8);}
 }
 DroneActor=Site(ApolloData::Surveyor);DroneActor->Tags.Add(TEXT("Surveyor3"));
 // Stationary three-legged Surveyor silhouette: open truss, solar panel, camera and scoop.
 Part(DroneActor,TEXT("Cube"),TEXT("M_ThermalFoil"),FVector(0,0,10),FVector(.8,.8,.55));
 Part(DroneActor,TEXT("Sphere"),TEXT("M_Ceramic"),FVector(0,0,40),FVector(.65));
 for(int32 I=0;I<3;++I){
  const float T=I*2*PI/3;FVector Foot(155*FMath::Cos(T),155*FMath::Sin(T),-65);
  Part(DroneActor,TEXT("Cylinder"),TEXT("M_Ceramic"),Foot+FVector(0,0,5),FVector(.48,.48,.09));
  Strut(DroneActor,FVector(0,0,55),Foot+FVector(0,0,12),3.3);
  Strut(DroneActor,FVector(-35*FMath::Sin(T),35*FMath::Cos(T),-10),Foot+FVector(0,0,12),2.1);
 }
 Strut(DroneActor,FVector(10,0,20),FVector(10,0,220),2.8);
 Part(DroneActor,TEXT("Cube"),TEXT("M_Ceramic"),FVector(10,0,205),FVector(1.4,.85,.045),FRotator(0,20,22));
 Part(DroneActor,TEXT("Cube"),TEXT("M_Solar"),FVector(10,0,209),FVector(1.32,.78,.025),FRotator(0,20,22));
 Part(DroneActor,TEXT("Cube"),TEXT("M_Ceramic"),FVector(10,0,255),FVector(.65,.65,.06));
 Strut(DroneActor,FVector(-15,-25,10),FVector(-92,-65,-25),3);
 Strut(DroneActor,FVector(-92,-65,-25),FVector(-155,-70,-52),2.3);
 const FVector Offsets[]={FVector(-35,25,115),FVector(-155,-70,-50),FVector(25,-40,0)};
 for(int32 I=0;I<3;++I){
  auto* A=GetWorld()->SpawnActor<AActor>();auto* Root=NewObject<USceneComponent>(A);A->SetRootComponent(Root);Root->RegisterComponent();A->SetActorLocation(DroneActor->GetActorLocation()+Offsets[I]);A->Tags.Add(FName(ApolloTasks::Parts[I]));ApolloPartActors.Add(A);
  if(I==0){Part(A,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector::ZeroVector,FVector(.2,.2,.55));Part(A,TEXT("Sphere"),TEXT("M_Graphite"),FVector(-10,0,12),FVector(.09));}
  else if(I==1)Part(A,TEXT("Cube"),TEXT("M_Ceramic"),FVector::ZeroVector,FVector(.3,.2,.12),FRotator(0,0,20));
  else for(int32 J=0;J<4;++J)Part(A,TEXT("Cylinder"),TEXT("M_Graphite"),FVector(0,J*3,0),FVector(.018,.018,.4),FRotator(0,0,25));
 }
 for(const FVector& P:ApolloData::Approach)ApolloRouteActors.Add(Site(P));
 Message(ApolloText::Get(TEXT("ReadyMessage")));
 if(FParse::Param(FCommandLine::Get(),TEXT("ApolloTest"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloChecks,3,false);}
 if(FParse::Param(FCommandLine::Get(),TEXT("ApolloReview"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloReview,7,false);}
 ApolloWalking=FParse::Param(FCommandLine::Get(),TEXT("ApolloWalkTest"));
 RefreshApolloCheckpoint();
 if(UGameplayStatics::ParseOption(OptionsString,TEXT("ResumeApollo"))==TEXT("1")){
  FTimerHandle H;GetWorldTimerManager().SetTimer(H,[this](){RestoreApolloCheckpoint();},.2f,false);
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveWriteTest"))||FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveReadTest"))){FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ApolloSaveChecks,3,false);}
}
void ALunarGameMode::StartApollo(bool Confirmed){
 if(!Confirmed&&ApolloSaveStatus!=EApolloSaveStatus::Missing){ApolloConfirmNew=true;return;}
 ApolloConfirmNew=false;ApolloStarted=true;ApolloRunId=FGuid::NewGuid();ApolloEvidence.Reset();
 ApolloElapsed=0;ApolloDistance=0;Oxygen=OxygenCapacity;ApolloPhotos=0;ApolloParts=0;ApolloInstruments=0;SamplesCollected=0;SelectedSample=0;SurveyorPhotographed=false;
 for(auto& Actor:SampleActors)Actor->SetActorHiddenInGame(false);for(auto& Actor:ApolloPartActors)Actor->SetActorHiddenInGame(false);
 Stage=ELunarStage::ApolloCheckout;ApolloAction=0;
 ApolloLookLearned=false;ApolloMoveLearned=false;
 if(auto* P=UGameplayStatics::GetPlayerPawn(this,0)){ApolloTutorialOrigin=P->GetActorLocation();ApolloTutorialAim=P->GetControlRotation();}
 Message(ApolloText::Get(TEXT("StartMessage")));
 SaveApolloCheckpoint();
}
int32 ALunarGameMode::ApolloStep() const {return Stage==ELunarStage::Won?6:Stage>=ELunarStage::ApolloCheckout&&Stage<=ELunarStage::ApolloReturn?int32(Stage)-int32(ELunarStage::ApolloCheckout):0;}
AActor* ALunarGameMode::ApolloTarget(bool Nearby) const {
 auto Pending=[&](const TArray<TObjectPtr<AActor>>& Actors,uint8 Mask)->AActor* {
  if(Nearby){
   AActor* Best=nullptr;float BestScore=-2;auto* P=UGameplayStatics::GetPlayerPawn(this,0);FVector Eye;FRotator Aim;
   if(P)P->GetActorEyesViewPoint(Eye,Aim);
   for(int32 I=0;I<Actors.Num();++I)if(!(Mask&(1<<I))&&CanInteractWith(Actors[I])){
    const float Score=FVector::DotProduct(Aim.Vector(),(Actors[I]->GetActorLocation()-Eye).GetSafeNormal());
    if(Score>BestScore){BestScore=Score;Best=Actors[I];}
   }
   if(Best)return Best;
  }
  if(Actors.IsValidIndex(SelectedSample)&&!(Mask&(1<<SelectedSample)))return Actors[SelectedSample];
  for(int32 I=0;I<Actors.Num();++I)if(!(Mask&(1<<I)))return Actors[I];return nullptr;
 };
 switch(Stage){
 case ELunarStage::ApolloCheckout:return ModuleActor;
 case ELunarStage::ApolloDeploy:return ApolloInstrumentActors.IsValidIndex(ApolloAction)?ApolloInstrumentActors[ApolloAction].Get():nullptr;
 case ELunarStage::ApolloSamples:return Pending(SampleActors,SamplesCollected);
 case ELunarStage::ApolloTraverse:return DroneActor;
 case ELunarStage::ApolloRecover:return SurveyorPhotographed?Pending(ApolloPartActors,ApolloParts):DroneActor.Get();
 case ELunarStage::ApolloReturn:return HomeActor;
 default:return nullptr;
 }
}
FString ALunarGameMode::ApolloObjective() const {
 switch(Stage){
 case ELunarStage::ApolloCheckout:return ApolloText::Get(TEXT("ObjectiveCheckout"));
 case ELunarStage::ApolloDeploy:return ApolloText::Get(TEXT("ObjectiveDeploy"));
 case ELunarStage::ApolloSamples:return ApolloText::Format(TEXT("ObjectiveSamples"),{{TEXT("Count"),SampleCount()}});
 case ELunarStage::ApolloTraverse:return ApolloText::Get(TEXT("ObjectiveTraverse"));
 case ELunarStage::ApolloRecover:return ApolloText::Format(TEXT("ObjectiveRecover"),{{TEXT("Count"),Bits(ApolloParts)}});
 case ELunarStage::ApolloReturn:return ApolloText::Get(TEXT("ObjectiveReturn"));
 default:return ApolloText::Get(TEXT("ObjectiveDefault"));
 }
}
float ALunarGameMode::ApolloHoldDuration() const {
 if(Stage==ELunarStage::ApolloCheckout)return 2.5f;
 if(Stage==ELunarStage::ApolloDeploy)return 4;
 if(Stage==ELunarStage::ApolloSamples){int32 I=IndexOf(SampleActors,ApolloTarget(true));return I>=0&&(ApolloPhotos&(1<<I))?4.f:0.f;}
 if(Stage==ELunarStage::ApolloRecover)return SurveyorPhotographed?5.f:0.f;
 if(Stage==ELunarStage::ApolloReturn)return 4;
 return 0;
}
FString ALunarGameMode::ApolloPrompt() const {
 if(Stage==ELunarStage::ApolloCheckout){const TCHAR* Tasks[]={TEXT("PromptCheckout0"),TEXT("PromptCheckout1"),TEXT("PromptCheckout2")};return ApolloText::Get(Tasks[FMath::Clamp(ApolloAction,0,2)]);}
 if(Stage==ELunarStage::ApolloDeploy)return ApolloText::Get(ApolloAction==0?TEXT("PromptSeismometer"):TEXT("PromptAntenna"));
 if(Stage==ELunarStage::ApolloSamples)return ApolloText::Get(ApolloHoldDuration()>0?TEXT("PromptSample"):TEXT("PromptPhoto"));
 if(Stage==ELunarStage::ApolloRecover){if(!SurveyorPhotographed)return ApolloText::Get(TEXT("PromptSurveyorPhoto"));int32 I=IndexOf(ApolloPartActors,ApolloTarget(true));const TCHAR* Parts[]={TEXT("PromptCamera"),TEXT("PromptScoop"),TEXT("PromptCable")};return ApolloText::Get(Parts[FMath::Clamp(I,0,2)]);}
 if(Stage==ELunarStage::ApolloReturn)return ApolloText::Get(TEXT("PromptReturn"));
 return ApolloText::Get(TEXT("PromptWalk"));
}
FString ALunarGameMode::ApolloSiteName() const {
 switch(Stage){
 case ELunarStage::ApolloCheckout:case ELunarStage::ApolloReturn:return ApolloText::Get(TEXT("SiteKit"));
 case ELunarStage::ApolloDeploy:return ApolloText::Get(ApolloAction==0?TEXT("SiteSeismometer"):TEXT("SiteAntenna"));
 case ELunarStage::ApolloSamples:{const TCHAR* N[]={TEXT("HEAD / 12031"),TEXT("BENCH / 12032"),TEXT("SHARP / 12027")};return N[FMath::Clamp(SelectedSample,0,2)];}
 case ELunarStage::ApolloTraverse:return TEXT("SURVEYOR 3");
 case ELunarStage::ApolloRecover:{
  if(!SurveyorPhotographed)return TEXT("SURVEYOR 3");
  const TCHAR* Parts[]={TEXT("SiteCamera"),TEXT("SiteScoop"),TEXT("SiteCable")};
  return ApolloText::Get(Parts[FMath::Clamp(SelectedSample,0,2)]);
 }
 default:return ApolloText::Get(TEXT("SiteNext"));
 }
}
FString ALunarGameMode::ApolloInstruction() const {
 switch(Stage){
 case ELunarStage::ApolloCheckout:{const TCHAR* A[]={TEXT("InstructionCheckout0"),TEXT("InstructionCheckout1"),TEXT("InstructionCheckout2")};return ApolloText::Get(A[FMath::Clamp(ApolloAction,0,2)]);}
 case ELunarStage::ApolloDeploy:return ApolloAction==0?ApolloText::Get(TEXT("InstructionSeismometer")):ApolloText::Format(TEXT("InstructionAntenna"),{{TEXT("Angle"),FMath::RoundToInt(FMath::Fmod(ApolloAntennaAngle+360.f,360.f))}});
 case ELunarStage::ApolloSamples:return ApolloText::Get(TEXT("InstructionSamples"));
 case ELunarStage::ApolloTraverse:return ApolloText::Get(TEXT("InstructionTraverse"));
 case ELunarStage::ApolloRecover:return ApolloText::Get(SurveyorPhotographed?TEXT("InstructionSurveyorParts"):TEXT("InstructionSurveyorPhoto"));
 case ELunarStage::ApolloReturn:return ApolloText::Get(TEXT("InstructionReturn"));
 default:return FString();
 }
}
FString ALunarGameMode::ApolloPurpose() const {
 const TCHAR* Keys[]={TEXT("PurposeCheckout"),TEXT("PurposeDeploy"),TEXT("PurposeSamples"),TEXT("PurposeTraverse"),TEXT("PurposeRecover"),TEXT("PurposeReturn")};
 return IsActive()?ApolloText::Get(Keys[FMath::Clamp(ApolloStep(),0,5)]):FString();
}
FString ALunarGameMode::ApolloTutorial() const {
 if(Stage!=ELunarStage::ApolloCheckout||ApolloAction>0)return FString();
 return ApolloText::Get(!ApolloLookLearned?TEXT("TutorialLook"):!ApolloMoveLearned?TEXT("TutorialMove"):TEXT("TutorialWork"));
}
void ALunarGameMode::CycleApollo(){
 if(Journal){JournalPage=(JournalPage+1)%4;return;}
 if(Paused||!IsActive())return;ReleaseInteract();
 if(Stage==ELunarStage::ApolloDeploy&&ApolloAction==1){
  ApolloAntennaAngle=FMath::UnwindDegrees(ApolloAntennaAngle+15);if(ApolloAntenna)ApolloAntenna->SetRelativeRotation(FRotator(-65,ApolloAntennaAngle,0));return;
 }
 const bool Samples=Stage==ELunarStage::ApolloSamples;
 if(!Samples&&!(Stage==ELunarStage::ApolloRecover&&SurveyorPhotographed))return;
 uint8 Mask=Samples?SamplesCollected:ApolloParts;
 for(int32 I=1;I<=3;++I){int32 Next=(SelectedSample+I)%3;if(!(Mask&(1<<Next))){SelectedSample=Next;return;}}
}
void ALunarGameMode::InteractApollo(){
 if(Stage==ELunarStage::ApolloTraverse)return;
 if(Stage==ELunarStage::ApolloDeploy&&ApolloAction==1&&FMath::Abs(FMath::FindDeltaAngleDegrees(ApolloAntennaAngle,-7.f))>8){Message(ApolloText::Get(TEXT("AntennaWrong")));return;}
 if(ApolloHoldDuration()>0){Holding=true;HoldProgress=0;HeldTarget=InteractionTarget();return;}
 FString PhotoName;int32 EvidenceIndex=INDEX_NONE;
 if(Stage==ELunarStage::ApolloSamples){int32 I=IndexOf(SampleActors,InteractionTarget());if(I<0)return;ApolloPhotos|=1<<I;EvidenceIndex=I;const TCHAR* IDs[]={TEXT("12031"),TEXT("12032"),TEXT("12027")};PhotoName=FString(TEXT("Sample-"))+IDs[I];PlayPowerSound(TEXT("PowerContact"),.3);Message(ApolloText::Get(TEXT("PhotoMessage")));}
 else if(Stage==ELunarStage::ApolloRecover&&!SurveyorPhotographed){SurveyorPhotographed=true;SelectedSample=0;EvidenceIndex=3;PhotoName=TEXT("Surveyor3-before-recovery");PlayPowerSound(TEXT("PowerContact"),.3);Message(ApolloText::Get(TEXT("SurveyorPhotoMessage")));}
 if(EvidenceIndex!=INDEX_NONE){
  FString RelativePath;
  if(FApp::CanEverRender()&&!FParse::Param(FCommandLine::Get(),TEXT("ApolloTest"))&&!FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveTest"))&&!FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveWriteTest"))&&!FParse::Param(FCommandLine::Get(),TEXT("ApolloSaveReadTest"))){
   RelativePath=FString(TEXT("Apollo12/Photos/"))+ApolloRunId.ToString(EGuidFormats::Digits)+TEXT("/")+PhotoName+TEXT(".png");
   IFileManager::Get().MakeDirectory(*FPaths::GetPath(FPaths::ProjectSavedDir()/RelativePath),true);FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/RelativePath,false,false);
  }
  RecordApolloEvidence(EvidenceIndex,RelativePath);SaveApolloCheckpoint();
 }
}
void ALunarGameMode::CompleteApolloInteraction(AActor* A){
 PlayPowerSound(TEXT("PowerContact"),.35);
 if(Stage==ELunarStage::ApolloCheckout){
  ++ApolloAction;if(ApolloAction==3){Stage=ELunarStage::ApolloDeploy;ApolloAction=0;JournalPage=0;Message(ApolloText::Get(TEXT("CheckoutDone")));}
  else Message(ApolloText::Get(ApolloAction==1?TEXT("SuitDone"):TEXT("ToolsDone")));
 }else if(Stage==ELunarStage::ApolloDeploy){
  ApolloInstruments|=1<<ApolloAction;++ApolloAction;
  if(ApolloAction==2){Stage=ELunarStage::ApolloSamples;SelectedSample=0;Message(ApolloText::Get(TEXT("InstrumentsDone")));}
  else Message(ApolloText::Get(TEXT("SeismometerDone")));
 }else if(Stage==ELunarStage::ApolloSamples){
  int32 I=IndexOf(SampleActors,A);if(I<0||!(ApolloPhotos&(1<<I))||SamplesCollected&(1<<I))return;
  SamplesCollected|=1<<I;A->SetActorHiddenInGame(true);
  if(SampleCount()==3){Stage=ELunarStage::ApolloTraverse;ApolloRouteIndex=0;Message(ApolloText::Get(TEXT("SamplesDone")));}
  else {for(int32 N=0;N<3;++N)if(!(SamplesCollected&(1<<N))){SelectedSample=N;break;}Message(ApolloText::Get(TEXT("SampleDone")));}
 }else if(Stage==ELunarStage::ApolloRecover){
  int32 I=IndexOf(ApolloPartActors,A);if(I<0||!SurveyorPhotographed||ApolloParts&(1<<I))return;ApolloParts|=1<<I;A->SetActorHiddenInGame(true);
  if(ApolloParts==7){Stage=ELunarStage::ApolloReturn;Message(ApolloText::Get(TEXT("PartsDone")));}
  else {for(int32 N=0;N<3;++N)if(!(ApolloParts&(1<<N))){SelectedSample=N;break;}Message(ApolloText::Get(TEXT("PartDone")));}
 }else if(Stage==ELunarStage::ApolloReturn&&SamplesCollected==7&&ApolloParts==7&&ApolloInstruments==3){Stage=ELunarStage::Won;Message(ApolloText::Get(TEXT("MissionDone")));}
 SaveApolloCheckpoint();
}
void ALunarGameMode::TickApollo(float D){
 auto* P=UGameplayStatics::GetPlayerPawn(this,0);
 if(!Paused)ApolloSaveNoticeRemaining=FMath::Max(0.f,ApolloSaveNoticeRemaining-FMath::Max(0.f,D));
 if(IsActive()&&!Paused&&P){
  ApolloElapsed+=FMath::Max(0.f,D);float Distance=FVector::Dist2D(P->GetActorLocation(),ApolloLastPosition);if(Distance<300)ApolloDistance+=Distance/100;
  if(Stage==ELunarStage::ApolloCheckout){
   const FRotator Aim=P->GetControlRotation();
   ApolloLookLearned|=FMath::Abs(FMath::FindDeltaAngleDegrees(ApolloTutorialAim.Yaw,Aim.Yaw))>18||FMath::Abs(FMath::FindDeltaAngleDegrees(ApolloTutorialAim.Pitch,Aim.Pitch))>12;
   ApolloMoveLearned|=FVector::Dist2D(P->GetActorLocation(),ApolloTutorialOrigin)>150;
  }
  // The historical traverse remains on the map, but is not an invisible sequence lock.
  if(Stage==ELunarStage::ApolloTraverse&&DroneActor&&FVector::Dist2D(P->GetActorLocation(),DroneActor->GetActorLocation())<900){
   Stage=ELunarStage::ApolloRecover;SelectedSample=0;Message(ApolloText::Get(TEXT("SurveyorReached")));
   SaveApolloCheckpoint();
  }
  if(auto* Character=Cast<ALunarCharacter>(P);Character&&Character->GetCharacterMovement()->IsMovingOnGround()){ApolloSafePosition=P->GetActorLocation();ApolloSafeView=P->GetControlRotation();ApolloHasSafePosition=true;}
 }
 if(P)ApolloLastPosition=P->GetActorLocation();
 if(ApolloWalking)ApolloWalkTick(D);
}
