#include "LunarSeleneProps.h"
#include "LunarGame.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

namespace {
bool Selene(ALunarGameMode* G){return G&&!G->Menu&&!G->Apollo&&!G->Mars&&G->GetWorld();}
AActor* Tagged(ALunarGameMode* G,FName Tag){
 // Verified persistent IDs plus mesh/class identify previously untagged props.
 // Actor labels are editor-only; the original map remains byte-identical.
 const TCHAR* Id=nullptr;const TCHAR* Mesh=nullptr;
 if(Tag==TEXT("Lunar.ModuleBeacon")){Id=TEXT("StaticMeshActor_563");Mesh=TEXT("Cube");}
 else if(Tag==TEXT("Lunar.ModuleBeaconLight"))Id=TEXT("PointLight_2");
 else if(Tag==TEXT("Lunar.SeleneMast")){Id=TEXT("StaticMeshActor_554");Mesh=TEXT("Cylinder");}
 else if(Tag==TEXT("Lunar.StaticLunokhod")){Id=TEXT("StaticMeshActor_8");Mesh=TEXT("Lunokhod");}
 for(TActorIterator<AActor> It(G->GetWorld());It;++It){if(It->ActorHasTag(Tag))return *It;if(!Id||It->GetFName()!=FName(Id))continue;auto* C=It->FindComponentByClass<UStaticMeshComponent>();if(Mesh?(!C||!C->GetStaticMesh()||C->GetStaticMesh()->GetFName()!=FName(Mesh)):!It->FindComponentByClass<UPointLightComponent>())continue;It->Tags.AddUnique(Tag);return *It;}
 return nullptr;
}
void Move(AActor* A,FVector P){if(!A||!A->GetRootComponent())return;A->GetRootComponent()->SetMobility(EComponentMobility::Movable);A->SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);}
void Solid(UPrimitiveComponent* C){C->SetCollisionProfileName(TEXT("BlockAll"));C->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);C->CanCharacterStepUpOn=ECB_No;C->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable,0));C->SetCanEverAffectNavigation(false);C->PrimaryComponentTick.bCanEverTick=false;C->ComponentTags.Add(TEXT("Lunar.StaticRoverCollision"));}
void RoverCollision(AActor* A){
 if(!A||A->ActorHasTag(TEXT("Lunar.StaticRoverSolid")))return;A->SetActorEnableCollision(true);
 if(auto* Mesh=A->FindComponentByClass<UStaticMeshComponent>())Solid(Mesh);
 // Small authored solid chassis and eight tire envelopes supplement the
 // original open display triangles. Source coordinates are converted OBJ cm.
 // No giant box across the raised lid, antenna or surrounding walkable air.
 auto* Hull=NewObject<UBoxComponent>(A);Hull->SetupAttachment(A->GetRootComponent());Hull->SetRelativeLocation(FVector(4.5,-25,81));Hull->SetBoxExtent(FVector(60.5,105,30));Solid(Hull);Hull->RegisterComponent();
 for(float X:{-70.0101f,78.9235f})for(float Y:{71.6832f,9.0526f,-40.9407f,-103.5709f}){
  auto* Tire=NewObject<UCapsuleComponent>(A);Tire->SetupAttachment(A->GetRootComponent());Tire->SetRelativeLocation(FVector(X,Y,22.2865));Tire->SetRelativeRotation(FRotator(90,0,0));Tire->InitCapsuleSize(21.f,29.f);Solid(Tire);Tire->RegisterComponent();
 }
 A->Tags.Add(TEXT("Lunar.StaticRoverSolid"));
}
bool SupportPlane(ALunarGameMode* G,FVector At,double& Z,bool* OnRoof=nullptr){
 // The retained habitat is open at the legacy mast's XY (also checked against
 // its source OBJ triangles). If no roof exists there, anchor a small footing
 // to the actual terrain rather than pretending an invisible roof supports it.
 for(const TCHAR* Name:{TEXT("HabitatMain"),TEXT("SM_LunarTerrain")})for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(!C||!C->GetStaticMesh()||C->GetStaticMesh()->GetName()!=Name)continue;
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(SeleneMastSupport),true);
  if(C->LineTraceComponent(Hit,At+FVector(0,0,4000),At-FVector(0,0,4000),Q)&&Hit.ImpactNormal.Z>.35){Z=Hit.ImpactPoint.Z;if(OnRoof)*OnRoof=FString(Name)==TEXT("HabitatMain");return true;}
 }return false;
}
void Mast(ALunarGameMode* G){auto* A=Tagged(G,TEXT("Lunar.SeleneMast"));auto* C=A?A->FindComponentByClass<UStaticMeshComponent>():nullptr;if(!C)return;
 double Surface=0;bool OnRoof=false;if(!SupportPlane(G,A->GetActorLocation(),Surface,&OnRoof)){UE_LOG(LogTemp,Warning,TEXT("SELENE_MAST missing_support"));return;}
 const double Base=Surface+4,Top=C->Bounds.Origin.Z+C->Bounds.BoxExtent.Z,Height=Top-Base;
 if(Height<=0||Height>2000)return;Move(A,FVector(A->GetActorLocation().X,A->GetActorLocation().Y,(Top+Base)*.5));FVector Scale=A->GetActorScale3D();Scale.Z=Height/100.;A->SetActorScale3D(Scale);C->UpdateBounds();
 auto* Foot=NewObject<UStaticMeshComponent>(A);Foot->SetupAttachment(C);Foot->SetMobility(EComponentMobility::Movable);Foot->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Foot->SetMaterial(0,C->GetMaterial(0));Foot->SetCollisionEnabled(ECollisionEnabled::NoCollision);Foot->SetCanEverAffectNavigation(false);Foot->PrimaryComponentTick.bCanEverTick=false;Foot->ComponentTags.Add(TEXT("Lunar.MastFooting"));Foot->RegisterComponent();Foot->SetWorldLocation(FVector(A->GetActorLocation().X,A->GetActorLocation().Y,Surface+2));Foot->SetWorldScale3D(FVector(.30,.30,.04));A->Tags.AddUnique(TEXT("Lunar.MastSupported"));
 UE_LOG(LogTemp,Display,TEXT("SELENE_MAST support=%s surface=%.3f bottom_after=%.3f top=%.3f"),OnRoof?TEXT("roof"):TEXT("terrain"),Surface,C->Bounds.Origin.Z-C->Bounds.BoxExtent.Z,Top);
}

void Beacon(ALunarGameMode* G){
 auto* A=Tagged(G,TEXT("Lunar.ModuleBeacon"));auto* Lamp=Tagged(G,TEXT("Lunar.ModuleBeaconLight"));auto* M=G->ModuleActor?G->ModuleActor->FindComponentByClass<UStaticMeshComponent>():nullptr;
 if(!M||!A)return;const FVector Top=M->Bounds.Origin+FVector(0,0,M->Bounds.BoxExtent.Z);A->SetActorScale3D(FVector(.12,.12,.06));Move(A,Top+FVector(0,0,3));Move(Lamp,Top+FVector(0,0,8));
 if(Lamp)if(auto* L=Lamp->FindComponentByClass<UPointLightComponent>()){L->SetAttenuationRadius(110);L->SetIntensity(45);}
}
void Indicators(ALunarGameMode* G){
 if(!G->PowerActor)return;const FTransform Frame=G->PowerActor->GetActorTransform();
 auto Place=[&](AActor* Bulb,UPointLightComponent* Light,float Y){const FVector P=Frame.TransformPosition(FVector(-55,Y,34));if(Bulb){Move(Bulb,P);Bulb->SetActorScale3D(FVector(.10));}if(Light&&Light->GetOwner()){Move(Light->GetOwner(),P+Frame.GetRotation().RotateVector(FVector(-3,0,0)));Light->SetAttenuationRadius(150);}};
 Place(G->PowerWarningBulb,G->PowerWarningLight,-30);for(int32 I=0;I<G->PowerBulbs.Num();++I)Place(G->PowerBulbs[I],G->PowerLights.IsValidIndex(I)?G->PowerLights[I].Get():nullptr,-15+I*15);
}
}
void LunarSeleneProps::Build(ALunarGameMode* G){if(!Selene(G)||G->ActorHasTag(TEXT("Lunar.SelenePropsBuilt")))return;RoverCollision(Tagged(G,TEXT("Lunar.StaticLunokhod")));Mast(G);Beacon(G);Indicators(G);G->Tags.Add(TEXT("Lunar.SelenePropsBuilt"));UE_LOG(LogTemp,Display,TEXT("SELENE_PROPS built source_terrain_unchanged=1 module_power_dish_anchors_unchanged=1"));}
void LunarSeleneProps::ModulePickedUp(ALunarGameMode* G){if(!Selene(G))return;for(FName Tag:{FName(TEXT("Lunar.ModuleBeacon")),FName(TEXT("Lunar.ModuleBeaconLight"))})if(auto* A=Tagged(G,Tag)){A->SetActorHiddenInGame(true);A->SetActorEnableCollision(false);if(auto* L=A->FindComponentByClass<UPointLightComponent>())L->SetIntensity(0);}}
void LunarSeleneProps::ValidateChecks(ALunarGameMode* G,TFunctionRef<void(bool,const TCHAR*)> Check){
 if(!Selene(G))return;Check(G->PowerActor&&G->ModuleActor,TEXT("SELENE prop checks retain their original module and power anchors"));if(!G->PowerActor||!G->ModuleActor)return;auto* Rover=Tagged(G,TEXT("Lunar.StaticLunokhod"));auto* MastActor=Tagged(G,TEXT("Lunar.SeleneMast"));auto* C=MastActor?MastActor->FindComponentByClass<UStaticMeshComponent>():nullptr;double RoofZ=0;
 Check(C&&SupportPlane(G,MastActor->GetActorLocation(),RoofZ)&&FMath::Abs(C->Bounds.Origin.Z-C->Bounds.BoxExtent.Z-RoofZ-4)<.1,TEXT("SELENE antenna mast contacts its footing on an actual supporting roof or terrain surface"));
 const FVector Warning=G->PowerActor->GetActorTransform().TransformPosition(FVector(-55,-30,34));Check(G->PowerWarningBulb&&G->PowerWarningLight&&G->PowerWarningBulb->GetActorLocation().Equals(Warning,.01)&&FVector::Dist(Warning,G->PowerWarningLight->GetComponentLocation())<4,TEXT("Warning lamp and its point light share a mount on the actual power panel"));
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));bool SolidSetup=Rover!=nullptr;int32 CollisionCount=0;
 if(Rover){TArray<UPrimitiveComponent*> Parts;Rover->GetComponents(Parts);for(auto* Part:Parts)if(Part->ComponentHasTag(TEXT("Lunar.StaticRoverCollision"))){++CollisionCount;SolidSetup&=Part->CanCharacterStepUpOn==ECB_No&&Part->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block&&Part->IsQueryCollisionEnabled();}}
 Check(SolidSetup&&CollisionCount==10,TEXT("Static Lunokhod body and eight wheels block the player and disallow automatic stepping"));
 if(P&&Rover){const FTransform Saved=P->GetActorTransform();auto* M=P->GetCharacterMovement();const FVector Velocity=M->Velocity;const EMovementMode Mode=M->MovementMode;const uint8 Custom=M->CustomMovementMode;const bool Collision=P->GetActorEnableCollision();P->SetActorEnableCollision(true);
  // Exercise the real character capsule's swept move into the chassis from
  // four sides and four diagonals at walking-body height. Not a flags-only test.
  for(int32 I=0;I<8;++I){const double Angle=I*PI/4;const FVector Local(FMath::Cos(Angle)*250+4.5,FMath::Sin(Angle)*250-25,90);const FVector Start=Rover->GetActorTransform().TransformPosition(Local),Goal=Rover->GetActorTransform().TransformPosition(FVector(4.5,-25,90));P->SetActorLocation(Start,false,nullptr,ETeleportType::TeleportPhysics);FHitResult Hit;M->SafeMoveUpdatedComponent(Goal-Start,P->GetActorQuat(),true,Hit);Check(Hit.bBlockingHit&&Hit.GetActor()==Rover&&FVector::Dist2D(P->GetActorLocation(),Goal)>P->GetCapsuleComponent()->GetScaledCapsuleRadius(),*FString::Printf(TEXT("Actual character capsule cannot enter static rover chassis from direction %d"),I));}
  P->SetActorLocation(Saved.GetLocation(),false,nullptr,ETeleportType::TeleportPhysics);P->SetActorRotation(Saved.GetRotation());P->SetActorEnableCollision(Collision);M->SetMovementMode(Mode,Custom);M->Velocity=Velocity;
 }
 auto* B=Tagged(G,TEXT("Lunar.ModuleBeacon"));auto* L=Tagged(G,TEXT("Lunar.ModuleBeaconLight"));
 Check(B&&L&&!B->IsHidden()&&!L->IsHidden()&&FVector::Dist2D(B->GetActorLocation(),G->ModuleActor->GetActorLocation())<1,TEXT("Module locator is mounted on the actual cell rather than floating over its old position"));
 const auto Stage=G->Stage;const bool Hidden=G->ModuleActor->IsHidden(),Colliding=G->ModuleActor->GetActorEnableCollision();const float Intensity=L&&L->FindComponentByClass<UPointLightComponent>()?L->FindComponentByClass<UPointLightComponent>()->Intensity:0;
 if(P){const FTransform Before=P->GetActorTransform();auto* PC=Cast<APlayerController>(P->GetController());const FRotator BeforeAim=PC?PC->GetControlRotation():FRotator::ZeroRotator;G->Stage=ELunarStage::FindModule;P->SetActorLocation(G->ModuleActor->GetActorLocation()+FVector(-100,0,70),false,nullptr,ETeleportType::TeleportPhysics);P->Camera->SetWorldRotation((G->ModuleActor->GetActorLocation()-P->Camera->GetComponentLocation()).Rotation());if(PC)PC->SetControlRotation(P->Camera->GetComponentRotation());G->Interact();Check(G->Stage==ELunarStage::RestorePower&&B&&B->IsHidden()&&L&&L->IsHidden()&&L->FindComponentByClass<UPointLightComponent>()->Intensity==0,TEXT("Real module pickup removes both the locator mesh and point light while entering power restoration"));P->SetActorTransform(Before);if(PC)PC->SetControlRotation(BeforeAim);}
 G->Stage=Stage;G->ModuleActor->SetActorHiddenInGame(Hidden);G->ModuleActor->SetActorEnableCollision(Colliding);if(B)B->SetActorHiddenInGame(false);if(L){L->SetActorHiddenInGame(false);if(auto* Light=L->FindComponentByClass<UPointLightComponent>())Light->SetIntensity(Intensity);}
}
