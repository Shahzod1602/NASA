#include "LunarGame.h"
#include "ApolloData.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "EngineUtils.h"

void ALunarGameMode::BuildApolloSurfaceDetail(){
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("Apollo.SurfaceDetail")))return;
 UStaticMeshComponent* Terrain=nullptr;
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName()==TEXT("SM_Apollo12Terrain")){Terrain=C;break;}}
 if(!Terrain){UE_LOG(LogTemp,Warning,TEXT("APOLLO_SURFACE_DETAIL skipped: imported collision surface missing"));return;}

 // Rebalance only the map's two shadowless artistic fill lights. Sun direction,
 // imported terrain and scientific coordinates remain untouched.
 int32 FillLights=0;
 for(TActorIterator<ADirectionalLight> It(GetWorld());It;++It){
  auto* Light=Cast<UDirectionalLightComponent>(It->GetLightComponent());
  if(Light&&!Light->CastShadows&&Light->Intensity>0.f&&Light->Intensity<1.f){
   Light->SetIntensity(Light->Intensity*2.5f);++FillLights;
  }else if(Light&&Light->CastShadows&&Light->Intensity>1.f){
   // Screen-space contact tracing produced distant ground stripes in the
   // matched render diagnostic. Keep ordinary shadow maps and ambient occlusion.
   Light->ContactShadowLength=0.f;Light->MarkRenderStateDirty();
  }
 }
 UE_LOG(LogTemp,Display,TEXT("APOLLO_PRESENTATION rebalanced %d existing shadowless fill lights"),FillLights);
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
  C->ComponentTags.Add(TEXT("Apollo.SurfaceBatch"));C->SetCastShadow(false);C->SetCullDistances(2800,4200);C->bAutoRebuildTreeOnInstanceChanges=false;C->RegisterComponent();Batches.Add(C);
 }
 FRandomStream Rand(19691120);
 const FVector Route[]={ApolloData::Check,ApolloData::Instruments[0],ApolloData::Samples[0],ApolloData::Samples[1],ApolloData::Samples[2],ApolloData::Surveyor,ApolloData::Check};
 const FVector WorkSites[]={ApolloData::Check,ApolloData::Instruments[0],ApolloData::Instruments[1],ApolloData::Samples[0],ApolloData::Samples[1],ApolloData::Samples[2],ApolloData::Surveyor};
 int Added=0,FineFragments=0,LocalClusters=0;
 for(int I=0;I<9000;++I){
  // Redistribute a bounded part of the existing budget toward the places the
  // player actually inspects, while leaving every work target's foreground clear.
  const bool NearWork=Rand.FRand()<.32f;FVector P;
  if(NearWork){P=WorkSites[Rand.RandRange(0,UE_ARRAY_COUNT(WorkSites)-1)];++LocalClusters;}
  else{const int Segment=Rand.RandRange(0,UE_ARRAY_COUNT(Route)-2);P=FMath::Lerp(Route[Segment],Route[Segment+1],Rand.FRand());}
  const float Angle=Rand.FRand()*2*PI,Radius=NearWork?FMath::Sqrt(FMath::Lerp(150.f*150.f,650.f*650.f,Rand.FRand())):FMath::Sqrt(Rand.FRand())*1900;
  P+=FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);
  bool WorkArea=false;for(const FVector& Site:WorkSites)WorkArea|=FVector::DistSquared2D(P,Site)<125.f*125.f;
  if(WorkArea||P.SizeSquared2D()<500.f*500.f||FVector::DistSquared2D(P,ApolloData::Surveyor)<250.f*250.f)continue;
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;
  // Trace only the actual NASA surface, not equipment tops or actor decorations.
  if(!Terrain->LineTraceComponent(Hit,P+FVector(0,0,5000),P-FVector(0,0,5000),Q)||Hit.ImpactNormal.ContainsNaN()||Hit.ImpactNormal.Z<.75f)continue;
  // Existing basalt assets are ~metre-scale. Most additions are 1-6 cm,
  // with sparse larger fragments. Partial burial avoids floating silhouettes.
  const bool Fine=NearWork&&Rand.FRand()<.3f;
  const float Scale=Fine?Rand.FRandRange(.005f,.016f):Rand.FRand()<.035f?Rand.FRandRange(.055f,.14f):Rand.FRandRange(.009f,.045f);
  if(Fine)++FineFragments;
  const FQuat R=FQuat::FindBetweenNormals(FVector::UpVector,Hit.ImpactNormal)*FRotator(Rand.FRandRange(-8,8),Rand.FRandRange(0,360),Rand.FRandRange(-8,8)).Quaternion();
  Batches[I%6]->AddInstance(FTransform(R,Hit.ImpactPoint-Hit.ImpactNormal*(Scale*7),FVector(Scale,Scale*Rand.FRandRange(.65f,1.2f),Scale*.55f)));
  ++Added;
 }
 for(auto* C:Batches)C->BuildTreeIfOutdated(false,true);
 UE_LOG(LogTemp,Display,TEXT("APOLLO_SURFACE_DETAIL %d cosmetic fragments in six collision-free batches; fine=%d work_cluster_attempts=%d cap=9000"),Added,FineFragments,LocalClusters);
}
