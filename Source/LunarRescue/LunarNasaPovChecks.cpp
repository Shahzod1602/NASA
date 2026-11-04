#include "LunarGame.h"
#include "LunarCarryPresentation.h"
#include "LunarPovMotion.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include <limits>

void ALunarGameMode::NasaPovChecks(){
 const FString OutputReportPath=ReportPath(TEXT("pov-checks.txt"));
 FString Report;int32 Failed=0;auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC||!Apollo||SampleActors.Num()<2){Check(false,TEXT("POV checks require the Apollo map and a possessed astronaut"));}
 else{
  Check(!LunarPovMotion::HandsVisible(-75,false)&&LunarPovMotion::HandsVisible(0,false)&&LunarPovMotion::HandsVisible(-85,true),TEXT("Free downward inspection hides hands while required task hands stay available"));
  Check(LunarPovMotion::HandRest(-28,false)==0&&LunarPovMotion::HandRest(-58,false)==1&&LunarPovMotion::HandRest(-43,false)>.4f&&LunarPovMotion::HandRest(-43,false)<.6f,TEXT("Free-hand lowering has a bounded transition before hiding"));
  Check(LunarPovMotion::Ratio(2,0)==0&&LunarPovMotion::Ratio(std::numeric_limits<float>::quiet_NaN(),3)==0&&LunarPovMotion::Ratio(-2,3)==0&&LunarPovMotion::Ratio(7,3)==1,TEXT("Gesture progress rejects zero, negative and non-finite resource inputs"));
  const FVector Ground(20,30,0),Carry(80,15,50);
  Check(LunarPovMotion::ScoopLift(Ground,Carry,0).Equals(Ground,.001f)&&LunarPovMotion::ScoopLift(Ground,Carry,1).Equals(Carry,.001f),TEXT("Soil motion starts at its measured contact and ends at the camera carry point"));
  Check(LunarPovMotion::ScoopLift(FVector::ZeroVector,FVector::ZeroVector,.14f).Z<0&&LunarPovMotion::ScoopLift(Ground,Carry,.5f).Z>25,TEXT("Soil presentation has a shallow dig followed by a lifted arc"));
  Check(LunarPovMotion::CargoApproach(1).IsNearlyZero(.001f)&&LunarPovMotion::CargoApproach(0).Z==78&&LunarPovMotion::CargoApproach(.5f).Z>0,TEXT("Cargo carry arcs settle exactly into their existing compartments"));
  StartApollo(true);P->GetCharacterMovement()->StopMovementImmediately();P->GetCharacterMovement()->DisableMovement();
  P->ThirdPerson=false;P->Camera->bUsePawnControlRotation=true;P->Camera->SetRelativeLocation(FVector(0,0,68));P->Camera->SetFieldOfView(85);
  PC->SetControlRotation(FRotator(-75,0,0));P->Camera->SetWorldRotation(PC->GetControlRotation());P->UpdateSuit(0);
  Check(P->HandRig&&!P->HandRig->IsVisible()&&!P->Gloves[0]->IsVisible()&&!P->Gloves[8]->IsVisible(),TEXT("Rendered first-person hands and forearms are hidden during free look-down"));
  Check(P->SuitBody[6]->IsVisible()&&P->SuitBody[9]->IsVisible()&&!P->SuitBody[0]->IsVisible()&&!P->SuitBody[1]->IsVisible(),TEXT("Free look-down retains boots and legs without exposing a waist cap"));
  PC->SetControlRotation(FRotator::ZeroRotator);P->Camera->SetWorldRotation(PC->GetControlRotation());P->UpdateSuit(0);
  Check(P->HandRig->IsVisible(),TEXT("First-person hands return when the player looks ahead"));
  Stage=ELunarStage::ApolloSamples;ApolloPhotos=2;
  P->SetActorLocation(SampleActors[1]->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);PC->SetControlRotation((SampleActors[1]->GetActorLocation()-Eye).Rotation());
  const bool Opened=OpenApolloTool();Check(Opened&&ApolloTool==4&&SelectedSample==1,TEXT("The actual 12032 site opens the soil scoop rather than sample tongs"));
  if(Opened){
   ApolloToolClick();Check(ApolloToolPhase==1&&!(SamplesCollected&2),TEXT("First scoop input enters step 2/2 without falsely recording the sample"));
   const FTransform CapsuleBefore=P->GetActorTransform();const float ClockBefore=ApolloElapsed,BudgetBefore=Oxygen;const uint8 CargoBefore=SamplesCollected;
   for(float Fov:{70.f,85.f,110.f}){
    P->Camera->SetFieldOfView(Fov);ApolloToolCooldown=0;P->UpdateSuit(0);
    bool Hidden=true;for(int32 I=4;I<10;++I)Hidden&=!P->SuitBody[I]->IsVisible();
    Check(Hidden&&P->HandRig->IsVisible(),TEXT("Exact soil step 2/2 omits near-cut thigh proxies and retains task hands at supported FOV"));
    Check(P->SuitRig->bCastHiddenShadow&&!P->SuitRig->IsVisible(),TEXT("Work-camera body hiding preserves the separate full-body shadow rig"));
   }
   Check(P->GetActorTransform().Equals(CapsuleBefore,.001f)&&ApolloElapsed==ClockBefore&&Oxygen==BudgetBefore&&SamplesCollected==CargoBefore,TEXT("Work-camera and hand presentation do not move the capsule or alter mission resources/cargo"));
   ApolloToolCooldown=.375f;P->UpdateSuit(0);const FVector MidTool=P->WorkTool->GetComponentLocation();
   ApolloToolCooldown=0;P->UpdateSuit(0);
   Check(!P->WorkTool->GetComponentLocation().Equals(MidTool,1)&&P->WorkPayload->IsVisible(),TEXT("A soil lift changes the actual rendered tool path and keeps the soil payload visible"));
   const FVector Palm=P->Gloves[9]->GetRelativeLocation()+P->Gloves[10]->GetRelativeRotation().RotateVector(FVector(5,0,0));
   Check(Palm.Equals(P->WorkGrip[1],.2f),TEXT("The moving soil tool and right palm share the same grip frame"));
   CancelApolloTool();ApolloPackedSample=1;ApolloPackedPreviewRemaining=2.2f;P->UpdateSuit(0);
   Check(P->SamplePouch->IsVisible()&&!P->PouchSeal->IsVisible()&&P->WorkPayload->IsVisible(),TEXT("Packed preview begins with an open pouch and a visible deposit"));
   ApolloPackedPreviewRemaining=1.2f;P->UpdateSuit(0);const FVector SealHand=P->WorkGrip[1];
   Check(P->PouchSeal->IsVisible()&&!P->WorkPayload->IsVisible(),TEXT("The actual preview seals after deposit rather than hiding both actions at once"));
   ApolloPackedPreviewRemaining=.2f;P->UpdateSuit(0);
   Check(P->WorkGrip[1].Z<SealHand.Z-20&&SamplesCollected==CargoBefore,TEXT("Sealed cargo lowers away without awarding another sample"));
   const FVector FrozenWrist=P->Gloves[9]->GetRelativeLocation();const FTransform FrozenCamera=P->Camera->GetComponentTransform();
   Paused=true;P->UpdateSuit(.5f);
   Check(P->Gloves[9]->GetRelativeLocation().Equals(FrozenWrist,.001f)&&P->Camera->GetComponentTransform().Equals(FrozenCamera,.001f)&&ApolloPackedPreviewRemaining==.2f,TEXT("Pause freezes preview hands and camera instead of advancing presentation"));Paused=false;
  }
  // These are isolated hand-presentation states, not a Mars traversal claim.
  CancelApolloTool();ApolloPackedPreviewRemaining=0;Apollo=false;Mars=true;Stage=ELunarStage::RestorePower;RepairStep=0;P->WorkBlend=1;P->UpdateSuit(0);
  LunarCarryPresentation::Update(P);FVector CellPalm;
  const FVector ActualCellPalm=P->Gloves[9]->GetRelativeLocation()+P->Gloves[10]->GetRelativeRotation().RotateVector(FVector(5,0,0));
  Check(P->HandRig->IsVisible()&&P->CarriedModule&&P->CarriedModule->IsVisible()&&LunarCarryPresentation::CellGrip(P,1,CellPalm)&&ActualCellPalm.Equals(CellPalm,.2f),TEXT("Power-cell carry has a visible fitted item and a palm on its actual source-side grip"));
  Stage=ELunarStage::CollectData;Holding=true;HoldProgress=1.95f;P->UpdateSuit(0);const FRotator DataRotation=P->Gloves[10]->GetRelativeRotation();const FVector DataHand=P->Gloves[9]->GetRelativeLocation();
  Check(FVector::DotProduct(P->Gloves[11]->GetForwardVector(),P->Gloves[10]->GetForwardVector())>FVector::DotProduct(P->Gloves[12]->GetForwardVector(),P->Gloves[10]->GetForwardVector()),TEXT("Mars science work extends the right index while the other fingers remain curved"));
  Stage=ELunarStage::AlignRelay;HoldProgress=2;P->UpdateSuit(0);
  Check(!P->Gloves[10]->GetRelativeRotation().Equals(DataRotation,5)&&!P->Gloves[9]->GetRelativeLocation().Equals(DataHand,1),TEXT("Mars antenna alignment has a distinct two-hand turning pose"));
  const FVector Frozen=P->Gloves[9]->GetRelativeLocation();Paused=true;P->UpdateSuit(.4f);
  Check(P->Gloves[9]->GetRelativeLocation().Equals(Frozen,.001f)&&HoldProgress==2,TEXT("Pause also freezes the Mars progress-driven hand gesture"));
 }
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failed);FFileHelper::SaveStringToFile(Report,*OutputReportPath);UE_LOG(LogTemp,Display,TEXT("NASA_POV_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}
