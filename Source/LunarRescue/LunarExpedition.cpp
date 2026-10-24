#include "LunarGame.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

namespace {
UStaticMeshComponent* ExpeditionPart(AActor* Owner,const TCHAR* Shape,const TCHAR* Material,FVector Offset,FVector Scale,FRotator Rotation=FRotator::ZeroRotator) {
 auto* Part=NewObject<UStaticMeshComponent>(Owner);Part->SetupAttachment(Owner->GetRootComponent());
 Part->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
 Part->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Materials/%s.%s"),Material,Material)));
 Part->SetMobility(EComponentMobility::Movable);Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Part->SetRelativeLocation(Offset);Part->SetRelativeScale3D(Scale);Part->SetRelativeRotation(Rotation);Part->RegisterComponent();return Part;
}
void ExpeditionSign(AActor* Owner,const FString& Text) {
 auto* Sign=NewObject<UTextRenderComponent>(Owner);Sign->SetupAttachment(Owner->GetRootComponent());
 Sign->SetText(FText::FromString(Text));Sign->SetWorldSize(18);Sign->SetTextRenderColor(FColor(125,231,240));
 Sign->SetRelativeLocation(FVector(-65,0,120));Sign->SetRelativeRotation(FRotator(0,180,0));
 Sign->SetHorizontalAlignment(EHTA_Center);Sign->RegisterComponent();
}
}
int32 ALunarGameMode::SampleCount() const {int32 Count=0;for(int32 I=0;I<3;++I)if(SamplesCollected&(1<<I))++Count;return Count;}
bool ALunarGameMode::SampleAvailable(int32 Index) const {return SampleActors.IsValidIndex(Index)&&IsValid(SampleActors[Index])&&!(SamplesCollected&(1<<Index));}
float ALunarGameMode::BeaconPhase() const {return FMath::Fmod(BeaconCycleSeconds,3.2f)/3.2f;}
bool ALunarGameMode::BeaconInWindow() const {return BeaconPhase()>=.34f&&BeaconPhase()<=.66f;}
FString ALunarGameMode::ExpeditionRank() const {return RecorderRecovered?(Oxygen>=120?TEXT("PATHFINDER"):TEXT("EXPLORER")):TEXT("RESCUER");}

void ALunarGameMode::BuildMoonExpedition() {
 if(Mars||Menu)return;
 FCollisionQueryParams Q;Q.bTraceComplex=true;
 for(TActorIterator<AActor> It(GetWorld());It;++It) {
  auto* Mesh=It->FindComponentByClass<UStaticMeshComponent>();
  if(!Mesh||!Mesh->GetStaticMesh()||!Mesh->GetStaticMesh()->GetName().Contains(TEXT("Terrain")))Q.AddIgnoredActor(*It);
 }
 auto Site=[&](FVector Position) {
  FHitResult Hit;if(GetWorld()->LineTraceSingleByChannel(Hit,Position+FVector(0,0,2500),Position-FVector(0,0,1400),ECC_Visibility,Q))Position.Z=Hit.ImpactPoint.Z;
  auto* Actor=GetWorld()->SpawnActor<AActor>();auto* Root=NewObject<USceneComponent>(Actor);Actor->SetRootComponent(Root);Root->RegisterComponent();
  Actor->SetActorLocation(Position+FVector(0,0,55));return Actor;
 };
 const FVector Positions[]={FVector(-650,650,0),FVector(2200,650,0),FVector(4900,650,0)};
 const TCHAR* Names[]={TEXT("A / REGOLITH"),TEXT("B / BASALT"),TEXT("C / IMPACT GLASS")};
 for(int32 I=0;I<3;++I) {
  auto* Probe=Site(Positions[I]);SampleActors.Add(Probe);Probe->Tags.Add(TEXT("MoonSample"));
  ExpeditionPart(Probe,TEXT("Cylinder"),TEXT("M_Graphite"),FVector(0,0,-42),FVector(1.1,1.1,.16));
  ExpeditionPart(Probe,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector::ZeroVector,FVector(.38,.38,.8));
  ExpeditionPart(Probe,TEXT("Cylinder"),TEXT("M_ThermalFoil"),FVector(0,0,25),FVector(.4,.4,.12));
  ExpeditionPart(Probe,TEXT("Sphere"),TEXT("M_Signal"),FVector(0,0,48),FVector(.17));
  ExpeditionPart(Probe,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector(45,0,40),FVector(.025,.025,1.6));
  ExpeditionPart(Probe,TEXT("Cube"),TEXT("M_Warning"),FVector(45,15,112),FVector(.035,.35,.2));
  ExpeditionSign(Probe,Names[I]);
 }
 // Wheeled survey robot, suitable for the airless lunar setting.
 DroneActor=Site(FVector(5400,-350,0));DroneActor->Tags.Add(TEXT("MoonSurveyRobot"));
 ExpeditionPart(DroneActor,TEXT("Cube"),TEXT("M_Ceramic"),FVector::ZeroVector,FVector(1.15,.7,.35));
 ExpeditionPart(DroneActor,TEXT("Cube"),TEXT("M_ThermalFoil"),FVector(0,0,25),FVector(.72,.6,.15));
 ExpeditionPart(DroneActor,TEXT("Cube"),TEXT("M_Solar"),FVector(0,0,40),FVector(1.65,1.25,.035));
 for(float X:{-40.f,40.f})for(float Y:{-45.f,45.f})ExpeditionPart(DroneActor,TEXT("Cylinder"),TEXT("M_Graphite"),FVector(X,Y,-29),FVector(.45,.45,.15),FRotator(0,0,90));
 ExpeditionPart(DroneActor,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector(-42,0,55),FVector(.04,.04,.85));
 ExpeditionPart(DroneActor,TEXT("Cube"),TEXT("M_Graphite"),FVector(-42,0,95),FVector(.28,.4,.18));
 ExpeditionPart(DroneActor,TEXT("Sphere"),TEXT("M_Warning"),FVector(-59,0,95),FVector(.1));
 ExpeditionPart(DroneActor,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector(30,-55,5),FVector(.22,.22,.55));
 ExpeditionSign(DroneActor,TEXT("NOMAD-7 / MEMORY CORE"));
 BeaconActor=Site(FVector(5700,650,0));BeaconActor->Tags.Add(TEXT("MoonBeacon"));
 ExpeditionPart(BeaconActor,TEXT("Cylinder"),TEXT("M_Graphite"),FVector(0,0,-38),FVector(1.3,1.3,.3));
 ExpeditionPart(BeaconActor,TEXT("Cube"),TEXT("M_Ceramic"),FVector::ZeroVector,FVector(.65,.6,.7));
 ExpeditionPart(BeaconActor,TEXT("Cube"),TEXT("M_Signal"),FVector(-34,0,12),FVector(.035,.4,.25));
 ExpeditionPart(BeaconActor,TEXT("Cylinder"),TEXT("M_Ceramic"),FVector(12,0,125),FVector(.07,.07,2.3));
 ExpeditionPart(BeaconActor,TEXT("Sphere"),TEXT("M_ThermalFoil"),FVector(12,0,225),FVector(.95,.95,.09),FRotator(0,0,25));
 BeaconLamp=ExpeditionPart(BeaconActor,TEXT("Sphere"),TEXT("M_Warning"),FVector(12,0,255),FVector(.18));
 ExpeditionSign(BeaconActor,TEXT("EARTHLINK / 3 CHANNELS"));
}
void ALunarGameMode::AdvanceExpedition(float Delta) {if(!Mars&&!Menu&&!Paused&&Stage==ELunarStage::ActivateBeacon)BeaconCycleSeconds+=FMath::Max(0.f,Delta);}
void ALunarGameMode::LockBeacon() {
 if(Stage!=ELunarStage::ActivateBeacon||!CanInteractWith(BeaconActor))return;
 if(!BeaconInWindow()){Message(TEXT("EARTHLINK: Wait for the green window, then tap E. Locked channels are safe."));return;}
 ++BeaconLocks;BeaconCycleSeconds=0;PlayPowerSound(TEXT("PowerContact"),.45f);
 if(BeaconLocks<3){Message(FString::Printf(TEXT("EARTHLINK: Channel %d/3 locked. Time the next green window."),BeaconLocks));return;}
 Stage=ELunarStage::ReturnHome;
 if(BeaconLamp)BeaconLamp->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Materials/M_Signal.M_Signal")));
 Message(TEXT("EARTHLINK ONLINE. Return to the lander. Q marks the optional crew recorder."));
}
int32 ALunarGameMode::ExpeditionChecks() {
 int32 Fail=0;FString Report;
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Fail;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 Check(!Mars&&SampleActors.Num()==3&&DroneActor&&BeaconActor,TEXT("Moon expedition props exist"));
 if(!P||!PC||SampleActors.Num()!=3||!DroneActor||!BeaconActor)return Fail+1;
 auto Aim=[&](AActor* Actor){P->SetActorLocation(Actor->GetActorLocation()+FVector(-170,0,40),false,nullptr,ETeleportType::TeleportPhysics);FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((Actor->GetActorLocation()-Eye).Rotation());};
 Check(Stage==ELunarStage::SurveySamples&&SampleCount()==0,TEXT("Download unlocks lunar survey"));
 Aim(HomeActor);Interact();Check(Stage==ELunarStage::SurveySamples,TEXT("Lander cannot bypass unfinished expedition"));
 Aim(DroneActor);Interact();Check(!DroneRecovered,TEXT("Robot recovery locked before survey"));
 auto* First=Target();ToggleRoute();Check(Target()!=First,TEXT("Q cycles uncollected sample markers"));
 for(int32 Index:{2,0,1}) {
  Aim(SampleActors[Index]);Check(CanInteract(),TEXT("Sample reachable independently of Q selection"));Interact();AdvanceInteraction(.8f);
  if(Index==2) {
   ReleaseInteract();Check(SampleCount()==0&&HoldProgress==0,TEXT("Sample scan requires uninterrupted hold"));
   Interact();AdvanceInteraction(.7f);Paused=true;AdvanceInteraction(10);Check(FMath::IsNearlyEqual(HoldProgress,.7f),TEXT("Pause freezes sample scan"));Paused=false;
   PC->SetControlRotation(FRotator(0,180,0));AdvanceInteraction(.2f);Check(!Holding&&SampleCount()==0,TEXT("Looking away cancels sample scan"));Aim(SampleActors[Index]);Interact();
   Aim(SampleActors[0]);AdvanceInteraction(3);Check(!Holding&&SampleCount()==0,TEXT("Changing sample target cancels held scan"));Aim(SampleActors[Index]);Interact();
  }
  AdvanceInteraction(2.6f);Check(!SampleAvailable(Index),TEXT("Completed sample removed from pending navigation"));
  int32 Saved=SampleCount();Interact();AdvanceInteraction(3);Check(SampleCount()==Saved,TEXT("Sample cannot be counted twice"));
 }
 Check(SampleCount()==3&&Stage==ELunarStage::RecoverDrone,TEXT("All three samples unlock robot mission"));
 Aim(DroneActor);Oxygen=300;Interact();AdvanceInteraction(1);ReleaseInteract();Check(!DroneRecovered&&Oxygen==300,TEXT("Incomplete robot recovery gives no oxygen reward"));
 Interact();AdvanceInteraction(4.1f);Check(DroneRecovered&&Stage==ELunarStage::ActivateBeacon&&Oxygen==360,TEXT("Robot memory and reserve oxygen recovered"));
 Interact();AdvanceInteraction(5);Check(Oxygen==360,TEXT("Robot oxygen reward cannot be repeated"));
 Aim(BeaconActor);BeaconCycleSeconds=0;Interact();Check(BeaconLocks==0,TEXT("Beacon rejects taps outside signal window"));
 Paused=true;AdvanceExpedition(10);Interact();Check(BeaconCycleSeconds==0&&BeaconLocks==0,TEXT("Pause freezes beacon timing and input"));Paused=false;
 AdvanceExpedition(-2);Check(BeaconCycleSeconds==0,TEXT("Negative time cannot rewind signal"));
 for(int32 I=0;I<3;++I){AdvanceExpedition(1.6f);Interact();Check(BeaconLocks==I+1,TEXT("Green-window tap locks exactly one channel"));if(I<2){Interact();Check(BeaconLocks==I+1,TEXT("Repeated tap cannot lock next channel"));}}
 Check(Stage==ELunarStage::ReturnHome&&Target()==HomeActor,TEXT("Three signal locks unlock extraction"));
 Check(ExpeditionRank()==TEXT("RESCUER"),TEXT("Base ending rank"));
 RecorderRecovered=true;Check(ExpeditionRank()==TEXT("PATHFINDER"),TEXT("Fast complete expedition rank"));
 Oxygen=110;Check(ExpeditionRank()==TEXT("EXPLORER"),TEXT("Complete story expedition rank"));RecorderRecovered=false;Oxygen=360;
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Fail);FFileHelper::SaveStringToFile(Report,*ReportPath(TEXT("expedition-checks.txt")));
 UE_LOG(LogTemp,Display,TEXT("LUNAR_EXPEDITION_CHECKS %s"),*Report);return Fail;
}
void ALunarGameMode::ExpeditionReview() {
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC||Mars||SampleActors.Num()!=3){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 Oxygen=400;P->GetCharacterMovement()->StopMovementImmediately();
 auto View=[&](AActor* Actor){P->SetActorLocation(Actor->GetActorLocation()+FVector(-330,-200,85));FVector Eye;FRotator Rot;P->GetActorEyesViewPoint(Eye,Rot);PC->SetControlRotation((Actor->GetActorLocation()+FVector(0,0,30)-Eye).Rotation());};
 switch(MovementPhase){
 case 0:Stage=ELunarStage::Briefing;break;
 case 1:FScreenshotRequest::RequestScreenshot(ReportPath(TEXT("expedition-briefing.png")),true,false);break;
 case 2:Stage=ELunarStage::SurveySamples;StationRebootComplete=true;View(SampleActors[0]);Message(TEXT("SURVEY: Recover three sample canisters. Q cycles the next site."));break;
 case 3:FScreenshotRequest::RequestScreenshot(ReportPath(TEXT("expedition-survey.png")),true,false);break;
 case 4:Stage=ELunarStage::RecoverDrone;SamplesCollected=7;View(DroneActor);break;
 case 5:FScreenshotRequest::RequestScreenshot(ReportPath(TEXT("expedition-robot.png")),true,false);break;
 case 6:Stage=ELunarStage::ActivateBeacon;DroneRecovered=true;View(BeaconActor);break;
 case 7:FScreenshotRequest::RequestScreenshot(ReportPath(TEXT("expedition-beacon.png")),true,false);break;
 case 8:Stage=ELunarStage::Won;BeaconLocks=3;RecorderRecovered=true;break;
 case 9:FScreenshotRequest::RequestScreenshot(ReportPath(TEXT("expedition-result.png")),true,false);break;
 default:FPlatformMisc::RequestExitWithStatus(false,0);return;
 }
 ++MovementPhase;FTimerHandle H;GetWorldTimerManager().SetTimer(H,this,&ALunarGameMode::ExpeditionReview,3,false);
}
