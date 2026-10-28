#include "LunarGame.h"
#include "LunarSettings.h"
#include "LunarPanels.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace {
int Index(const TArray<TObjectPtr<AActor>>& A,AActor* V){return A.IndexOfByPredicate([V](const auto& P){return P.Get()==V;});}
}
bool ALunarGameMode::OpenApolloTool(){
 if(!Apollo||!IsActive()||Paused||ApolloTool||ApolloPhotoPending||!CanInteract()||Stage==ELunarStage::ApolloTraverse)return false;
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));P&&P->ThirdPerson)P->ToggleSuitView();
 ApolloWorkTarget=InteractionTarget();if(!ApolloWorkTarget)return false;
 ApolloInputHintRemaining=0;
 ApolloToolPhase=0;ApolloToolCooldown=0;ApolloConfirmationRemaining=0;ApolloPackedPreviewRemaining=0;ApolloPackedPartRemaining=0;
 if(ApolloDustMode)ApolloTool=ApolloDustPhotos==3?9:1;
 else if(Stage==ELunarStage::ApolloCheckout)ApolloTool=8;
 else if(Stage==ELunarStage::ApolloDeploy)ApolloTool=ApolloAction==0?2:3;
 else if(Stage==ELunarStage::ApolloSamples){const int I=Index(SampleActors,ApolloWorkTarget);if(I<0)return false;SelectedSample=I;ApolloTool=(ApolloPhotos&(1<<I))?(I==2?5:4):1;}
 else if(Stage==ELunarStage::ApolloRecover){if(!SurveyorPhotographed)ApolloTool=1;else {SelectedSample=Index(ApolloPartActors,ApolloWorkTarget);ApolloTool=6;}}
 else if(Stage==ELunarStage::ApolloReturn)ApolloTool=7;
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0))){P->GetCharacterMovement()->StopMovementImmediately();P->FastStride=false;P->Camera->SetFieldOfView(ULunarSettings::Get()->FieldOfView);P->Camera->bUsePawnControlRotation=ApolloTool==1;}
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))P->BeginApolloWork(ApolloWorkTarget);
 ReleaseInteract();return ApolloTool!=0;
}
void ALunarGameMode::CancelApolloTool(){if(ApolloTool==4&&SelectedSample==0)if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));P&&P->WorkSourceMesh)P->WorkSourceMesh->SetVisibility(true);if(ApolloTool)if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0))){P->Camera->bUsePawnControlRotation=true;P->Camera->SetRelativeLocation(FVector(0,0,68));P->Camera->SetRelativeRotation(FRotator::ZeroRotator);}ApolloTool=0;ApolloToolPhase=0;ApolloToolCooldown=0;ApolloWorkTarget=nullptr;ReleaseInteract();}
void ALunarGameMode::ApolloToolAxis(float X,float Y){
 if(Paused)return;
 if(ApolloTool==2){ApolloLevel.X=FMath::Clamp(ApolloLevel.X+X*.035f,-1.f,1.f);ApolloLevel.Y=FMath::Clamp(ApolloLevel.Y+Y*.035f,-1.f,1.f);}
 if(ApolloTool==3){ApolloAntennaAngle=FMath::UnwindDegrees(ApolloAntennaAngle+X*.6f);if(ApolloAntenna)ApolloAntenna->SetRelativeRotation(FRotator(-65,ApolloAntennaAngle,0));}
}
FString ALunarGameMode::ApolloToolTitle() const {
 switch(ApolloTool){case 9:return TEXT("SURVEYOR / DUST INVESTIGATION");case 1:return TEXT("HASSELBLAD / CONTEXT PHOTOGRAPH");case 2:return TEXT("ALSEP / LEVEL THE SEISMOMETER");case 3:return TEXT("ALSEP / EARTH LINK");case 4:return SelectedSample==1?TEXT("12032 / SOIL SCOOP"):TEXT("12031 / SAMPLE TONGS");case 5:return TEXT("12027 / DRIVE-TUBE CORE");case 6:return SelectedSample==0?TEXT("SURVEYOR / CAMERA BRACKETS"):SelectedSample==1?TEXT("SURVEYOR / SCOOP FITTING"):TEXT("SURVEYOR / CABLE CUTTERS");case 7:return TEXT("INTREPID / CARGO MANIFEST");case 8:return TEXT("INTREPID / EQUIPMENT CHECK");default:return FString();}
}
FString ALunarGameMode::ApolloToolHelp() const {
 if(ApolloToolCooldown>0&&(ApolloTool==4||ApolloTool==5||ApolloTool==6))return TEXT("Working... wait for the tool motion to finish.");
 switch(ApolloTool){
 case 9:return ApolloDustCompared?TEXT("Study recorded. Left click: return to the main task."):FString::Printf(TEXT("Compare the photos. %s: choose. Left click: confirm."),*ULunarSettings::Get()->Key(TEXT("Route")).GetDisplayName().ToString());
 case 1:if(ApolloDustMode)return TEXT("Center the coating panel. Left click: photograph it.");return ApolloWorkTarget==DroneActor?TEXT("Fit the whole spacecraft in the frame. Left click: take photo."):TEXT("Center the sample and nearby ground. Left click: take photo.");
 case 2:return TEXT("Mouse / arrows: center the bubble. Left click: confirm.");
 case 3:return TEXT("Mouse / arrows: find the strongest signal. Left click: confirm.");
 case 4:return ApolloToolPhase==0?(SelectedSample==1?TEXT("Left click: scoop the soil."):TEXT("Left click: lift the rock with tongs.")):TEXT("Left click: place the sample in the pouch and seal it.");
 case 5:return ApolloToolPhase==0?TEXT("Left click: place the tube upright."):ApolloToolPhase<5?FString::Printf(TEXT("Left click: hammer stroke %d / 4."),ApolloToolPhase):ApolloToolPhase==5?TEXT("Left click: withdraw the filled tube."):TEXT("Left click: cap and label the tube.");
 case 6:return ApolloToolPhase==0?(SelectedSample==2?TEXT("Left click: cut the first marked cable end."):TEXT("Left click: release the first marked fitting.")):ApolloToolPhase==1?(SelectedSample==2?TEXT("Left click: cut the second marked cable end."):TEXT("Left click: release the second marked fitting.")):TEXT("Left click: remove and pack this part.");
 case 7:return TEXT("Left click: stow all samples and parts to finish. I: cargo list.");
 case 8:return ApolloAction==0?TEXT("Left click: confirm the suit check."):ApolloAction==1?TEXT("Left click: confirm the camera and tools."):TEXT("Left click: confirm Intrepid as your return point.");
 default:return FString();
 }
}
void ALunarGameMode::ApolloToolClick(){
 if(!ApolloTool||Paused||!IsActive()||ApolloToolCooldown>0)return;
 if(!ApolloWorkTarget||!CanInteractWith(ApolloWorkTarget)){Message(TEXT("Face the work site and move closer. Your unfinished work is safe to cancel."));return;}
 auto* A=ApolloWorkTarget.Get();const int Tool=ApolloTool;
 if(Tool==9){
  if(ApolloDustCompared){CancelApolloTool();ApolloDustMode=false;return;}
  if(ApolloDustChoice!=0){Message(TEXT("The Moon has no rain or Earth-like wind. Compare the protected strip with exposed coating, then try again."));return;}
  ApolloDustCompared=true;CancelApolloTool();ApolloDustMode=false;ConfirmApolloWork(TEXT("Dust comparison recorded. I: evidence and explanation."));Message(ApolloDustExplanation());SaveApolloCheckpoint();return;
 }
 if(Tool==1){
  auto* P=UGameplayStatics::GetPlayerPawn(this,0);FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);
  const FVector Center=A->GetActorLocation()+(A==DroneActor?FVector(0,0,90):FVector::ZeroVector);
  const float Dot=FVector::DotProduct(Aim.Vector(),(Center-Eye).GetSafeNormal());
  if(Dot<.985f){Message(TEXT("Center the target inside the frame, then left click."));return;}
  if(ApolloDustMode){CaptureDustPhoto();return;}
  if(A==DroneActor){
   int W=0,Height=0;if(auto* PC=UGameplayStatics::GetPlayerController(this,0))PC->GetViewportSize(W,Height);
   const float Aspect=W>0&&Height>0?float(W)/Height:16.f/9.f,Tangent=FMath::Tan(FMath::DegreesToRadians(ULunarSettings::Get()->FieldOfView*.5f));
   const FBox Bounds=A->GetComponentsBoundingBox(true);const FRotationMatrix Frame(Aim);const float H=Tangent*.80f,V=Tangent/Aspect*.78f;
   for(int I=0;I<8;++I){const FVector Corner((I&1)?Bounds.Max.X:Bounds.Min.X,(I&2)?Bounds.Max.Y:Bounds.Min.Y,(I&4)?Bounds.Max.Z:Bounds.Min.Z),D=Corner-Eye;const float Depth=FVector::DotProduct(D,Frame.GetUnitAxis(EAxis::X));
    if(Depth<=0||FMath::Abs(FVector::DotProduct(D,Frame.GetUnitAxis(EAxis::Y)))>Depth*H||FMath::Abs(FVector::DotProduct(D,Frame.GetUnitAxis(EAxis::Z)))>Depth*V){Message(TEXT("CAMERA: step back and keep the whole spacecraft, including its legs and solar panel, inside the frame."));return;}
   }
  }
  CancelApolloTool();const bool Enabled=ApolloToolsEnabled;ApolloToolsEnabled=false;InteractApollo();ApolloToolsEnabled=Enabled;return;
 }
 if(Tool==2&&ApolloLevel.Size()>.09f){Message(TEXT("ALSEP: center the bubble before confirming. Use the mouse or arrow keys."));return;}
 if(Tool==3&&FMath::Abs(FMath::FindDeltaAngleDegrees(ApolloAntennaAngle,-7.f))>8){Message(TEXT("ALSEP: signal is weak. Rotate toward the strongest signal, then verify."));return;}
 if(Tool==4||Tool==5||Tool==6){
  ++ApolloToolPhase;ApolloToolCooldown=.75f;
  if(Tool==4&&SelectedSample==0&&ApolloToolPhase==1)if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));P&&P->WorkSourceMesh)P->WorkSourceMesh->SetVisibility(false);PlayPowerSound(TEXT("PowerContact"),.22f);
  const int Required=Tool==4?2:Tool==5?7:3;
  if(ApolloToolPhase<Required)return;
 }
 CancelApolloTool();CompleteApolloInteraction(A);
}
void ALunarCharacter::OpenSettings(){
 if(ActivePanel){ClosePanel();return;}
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* PC=Cast<APlayerController>(GetController());if(!G||!PC)return;
 G->CancelApolloTool();PanelWasPaused=G->Paused;if(G->IsActive()&&!G->Paused)PauseMission();
 ActivePanel=CreateWidget<ULunarPanel>(PC);ActivePanel->Settings=true;ActivePanel->AddToViewport(100);
 PC->bShowMouseCursor=true;FInputModeGameAndUI Input;Input.SetWidgetToFocus(ActivePanel->TakeWidget());Input.SetHideCursorDuringCapture(false);PC->SetInputMode(Input);
}
void ALunarCharacter::OpenEvidence(){
 if(ActivePanel){ClosePanel();return;}
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* PC=Cast<APlayerController>(GetController());if(!G||!G->Apollo||!PC)return;
 G->CancelApolloTool();PanelWasPaused=G->Paused;if(G->IsActive()&&!G->Paused)PauseMission();
 ActivePanel=CreateWidget<ULunarPanel>(PC);ActivePanel->AddToViewport(100);PC->bShowMouseCursor=true;FInputModeGameAndUI Input;Input.SetWidgetToFocus(ActivePanel->TakeWidget());Input.SetHideCursorDuringCapture(false);PC->SetInputMode(Input);
}
void ALunarCharacter::ClosePanel(){
 if(!ActivePanel)return;ULunarSettings::Get()->RevertDisplay();ActivePanel->RemoveFromParent();ActivePanel=nullptr;
 if(auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this))){if(G->Paused&&!PanelWasPaused)PauseMission();if(auto* PC=Cast<APlayerController>(GetController())){PC->bShowMouseCursor=G->Menu;if(G->Menu)PC->SetInputMode(FInputModeGameAndUI());else PC->SetInputMode(FInputModeGameOnly());}}
}
void ALunarCharacter::ToggleApolloMode(){if(auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));G&&G->Apollo&&G->Stage==ELunarStage::Briefing&&!G->ApolloConfirmNew&&!ActivePanel)G->ApolloTimed=!G->ApolloTimed;}
