#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarLoading.h"
#include "LunarMissionCard.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include <limits>
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

// Keep the state across local OpenLevel calls so the same input path is checked
// before travel and the actual destination is checked in the next world.
void ALunarGameMode::TravelCheck(){
 static int32 Phase=0,Failures=0;
 static FString Report;
 const bool LoadingTest=FParse::Param(FCommandLine::Get(),TEXT("LoadingTest"));
 const TCHAR* Normal[]={TEXT("MissionSelect"),TEXT("Apollo12"),TEXT("Apollo12"),TEXT("MissionSelect"),TEXT("MoonBase"),TEXT("MoonBase"),TEXT("MissionSelect"),TEXT("MarsBase"),TEXT("MarsBase"),TEXT("MissionSelect")};
 const TCHAR* Loading[]={TEXT("MissionSelect"),TEXT("Apollo12"),TEXT("Apollo12"),TEXT("MissionSelect"),TEXT("Apollo12"),TEXT("MissionSelect"),TEXT("MoonBase"),TEXT("MoonBase"),TEXT("MissionSelect"),TEXT("MarsBase"),TEXT("MarsBase"),TEXT("MissionSelect")};
 const int32 Last=LoadingTest?UE_ARRAY_COUNT(Loading)-1:UE_ARRAY_COUNT(Normal)-1;
 const FString Map=UGameplayStatics::GetCurrentLevelName(this,true);
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto Check=[&](bool Good,const FString& Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failures;};
 auto Finish=[&](){
  Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);
  const FString Folder=FPaths::ProjectDir()/TEXT("Reports");IFileManager::Get().MakeDirectory(*Folder,true);
  FFileHelper::SaveStringToFile(Report,*(Folder/(LoadingTest?TEXT("loading-checks.txt"):TEXT("menu-travel-checks.txt"))));
  FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
 };
 Check(Phase>=0&&Phase<=Last,TEXT("Travel phase is in the expected sequence"));
 if(Failures){Finish();return;}
 Check(P&&Map==(LoadingTest?Loading[Phase]:Normal[Phase])&&(Menu||FMath::IsNearlyEqual(GetWorld()->GetGravityZ(),-SurfaceGravity,.1f)),FString::Printf(TEXT("travel phase %d: %s"),Phase,*Map));
 if(LoadingTest){
  if(Phase>0)Check(LunarLoading::Completed(Map,FParse::Param(FCommandLine::Get(),TEXT("LoadingRenderedTest"))),TEXT("Matching destination completed; rendered progress has visible 0/100, monotonic order and held world time"));
  if(Phase==0)Check(!LunarLoading::Open(this,TEXT("/Game/Missing/NoSuchMission"))&&!LunarLoading::IsBusy(),TEXT("Missing map is rejected without trapping loading state"));
  if(Phase==4)Check(ApolloStarted&&Stage==ELunarStage::ApolloCheckout&&ApolloElapsed>=17&&ApolloElapsed<25,TEXT("Continue keeps ResumeApollo and restores the isolated checkpoint"));
 }
 if(Failures||Phase==Last){Finish();return;}

 // Choosing a destination must not start a mission. Only a valid selected card
 // followed by explicit launch may enqueue local travel.
 auto SelectAndLaunch=[&](bool ToMars,int32 MissionIndex,bool ConfirmWithEnter){
  Check(MainMenu!=nullptr,TEXT("Mission selection has a live frontend"));
  if(!MainMenu||!P)return;
  const ELunarStage BeforeStage=Stage;const float BeforeOxygen=Oxygen;
  if(ToMars)P->ChooseMars();else P->ChooseMoon();
  Check(Menu&&Map==TEXT("MissionSelect")&&MainMenu->MissionSelection&&MainMenu->SelectionMars==ToMars&&MainMenu->SelectedMission==-1&&!LunarLoading::IsBusy(),ToMars?TEXT("Mars shortcut opens its picker without travel"):TEXT("Moon shortcut opens its picker without travel"));
  MainMenu->LaunchMission();MainMenu->ConfirmSelection();P->PrimaryClick();
  Check(!LunarLoading::IsBusy()&&MainMenu->SelectedMission==-1,TEXT("Unselected picker cannot launch through button, Enter or background click"));
  MainMenu->SelectMission(-1);MainMenu->SelectMission(3);MainMenu->SelectMission(ToMars?0:2);
  Check(MainMenu->SelectedMission==-1&&!LunarLoading::IsBusy(),TEXT("Invalid and other-destination mission indices are rejected"));
  Check(Stage==BeforeStage&&Oxygen==BeforeOxygen,TEXT("Browsing and rejected launch leave mission state and resource unchanged"));
  MainMenu->SelectMission(MissionIndex);
  Check(MainMenu->SelectedMission==MissionIndex&&!LunarLoading::IsBusy(),TEXT("Valid mission selection stays on the picker until confirmation"));
  if(Failures)return;
  ++Phase;
  if(ConfirmWithEnter)MainMenu->ConfirmSelection();else MainMenu->LaunchMission();
  Check(LunarLoading::IsBusy(),TEXT("Explicit selected-mission confirmation queues loading"));
  Check(!LunarLoading::Open(this,ToMars?TEXT("/Game/Lunar/Maps/Apollo12"):TEXT("/Game/Mars/Maps/MarsBase")),TEXT("Duplicate input cannot replace the selected destination"));
 };

 if(Phase==0){
  Check(MainMenu!=nullptr,TEXT("Home menu owns its mission-selection widget"));
  if(MainMenu&&P){
   Check(!MainMenu->MissionSelection&&!LunarLoading::IsBusy(),TEXT("Initial home page has not begun loading"));
   P->StartMission();
   Check(MainMenu->MissionSelection&&!MainMenu->SelectionMars&&MainMenu->SelectedMission==-1&&!LunarLoading::IsBusy(),TEXT("Enter from home opens Moon selection without starting its default mission"));
   MainMenu->Back();
   Check(!MainMenu->MissionSelection&&!LunarLoading::IsBusy(),TEXT("Back returns to home without mission travel"));
   SelectPlanet(true);
   Check(MainMenu->MissionSelection&&MainMenu->SelectionMars&&MainMenu->SelectedMission==-1&&!LunarLoading::IsBusy(),TEXT("Game mode planet selection opens the Mars picker without travel"));
   MainMenu->Back();
  }
  if(!Failures)SelectAndLaunch(false,0,true);
 }else if((!LoadingTest&&(Phase==3||Phase==6))||(LoadingTest&&(Phase==5||Phase==8))){
  const bool ToMars=LoadingTest?Phase==8:Phase==6;
  SelectAndLaunch(ToMars,ToMars?2:1,false);
 }else if(LoadingTest&&Phase==3){
  Check(ApolloCachedCheckpoint!=nullptr,TEXT("Menu retains the isolated resumable Apollo checkpoint"));
  if(!Failures){++Phase;ContinueApollo();Check(LunarLoading::IsBusy(),TEXT("Continue queues saved Apollo loading independently of mission selection"));}
 }else{
  const bool Restart=LoadingTest?(Phase==1||Phase==6||Phase==9):(Phase==1||Phase==4||Phase==7);
  if(Restart){
   Stage=ELunarStage::Won;++Phase;P->RestartMission();
   Check(LunarLoading::IsBusy(),TEXT("Mission restart queues the current destination"));
  }else{
   if(LoadingTest&&Phase==2){StartApollo(true);ApolloElapsed=17;Check(SaveApolloCheckpoint(),TEXT("Loading fixture saves committed Apollo state in its isolated profile"));}
   if(!Failures){Paused=true;++Phase;P->MissionMenu();Check(LunarLoading::IsBusy(),TEXT("Return to mission menu queues local loading"));}
  }
 }
 // These assertions run before OpenLevel is processed. Finalize immediately on
 // any failure instead of letting a later successful world hide the result.
 if(Failures)Finish();
}

void ALunarGameMode::MissionSelectionChecks(){
 FString Report;int32 Failures=0;
 auto Check=[&](bool Good,const FString& Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failures;};
 auto Finish=[&](){
  Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);
  const FString Folder=FPaths::ProjectDir()/TEXT("Reports");IFileManager::Get().MakeDirectory(*Folder,true);
  FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("mission-selection-checks.txt")));
  FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
 };
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* Main=MainMenu.Get();
 Check(Menu&&P&&Main,TEXT("Mission-selection fixture starts in the menu with its player and frontend"));
 if(!P||!Main||!Menu){Finish();return;}
 Check(Main->HomeCards.Num()==3&&Main->MissionCards.Num()==3&&Main->HomePage&&Main->SelectionPage&&Main->LaunchButton,TEXT("Frontend builds three shared expedition entries and three real mission cards"));
 if(Main->HomeCards.Num()!=3||Main->MissionCards.Num()!=3||!Main->HomePage||!Main->SelectionPage||!Main->LaunchButton){Finish();return;}
 Check(Main->MoonBackdrop&&Main->MarsBackdrop&&Main->DestinationTextureLoads==2,TEXT("Destination textures are retained once before destination input"));
 const int32 BuildsBefore=Main->FrontendBuilds,LoadsBefore=Main->DestinationTextureLoads;
 Main->PreviewMoon();const int32 MoonUpdates=Main->DestinationPresentationUpdates;
 for(int32 I=0;I<20;++I){Main->HomeCards[1]->Hover();Main->HomeCards[1]->Unhover();Main->PreviewMoon();}
 Check(Main->DestinationPresentationUpdates==MoonUpdates,TEXT("Repeated same-destination hover does not reset text or background brushes"));
 Main->PreviewMars();const int32 MarsUpdates=Main->DestinationPresentationUpdates;
 for(int32 I=0;I<20;++I){Main->HomeCards[2]->Hover();Main->HomeCards[2]->Unhover();Main->PreviewMars();}
 Main->PreviewMoon();
 Check(Main->DestinationPresentationUpdates==MarsUpdates+1&&Main->DestinationTextureLoads==LoadsBefore&&Main->FrontendBuilds==BuildsBefore,TEXT("Repeated hover and destination switching reuse textures and the existing widget tree"));
 for(UButton* Button:{Main->LaunchButton.Get(),Main->SelectionBackButton.Get(),Main->SelectionSettingsButton.Get()}){
  auto* Action=Cast<ULunarFrontendButton>(Button);FString Reason;
  Check(Action&&Action->ValidateSurface(false,Reason),TEXT("Picker action has full-size glass and centred label slots"));
  if(Action){const auto Normal=Action->GetStyle().Normal;Action->OnReceivedFocus.ExecuteIfBound();Check(Action->GetStyle().Normal.TintColor==Action->GetStyle().Hovered.TintColor,TEXT("Keyboard focus uses the same visible blue action state as hover"));Action->OnLostFocus.ExecuteIfBound();Check(Action->GetStyle().Normal.TintColor==Normal.TintColor,TEXT("Leaving keyboard focus restores the normal action surface"));}
 }
 bool Built=true;
 for(int32 I=0;I<3;++I){
  const bool Ready=Main->HomeCards[I]&&Main->HomeCards[I]->Button&&Main->MissionCards[I]&&Main->MissionCards[I]->Button&&Main->MissionCards[I]->SelectionLabel;
  Check(Ready,FString::Printf(TEXT("Expedition entry and mission card %d have their bound UMG controls"),I));Built=Built&&Ready;
 }
 if(!Built){Finish();return;}
 const FVector InitialPosition=P->GetActorLocation();const ELunarStage InitialStage=Stage;const float InitialOxygen=Oxygen;const bool InitialPause=Paused;
 auto Idle=[&](){return Menu&&UGameplayStatics::GetCurrentLevelName(this,true)==TEXT("MissionSelect")&&!LunarLoading::IsBusy();};
 auto Visible=[&](int32 Index){return Main->MissionCards[Index]->GetVisibility()!=ESlateVisibility::Collapsed;};
 auto OpenByClick=[&](bool Mars){Main->Back();Main->HomeCards[Mars?2:1]->Button->OnClicked.Broadcast();};

 Check(!Main->MissionSelection&&Idle(),TEXT("Home begins without a selected mission or pending travel"));
 OpenByClick(false);
 Check(Main->MissionSelection&&!Main->SelectionMars&&Idle(),TEXT("Explore Moon UMG click opens the lunar mission-selection page without travel"));
 Check(Main->HomePage->GetVisibility()==ESlateVisibility::Collapsed&&Main->SelectionPage->GetVisibility()!=ESlateVisibility::Collapsed,TEXT("Destination click changes the visible frontend page"));
 Check(Visible(0)&&Visible(1)&&!Visible(2),TEXT("Moon picker exposes Apollo 12 and SELENE and hides ARES"));
 Check(Main->SelectedMission==INDEX_NONE&&!Main->LaunchButton->GetIsEnabled(),TEXT("Mission launch remains disabled before a card is selected"));
 Main->LaunchButton->OnClicked.Broadcast();P->StartMission();P->PrimaryClick();
 Check(Idle()&&Main->SelectedMission==INDEX_NONE,TEXT("Disabled launch, Enter and background click cannot bypass mission selection"));
 Main->SelectMission(-1);Main->SelectMission(3);Main->MissionCards[2]->Button->OnClicked.Broadcast();
 Check(Main->SelectedMission==INDEX_NONE&&!Main->LaunchButton->GetIsEnabled()&&Idle(),TEXT("Invalid indices and a hidden other-planet mission cannot be selected"));

 // Exercise each real card through its actual UMG click delegate. Animation is
 // advanced deterministically here; rendered review separately checks the look.
 for(int32 I=0;I<3;++I){
  OpenByClick(I==2);auto* Card=Main->MissionCards[I].Get();
  for(const auto& Entry:Main->MissionCards){Entry->SetSelected(false);Entry->AdvancePresentation(.28f);}
  Check(Card->GetClass()==Main->MissionCards[0]->GetClass()&&Card->GetClass()==Main->HomeCards[0]->GetClass(),FString::Printf(TEXT("Mission %d and expedition entries share one presentation class"),I));
  Card->Button->OnClicked.Broadcast();
  Check(Main->SelectedMission==I&&Card->IsSelected()&&Main->LaunchButton->GetIsEnabled()&&Idle(),FString::Printf(TEXT("Mission %d UMG click selects a card without starting it"),I));
  Check(Card->SelectionLabel->GetText().ToString()==TEXT("SELECTED"),FString::Printf(TEXT("Mission %d exposes a persistent textual selection cue"),I));
  bool Monotonic=FMath::IsNearlyZero(Card->SelectionBlend());float Before=Card->SelectionBlend();
  for(int32 Step=0;Step<4;++Step){
   Card->AdvancePresentation(.07f);const float Value=Card->SelectionBlend();
   Monotonic=Monotonic&&FMath::IsFinite(Value)&&Value>Before&&Value>=0&&Value<=1&&(Step==3||Value<1);Before=Value;
  }
  Check(Monotonic&&FMath::IsNearlyEqual(Card->SelectionBlend(),1.f),FString::Printf(TEXT("Mission %d selection smoothly rises through four bounded samples and settles within 280 ms"),I));
  Card->Button->OnHovered.Broadcast();Card->Button->OnUnhovered.Broadcast();Card->AdvancePresentation(1.f);
  Check(Card->IsSelected()&&FMath::IsNearlyEqual(Card->SelectionBlend(),1.f)&&Main->SelectedMission==I,FString::Printf(TEXT("Mission %d remains highlighted after hover exit and settling"),I));
  const int32 IdlePresentationUpdates=Card->PresentationUpdateCount();
  for(int32 IdleFrame=0;IdleFrame<120;++IdleFrame)Card->AdvancePresentation(1.f/60.f);
  Check(Card->PresentationUpdateCount()==IdlePresentationUpdates,FString::Printf(TEXT("Settled mission %d does not rebuild glow or blur brushes on 120 idle frames"),I));
  const float Stable=Card->SelectionBlend();Card->AdvancePresentation(-1);Card->AdvancePresentation(0);Card->AdvancePresentation(std::numeric_limits<float>::quiet_NaN());Card->AdvancePresentation(std::numeric_limits<float>::infinity());
  Check(Card->SelectionBlend()==Stable&&FMath::IsFinite(Card->SelectionBlend()),FString::Printf(TEXT("Mission %d ignores zero, negative and non-finite animation deltas"),I));
  Card->SetSelected(false);Card->AdvancePresentation(.07f);const float Reversed=Card->SelectionBlend();
  Card->SetSelected(true);Card->AdvancePresentation(.07f);const float Reselected=Card->SelectionBlend();Card->AdvancePresentation(1.f);
  Check(Reversed>0&&Reversed<Stable&&Reselected>=Reversed&&Reselected<=1&&FMath::IsFinite(Reselected)&&FMath::IsNearlyEqual(Card->SelectionBlend(),1.f),FString::Printf(TEXT("Mission %d reverses and reselects without a jump outside the blend range"),I));
 }

 OpenByClick(false);Main->MissionCards[0]->Button->OnClicked.Broadcast();
 Main->MissionCards[1]->Button->OnHovered.Broadcast();Main->MissionCards[1]->Button->OnUnhovered.Broadcast();
 Check(Main->SelectedMission==0&&Main->MissionCards[0]->IsSelected()&&!Main->MissionCards[1]->IsSelected(),TEXT("Hovering another mission never replaces the committed selection"));
 Main->MissionCards[1]->Button->OnClicked.Broadcast();
 Check(Main->SelectedMission==1&&!Main->MissionCards[0]->IsSelected()&&Main->MissionCards[1]->IsSelected()&&!Main->MissionCards[2]->IsSelected(),TEXT("Selecting another card clears the previous selection exclusively"));
 Main->SelectMission(2);Main->SelectMission(-1);Main->SelectMission(99);
 Check(Main->SelectedMission==1&&Main->MissionCards[1]->IsSelected()&&Idle(),TEXT("Rejected selections preserve an existing valid chosen mission"));
 P->PrimaryClick();Check(Main->SelectedMission==1&&Idle(),TEXT("Background gameplay click cannot launch a selected mission"));
 P->PauseMission();Check(!Main->MissionSelection&&Idle(),TEXT("Escape from mission selection returns to home without loading"));
 P->StartMission();Check(Main->MissionSelection&&!Main->SelectionMars&&Main->SelectedMission==INDEX_NONE&&Idle(),TEXT("Enter from home opens the picker without automatically choosing a mission"));
 P->NextMenuMission();Check(Main->SelectedMission==0&&Idle(),TEXT("Right selects the first lunar mission without starting it"));
 P->NextMenuMission();Check(Main->SelectedMission==1&&Idle(),TEXT("Right advances to SELENE without starting it"));
 P->PreviousMenuMission();Check(Main->SelectedMission==0&&Idle(),TEXT("Left returns to Apollo 12 without starting it"));
 P->PreviousMenuMission();Check(Main->SelectedMission==1&&Idle(),TEXT("Left wraps between the available lunar missions"));
 OpenByClick(true);
 Check(Main->MissionSelection&&Main->SelectionMars&&!Visible(0)&&!Visible(1)&&Visible(2)&&Idle(),TEXT("Explore Mars UMG click exposes only ARES without travel"));
 Main->MissionCards[0]->Button->OnClicked.Broadcast();Check(Main->SelectedMission==INDEX_NONE, TEXT("Hidden lunar card cannot select from the Mars picker"));
 P->NextMenuMission();P->PreviousMenuMission();Check(Main->SelectedMission==2&&Main->MissionCards[2]->IsSelected()&&Idle(),TEXT("Both arrow directions retain the only available ARES mission without travel"));

 OpenByClick(false);Main->MissionCards[0]->Button->OnClicked.Broadcast();P->OpenSettings();
 Check(P->ActivePanel&&P->ActivePanel->Settings,TEXT("Settings can overlay the mission-selection page"));
 if(P->ActivePanel){
  Main->MissionCards[1]->Button->OnClicked.Broadcast();Main->SelectMission(1);Main->LaunchButton->OnClicked.Broadcast();Main->ConfirmSelection();
  Main->HomeCards[2]->Button->OnClicked.Broadcast();Main->ExploreMars();P->ChooseMars();P->ChooseMoon();P->StartMission();P->NextMenuMission();P->PreviousMenuMission();
  Check(P->ActivePanel&&Main->MissionSelection&&!Main->SelectionMars&&Main->SelectedMission==0&&Main->MissionCards[0]->IsSelected()&&Idle(),TEXT("Settings blocks card selection, launch, destination shortcuts, arrows and Enter underneath it"));
  P->PauseMission();
  Check(!P->ActivePanel&&Main->MissionSelection&&Main->SelectedMission==0&&Main->MissionCards[0]->IsSelected()&&Idle(),TEXT("Escape closes settings while preserving the chosen mission"));
 }
 Main->Back();
 Check(!Main->MissionSelection&&!P->ActivePanel&&Idle(),TEXT("Selection checks finish on the clean home page without pending loading"));
 Check(Stage==InitialStage&&Oxygen==InitialOxygen&&Paused==InitialPause&&P->GetActorLocation().Equals(InitialPosition,.001f),TEXT("Browsing, animation and settings leave mission state, resource, pause and player position unchanged"));
 Finish();
}
