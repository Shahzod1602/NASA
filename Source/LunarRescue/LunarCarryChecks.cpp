#include "LunarCarryPresentation.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "PhysicsEngine/BodySetup.h"
#include "Math/RotationMatrix.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include <limits>

namespace LunarCarryCheckPrivate {
bool IndependentFrustum(const FBox& B,const FTransform& T,float Fov,float Aspect){
 const double H=FMath::Tan(FMath::DegreesToRadians(double(Fov)*.5)),V=H/Aspect;
 for(int32 I=0;I<8;++I){const FVector P=T.TransformPosition(FVector((I&1)?B.Max.X:B.Min.X,(I&2)?B.Max.Y:B.Min.Y,(I&4)?B.Max.Z:B.Min.Z));
  if(P.ContainsNaN()||P.X<=10||FMath::Abs(P.Y)>.90*P.X*H||FMath::Abs(P.Z)>.90*P.X*V)return false;
 }
 return true;
}
bool ProjectEnvelope(ALunarCharacter* P,UStaticMeshComponent* C,float Aspect,FBox2D& Screen){
 Screen=FBox2D(ForceInit);if(!P||!P->Camera||!C||!C->GetStaticMesh())return false;
 const double H=FMath::Tan(FMath::DegreesToRadians(double(P->Camera->FieldOfView)*.5)),V=H/Aspect;
 const FBox B=C->GetStaticMesh()->GetBoundingBox();const FTransform View=P->Camera->GetComponentTransform();
 for(int32 I=0;I<8;++I){const FVector World=C->GetComponentTransform().TransformPosition(FVector((I&1)?B.Max.X:B.Min.X,(I&2)?B.Max.Y:B.Min.Y,(I&4)?B.Max.Z:B.Min.Z));
  const FVector At=View.InverseTransformPosition(World);if(At.ContainsNaN()||At.X<=10)return false;
  Screen+=FVector2D(.5+.5*At.Y/(At.X*H),.5-.5*At.Z/(At.X*V));
 }
 return Screen.bIsValid;
}
void LogRigPose(ALunarCharacter* P,bool FirstPerson,const TCHAR* Context){
 auto* Rig=P?(FirstPerson?P->HandRig.Get():P->SuitRig.Get()):nullptr;if(!Rig||!P->Camera)return;const FTransform View=P->Camera->GetComponentTransform();
 for(const TCHAR* Side:{TEXT("l"),TEXT("r")}){
  const FVector Shoulder=View.InverseTransformPosition(Rig->GetBoneLocationByName(FName(*(FString(TEXT("upperarm_"))+Side)),EBoneSpaces::WorldSpace));
  const FVector Elbow=View.InverseTransformPosition(Rig->GetBoneLocationByName(FName(*(FString(TEXT("lowerarm_"))+Side)),EBoneSpaces::WorldSpace));
  const FVector Wrist=View.InverseTransformPosition(Rig->GetBoneLocationByName(FName(*(FString(TEXT("hand_"))+Side)),EBoneSpaces::WorldSpace));
  UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG pose context=%s view=%d side=%s fov=%.3f pitch=%.3f camera_shoulder=%s camera_elbow=%s camera_wrist=%s"),Context,FirstPerson,Side,P->Camera->FieldOfView,P->Camera->GetComponentRotation().Pitch,*Shoulder.ToString(),*Elbow.ToString(),*Wrist.ToString());
 }
}
bool ProtectedFaceClear(ALunarCharacter* P,float Aspect);
bool RenderedWristOnGrip(ALunarCharacter* P,bool FirstPerson){
 if(!P||!P->Camera){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG wrist missing_player_or_camera view=%d"),FirstPerson);return false;}auto* Rig=FirstPerson?P->HandRig.Get():P->SuitRig.Get();if(!Rig||!Rig->IsVisible()){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG wrist unavailable_rig view=%d rig=%d visible=%d"),FirstPerson,Rig!=nullptr,Rig&&Rig->IsVisible());return false;}
 const FTransform Space=FirstPerson?P->Camera->GetComponentTransform():P->GetActorTransform();bool Good=true;
 for(int32 Index=0;Index<2;++Index){FVector Wrist=FVector::ZeroVector;FRotator Rotation=FRotator::ZeroRotator;const bool Grip=LunarCarryPresentation::GripPose(P,FirstPerson,Index?1.f:-1.f,Wrist,Rotation);
  const FName Bone(Index?TEXT("hand_r"):TEXT("hand_l"));const bool Found=Rig->GetBoneIndex(Bone)!=INDEX_NONE;const FVector Actual=Found?Rig->GetBoneLocationByName(Bone,EBoneSpaces::WorldSpace):FVector::ZeroVector,Expected=Space.TransformPosition(Wrist);const double Error=FVector::Dist(Actual,Expected);
  if(!Grip||!Found||Error>.3){Good=false;UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG wrist view=%d side=%d grip=%d bone_found=%d error_cm=%.6f limit=.3 fov=%.3f pitch=%.3f local_target=%s world_target=%s actual=%s"),FirstPerson,Index,Grip,Found,Error,P->Camera->FieldOfView,P->Camera->GetComponentRotation().Pitch,*Wrist.ToString(),*Expected.ToString(),*Actual.ToString());}
 }return Good;
}
bool RenderedArmReach(ALunarCharacter* P,bool FirstPerson){
 auto* Rig=P?(FirstPerson?P->HandRig.Get():P->SuitRig.Get()):nullptr;if(!Rig||!Rig->IsVisible()){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG arm unavailable_rig view=%d rig=%d visible=%d"),FirstPerson,Rig!=nullptr,Rig&&Rig->IsVisible());return false;}bool Good=true;
 for(const TCHAR* Side:{TEXT("l"),TEXT("r")}){
  const FName Shoulder(*(FString(TEXT("upperarm_"))+Side)),Elbow(*(FString(TEXT("lowerarm_"))+Side)),Hand(*(FString(TEXT("hand_"))+Side));
  if(Rig->GetBoneIndex(Shoulder)==INDEX_NONE||Rig->GetBoneIndex(Elbow)==INDEX_NONE||Rig->GetBoneIndex(Hand)==INDEX_NONE){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG arm missing_bone view=%d side=%s shoulder=%d elbow=%d hand=%d"),FirstPerson,Side,Rig->GetBoneIndex(Shoulder),Rig->GetBoneIndex(Elbow),Rig->GetBoneIndex(Hand));Good=false;continue;}
  const FVector A=Rig->GetBoneLocationByName(Shoulder,EBoneSpaces::WorldSpace),B=Rig->GetBoneLocationByName(Elbow,EBoneSpaces::WorldSpace),C=Rig->GetBoneLocationByName(Hand,EBoneSpaces::WorldSpace);const double Upper=FVector::Dist(A,B),Lower=FVector::Dist(B,C),Reach=FVector::Dist(A,C);
  if(A.ContainsNaN()||B.ContainsNaN()||C.ContainsNaN()||!FMath::IsNearlyEqual(Upper,28.,1.5)||!FMath::IsNearlyEqual(Lower,24.,1.5)||Reach>52.1){Good=false;UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG arm view=%d side=%s upper_cm=%.6f lower_cm=%.6f reach_cm=%.6f expected28/24 tol1.5 max52.1 fov=%.3f shoulder=%s elbow=%s hand=%s"),FirstPerson,Side,Upper,Lower,Reach,P->Camera->FieldOfView,*A.ToString(),*B.ToString(),*C.ToString());}
 }return Good;
}
bool BodyItemClear(ALunarCharacter* P){
 auto* Item=P?P->BodyCarryItem.Get():nullptr;if(!Item||!Item->IsVisible()||!Item->GetStaticMesh()){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG body_clear unavailable item=%d visible=%d mesh=%d"),Item!=nullptr,Item&&Item->IsVisible(),Item&&Item->GetStaticMesh()!=nullptr);return false;}
 const FBox B=Item->GetStaticMesh()->GetBoundingBox();FBox BodySpace(ForceInit);
 for(int32 I=0;I<8;++I){const FVector V((I&1)?B.Max.X:B.Min.X,(I&2)?B.Max.Y:B.Min.Y,(I&4)?B.Max.Z:B.Min.Z);BodySpace+=P->GetActorTransform().InverseTransformPosition(Item->GetComponentTransform().TransformPosition(V));}
 const bool Good=BodySpace.IsValid&&!BodySpace.Min.ContainsNaN()&&!BodySpace.Max.ContainsNaN()&&BodySpace.Min.X>=20;
 if(!Good)UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG body_clear mesh=%s valid=%d body_min=%s body_max=%s minX=%.9f required20 source_min=%s source_max=%s world_scale=%s"),*Item->GetStaticMesh()->GetName(),BodySpace.IsValid,*BodySpace.Min.ToString(),*BodySpace.Max.ToString(),BodySpace.Min.X,*B.Min.ToString(),*B.Max.ToString(),*Item->GetComponentScale().ToString());return Good;
}
// Query the actual cooked mesh, not a box-clamped point. An interior point
// has a negative surface gap and cannot silently count as perfect contact.
// This checks a sampled fingertip per opposed hand, not every glove triangle.
struct FCarrySurfaceProbe {
 UStaticMeshComponent* Mesh=nullptr;FBox Bounds=FBox(ForceInit);mutable int32 RayQueries=0,RawHits=0,IsolatedHits=0,FaceHits=0;
 FCarrySurfaceProbe(ALunarCharacter* P,UStaticMeshComponent* Item){
  if(!P||!Item||!Item->GetStaticMesh()||Item->GetComponentTransform().ContainsNaN())return;
  Bounds=Item->GetStaticMesh()->GetBoundingBox();if(!Bounds.IsValid||Bounds.Min.ContainsNaN()||Bounds.Max.ContainsNaN())return;
  Mesh=NewObject<UStaticMeshComponent>(P);Mesh->SetStaticMesh(Item->GetStaticMesh());Mesh->SetMobility(EComponentMobility::Movable);
  Mesh->SetCanEverAffectNavigation(false);Mesh->SetGenerateOverlapEvents(false);Mesh->SetCastShadow(false);Mesh->SetVisibility(false);Mesh->SetHiddenInGame(true);
  Mesh->SetCollisionProfileName(TEXT("BlockAll"));Mesh->SetCollisionObjectType(ECC_PhysicsBody);Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);Mesh->RegisterComponent();Mesh->SetWorldTransform(Item->GetComponentTransform());
 }
 ~FCarrySurfaceProbe(){if(Mesh)Mesh->DestroyComponent();}
 bool FrontDistance(const FVector& Start,const FVector& Toward,double& Distance)const{
  if(!Mesh||!Mesh->GetWorld()||Start.ContainsNaN()||Toward.ContainsNaN())return false;const FVector Direction=(Toward-Start).GetSafeNormal();if(Direction.IsNearlyZero())return false;
  FCollisionQueryParams Q;Q.bTraceComplex=true;Q.bReturnFaceIndex=true;TArray<FHitResult> Hits;
  Mesh->GetWorld()->LineTraceMultiByObjectType(Hits,Start,Start+Direction*2000,FCollisionObjectQueryParams(ECC_PhysicsBody),Q);Distance=1.e30;
  for(const auto& Hit:Hits)if(Hit.GetComponent()==Mesh&&Hit.FaceIndex!=INDEX_NONE&&!Hit.ImpactPoint.ContainsNaN())Distance=FMath::Min(Distance,FVector::Dist(Start,Hit.ImpactPoint));
  return FMath::IsFinite(Distance)&&Distance<1.e29;
 }

 bool SignedGap(const FVector& WorldPoint,double& Gap)const{
  if(!Mesh||WorldPoint.ContainsNaN())return false;const FTransform Frame=Mesh->GetComponentTransform();const FVector Local=Frame.InverseTransformPosition(WorldPoint);
  const double Radius=Bounds.GetSize().GetMax()+10;if(!FMath::IsFinite(Radius)||Radius<=0||Radius>100000||!Mesh->GetWorld())return false;double Nearest=1.e30;bool Found=false;
  auto Ray=[&](const FVector& From,const FVector& To){
   const FVector Start=Frame.TransformPosition(From),End=Frame.TransformPosition(To);const double Length=FVector::Dist(Start,End);
   if(Start.ContainsNaN()||End.ContainsNaN()||!FMath::IsFinite(Length)||Length<=UE_SMALL_NUMBER||Length>1000000)return;
   FCollisionQueryParams Q;Q.bTraceComplex=true;Q.bReturnFaceIndex=true;TArray<FHitResult> Hits;
   // The direct component/body trace API does not expose a face-index option
   // in its lower-level signature. Use the full world query parameters, then
   // accept only this isolated test component; other PhysicsBody hits cannot
   // substitute for a carry-surface measurement.
   ++RayQueries;Mesh->GetWorld()->LineTraceMultiByObjectType(Hits,Start,End,FCollisionObjectQueryParams(ECC_PhysicsBody),Q);RawHits+=Hits.Num();
   for(const FHitResult& Hit:Hits){
    if(Hit.GetComponent()==Mesh)++IsolatedHits;
    if(Hit.GetComponent()!=Mesh||Hit.FaceIndex==INDEX_NONE||Hit.ImpactPoint.ContainsNaN()||Hit.ImpactNormal.ContainsNaN())continue;++FaceHits;
    const double Distance=FVector::Dist(WorldPoint,Hit.ImpactPoint);if(Distance<Nearest){Nearest=Distance;Gap=FVector::DotProduct(WorldPoint-Hit.ImpactPoint,Hit.ImpactNormal)<0?-Distance:Distance;Found=true;}
   }
  };
  for(int32 Axis=0;Axis<3;++Axis){FVector From=Local,To=Local;From[Axis]=Bounds.Max[Axis]+Radius;To[Axis]=Bounds.Min[Axis]-Radius;Ray(From,To);Ray(To,From);}
  FVector Radial=Local-Bounds.GetCenter();Radial.Z=0;if(Radial.Normalize()){
   FVector A=Bounds.GetCenter()+Radial*Radius,B=Bounds.GetCenter()-Radial*Radius;A.Z=B.Z=FMath::Clamp(Local.Z,Bounds.Min.Z+.01,Bounds.Max.Z-.01);Ray(A,B);Ray(B,A);
  }return Found&&FMath::IsFinite(Gap);
 }
};
bool ProtectedFaceClear(ALunarCharacter* P,float Aspect){
 if(!P||!P->Camera||!P->HandRig||!P->HandRig->IsVisible()||P->Gloves.Num()!=16)return false;
 FBox2D Cell;const bool Projected=ProjectEnvelope(P,P->CarriedModule,Aspect,Cell);
 if(!Projected||Cell.GetCenter().Y>.51||Cell.Max.Y>.64){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG protected cell projected=%d fov=%.3f min=%s max=%s centreY=%.6f limitCentre=.51 limitBottom=.64"),Projected,P->Camera->FieldOfView,*Cell.Min.ToString(),*Cell.Max.ToString(),Cell.GetCenter().Y);return false;}
 const FVector2D Size=Cell.GetSize();const FBox2D Protected(Cell.Min+Size*FVector2D(.28,.20),Cell.Min+Size*FVector2D(.72,.72));
 FCarrySurfaceProbe Surface(P,P->CarriedModule);if(!Surface.Mesh)return false;
 const FTransform View=P->Camera->GetComponentTransform();const double H=FMath::Tan(FMath::DegreesToRadians(double(P->Camera->FieldOfView)*.5)),V=H/Aspect;int32 Samples=0;
 auto ClearPoint=[&](const FVector& World,const FString& Label){
  ++Samples;const FVector At=View.InverseTransformPosition(World);if(At.ContainsNaN()||At.X<=10)return false;
  const FVector2D UV(.5+.5*At.Y/(At.X*H),.5-.5*At.Z/(At.X*V));
  if(UV.X<Protected.Min.X||UV.X>Protected.Max.X||UV.Y<Protected.Min.Y||UV.Y>Protected.Max.Y)return true;
  double Front=0;if(!Surface.FrontDistance(View.GetLocation(),World,Front)){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG protected sample=%s unknown_cell_front uv=%s camera=%s"),*Label,*UV.ToString(),*At.ToString());return false;}
  // A hand point behind the actual near-facing cell triangles cannot cover its
  // readable face. This avoids projecting occluded fingers as foreground ink.
  const double Depth=FVector::Dist(View.GetLocation(),World);const bool Clear=Depth>=Front;
  if(!Clear)UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG protected visible_sample=%s fov=%.3f uv=%s camera=%s hand_depth=%.6f actual_cell_front=%.6f"),*Label,P->Camera->FieldOfView,*UV.ToString(),*At.ToString(),Depth,Front);
  return Clear;
 };
 // The authored SK_A7L_Hands source contains cuff, palm and blended digits;
 // it deliberately omits forearms. Hidden glove0/8 proxy AABBs are not visible
 // geometry. Sample the actual posed hand frame and every finger bone chain.
 // These are bounded geometry points, not full triangle/pixel certification.
 const FQuat RestHand=FRotationMatrix::MakeFromXY(FVector(0,0,-1),FVector(0,1,0)).ToQuat();
 for(int32 Side=0;Side<2;++Side){const FString Suffix=Side?TEXT("r"):TEXT("l");const FName HandBone(*(FString(TEXT("hand_"))+Suffix));
  if(P->HandRig->GetBoneIndex(HandBone)==INDEX_NONE)return false;const FTransform Bone=P->HandRig->GetBoneTransformByName(HandBone,EBoneSpaces::WorldSpace);const FTransform Frame(Bone.GetRotation()*RestHand,Bone.GetLocation());
  for(int32 Part:{1,2}){auto* Proxy=P->Gloves[Side*8+Part].Get();if(!Proxy||!Proxy->GetStaticMesh())return false;const FBox B=Proxy->GetStaticMesh()->GetBoundingBox();if(!B.IsValid)return false;
   if(!ClearPoint(Frame.TransformPosition(B.GetCenter()),FString::Printf(TEXT("%s_%d_centre"),*Suffix,Part)))return false;
   for(int32 Corner=0;Corner<8;++Corner){const FVector Point((Corner&1)?B.Max.X:B.Min.X,(Corner&2)?B.Max.Y:B.Min.Y,(Corner&4)?B.Max.Z:B.Min.Z);if(!ClearPoint(Frame.TransformPosition(Point),FString::Printf(TEXT("%s_%d_corner%d"),*Suffix,Part,Corner)))return false;}
  }
  for(int32 Finger=0;Finger<5;++Finger){const FName Root(*FString::Printf(TEXT("finger%d_%s"),Finger,*Suffix)),Tip(*FString::Printf(TEXT("tip%d_%s"),Finger,*Suffix));
   if(P->HandRig->GetBoneIndex(Root)==INDEX_NONE||P->HandRig->GetBoneIndex(Tip)==INDEX_NONE)return false;const FVector A=P->HandRig->GetBoneLocationByName(Root,EBoneSpaces::WorldSpace),B=P->HandRig->GetBoneLocationByName(Tip,EBoneSpaces::WorldSpace);
   for(int32 Step=0;Step<3;++Step)if(!ClearPoint(FMath::Lerp(A,B,Step*.5f),FString::Printf(TEXT("%s_digit%d_step%d"),*Suffix,Finger,Step)))return false;
  }
 }
 return Samples==66;
}
bool RenderedFingerContact(ALunarCharacter* P,UStaticMeshComponent* Item,bool FirstPerson,float MaximumGap=3.f){
 auto* Rig=P?(FirstPerson?P->HandRig.Get():P->SuitRig.Get()):nullptr;if(!Rig||!Item||!Item->GetStaticMesh()){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG contact unavailable view=%d rig=%d item=%d mesh=%d"),FirstPerson,Rig!=nullptr,Item!=nullptr,Item&&Item->GetStaticMesh()!=nullptr);return false;}
 const FString MeshPath=Item->GetStaticMesh()->GetPathName();
 const bool OpenCloth=MeshPath==TEXT("/Game/Lunar/Apollo12/Tools/SM_SamplePouch.SM_SamplePouch");
 const bool SignedGrip=MeshPath==TEXT("/Engine/BasicShapes/Cube.Cube")||MeshPath==TEXT("/Engine/BasicShapes/Cylinder.Cylinder")||MeshPath==TEXT("/Game/Lunar/Imported/PowerCell/PowerCell.PowerCell")||MeshPath==TEXT("/Game/Lunar/Apollo12/Tools/SM_DriveTube.SM_DriveTube");
 if(!OpenCloth&&!SignedGrip){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG contact unsupported_surface_domain path=%s"),*MeshPath);return false;}
 // Open cloth source is non-watertight with inward winding. Its normal sign
 // cannot identify solid penetration. Require unsigned real-triangle proximity
 // there; retain strict signed depth on the catalogued solid grip regions.
 // Composite source parts do not imply a globally certified closed volume.
 FCarrySurfaceProbe Surface(P,Item);if(!Surface.Mesh){UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG contact no_surface_probe view=%d mesh=%s bounds_valid=%d transform_invalid=%d"),FirstPerson,*Item->GetStaticMesh()->GetName(),Surface.Bounds.IsValid,Item->GetComponentTransform().ContainsNaN());return false;}bool Good=true;
 for(const TCHAR* Side:{TEXT("l"),TEXT("r")}){bool Contact=false,Penetrating=false;int32 Bones=0,Probed=0;double MinGap=1.e30,MaxGap=-1.e30,NearestAbs=1.e30;FString DeepestBone,NearestBone,Details;
  const int32 RaysBefore=Surface.RayQueries,HitsBefore=Surface.RawHits,IsolatedBefore=Surface.IsolatedHits,FacesBefore=Surface.FaceHits;
  for(int32 Finger=0;Finger<5;++Finger){const FName Bone(*FString::Printf(TEXT("tip%d_%s"),Finger,Side));if(Rig->GetBoneIndex(Bone)==INDEX_NONE){Details+=Bone.ToString()+TEXT(":missing ");continue;}++Bones;
   double Gap=0;const FVector Tip=Rig->GetBoneLocationByName(Bone,EBoneSpaces::WorldSpace);if(!Surface.SignedGap(Tip,Gap)){Details+=Bone.ToString()+TEXT(":unprobed ");continue;}++Probed;
   Details+=FString::Printf(TEXT("%s:%.6f@camera(%s) "),*Bone.ToString(),Gap,*P->Camera->GetComponentTransform().InverseTransformPosition(Tip).ToString());if(Gap<MinGap){MinGap=Gap;DeepestBone=Bone.ToString();}MaxGap=FMath::Max(MaxGap,Gap);if(FMath::Abs(Gap)<NearestAbs){NearestAbs=FMath::Abs(Gap);NearestBone=Bone.ToString();}
   Penetrating|=SignedGrip&&Gap< -1.5;Contact|=OpenCloth?FMath::Abs(Gap)<=MaximumGap:Gap<=MaximumGap;
  }
  if(Bones!=5||Probed!=5||!Contact||Penetrating){Good=false;UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG contact view=%d side=%s mesh=%s fov=%.3f bones=%d probed=%d contact=%d deep_penetration=%d min_gap_cm=%.6f max_gap_cm=%.6f nearest_abs_cm=%.6f nearest_bone=%s deepest_bone=%s max_gap=%.3f min_allowed=-1.5 rays=%d raw_hits=%d isolated_hits=%d valid_face_hits=%d gaps=[%s]"),FirstPerson,Side,*Item->GetStaticMesh()->GetName(),P->Camera->FieldOfView,Bones,Probed,Contact,Penetrating,MinGap,MaxGap,NearestAbs,*NearestBone,*DeepestBone,MaximumGap,Surface.RayQueries-RaysBefore,Surface.RawHits-HitsBefore,Surface.IsolatedHits-IsolatedBefore,Surface.FaceHits-FacesBefore,*Details);}
 }
 if(OpenCloth)UE_LOG(LogTemp,Display,TEXT("CARRY_DIAG surface_domain open_cloth_unsigned path=%s; no closed-volume penetration claim"),*MeshPath);
 if(!Good){const auto* Setup=Item->GetStaticMesh()->GetBodySetup();UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG surface mesh=%s setup=%d raw_trace_flag=%d simple_shapes=%d triangle_meshes=%d component_scale=%s total_rays=%d raw_hits=%d isolated_hits=%d valid_face_hits=%d"),*Item->GetStaticMesh()->GetName(),Setup!=nullptr,Setup?int32(Setup->CollisionTraceFlag):-1,Setup?Setup->AggGeom.GetElementCount():-1,Setup?Setup->TriMeshGeometries.Num():-1,*Item->GetComponentScale().ToString(),Surface.RayQueries,Surface.RawHits,Surface.IsolatedHits,Surface.FaceHits);LogRigPose(P,FirstPerson,TEXT("contact"));}
 return Good;
}
bool TipsNearSource(ALunarCharacter* P){return P&&RenderedFingerContact(P,P->CarriedModule,true);}
bool ActualViewport(ALunarCharacter* P,APlayerController* PC,UStaticMeshComponent* C,int32 Width,int32 Height){
 if(!P||!PC||!C||!C->GetStaticMesh()||Width<=0||Height<=0)return false;
 if(PC->PlayerCameraManager)PC->PlayerCameraManager->UpdateCamera(0);
 const FBox B=C->GetStaticMesh()->GetBoundingBox();
 for(int32 I=0;I<8;++I){FVector2D Pixel;const FVector World=C->GetComponentTransform().TransformPosition(FVector((I&1)?B.Max.X:B.Min.X,(I&2)?B.Max.Y:B.Min.Y,(I&4)?B.Max.Z:B.Min.Z));
  if(!UGameplayStatics::ProjectWorldToScreen(PC,World,Pixel,true)||Pixel.X<Width*.035||Pixel.Y<Height*.035||Pixel.X>Width*.965||Pixel.Y>Height*.965)return false;
 }
 return true;
}
}

void LunarCarryPresentation::ValidateChecks(ALunarGameMode* G,TFunctionRef<void(bool,const TCHAR*)> Check){
 const FBox Asymmetric(FVector(-32,-33,0),FVector(32,33,80));
 for(float Fov:{45.f,60.f,85.f,110.f})for(float Aspect:{4.f/3.f,16.f/9.f,21.f/9.f}){
  FTransform Fit;const bool Valid=FitViewBounds(Asymmetric,Fov,Aspect,Fit);
  Check(Valid&&LunarCarryCheckPrivate::IndependentFrustum(Asymmetric,Fit,Fov,Aspect),*FString::Printf(TEXT("Asymmetric carried bounds fit independent %.0f FOV %.3f aspect frustum"),Fov,Aspect));
 }
 bool LongSourceFits=true;const FBox LongSource(FVector(-100,-1,-2),FVector(240,1,2));
 for(float Fov:{45.f,60.f,85.f,110.f}){FTransform Fit;LongSourceFits&=FitViewBounds(LongSource,Fov,21.f/9,Fit)&&LunarCarryCheckPrivate::IndependentFrustum(LongSource,Fit,Fov,21.f/9);}
 Check(LongSourceFits,TEXT("Long asymmetric source bounds keep an interior margin instead of a floating-point edge failure"));
 const float NaN=std::numeric_limits<float>::quiet_NaN();FTransform Invalid;
 Check(!FitViewBounds(FBox(ForceInit),85,16.f/9,Invalid)&&!FitViewBounds(FBox(FVector::ZeroVector,FVector::ZeroVector),85,16.f/9,Invalid)&&!FitViewBounds(Asymmetric,NaN,16.f/9,Invalid)&&!FitViewBounds(Asymmetric,0,16.f/9,Invalid)&&!FitViewBounds(Asymmetric,180,16.f/9,Invalid)&&!FitViewBounds(Asymmetric,85,0,Invalid)&&!FitViewBounds(Asymmetric,85,NaN,Invalid),TEXT("Carry fit rejects missing zero nonfinite and invalid frustum inputs"));
 auto* P=G?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0)):nullptr;
 auto* PC=P?Cast<APlayerController>(P->GetController()):nullptr;
 Check(G&&P&&P->Camera&&PC,TEXT("Carry checks have an actual possessed astronaut and camera"));if(!G||!P||!P->Camera||!PC)return;
 Build(P);Check(P->CarriedModule&&P->CarriedModule->GetStaticMesh()&&P->BodyCarryItem,TEXT("Carry viewmodel source and reusable body proxy exist in the playable world"));if(!P->CarriedModule||!P->CarriedModule->GetStaticMesh()||!P->BodyCarryItem)return;
 const bool WasApollo=G->Apollo;const FTransform ActorBefore=P->GetActorTransform(),CameraBefore=P->Camera->GetRelativeTransform();
 const bool UseControlBefore=P->Camera->bUsePawnControlRotation,ConstrainBefore=P->Camera->bConstrainAspectRatio;
 const float OxygenBefore=G->Oxygen,ElapsedBefore=G->ApolloElapsed;const uint8 SamplesBefore=G->SamplesCollected,PartsBefore=G->ApolloParts;
 {
  TGuardValue<bool> ApolloGuard(G->Apollo,false),MarsGuard(G->Mars,G->Mars),MenuGuard(G->Menu,false),PausedGuard(G->Paused,false),JournalGuard(G->Journal,false),ThirdGuard(P->ThirdPerson,false),PhotoPendingGuard(G->ApolloPhotoPending,false);
  TGuardValue<ELunarStage> StageGuard(G->Stage,ELunarStage::RestorePower);
  TGuardValue<int32> RepairGuard(G->RepairStep,0),PhotoGuard(P->PhotoCaptureFrames,0),ToolGuard(G->ApolloTool,0),SampleGuard(G->ApolloPackedSample,INDEX_NONE),PartGuard(G->ApolloPackedPart,INDEX_NONE);
  TGuardValue<float> FovGuard(P->Camera->FieldOfView,85),SampleTimerGuard(G->ApolloPackedPreviewRemaining,0),PartTimerGuard(G->ApolloPackedPartRemaining,0),BlendGuard(P->WorkBlend,1.f);
  TGuardValue<uint8> SampleMaskGuard(G->SamplesCollected,G->SamplesCollected),PartMaskGuard(G->ApolloParts,G->ApolloParts);
  TGuardValue<TObjectPtr<ULunarPanel>> PanelGuard(P->ActivePanel,nullptr);
  TGuardValue<TObjectPtr<AActor>> SourceGuard(P->WorkSource,P->WorkSource);
  TGuardValue<TObjectPtr<UStaticMeshComponent>> SourceMeshGuard(P->WorkSourceMesh,P->WorkSourceMesh);
  TGuardValue<FTransform> SourceTransformGuard(P->WorkSourceTransform,P->WorkSourceTransform);
  TGuardValue<FVector> ContactGuard(P->WorkContact,P->WorkContact);
  P->Camera->bUsePawnControlRotation=false;P->Camera->bConstrainAspectRatio=false;
  int32 Width=0,Height=0;PC->GetViewportSize(Width,Height);const float Aspect=Width>0&&Height>0?float(Width)/Height:16.f/9;
  for(float Fov:{45.f,60.f,85.f,110.f})for(float Pitch:{-75.f,0.f,65.f}){
   P->Camera->SetFieldOfView(Fov);P->Camera->SetWorldRotation(FRotator(Pitch,31,0));const FTransform Pinned=P->Camera->GetComponentTransform();
   Update(P);P->WorkBlend=1;P->UpdateSuit(0);P->Camera->SetWorldTransform(Pinned);Update(P);
   const FTransform InView=P->CarriedModule->GetComponentTransform().GetRelativeTransform(P->Camera->GetComponentTransform());
   const bool BoundsGood=LunarCarryCheckPrivate::IndependentFrustum(P->CarriedModule->GetStaticMesh()->GetBoundingBox(),InView,Fov,Aspect);
   Check(P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible()&&BoundsGood,*FString::Printf(TEXT("Actual cell remains visible and bounded at %.0f FOV and %.0f pitch"),Fov,Pitch));
   Check(LunarCarryCheckPrivate::ProtectedFaceClear(P,Aspect),*FString::Printf(TEXT("Cell centre and upper face clear sampled visible hand-rig geometry at %.0f FOV %.0f pitch"),Fov,Pitch));
   Check(LunarCarryCheckPrivate::TipsNearSource(P),*FString::Printf(TEXT("Both rendered hands have a sampled fingertip within 3cm of the actual cell mesh and at most 1.5cm signed grip-region depth at %.0f FOV %.0f pitch"),Fov,Pitch));
   Check(LunarCarryCheckPrivate::RenderedWristOnGrip(P,true),*FString::Printf(TEXT("Actual rendered hand bones follow the cell grip frame at %.0f FOV %.0f pitch"),Fov,Pitch));
   Check(LunarCarryCheckPrivate::RenderedArmReach(P,true),*FString::Printf(TEXT("Actual rendered cell-carry arm bones retain 28/24cm segment lengths and bounded reach at %.0f FOV %.0f pitch"),Fov,Pitch));
   if(Width>0&&Height>0)Check(LunarCarryCheckPrivate::ActualViewport(P,PC,P->CarriedModule,Width,Height),*FString::Printf(TEXT("Engine projects every actual cell corner inside viewport at %.0f FOV %.0f pitch"),Fov,Pitch));
  }
  if(Width<=0||Height<=0)UE_LOG(LogTemp,Display,TEXT("CARRY_CHECKS SKIP engine viewport projection: no rendered viewport; camera-space source bounds were checked"));
  FVector Left,Right;Check(CellGrip(P,-1,Left)&&CellGrip(P,1,Right)&&Left.Y<Right.Y,TEXT("Cell palms derive from the actual fitted source side bounds"));
  P->ThirdPerson=true;Update(P);
  Check(!P->CarriedModule->IsVisible()&&P->BodyCarryItem->IsVisible()&&P->BodyCarryItem->GetAttachParent()&&P->BodyCarryItem->GetAttachParent()->GetAttachParent()==P->GetCapsuleComponent(),TEXT("Third-person cell uses a capsule-attached body proxy instead of floating in the camera"));
  P->UpdateSuit(0);Update(P);const bool BodyCellWrist=LunarCarryCheckPrivate::RenderedWristOnGrip(P,false),BodyCellContact=LunarCarryCheckPrivate::RenderedFingerContact(P,P->BodyCarryItem,false);
  Check(BodyCellWrist&&BodyCellContact,TEXT("Actual third-person hand and fingertip bones support the body-carried cell"));
  const bool BodyCellReach=LunarCarryCheckPrivate::RenderedArmReach(P,false),BodyCellClear=LunarCarryCheckPrivate::BodyItemClear(P);
  Check(BodyCellReach&&BodyCellClear,TEXT("Body cell stays clear of the torso while both actual arm segments remain within authored reach"));
  {
   TGuardValue<float> GaitGuard(P->GaitBlend,.85f),PhaseGuard(P->GaitPhase,.3f);TGuardValue<bool> FastGuard(P->FastStride,true);
   P->UpdateSuit(0);Update(P);
   const bool CrouchReach=LunarCarryCheckPrivate::RenderedArmReach(P,false),CrouchWrist=LunarCarryCheckPrivate::RenderedWristOnGrip(P,false),CrouchClear=LunarCarryCheckPrivate::BodyItemClear(P);
   Check(CrouchReach&&CrouchWrist&&CrouchClear,TEXT("A synthetic fast-stride crouch keeps actual body carry segments and chest clearance bounded"));
  }
  P->UpdateSuit(0);Update(P);

  const FTransform BodyBefore=P->BodyCarryItem->GetComponentTransform().GetRelativeTransform(P->GetActorTransform());P->Camera->SetWorldRotation(FRotator(-70,-115,0));Update(P);
  Check(P->BodyCarryItem->GetComponentTransform().GetRelativeTransform(P->GetActorTransform()).Equals(BodyBefore,.001),TEXT("Orbit and look pitch do not detach the body-carried cell"));
  G->RepairStep=1;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Seating the cell clears both carried proxies immediately"));G->RepairStep=0;
  P->ThirdPerson=false;Update(P);const FTransform Frozen=P->CarriedModule->GetComponentTransform().GetRelativeTransform(P->Camera->GetComponentTransform());
  G->Paused=true;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible()&&P->CarriedModule->GetComponentTransform().GetRelativeTransform(P->Camera->GetComponentTransform()).Equals(Frozen,.001),TEXT("Pause hides carried proxies without advancing their transform or gameplay"));G->Paused=false;
  P->ActivePanel=NewObject<ULunarPanel>(P);Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Notebook and settings panels suppress carried items"));P->ActivePanel=nullptr;
  G->Journal=true;Update(P);Check(!P->CarriedModule->IsVisible(),TEXT("Journal overlay suppresses the cell viewmodel"));G->Journal=false;
  G->Menu=true;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Mission menus never retain a carried cell"));G->Menu=false;
  P->PhotoCaptureFrames=1;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Evidence photo frames suppress all carried proxies"));P->PhotoCaptureFrames=0;
  Update(P,true);Check(!P->CarriedModule->IsVisible(),TEXT("Latched photo suppression survives the rig decrementing its frame counter"));Update(P);
  Check(P->CarriedModule->IsVisible(),TEXT("Cell returns after photograph suppression without new cargo or pickup"));
  G->Stage=ELunarStage::Briefing;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Restart briefing clears both carried representations"));
  if(WasApollo&&G->SampleActors.Num()>=3&&G->ApolloPartActors.Num()>=3){
   G->Apollo=true;G->Stage=ELunarStage::ApolloSamples;G->SamplesCollected=1;G->ApolloPackedSample=0;G->ApolloPackedPreviewRemaining=2.2f;P->ThirdPerson=true;P->BeginApolloWork(G->SampleActors[0]);P->UpdateApolloWorkVisuals();Update(P);
   Check(P->BodyCarryItem->IsVisible()&&P->BodyCarryItem->GetStaticMesh()==P->ApolloToolMeshes.FindRef(TEXT("SM_SamplePouch"))&&!P->WorkPayload->IsVisible(),TEXT("Committed Apollo sample has a body pouch in third person without a camera payload"));
   G->SamplesCollected=0;Update(P);Check(!P->BodyCarryItem->IsVisible(),TEXT("A preview timer alone cannot fabricate committed body cargo"));G->SamplesCollected=1;
   G->ApolloPackedPreviewRemaining=0;Update(P);Check(!P->BodyCarryItem->IsVisible()&&G->SamplesCollected==1,TEXT("Automatic stow ends the body preview without losing recorded cargo"));
   G->SamplesCollected=4;G->ApolloPackedSample=2;G->ApolloPackedPreviewRemaining=1.5f;Update(P);
   Check(P->BodyCarryItem->IsVisible()&&P->BodyCarryItem->GetStaticMesh()==P->ApolloToolMeshes.FindRef(TEXT("SM_DriveTube")),TEXT("Committed core uses the actual capped tube body preview"));
   G->ApolloPackedPreviewRemaining=0;G->ApolloParts=1;G->ApolloPackedPart=0;G->ApolloPackedPartRemaining=2.2f;P->BeginApolloWork(G->ApolloPartActors[0]);Update(P);
   Check(P->BodyCarryItem->IsVisible()&&P->WorkSourceMesh&&P->BodyCarryItem->GetStaticMesh()==P->WorkSourceMesh->GetStaticMesh(),TEXT("Committed hardware body preview copies the actual retrieved source mesh"));
   UStaticMesh* RecordedMesh=P->BodyCarryItem->GetStaticMesh();P->BeginApolloWork(G->ApolloPartActors[1]);Update(P);
   Check(P->BodyCarryItem->IsVisible()&&P->BodyCarryItem->GetStaticMesh()==RecordedMesh&&P->WorkSource==G->ApolloPartActors[1],TEXT("Starting another task cannot replace the already-recorded carried hardware"));
   const uint8 Cargo=G->ApolloParts;G->Paused=true;Update(P);Check(!P->BodyCarryItem->IsVisible()&&G->ApolloPackedPartRemaining==2.2f&&G->ApolloParts==Cargo,TEXT("Paused hardware carry neither advances stow time nor changes cargo"));G->Paused=false;
   P->ThirdPerson=false;P->Camera->SetFieldOfView(85);P->UpdateApolloWorkVisuals();Update(P);
   Check(P->WorkPayload->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Switching to first person restores the existing hardware carry preview"));
   Update(P,true);Check(!P->WorkPayload->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("A screenshot also clears an already-visible Apollo payload"));
   G->ApolloPackedPartRemaining=0;P->UpdateApolloWorkVisuals();Update(P);Check(!P->WorkPayload->IsVisible()&&!P->BodyCarryItem->IsVisible()&&G->ApolloParts==Cargo,TEXT("Hardware stow removes both previews while keeping the committed item"));
   for(int32 Part=0;Part<3;++Part)for(float Fov:{45.f,85.f})for(bool Body:{false,true}){
    G->ApolloParts=1<<Part;G->ApolloPackedPart=Part;G->ApolloPackedPartRemaining=1.4f;G->ApolloPackedPreviewRemaining=0;P->ThirdPerson=Body;P->Camera->SetFieldOfView(Fov);P->BeginApolloWork(G->ApolloPartActors[Part]);
    P->UpdateApolloWorkVisuals();Update(P);P->UpdateSuit(0);Update(P);auto* Item=Body?P->BodyCarryItem.Get():P->WorkPayload.Get();
    const bool ItemVisible=Item&&Item->IsVisible(),WristOK=LunarCarryCheckPrivate::RenderedWristOnGrip(P,!Body),ReachOK=LunarCarryCheckPrivate::RenderedArmReach(P,!Body),ClearOK=!Body||LunarCarryCheckPrivate::BodyItemClear(P),ContactOK=LunarCarryCheckPrivate::RenderedFingerContact(P,Item,!Body);
    if(!(ItemVisible&&WristOK&&ReachOK&&ClearOK&&ContactOK))UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG combined hardware=%d fov=%.3f body=%d visible=%d wrist=%d reach=%d clear=%d contact=%d"),Part,Fov,Body,ItemVisible,WristOK,ReachOK,ClearOK,ContactOK);
    Check(ItemVisible&&WristOK&&ReachOK&&ClearOK&&ContactOK,*FString::Printf(TEXT("Recorded hardware %d has bounded actual arm segments, torso clearance and sampled mesh-surface fingertip support within its explicit solid/cloth domain at %.0f FOV in %s"),Part,Fov,Body?TEXT("body view"):TEXT("first person")));
   }
   G->ApolloPackedPartRemaining=0;
   for(int32 Sample=0;Sample<3;++Sample)for(float Fov:{45.f,85.f})for(bool Body:{false,true}){
    G->SamplesCollected=1<<Sample;G->ApolloPackedSample=Sample;G->ApolloPackedPreviewRemaining=1.2f;P->ThirdPerson=Body;P->Camera->SetFieldOfView(Fov);P->BeginApolloWork(G->SampleActors[Sample]);
    P->UpdateApolloWorkVisuals();Update(P);P->UpdateSuit(0);Update(P);auto* Item=Body?P->BodyCarryItem.Get():Sample==2?P->WorkTool.Get():P->SamplePouch.Get();
    const bool ItemVisible=Item&&Item->IsVisible(),WristOK=LunarCarryCheckPrivate::RenderedWristOnGrip(P,!Body),ReachOK=LunarCarryCheckPrivate::RenderedArmReach(P,!Body),ClearOK=!Body||LunarCarryCheckPrivate::BodyItemClear(P),ContactOK=LunarCarryCheckPrivate::RenderedFingerContact(P,Item,!Body);
    if(!(ItemVisible&&WristOK&&ReachOK&&ClearOK&&ContactOK))UE_LOG(LogTemp,Warning,TEXT("CARRY_DIAG combined sample=%d fov=%.3f body=%d visible=%d wrist=%d reach=%d clear=%d contact=%d"),Sample,Fov,Body,ItemVisible,WristOK,ReachOK,ClearOK,ContactOK);
    Check(ItemVisible&&WristOK&&ReachOK&&ClearOK&&ContactOK,*FString::Printf(TEXT("Recorded sample %d has bounded actual arm segments, torso clearance and sampled mesh-surface fingertip support within its explicit solid/cloth domain at %.0f FOV in %s"),Sample,Fov,Body?TEXT("body view"):TEXT("first person")));
   }
   G->ApolloPackedPreviewRemaining=0;Update(P);FVector DefaultWrist(1,2,3);FRotator DefaultRotation(4,5,6);
   Check(!GripPose(P,true,1,DefaultWrist,DefaultRotation)&&DefaultWrist==FVector(1,2,3)&&DefaultRotation==FRotator(4,5,6),TEXT("Stowing clears the carry grip and leaves the normal hand pose untouched"));

  }
  Check(P->CarriedModule->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&P->BodyCarryItem->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!P->BodyCarryItem->CanEverAffectNavigation()&&!P->BodyCarryItem->IsComponentTickEnabled(),TEXT("Carry proxies have no collision navigation or independent Tick"));
 }
 P->Camera->bUsePawnControlRotation=UseControlBefore;P->Camera->bConstrainAspectRatio=ConstrainBefore;P->Camera->SetRelativeTransform(CameraBefore);P->UpdateSuit(0);P->Camera->SetRelativeTransform(CameraBefore);P->UpdateApolloWorkVisuals();Update(P);
 Check(G->Oxygen==OxygenBefore&&G->ApolloElapsed==ElapsedBefore&&G->SamplesCollected==SamplesBefore&&G->ApolloParts==PartsBefore&&P->GetActorTransform().Equals(ActorBefore),TEXT("Carry checks restore mission cargo and never change resources time or capsule position"));
}
void LunarCarryPresentation::RunChecks(ALunarGameMode* G){
 FString Report;int32 Failed=0;auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};ValidateChecks(G,Check);
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);if(G&&!FFileHelper::SaveStringToFile(Report,*G->ReportPath(TEXT("carry-checks.txt"))))++Failed;
 UE_LOG(LogTemp,Display,TEXT("CARRY_PRESENTATION_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}
int32 LunarCarryPresentation::ReviewFixtureCount(){return 10;}
bool LunarCarryPresentation::ApplyReviewFixture(ALunarGameMode* G,int32 Index,FString& Name){
 auto* P=G?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0)):nullptr;if(!G||!P||!P->Camera||Index<0||Index>=ReviewFixtureCount())return false;
 // Explicit synthetic, transient presentation fixtures. They never invoke a
 // pickup, save, resource update or mission completion action.
 G->Menu=false;G->Paused=false;G->Journal=false;G->ApolloTool=0;G->ApolloPhotoPending=false;P->PhotoCaptureFrames=0;G->ApolloPackedPreviewRemaining=G->ApolloPackedPartRemaining=0;
 P->Camera->SetFieldOfView(Index==1?45:85);P->Camera->bUsePawnControlRotation=false;P->ThirdPerson=Index==2;
 if(G->Apollo){
  if(G->SampleActors.Num()<3||G->ApolloPartActors.Num()<3)return false;
  G->Stage=Index==2?ELunarStage::ApolloRecover:ELunarStage::ApolloSamples;G->SamplesCollected=Index==2?7:Index==1?4:1;G->ApolloParts=Index==2?1:0;
  G->ApolloInstruments=3;G->ApolloPhotos=G->SamplesCollected;G->SurveyorPhotographed=Index==2;
  if(Index==2){P->BeginApolloWork(G->ApolloPartActors[0]);G->ApolloPackedPart=0;G->ApolloPackedPartRemaining=1.8f;Name=TEXT("apollo-hardware-body");}
  else {P->BeginApolloWork(G->SampleActors[Index==1?2:0]);G->ApolloPackedSample=Index==1?2:0;G->ApolloPackedPreviewRemaining=Index==3?0:1.8f;Name=Index==1?TEXT("apollo-core-fp45"):Index==3?TEXT("apollo-stowed"):TEXT("apollo-pouch-fp");}
 }else{G->Stage=ELunarStage::RestorePower;G->RepairStep=Index==3?1:0;Name=Index==0?TEXT("cell-fp85"):Index==1?TEXT("cell-fp45-lookdown"):Index==2?TEXT("cell-body"):TEXT("cell-seated");}
 if(P->ThirdPerson){P->Camera->SetWorldLocation(P->GetActorLocation()+P->GetActorForwardVector()*230+FVector(0,100,60));P->Camera->SetWorldRotation((P->GetActorLocation()-P->Camera->GetComponentLocation()).Rotation());}
 else{P->Camera->SetRelativeLocation(FVector(0,0,68));P->Camera->SetWorldRotation(FRotator(Index==1?-70:0,P->GetActorRotation().Yaw,0));}
 if(Index>=4){
  P->ThirdPerson=Index==6||Index==7;P->Camera->SetFieldOfView(Index==5||Index==8?45:85);
  if(G->Apollo){
   G->ApolloPackedPartRemaining=G->ApolloPackedPreviewRemaining=0;G->Stage=ELunarStage::ApolloRecover;
   if(Index==4||Index==5){G->ApolloParts=1;G->ApolloPackedPart=0;G->ApolloPackedPartRemaining=1.4f;P->BeginApolloWork(G->ApolloPartActors[0]);Name=Index==4?TEXT("apollo-hardware-fp85"):TEXT("apollo-hardware-fp45");}
   else {const int32 Sample=Index==7||Index==9?2:0;G->SamplesCollected=1<<Sample;G->ApolloPackedSample=Sample;G->ApolloPackedPreviewRemaining=1.2f;P->BeginApolloWork(G->SampleActors[Sample]);Name=Index==6?TEXT("apollo-pouch-body"):Index==7?TEXT("apollo-core-body"):Index==8?TEXT("apollo-pouch-fp45"):TEXT("apollo-core-fp85");}
  }else{G->Stage=ELunarStage::RestorePower;G->RepairStep=0;Name=Index==4?TEXT("cell-fp85-grip"):Index==5?TEXT("cell-fp45-grip"):Index==6?TEXT("cell-body-grip"):Index==7?TEXT("cell-body-orbit"):Index==8?TEXT("cell-fp45-up"):TEXT("cell-fp85-down");}
  if(P->ThirdPerson){P->Camera->SetWorldLocation(P->GetActorLocation()+P->GetActorForwardVector()*230+FVector(0,Index==7?-120:100,60));P->Camera->SetWorldRotation((P->GetActorLocation()-P->Camera->GetComponentLocation()).Rotation());}
  else{P->Camera->SetRelativeLocation(FVector(0,0,68));P->Camera->SetWorldRotation(FRotator(Index==8?45:Index==9?-70:0,P->GetActorRotation().Yaw,0));}
 }
 // Settle only the cosmetic carry pose; preserve the explicitly pinned shot.
 const FTransform PinnedView=P->Camera->GetComponentTransform();
 P->WorkBlend=(IsCellCarried(G)||G->ApolloPackedPreviewRemaining>0||G->ApolloPackedPartRemaining>0)?1.f:0.f;
 P->UpdateSuit(0);P->Camera->SetWorldTransform(PinnedView);P->UpdateApolloWorkVisuals();Update(P);return true;
}
