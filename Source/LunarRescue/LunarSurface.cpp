#include "LunarGame.h"
#include "ApolloData.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

void ALunarGameMode::BuildApolloSurfaceDetail(){
 // Cosmetic authored debris; the NASA 2 m collision mesh and task positions
 // are unchanged. One HISM per existing rock mesh bounds draw/scene overhead.
 auto* Detail=GetWorld()->SpawnActor<AActor>();Detail->Tags.Add(TEXT("Apollo.SurfaceDetail"));
 auto* Root=NewObject<USceneComponent>(Detail);Detail->SetRootComponent(Root);Root->RegisterComponent();
 TArray<UHierarchicalInstancedStaticMeshComponent*> Batches;
 auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Apollo12/Realism/M_RegolithRock.M_RegolithRock"));
 for(int I=0;I<6;++I){
  auto* C=NewObject<UHierarchicalInstancedStaticMeshComponent>(Detail);C->SetupAttachment(Root);
  C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_%d.SM_Basalt_%d"),I,I)));
  if(Material)C->SetMaterial(0,Material);
  C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);
  C->SetCastShadow(false);C->SetCullDistances(2800,4200);C->bAutoRebuildTreeOnInstanceChanges=false;C->RegisterComponent();Batches.Add(C);
 }
 FRandomStream Rand(19691120);
 const FVector Route[]={ApolloData::Check,ApolloData::Instruments[0],ApolloData::Samples[0],ApolloData::Samples[1],ApolloData::Samples[2],ApolloData::Surveyor,ApolloData::Check};
 int Added=0;
 for(int I=0;I<9000;++I){
  const int Segment=Rand.RandRange(0,UE_ARRAY_COUNT(Route)-2);
  FVector P=FMath::Lerp(Route[Segment],Route[Segment+1],Rand.FRand());
  const float Angle=Rand.FRand()*2*PI,Radius=FMath::Sqrt(Rand.FRand())*1900;
  P+=FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);
  bool WorkArea=false;for(const FVector& Site:ApolloData::Samples)WorkArea|=FVector::DistSquared2D(P,Site)<10000;
  if(WorkArea||P.SizeSquared2D()<180000)continue;
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;
  if(!GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,5000),P-FVector(0,0,5000),ECC_Visibility,Q)||Hit.ImpactNormal.Z<.75f)continue;
  // Existing basalt assets are ~metre-scale. Most additions are 1-6 cm,
  // with sparse larger fragments. Partial burial avoids floating silhouettes.
  const float Scale=Rand.FRand()<.035f?Rand.FRandRange(.055f,.14f):Rand.FRandRange(.009f,.045f);
  const FQuat R=FRotator(Rand.FRandRange(-18,18),Rand.FRandRange(0,360),Rand.FRandRange(-15,15)).Quaternion();
  Batches[I%6]->AddInstance(FTransform(R,Hit.ImpactPoint-Hit.ImpactNormal*(Scale*7),FVector(Scale,Scale*Rand.FRandRange(.65f,1.2f),Scale*.55f)));
  ++Added;
 }
 for(auto* C:Batches)C->BuildTreeIfOutdated(false,true);
 UE_LOG(LogTemp,Display,TEXT("APOLLO_SURFACE_DETAIL %d cosmetic fragments in six collision-free batches"),Added);
}
