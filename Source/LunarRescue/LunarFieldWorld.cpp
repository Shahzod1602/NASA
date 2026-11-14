#include "LunarFieldOperations.h"
#include "LunarGame.h"
#include "LunarSurfaceVehicle.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"
#include "Kismet/GameplayStatics.h"

namespace {
struct FSupportBuilder {
 UFieldOperationsComponent* Ops;ALunarGameMode* G;UStaticMeshComponent* Terrain=nullptr;UStaticMesh* Cube=nullptr;UStaticMesh* Cylinder=nullptr;UStaticMesh* Sphere=nullptr;UMaterialInterface* M[4]={};int32 Count=0;
 explicit FSupportBuilder(UFieldOperationsComponent* O):Ops(O),G(O->Game()){
  Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
  const TCHAR* Paths[]={TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentPaint.M_EquipmentPaint"),TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentDark.M_EquipmentDark"),TEXT("/Game/Lunar/Suit/Materials/M_Metal.M_Metal"),TEXT("/Game/Lunar/Materials/M_Signal.M_Signal")};for(int32 I=0;I<4;++I)M[I]=LoadObject<UMaterialInterface>(nullptr,Paths[I]);
  const FString Wanted=G->Apollo?TEXT("SM_Apollo12Terrain"):G->Mars?TEXT("SM_MarsTerrain"):TEXT("SM_LunarTerrain");
  for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName()==Wanted){Terrain=C;break;}}
  for(const auto& A:Ops->SupportActors)if(IsValid(A)){TArray<UStaticMeshComponent*> C;A->GetComponents(C);Count+=C.Num();}
 }
 bool Ready()const{return Terrain&&Cube&&Cylinder&&Sphere&&M[0]&&M[1]&&M[2]&&M[3];}
 bool Site(FVector& Requested)const{
  auto* P=Ops->Pilot();if(!P||!P->GetCapsuleComponent()||!Terrain)return false;
  const float Radius=P->GetCapsuleComponent()->GetScaledCapsuleRadius(),Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  auto Sample=[&](FVector Probe,FHitResult& Hit){FCollisionQueryParams T;T.bTraceComplex=true;return Terrain->LineTraceComponent(Hit,Probe+FVector(0,0,6000),Probe-FVector(0,0,6000),T)&&Hit.ImpactNormal.Z>=.75f;};
  FCollisionQueryParams Solid(SCENE_QUERY_STAT(FieldSupportSite),false);Solid.AddIgnoredActor(P);Solid.AddIgnoredActor(Terrain->GetOwner());Solid.AddIgnoredActor(G);
  FCollisionQueryParams Sight(SCENE_QUERY_STAT(FieldSupportSiteSight),false);Sight.AddIgnoredActor(P);Sight.AddIgnoredActor(G);
  for(const auto& A:Ops->SupportActors)if(IsValid(A.Get())){Solid.AddIgnoredActor(A.Get());Sight.AddIgnoredActor(A.Get());}
  // Bounded authored-prop placement. Never move or flatten measured terrain.
  for(int32 Candidate=0;Candidate<25;++Candidate){const int32 Ring=Candidate==0?0:(Candidate-1)/8+1;const float Angle=Candidate==0?0.f:2*PI*((Candidate-1)%8)/8;
   const FVector Probe=Requested+FVector(FMath::Cos(Angle)*Ring*400,FMath::Sin(Angle)*Ring*400,0);FHitResult Ground;if(!Sample(Probe,Ground))continue;
   const FVector Focus=Ground.ImpactPoint+Ground.ImpactNormal*64.5f;
   if(G->GetWorld()->OverlapBlockingTestByChannel(Focus,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeBox(FVector(48,48,60)),Solid))continue;
   bool Observable=false;
   for(float Distance:{220.f,340.f}){for(int32 Side=0;Side<8;++Side){const float A=2*PI*Side/8;FHitResult Foot;if(!Sample(Ground.ImpactPoint+FVector(FMath::Cos(A)*Distance,FMath::Sin(A)*Distance,0),Foot))continue;
    const FVector Standing=Foot.ImpactPoint+FVector(0,0,(Half-Radius)+(Radius+2)/FMath::Max(.75f,Foot.ImpactNormal.Z));
    if(G->GetWorld()->OverlapBlockingTestByChannel(Standing,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(Radius,Half),Solid))continue;
    FHitResult Hit;const FVector Eye=Standing+FVector(0,0,68);
    if(!G->GetWorld()->LineTraceSingleByChannel(Hit,Eye,Focus,ECC_Visibility,Sight)){Observable=true;break;}
   }if(Observable)break;}
   if(Observable){Requested=Ground.ImpactPoint;return true;}
  }return false;
 }

 AActor* Actor(FVector Near,const FName& Tag){
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;if(!Terrain->LineTraceComponent(Hit,Near+FVector(0,0,6000),Near-FVector(0,0,6000),Q)||Hit.ImpactNormal.Z<.75f)return nullptr;
  auto* A=G->GetWorld()->SpawnActor<AActor>();auto* R=NewObject<USceneComponent>(A);R->SetMobility(EComponentMobility::Movable);A->SetRootComponent(R);R->RegisterComponent();A->SetActorLocation(Hit.ImpactPoint+Hit.ImpactNormal*.5f);
  FVector To=G->HomeActor->GetActorLocation()-A->GetActorLocation();To.Z=0;A->SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector,Hit.ImpactNormal)*FRotator(0,To.Rotation().Yaw,0).Quaternion());A->SetOwner(G);A->SetActorEnableCollision(false);A->SetActorTickEnabled(false);A->PrimaryActorTick.bCanEverTick=false;A->Tags.Add(Tag);Ops->SupportActors.Add(A);return A;
 }
 UStaticMeshComponent* Mesh(AActor* A,UStaticMesh* Asset,FVector P,FVector Size,int32 Material,FQuat Rotation=FQuat::Identity,const FName& Tag=NAME_None){
  if(!A||!Asset||Count>=128)return nullptr;auto* C=NewObject<UStaticMeshComponent>(A);C->SetupAttachment(A->GetRootComponent());C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Asset);for(int32 I=0;I<FMath::Max(1,Asset->GetStaticMaterials().Num());++I)C->SetMaterial(I,M[Material]);C->SetRelativeLocation(P);C->SetRelativeRotation(Rotation);C->SetRelativeScale3D(Size/100);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetGenerateOverlapEvents(false);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->PrimaryComponentTick.bCanEverTick=false;if(!Tag.IsNone())C->ComponentTags.Add(Tag);C->RegisterComponent();++Count;return C;
 }
 UStaticMeshComponent* Box(AActor* A,FVector P,FVector Size,int32 Material=0,const FName& Tag=NAME_None){return Mesh(A,Cube,P,Size,Material,FQuat::Identity,Tag);}
 void Rod(AActor* A,FVector From,FVector To,float Radius=2,int32 Material=2){const FVector D=To-From;Mesh(A,Cylinder,(From+To)*.5,FVector(Radius*2,Radius*2,D.Size()),Material,FRotationMatrix::MakeFromZ(D).ToQuat());}
 void Text(AActor* A,const FString& Value,FVector P,float Size){if(!A)return;auto* T=NewObject<UTextRenderComponent>(A);T->SetupAttachment(A->GetRootComponent());T->SetRelativeLocation(P);T->SetRelativeRotation(FRotator(0,180,0));T->SetText(FText::FromString(Value));T->SetWorldSize(Size);T->SetHorizontalAlignment(EHTA_Center);T->SetTextRenderColor(FColor(200,250,255));T->SetCollisionEnabled(ECollisionEnabled::NoCollision);T->SetCanEverAffectNavigation(false);T->SetCastShadow(false);T->PrimaryComponentTick.bCanEverTick=false;T->RegisterComponent();}
 void PackageStand(AActor* A){
  // The cartridge stays reachable at its existing interaction height. Its
  // stationary decorative delivery robot has no gameplay role and is removed.
  Box(A,FVector(0,0,4),FVector(48,42,8),1);Box(A,FVector(0,0,26),FVector(14,14,44),2);Box(A,FVector(0,0,48),FVector(42,38,4),1);
 }
};
}
void UFieldOperationsComponent::BuildWorld(){
 auto* G=Game();auto* P=Pilot();if(!Enabled||!G||!G->GetWorld()||!G->HomeActor||!P)return;
 FSupportBuilder B(this);if(!B.Ready()){UE_LOG(LogTemp,Warning,TEXT("FIELD_OPERATIONS missing retained terrain/material. Main mission remains available; no fallback measured terrain created."));return;}
 if(!IsValid(ResourceBoard)){
  ResourceBoard=B.Actor(G->HomeActor->GetActorLocation()+FVector(720,-520,0),TEXT("Field.ResourceBoard"));auto* A=ResourceBoard.Get();
  if(!G->Apollo&&!G->Mars){
   // Retain the exact service anchor/interaction rules and hotkey, without
   // the standing instruction display. A small dock marks the service point.
   B.Box(A,FVector(0,0,5),FVector(90,65,10),1);
   for(int32 I=0;I<4;++I)B.Box(A,FVector(-24+16*I,0,11),FVector(8,24,2),3);
   A->Tags.Add(TEXT("Lunar.ServiceDock.NoInstructionBoard"));
  }else{
  B.Box(A,FVector(0,0,5),FVector(90,65,10),1);B.Rod(A,FVector(0,0,10),FVector(0,0,120),5);B.Box(A,FVector(0,0,175),FVector(20,170,115));B.Box(A,FVector(-12,0,175),FVector(3,156,101),1);
  for(int32 I=0;I<4;++I)B.Box(A,FVector(-14,-60+40*I,135),FVector(3,22,5),3);
  B.Text(A,TEXT("FIELD OPERATIONS"),FVector(-16,0,205),12);B.Text(A,TEXT("B / RESOURCES + MAP"),FVector(-16,0,178),8);B.Text(A,TEXT("GAMEPLAY MODEL"),FVector(-16,0,153),6);
  }
 }
 Vehicle=ALunarSurfaceVehicle::Build(G);
 if(Vehicle&&!HasVehicleHomePose){VehicleHomePose=Vehicle->GetActorTransform();HasVehicleHomePose=true;}
 SamplePackages.SetNum(3);
 for(int32 I=0;I<3;++I){
  if(IsValid(SamplePackages[I]))continue;
  AActor* Anchor=I==0?G->HomeActor.Get():I==1?(G->PowerActor?G->PowerActor.Get():G->HomeActor.Get()):(G->DroneActor?G->DroneActor.Get():G->DataActor?G->DataActor.Get():G->HomeActor.Get());
  const FVector Offset=I==0?FVector(1800,900,0):I==1?FVector(950,-850,0):FVector(-1000,850,0);
  FVector At=Anchor->GetActorLocation()+Offset;if(!B.Site(At)){UE_LOG(LogTemp,Warning,TEXT("FIELD_OPERATIONS no clear authored cartridge site %d; optional prop omitted rather than embedding it in geometry."),I);continue;}auto* Robot=B.Actor(At,FName(*FString::Printf(TEXT("Field.SampleRobot.%d"),I)));if(!Robot)continue;B.PackageStand(Robot);if(!SampleRobot)SampleRobot=Robot;
  auto* Cartridge=B.Actor(At,FName(*FString::Printf(TEXT("Field.Package.%d"),I)));if(!Cartridge)continue;
  B.Box(Cartridge,FVector(0,0,64),FVector(34,30,28),0,TEXT("work.focus"));B.Box(Cartridge,FVector(0,0,80),FVector(38,34,4),2);B.Box(Cartridge,FVector(-18,0,65),FVector(2,22,15),1);B.Box(Cartridge,FVector(-20,0,67),FVector(1,14,3),3);
  B.Text(Cartridge,FString::Printf(TEXT("TRAINING-%02d"),I+1),FVector(-22,0,68),3.5f);SamplePackages[I]=Cartridge;
 }
 if(ScannerVisuals.IsEmpty()&&P->Camera){
  auto Make=[&](FVector From,FVector To,float Radius){auto* C=NewObject<UStaticMeshComponent>(P);C->SetupAttachment(P->Camera);C->SetStaticMesh(B.Cylinder);C->SetMaterial(0,B.M[2]);const FVector D=To-From;C->SetRelativeLocation((From+To)*.5);C->SetRelativeRotation(FRotationMatrix::MakeFromZ(D).Rotator());C->SetRelativeScale3D(FVector(Radius*2/100,Radius*2/100,D.Size()/100));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetOnlyOwnerSee(true);C->SetCastShadow(false);C->PrimaryComponentTick.bCanEverTick=false;C->SetVisibility(false);C->RegisterComponent();ScannerVisuals.Add(C);};
  const FVector Centre(50,0,0);for(int32 I=0;I<20;++I){const float A=I*2*PI/20,Beta=(I+1)*2*PI/20;Make(Centre+FVector(0,FMath::Cos(A)*5,FMath::Sin(A)*5),Centre+FVector(0,FMath::Cos(Beta)*5,FMath::Sin(Beta)*5),.38f);}Make(Centre+FVector(0,0,-5),FVector(43,10,-17),.8f);
 }
 RefreshWorldVisibility();UE_LOG(LogTemp,Display,TEXT("FIELD_OPERATIONS authored support meshes=%d/128 packages=%d vehicle=%d scanner_parts=%d NASA_terrain_changed=0"),B.Count,SamplePackages.Num(),IsValid(Vehicle),ScannerVisuals.Num());
}
