#include "LunarIntrepidVisual.h"
#include "LunarGame.h"
#include "ApolloData.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace LunarIntrepidVisual {
namespace {
constexpr int32 FineBudget = 2048;
constexpr int32 ShadowBudget = 64;
const TCHAR* GroundPath = TEXT("/Game/Lunar/Apollo12/IntrepidVisual/M_IntrepidRegolith.M_IntrepidRegolith");
const TCHAR* RockPath = TEXT("/Game/Lunar/Apollo12/IntrepidVisual/M_IntrepidRock.M_IntrepidRock");

bool Enabled() {
 return !FParse::Param(FCommandLine::Get(), TEXT("IntrepidVisualBaseline"));
}

bool GroundAt(UStaticMeshComponent* Terrain, const FVector& At, FHitResult& Hit) {
 FCollisionQueryParams Query(SCENE_QUERY_STAT(IntrepidVisualGround), true);
 return Terrain->LineTraceComponent(Hit, At + FVector(0, 0, 3000), At - FVector(0, 0, 3000), Query)
  && !Hit.ImpactPoint.ContainsNaN() && !Hit.ImpactNormal.ContainsNaN() && Hit.ImpactNormal.Z > .85f;
}

bool ClearApron(const FVector& At) {
 // Retain a clear lander/ladder apron and the registered field-kit approach.
 // These are presentation exclusions, not new historical ground measurements.
 if (FVector::DistSquared2D(At, ApolloData::Home) < FMath::Square(620.f)
  || FVector::DistSquared2D(At, ApolloData::Check) < FMath::Square(200.f)) return false;
 const FVector2D P(At.X, At.Y);
 const FVector2D A(ApolloData::Check.X, ApolloData::Check.Y);
 const FVector2D B(-1700.f, -1200.f);
 const FVector2D D = B - A;
 const float T = FMath::Clamp(float(FVector2D::DotProduct(P - A, D) / D.SizeSquared()), 0.f, 1.f);
 return FVector2D::DistSquared(P, A + D * T) > FMath::Square(110.f);
}
}

bool Inside(const FVector& P) {
 return !P.ContainsNaN() && FMath::Abs(P.X - ApolloData::Home.X) <= HalfExtentCM
  && FMath::Abs(P.Y - ApolloData::Home.Y) <= HalfExtentCM;
}

bool Available(UStaticMeshComponent* Terrain) {
 if (!Enabled() || !Terrain || !Terrain->GetStaticMesh()
  || !LoadObject<UMaterialInterface>(nullptr, GroundPath) || !LoadObject<UMaterialInterface>(nullptr, RockPath)) return false;
 for (int32 I = 0; I < 6; ++I)
  if (!LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_%d.SM_Basalt_%d"), I, I))) return false;
 return true;
}

void Build(ALunarGameMode* Game, UStaticMeshComponent* Terrain) {
 if (!Game || !Game->Apollo || Game->Mars || Game->Menu || !Available(Terrain)) return;
 UWorld* World = Game->GetWorld();
 for (TActorIterator<AActor> It(World); It; ++It)
  if (It->ActorHasTag(TEXT("Apollo.IntrepidVisualSurface"))) return;

 auto* GroundMaterial = LoadObject<UMaterialInterface>(nullptr, GroundPath);
 auto* RockMaterial = LoadObject<UMaterialInterface>(nullptr, RockPath);
 TArray<UStaticMesh*> Meshes;
 for (int32 I = 0; I < 6; ++I) {
  auto* Mesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_%d.SM_Basalt_%d"), I, I));
  if (!Mesh) { UE_LOG(LogTemp, Warning, TEXT("INTREPID_VISUAL missing retained basalt; leaving terrain presentation unchanged")); return; }
  Meshes.Add(Mesh);
 }

 auto* Detail = World->SpawnActor<AActor>();
 if (!Detail) return;
 Detail->PrimaryActorTick.bCanEverTick = false;
 Detail->Tags.Add(TEXT("Apollo.IntrepidVisualSurface"));
 auto* Root = NewObject<USceneComponent>(Detail, TEXT("IntrepidVisualRoot"));
 Root->SetMobility(EComponentMobility::Static); Detail->SetRootComponent(Root); Root->RegisterComponent();

 // The shader preserves the previous material exactly outside the 50 m square;
 // it changes neither world-position offset nor the imported collision mesh.
 auto* GroundMID = UMaterialInstanceDynamic::Create(GroundMaterial, Detail);
 GroundMID->SetVectorParameterValue(TEXT("SliceCenter"), FLinearColor(ApolloData::Home.X, ApolloData::Home.Y, 0, 0));
 GroundMID->SetScalarParameterValue(TEXT("HalfExtentCM"), HalfExtentCM);
 GroundMID->SetScalarParameterValue(TEXT("EdgeFadeCM"), EdgeFadeCM);
 Terrain->SetMaterial(0, GroundMID);

 TArray<UHierarchicalInstancedStaticMeshComponent*> Fine, Shadow;
 auto Batch = [&](int32 MeshIndex, bool CastShadow) {
  const FName Name(*FString::Printf(TEXT("Intrepid%s_%d"), CastShadow ? TEXT("Shadow") : TEXT("Fine"), MeshIndex));
  auto* C = NewObject<UHierarchicalInstancedStaticMeshComponent>(Detail, Name);
  C->SetupAttachment(Root); C->SetMobility(EComponentMobility::Static);
  C->SetStaticMesh(Meshes[MeshIndex]); C->SetMaterial(0, RockMaterial);
  C->SetCollisionEnabled(ECollisionEnabled::NoCollision); C->SetCanEverAffectNavigation(false);
  C->SetCastShadow(CastShadow); C->SetReceivesDecals(false);
  C->SetCullDistances(CastShadow ? 3500 : 2000, CastShadow ? 5000 : 3200);
  C->bAutoRebuildTreeOnInstanceChanges = false;
  C->ComponentTags.Add(CastShadow ? TEXT("Apollo.IntrepidVisualShadow") : TEXT("Apollo.IntrepidVisualMicro"));
  C->RegisterComponent(); return C;
 };
 for (int32 I = 0; I < 6; ++I) Fine.Add(Batch(I, false));
 for (int32 I = 0; I < 3; ++I) Shadow.Add(Batch(I, true));

 FRandomStream Random(1969111901);
 TArray<FVector> Clusters, ShadowSites;
 for (int32 I = 0; I < 12; ++I)
  Clusters.Add(ApolloData::Home + FVector(Random.FRandRange(-2100, 2100), Random.FRandRange(-2100, 2100), 0));
 auto Position = [&](bool Small) {
  FVector P = ApolloData::Home + FVector(Random.FRandRange(-2450, 2450), Random.FRandRange(-2450, 2450), 0);
  if (Small && Random.FRand() < .65f) {
   const float Angle = Random.FRandRange(0, 2 * PI), Radius = FMath::Sqrt(Random.FRand()) * 420.f;
   P = Clusters[Random.RandRange(0, Clusters.Num() - 1)] + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0);
  }
  return P;
 };
 auto Place = [&](UHierarchicalInstancedStaticMeshComponent* C, FVector P, float WidthCM) {
  if (!Inside(P) || !ClearApron(P)) return false;
  const float Edge = HalfExtentCM - FMath::Max(FMath::Abs(float(P.X)), FMath::Abs(float(P.Y)));
  if (Random.FRand() > FMath::SmoothStep(0.f, EdgeFadeCM, Edge)) return false;
  FHitResult Hit; if (!GroundAt(Terrain, P, Hit)) return false;
  const FBoxSphereBounds Bounds = C->GetStaticMesh()->GetBounds();
  const float SourceWidth = float(FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y) * 2);
  if (SourceWidth <= 0) return false;
  const float S = WidthCM / SourceWidth;
  const FVector Scale(S, S * Random.FRandRange(.75f, 1.05f), S * Random.FRandRange(.55f, .85f));
  const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Hit.ImpactNormal)
   * FRotator(Random.FRandRange(-5, 5), Random.FRandRange(0, 360), Random.FRandRange(-5, 5)).Quaternion();
  // Derive burial from the actual transformed source bounds, rather than an
  // arbitrary pivot offset that hid ~75% of the old fine fragments.
  float Min = TNumericLimits<float>::Max(), Max = -TNumericLimits<float>::Max();
  for (int32 Corner = 0; Corner < 8; ++Corner) {
   const FVector Sign(Corner & 1 ? 1 : -1, Corner & 2 ? 1 : -1, Corner & 4 ? 1 : -1);
   const FVector Local = Bounds.Origin + Bounds.BoxExtent * Sign;
   const float Height = float(FVector::DotProduct(Rotation.RotateVector(Local * Scale), Hit.ImpactNormal));
   Min = FMath::Min(Min, Height); Max = FMath::Max(Max, Height);
  }
  const FVector At = Hit.ImpactPoint - Hit.ImpactNormal * (Min + (Max - Min) * .25f);
  for (int32 Corner = 0; Corner < 8; ++Corner) {
   const FVector Sign(Corner & 1 ? 1 : -1, Corner & 2 ? 1 : -1, Corner & 4 ? 1 : -1);
   const FVector WorldCorner = At + Rotation.RotateVector((Bounds.Origin + Bounds.BoxExtent * Sign) * Scale);
   if (!Inside(WorldCorner) || !ClearApron(WorldCorner)) return false;
  }
  C->AddInstance(FTransform(Rotation, At, Scale), true); return true;
 };
 int32 FineCount = 0, ShadowCount = 0;
 for (int32 Attempt = 0; Attempt < FineBudget * 4 && FineCount < FineBudget; ++Attempt) {
  const float Width = Random.FRand() < .85f ? Random.FRandRange(1.5f, 5.f) : Random.FRandRange(5.f, 11.f);
  if (Place(Fine[FineCount % Fine.Num()], Position(true), Width)) ++FineCount;
 }
 for (int32 Attempt = 0; Attempt < ShadowBudget * 12 && ShadowCount < ShadowBudget; ++Attempt) {
  const FVector P = Position(false);
  bool Spaced = true;
  for (const FVector& Other : ShadowSites) if (FVector::DistSquared2D(P, Other) < FMath::Square(180.f)) { Spaced = false; break; }
  if (Spaced && Place(Shadow[ShadowCount % Shadow.Num()], P, Random.FRandRange(11.f, 26.f))) { ShadowSites.Add(P); ++ShadowCount; }
 }
 for (auto* C : Fine) C->BuildTreeIfOutdated(false, true);
 for (auto* C : Shadow) C->BuildTreeIfOutdated(false, true);

 // A bounded contact-shading adjustment; the astronomical sun, black sky,
 // existing artistic fills and player exposure settings are unchanged.
 auto* Box = NewObject<UBoxComponent>(Detail, TEXT("IntrepidContactBounds"));
 Box->SetupAttachment(Root); Box->SetMobility(EComponentMobility::Static);
 // The outer 250 cm blend finishes at the same +/-25 m square boundary.
 Box->SetBoxExtent(FVector(HalfExtentCM - 250.f, HalfExtentCM - 250.f, 1000)); Box->SetRelativeLocation(FVector(0, 0, 250));
 Box->SetCollisionEnabled(ECollisionEnabled::NoCollision); Box->SetCanEverAffectNavigation(false); Box->RegisterComponent();
 auto* Contact = NewObject<UPostProcessComponent>(Detail, TEXT("IntrepidContactShading"));
 Contact->SetupAttachment(Box); Contact->bUnbound = false; Contact->BlendRadius = 250;
 Contact->BlendWeight = 1; Contact->Priority = 2;
 Contact->Settings.bOverride_AmbientOcclusionIntensity = true; Contact->Settings.AmbientOcclusionIntensity = .78f;
 Contact->Settings.bOverride_AmbientOcclusionRadius = true; Contact->Settings.AmbientOcclusionRadius = 35.f;
 Contact->ComponentTags.Add(TEXT("Apollo.IntrepidVisualLighting")); Contact->RegisterComponent();
 UE_LOG(LogTemp, Display, TEXT("INTREPID_VISUAL_SURFACE extent_cm=2500 edge_cm=350 micro=%d shadow=%d batches=9 burial=.25 collision_unchanged=1 data_unchanged=1"), FineCount, ShadowCount);
}
}
