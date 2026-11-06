#include "LunarCarryPresentation.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Math/RotationMatrix.h"
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
bool CommittedSample(const ALunarGameMode* G);
bool CommittedPart(const ALunarGameMode* G);
UStaticMeshComponent* CarryUpperSleeve(ALunarCharacter* P,int32 Side,bool Create){
 if(!P||!P->Camera)return nullptr;const FName Name(Side?TEXT("CarryUpperSleeveR"):TEXT("CarryUpperSleeveL"));auto* C=FindObjectFast<UStaticMeshComponent>(P,Name);if(C||!Create)return C;
 C=NewObject<UStaticMeshComponent>(P,Name);C->SetupAttachment(P->Camera);C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Suit/Meshes/SM_SuitUpperArm.SM_SuitUpperArm")));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetGenerateOverlapEvents(false);C->SetCastShadow(false);C->SetOnlyOwnerSee(true);C->SetVisibility(false);C->PrimaryComponentTick.bCanEverTick=false;C->SetLightingChannels(true,true,false);C->RegisterComponent();return C;
}
UPointLightComponent* CarryFill(ALunarCharacter* P,bool Create){
 if(!P||!P->Camera)return nullptr;const FName Name(TEXT("CarryReadabilityFill"));auto* L=FindObjectFast<UPointLightComponent>(P,Name);if(L||!Create)return L;
 // Conservative authored visibility aid; inspection overrides are restricted
 // to isolated automation and never change normal player settings or assets.
 float Lumens=3.f,Requested=0;bool Overridden=false;
 if(ULunarSettings::IsAutomation()&&FParse::Value(FCommandLine::Get(),TEXT("CarryFillLumens="),Requested)&&FMath::IsFinite(Requested)){Lumens=FMath::Clamp(Requested,0.f,20.f);Overridden=true;}
 L=NewObject<UPointLightComponent>(P,Name);L->SetupAttachment(P->Camera);L->SetMobility(EComponentMobility::Movable);L->SetRelativeLocation(FVector(8,0,14));L->SetIntensityUnits(ELightUnits::Lumens);L->SetIntensity(Lumens);L->SetAttenuationRadius(145);L->SetCastShadows(false);L->SetLightingChannels(false,true,false);L->SetVisibility(false);L->PrimaryComponentTick.bCanEverTick=false;L->RegisterComponent();
 UE_LOG(LogTemp,Display,TEXT("CARRY_READABILITY_FILL lumens=%.3f automation_override=%d channel=1 shadows=0"),Lumens,Overridden);return L;
}
void CarryReadability(ALunarCharacter* P,const ALunarGameMode* G,bool SuppressPhoto){
 const bool Show=LunarCarryPresentation::ShouldShow(P,G,true,SuppressPhoto)&&(LunarCarryPresentation::IsCellCarried(G)||CommittedPart(G)||CommittedSample(G));
 if(auto* L=CarryFill(P,false))L->SetVisibility(Show);
 if(Show){
  if(P->HandRig)P->HandRig->SetLightingChannels(true,true,false);
  for(auto& C:P->Gloves)if(C)C->SetLightingChannels(true,true,false);
  for(UStaticMeshComponent* C:{P->CarriedModule.Get(),P->WorkPayload.Get(),P->SamplePouch.Get(),P->PouchSeal.Get(),P->WorkTool.Get()})if(C)C->SetLightingChannels(true,true,false);
 }
 for(int32 I=0;I<2;++I)if(auto* Sleeve=CarryUpperSleeve(P,I,false)){
  const bool Upper=Show&&G&&G->Apollo&&(CommittedPart(G)||CommittedSample(G))&&P->Gloves.IsValidIndex(I*8)&&P->Gloves[I*8];Sleeve->SetVisibility(Upper&&Sleeve->GetStaticMesh());
  if(Upper){const FVector Shoulder(14,I?25.f:-25.f,-24),Elbow=P->Gloves[I*8]->GetRelativeLocation(),D=Elbow-Shoulder;
   if(!D.ContainsNaN()&&D.Size()>1&&D.Size()<35){Sleeve->SetRelativeLocation(Shoulder);Sleeve->SetRelativeRotation(FRotationMatrix::MakeFromX(D).Rotator());Sleeve->SetRelativeScale3D(FVector(D.Size()/28,1.025,1.025));}else Sleeve->SetVisibility(false);
  }
 }
}
const FName GripFrameTag(TEXT("Lunar.Carry.GripFrame"));
void AttachGripFrame(ALunarCharacter* Player,UStaticMeshComponent* Item,USceneComponent* Parent,const TCHAR* Name){
 if(!Player||!Item||!Parent)return;
 if(Item->GetAttachParent()&&Item->GetAttachParent()->ComponentHasTag(GripFrameTag))return;
 const FTransform Existing=Item->GetComponentTransform().GetRelativeTransform(Parent->GetComponentTransform());
 auto* Frame=NewObject<USceneComponent>(Player,FName(Name));Frame->SetupAttachment(Parent);Frame->SetMobility(EComponentMobility::Movable);Frame->ComponentTags.Add(GripFrameTag);Frame->PrimaryComponentTick.bCanEverTick=false;Frame->RegisterComponent();Frame->SetRelativeTransform(Existing);
 Item->AttachToComponent(Frame,FAttachmentTransformRules::KeepWorldTransform);Item->SetRelativeTransform(FTransform::Identity);
}
void ItemPose(UStaticMeshComponent* Item,const FTransform& Pose){
 if(!Item)return;if(auto* Frame=Item->GetAttachParent();Frame&&Frame->ComponentHasTag(GripFrameTag)){Frame->SetRelativeTransform(Pose);Item->SetRelativeTransform(FTransform::Identity);}else Item->SetRelativeTransform(Pose);
}
void BodyPose(UStaticMeshComponent* Component,FVector Center,float MaximumDimension,float Drop,FVector SourceScale=FVector::OneVector){
 if(!Component||!Component->GetStaticMesh())return;
 const FBox Bounds=Component->GetStaticMesh()->GetBoundingBox();if(!SourceBox(Bounds))return;
 const double Largest=(Bounds.GetSize()*SourceScale.GetAbs()).GetMax();if(!FMath::IsFinite(Largest)||Largest<=UE_SMALL_NUMBER||SourceScale.ContainsNaN())return;
 const FVector Scale=SourceScale*(MaximumDimension/Largest);
 ItemPose(Component,FTransform(FQuat::Identity,Center-FVector(0,0,Drop)-Bounds.GetCenter()*Scale,Scale));
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
bool LunarCarryPresentation::GripPose(const ALunarCharacter* P,bool FirstPerson,float Side,FVector& Wrist,FRotator& Rotation){
 if(!P||!P->Camera||!FMath::IsFinite(Side))return false;
 const auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(P));if(!G)return false;
 const float Sign=Side<0?-1.f:1.f;const bool Cell=IsCellCarried(G);UStaticMeshComponent* Item=nullptr;
 if(FirstPerson){
  if(Cell)Item=P->CarriedModule;
  else if(LunarCarryPrivate::CommittedPart(G))Item=P->WorkPayload;
  else if(LunarCarryPrivate::CommittedSample(G)){
   if(G->ApolloPackedSample==2)Item=P->WorkTool;
   else Item=Sign>0&&P->WorkPayload&&P->WorkPayload->IsVisible()?P->WorkPayload.Get():P->SamplePouch.Get();
  }
 }else if(Cell||LunarCarryPrivate::CommittedPart(G)||LunarCarryPrivate::CommittedSample(G))Item=P->BodyCarryItem;
 if(!Item||!Item->IsVisible()||!Item->GetStaticMesh())return false;
 const FBox B=Item->GetStaticMesh()->GetBoundingBox();if(!LunarCarryPrivate::SourceBox(B))return false;
 const FTransform Space=FirstPerson?P->Camera->GetComponentTransform():P->GetActorTransform();
 const FTransform Source=Item->GetComponentTransform().GetRelativeTransform(Space);
 // The retained battery has raised end bands: its outer AABB is not the
 // waist the gloves hold. The authored grip is at the source waist (0.715 of
 // outer Y radius), confirmed against the imported source geometry. Other
 // carried meshes use their own transformed side surfaces.
 const float Waist=Cell?.715f:1.f;const bool Pouch=Item->GetStaticMesh()->GetName()==TEXT("SM_SamplePouch");
 float PalmGap=2.35f;
 if(Cell){
  PalmGap+=FMath::Clamp((float(Source.GetScale3D().GetAbsMax())-.2f)*28.5f,0.f,3.f);
  // Keep the opposed thumb knuckle outside the central cell label at narrow
  // visor FOVs. Actual finger-surface contact is independently checked.
  if(FirstPerson)PalmGap+=1.2f;
 }
 else if(LunarCarryPrivate::CommittedPart(G)&&G->ApolloPackedPart==1)PalmGap=5.6f;
 const FVector ContactLocal(B.GetCenter().X-B.GetExtent().X*.12,B.GetCenter().Y+Sign*B.GetExtent().Y*Waist,B.GetCenter().Z-(Pouch?B.GetExtent().Z*.3:0));
 const FVector Normal=Source.GetRotation().RotateVector(FVector(0,Sign,0)).GetSafeNormal();
 const FVector Palm=Source.TransformPosition(ContactLocal)+Normal*PalmGap;
 const FRotator SolvedRotation=(Source.GetRotation()*FRotator(-8,-Sign*30,Sign*16).Quaternion()).Rotator();
 const FVector SolvedWrist=Palm-SolvedRotation.RotateVector(FVector(5,0,0));if(SolvedWrist.ContainsNaN()||SolvedRotation.ContainsNaN())return false;
 Wrist=SolvedWrist;Rotation=SolvedRotation;return true;
}
bool LunarCarryPresentation::CellGrip(const ALunarCharacter* P,float Side,FVector& Point){
 Point=FVector::ZeroVector;const auto* G=P?Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(P)):nullptr;
 FVector Wrist;FRotator Rotation;if(!IsCellCarried(G)||!GripPose(P,true,Side,Wrist,Rotation))return false;
 Point=Wrist+Rotation.RotateVector(FVector(5,0,0));return true;
}
void LunarCarryPresentation::Build(ALunarCharacter* P){
 if(!P||!P->Camera)return;
 auto Create=[&](const TCHAR* Name,USceneComponent* Parent){auto* C=NewObject<UStaticMeshComponent>(P,FName(Name));C->SetupAttachment(Parent);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->SetReceivesDecals(false);C->SetOnlyOwnerSee(true);C->SetComponentTickEnabled(false);C->SetVisibility(false);C->RegisterComponent();return C;};
 if(!P->CarriedModule){P->CarriedModule=Create(TEXT("CarryCellView"),P->Camera);P->CarriedModule->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell")));}
 P->CarriedModule->SetCanEverAffectNavigation(false);P->CarriedModule->SetComponentTickEnabled(false);P->CarriedModule->SetReceivesDecals(false);
 LunarCarryPrivate::AttachGripFrame(P,P->CarriedModule,P->Camera,TEXT("CellGripAnchor"));
 if(!P->BodyCarryItem)P->BodyCarryItem=Create(TEXT("CarryBodyItem"),P->GetCapsuleComponent());
 LunarCarryPrivate::AttachGripFrame(P,P->BodyCarryItem,P->GetCapsuleComponent(),TEXT("BodyGripAnchor"));
 LunarCarryPrivate::CarryFill(P,true);LunarCarryPrivate::CarryUpperSleeve(P,0,true);LunarCarryPrivate::CarryUpperSleeve(P,1,true);
}
void LunarCarryPresentation::Update(ALunarCharacter* P,bool SuppressPhoto){
 if(!P)return;Build(P);auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(P));
 if(P->CarriedModule)P->CarriedModule->SetVisibility(false);if(P->BodyCarryItem)P->BodyCarryItem->SetVisibility(false);
 LunarCarryPrivate::CarryReadability(P,G,SuppressPhoto);
 // UpdateSuit deliberately leaves normal pause ticks still. Clear viewmodels
 // here too, so an open panel/photo/pause cannot retain the previous frame.
 if(!ShouldShow(P,G,true,SuppressPhoto)){
  for(UStaticMeshComponent* C:{P->WorkTool.Get(),P->SamplePouch.Get(),P->WorkPayload.Get(),P->DrivenTube.Get(),P->PouchSeal.Get()})if(C)C->SetVisibility(false);
  for(auto& C:P->ToolJaws)if(C)C->SetVisibility(false);if(P->PouchLabel)P->PouchLabel->SetVisibility(false);
 }
 if(!G)return;
 if(IsCellCarried(G)){
  if(P->CarriedModule&&P->CarriedModule->GetStaticMesh()&&ShouldShow(P,G,true,SuppressPhoto)){
   FTransform Fit;if(FitViewBounds(P->CarriedModule->GetStaticMesh()->GetBoundingBox(),P->Camera->FieldOfView,LunarCarryPrivate::ViewAspect(P),Fit)){LunarCarryPrivate::ItemPose(P->CarriedModule,Fit);P->CarriedModule->SetVisibility(true);}
  }
  if(P->BodyCarryItem&&P->CarriedModule&&P->CarriedModule->GetStaticMesh()&&ShouldShow(P,G,false,SuppressPhoto)){
   LunarCarryPrivate::CopyAppearance(P->BodyCarryItem,P->CarriedModule);LunarCarryPrivate::BodyPose(P->BodyCarryItem,FVector(35,0,14),28,0);P->BodyCarryItem->SetVisibility(true);
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
 auto* RecordedScale=Part?LunarCarryPrivate::RecordedPart(G):nullptr;
 LunarCarryPrivate::BodyPose(P->BodyCarryItem,FVector(35,0,17),Part?28.f:23.f,Drop,RecordedScale?RecordedScale->GetComponentScale():FVector::OneVector);P->BodyCarryItem->SetVisibility(P->BodyCarryItem->GetStaticMesh()!=nullptr);
}
