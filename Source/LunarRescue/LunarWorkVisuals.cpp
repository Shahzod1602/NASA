#include "LunarGame.h"
#include "LunarPovMotion.h"
#include "LunarCarryPresentation.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UnrealClient.h"

namespace {
// Visual minimum-jerk interpolation: zero speed and acceleration at both ends.
// Tool input, commit points and cooldown durations remain owned by gameplay.
float ApolloWorkMinimumJerk(float T){if(!FMath::IsFinite(T))return 0;T=FMath::Clamp(T,0.f,1.f);return T*T*T*(T*(T*6-15)+10);}
// Committed carry is a bounded viewmodel, separate from the still-physical
// work tool and source actor. Preserve source proportions/appearance while
// keeping the item within the authored glove reach and supported view frustum.
bool BoundedCommittedPreview(ALunarCharacter* P,UStaticMeshComponent* Item,float MaximumDimension,float Drop=0,float Lateral=0,float Depth=55){
 if(!P||!P->Camera||!Item||!Item->GetStaticMesh())return false;
 const FBox Bounds=Item->GetStaticMesh()->GetBoundingBox();if(!Bounds.IsValid||Bounds.Min.ContainsNaN()||Bounds.Max.ContainsNaN())return false;
 const FTransform Original=Item->GetComponentTransform().GetRelativeTransform(P->Camera->GetComponentTransform());
 if(Original.ContainsNaN()||Original.GetScale3D().GetAbsMin()<=UE_SMALL_NUMBER)return false;
 const FQuat Rotation=Original.GetRotation();const FVector SourceScale=Original.GetScale3D();FBox Shape(ForceInit);
 for(int32 I=0;I<8;++I){const FVector V((I&1)?Bounds.Max.X:Bounds.Min.X,(I&2)?Bounds.Max.Y:Bounds.Min.Y,(I&4)?Bounds.Max.Z:Bounds.Min.Z);Shape+=Rotation.RotateVector((V-Bounds.GetCenter())*SourceScale);}
 const double Largest=Shape.GetSize().GetMax();if(!FMath::IsFinite(Largest)||Largest<=UE_SMALL_NUMBER)return false;
 int32 W=0,H=0;if(auto* PC=Cast<APlayerController>(P->GetController()))PC->GetViewportSize(W,H);
 const double Aspect=P->Camera->bConstrainAspectRatio?P->Camera->AspectRatio:W>0&&H>0?double(W)/H:16./9.;
 const double TanH=FMath::Tan(FMath::DegreesToRadians(double(P->Camera->FieldOfView)*.5)),TanV=TanH/Aspect;
 if(!FMath::IsFinite(TanH)||!FMath::IsFinite(TanV)||TanV<=0||!FMath::IsFinite(Drop)||!FMath::IsFinite(Lateral))return false;
 const FVector Extent=Shape.GetExtent();const double CentreZ=.10*Depth*TanV;
 double Factor=FMath::Min(1.,MaximumDimension/Largest);
 Factor=FMath::Min(Factor,(.76*Depth*TanH-FMath::Abs(Lateral))/(Extent.Y+.76*TanH*Extent.X));
 Factor=FMath::Min(Factor,(.76*Depth*TanV-FMath::Abs(CentreZ))/(Extent.Z+.76*TanV*Extent.X));
 if(!FMath::IsFinite(Factor)||Factor<=UE_SMALL_NUMBER)return false;
 const FVector Scale=SourceScale*Factor,Center(Depth,Lateral,CentreZ-Drop);
 Item->SetRelativeTransform(FTransform(Rotation,Center-Rotation.RotateVector(Bounds.GetCenter()*Scale),Scale));return true;
}
UStaticMeshComponent* SourceMesh(AActor* A){
 if(!A)return nullptr;TArray<UStaticMeshComponent*> Parts;A->GetComponents(Parts);
 for(auto* C:Parts)if(C->ComponentHasTag(TEXT("work.focus")))return C;
 return Parts.Num()?Parts[0]:nullptr;
}
}

void ALunarCharacter::PrepareApolloWorkAssets(){
 if(ApolloToolMeshes.Num())return;
 const TCHAR* Names[]={TEXT("SM_ContextCamera"),TEXT("SM_SampleTongs"),TEXT("SM_SoilScoop"),TEXT("SM_DriveTube"),TEXT("SM_GeologyHammer"),TEXT("SM_Cutters"),TEXT("SM_TongsBase"),TEXT("SM_TongsJawL"),TEXT("SM_TongsJawR"),TEXT("SM_CuttersBase"),TEXT("SM_CuttersJawL"),TEXT("SM_CuttersJawR"),TEXT("SM_SamplePouch")};
 for(const TCHAR* Name:Names)if(auto* LoadedTool=LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Apollo12/Tools/%s.%s"),Name,Name)))ApolloToolMeshes.Add(FName(Name),LoadedTool);
 ApolloToolMeshes.Add(TEXT("PayloadSphere"),LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));
 WorkSoilMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Apollo12/Realism/M_RegolithRock.M_RegolithRock"));
 auto Make=[&](){auto* C=NewObject<UStaticMeshComponent>(this);C->SetupAttachment(Camera);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(false);C->SetOnlyOwnerSee(true);C->SetVisibility(false);C->RegisterComponent();return C;};
 WorkTool=Make();SamplePouch=Make();WorkPayload=Make();DrivenTube=Make();PouchSeal=Make();ToolJaws.Add(Make());ToolJaws.Add(Make());
 SamplePouch->SetStaticMesh(ApolloToolMeshes.FindRef(TEXT("SM_SamplePouch")));
 PouchSeal->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 PouchSeal->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Suit/Materials/M_Cloth.M_Cloth")));
 PouchLabel=NewObject<UTextRenderComponent>(this);PouchLabel->SetupAttachment(Camera);PouchLabel->SetOnlyOwnerSee(true);PouchLabel->SetWorldSize(2.5f);PouchLabel->SetHorizontalAlignment(EHTA_Center);PouchLabel->SetTextRenderColor(FColor(225,235,235));PouchLabel->SetCastShadow(false);PouchLabel->SetVisibility(false);PouchLabel->RegisterComponent();
}

void ALunarCharacter::BeginApolloWork(AActor* Source){
 WorkSource=Source;
 WorkSourceMesh=SourceMesh(Source);if(WorkSourceMesh){WorkSourceTransform=WorkSourceMesh->GetComponentTransform();WorkContact=WorkSourceMesh->Bounds.Origin;}
}

void ALunarCharacter::UpdateApolloWorkVisuals(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));WorkGripActive=false;
 // Clear the previous mission/view before an early return; camera-only payloads
 // must not survive a panel, pause, photograph or mission/view switch.
 for(UStaticMeshComponent* C:{WorkTool.Get(),SamplePouch.Get(),WorkPayload.Get(),DrivenTube.Get(),PouchSeal.Get()})if(C)C->SetVisibility(false);
 if(PouchLabel)PouchLabel->SetVisibility(false);for(const auto& Jaw:ToolJaws)if(Jaw)Jaw->SetVisibility(false);
 if(!G||!G->Apollo)return;
 PrepareApolloWorkAssets();if(!WorkTool)return;
 if(!LunarCarryPresentation::ShouldShow(this,G,true))return;
 const int Tool=G->ApolloTool,Phase=G->ApolloToolPhase;
 const float RawMotion=1-LunarPovMotion::Ratio(G->ApolloToolCooldown,.75f);
 const float T=ApolloWorkMinimumJerk(RawMotion);
 const FTransform View=Camera->GetComponentTransform();
 auto Local=[&](FVector P){return View.InverseTransformPosition(P);};
 auto AssignToolMesh=[&](UStaticMeshComponent* C,const TCHAR* Name){C->SetStaticMesh(ApolloToolMeshes.FindRef(FName(Name)));C->SetVisibility(C->GetStaticMesh()!=nullptr);};
 auto Payload=[&](FVector P,bool Soil,UStaticMeshComponent* Recorded=nullptr){
  const FTransform SourceFrame=Recorded?Recorded->GetComponentTransform():WorkSourceTransform;
  if(Soil){WorkPayload->SetStaticMesh(ApolloToolMeshes.FindRef(TEXT("PayloadSphere")));WorkPayload->SetMaterial(0,WorkSoilMaterial);WorkPayload->SetRelativeScale3D(FVector(.08,.08,.025));}
  else if(auto* Source=Recorded?Recorded:WorkSourceMesh.Get()){WorkPayload->SetStaticMesh(Source->GetStaticMesh());for(int I=0;I<Source->GetNumMaterials();++I)WorkPayload->SetMaterial(I,Source->GetMaterial(I));WorkPayload->SetWorldScale3D(SourceFrame.GetScale3D());}
  WorkPayload->SetWorldLocation(P);WorkPayload->SetWorldRotation(SourceFrame.Rotator());WorkPayload->SetVisibility(WorkPayload->GetStaticMesh()!=nullptr);
 };
 // The soft pouch stays open until the final seal input. The committed preview
 // deposits the sample, closes the lip, then lowers it without blocking walking.
 const bool Packing=G->ApolloPackedPreviewRemaining>0;
 const int Sample=Packing?G->ApolloPackedSample:G->SelectedSample;
 if((Tool==4&&Phase>=1)||(Packing&&Sample<2)){
  AssignToolMesh(SamplePouch,TEXT("SM_SamplePouch"));PouchLabel->SetWorldSize(2.5f);
  const float PackedAge=FMath::Clamp(2.2f-G->ApolloPackedPreviewRemaining,0.f,2.2f);
  const float Drop=Packing?ApolloWorkMinimumJerk((PackedAge-1.35f)/.85f)*32:0;
  SamplePouch->SetRelativeLocation(FVector(80,-28,-24-Drop));SamplePouch->SetRelativeRotation(FRotator(0,-8,4));SamplePouch->SetRelativeScale3D(FVector::OneVector);
  const FString Caption=Sample==1?TEXT("12032"):TEXT("12031");if(PouchCaption!=Caption){PouchCaption=Caption;PouchLabel->SetText(FText::FromString(Caption));}PouchLabel->SetRelativeLocation(FVector(77.7f,-28,-25-Drop));PouchLabel->SetRelativeRotation(FRotator(0,180,0));PouchLabel->SetVisibility(true);
  if(Packing){
   // Separate the deposit, sealing sweep and lowering, using only the existing
   // committed preview clock. Cargo is already recorded; this awards nothing.
   const float Deposit=ApolloWorkMinimumJerk(PackedAge/.75f);
   const float Seal=ApolloWorkMinimumJerk((PackedAge-.65f)/.65f);
   const FVector Carried=FMath::Lerp(FVector(90,18,-8),FVector(80,-28,-23),Deposit)+FVector(0,0,6*FMath::Sin(Deposit*PI)-Drop);
   if(Deposit<1)Payload(View.TransformPosition(Carried),Sample==1);
   PouchSeal->SetRelativeLocation(FVector(80,-28,-15-Drop));PouchSeal->SetRelativeScale3D(FVector(.04*Seal,.13,.009));PouchSeal->SetRelativeRotation(FRotator(0,-8,4));PouchSeal->SetVisibility(Seal>.025f);
   const FTransform OldPouch=SamplePouch->GetRelativeTransform();
   if(BoundedCommittedPreview(this,SamplePouch,23,Drop*.625f)){
    const FTransform NewPouch=SamplePouch->GetRelativeTransform();
    PouchLabel->SetRelativeLocation(NewPouch.TransformPosition(OldPouch.InverseTransformPosition(PouchLabel->GetRelativeLocation())));
    PouchLabel->SetRelativeRotation((NewPouch.GetRotation()*OldPouch.GetRotation().Inverse()*PouchLabel->GetRelativeRotation().Quaternion()).Rotator());
    PouchLabel->SetWorldSize(2.5f*float(NewPouch.GetScale3D().GetAbsMax()/FMath::Max(.001,OldPouch.GetScale3D().GetAbsMax())));
    PouchSeal->SetRelativeLocation(NewPouch.TransformPosition(OldPouch.InverseTransformPosition(PouchSeal->GetRelativeLocation())));
    PouchSeal->SetRelativeRotation((NewPouch.GetRotation()*OldPouch.GetRotation().Inverse()*PouchSeal->GetRelativeRotation().Quaternion()).Rotator());
    PouchSeal->SetRelativeScale3D(PouchSeal->GetRelativeScale3D()*NewPouch.GetScale3D().GetAbsMax()/FMath::Max(.001,OldPouch.GetScale3D().GetAbsMax()));
   }
   if(WorkPayload->IsVisible())BoundedCommittedPreview(this,WorkPayload,12,Drop*.625f-6*(1-Deposit),8*(1-Deposit));
   WorkGrip[0]=FVector(49,-10,-4-Drop*.625f);WorkGrip[1]=FVector(49,10,-4-Drop*.625f);WorkGripActive=true;
  }
 }
 if(G->ApolloPackedPartRemaining>0){
  const float Move=ApolloWorkMinimumJerk((2.2f-G->ApolloPackedPartRemaining)/.7f),Drop=ApolloWorkMinimumJerk((1-G->ApolloPackedPartRemaining)/1.2f)*45;
  // A new tool may update WorkSource before this old committed preview ends.
  // Resolve the recorded part, rather than substituting the new work target.
  auto* Packed=G->ApolloPartActors.IsValidIndex(G->ApolloPackedPart)?SourceMesh(G->ApolloPartActors[G->ApolloPackedPart]):nullptr;
  if(!Packed||!Packed->GetStaticMesh()){WorkGripActive=false;return;}
  Payload(FMath::Lerp(Packed->GetComponentLocation(),View.TransformPosition(FVector(105,10,-18-Drop)),Move),false,Packed);
  // A committed viewmodel uses the authored part orientation, independent of
  // world/camera yaw; the source actor and work camera remain untouched.
  WorkPayload->SetRelativeRotation(Packed->GetRelativeRotation());
  BoundedCommittedPreview(this,WorkPayload,25,Drop*.45f,4*(1-Move),FMath::Lerp(58.f,55.f,Move));
  WorkGrip[0]=FVector(49,-10,-4-Drop*.45f);WorkGrip[1]=FVector(49,10,-4-Drop*.45f);WorkGripActive=true;return;
 }
 if(Packing&&Sample==2){AssignToolMesh(WorkTool,TEXT("SM_DriveTube"));const float Lower=ApolloWorkMinimumJerk((1-G->ApolloPackedPreviewRemaining)/1.2f)*20;WorkTool->SetRelativeLocation(FVector(51,0,-15-Lower));WorkTool->SetRelativeRotation(FRotator(0,0,12));WorkTool->SetRelativeScale3D(FVector::OneVector);BoundedCommittedPreview(this,WorkTool,23,Lower);WorkGrip[0]=FVector(49,-5,-4-Lower);WorkGrip[1]=FVector(49,5,-4-Lower);WorkGripActive=true;return;}
 if(Tool==1){AssignToolMesh(WorkTool,TEXT("SM_ContextCamera"));WorkTool->SetRelativeLocation(FVector(45,0,-15));WorkTool->SetRelativeRotation(FRotator::ZeroRotator);WorkTool->SetRelativeScale3D(FVector::OneVector);WorkGrip[0]=FVector(33,-8,-16);WorkGrip[1]=FVector(33,8,-16);WorkGripActive=true;return;}
 if(Tool!=4&&Tool!=5&&Tool!=6)return;
 FVector Contact=G->ApolloContactPoint(),Tip;
 if(Tool==4){
  Contact=WorkContact;
  const FVector Carry=View.TransformPosition(FVector(90,18,-8));
  Tip=Phase==0?Contact:Sample==1?LunarPovMotion::ScoopLift(Contact,Carry,RawMotion):FMath::Lerp(Contact,Carry,T);
  if(Phase>=1&&G->ApolloToolCooldown<=0)Tip=Carry;
  if(Phase>=1)Payload(Tip+FVector(0,0,1),Sample==1);
 }else if(Tool==6){Tip=Contact;}
 else{
  const FVector Soil=WorkContact;
  if(Phase>=1&&Phase<6&&!(Phase==1&&G->ApolloToolCooldown>0)){AssignToolMesh(DrivenTube,TEXT("SM_DriveTube"));DrivenTube->SetWorldLocation(Soil+FVector(0,0,19));DrivenTube->SetWorldRotation(FRotator::ZeroRotator);DrivenTube->SetWorldScale3D(FVector::OneVector);}
  const bool Hammer=Phase>=1&&Phase<=5&&!(Phase==1&&G->ApolloToolCooldown>0);
  if(Hammer){
   AssignToolMesh(WorkTool,TEXT("SM_GeologyHammer"));float Lift=18;
   const float Stroke=Phase>=2&&G->ApolloToolCooldown>0?FMath::Sin(T*PI):0.f;
   if(Phase>=2&&G->ApolloToolCooldown>0)Lift=20*(1-Stroke);
   // A small wrist-led swing brings the authored head onto the tube and back;
   // this is a visual approximation, not a force or drilling simulation.
   WorkTool->SetWorldLocation(Soil+FVector(0,0,28+Lift));WorkTool->SetWorldRotation(FRotator(FMath::Lerp(18.f,-6.f,Stroke),0,0));WorkTool->SetWorldScale3D(FVector::OneVector);
   WorkGrip[0]=Local(Soil+FVector(-9,-5,19));WorkGrip[1]=Local(WorkTool->GetComponentLocation()+WorkTool->GetComponentQuat().RotateVector(FVector(-7,4,-8)));WorkGripActive=true;return;
  }
  AssignToolMesh(WorkTool,TEXT("SM_DriveTube"));
  const FVector Carry=View.TransformPosition(FVector(48,0,-7));
  const FVector Center=Phase==0?Carry:Phase==1?FMath::Lerp(Carry,Soil+FVector(0,0,19),T):FMath::Lerp(Soil+FVector(0,0,19),Carry,T);
  WorkTool->SetWorldLocation(Center);WorkTool->SetWorldRotation(FRotator::ZeroRotator);WorkTool->SetWorldScale3D(FVector::OneVector);
  WorkGrip[0]=Local(Center+FVector(-8,-5,0));WorkGrip[1]=Local(Center+FVector(-8,5,0));WorkGripActive=true;return;
 }
 const bool Tongs=Tool==4&&Sample==0,Cutters=Tool==6;
 const FVector End=Tongs?FVector(22,0,-4):Cutters?FVector(11,0,0):FVector(14,0,-2);
 // A soil scoop digs, curls and lifts rather than translating as a rigid rod.
 // The handle grip is derived from this same frame, so the palm stays attached.
 const bool SoilScoop=Tool==4&&Sample==1;
 const float Dig=SoilScoop&&Phase>=1?FMath::Sin(RawMotion*PI):0;
 const FRotator ToolPose=SoilScoop?FRotator(Phase==0?48.f:FMath::Lerp(48.f,18.f,T),-25+Dig*10,Dig*12):FRotator(35,-25,0);
 const FQuat Rotation=View.GetRotation()*ToolPose.Quaternion();const FVector Base=Tip-Rotation.RotateVector(End);
 AssignToolMesh(WorkTool,Tongs?TEXT("SM_TongsBase"):Cutters?TEXT("SM_CuttersBase"):TEXT("SM_SoilScoop"));WorkTool->SetWorldLocation(Base);WorkTool->SetWorldRotation(Rotation);WorkTool->SetWorldScale3D(FVector::OneVector);
 if(SoilScoop&&WorkPayload->IsVisible())WorkPayload->SetWorldRotation(Rotation);
 if(Tongs||Cutters){
  const float Close=Tongs?(Phase>=1?T:0):G->ApolloToolCooldown>0?FMath::Sin(T*PI):0;
  for(int I=0;I<2;++I){AssignToolMesh(ToolJaws[I],Tongs?(I?TEXT("SM_TongsJawR"):TEXT("SM_TongsJawL")):(I?TEXT("SM_CuttersJawR"):TEXT("SM_CuttersJawL")));
   ToolJaws[I]->SetWorldLocation(Base+Rotation.RotateVector(FVector(Tongs?10.f:5.f,0,0)));ToolJaws[I]->SetWorldRotation(Rotation*FRotator(0,(I?-1.f:1.f)*Close*6,0).Quaternion());ToolJaws[I]->SetWorldScale3D(FVector::OneVector);
  }
 }
 WorkGrip[0]=Local(Base+Rotation.RotateVector(FVector(-14,-4,-1)));WorkGrip[1]=Local(Base+Rotation.RotateVector(FVector(-14,4,-1)));
 if(Tool==4)WorkGrip[0]=Phase>=1?FVector(75,-35,-29):FVector(50,-30,-32);
 if(Tool==6)WorkGrip[0]=Local(Contact)+FVector(-8,-18,-15);
 WorkGripActive=true;
}
