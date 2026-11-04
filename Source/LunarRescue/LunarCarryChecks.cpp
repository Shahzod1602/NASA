#include "LunarCarryPresentation.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/StaticMesh.h"
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
bool ProtectedFaceClear(ALunarCharacter* P,float Aspect){
 if(!P||!P->HandRig||!P->HandRig->IsVisible()||P->Gloves.Num()!=16||P->Gloves[0]->IsVisible()||P->Gloves[8]->IsVisible())return false;
 FBox2D Cell;if(!ProjectEnvelope(P,P->CarriedModule,Aspect,Cell)||Cell.GetCenter().Y>.51||Cell.Max.Y>.64)return false;
 const FVector2D Size=Cell.GetSize();const FBox2D Protected(Cell.Min+Size*FVector2D(.28,.20),Cell.Min+Size*FVector2D(.72,.72));
 // These posed static meshes drive the actual skinned hand bones. Their
 // projected boxes are conservative palm/cuff/forearm envelopes, not proof of
 // final pixels; the paired rendered-cell control is a separate visual check.
 for(int32 Index:{0,1,2,8,9,10}){FBox2D Hand;if(!ProjectEnvelope(P,P->Gloves[Index],Aspect,Hand))return false;
  if(Hand.Min.X<Protected.Max.X&&Hand.Max.X>Protected.Min.X&&Hand.Min.Y<Protected.Max.Y&&Hand.Max.Y>Protected.Min.Y)return false;
 }
 return true;
}
bool TipsNearSource(ALunarCharacter* P){
 if(!P||!P->HandRig||!P->CarriedModule||!P->CarriedModule->GetStaticMesh())return false;
 const FBox B=P->CarriedModule->GetStaticMesh()->GetBoundingBox();const FTransform Cell=P->CarriedModule->GetComponentTransform();
 for(const TCHAR* Side:{TEXT("l"),TEXT("r")}){double Nearest=1.e9;int32 Found=0;
  for(int32 Finger=0;Finger<5;++Finger){const FName Bone(*FString::Printf(TEXT("tip%d_%s"),Finger,Side));if(P->HandRig->GetBoneIndex(Bone)==INDEX_NONE)continue;++Found;
   const FVector Tip=P->HandRig->GetBoneLocationByName(Bone,EBoneSpaces::WorldSpace),Local=Cell.InverseTransformPosition(Tip);
   const FVector OnBox(FMath::Clamp(Local.X,B.Min.X,B.Max.X),FMath::Clamp(Local.Y,B.Min.Y,B.Max.Y),FMath::Clamp(Local.Z,B.Min.Z,B.Max.Z));
   Nearest=FMath::Min(Nearest,FVector::Dist(Tip,Cell.TransformPosition(OnBox)));
  }
  if(!Found||Nearest>3.)return false;
 }
 return true;
}
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
   Check(LunarCarryCheckPrivate::ProtectedFaceClear(P,Aspect),*FString::Printf(TEXT("Cell centre and upper face clear posed palm forearm envelopes at %.0f FOV %.0f pitch"),Fov,Pitch));
   Check(LunarCarryCheckPrivate::TipsNearSource(P),*FString::Printf(TEXT("Both posed hands retain a fingertip within 3cm of the source cell box at %.0f FOV %.0f pitch"),Fov,Pitch));
   if(Width>0&&Height>0)Check(LunarCarryCheckPrivate::ActualViewport(P,PC,P->CarriedModule,Width,Height),*FString::Printf(TEXT("Engine projects every actual cell corner inside viewport at %.0f FOV %.0f pitch"),Fov,Pitch));
  }
  if(Width<=0||Height<=0)UE_LOG(LogTemp,Display,TEXT("CARRY_CHECKS SKIP engine viewport projection: no rendered viewport; camera-space source bounds were checked"));
  FVector Left,Right;Check(CellGrip(P,-1,Left)&&CellGrip(P,1,Right)&&Left.Y<Right.Y,TEXT("Cell palms derive from the actual fitted source side bounds"));
  P->ThirdPerson=true;Update(P);
  Check(!P->CarriedModule->IsVisible()&&P->BodyCarryItem->IsVisible()&&P->BodyCarryItem->GetAttachParent()==P->GetCapsuleComponent(),TEXT("Third-person cell uses a capsule-attached body proxy instead of floating in the camera"));
  const FTransform BodyBefore=P->BodyCarryItem->GetRelativeTransform();P->Camera->SetWorldRotation(FRotator(-70,-115,0));Update(P);
  Check(P->BodyCarryItem->GetRelativeTransform().Equals(BodyBefore,.001),TEXT("Orbit and look pitch do not detach the body-carried cell"));
  G->RepairStep=1;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible(),TEXT("Seating the cell clears both carried proxies immediately"));G->RepairStep=0;
  P->ThirdPerson=false;Update(P);const FTransform Frozen=P->CarriedModule->GetRelativeTransform();
  G->Paused=true;Update(P);Check(!P->CarriedModule->IsVisible()&&!P->BodyCarryItem->IsVisible()&&P->CarriedModule->GetRelativeTransform().Equals(Frozen),TEXT("Pause hides carried proxies without advancing their transform or gameplay"));G->Paused=false;
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
int32 LunarCarryPresentation::ReviewFixtureCount(){return 4;}
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
 // Settle only the cosmetic carry pose; preserve the explicitly pinned shot.
 const FTransform PinnedView=P->Camera->GetComponentTransform();
 P->WorkBlend=(IsCellCarried(G)||G->ApolloPackedPreviewRemaining>0||G->ApolloPackedPartRemaining>0)?1.f:0.f;
 P->UpdateSuit(0);P->Camera->SetWorldTransform(PinnedView);P->UpdateApolloWorkVisuals();Update(P);return true;
}
