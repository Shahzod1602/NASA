#include "LunarGame.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UnrealClient.h"

namespace {
// Visual minimum-jerk interpolation: zero speed and acceleration at both ends.
// Tool input, commit points and cooldown durations remain owned by gameplay.
float ApolloWorkMinimumJerk(float T){if(!FMath::IsFinite(T))return 0;T=FMath::Clamp(T,0.f,1.f);return T*T*T*(T*(T*6-15)+10);}
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
 if(!G||!G->Apollo)return;
 PrepareApolloWorkAssets();if(!WorkTool)return;
 const bool Show=!ThirdPerson&&!ActivePanel&&PhotoCaptureFrames<=0&&!(FScreenshotRequest::IsScreenshotRequested()&&!FScreenshotRequest::ShouldShowUI())&&G->IsActive();
 WorkTool->SetVisibility(false);SamplePouch->SetVisibility(false);WorkPayload->SetVisibility(false);DrivenTube->SetVisibility(false);PouchSeal->SetVisibility(false);PouchLabel->SetVisibility(false);for(const auto& Jaw:ToolJaws)Jaw->SetVisibility(false);
 if(!Show)return;
 const int Tool=G->ApolloTool,Phase=G->ApolloToolPhase;
 const float T=ApolloWorkMinimumJerk(1-G->ApolloToolCooldown/.75f);
 const FTransform View=Camera->GetComponentTransform();
 auto Local=[&](FVector P){return View.InverseTransformPosition(P);};
 auto AssignToolMesh=[&](UStaticMeshComponent* C,const TCHAR* Name){C->SetStaticMesh(ApolloToolMeshes.FindRef(FName(Name)));C->SetVisibility(C->GetStaticMesh()!=nullptr);};
 auto Payload=[&](FVector P,bool Soil){
  if(Soil){WorkPayload->SetStaticMesh(ApolloToolMeshes.FindRef(TEXT("PayloadSphere")));WorkPayload->SetMaterial(0,WorkSoilMaterial);WorkPayload->SetRelativeScale3D(FVector(.08,.08,.025));}
  else if(auto* Source=WorkSourceMesh.Get()){WorkPayload->SetStaticMesh(Source->GetStaticMesh());for(int I=0;I<Source->GetNumMaterials();++I)WorkPayload->SetMaterial(I,Source->GetMaterial(I));WorkPayload->SetWorldScale3D(WorkSourceTransform.GetScale3D());}
  WorkPayload->SetWorldLocation(P);WorkPayload->SetWorldRotation(WorkSourceTransform.Rotator());WorkPayload->SetVisibility(WorkPayload->GetStaticMesh()!=nullptr);
 };
 // The soft pouch stays open until the final seal input. The committed preview
 // deposits the sample, closes the lip, then lowers it without blocking walking.
 const bool Packing=G->ApolloPackedPreviewRemaining>0;
 const int Sample=Packing?G->ApolloPackedSample:G->SelectedSample;
 if((Tool==4&&Phase>=1)||(Packing&&Sample<2)){
  AssignToolMesh(SamplePouch,TEXT("SM_SamplePouch"));const float Drop=Packing?ApolloWorkMinimumJerk((1-G->ApolloPackedPreviewRemaining)/1.2f)*32:0;
  SamplePouch->SetRelativeLocation(FVector(80,-28,-24-Drop));SamplePouch->SetRelativeRotation(FRotator(0,-8,4));SamplePouch->SetRelativeScale3D(FVector::OneVector);
  const FString Caption=Sample==1?TEXT("12032"):TEXT("12031");if(PouchCaption!=Caption){PouchCaption=Caption;PouchLabel->SetText(FText::FromString(Caption));}PouchLabel->SetRelativeLocation(FVector(77.7f,-28,-25-Drop));PouchLabel->SetRelativeRotation(FRotator(0,180,0));PouchLabel->SetVisibility(true);
  if(Packing){
   const float Deposit=ApolloWorkMinimumJerk((2.2f-G->ApolloPackedPreviewRemaining)/.45f);
   if(Deposit<1)Payload(View.TransformPosition((FMath::Lerp(FVector(90,18,-8),FVector(80,-28,-23),Deposit)-FVector(0,0,Drop))),Sample==1);
   PouchSeal->SetRelativeLocation(FVector(80,-28,-15-Drop));PouchSeal->SetRelativeScale3D(FVector(.04*Deposit,.13,.009));PouchSeal->SetRelativeRotation(FRotator(0,-8,4));PouchSeal->SetVisibility(Deposit>.1f);
   WorkGrip[0]=FVector(58,-27,-25-Drop);WorkGrip[1]=FVector(60,-10,-24-Drop);WorkGripActive=true;
  }
 }
 if(G->ApolloPackedPartRemaining>0){
  const float Move=ApolloWorkMinimumJerk((2.2f-G->ApolloPackedPartRemaining)/.7f),Drop=ApolloWorkMinimumJerk((1-G->ApolloPackedPartRemaining)/1.2f)*45;
  Payload(FMath::Lerp(WorkSourceTransform.GetLocation(),View.TransformPosition(FVector(105,10,-18-Drop)),Move),false);
  WorkGrip[0]=FVector(65,-6,-29-Drop);WorkGrip[1]=FVector(65,18,-29-Drop);WorkGripActive=true;return;
 }
 if(Packing&&Sample==2){AssignToolMesh(WorkTool,TEXT("SM_DriveTube"));WorkTool->SetRelativeLocation(FVector(51,0,-15-ApolloWorkMinimumJerk((1-G->ApolloPackedPreviewRemaining)/1.2f)*35));WorkTool->SetRelativeRotation(FRotator(0,0,12));WorkTool->SetRelativeScale3D(FVector::OneVector);WorkGrip[0]=WorkTool->GetRelativeLocation()+FVector(-7,-5,0);WorkGrip[1]=WorkTool->GetRelativeLocation()+FVector(-7,5,0);WorkGripActive=true;return;}
 if(Tool==1){AssignToolMesh(WorkTool,TEXT("SM_ContextCamera"));WorkTool->SetRelativeLocation(FVector(45,0,-15));WorkTool->SetRelativeRotation(FRotator::ZeroRotator);WorkTool->SetRelativeScale3D(FVector::OneVector);WorkGrip[0]=FVector(33,-8,-16);WorkGrip[1]=FVector(33,8,-16);WorkGripActive=true;return;}
 if(Tool!=4&&Tool!=5&&Tool!=6)return;
 FVector Contact=G->ApolloContactPoint(),Tip;
 if(Tool==4){
  Contact=WorkContact;
  Tip=Phase==0?Contact:FMath::Lerp(Contact,View.TransformPosition(FVector(90,18,-8)),T);
  if(Phase>=1&&G->ApolloToolCooldown<=0)Tip=View.TransformPosition(FVector(90,18,-8));
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
 const FQuat Rotation=View.GetRotation()*FRotator(35,-25,0).Quaternion();const FVector Base=Tip-Rotation.RotateVector(End);
 AssignToolMesh(WorkTool,Tongs?TEXT("SM_TongsBase"):Cutters?TEXT("SM_CuttersBase"):TEXT("SM_SoilScoop"));WorkTool->SetWorldLocation(Base);WorkTool->SetWorldRotation(Rotation);WorkTool->SetWorldScale3D(FVector::OneVector);
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
