#include "LunarIntrepidVisualChecks.h"
#include "LunarIntrepidVisual.h"
#include "LunarIntrepidCabin.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "LunarFlightHUD.h"
#include "LunarFieldOperations.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/HUD.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "PhysicsEngine/BodySetup.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Async/Async.h"
#include "UnrealClient.h"
#include <limits>

namespace LunarIntrepidVisualChecks {
namespace {
TSharedPtr<FJsonObject> VectorJson(const FVector& V){auto J=MakeShared<FJsonObject>();J->SetNumberField(TEXT("x"),V.X);J->SetNumberField(TEXT("y"),V.Y);J->SetNumberField(TEXT("z"),V.Z);return J;}
TSharedPtr<FJsonObject> RotationJson(const FRotator& R){auto J=MakeShared<FJsonObject>();J->SetNumberField(TEXT("pitch"),R.Pitch);J->SetNumberField(TEXT("yaw"),R.Yaw);J->SetNumberField(TEXT("roll"),R.Roll);return J;}
FString Sha1(const FString& Text){FTCHARToUTF8 Utf8(*Text);FSHAHash H;FSHA1::HashBuffer(Utf8.Get(),Utf8.Length(),H.Hash);return H.ToString().ToLower();}
FString TransformRow(const FTransform& T){const FVector P=T.GetLocation(),S=T.GetScale3D();const FQuat Q=T.GetRotation();return FString::Printf(TEXT("%.5f,%.5f,%.5f|%.8f,%.8f,%.8f,%.8f|%.8f,%.8f,%.8f"),P.X,P.Y,P.Z,Q.X,Q.Y,Q.Z,Q.W,S.X,S.Y,S.Z);}
bool SafeLabel(const FString& Label){if(Label.IsEmpty()||Label.Len()>64)return false;for(TCHAR C:Label)if(!((C>='a'&&C<='z')||(C>='A'&&C<='Z')||(C>='0'&&C<='9')||C=='_'||C=='-'))return false;return true;}
bool ValidPng(const FString& Path,int32 Width,int32 Height){
 TArray<uint8> Data;if(!FFileHelper::LoadFileToArray(Data,*Path)||Data.Num()<33)return false;
 const uint8 Signature[]={137,80,78,71,13,10,26,10};for(int32 I=0;I<8;++I)if(Data[I]!=Signature[I])return false;
 if(Data[8]!=0||Data[9]!=0||Data[10]!=0||Data[11]!=13||Data[12]!='I'||Data[13]!='H'||Data[14]!='D'||Data[15]!='R')return false;
 auto U32=[&](int32 At){return (uint32(Data[At])<<24)|(uint32(Data[At+1])<<16)|(uint32(Data[At+2])<<8)|uint32(Data[At+3]);};
 return Width>0&&Height>0&&U32(16)==uint32(Width)&&U32(20)==uint32(Height);
}
struct FCameraView {FString Name;FVector Eye;FRotator Rotation;float Fov=85;};
struct FVisibility {TWeakObjectPtr<UPrimitiveComponent> Component;bool Visible=false,Shadow=false;};
class FRun:public TSharedFromThis<FRun>{
 TWeakObjectPtr<ALunarGameMode> Game;TWeakObjectPtr<ALunarCharacter> Player;TWeakObjectPtr<APlayerController> PC;
 UStaticMeshComponent* Terrain=nullptr;TWeakObjectPtr<ACameraActor> Camera;
 FString Folder,Label,Report,HeightHash,OutsideHash,OriginalHeightHash;int32 Failures=0,Passes=0;bool Review=false,Profile=false,Baseline=false,Done=false,CabinScope=false,CabinBaseline=false,CabinReview=false;
 TWeakObjectPtr<AStaticMeshActor> CabinActor;FTransform CabinPoseBefore;TArray<FVector> CabinPadsBefore;FString CabinLightsBefore;
 TSharedPtr<FJsonObject> Manifest=MakeShared<FJsonObject>();TArray<TSharedPtr<FJsonValue>> Frames,ViewsJson,Phases;
 TArray<FCameraView> Views;TArray<FVisibility> Visibility;
 double Started=0,WalkStarted=0,WalkDistance=0,ReviewWait=0;int32 WalkIndex=0,ReviewIndex=0,ProfileFrame=-300,ProfileFrames=6000,ProfilePhase=-1;bool Walking=false,ShotPending=false,ReviewPrepared=false,CaptureStarted=false;
 FVector Previous=FVector::ZeroVector;FDelegateHandle ShotHandle,CsvHandle;
 FTransform PlayerBefore;FRotator AimBefore;FVector VelocityBefore;EMovementMode ModeBefore=MOVE_None;uint8 CustomBefore=0;
 ELunarStage StageBefore=ELunarStage::Briefing;bool PausedBefore=false,GameTickBefore=false,PlayerTickBefore=false,MoveTickBefore=false,HiddenBefore=false,HUDShownBefore=true;ESlateVisibility FlightVisibility=ESlateVisibility::Visible;
 int32 QualityBefore=1,ScalabilityBefore=1;float OxygenBefore=0,ElapsedBefore=0,DistanceBefore=0;uint8 SamplesBefore=0,PartsBefore=0,InstrumentsBefore=0;
 const FVector WalkStarts[3]={FVector(-1700,-1200,0),FVector(-1900,900,0),FVector(900,1200,0)};
 const FVector WalkEnds[3]={FVector(-730,-140,0),FVector(-1100,900,0),FVector(1600,1200,0)};
 void Check(bool Good,const FString& Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(Good)++Passes;else{++Failures;UE_LOG(LogTemp,Warning,TEXT("INTREPID_VISUAL_CHECK FAIL %s"),*Name);}}
 bool Ground(const FVector& At,FHitResult& Hit)const{FCollisionQueryParams Q(SCENE_QUERY_STAT(IntrepidCheckGround),true);return Terrain&&Terrain->LineTraceComponent(Hit,FVector(At.X,At.Y,3000),FVector(At.X,At.Y,-3000),Q)&&!Hit.ImpactPoint.ContainsNaN()&&!Hit.ImpactNormal.ContainsNaN();}
 FString HeightGrid(bool Save){
  FString Rows;TArray<TSharedPtr<FJsonValue>> Points;bool Good=true;
  auto Add=[&](double X,double Y){FHitResult H;const bool Hit=Ground(FVector(X,Y,0),H);Good&=Hit;if(Hit){Rows+=FString::Printf(TEXT("%.4f,%.4f,%.4f,%.6f,%.6f,%.6f\n"),X,Y,H.ImpactPoint.Z,H.ImpactNormal.X,H.ImpactNormal.Y,H.ImpactNormal.Z);auto J=VectorJson(H.ImpactPoint);J->SetObjectField(TEXT("normal"),VectorJson(H.ImpactNormal));Points.Add(MakeShared<FJsonValueObject>(J));}};
  for(int32 Y=-4;Y<=4;++Y)for(int32 X=-4;X<=4;++X)Add(X*600.,Y*600.);
  for(const FVector& P:{FVector(-3000,-3000,0),FVector(3000,-3000,0),FVector(3000,3000,0),FVector(-3000,3000,0)})Add(P.X,P.Y);
  if(Save){Check(Good&&Points.Num()==85,TEXT("All 81 local and four outside collision-grid samples hit finite imported terrain"));Manifest->SetArrayField(TEXT("height_grid"),Points);Manifest->SetStringField(TEXT("height_grid_encoding"),TEXT("UTF8 x,y,z cm to4dec; normals to6dec, row-major; SHA1"));FFileHelper::SaveStringToFile(Rows,*(Folder/TEXT("height-grid.csv")));}
  return Good?Sha1(Rows):FString();
 }
 FString OutsideInstances(){TArray<FString> Rows;for(TActorIterator<AActor> It(Game->GetWorld());It;++It)if(It->ActorHasTag(TEXT("Apollo.SurfaceDetail"))){TArray<UHierarchicalInstancedStaticMeshComponent*> B;It->GetComponents(B);for(auto* C:B)for(int32 I=0;I<C->GetInstanceCount();++I){FTransform T;C->GetInstanceTransform(I,T,true);const FVector P=T.GetLocation();if(FMath::Abs(P.X)>2500||FMath::Abs(P.Y)>2500)Rows.Add((C->GetStaticMesh()?C->GetStaticMesh()->GetPathName():TEXT("missing"))+TEXT("|")+TransformRow(T));}}
  Rows.Sort();Manifest->SetNumberField(TEXT("outside_instance_count"),Rows.Num());const FString Text=FString::Join(Rows,TEXT("\n"));FFileHelper::SaveStringToFile(Text,*(Folder/TEXT("outside-instances.txt")));return Sha1(Text);
 }
 void StaticChecks(){
  auto* G=Game.Get();auto* P=Player.Get();Check(G&&G->Apollo&&!G->Mars&&!G->Menu&&P&&PC.IsValid(),TEXT("Driver owns an isolated Apollo world with a possessed astronaut"));
  Check(ULunarSettings::IsAutomation()&&!G->ApolloSaveEnabled(),TEXT("Automation never writes the owner's Apollo checkpoint"));
  Check(Terrain&&Terrain->GetStaticMesh()&&Terrain->GetStaticMesh()->GetPathName()==TEXT("/Game/Lunar/Apollo12/Meshes/SM_Apollo12Terrain.SM_Apollo12Terrain"),TEXT("The retained imported LROC terrain mesh is used directly"));
  if(!Terrain||!Terrain->GetStaticMesh())return;
  Check(Terrain->GetComponentTransform().Equals(FTransform::Identity,.001),TEXT("Imported terrain retains its original centimetre transform"));
  Check(Terrain->GetCollisionEnabled()!=ECollisionEnabled::NoCollision&&Terrain->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Block&&Terrain->GetCollisionResponseToChannel(ECC_Visibility)==ECR_Block,TEXT("Imported terrain still blocks the actual capsule and visibility queries"));
  auto* Body=Terrain->GetStaticMesh()->GetBodySetup();Check(Body&&Body->CollisionTraceFlag==CTF_UseComplexAsSimple,TEXT("Ground collision still uses the imported triangle surface"));
  Check(G->ModuleActor&&G->HomeActor==G->ModuleActor&&FVector::Dist2D(G->ModuleActor->GetActorLocation(),FVector(-500,-100,0))<1,TEXT("The registered equipment/return worksite keeps its original XY"));
  const double NaN=std::numeric_limits<double>::quiet_NaN();Check(!LunarIntrepidVisual::Inside(FVector(NaN,0,0))&&!LunarIntrepidVisual::Inside(FVector(2501,0,0))&&LunarIntrepidVisual::Inside(FVector(0,0,0)),TEXT("Visual ROI rejects invalid/outside coordinates and includes the lander anchor"));
  HeightHash=OriginalHeightHash=HeightGrid(true);OutsideHash=OutsideInstances();Check(!OutsideHash.IsEmpty(),TEXT("Outside original surface-instance inventory has a reproducible transform fingerprint"));
  TArray<TSharedPtr<FJsonValue>> Lights;int32 Sun=0;
  for(TActorIterator<ADirectionalLight> It(G->GetWorld());It;++It)if(auto* L=Cast<UDirectionalLightComponent>(It->GetLightComponent())){auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("name"),It->GetName());J->SetNumberField(TEXT("intensity"),L->Intensity);J->SetBoolField(TEXT("shadows"),L->CastShadows);J->SetObjectField(TEXT("rotation"),RotationJson(It->GetActorRotation()));Lights.Add(MakeShared<FJsonValueObject>(J));if(L->CastShadows&&L->Intensity>1)++Sun;}
  Manifest->SetArrayField(TEXT("light_snapshot"),Lights);Check(Sun==1,TEXT("The scene retains one shadow-casting lunar sun"));
  int32 Actors=0,MicroBatches=0,ShadowBatches=0,MicroCount=0,ShadowCount=0,AOCount=0,LayerLights=0,Grounded=0;bool Safe=true,Bounds=true,Apron=true,Material=true,Burial=true,Shadows=true,Cull=true,AO=true;TArray<FVector> LargeSites;
  for(TActorIterator<AActor> It(G->GetWorld());It;++It)if(It->ActorHasTag(TEXT("Apollo.IntrepidVisualSurface"))){++Actors;Safe&=!It->IsActorTickEnabled();TArray<ULightComponent*> L;It->GetComponents(L);LayerLights+=L.Num();TArray<UHierarchicalInstancedStaticMeshComponent*> Batches;It->GetComponents(Batches);
   for(auto* C:Batches){const bool Micro=C->ComponentHasTag(TEXT("Apollo.IntrepidVisualMicro")),Shadow=C->ComponentHasTag(TEXT("Apollo.IntrepidVisualShadow"));MicroBatches+=Micro;ShadowBatches+=Shadow;MicroCount+=Micro?C->GetInstanceCount():0;ShadowCount+=Shadow?C->GetInstanceCount():0;
    Safe&=(Micro!=Shadow)&&C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!C->CanEverAffectNavigation()&&!C->IsComponentTickEnabled()&&C->GetStaticMesh();Shadows&=C->CastShadow==Shadow;Cull&=C->InstanceEndCullDistance>0&&C->InstanceEndCullDistance<=(Micro?3200:5000);
    auto* Mat=C->GetMaterial(0);Material&=Mat&&Mat->GetMaterial()&&Mat->GetMaterial()->GetName()==TEXT("M_IntrepidRock");if(!C->GetStaticMesh())continue;const FBox Source=C->GetStaticMesh()->GetBoundingBox();
    for(int32 I=0;I<C->GetInstanceCount();++I){FTransform T;C->GetInstanceTransform(I,T,true);const FVector At=T.GetLocation();Bounds&=!T.ContainsNaN()&&T.GetScale3D().GetMin()>0;
     Apron&=FVector::Dist2D(At,FVector::ZeroVector)>=600&&FVector::Dist2D(At,FVector(-500,-100,0))>=180;
     for(int32 K=0;K<8;++K){const FVector V((K&1)?Source.Max.X:Source.Min.X,(K&2)?Source.Max.Y:Source.Min.Y,(K&4)?Source.Max.Z:Source.Min.Z);const FVector W=T.TransformPosition(V);Bounds&=FMath::Abs(W.X)<=2500.5&&FMath::Abs(W.Y)<=2500.5;}
     if(Shadow)LargeSites.Add(At);
     if(Shadow||I%31==0){FHitResult H;if(!Ground(At,H)){Burial=false;continue;}double Low=1.e30,High=-1.e30;for(int32 K=0;K<8;++K){const FVector V((K&1)?Source.Max.X:Source.Min.X,(K&2)?Source.Max.Y:Source.Min.Y,(K&4)?Source.Max.Z:Source.Min.Z);const double D=FVector::DotProduct(T.TransformPosition(V)-H.ImpactPoint,H.ImpactNormal);Low=FMath::Min(Low,D);High=FMath::Max(High,D);}const double Span=High-Low;Burial&=Span>0&&Low<0&&High>0&&(-Low/Span)>=.19&&(-Low/Span)<=.31;++Grounded;}
    }
   }
   TArray<UPostProcessComponent*> PP;It->GetComponents(PP);for(auto* A:PP){++AOCount;auto* Box=Cast<UBoxComponent>(A->GetAttachParent());AO&=!A->bUnbound&&Box&&FMath::IsNearlyEqual(Box->GetUnscaledBoxExtent().X,2250.f)&&FMath::IsNearlyEqual(Box->GetUnscaledBoxExtent().Y,2250.f)&&A->BlendRadius>=0&&Box->GetUnscaledBoxExtent().X+A->BlendRadius<=2500.5&&Box->GetUnscaledBoxExtent().Y+A->BlendRadius<=2500.5&&A->Settings.bOverride_AmbientOcclusionIntensity&&A->Settings.bOverride_AmbientOcclusionRadius&&!A->Settings.bOverride_AutoExposureBias&&!A->Settings.bOverride_BloomIntensity&&!A->Settings.bOverride_ColorGain;}
  }
  Check(Actors==(Baseline?0:1),TEXT("Baseline omits the new layer; feature mode owns one bounded layer actor"));
  Check(Baseline?(MicroBatches==0&&ShadowBatches==0):(MicroBatches==6&&ShadowBatches==3),TEXT("The local layer uses six fine and three shadowed instanced batches"));
  Check(Baseline?(MicroCount==0&&ShadowCount==0):(MicroCount>0&&MicroCount<=2048&&ShadowCount>0&&ShadowCount<=64),TEXT("Instance counts stay within the independent 2048 fine /64 shadow budget"));
  Check(Safe,TEXT("Every added mesh is finite, collision-free, navigation-free and without component or actor Tick"));Check(Bounds,TEXT("Every transformed added-rock bound stays within the 50m square"));Check(Apron,TEXT("The lander apron and registered kit foreground stay free of added rocks"));Check(Material,TEXT("Every local rock batch has the dedicated authored rock material"));Check(Shadows,TEXT("Only the limited medium batches cast dynamic shadows"));Check(Cull,TEXT("Every batch has bounded fine/medium culling distances"));
  bool Spaced=true;for(int32 I=0;I<LargeSites.Num();++I)for(int32 J=I+1;J<LargeSites.Num();++J)Spaced&=FVector::Dist2D(LargeSites[I],LargeSites[J])>=175;
  Check(Spaced,TEXT("Medium rock placements remain separated rather than forming dense shadow overdraw"));Check(Baseline||(Burial&&Grounded>=64),TEXT("Independent terrain rays place sampled rocks partially below and above ground without floating"));Check(AOCount==(Baseline?0:1)&&AO&&LayerLights==0,TEXT("The contact layer is bounded AO only, without added sun/fill/exposure/bloom changes"));
  auto* GroundMat=Terrain->GetMaterial(0);const FString GroundName=GroundMat&&GroundMat->GetMaterial()?GroundMat->GetMaterial()->GetName():TEXT("");Check(GroundName==(Baseline?TEXT("M_RegolithSurface"):TEXT("M_IntrepidRegolith")),TEXT("The baseline and local ground material modes are explicit"));
  float Half=0,Edge=0;const bool Params=GroundMat&&GroundMat->GetScalarParameterValue(FMaterialParameterInfo(TEXT("HalfExtentCM")),Half)&&GroundMat->GetScalarParameterValue(FMaterialParameterInfo(TEXT("EdgeFadeCM")),Edge);
  Check(Baseline||(Params&&FMath::IsNearlyEqual(Half,2500.f)&&FMath::IsNearlyEqual(Edge,350.f)),TEXT("The local material carries the correct centimetre ROI and edge-fade parameters"));
  Manifest->SetNumberField(TEXT("micro_instances"),MicroCount);Manifest->SetNumberField(TEXT("shadow_instances"),ShadowCount);Manifest->SetNumberField(TEXT("burial_samples"),Grounded);
 }

 FString SceneLights()const{
  TArray<FString> Rows;for(TActorIterator<AActor> It(Game->GetWorld());It;++It){TArray<ULightComponent*> Lights;It->GetComponents(Lights);for(auto* L:Lights)Rows.Add(FString::Printf(TEXT("%s|%s|%.6f|%d"),*L->GetClass()->GetName(),*L->GetName(),L->Intensity,int32(L->CastShadows)));}Rows.Sort();return FString::Join(Rows,TEXT("\n"));
 }
 TArray<FVector> ModelPads(AStaticMeshActor* Actor,UStaticMesh* Mesh)const{
  TArray<FVector> Result;if(!Actor||!Mesh)return Result;const auto B=Mesh->GetBounds();const FVector S=Actor->GetActorScale3D();if(S.X<=0||S.ContainsNaN())return Result;
  const FVector Datum(B.Origin.X,B.Origin.Y,B.Origin.Z-B.BoxExtent.Z);
  for(const FVector P:{FVector(-397.53,-5.19,0),FVector(397.53,-5.19,0),FVector(-4.21,-397.50,0),FVector(-4.34,397.48,0)})Result.Add(Actor->GetActorTransform().TransformPosition(Datum+P/S.X));return Result;
 }
 void CabinChecks(){
  if(!CabinScope)return;auto* Source=LoadObject<UStaticMesh>(nullptr,LunarIntrepidCabin::SourcePath);auto* Derived=LoadObject<UStaticMesh>(nullptr,LunarIntrepidCabin::DerivedPath);
  Check(Source&&Derived&&LunarIntrepidCabin::Compatible(Source,Derived),TEXT("Cabin derivative is present and compatible with the retained source bounds and section inventory"));
  int32 Actors=0;for(TActorIterator<AStaticMeshActor> It(Game->GetWorld());It;++It)if(It->ActorHasTag(TEXT("apollo.reference-lander"))){CabinActor=*It;++Actors;}
  auto* Actor=CabinActor.Get();auto* C=Actor?Actor->GetStaticMeshComponent():nullptr;UStaticMesh* Active=C?C->GetStaticMesh():nullptr;
  Check(Actors==1&&Active&&Active->GetPathName()==(CabinBaseline?LunarIntrepidCabin::SourcePath:LunarIntrepidCabin::DerivedPath),TEXT("The separate cabin baseline selects the source mesh and feature mode selects its derivative"));
  if(!Source||!Derived||!C||!Active)return;const auto A=Source->GetBounds(),B=Derived->GetBounds();const auto& Original=Source->GetStaticMaterials();const auto& Replacement=Derived->GetStaticMaterials();
  bool Inventory=Original.Num()==Replacement.Num();int32 Approved=0;TArray<TSharedPtr<FJsonValue>> Materials;
  bool Effective=true;int32 CabinParents=0;
  for(int32 I=0;I<Original.Num();++I){const FName Slot=Original[I].MaterialSlotName;const bool White=Slot==FName(TEXT("initialShadingGr_001"))||Slot==FName(TEXT("blinn3SG_001"));Approved+=White;
   Inventory&=Replacement.IsValidIndex(I)&&Replacement[I].MaterialSlotName==Slot&&Replacement[I].MaterialInterface==Original[I].MaterialInterface;
   UMaterialInterface* M=C->GetMaterial(I);auto* Instance=Cast<UMaterialInstance>(M);const bool Parent=Instance&&Instance->Parent&&Instance->Parent->GetPathName()==LunarIntrepidCabin::MaterialPath;CabinParents+=Parent;
   FLinearColor Tint;const bool HasTint=M&&M->GetVectorParameterValue(FMaterialParameterInfo(TEXT("CabinTint")),Tint);const float Expected=Slot==FName(TEXT("initialShadingGr_001"))?.498039216f:.678431392f;
   Effective&=M&&(White?(CabinBaseline?!Parent:(Parent&&HasTint&&FMath::IsNearlyEqual(Tint.R,Expected,.00001f)&&FMath::IsNearlyEqual(Tint.G,Expected,.00001f)&&FMath::IsNearlyEqual(Tint.B,Expected,.00001f))):!Parent);
   auto J=MakeShared<FJsonObject>();J->SetNumberField(TEXT("index"),I);J->SetStringField(TEXT("slot"),Slot.ToString());J->SetBoolField(TEXT("approved_shared_white_slot"),White);J->SetStringField(TEXT("source_material"),Original[I].MaterialInterface?Original[I].MaterialInterface->GetPathName():TEXT("missing"));J->SetStringField(TEXT("effective_material"),M?M->GetPathName():TEXT("missing"));J->SetStringField(TEXT("effective_parent"),Instance&&Instance->Parent?Instance->Parent->GetPathName():TEXT("none"));J->SetBoolField(TEXT("cabin_parent"),Parent);if(HasTint){J->SetNumberField(TEXT("tint_r"),Tint.R);J->SetNumberField(TEXT("tint_g"),Tint.G);J->SetNumberField(TEXT("tint_b"),Tint.B);}Materials.Add(MakeShared<FJsonValueObject>(J));
  }
  Check(Inventory&&Approved==2,TEXT("Source and derivative keep every material slot/default binding including exactly two independently audited shared white slots"));
  Check(Effective&&CabinParents==(CabinBaseline?0:2),TEXT("Only the two approved shared white slots receive cabin material instances with retained neutral base factors"));
  const FVector Scale=Actor->GetActorScale3D();const bool Bounds=A.Origin.Equals(B.Origin,.001)&&A.BoxExtent.Equals(B.BoxExtent,.001);
  Check(Bounds&&!Scale.ContainsNaN()&&Scale.X>0&&Scale.Equals(FVector(Scale.X),.0001)&&FMath::IsNearlyEqual(A.BoxExtent.Z*2*Scale.X,698.,2.)&&!Actor->GetActorTransform().ContainsNaN(),TEXT("The cabin keeps the finite source bounds and uniform 698cm presentation normalization"));
  CabinPoseBefore=Actor->GetActorTransform();CabinPadsBefore=ModelPads(Actor,Active);const auto SourcePads=ModelPads(Actor,Source);bool Pads=CabinPadsBefore.Num()==4&&SourcePads.Num()==4;TArray<TSharedPtr<FJsonValue>> PadJson;
  for(int32 I=0;I<CabinPadsBefore.Num();++I){Pads&=!CabinPadsBefore[I].ContainsNaN()&&SourcePads.IsValidIndex(I)&&CabinPadsBefore[I].Equals(SourcePads[I],.001);PadJson.Add(MakeShared<FJsonValueObject>(VectorJson(CabinPadsBefore[I])));}
  Check(Pads,TEXT("All four model pad anchors equal the source-derived world anchors without moving terrain or the lander"));
  TArray<ULightComponent*> Lights;Actor->GetComponents(Lights);CabinLightsBefore=SceneLights();
  Check(!Actor->GetActorEnableCollision()&&C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!Actor->IsActorTickEnabled()&&!C->IsComponentTickEnabled()&&Lights.Num()==0,TEXT("Cabin presentation remains non-colliding and without actor/component Tick or lander light components"));
  auto BoundsJson=[](const FBoxSphereBounds& V){auto J=MakeShared<FJsonObject>();J->SetObjectField(TEXT("origin"),VectorJson(V.Origin));J->SetObjectField(TEXT("box_extent"),VectorJson(V.BoxExtent));J->SetNumberField(TEXT("sphere_radius"),V.SphereRadius);return J;};
  Manifest->SetStringField(TEXT("cabin_mesh"),Active->GetPathName());Manifest->SetStringField(TEXT("cabin_source_mesh"),Source->GetPathName());Manifest->SetObjectField(TEXT("cabin_source_bounds"),BoundsJson(A));Manifest->SetObjectField(TEXT("cabin_derived_bounds"),BoundsJson(B));Manifest->SetArrayField(TEXT("cabin_pad_world"),PadJson);Manifest->SetArrayField(TEXT("cabin_effective_materials"),Materials);Manifest->SetStringField(TEXT("cabin_lights_sha1"),Sha1(CabinLightsBefore));Manifest->SetStringField(TEXT("cabin_normals_validation"),TEXT("Not inspected at runtime; require the separate source/editor builder receipt for 1237 main-cabin triangles. The two shared white material slots also include retained trim."));
 }
 void MakeCabinViews(){
  auto* Actor=CabinActor.Get();auto* C=Actor?Actor->GetStaticMeshComponent():nullptr;auto* Source=LoadObject<UStaticMesh>(nullptr,LunarIntrepidCabin::SourcePath);if(!Actor||!C||!Source){Check(false,TEXT("Cabin close cameras require the actual source-based reference actor"));return;}
  const auto B=Source->GetBounds();const double Scale=Actor->GetActorScale3D().X;const FVector Datum(B.Origin.X,B.Origin.Y,B.Origin.Z-B.BoxExtent.Z);const FTransform Pose=Actor->GetActorTransform();
  auto World=[&](const FVector& P){return Pose.TransformPosition(Datum+P/Scale);};
  // Bounds audited from the two original cabin OBJ objects, normalized to the
  // retained 698cm model. This is authored model geometry, not planetary data.
  const FVector Low(-181.8,-88.9,322.6),High(189.9,222.6,629.4),TargetCM(0,55,500);TArray<FVector> Corners;TArray<TSharedPtr<FJsonValue>> CornerJson;
  for(int32 I=0;I<8;++I){const FVector P=World(FVector((I&1)?High.X:Low.X,(I&2)?High.Y:Low.Y,(I&4)?High.Z:Low.Z));Corners.Add(P);CornerJson.Add(MakeShared<FJsonValueObject>(VectorJson(P)));}
  int32 W=0,H=0;PC->GetViewportSize(W,H);const double Aspect=H>0?double(W)/H:0,Tan=FMath::Tan(FMath::DegreesToRadians(30.));bool Good=Scale>0&&Aspect>0&&!Pose.ContainsNaN();const FVector Target=World(TargetCM);const FVector Eyes[]={FVector(-620,-570,570),FVector(570,330,570)};const TCHAR* Names[]={TEXT("cabin-front"),TEXT("cabin-side")};
  for(int32 I=0;I<2;++I){double Pullback=1;FCameraView V;V.Name=Names[I];V.Fov=60;bool Fits=false;
   for(int32 Attempt=0;Attempt<15;++Attempt){V.Eye=Target+(World(Eyes[I])-Target)*Pullback;V.Rotation=(Target-V.Eye).Rotation();const FQuat Q=V.Rotation.Quaternion();Fits=true;for(const FVector& Point:Corners){const FVector D=Q.UnrotateVector(Point-V.Eye);Fits&=!D.ContainsNaN()&&D.X>1&&FMath::Abs(D.Y)<=D.X*Tan*.92&&FMath::Abs(D.Z)<=D.X*Tan/Aspect*.92;}if(Fits)break;Pullback*=1.05;}
   FHitResult EyeHit,TargetHit;Good&=Fits&&!V.Eye.ContainsNaN()&&Ground(V.Eye,EyeHit)&&Ground(Target,TargetHit);Views.Add(V);auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("name"),V.Name);J->SetObjectField(TEXT("eye"),VectorJson(V.Eye));J->SetObjectField(TEXT("rotation"),RotationJson(V.Rotation));J->SetObjectField(TEXT("target"),VectorJson(Target));J->SetNumberField(TEXT("fov"),V.Fov);J->SetNumberField(TEXT("fit_pullback"),Pullback);J->SetNumberField(TEXT("aspect_ratio"),Aspect);J->SetBoolField(TEXT("focus_box_in_frustum"),Fits);J->SetArrayField(TEXT("focus_bounds_corners_world"),CornerJson);ViewsJson.Add(MakeShared<FJsonValueObject>(J));
  }
  Check(Good&&Views.Num()==2,TEXT("Two finite source-pose cabin close cameras fit all eight audited focus-box corners at FOV60 with ground coverage"));
  Manifest->SetStringField(TEXT("cabin_camera_bounds_basis"),TEXT("Independently audited main-cabin OBJ box in 698cm normalized model coordinates; actual runtime source datum and support pose"));
  if(!Good)return;Camera=Game->GetWorld()->SpawnActor<ACameraActor>();if(Camera.IsValid()){Camera->SetActorTickEnabled(false);Camera->GetCameraComponent()->SetFieldOfView(60);Camera->GetCameraComponent()->bConstrainAspectRatio=false;PC->SetViewTarget(Camera.Get());ApplyView(0);}
 }
 void HidePresentation(){auto* G=Game.Get();auto* P=Player.Get();if(auto* H=PC->GetHUD()){HUDShownBefore=H->bShowHUD;H->bShowHUD=false;}if(G->FlightHUD){FlightVisibility=G->FlightHUD->GetVisibility();G->FlightHUD->SetVisibility(ESlateVisibility::Hidden);}
  P->SetActorHiddenInGame(true);for(TActorIterator<AActor> It(G->GetWorld());It;++It){TArray<UPrimitiveComponent*> C;It->GetComponents(C);for(auto* V:C)if(V->GetOwner()==P||V->IsA<UTextRenderComponent>()){Visibility.Add({V,V->IsVisible(),bool(V->CastShadow)});V->SetVisibility(false);V->SetCastShadow(false);}}
 }
 void MakeViews(){
  if(CabinReview){MakeCabinViews();return;}
  FVector Eyes[]={FVector(-1600,-1400,420),FVector(-780,-260,125),FVector::ZeroVector,FVector(-17680,-2070,145)};
  FVector Targets[]={FVector(0,0,200),FVector(-650,-160,2),FVector::ZeroVector,FVector(-17520,-1840,2)};
  // Read the retained model's normalized source coordinates, exactly as the
  // cosmetic ladder detail does. Ground rays verify coverage but do not move
  // the camera away from the lander's actual tilted/posed ladder.
  bool LadderAnchor=false;
  for(TActorIterator<AStaticMeshActor> It(Game->GetWorld());It;++It)if(It->ActorHasTag(TEXT("apollo.reference-lander"))){
   auto* C=It->GetStaticMeshComponent();UStaticMesh* Mesh=C?C->GetStaticMesh():nullptr;if(!Mesh)break;
   const auto Bounds=Mesh->GetBounds();const FVector Scale=It->GetActorScale3D();
   if(Scale.ContainsNaN()||Scale.X<=0||!Scale.Equals(FVector(Scale.X),.0001f)||Bounds.Origin.ContainsNaN()||Bounds.BoxExtent.ContainsNaN()||Bounds.BoxExtent.GetMin()<=0||!FMath::IsNearlyEqual(Bounds.BoxExtent.Z*2*Scale.X,698.,2.)||Mesh->GetStaticMaterials().Num()==0||It->GetActorTransform().ContainsNaN())break;
   const FVector Datum(Bounds.Origin.X,Bounds.Origin.Y,Bounds.Origin.Z-Bounds.BoxExtent.Z);
   // The OBJ importer changes handedness: the retained ladder is on +Y in
   // Unreal. This is a review camera only, never a source-geometry edit.
   Eyes[2]=It->GetActorTransform().TransformPosition(Datum+FVector(-170,720,180)/Scale.X);
   Targets[2]=It->GetActorTransform().TransformPosition(Datum+FVector(-6,340,160)/Scale.X);
   LadderAnchor=!Eyes[2].ContainsNaN()&&!Targets[2].ContainsNaN();break;
  }
  Check(LadderAnchor,TEXT("The ladder camera uses the retained model's finite uniform 698cm source datum and actual pose"));if(!LadderAnchor)return;
  const TCHAR* Names[]={TEXT("intrepid-wide"),TEXT("fp-ground"),TEXT("ladder-foot"),TEXT("outside-ground")};bool CameraSupport=true;
  for(int32 I=0;I<4;++I){FHitResult E,T;const bool EyeSupport=Ground(Eyes[I],E),TargetSupport=Ground(Targets[I],T);CameraSupport&=EyeSupport&&TargetSupport;FCameraView V;V.Name=Names[I];V.Eye=I==2?Eyes[I]:FVector(Eyes[I].X,Eyes[I].Y,E.ImpactPoint.Z+Eyes[I].Z);const FVector Target=I==2?Targets[I]:FVector(Targets[I].X,Targets[I].Y,T.ImpactPoint.Z+Targets[I].Z);V.Rotation=(Target-V.Eye).Rotation();Views.Add(V);auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("name"),V.Name);J->SetObjectField(TEXT("eye"),VectorJson(V.Eye));J->SetObjectField(TEXT("rotation"),RotationJson(V.Rotation));J->SetNumberField(TEXT("fov"),85);ViewsJson.Add(MakeShared<FJsonValueObject>(J));}
  Check(CameraSupport,TEXT("Every fixed camera and target is anchored to a successful imported-terrain ray"));if(!CameraSupport)return;
  Camera=Game->GetWorld()->SpawnActor<ACameraActor>();if(Camera.IsValid()){Camera->SetActorTickEnabled(false);Camera->GetCameraComponent()->SetFieldOfView(85);Camera->GetCameraComponent()->bConstrainAspectRatio=false;PC->SetViewTarget(Camera.Get());ApplyView(0);}
 }
 void ApplyView(int32 Index){if(!Camera.IsValid()||!Views.IsValidIndex(Index))return;Camera->SetActorLocationAndRotation(Views[Index].Eye,Views[Index].Rotation);Camera->GetCameraComponent()->SetFieldOfView(Views[Index].Fov);PC->SetViewTarget(Camera.Get());if(PC->PlayerCameraManager)PC->PlayerCameraManager->UpdateCamera(0);}
 void Restore(){if(!Game.IsValid()||!Player.IsValid()||!PC.IsValid())return;auto* G=Game.Get();auto* P=Player.Get();auto* M=P->GetCharacterMovement();for(const auto& V:Visibility)if(V.Component.IsValid()){V.Component->SetVisibility(V.Visible);V.Component->SetCastShadow(V.Shadow);}if(auto* H=PC->GetHUD())H->bShowHUD=HUDShownBefore;if(G->FlightHUD)G->FlightHUD->SetVisibility(FlightVisibility);if(Camera.IsValid())Camera->Destroy();PC->SetViewTarget(P);P->SetActorTransform(PlayerBefore,false,nullptr,ETeleportType::TeleportPhysics);PC->SetControlRotation(AimBefore);M->SetMovementMode(ModeBefore,CustomBefore);M->Velocity=VelocityBefore;M->SetComponentTickEnabled(MoveTickBefore);P->SetActorHiddenInGame(HiddenBefore);P->SetActorTickEnabled(PlayerTickBefore);G->Stage=StageBefore;G->Paused=PausedBefore;G->SetActorTickEnabled(GameTickBefore);ULunarSettings::Get()->Quality=QualityBefore;ULunarSettings::Get()->Apply(P,false);if(auto* GS=UGameUserSettings::GetGameUserSettings()){GS->SetOverallScalabilityLevel(ScalabilityBefore);GS->ApplyNonResolutionSettings();}}
 void SaveManifest(){Manifest->SetNumberField(TEXT("schema_version"),1);Manifest->SetStringField(TEXT("kind"),Profile?TEXT("profile"):Review?TEXT("review"):TEXT("test"));Manifest->SetBoolField(TEXT("passed"),Failures==0);Manifest->SetBoolField(TEXT("baseline"),Baseline);Manifest->SetBoolField(TEXT("cabin_scope"),CabinScope);Manifest->SetBoolField(TEXT("cabin_baseline"),CabinBaseline);Manifest->SetBoolField(TEXT("cabin_review"),CabinReview);Manifest->SetBoolField(TEXT("synthetic"),true);Manifest->SetBoolField(TEXT("human_playtest"),false);Manifest->SetBoolField(TEXT("performance_measurement"),Profile);Manifest->SetBoolField(TEXT("stage_static_visual"),Review||Profile);Manifest->SetBoolField(TEXT("render_capable"),FApp::CanEverRender());Manifest->SetBoolField(TEXT("field_operations_enabled"),Game.IsValid()&&Game->FieldOperations&&Game->FieldOperations->Enabled);Manifest->SetNumberField(TEXT("pass_assertions"),Passes);Manifest->SetNumberField(TEXT("fail_assertions"),Failures);Manifest->SetStringField(TEXT("generated_utc"),FDateTime::UtcNow().ToIso8601());Manifest->SetStringField(TEXT("height_grid_sha1"),HeightHash);Manifest->SetStringField(TEXT("outside_instances_sha1"),OutsideHash);Manifest->SetStringField(TEXT("terrain_asset"),Terrain&&Terrain->GetStaticMesh()?Terrain->GetStaticMesh()->GetPathName():TEXT("missing"));Manifest->SetArrayField(TEXT("cameras"),ViewsJson);Manifest->SetArrayField(TEXT("frames"),Frames);Manifest->SetArrayField(TEXT("camera_phases"),Phases);int32 W=0,H=0;if(PC.IsValid())PC->GetViewportSize(W,H);Manifest->SetNumberField(TEXT("width"),W);Manifest->SetNumberField(TEXT("height"),H);FString Text;auto Writer=TJsonWriterFactory<>::Create(&Text);FJsonSerializer::Serialize(Manifest.ToSharedRef(),Writer);FFileHelper::SaveStringToFile(Text,*(Folder/TEXT("manifest.json")));}
 void Finish(){if(Done)return;Done=true;if(ShotHandle.IsValid())FScreenshotRequest::OnScreenshotRequestProcessed().Remove(ShotHandle);
#if CSV_PROFILER
  if(CsvHandle.IsValid())FCsvProfiler::Get()->OnCSVProfileFinished().Remove(CsvHandle);
#endif
  if(CabinScope&&CabinActor.IsValid()){bool Stable=CabinActor->GetActorTransform().Equals(CabinPoseBefore,.0001);const auto Pads=ModelPads(CabinActor.Get(),CabinActor->GetStaticMeshComponent()->GetStaticMesh());Stable&=Pads.Num()==CabinPadsBefore.Num();for(int32 I=0;I<Pads.Num();++I)Stable&=CabinPadsBefore.IsValidIndex(I)&&Pads[I].Equals(CabinPadsBefore[I],.001);Check(Stable&&SceneLights()==CabinLightsBefore,TEXT("Fixture execution leaves the source-derived cabin pose, four pad anchors and scene light inventory unchanged"));}
  if(Game.IsValid()){Check(HeightGrid(false)==OriginalHeightHash,TEXT("Fixture execution leaves the actual terrain collision grid unchanged"));Check(Game->Oxygen==OxygenBefore&&Game->ApolloElapsed==ElapsedBefore&&Game->ApolloDistance==DistanceBefore&&Game->SamplesCollected==SamplesBefore&&Game->ApolloParts==PartsBefore&&Game->ApolloInstruments==InstrumentsBefore,TEXT("Visual and movement fixtures never grant cargo or consume mission resources"));}
  Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);if(!FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("checks.txt"))))++Failures;SaveManifest();Restore();UE_LOG(LogTemp,Display,TEXT("INTREPID_VISUAL_%s %s assertions=%d frames=%d"),Profile?TEXT("PROFILE"):Review?TEXT("REVIEW"):TEXT("TEST"),Failures?TEXT("FAILED"):TEXT("COMPLETE"),Passes+Failures,Frames.Num());FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
 }
 bool StartWalk(){auto* P=Player.Get();auto* G=Game.Get();if(!P||!G||WalkIndex>=3)return false;FHitResult H;if(!Ground(WalkStarts[WalkIndex],H)){Check(false,TEXT("Live corridor start has imported terrain support"));return false;}const float Half=P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();P->SetActorLocation(H.ImpactPoint+FVector(0,0,Half+3),false,nullptr,ETeleportType::TeleportPhysics);auto* M=P->GetCharacterMovement();M->SetMovementMode(MOVE_Walking);M->SetComponentTickEnabled(true);M->StopMovementImmediately();P->SetActorTickEnabled(true);Previous=P->GetActorLocation();WalkStarted=FPlatformTime::Seconds();Walking=true;return true;}
 bool Tick(float Delta){if(Done||!Game.IsValid()||!Player.IsValid()||!PC.IsValid())return false;auto* P=Player.Get();auto* G=Game.Get();if(FPlatformTime::Seconds()-Started>(Profile?600:180)){Check(false,TEXT("Bounded driver timeout was not exceeded"));Finish();return false;}
  if(Profile){
#if CSV_PROFILER
   if(!CaptureStarted){if(++ProfileFrame>=0){Check(!FCsvProfiler::IsCapturing(),TEXT("CSV capture begins only after the controlled 300-frame settling period"));if(Failures){Finish();return false;}CaptureStarted=true;ProfileFrame=0;ProfilePhase=0;ApplyView(0);auto Initial=MakeShared<FJsonObject>();Initial->SetNumberField(TEXT("camera_index"),0);Initial->SetNumberField(TEXT("first_frame"),0);Initial->SetNumberField(TEXT("last_frame"),ProfileFrames/4-1);Initial->SetObjectField(TEXT("eye"),VectorJson(Views[0].Eye));Initial->SetObjectField(TEXT("rotation"),RotationJson(Views[0].Rotation));Phases.Add(MakeShared<FJsonValueObject>(Initial));const auto Self=AsShared();CsvHandle=FCsvProfiler::Get()->OnCSVProfileFinished().AddLambda([Self](const FString& File){AsyncTask(ENamedThreads::GameThread,[Self,File](){Self->Manifest->SetStringField(TEXT("csv_file"),FPaths::GetCleanFilename(File));Self->Manifest->SetNumberField(TEXT("captured_frames"),Self->ProfileFrames);Self->Check(Self->Phases.Num()==4,TEXT("Normal-delta capture records four fixed camera phases"));Self->Check(IFileManager::Get().FileSize(*File)>0,TEXT("The finite normal-delta CSV capture was written successfully"));Self->Finish();});});FCsvProfiler::SetMetadata(TEXT("IntrepidRun"),*Label);FCsvProfiler::SetMetadata(TEXT("IntrepidBaseline"),Baseline?TEXT("true"):TEXT("false"));FCsvProfiler::Get()->BeginCapture(ProfileFrames,Folder,TEXT("intrepid-profile.csv"));SaveManifest();}return true;}
   if(FCsvProfiler::IsCapturing()){const int32 Frame=FCsvProfiler::Get()->GetCaptureFrameNumber();const int32 Phase=FMath::Min(3,Frame*4/ProfileFrames);if(Phase!=ProfilePhase){ProfilePhase=Phase;ApplyView(Phase);auto J=MakeShared<FJsonObject>();J->SetNumberField(TEXT("camera_index"),Phase);J->SetNumberField(TEXT("first_frame"),Frame);J->SetNumberField(TEXT("last_frame"),(Phase+1)*ProfileFrames/4-1);J->SetObjectField(TEXT("eye"),VectorJson(Views[Phase].Eye));J->SetObjectField(TEXT("rotation"),RotationJson(Views[Phase].Rotation));Phases.Add(MakeShared<FJsonValueObject>(J));CSV_EVENT_GLOBAL(TEXT("Intrepid camera %d"),Phase);}}
#else
   Check(false,TEXT("CSV profiler is available in this build"));Finish();return false;
#endif
   return true;
  }
  if(Review){if(ShotPending||FPlatformTime::Seconds()<ReviewWait)return true;if(ReviewIndex>=Views.Num()*2){int32 W=0,H=0;PC->GetViewportSize(W,H);bool Complete=Frames.Num()==Views.Num()*2;for(const auto& Frame:Frames){const auto J=Frame->AsObject();Complete&=J.IsValid()&&J->GetBoolField(TEXT("good"))&&ValidPng(Folder/J->GetStringField(TEXT("path")),W,H);}Check(Complete,CabinReview?TEXT("Two cabin close cameras have four verified PNG files at Low and Medium without HUD"):TEXT("Four fixed cameras have eight verified PNG files at Low and Medium without HUD"));Finish();return false;}const int32 CameraIndex=ReviewIndex%Views.Num(),Quality=ReviewIndex/Views.Num();if(!ReviewPrepared){ULunarSettings::Get()->Quality=Quality;ULunarSettings::Get()->Apply(P,false);if(auto* GS=UGameUserSettings::GetGameUserSettings()){GS->SetOverallScalabilityLevel(Quality);GS->ApplyNonResolutionSettings();}ApplyView(CameraIndex);ReviewPrepared=true;ReviewWait=FPlatformTime::Seconds()+1.2;return true;}const int32 ActualQuality=UGameUserSettings::GetGameUserSettings()?UGameUserSettings::GetGameUserSettings()->GetOverallScalabilityLevel():-1;const FString Name=Views[CameraIndex].Name+FString::Printf(TEXT("-q%d"),Quality),Path=Folder/(Name+TEXT(".png"));ShotPending=true;const auto Self=AsShared();ShotHandle=FScreenshotRequest::OnScreenshotRequestProcessed().AddLambda([Self,Name,Path,CameraIndex,Quality,ActualQuality](){
 // Removing this delegate destroys its closure. Own every value on the stack
 // before removal; never read a captured value after this point.
 const auto Run=Self;const FString SavedName=Name,SavedPath=Path;const int32 SavedCamera=CameraIndex,SavedQuality=Quality,SavedActualQuality=ActualQuality;
 FScreenshotRequest::OnScreenshotRequestProcessed().Remove(Run->ShotHandle);Run->ShotHandle.Reset();int32 W=0,H=0;Run->PC->GetViewportSize(W,H);
 const bool Good=ValidPng(SavedPath,W,H)&&SavedActualQuality==SavedQuality;Run->Check(Good,TEXT("A staged fixed-camera PNG is written at the requested quality"));
 auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("name"),SavedName);J->SetStringField(TEXT("path"),SavedName+TEXT(".png"));J->SetBoolField(TEXT("good"),Good);J->SetNumberField(TEXT("quality"),SavedQuality);J->SetNumberField(TEXT("actual_quality"),SavedActualQuality);J->SetNumberField(TEXT("camera_index"),SavedCamera);J->SetObjectField(TEXT("eye"),VectorJson(Run->Views[SavedCamera].Eye));J->SetObjectField(TEXT("rotation"),RotationJson(Run->Views[SavedCamera].Rotation));J->SetNumberField(TEXT("fov"),Run->Views[SavedCamera].Fov);Run->Frames.Add(MakeShared<FJsonValueObject>(J));Run->ShotPending=false;Run->ReviewPrepared=false;++Run->ReviewIndex;Run->ReviewWait=FPlatformTime::Seconds();
});FScreenshotRequest::RequestScreenshot(Path,false,false);return true;}
  if(Walking){const FVector At=P->GetActorLocation();const double Step=FVector::Dist2D(At,Previous);if(Step<100)WalkDistance+=Step/100.;Previous=At;const FVector End=WalkEnds[WalkIndex];const double Remaining=FVector::Dist2D(At,End);PC->SetControlRotation((FVector(End.X,End.Y,At.Z)-At).Rotation());P->Forward(1);
   if(Remaining<35){P->GetCharacterMovement()->StopMovementImmediately();P->ConsumeMovementInputVector();Check(FVector::Dist2D(At,WalkStarts[WalkIndex])>=FVector::Dist2D(End,WalkStarts[WalkIndex])-65,FString::Printf(TEXT("Normal collision-enabled walking completes clear local corridor %d"),WalkIndex+1));if(WalkIndex==0){PC->SetControlRotation((G->ModuleActor->GetActorLocation()-(At+FVector(0,0,64))).Rotation());if(PC->PlayerCameraManager)PC->PlayerCameraManager->UpdateCamera(0);Check(G->CanInteractWith(G->ModuleActor),TEXT("The registered kit remains reachable and visible after a physical approach"));}Walking=false;++WalkIndex;if(WalkIndex>=3){Manifest->SetNumberField(TEXT("physical_walk_metres"),WalkDistance);Check(WalkDistance>25,TEXT("Local corridor fixtures measure real pawn travel rather than teleport distance"));Finish();return false;}if(!StartWalk()){Finish();return false;}}
   else if(FPlatformTime::Seconds()-WalkStarted>55){Check(false,FString::Printf(TEXT("Physical corridor %d cannot stall on the visual layer"),WalkIndex+1));Finish();return false;}
  }return true;
 }
public:
 FRun(ALunarGameMode* G,bool InReview,bool InProfile):Game(G),Review(InReview),Profile(InProfile),Baseline(FParse::Param(FCommandLine::Get(),TEXT("IntrepidVisualBaseline"))),CabinScope(FParse::Param(FCommandLine::Get(),TEXT("IntrepidCabinScope"))||FParse::Param(FCommandLine::Get(),TEXT("IntrepidCabinBaseline"))||FParse::Param(FCommandLine::Get(),TEXT("IntrepidCabinReview"))),CabinBaseline(FParse::Param(FCommandLine::Get(),TEXT("IntrepidCabinBaseline"))),CabinReview(FParse::Param(FCommandLine::Get(),TEXT("IntrepidCabinReview"))){}
 void Begin(){auto* G=Game.Get();if(!G)return;Player=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));PC=UGameplayStatics::GetPlayerController(G,0);if(!Player.IsValid()||!PC.IsValid()){FPlatformMisc::RequestExitWithStatus(false,1);return;}auto* P=Player.Get();Started=FPlatformTime::Seconds();FParse::Value(FCommandLine::Get(),TEXT("IntrepidRun="),Label);if(Label.IsEmpty())Label=FString(Profile?TEXT("profile-"):Review?TEXT("review-"):TEXT("test-"))+FDateTime::UtcNow().ToString(TEXT("%Y%m%d-%H%M%S-%s"))+FString::Printf(TEXT("-%u"),FPlatformProcess::GetCurrentProcessId());if(!SafeLabel(Label)){UE_LOG(LogTemp,Error,TEXT("INTREPID_VISUAL invalid ASCII run label"));FPlatformMisc::RequestExitWithStatus(false,1);return;}Folder=FPaths::ProjectDir()/(CabinScope?TEXT("Reports/IntrepidCabin20261111"):TEXT("Reports/IntrepidVisual20261111"))/Label;if(IFileManager::Get().DirectoryExists(*Folder)){UE_LOG(LogTemp,Error,TEXT("INTREPID_VISUAL refuses existing run folder"));FPlatformMisc::RequestExitWithStatus(false,1);return;}IFileManager::Get().MakeDirectory(*Folder,true);
  PlayerBefore=P->GetActorTransform();AimBefore=PC->GetControlRotation();auto* M=P->GetCharacterMovement();VelocityBefore=M->Velocity;ModeBefore=M->MovementMode;CustomBefore=M->CustomMovementMode;MoveTickBefore=M->IsComponentTickEnabled();StageBefore=G->Stage;PausedBefore=G->Paused;GameTickBefore=G->IsActorTickEnabled();PlayerTickBefore=P->IsActorTickEnabled();HiddenBefore=P->IsHidden();QualityBefore=ULunarSettings::Get()->Quality;if(auto* GS=UGameUserSettings::GetGameUserSettings())ScalabilityBefore=GS->GetOverallScalabilityLevel();OxygenBefore=G->Oxygen;ElapsedBefore=G->ApolloElapsed;DistanceBefore=G->ApolloDistance;SamplesBefore=G->SamplesCollected;PartsBefore=G->ApolloParts;InstrumentsBefore=G->ApolloInstruments;
  for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName()==TEXT("SM_Apollo12Terrain")){Terrain=C;break;}}
  G->SetActorTickEnabled(false);StaticChecks();CabinChecks();if(CabinReview&&!Review)Check(false,TEXT("Cabin close-camera mode is review-only"));if(!Terrain||Failures){Finish();return;}G->Paused=false;G->Stage=ELunarStage::ApolloCheckout;
  if(Review||Profile){Check(FApp::CanEverRender(),TEXT("Review/profile uses an actual rendered viewport, not NullRHI"));P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();P->SetActorTickEnabled(false);HidePresentation();MakeViews();Check(Camera.IsValid()&&Views.Num()==(CabinReview?2:4),CabinReview?TEXT("Two source-pose-based cabin cameras are available"):TEXT("Four source-terrain-based fixed cameras are available"));if(Profile){Check(!FApp::UseFixedTimeStep(),TEXT("Performance capture uses normal delta time without fixed-step acceleration"));FParse::Value(FCommandLine::Get(),TEXT("IntrepidProfileFrames="),ProfileFrames);ProfileFrames=FMath::Clamp(ProfileFrames,1200,12000);ProfileFrames-=ProfileFrames%4;Manifest->SetNumberField(TEXT("settle_frames"),300);Manifest->SetNumberField(TEXT("requested_profile_frames"),ProfileFrames);Manifest->SetStringField(TEXT("profile_workload"),TEXT("Four fixed cameras, frozen mission/pawn, normal engine delta; optional parked support actors as configured; no driving or human session"));}ReviewWait=FPlatformTime::Seconds()+1.5;}
  else{Check(P->GetCapsuleComponent()->GetCollisionEnabled()!=ECollisionEnabled::NoCollision,TEXT("Traversal uses the actual enabled astronaut capsule"));Check(!G->FieldOperations||!G->FieldOperations->Enabled||G->FieldOperations->Briefed,TEXT("Corridor automation preserves the resource-briefing movement gate"));bool Clear=true;const auto Shape=FCollisionShape::MakeCapsule(P->GetCapsuleComponent()->GetScaledCapsuleRadius(),P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());for(int32 I=0;I<3;++I){FHitResult A,B,H;Clear&=Ground(WalkStarts[I],A)&&Ground(WalkEnds[I],B);FCollisionQueryParams Q(SCENE_QUERY_STAT(IntrepidCapsuleCorridor),false);Q.AddIgnoredActor(P);Q.AddIgnoredActor(Terrain->GetOwner());const FVector Lift(0,0,P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+4);Clear&=!G->GetWorld()->SweepSingleByChannel(H,A.ImpactPoint+Lift,B.ImpactPoint+Lift,FQuat::Identity,ECC_Pawn,Shape,Q);}Check(Clear,TEXT("Real capsule sweeps find no added blocking geometry across three local corridors"));if(Failures||!StartWalk()){Finish();return;}}
  if(Failures){Finish();return;}const auto Self=AsShared();FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Self](float D){return Self->Tick(D);}));
 }
};
}
void Start(ALunarGameMode* Game,bool Review,bool Profile){if(Game){auto Run=MakeShared<FRun>(Game,Review,Profile);Run->Begin();}}
}
