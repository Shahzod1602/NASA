#include "LunarCarryPresentation.h"
#include "LunarGame.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UnrealClient.h"

namespace LunarCarryPrivate {
float ViewAspect(const ALunarCharacter* Player){
 if(Player&&Player->Camera&&Player->Camera->bConstrainAspectRatio&&Player->Camera->AspectRatio>0)return Player->Camera->AspectRatio;
 int32 Width=0,Height=0;if(Player)if(auto* PC=Cast<APlayerController>(Player->GetController()))PC->GetViewportSize(Width,Height);
 return Width>0&&Height>0?float(Width)/Height:16.f/9.f;
}
bool Frustum(float Fov,float Aspect,double& Horizontal,double& Vertical){
 if(!FMath::IsFinite(Fov)||!FMath::IsFinite(Aspect)||Fov<5||Fov>170||Aspect<.2f||Aspect>8)return false;
 Horizontal=FMath::Tan(FMath::DegreesToRadians(double(Fov)*.5));Vertical=Horizontal/Aspect;
 return FMath::IsFinite(Horizontal)&&FMath::IsFinite(Vertical)&&Vertical>0;
}
bool SourceBox(const FBox& Box){return Box.IsValid&&!Box.Min.ContainsNaN()&&!Box.Max.ContainsNaN()&&Box.GetSize().GetMin()>UE_SMALL_NUMBER;}
void CopyAppearance(UStaticMeshComponent* Target,UStaticMeshComponent* Source){
 if(!Target||!Source)return;Target->SetStaticMesh(Source->GetStaticMesh());
 for(int32 I=0;I<Source->GetNumMaterials();++I)if(Target->GetMaterial(I)!=Source->GetMaterial(I))Target->SetMaterial(I,Source->GetMaterial(I));
}
void BodyPose(UStaticMeshComponent* Component,FVector Center,float MaximumDimension,float Drop){
 if(!Component||!Component->GetStaticMesh())return;
 const FBox Bounds=Component->GetStaticMesh()->GetBoundingBox();if(!SourceBox(Bounds))return;
 const double Scale=MaximumDimension/Bounds.GetSize().GetMax();
 Component->SetRelativeTransform(FTransform(FQuat::Identity,Center-FVector(0,0,Drop)-Bounds.GetCenter()*Scale,FVector(Scale)));
}
bool CommittedSample(const ALunarGameMode* G){return G&&G->Apollo&&G->ApolloPackedPreviewRemaining>0&&G->ApolloPackedSample>=0&&G->ApolloPackedSample<3&&(G->SamplesCollected&(1<<G->ApolloPackedSample));}
bool CommittedPart(const ALunarGameMode* G){return G&&G->Apollo&&G->ApolloPackedPartRemaining>0&&G->ApolloPackedPart>=0&&G->ApolloPackedPart<3&&(G->ApolloParts&(1<<G->ApolloPackedPart));}
UStaticMeshComponent* RecordedPart(const ALunarGameMode* G){
 if(!G||!G->ApolloPartActors.IsValidIndex(G->ApolloPackedPart)||!G->ApolloPartActors[G->ApolloPackedPart])return nullptr;
 TArray<UStaticMeshComponent*> Parts;G->ApolloPartActors[G->ApolloPackedPart]->GetComponents(Parts);
 for(auto* C:Parts)if(C->ComponentHasTag(TEXT("work.focus"))&&C->GetStaticMesh())return C;
 for(auto* C:Parts)if(C->GetStaticMesh())return C;return nullptr;
}
}

bool LunarCarryPresentation::ProjectViewBounds(const FBox& Bounds,const FTransform& Relative,float Fov,float Aspect,FBox2D& Result){
 Result=FBox2D(ForceInit);double H=0,V=0;if(!LunarCarryPrivate::SourceBox(Bounds)||!LunarCarryPrivate::Frustum(Fov,Aspect,H,V)||Relative.ContainsNaN())return false;
 for(int32 I=0;I<8;++I){
  const FVector P=Relative.TransformPosition(FVector((I&1)?Bounds.Max.X:Bounds.Min.X,(I&2)?Bounds.Max.Y:Bounds.Min.Y,(I&4)?Bounds.Max.Z:Bounds.Min.Z));
  if(P.ContainsNaN()||P.X<=10)return false;
  Result+=FVector2D(.5+.5*P.Y/(P.X*H),.5-.5*P.Z/(P.X*V));
 }
 return Result.bIsValid&&!Result.Min.ContainsNaN()&&!Result.Max.ContainsNaN();
}
bool LunarCarryPresentation::FitViewBounds(const FBox& Bounds,float Fov,float Aspect,FTransform& Relative){
 Relative=FTransform::Identity;double H=0,V=0;if(!LunarCarryPrivate::SourceBox(Bounds)||!LunarCarryPrivate::Frustum(Fov,Aspect,H,V))return false;
 const FVector E=Bounds.GetExtent();constexpr double Depth=58,Safe=.86;
 // Keep the item above the expanded 150% lower field guide, not behind it.
 const FVector Center(Depth,0,.10*Depth*V);
 // Conservative perspective bounds include source depth: the nearest corner,
 // rather than the source origin, determines both projected size and margins.
 double Scale=.23;
 Scale=FMath::Min(Scale,Depth*.27*H/(E.Y+.27*H*E.X));
 Scale=FMath::Min(Scale,Depth*.32*V/(E.Z+.32*V*E.X));
 Scale=FMath::Min(Scale,(Safe*Depth*V-FMath::Abs(Center.Z))/(E.Z+Safe*V*E.X));
 Scale=FMath::Min(Scale,(Depth-12)/E.X);
 if(!FMath::IsFinite(Scale)||Scale<=UE_SMALL_NUMBER)return false;
 Relative=FTransform(FQuat::Identity,Center-Bounds.GetCenter()*Scale,FVector(Scale));
 FBox2D Screen;return ProjectViewBounds(Bounds,Relative,Fov,Aspect,Screen)&&Screen.Min.X>=.05&&Screen.Min.Y>=.05&&Screen.Max.X<=.95&&Screen.Max.Y<=.95;
}
bool LunarCarryPresentation::ShouldShow(const ALunarCharacter* P,const ALunarGameMode* G,bool FirstPerson,bool SuppressPhoto){
 return P&&P->Camera&&G&&!G->Menu&&G->IsActive()&&!G->Paused&&!G->Journal&&!P->ActivePanel&&
  P->ThirdPerson!=FirstPerson&&!SuppressPhoto&&P->PhotoCaptureFrames<=0&&!G->ApolloPhotoPending&&
  !(FScreenshotRequest::IsScreenshotRequested()&&!FScreenshotRequest::ShouldShowUI());
}
bool LunarCarryPresentation::IsCellCarried(const ALunarGameMode* G){return G&&!G->Apollo&&G->Stage==ELunarStage::RestorePower&&G->RepairStep==0;}
bool LunarCarryPresentation::CellGrip(const ALunarCharacter* P,float Side,FVector& Point){
 Point=FVector::ZeroVector;if(!P||!P->Camera||!P->CarriedModule||!P->CarriedModule->GetStaticMesh()||!FMath::IsFinite(Side))return false;
 const auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(P));if(!IsCellCarried(G))return false;
 const FBox B=P->CarriedModule->GetStaticMesh()->GetBoundingBox();FTransform Fit;
 if(!FitViewBounds(B,P->Camera->FieldOfView,LunarCarryPrivate::ViewAspect(P),Fit))return false;
 const FVector At(B.GetCenter().X-B.GetExtent().X*.15,B.GetCenter().Y+(Side<0?-1:1)*B.GetExtent().Y,B.GetCenter().Z);
 // The palm has width: placing its centre ON the box face buries half the
 // glove in a narrow-FOV cell. Fingers curl inward from this outside support.
 Point=Fit.TransformPosition(At);Point.Y+=(Side<0?-1.f:1.f)*6.f;return !Point.ContainsNaN();
}
void LunarCarryPresentation::Build(ALunarCharacter* P){
 if(!P||!P->Camera)return;
 auto Create=[&](const TCHAR* Name,USceneComponent* Parent){auto* C=NewObject<UStaticMeshComponent>(P,FName(Name));C->SetupAttachment(Parent);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->SetReceivesDecals(false);C->SetOnlyOwnerSee(true);C->SetComponentTickEnabled(false);C->SetVisibility(false);C->RegisterComponent();return C;};
 if(!P->CarriedModule){P->CarriedModule=Create(TEXT("CarryCellView"),P->Camera);P->CarriedModule->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell")));}
 P->CarriedModule->SetCanEverAffectNavigation(false);P->CarriedModule->SetComponentTickEnabled(false);P->CarriedModule->SetReceivesDecals(false);
 if(!P->BodyCarryItem)P->BodyCarryItem=Create(TEXT("CarryBodyItem"),P->GetCapsuleComponent());
}
void LunarCarryPresentation::Update(ALunarCharacter* P,bool SuppressPhoto){
 if(!P)return;Build(P);auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(P));
 if(P->CarriedModule)P->CarriedModule->SetVisibility(false);if(P->BodyCarryItem)P->BodyCarryItem->SetVisibility(false);
 // UpdateSuit deliberately leaves normal pause ticks still. Clear viewmodels
 // here too, so an open panel/photo/pause cannot retain the previous frame.
 if(!ShouldShow(P,G,true,SuppressPhoto)){
  for(UStaticMeshComponent* C:{P->WorkTool.Get(),P->SamplePouch.Get(),P->WorkPayload.Get(),P->DrivenTube.Get(),P->PouchSeal.Get()})if(C)C->SetVisibility(false);
  for(auto& C:P->ToolJaws)if(C)C->SetVisibility(false);if(P->PouchLabel)P->PouchLabel->SetVisibility(false);
 }
 if(!G)return;
 if(IsCellCarried(G)){
  if(P->CarriedModule&&P->CarriedModule->GetStaticMesh()&&ShouldShow(P,G,true,SuppressPhoto)){
   FTransform Fit;if(FitViewBounds(P->CarriedModule->GetStaticMesh()->GetBoundingBox(),P->Camera->FieldOfView,LunarCarryPrivate::ViewAspect(P),Fit)){P->CarriedModule->SetRelativeTransform(Fit);P->CarriedModule->SetVisibility(true);}
  }
  if(P->BodyCarryItem&&P->CarriedModule&&P->CarriedModule->GetStaticMesh()&&ShouldShow(P,G,false,SuppressPhoto)){
   LunarCarryPrivate::CopyAppearance(P->BodyCarryItem,P->CarriedModule);LunarCarryPrivate::BodyPose(P->BodyCarryItem,FVector(35,0,-8),28,0);P->BodyCarryItem->SetVisibility(true);
  }
  return;
 }
 if(!P->BodyCarryItem||!ShouldShow(P,G,false,SuppressPhoto))return;
 const bool Part=LunarCarryPrivate::CommittedPart(G),Sample=LunarCarryPrivate::CommittedSample(G);if(!Part&&!Sample)return;
 P->PrepareApolloWorkAssets();
 if(Part){auto* Recorded=LunarCarryPrivate::RecordedPart(G);if(!Recorded)return;LunarCarryPrivate::CopyAppearance(P->BodyCarryItem,Recorded);}
 else {UStaticMesh* Mesh=P->ApolloToolMeshes.FindRef(G->ApolloPackedSample==2?TEXT("SM_DriveTube"):TEXT("SM_SamplePouch"));if(!Mesh)return;P->BodyCarryItem->SetStaticMesh(Mesh);for(int32 I=0;I<Mesh->GetStaticMaterials().Num();++I)P->BodyCarryItem->SetMaterial(I,Mesh->GetMaterial(I));}
 const float Remaining=Part?G->ApolloPackedPartRemaining:G->ApolloPackedPreviewRemaining;
 const float Drop=FMath::Clamp((1.f-Remaining)/1.f,0.f,1.f)*13;
 LunarCarryPrivate::BodyPose(P->BodyCarryItem,FVector(34,-7,-9),Part?31.f:23.f,Drop);P->BodyCarryItem->SetVisibility(P->BodyCarryItem->GetStaticMesh()!=nullptr);
}
