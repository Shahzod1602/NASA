#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarPauseMenu.h"
#include "LunarFlightHUD.h"
#include "LunarLoading.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "LunarCargoFinale.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void ALunarGameMode::BuildMissionInterface(){
 if(Menu||FlightHUD)return;
 if(auto* PC=UGameplayStatics::GetPlayerController(this,0)){
  FlightHUD=CreateWidget<ULunarFlightHUD>(PC);FlightHUD->AddToViewport(5);FlightHUD->Refresh();
 }
}
void ALunarCharacter::SyncPauseMenu(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* PC=Cast<APlayerController>(GetController());
 if(!G||!PC||G->Menu)return;
 const bool PausedScreen=G->Paused&&!G->Journal&&!G->ApolloConfirmNew&&(G->IsActive()||ALunarCargoFinale::Active(G))&&!LunarLoading::IsBusy();
 const bool Show=PausedScreen&&!ActivePanel;
 if(PausedScreen&&!PauseMenu){PauseMenu=CreateWidget<ULunarPauseMenu>(PC);PauseMenu->AddToViewport(60);PauseMenu->SetVisibility(ESlateVisibility::Collapsed);}
 if(!PauseMenu)return;
 const bool WasVisible=PauseMenu->GetVisibility()!=ESlateVisibility::Collapsed;
 if(Show&&(!WasVisible||!PC->bShowMouseCursor)){
  PauseMenu->SetVisibility(ESlateVisibility::Visible);if(!WasVisible)PauseMenu->NotifyOpened();
  FInputModeGameAndUI Input;Input.SetHideCursorDuringCapture(false);PC->SetInputMode(Input);PC->bShowMouseCursor=true;
 }else if(!Show&&WasVisible){
  PauseMenu->SetVisibility(ESlateVisibility::Collapsed);
  if(!ActivePanel){PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;}
 }
}
void ALunarCharacter::RestartCurrentMission(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));
 if(!G||G->Menu||!G->Paused||ActivePanel||LunarLoading::IsBusy())return;
 // Re-enter the current mission briefing. On-disk checkpoints remain available;
 // this is distinct from R's paused Apollo checkpoint reload.
 // Use the supported mission package, independent of transient world names.
 const FName MissionMap(G->Apollo?TEXT("/Game/Lunar/Maps/Apollo12"):G->Mars?TEXT("/Game/Mars/Maps/MarsBase"):TEXT("/Game/Lunar/Maps/MoonBase"));
 LunarLoading::Open(this,MissionMap);
}
