#include "LunarSurfaceVehicle.h"
#include "LunarGame.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Math/RotationMatrix.h"

// Authored seated pose on the existing controller skeleton. The possessed pawn
// stays hidden for safe mount/dismount; this bounded visual has no collision/Tick.
void ALunarSurfaceVehicle::PoseDriver(){
 if(!IsOccupied()||!Rider)return;Rider->PoseSuitRig();if(Rider->SuitRig)Rider->SuitRig->bCastHiddenShadow=false;
 if(!Driver){
  if(!Rider->SuitRig||!Rider->SuitRig->GetSkinnedAsset())return;
  Driver=NewObject<UPoseableMeshComponent>(this,TEXT("SeatedExpeditionDriver"));Driver->SetupAttachment(Hull);
  Driver->SetSkeletalMesh(Cast<USkeletalMesh>(Rider->SuitRig->GetSkinnedAsset()));
  for(int32 I=0;I<Rider->SuitRig->GetNumMaterials();++I)Driver->SetMaterial(I,Rider->SuitRig->GetMaterial(I));
  Driver->SetCollisionEnabled(ECollisionEnabled::NoCollision);Driver->SetCanEverAffectNavigation(false);
  Driver->PrimaryComponentTick.bCanEverTick=false;Driver->SetComponentTickEnabled(false);Driver->RegisterComponent();
 }
 Driver->SetRelativeRotation(Rider->SuitCoordinate.Inverse());Driver->SetVisibility(true);
 // The Moon museum model has two seats. Mars retains its authored central seat.
 const FVector Shift=MuseumRover?FVector::ZeroVector:FVector(22,31,65);
 const float Y=-31;
 auto Set=[&](const FString& Name,FVector P,FQuat R=FQuat::Identity){
  if(const FQuat* Bind=Rider->SuitBindRotations.Find(FName(*Name)))Driver->SetBoneTransformByName(FName(*Name),FTransform(Rider->SuitCoordinate*R*Rider->SuitCoordinate.Inverse()*(*Bind),Rider->SuitCoordinate.RotateVector(P+Shift)),EBoneSpaces::ComponentSpace);
 };
 Set(TEXT("root"),FVector::ZeroVector);Set(TEXT("pelvis"),FVector(-44,Y,-26));Set(TEXT("chest"),FVector(-39,Y,16));Set(TEXT("head"),FVector(-33,Y,50));
 const FQuat RestHand=FRotationMatrix::MakeFromXY(FVector(0,0,-1),FVector(0,1,0)).ToQuat();
 for(int32 S=0;S<2;++S){
  const float Sign=S?1.f:-1.f;const FString Side=S?TEXT("r"):TEXT("l");
  const FVector Hip(-44,Y+Sign*12,-26),Knee(-7,Y+Sign*13,-33),Foot(7,Y+Sign*13,-60);
  Set(TEXT("thigh_")+Side,Hip,FQuat::FindBetweenNormals(FVector(4,0,-35).GetSafeNormal(),(Knee-Hip).GetSafeNormal()));
  Set(TEXT("calf_")+Side,Knee,FQuat::FindBetweenNormals(FVector(-1,0,-31).GetSafeNormal(),(Foot-Knee).GetSafeNormal()));Set(TEXT("foot_")+Side,Foot);
  const FVector Shoulder(-39,Y+Sign*22,20),Elbow(-27,Y+Sign*29,-4),Wrist(-9,Y+Sign*12,8);
  Set(TEXT("upperarm_")+Side,Shoulder,FQuat::FindBetweenNormals(FVector(3,Sign*9,-26).GetSafeNormal(),(Elbow-Shoulder).GetSafeNormal()));
  Set(TEXT("lowerarm_")+Side,Elbow,FQuat::FindBetweenNormals(FVector(3,Sign*4,-23).GetSafeNormal(),(Wrist-Elbow).GetSafeNormal()));
  const FQuat Palm=FRotator(-8,-Sign*15,Sign*8).Quaternion();Set(TEXT("hand_")+Side,Wrist,Palm*RestHand.Inverse());
  for(int32 Digit=0;Digit<5;++Digit){
   const FVector P=Wrist+Palm.RotateVector(FVector(4.5f,Sign*(-2.5f+Digit*1.25f),0));
   const FQuat Curl=Palm*FRotator(Digit==4?-20.f:-45.f,0,0).Quaternion();
   Set(FString::Printf(TEXT("finger%d_"),Digit)+Side,P,Palm*RestHand.Inverse());
   Set(FString::Printf(TEXT("tip%d_"),Digit)+Side,P+Palm.RotateVector(FVector(3.5,0,0)),Curl*RestHand.Inverse());
  }
 }
 // Poseable meshes require an explicit component-space scale after posing;
 // generic HideBone does not remove this manually refreshed helmet.
 if(CockpitView)Driver->SetBoneScaleByName(TEXT("head"),FVector::ZeroVector,EBoneSpaces::ComponentSpace);
 Driver->RefreshBoneTransforms();
}
