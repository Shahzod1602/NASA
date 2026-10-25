#include "LunarGame.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"

void ALunarCharacter::PoseSuitRig(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));if(!G)return;
 if(!SuitRig){
  auto Make=[&](const TCHAR* Path,USceneComponent* Parent){auto* C=NewObject<UPoseableMeshComponent>(this);C->SetupAttachment(Parent);C->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,Path));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetOnlyOwnerSee(true);C->RegisterComponent();return C;};
  SuitRig=Make(TEXT("/Game/Lunar/Suit/Rig/SK_A7L/SkeletalMeshes/SK_A7L.SK_A7L"),GetCapsuleComponent());
  HandRig=Make(TEXT("/Game/Lunar/Suit/Rig/SK_A7L_Hands/SkeletalMeshes/SK_A7L_Hands.SK_A7L_Hands"),Camera);HandRig->SetCastShadow(false);
  if(auto* RigAsset=Cast<USkeletalMesh>(SuitRig->GetSkinnedAsset())){
   const auto& Ref=RigAsset->GetRefSkeleton();TArray<FTransform> World;World.SetNum(Ref.GetNum());
   for(int I=0;I<Ref.GetNum();++I){World[I]=Ref.GetRefBonePose()[I];const int Parent=Ref.GetParentIndex(I);if(Parent>=0)World[I]*=World[Parent];}
   for(int I=0;I<Ref.GetNum();++I)SuitBindRotations.Add(Ref.GetBoneName(I),World[I].GetRotation());
   auto Pos=[&](const TCHAR* N){const int I=Ref.FindBoneIndex(N);return I>=0?World[I].GetLocation():FVector::ZeroVector;};
   const FVector Right=(Pos(TEXT("thigh_r"))-Pos(TEXT("thigh_l"))).GetSafeNormal(),Up=(Pos(TEXT("head"))-Pos(TEXT("root"))).GetSafeNormal();
   SuitCoordinate=FRotationMatrix::MakeFromXY(FVector::CrossProduct(Right,Up),Right).ToQuat();
   UE_LOG(LogTemp,Display,TEXT("SUIT_RIG %d bones; reference head %s right %s hand_r %s"),Ref.GetNum(),*Pos(TEXT("head")).ToString(),*Right.ToString(),*Pos(TEXT("hand_r")).ToString());
  }
  SuitRig->SetRelativeRotation(SuitCoordinate.Inverse());HandRig->SetRelativeRotation(SuitCoordinate.Inverse());
 }
 const bool Show=!G->Menu&&(G->IsActive()||G->Stage==ELunarStage::Briefing||G->Stage==ELunarStage::Won);
 const bool Capture=PhotoCaptureFrames>0||(FScreenshotRequest::IsScreenshotRequested()&&!FScreenshotRequest::ShouldShowUI());
 SuitRig->SetVisibility(Show&&ThirdPerson);HandRig->SetVisibility(Show&&!ThirdPerson&&Camera->FieldOfView>60&&!Capture);
 if(PhotoCaptureFrames>0)--PhotoCaptureFrames;
 if(!SuitRig->GetSkinnedAsset()||!HandRig->GetSkinnedAsset())return;
 for(const auto& C:SuitArms){C->SetVisibility(false);C->SetCastShadow(false);}for(int I=0;I<Gloves.Num();++I)Gloves[I]->SetVisibility((I==0||I==8)&&Show&&!ThirdPerson&&Camera->FieldOfView>60&&!Capture);
 for(const auto& C:SuitBody)C->SetCastShadow(false);SuitRig->bCastHiddenShadow=true;
 if(ThirdPerson)for(const auto& C:SuitBody)C->SetVisibility(false,true);
 auto Set=[&](UPoseableMeshComponent* C,const FString& Name,FVector P,FQuat R=FQuat::Identity){
  const FQuat* Bind=SuitBindRotations.Find(FName(*Name));if(!Bind)return;
  C->SetBoneTransformByName(FName(*Name),FTransform(SuitCoordinate*R*SuitCoordinate.Inverse()*(*Bind),SuitCoordinate.RotateVector(P)),EBoneSpaces::ComponentSpace);
 };
 const FQuat RestHand=FRotationMatrix::MakeFromXY(FVector(0,0,-1),FVector(0,1,0)).ToQuat();
 auto Arms=[&](UPoseableMeshComponent* C,bool View){
  for(int S=0;S<2;++S){
   const FString Side=S?TEXT("r"):TEXT("l");const float Sign=S?1:-1;
   const auto& Parts=View?Gloves:SuitArms;const int B=S*(View?8:9),F=B+(View?0:1);
   const FVector Shoulder(0,Sign*25,40);const FVector Elbow=Parts[F]->GetRelativeLocation(),Wrist=Parts[F+1]->GetRelativeLocation();
   const FVector RefUpper=FVector(3,Sign*9,-26).GetSafeNormal(),RefLower=FVector(3,Sign*4,-23).GetSafeNormal();
   Set(C,TEXT("upperarm_")+Side,Shoulder,FQuat::FindBetweenNormals(RefUpper,(Elbow-Shoulder).GetSafeNormal()));
   Set(C,TEXT("lowerarm_")+Side,Elbow,FQuat::FindBetweenNormals(RefLower,(Wrist-Elbow).GetSafeNormal()));
   Set(C,TEXT("hand_")+Side,Wrist,Parts[F+2]->GetRelativeRotation().Quaternion()*RestHand.Inverse());
   for(int Digit=0;Digit<5;++Digit){
    auto* FingerMesh=Parts[F+3+Digit].Get();const FQuat Rotation=FingerMesh->GetRelativeRotation().Quaternion();
    Set(C,FString::Printf(TEXT("finger%d_"),Digit)+Side,FingerMesh->GetRelativeLocation(),Rotation*RestHand.Inverse());
    const FQuat Curl=FRotator(-WorkBlend*38,0,0).Quaternion();
    Set(C,FString::Printf(TEXT("tip%d_"),Digit)+Side,FingerMesh->GetRelativeLocation()+Rotation.RotateVector(FVector(4,0,0)),Rotation*Curl*RestHand.Inverse());
   }
  }
 };
 if(Show){
  Set(SuitRig,TEXT("root"),FVector::ZeroVector);Set(SuitRig,TEXT("pelvis"),SuitBody[1]->GetRelativeLocation()+FVector(0,0,-12));
  Set(SuitRig,TEXT("chest"),SuitBody[0]->GetRelativeLocation()+FVector(0,0,30));Set(SuitRig,TEXT("head"),SuitBody[2]->GetRelativeLocation());
  for(int S=0;S<2;++S){const int B=4+S*3;const FString Side=S?TEXT("r"):TEXT("l");const FVector Hip=SuitBody[B]->GetRelativeLocation(),Knee=SuitBody[B+1]->GetRelativeLocation(),Foot=SuitBody[B+2]->GetRelativeLocation();
   Set(SuitRig,TEXT("thigh_")+Side,Hip,FQuat::FindBetweenNormals(FVector(4,0,-35).GetSafeNormal(),(Knee-Hip).GetSafeNormal()));
   Set(SuitRig,TEXT("calf_")+Side,Knee,FQuat::FindBetweenNormals(FVector(-1,0,-31).GetSafeNormal(),(Foot-Knee).GetSafeNormal()));
   Set(SuitRig,TEXT("foot_")+Side,Foot,SuitBody[B+2]->GetRelativeRotation().Quaternion());
  }
  Arms(SuitRig,false);SuitRig->RefreshBoneTransforms();
 }
 if(!ThirdPerson&&Show){Set(HandRig,TEXT("root"),FVector::ZeroVector);Set(HandRig,TEXT("pelvis"),FVector(0,0,-12));Set(HandRig,TEXT("chest"),FVector(0,0,30));Arms(HandRig,true);HandRig->RefreshBoneTransforms();}
}
