#include "LunarGame.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

void ALunarGameMode::BuildApolloLanderDetail(){
 UStaticMeshComponent* Terrain=nullptr;
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){
  auto* C=It->GetStaticMeshComponent();
  if(C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName()==TEXT("SM_Apollo12Terrain")){Terrain=C;break;}
 }
 if(!Terrain){UE_LOG(LogTemp,Warning,TEXT("APOLLO_LANDER_DETAIL skipped: imported collision surface missing"));return;}
 auto Ground=[&](const FVector& P,FHitResult& Hit){
  FCollisionQueryParams Query;Query.bTraceComplex=true;
  return Terrain->LineTraceComponent(Hit,P+FVector(0,0,1500),P-FVector(0,0,1500),Query)&&!Hit.ImpactPoint.ContainsNaN()&&!Hit.ImpactNormal.ContainsNaN()&&Hit.ImpactNormal.Z>.75f;
 };
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){
  auto* C=It->GetStaticMeshComponent();UStaticMesh* Mesh=C?C->GetStaticMesh():nullptr;
  if(!Mesh||!It->ActorHasTag(TEXT("apollo.reference-lander"))||It->ActorHasTag(TEXT("Apollo.LanderDetailBuilt")))continue;
  // This pass poses the retained reference art only. A user-edited collider is
  // never moved: all original interaction and NASA collision actors stay put.
  bool HasCollision=false;TArray<UPrimitiveComponent*> Primitives;It->GetComponents(Primitives);
  if(It->GetActorEnableCollision())for(auto* Primitive:Primitives)HasCollision|=Primitive->GetCollisionEnabled()!=ECollisionEnabled::NoCollision;
  if(HasCollision){UE_LOG(LogTemp,Warning,TEXT("APOLLO_LANDER_DETAIL skipped: reference art has enabled collision"));continue;}
  const auto Bounds=Mesh->GetBounds();const FVector Scale=It->GetActorScale3D();
  if(Scale.ContainsNaN()||Scale.X<=0||!Scale.Equals(FVector(Scale.X),.0001f))continue;
  const float Uniform=float(Scale.X);
  // Four pad centres read from the existing Carbajal model, expressed in cm
  // at its existing 698 cm normalization. They are model pivots, not NASA
  // measurements of touchdown locations or a landing-gear physics model.
  const FVector PadCm[]={FVector(-397.53f,-5.19f,0),FVector(397.53f,-5.19f,0),FVector(-4.21f,-397.50f,0),FVector(-4.34f,397.48f,0)};
  const FVector Datum(Bounds.Origin.X,Bounds.Origin.Y,Bounds.Origin.Z-Bounds.BoxExtent.Z);
  FVector LocalPads[4],Points[4],MeanLocal=FVector::ZeroVector;
  bool Complete=true;
  for(int32 I=0;I<4;++I){LocalPads[I]=Datum+PadCm[I]/Uniform;MeanLocal+=LocalPads[I]/4;FHitResult Hit;
   if(!Ground(It->GetActorTransform().TransformPosition(LocalPads[I]),Hit)){Complete=false;break;}Points[I]=Hit.ImpactPoint;
  }
  if(!Complete){UE_LOG(LogTemp,Warning,TEXT("APOLLO_LANDER_DETAIL skipped: incomplete foot collision traces"));continue;}
  FVector Normal=FVector::CrossProduct(Points[1]-Points[0],Points[3]-Points[2]).GetSafeNormal();if(Normal.Z<0)Normal*=-1;
  if(Normal.ContainsNaN()||Normal.IsNearlyZero()||Normal.Z<FMath::Cos(FMath::DegreesToRadians(12.f))){UE_LOG(LogTemp,Warning,TEXT("APOLLO_LANDER_DETAIL skipped: steep/invalid support plane"));continue;}
  const FQuat Pose=FRotationMatrix::MakeFromZX(Normal,It->GetActorQuat().GetAxisX()).ToQuat();
  const float MeanHeight=float((Points[0].Z+Points[1].Z+Points[2].Z+Points[3].Z)/4);
  FVector Position=It->GetActorLocation();const float Correction=MeanHeight-float(Pose.RotateVector(MeanLocal*Uniform).Z)-float(Position.Z);
  if(!FMath::IsFinite(Correction)||FMath::Abs(Correction)>80){UE_LOG(LogTemp,Warning,TEXT("APOLLO_LANDER_DETAIL skipped: excessive support correction"));continue;}
  Position.Z+=Correction;It->SetActorLocationAndRotation(Position,Pose);It->Tags.Add(TEXT("Apollo.LanderDetailBuilt"));
  // Brushed existing metal makes the retained ladder/strut geometry readable;
  // textured panels and the previously assigned thermal blanket stay intact.
  if(auto* Metal=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Suit/Materials/M_Metal.M_Metal"))){
   for(int32 Slot=0;Slot<Mesh->GetStaticMaterials().Num();++Slot)if(Mesh->GetStaticMaterials()[Slot].MaterialSlotName.ToString().Contains(TEXT("blinn4SG")))C->SetMaterial(Slot,Metal);
  }
  auto* DetailActor=GetWorld()->SpawnActor<AActor>();DetailActor->Tags.Add(TEXT("Apollo.LanderDetail"));
  auto* Root=NewObject<USceneComponent>(DetailActor);DetailActor->SetRootComponent(Root);Root->SetMobility(EComponentMobility::Static);Root->RegisterComponent();
  TMap<FName,UHierarchicalInstancedStaticMeshComponent*> Batches;
  auto Batch=[&](const TCHAR* BatchKey,const TCHAR* Shape,const TCHAR* Material){
   const FName Key(BatchKey);if(auto** Existing=Batches.Find(Key))return *Existing;
   auto* B=NewObject<UHierarchicalInstancedStaticMeshComponent>(DetailActor,Key);B->SetupAttachment(Root);B->SetMobility(EComponentMobility::Static);
   B->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
   const FString Folder=FString(Material)==TEXT("M_Metal")?TEXT("/Game/Lunar/Suit/Materials/"):TEXT("/Game/Lunar/Apollo12/Realism/");
   B->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*(Folder+Material+TEXT(".")+Material)));
   B->SetCollisionEnabled(ECollisionEnabled::NoCollision);B->SetCanEverAffectNavigation(false);B->SetCastShadow(false);B->SetCullDistances(4500,7000);B->bAutoRebuildTreeOnInstanceChanges=false;
   B->ComponentTags.Add(TEXT("Apollo.LanderDetail"));B->RegisterComponent();Batches.Add(Key,B);return B;
  };
  auto AddLocal=[&](UHierarchicalInstancedStaticMeshComponent* B,FVector At,FVector Dimensions,FQuat Rotation=FQuat::Identity){
   const FTransform Local(Rotation,Datum+At/Uniform,Dimensions/Uniform);B->AddInstance(Local*It->GetActorTransform(),true);
  };
  auto* Soles=Batch(TEXT("ContactSoles"),TEXT("Cylinder"),TEXT("M_EquipmentDark"));Soles->ComponentTags.Add(TEXT("Apollo.LanderContact"));
  auto* Collars=Batch(TEXT("ContactCollars"),TEXT("Cylinder"),TEXT("M_Metal"));Collars->ComponentTags.Add(TEXT("Apollo.LanderContactCollar"));
  float LargestResidual=0;int32 Contacts=0;
  for(int32 I=0;I<4;++I){FHitResult Hit;const FVector Bottom=It->GetActorTransform().TransformPosition(LocalPads[I]);if(!Ground(Bottom,Hit))continue;
   const FQuat Align=FRotationMatrix::MakeFromZ(Hit.ImpactNormal).ToQuat();
   // Thin authored contact lip rests on the collision plane. The shallow collar
   // closes the retained rigid mesh's residual mismatch on uneven 2 m terrain.
   Soles->AddInstance(FTransform(Align,Hit.ImpactPoint+Hit.ImpactNormal*.9f,FVector(.76f,.76f,.018f)),true);++Contacts;
   const float Gap=FVector::DotProduct(Bottom-Hit.ImpactPoint,Hit.ImpactNormal);LargestResidual=FMath::Max(LargestResidual,FMath::Abs(Gap));
   if(Gap>1.8f&&Gap<=24.f)Collars->AddInstance(FTransform(Align,Hit.ImpactPoint+Hit.ImpactNormal*(Gap*.5f+1),FVector(.47f,.47f,(Gap+2)/100)),true);
  }
  auto* Seams=Batch(TEXT("BlanketSeams"),TEXT("Cube"),TEXT("M_EquipmentDark"));
  auto* Edges=Batch(TEXT("BlanketEdges"),TEXT("Cube"),TEXT("M_LunarFoil"));
  auto* Panels=Batch(TEXT("EquipmentPlates"),TEXT("Cube"),TEXT("M_EquipmentPaint"));
  auto* Fasteners=Batch(TEXT("EquipmentFasteners"),TEXT("Cylinder"),TEXT("M_Metal"));
  for(int32 Side=0;Side<4;++Side){
   const float Angle=Side*HALF_PI;const FQuat Turn=FRotator(0,FMath::RadiansToDegrees(Angle),0).Quaternion();
   auto Face=[&](FVector V){return Turn.RotateVector(V);};
   for(float Along:{-56.f,56.f})AddLocal(Seams,Face(FVector(210.8f,Along,228.5f)),FVector(.006f,.012f,1.72f),Turn);
   for(float Z:{138.f,319.f})AddLocal(Edges,Face(FVector(210.8f,0,Z)),FVector(.009f,1.70f,.018f),Turn);
   AddLocal(Panels,Face(FVector(211.4f,0,184)),FVector(.012f,.34f,.24f),Turn);
   const FQuat BoltRotation=FRotationMatrix::MakeFromZ(Face(FVector::ForwardVector)).ToQuat();
   for(float Along:{-12.f,12.f})for(float Z:{176.f,192.f})AddLocal(Fasteners,Face(FVector(212.3f,Along,Z)),FVector(.018f,.018f,.006f),BoltRotation);
  }
  int32 Instances=0;for(auto& Pair:Batches){Pair.Value->BuildTreeIfOutdated(false,true);Instances+=Pair.Value->GetInstanceCount();}
  UE_LOG(LogTemp,Display,TEXT("APOLLO_LANDER_DETAIL contacts=%d batches=%d instances=%d height_cm=%.2f support_tilt_deg=%.2f z_correction_cm=%.2f max_rigid_residual_cm=%.2f collision_unchanged=1"),Contacts,Batches.Num(),Instances,float(Bounds.BoxExtent.Z*2*Uniform),FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(float(Normal.Z),-1.f,1.f))),Correction,LargestResidual);
 }
}
