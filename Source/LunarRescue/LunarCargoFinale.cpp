#include "LunarCargoFinale.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include <limits>

namespace {
constexpr float StowageDuration=8.8f;
float Ease(float V){V=FMath::Clamp(V,0.f,1.f);return V*V*V*(V*(V*6-15)+10);}
UStaticMeshComponent* Visual(AActor* Owner,USceneComponent* Parent,const TCHAR* Path,FVector Position,FVector Scale,const TCHAR* Surface=TEXT("M_EquipmentPaint")){
 auto* Mesh=LoadObject<UStaticMesh>(nullptr,Path);if(!Mesh)return nullptr;
 auto* C=NewObject<UStaticMeshComponent>(Owner);C->SetupAttachment(Parent);C->SetStaticMesh(Mesh);
 C->SetRelativeLocation(Position);C->SetRelativeScale3D(Scale);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->SetReceivesDecals(false);C->ComponentTags.Add(TEXT("Apollo.CargoFinaleVisual"));
 if(Surface)C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Apollo12/Realism/%s.%s"),Surface,Surface)));
 C->RegisterComponent();return C;
}
int32 MaskCount(uint8 Mask){int32 N=0;for(int32 I=0;I<3;++I)N+=(Mask&(1<<I))?1:0;return N;}
}

ALunarCargoFinale::ALunarCargoFinale(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.bStartWithTickEnabled=false;
 auto* Root=CreateDefaultSubobject<USceneComponent>(TEXT("StowagePresentation"));SetRootComponent(Root);
 ShotCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("CargoCamera"));ShotCamera->SetupAttachment(Root);
 ShotCamera->SetFieldOfView(60);ShotCamera->bUsePawnControlRotation=false;SetActorEnableCollision(false);
 Tags.Add(TEXT("Apollo.CargoFinale"));
}

bool ALunarCargoFinale::Active(const ALunarGameMode* Mission){return Mission&&IsValid(Mission->CargoFinale)&&Mission->CargoFinale->IsPlaying();}
ALunarCargoFinale* ALunarCargoFinale::Begin(ALunarGameMode* Mission){
 if(!Mission||!Mission->Apollo||Mission->Stage!=ELunarStage::Won)return nullptr;
 if(Active(Mission))return Mission->CargoFinale;
 Reset(Mission);auto* Scene=Mission->GetWorld()->SpawnActor<ALunarCargoFinale>();if(!Scene)return nullptr;
 Mission->CargoFinale=Scene;
 if(!Scene->Initialize(Mission)){Mission->CargoFinale=nullptr;Scene->Destroy();return nullptr;}return Scene;
}
bool ALunarCargoFinale::Skip(ALunarGameMode* Mission){
 if(!Active(Mission))return false;
 // Input while paused belongs to the pause menu; it cannot silently skip.
 auto* Scene=Mission->CargoFinale.Get();
 if(!Mission->Paused&&(!IsValid(Scene->Astronaut)||!Scene->Astronaut->ActivePanel))Scene->Finish();return true;
}
void ALunarCargoFinale::Reset(ALunarGameMode* Mission){
 if(!Mission||!IsValid(Mission->CargoFinale))return;
 auto* Old=Mission->CargoFinale.Get();Mission->CargoFinale=nullptr;Old->Finish();Old->Destroy();
}

bool ALunarCargoFinale::Initialize(ALunarGameMode* Mission){
 Game=Mission;Player=UGameplayStatics::GetPlayerController(Mission,0);Astronaut=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Mission,0));
 if(!Player||!Astronaut||!Mission->HomeActor)return false;
 SampleMask=Mission->SamplesCollected&7;PartMask=Mission->ApolloParts&7;
 // The display rack is authored equipment beside the existing field kit.
 // No source position, imported terrain, historical model or collider moves.
 FVector At=Mission->HomeActor->GetActorLocation()+FVector(-110,-160,0);
 FHitResult Hit;FCollisionQueryParams Query;Query.AddIgnoredActor(this);Query.AddIgnoredActor(Astronaut);Query.bTraceComplex=true;
 if(GetWorld()->LineTraceSingleByChannel(Hit,At+FVector(0,0,1000),At-FVector(0,0,1500),ECC_Visibility,Query))At.Z=Hit.ImpactPoint.Z+45;
 else At.Z-=20;
 SetActorLocation(At);BuildCargo();
 PreviousViewTarget=Player->GetViewTarget();PreviousPawnHidden=Astronaut->IsHidden();
 // External presentation camera avoids moving the player or mutating their
 // control rotation, first/third-person choice, FOV or comfort preferences.
 Astronaut->GetCharacterMovement()->StopMovementImmediately();Astronaut->SetActorHiddenInGame(true);
 Playing=true;Elapsed=0;PoseCargo();Player->SetViewTarget(this);SetActorTickEnabled(true);
 UE_LOG(LogTemp,Display,TEXT("APOLLO_CARGO_FINALE start samples=%d parts=%d visual_items=%d authored_display=1"),MaskCount(SampleMask),MaskCount(PartMask),CargoRoots.Num());
 return true;
}

void ALunarCargoFinale::BuildCargo(){
 const TCHAR* Cube=TEXT("/Engine/BasicShapes/Cube.Cube");
 Visual(this,GetRootComponent(),Cube,FVector::ZeroVector,FVector(1.76f,.98f,.07f),TEXT("M_EquipmentDark"));
 for(float X:{-87.f,87.f})Visual(this,GetRootComponent(),Cube,FVector(X,0,9),FVector(.025f,.98f,.16f));
 for(float Y:{-48.f,48.f})Visual(this,GetRootComponent(),Cube,FVector(0,Y,9),FVector(1.76f,.025f,.16f));
 for(float X:{-29.f,29.f})Visual(this,GetRootComponent(),Cube,FVector(X,0,7),FVector(.018f,.95f,.12f));
 for(float X:{-76.f,76.f})for(float Y:{-36.f,36.f})Visual(this,GetRootComponent(),Cube,FVector(X,Y,-20),FVector(.035f,.035f,.40f),TEXT("M_EquipmentDark"));
 auto* Light=NewObject<UPointLightComponent>(this);Light->SetupAttachment(GetRootComponent());Light->SetRelativeLocation(FVector(-90,-100,170));Light->SetIntensity(2800);Light->SetAttenuationRadius(420);Light->SetCastShadows(false);Light->RegisterComponent();
 const FVector Positions[]={FVector(-57,-23,7),FVector(0,-23,7),FVector(57,-23,7),FVector(-57,23,16),FVector(0,23,10),FVector(57,23,12)};
 const TCHAR* Labels[]={TEXT("Sample 12031"),TEXT("Sample 12032"),TEXT("Core 12027"),TEXT("Surveyor TV camera"),TEXT("Surveyor soil scoop"),TEXT("Surveyor cable section")};
 for(int32 I=0;I<6;++I){
  const bool Present=I<3?(SampleMask&(1<<I))!=0:(PartMask&(1<<(I-3)))!=0;if(!Present)continue;
  auto* Item=NewObject<USceneComponent>(this);Item->SetupAttachment(GetRootComponent());Item->RegisterComponent();
  CargoRoots.Add(Item);CargoTargets.Add(Positions[I]);CargoLabels.Add(Labels[I]);
  if(I<2){
   auto* Pouch=Visual(this,Item,TEXT("/Game/Lunar/Apollo12/Tools/SM_SamplePouch.SM_SamplePouch"),FVector(0,0,5),FVector(.68f),nullptr);
   if(Pouch){const FBoxSphereBounds B=Pouch->GetStaticMesh()->GetBounds();Pouch->SetRelativeLocation(FVector(0,0,4)-FVector(B.Origin)*.68f);}
   Visual(this,Item,Cube,FVector(0,0,13),FVector(.20f,.025f,.015f),TEXT("M_EquipmentDark"));
  }else if(I==2){
   auto* Tube=Visual(this,Item,TEXT("/Game/Lunar/Apollo12/Tools/SM_DriveTube.SM_DriveTube"),FVector(0,0,9),FVector(.72f),nullptr);
   if(Tube){Tube->SetRelativeRotation(FRotator(0,0,90));const auto B=Tube->GetStaticMesh()->GetBounds();Tube->SetRelativeLocation(FVector(0,0,9)-Tube->GetRelativeTransform().GetRotation().RotateVector(FVector(B.Origin)*.72f));}
  }else if(Game->ApolloPartActors.IsValidIndex(I-3)&&Game->ApolloPartActors[I-3]){
   AActor* Source=Game->ApolloPartActors[I-3];const FBox Bounds=Source->GetComponentsBoundingBox(true);const FVector Center=Bounds.GetCenter();
   const float Fit=32.f/FMath::Max(1.f,float(Bounds.GetSize().GetMax()));
   TArray<UStaticMeshComponent*> Parts;Source->GetComponents(Parts);int32 Clones=0;
   for(auto* Original:Parts){if(!Original->GetStaticMesh()||Clones>=12)continue;
    auto* C=Visual(this,Item,*Original->GetStaticMesh()->GetPathName(),(Original->GetComponentLocation()-Center)*Fit,Original->GetComponentScale()*Fit,nullptr);
    if(C){C->SetRelativeRotation(Original->GetComponentQuat());for(int32 M=0;M<Original->GetNumMaterials();++M)C->SetMaterial(M,Original->GetMaterial(M));++Clones;}
   }
  }
 }
}

void ALunarCargoFinale::PoseCargo(){
 for(int32 I=0;I<CargoRoots.Num();++I){
  const float T=Ease((Elapsed-.35f-I*.82f)/.70f);
  CargoRoots[I]->SetVisibility(Elapsed>=.35f+I*.82f,true);
  CargoRoots[I]->SetRelativeLocation(CargoTargets[I]+FVector(-18,-20,65)*(1-T));
  CargoRoots[I]->SetRelativeRotation(FRotator(0,-8*(1-T),0));
 }
 const float T=Ease(Elapsed/StowageDuration);const FVector Eye=FMath::Lerp(FVector(-310,-375,285),FVector(-285,-350,265),T);
 ShotCamera->SetRelativeLocation(Eye);ShotCamera->SetRelativeRotation((FVector(0,0,24)-Eye).Rotation());
}
void ALunarCargoFinale::Tick(float Delta){Super::Tick(Delta);AdvancePresentation(Delta);}
void ALunarCargoFinale::AdvancePresentation(float Delta){
 if(!Playing)return;
 if(!IsValid(Game)||Game->Stage!=ELunarStage::Won){Finish();return;}
 if(Game->Paused||(IsValid(Astronaut)&&Astronaut->ActivePanel)||!FMath::IsFinite(Delta)||Delta<=0)return;
 Elapsed=FMath::Min(StowageDuration,Elapsed+Delta);PoseCargo();if(Elapsed>=StowageDuration)Finish();
}
float ALunarCargoFinale::Progress()const{return FMath::Clamp(Elapsed/StowageDuration,0.f,1.f);}
FString ALunarCargoFinale::Caption()const{
 if(CargoRoots.IsEmpty())return TEXT("No packed cargo recorded for this display.");
 const int32 Current=FMath::FloorToInt((Elapsed-.35f)/.82f);
 if(Current<0)return FString::Printf(TEXT("%d samples / %d Surveyor parts recorded"),MaskCount(SampleMask),MaskCount(PartMask));
 if(Current<CargoLabels.Num())return TEXT("Stowing: ")+CargoLabels[Current];
 return TEXT("Cargo manifest verified. Expedition complete.");
}
void ALunarCargoFinale::RestoreView(){
 if(IsValid(Astronaut))Astronaut->SetActorHiddenInGame(PreviousPawnHidden);
 if(IsValid(Player)&&Player->GetViewTarget()==this){AActor* Previous=PreviousViewTarget.Get();if(!IsValid(Previous))Previous=Astronaut; if(IsValid(Previous))Player->SetViewTarget(Previous);}
}
void ALunarCargoFinale::Finish(){
 if(!Playing)return;Playing=false;SetActorTickEnabled(false);RestoreView();SetActorHiddenInGame(true);
 UE_LOG(LogTemp,Display,TEXT("APOLLO_CARGO_FINALE finished; committed mission state unchanged"));
}
void ALunarCargoFinale::EndPlay(const EEndPlayReason::Type Reason){Finish();Super::EndPlay(Reason);}

void ALunarCargoFinale::ValidateChecks(ALunarGameMode* Mission,TFunctionRef<void(bool,const TCHAR*)> Check){
 if(!Mission){Check(false,TEXT("Cargo finale test has a mission"));return;}
 Reset(Mission);auto* PC=UGameplayStatics::GetPlayerController(Mission,0);auto* Pawn=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Mission,0));
 if(!PC||!Pawn){Check(false,TEXT("Cargo finale has a possessed player"));return;}
 const auto BeforeStage=Mission->Stage;const int32 BeforeSamples=Mission->SamplesCollected;const uint8 BeforeParts=Mission->ApolloParts;
 const bool BeforePaused=Mission->Paused,BeforeApollo=Mission->Apollo;const float BeforeElapsed=Mission->ApolloElapsed,BeforeDistance=Mission->ApolloDistance,BeforeBudget=Mission->Oxygen;
 const FRotator BeforeAim=PC->GetControlRotation();const float BeforeFOV=Pawn->Camera->FieldOfView;AActor* BeforeView=PC->GetViewTarget();const bool BeforeHidden=Pawn->IsHidden();
 Mission->Apollo=true;Mission->Paused=false;Mission->Stage=ELunarStage::ApolloReturn;
 Check(Begin(Mission)==nullptr,TEXT("Stowage scene cannot begin before the mission is won"));
 Mission->Stage=ELunarStage::Won;Mission->SamplesCollected=5;Mission->ApolloParts=2;
 auto* Scene=Begin(Mission);Check(Scene&&Scene->CargoCount()==3,TEXT("Stowage visualizes only the three actually packed mask entries"));
 if(Scene){
  Check(Begin(Mission)==Scene,TEXT("Repeated finale start is idempotent while playing"));
  Check(PC->GetViewTarget()==Scene&&Pawn->IsHidden(),TEXT("External cargo camera hides the pawn without moving it"));
  const FVector OriginalLocation=Pawn->GetActorLocation();
  Scene->AdvancePresentation(1);const float T=Scene->PresentationTime();Mission->Paused=true;Scene->AdvancePresentation(3);
  Check(FMath::IsNearlyEqual(Scene->PresentationTime(),T)&&Active(Mission),TEXT("Pause freezes the cargo animation"));
  Check(Skip(Mission)&&Active(Mission),TEXT("A paused input cannot accidentally skip stowage"));
  Mission->Paused=false;
  const auto PreviousPanel=Pawn->ActivePanel;Pawn->ActivePanel=NewObject<ULunarPanel>(Pawn);Scene->AdvancePresentation(2);
  Check(Skip(Mission)&&Active(Mission)&&FMath::IsNearlyEqual(Scene->PresentationTime(),T),TEXT("An open panel freezes stowage and blocks game interaction from skipping behind it"));Pawn->ActivePanel=PreviousPanel;
  Scene->AdvancePresentation(-2);Scene->AdvancePresentation(std::numeric_limits<float>::quiet_NaN());
  Check(FMath::IsNearlyEqual(Scene->PresentationTime(),T),TEXT("Negative and non-finite presentation deltas are ignored"));
  Check(Scene->Caption().Contains(TEXT("12031"))&&!Scene->Caption().Contains(TEXT("12032")),TEXT("Current stowage caption follows the packed sample ID"));
  Check(Skip(Mission)&&!Active(Mission),TEXT("Skip finishes the active scene immediately"));
  Check(PC->GetViewTarget()==BeforeView&&Pawn->IsHidden()==BeforeHidden&&Pawn->GetActorLocation().Equals(OriginalLocation,.01f),TEXT("Skip restores the original view and visibility without teleporting"));
  Check(PC->GetControlRotation().Equals(BeforeAim,.01f)&&FMath::IsNearlyEqual(Pawn->Camera->FieldOfView,BeforeFOV),TEXT("Stowage preserves player aim and first-person camera FOV"));
  Check(Mission->SamplesCollected==5&&Mission->ApolloParts==2&&Mission->Stage==ELunarStage::Won&&Mission->ApolloElapsed==BeforeElapsed&&Mission->ApolloDistance==BeforeDistance&&Mission->Oxygen==BeforeBudget,TEXT("Presentation and skip never award cargo or change mission telemetry"));
 }
 Reset(Mission);Check(!Mission->CargoFinale&&!Active(Mission),TEXT("Reset clears transient stowage and does not replay completed results"));
 Mission->SamplesCollected=Mission->ApolloParts=7;Scene=Begin(Mission);
 Check(Scene&&Scene->CargoCount()==6,TEXT("Completed six-entry cargo manifest appears in the finale"));
 if(Scene){Scene->AdvancePresentation(99);Check(!Active(Mission)&&PC->GetViewTarget()==BeforeView,TEXT("The bounded finale automatically returns to the normal result view"));}
 Reset(Mission);Mission->SamplesCollected=Mission->ApolloParts=0;Scene=Begin(Mission);
 Check(Scene&&Scene->CargoCount()==0&&Scene->Caption().Contains(TEXT("No packed cargo")),TEXT("Empty packed masks never fabricate samples or hardware"));
 Reset(Mission);Mission->Stage=BeforeStage;Mission->SamplesCollected=BeforeSamples;Mission->ApolloParts=BeforeParts;Mission->Paused=BeforePaused;Mission->Apollo=BeforeApollo;
}
