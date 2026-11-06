#include "LunarStrideRegression.h"
#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "InputKeyEventArgs.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

namespace {
struct FStrideRun:public TSharedFromThis<FStrideRun> {
 TWeakObjectPtr<ALunarGameMode> Game;TWeakObjectPtr<ALunarCharacter> Pilot;TWeakObjectPtr<APlayerController> Controller;
 TWeakObjectPtr<AActor> Floor;int32 Phase=0,Failures=0;FString Report;float Walk=0,Fast=0,TimeBefore=0,OxygenBefore=0;FVector Before;
 void Check(bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failures;}
 void Key(FKey K,EInputEvent E){if(auto* PC=Controller.Get())PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,E,E==IE_Released?0.f:1.f));}
 void Finish(){
  Key(EKeys::W,IE_Released);Key(EKeys::LeftShift,IE_Released);
  if(auto* P=Pilot.Get()){ULunarSettings::Get()->ToggleSprint=false;P->FastStride=false;P->UpdateStrideSpeed();}
  if(Floor.IsValid())Floor->Destroy();
  Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);
  if(auto* G=Game.Get())FFileHelper::SaveStringToFile(Report,*G->ReportPath(TEXT("stride-regression-checks.txt")));
  FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
 }
 void Step(){
  auto* G=Game.Get();auto* P=Pilot.Get();auto* PC=Controller.Get();if(!G||!P||!PC){++Failures;Finish();return;}
  auto* Move=P->GetCharacterMovement();auto* C=G->FieldOperations.Get();auto* Settings=ULunarSettings::Get();
  if(Phase==0){
   Check(ULunarSettings::IsAutomation()&&!G->ApolloSaveEnabled(),TEXT("Stride fixture is isolated from owner checkpoint writes"));
   if(C)C->Enabled=false;G->ApolloConfirmNew=false;G->StartMission();
   if(C){C->Enabled=true;C->Prepared=C->Briefed=true;}
   Check(G->IsActive()&&!G->Paused&&!P->ActivePanel,TEXT("Movement fixture starts active with resource briefing satisfied"));
   auto* F=G->GetWorld()->SpawnActor<AStaticMeshActor>();Floor=F;auto* Mesh=F?F->GetStaticMeshComponent():nullptr;auto* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));Check(F&&Mesh&&Cube,TEXT("Flat collision fixture actor component and cube asset are available"));if(!F||!Mesh||!Cube){Finish();return;}Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(Cube);Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Mesh->SetCollisionResponseToAllChannels(ECR_Block);F->SetActorLocation(FVector(0,0,29900));F->SetActorScale3D(FVector(100,100,.2f));
   P->SetActorLocation(FVector(0,0,30002),false,nullptr,ETeleportType::TeleportPhysics);Move->StopMovementImmediately();Move->SetMovementMode(MOVE_Walking);P->FastStride=false;Settings->ToggleSprint=false;Settings->Apply(P,false);P->UpdateStrideSpeed();
   G->ApolloTimed=true;G->Oxygen=360;G->OxygenCapacity=480;PC->SetControlRotation(FRotator::ZeroRotator);PC->SetInputMode(FInputModeGameOnly());PC->SetIgnoreMoveInput(false);PC->SetIgnoreLookInput(false);
   Walk=G->Apollo?140.f:320.f;Fast=G->Apollo?220.f:500.f;
   Check(FMath::IsNearlyEqual(Move->MaxWalkSpeed,Walk),TEXT("Normal speed uses mission-specific centimetres per second"));Key(EKeys::W,IE_Pressed);
  }else if(Phase==1){
   Check(Move->IsMovingOnGround()&&P->GetVelocity().Size2D()>Walk*.8f&&P->GetVelocity().Size2D()<Walk*1.1f,TEXT("Real W input reaches walking speed on a flat collision fixture"));Before=P->GetActorLocation();OxygenBefore=G->Oxygen;TimeBefore=G->GetWorld()->GetTimeSeconds();Key(EKeys::LeftShift,IE_Pressed);
  }else if(Phase==2){
   Check(P->FastStride&&FMath::IsNearlyEqual(Move->MaxWalkSpeed,Fast)&&P->GetVelocity().Size2D()>Fast*.85f,TEXT("Real held Shift increases actual capsule velocity in this mission"));
   Check(FVector::Dist2D(Before,P->GetActorLocation())>Walk*1.2f,TEXT("Fast stride covers more ground than the normal one-second walk"));
   Check(FMath::IsNearlyEqual(OxygenBefore-G->Oxygen,G->GetWorld()->GetTimeSeconds()-TimeBefore,.15f),TEXT("Fast stride retains the same elapsed-second gameplay oxygen formula"));Key(EKeys::LeftShift,IE_Released);
  }else if(Phase==3){
   Check(!P->FastStride&&FMath::IsNearlyEqual(Move->MaxWalkSpeed,Walk)&&P->GetVelocity().Size2D()<Walk*1.1f,TEXT("Releasing Shift restores both normal speed and actual walking velocity"));Key(EKeys::W,IE_Released);Settings->ToggleSprint=true;Key(EKeys::LeftShift,IE_Pressed);
  }else if(Phase==4){
   Check(P->FastStride,TEXT("Configured toggle mode starts fast stride"));Key(EKeys::LeftShift,IE_Released);
   Check(P->FastStride,TEXT("Configured toggle mode retains stride on release"));Key(EKeys::LeftShift,IE_Pressed);
  }else if(Phase==5){
   Check(!P->FastStride,TEXT("Second toggle press restores walking"));Key(EKeys::LeftShift,IE_Released);Settings->ToggleSprint=false;P->BeginFastStride();P->PauseMission();P->UpdateStrideSpeed();OxygenBefore=G->Oxygen;
   Check(!P->FastStride&&FMath::IsNearlyEqual(Move->MaxWalkSpeed,Walk),TEXT("Pause cancels held fast stride immediately"));
  }else if(Phase==6){
   P->BeginFastStride();Check(G->Paused&&!P->FastStride&&G->Oxygen==OxygenBefore,TEXT("Paused mission rejects stride and freezes the gameplay oxygen clock"));P->PauseMission();P->OpenSettings();P->BeginFastStride();Check(P->ActivePanel&&!P->FastStride,TEXT("Settings focus cannot start sprint"));P->ClosePanel();
   if(C){C->Briefed=false;P->BeginFastStride();Check(!P->FastStride,TEXT("Unconfirmed mandatory planning cannot enable sprint"));C->Briefed=true;}
   G->Holding=true;P->BeginFastStride();Check(!P->FastStride,TEXT("Stationary work cannot start fast stride"));G->Holding=false;P->BeginFastStride();Check(P->FastStride,TEXT("Normal active control can sprint again after UI closes"));
   G->Stage=ELunarStage::Briefing;P->UpdateStrideSpeed();Check(!P->FastStride&&FMath::IsNearlyEqual(Move->MaxWalkSpeed,Walk),TEXT("New briefing clears stride state and restores base speed"));Finish();return;
  }
  ++Phase;FTimerHandle H;G->GetWorldTimerManager().SetTimer(H,[Self=AsShared()](){Self->Step();},1.f,false);
 }
};
}
void LunarStrideRegression::Start(ALunarGameMode* G){
 auto Run=MakeShared<FStrideRun>();Run->Game=G;Run->Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(G,0));Run->Controller=UGameplayStatics::GetPlayerController(G,0);Run->Step();
}
