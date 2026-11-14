#include "LunarDeparture.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include <limits>

// Run only in a fresh automated briefing. The last assertion deliberately
// enters the actual mission through Skip -> CommitMissionStart, once.
void ALunarDepartureScene::ValidateChecks(ALunarGameMode* Mission,TFunctionRef<void(bool,const TCHAR*)> Check){
 const bool Ready=Mission&&Mission->GetWorld()&&!Mission->Menu&&Mission->Stage==ELunarStage::Briefing&&!Mission->Paused&&ULunarSettings::IsAutomation();
 Check(Ready,TEXT("Departure checks use an isolated fresh mission briefing"));if(!Ready)return;
 auto* Pawn=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Mission,0));auto* PC=UGameplayStatics::GetPlayerController(Mission,0);
 Check(Pawn&&PC&&Pawn->Camera&&Pawn->GetCharacterMovement(),TEXT("Departure has a possessed camera and movement component"));if(!Pawn||!PC||!Pawn->Camera||!Pawn->GetCharacterMovement())return;
 Reset(Mission);
 const FVector Location=Pawn->GetActorLocation();const FRotator Aim=PC->GetControlRotation();const float FOV=Pawn->Camera->FieldOfView;const bool Hidden=Pawn->IsHidden(),ThirdPerson=Pawn->ThirdPerson;
 auto* Move=Pawn->GetCharacterMovement();const auto MoveMode=Move->MovementMode;const bool MoveTick=Move->IsComponentTickEnabled();AActor* View=PC->GetViewTarget();
 const float Oxygen=Mission->Oxygen,Elapsed=Mission->ApolloElapsed,Distance=Mission->ApolloDistance;const uint8 Samples=Mission->SamplesCollected,Photos=Mission->ApolloPhotos,Parts=Mission->ApolloParts,Instruments=Mission->ApolloInstruments;const int32 Repair=Mission->RepairStep;const FGuid Run=Mission->ApolloRunId;
 auto FactsUnchanged=[&](){return Mission->Stage==ELunarStage::Briefing&&Mission->Oxygen==Oxygen&&Mission->ApolloElapsed==Elapsed&&Mission->ApolloDistance==Distance&&Mission->SamplesCollected==Samples&&Mission->ApolloPhotos==Photos&&Mission->ApolloParts==Parts&&Mission->ApolloInstruments==Instruments&&Mission->RepairStep==Repair&&Mission->ApolloRunId==Run;};
 Mission->Menu=true;Check(Begin(Mission)==nullptr,TEXT("Menu cannot launch the in-mission departure scene"));Mission->Menu=false;
 Mission->Stage=ELunarStage::Won;Check(Begin(Mission)==nullptr,TEXT("Completed missions cannot replay departure over their results"));Mission->Stage=ELunarStage::Briefing;
 auto* Scene=Begin(Mission);Check(Scene&&Active(Mission),TEXT("Fresh mission builds the authored 3D cabin"));if(!Scene)return;
 Check(Begin(Mission)==Scene,TEXT("Repeated departure start is idempotent"));
 Check(Scene->VisualCount()>0&&Scene->VisualCount()<=MaxVisualComponents,TEXT("Departure stays inside its 48-mesh component budget"));
 bool Cosmetic=true;TArray<UStaticMeshComponent*> Meshes;Scene->GetComponents(Meshes);for(auto* C:Meshes)Cosmetic&=C->GetStaticMesh()&&C->GetMaterial(0)&&C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!C->CanEverAffectNavigation()&&!C->IsComponentTickEnabled();
 Check(Cosmetic,TEXT("Cabin geometry has retained materials and no collision navigation or per-component Tick"));
 Check(PC->GetViewTarget()==Scene&&Pawn->IsHidden()&&Move->MovementMode==MOVE_None,TEXT("External departure view hides and freezes the unchanged player"));
 Check(Skip(Mission)&&Active(Mission)&&Scene->PresentationTime()==0,TEXT("Initial input is consumed without click-through skipping"));
 Check(Scene->ShotCamera->GetComponentLocation().Equals(Scene->FinalEye,.05f)&&Scene->ShotCamera->GetComponentRotation().Equals(Scene->FinalAim,.05f),TEXT("Departure begins at the exact original view without a zoom or travel curve"));
 Scene->AdvancePresentation(.4f);const float Time=Scene->PresentationTime();Mission->Paused=true;Scene->AdvancePresentation(3);
 Check(Active(Mission)&&Scene->PresentationTime()==Time&&Skip(Mission)&&Active(Mission),TEXT("Pause freezes cabin time and consumes skip without starting gameplay"));Mission->Paused=false;
 const auto Panel=Pawn->ActivePanel;Pawn->ActivePanel=NewObject<ULunarPanel>(Pawn);Scene->AdvancePresentation(3);
 Check(Scene->PresentationTime()==Time&&Skip(Mission)&&Active(Mission),TEXT("An open UI panel freezes departure and blocks hidden skip input"));Pawn->ActivePanel=Panel;
 Scene->AdvancePresentation(0);Scene->AdvancePresentation(-10);Scene->AdvancePresentation(std::numeric_limits<float>::quiet_NaN());Scene->AdvancePresentation(std::numeric_limits<float>::infinity());
 Check(Scene->PresentationTime()==Time,TEXT("Zero negative and nonfinite departure deltas leave the clock unchanged"));
 Scene->AdvancePresentation(.65f);Check(Scene->HatchFraction()==1&&Scene->Progress()>0&&Scene->Progress()<1&&!Scene->Caption().IsEmpty()&&!Scene->Guide().IsEmpty(),TEXT("Timed hatch opens with readable mission-specific guidance before exit"));
 Scene->AdvancePresentation(Duration-Scene->PresentationTime()-.01f);
 // Inspect the actual camera rather than a camera-manager cache in this
 // direct fixture; a separate staged rendered review is still required.
 Check(Scene->ShotCamera->GetComponentLocation().Equals(Scene->FinalEye,.05f)&&Scene->ShotCamera->GetComponentRotation().Equals(Scene->FinalAim,.05f)&&FMath::IsNearlyEqual(Scene->ShotCamera->FieldOfView,FOV),TEXT("Camera position rotation and FOV remain at the original surface view before handover"));
 Check(FactsUnchanged()&&Pawn->GetActorLocation().Equals(Location,.001f),TEXT("Cabin playback never consumes mission resources moves the pawn or edits mission progress"));
 Abort(Mission);Check(!Active(Mission)&&!Mission->DepartureScene,TEXT("Abort removes the transient cabin without launching the mission"));
 Check(FactsUnchanged()&&PC->GetViewTarget()==View&&Pawn->IsHidden()==Hidden&&Move->MovementMode==MoveMode&&Move->IsComponentTickEnabled()==MoveTick,TEXT("Abort restores camera visibility and movement without resetting progress"));
 Check(PC->GetControlRotation().Equals(Aim,.001f)&&Pawn->ThirdPerson==ThirdPerson&&FMath::IsNearlyEqual(Pawn->Camera->FieldOfView,FOV),TEXT("Departure preserves player aim FOV and camera-view preference"));
 Scene=Begin(Mission);Check(Scene&&Active(Mission),TEXT("A fresh launch can begin after an aborted presentation"));if(!Scene)return;
 Scene->AdvancePresentation(SkipDelay+.01f);Check(Skip(Mission)&&!Active(Mission)&&Mission->IsActive(),TEXT("Allowed skip restores the surface view and commits the actual mission start"));
 Check(PC->GetViewTarget()==View&&Pawn->IsHidden()==Hidden&&Pawn->GetActorLocation().Equals(Location,.001f)&&PC->GetControlRotation().Equals(Aim,.001f)&&FMath::IsNearlyEqual(Pawn->Camera->FieldOfView,FOV),TEXT("Committed handover restores the original player view without a teleport"));
 Check(!Skip(Mission)&&Begin(Mission)==nullptr,TEXT("A completed departure cannot commit twice or replay during active gameplay"));
 Reset(Mission);Check(!Mission->DepartureScene&&!Active(Mission),TEXT("Departure reset removes hidden presentation state after handover"));
}
