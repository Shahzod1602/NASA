#include "LunarPauseMenu.h"
#include "LunarCargoFinale.h"
#include "LunarGame.h"
#include "LunarDeparture.h"
#include "LunarMissionCard.h"
#include "LunarSettings.h"
#include "LunarLoading.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace {
const FLinearColor Ink(.92f,.96f,1),Dim(.55f,.68f,.76f),Cyan(.03f,.82f,.94f);
ALunarCharacter* PausePilot(const UObject* Context){return Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Context,0));}
ALunarGameMode* PauseGame(const UObject* Context){return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(Context));}
void PlacePause(UCanvasPanel* Canvas,UWidget* Widget,float X,float Y,float Width,float Height){
 auto* Layout=Canvas->AddChildToCanvas(Widget);Layout->SetPosition(FVector2D(X,Y));Layout->SetSize(FVector2D(Width,Height));
}
void FillPause(UOverlay* Overlay,UWidget* Widget){auto* Layout=Overlay->AddChildToOverlay(Widget);Layout->SetHorizontalAlignment(HAlign_Fill);Layout->SetVerticalAlignment(VAlign_Fill);}
FString PauseTime(float Seconds){const int32 Time=FMath::Max(0,FMath::CeilToInt(Seconds));return FString::Printf(TEXT("%02d:%02d"),Time/60,Time%60);}
FString LegacyInstruction(const ALunarGameMode* Game){
 if(Game->PowerSequenceActive)return TEXT("The station restart is in progress. Wait for its power sequence to finish.");
 switch(Game->Stage){
 case ELunarStage::FindModule:return TEXT("Follow the marker to recover the spare power cell beside the survey tower.");
 case ELunarStage::RestorePower:return Game->RepairStep==0?TEXT("Bring the cell to the station's external power port."):Game->RepairStep==1?TEXT("Connect the orange cable to complete the power circuit."):TEXT("Hold the work key at the power port to restart the station.");
 case ELunarStage::CollectData:return Game->Mars?TEXT("Download the subsurface ice survey from the powered science terminal."):TEXT("Read the research terminal before returning to the marked field sites.");
 case ELunarStage::AlignRelay:return TEXT("Reach the orbital relay and align its antenna before returning to the lander.");
 case ELunarStage::SurveySamples:return TEXT("Visit the marked sample sites and seal all three canisters.");
 case ELunarStage::RecoverDrone:return TEXT("Recover NOMAD-7's memory and its emergency gameplay oxygen reserve.");
 case ELunarStage::ActivateBeacon:return TEXT("Reach Earthlink. Lock each channel while its cursor is in the green window.");
 case ELunarStage::ReturnHome:return Game->TrackRecorder&&!Game->RecorderRecovered?TEXT("The crew recorder is optional. Extra travel uses your remaining gameplay oxygen."):TEXT("Return to the lander to complete the mission. The crew recorder is optional.");
 default:return TEXT("Review the mission objective and resume when you are ready.");
 }
}
}

TSharedRef<SWidget> ULunarPauseMenu::RebuildWidget(){
 SetIsFocusable(false);Labels.Reset();LabelSizes.Reset();ActionCards.Reset();
 auto* Root=WidgetTree->ConstructWidget<UOverlay>();WidgetTree->RootWidget=Root;
 WorldBlur=WidgetTree->ConstructWidget<UBackgroundBlur>();WorldBlur->SetPadding(FMargin(0));WorldBlur->SetApplyAlphaToBlur(false);WorldBlur->SetVisibility(ESlateVisibility::HitTestInvisible);
 WorldBlur->SetLowQualityFallbackBrush(FSlateRoundedBoxBrush(FLinearColor(.006f,.014f,.024f,.86f),0.f));FillPause(Root,WorldBlur);
 WorldDim=WidgetTree->ConstructWidget<UBorder>();WorldDim->SetVisibility(ESlateVisibility::HitTestInvisible);FillPause(Root,WorldDim);
 Presentation=WidgetTree->ConstructWidget<UScaleBox>();Presentation->SetStretch(EStretch::ScaleToFit);FillPause(Root,Presentation);
 auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetWidthOverride(1280);Size->SetHeightOverride(720);Presentation->AddChild(Size);
 auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();Size->AddChild(Canvas);
 auto Text=[&](const FString& Value,int32 FontSize,FLinearColor Color){auto* Label=WidgetTree->ConstructWidget<UTextBlock>();auto Font=Label->GetFont();Font.Size=FMath::RoundToInt(FontSize*ULunarSettings::Get()->TextScale);Font.TypefaceFontName=TEXT("Regular");Label->SetFont(Font);Label->SetText(FText::FromString(Value));Label->SetColorAndOpacity(Color);Label->SetAutoWrapText(true);Label->SetVisibility(ESlateVisibility::HitTestInvisible);Labels.Add(Label);LabelSizes.Add(FontSize);return Label;};
 auto Button=[&](const FString& Value){auto* Control=WidgetTree->ConstructWidget<ULunarMissionButton>();auto Style=Control->GetStyle();
  Style.Normal=FSlateRoundedBoxBrush(FLinearColor(.014f,.04f,.06f,.9f),3.f,FLinearColor(.24f,.43f,.53f,.85f),1.f);
  Style.Hovered=FSlateRoundedBoxBrush(FLinearColor(.025f,.19f,.29f,.92f),3.f,Cyan,1.f);
  Style.Pressed=FSlateRoundedBoxBrush(FLinearColor(.015f,.27f,.36f,.96f),3.f,Cyan,1.f);Style.NormalPadding=Style.PressedPadding=FMargin(12,6);Control->SetStyle(Style);Control->AddChild(Text(Value,13,Ink));return Control;};
 PlacePause(Canvas,Text(TEXT("L U N A R R E S C U E"),10,Dim),72,73,1100,24);
 PlacePause(Canvas,Text(TEXT("PAUSED"),36,Ink),70,104,1100,84);
 MissionText=Text(TEXT(""),11,Dim);PlacePause(Canvas,MissionText,73,198,1100,32);
 const TCHAR* Actions[]={TEXT("Resume"),TEXT("Settings"),TEXT("Restart Mission"),TEXT("Back to Main Menu"),TEXT("Exit Game")};
 for(int32 Index=0;Index<5;++Index){auto* Card=CreateWidget<ULunarMissionCard>(GetOwningPlayer());Card->Configure(Index,Actions[Index],TEXT(""),TEXT(""),true);Card->OnChosen.AddDynamic(this,&ULunarPauseMenu::ChooseAction);Card->OnPreview.AddDynamic(this,&ULunarPauseMenu::PreviewAction);PlacePause(Canvas,Card,70,238+66*Index,280,56);ActionCards.Add(Card);}
 auto* ObjectiveScroll=WidgetTree->ConstructWidget<UScrollBox>();ObjectiveScroll->SetAllowOverscroll(false);ObjectiveScroll->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);ObjectiveScroll->SetIsFocusable(false);ObjectiveScroll->SetScrollbarThickness(FVector2D(3,3));PlacePause(Canvas,ObjectiveScroll,394,242,794,230);
 auto* ObjectiveStack=WidgetTree->ConstructWidget<UVerticalBox>();ObjectiveScroll->AddChild(ObjectiveStack);
 ObjectiveStack->AddChildToVerticalBox(Text(TEXT("CURRENT OBJECTIVE"),10,Dim))->SetPadding(FMargin(0,0,20,9));
 ObjectiveText=Text(TEXT(""),21,Ink);ObjectiveStack->AddChildToVerticalBox(ObjectiveText)->SetPadding(FMargin(0,0,20,10));
 ObjectiveDetail=Text(TEXT(""),14,Dim);ObjectiveStack->AddChildToVerticalBox(ObjectiveDetail)->SetPadding(FMargin(0,0,20,25));
 auto* Divider=WidgetTree->ConstructWidget<UBorder>();Divider->SetBrushColor(FLinearColor(.23f,.39f,.47f,.6f));Divider->SetVisibility(ESlateVisibility::HitTestInvisible);auto* DividerSize=WidgetTree->ConstructWidget<USizeBox>();DividerSize->SetHeightOverride(1);DividerSize->AddChild(Divider);PlacePause(Canvas,DividerSize,394,486,774,1);
 ResourceText=Text(TEXT(""),14,Cyan);PlacePause(Canvas,ResourceText,394,506,774,76);
 PlacePause(Canvas,Text(TEXT("Resume to continue your current objective."),11,Dim),394,586,774,34);
 PlacePause(Canvas,Text(TEXT("Movement, work and the mission clock are paused."),11,Dim),73,634,1100,56);

 ConfirmationLayer=WidgetTree->ConstructWidget<UCanvasPanel>();PlacePause(Canvas,ConfirmationLayer,0,0,1280,720);
 auto* Blocker=WidgetTree->ConstructWidget<UBorder>();Blocker->SetBrushColor(FLinearColor(.001f,.004f,.009f,.68f));PlacePause(ConfirmationLayer,Blocker,0,0,1280,720);
 auto* ConfirmFrame=WidgetTree->ConstructWidget<UBorder>();ConfirmFrame->SetBrush(FSlateRoundedBoxBrush(FLinearColor(.008f,.022f,.035f,.98f),5.f,FLinearColor(.08f,.63f,.77f,.9f),1.f));ConfirmFrame->SetVisibility(ESlateVisibility::HitTestInvisible);PlacePause(ConfirmationLayer,ConfirmFrame,275,190,730,340);
 PlacePause(ConfirmationLayer,Text(TEXT("RESTART MISSION?"),22,Ink),305,218,670,56);
 ConfirmationBody=Text(TEXT(""),14,Dim);auto* ConfirmScroll=WidgetTree->ConstructWidget<UScrollBox>();ConfirmScroll->SetAllowOverscroll(false);ConfirmScroll->SetIsFocusable(false);ConfirmScroll->SetScrollbarThickness(FVector2D(3,3));ConfirmScroll->AddChild(ConfirmationBody);PlacePause(ConfirmationLayer,ConfirmScroll,305,284,670,146);
 RestartButton=Button(TEXT("RESTART"));RestartButton->OnClicked.AddDynamic(this,&ULunarPauseMenu::ConfirmRestart);PlacePause(ConfirmationLayer,RestartButton,305,456,315,46);
 CancelButton=Button(TEXT("CANCEL"));CancelButton->OnClicked.AddDynamic(this,&ULunarPauseMenu::CancelRestart);PlacePause(ConfirmationLayer,CancelButton,660,456,315,46);
 ConfirmationLayer->SetVisibility(ESlateVisibility::Collapsed);
 CachedTextScale=ULunarSettings::Get()->TextScale;Sync();return Super::RebuildWidget();
}
void ULunarPauseMenu::NativeConstruct(){Super::NativeConstruct();NotifyOpened();}
void ULunarPauseMenu::NotifyOpened(){OpenElapsed=0;OpenBlend=0;RefreshElapsed=0;FocusedAction=0;ShowConfirmation(false);PreviewAction(0);Sync();if(Presentation){Presentation->SetRenderOpacity(0);Presentation->SetRenderTranslation(FVector2D(-12,0));}if(WorldBlur)WorldBlur->SetBlurStrength(0);if(WorldDim)WorldDim->SetBrushColor(FLinearColor::Transparent);}
void ULunarPauseMenu::NativeTick(const FGeometry& Geometry,float Delta){
 Super::NativeTick(Geometry,Delta);AdvancePresentation(Delta);
 if(FMath::IsFinite(Delta)&&Delta>0)RefreshElapsed+=Delta;
 if(RefreshElapsed>=.2f||!FMath::IsNearlyEqual(CachedTextScale,ULunarSettings::Get()->TextScale,.001f)){RefreshElapsed=0;Sync();}
}
void ULunarPauseMenu::AdvancePresentation(float Delta){
 if(!FMath::IsFinite(Delta)||Delta<=0||OpenBlend>=1)return;
 OpenElapsed=FMath::Min(.24f,OpenElapsed+Delta);const float T=FMath::Clamp(OpenElapsed/.24f,0.f,1.f);OpenBlend=OpenElapsed>=.24f?1.f:T*T*(3-2*T);
 if(Presentation){Presentation->SetRenderOpacity(OpenBlend);Presentation->SetRenderTranslation(FVector2D(-12*(1-OpenBlend),0));}
 if(WorldBlur)WorldBlur->SetBlurStrength(6.f*OpenBlend);
 if(WorldDim)WorldDim->SetBrushColor(FLinearColor(.002f,.009f,.016f,.62f*OpenBlend));
}
void ULunarPauseMenu::Sync(){
 CachedTextScale=ULunarSettings::Get()->TextScale;
 for(int32 Index=0;Index<Labels.Num()&&Index<LabelSizes.Num();++Index)if(Labels[Index]){auto Font=Labels[Index]->GetFont();const int32 Size=FMath::RoundToInt(LabelSizes[Index]*CachedTextScale);if(Font.Size!=Size){Font.Size=Size;Labels[Index]->SetFont(Font);}}
 auto* Game=PauseGame(this);if(!Game)return;
 if(MissionText)MissionText->SetText(FText::FromString(Game->Apollo?(Game->ApolloTimed?TEXT("APOLLO 12 / TIMED EXERCISE"):TEXT("APOLLO 12 / HISTORICAL EXPLORATION")):Game->Mars?TEXT("MARS / RESCUE / EXPEDITION 02"):TEXT("MOON / SELENE RESCUE / EXPEDITION 01")));
 if(ObjectiveText)ObjectiveText->SetText(FText::FromString(Game->Objective()));
 if(ObjectiveDetail)ObjectiveDetail->SetText(FText::FromString(Game->Apollo?Game->ApolloInstruction():LegacyInstruction(Game)));
 if(ResourceText)ResourceText->SetText(FText::FromString(Game->Apollo&&!Game->ApolloTimed?TEXT("Untimed historical exploration"):FString(Game->Apollo?TEXT("Exercise budget remaining  "):TEXT("Oxygen gameplay budget remaining  "))+PauseTime(Game->Oxygen)));
 if(ConfirmationBody)ConfirmationBody->SetText(FText::FromString(Game->Apollo&&Game->ApolloCachedCheckpoint?TEXT("Return to this mission's briefing? Your saved Apollo checkpoint stays available. Starting a new expedition uses its separate confirmation. Current unfinished work will be discarded."):TEXT("Return to this mission's briefing? The current run and unfinished work will be reset.")));
}
bool ULunarPauseMenu::CanAct() const {const auto* Game=PauseGame(this);const auto* Pilot=PausePilot(this);return Game&&Pilot&&Game->Paused&&!Game->Menu&&!Game->Journal&&(Game->IsActive()||ALunarCargoFinale::Active(Game)||ALunarDepartureScene::Active(Game))&&IsInViewport()&&GetVisibility()!=ESlateVisibility::Collapsed&&!Pilot->ActivePanel&&!LunarLoading::IsBusy();}
void ULunarPauseMenu::ShowConfirmation(bool Show){ConfirmingRestart=Show;if(ConfirmationLayer)ConfirmationLayer->SetVisibility(Show?ESlateVisibility::Visible:ESlateVisibility::Collapsed);}
bool ULunarPauseMenu::HandleBack(){if(ConfirmingRestart){CancelRestart();return true;}return false;}
void ULunarPauseMenu::PreviewAction(int32 Index){if(ConfirmingRestart||Index<0||Index>=ActionCards.Num())return;FocusedAction=Index;for(int32 Item=0;Item<ActionCards.Num();++Item)ActionCards[Item]->SetSelected(Item==Index);}
void ULunarPauseMenu::Navigate(int32 Direction){if(CanAct()&&!ConfirmingRestart&&Direction&&ActionCards.Num()>0)PreviewAction((FocusedAction+(Direction>0?1:ActionCards.Num()-1))%ActionCards.Num());}
void ULunarPauseMenu::Activate(){if(CanAct()&&!ConfirmingRestart)ChooseAction(FocusedAction);}
void ULunarPauseMenu::ChooseAction(int32 Index){if(!CanAct()||ConfirmingRestart)return;switch(Index){case 0:Resume();break;case 1:Settings();break;case 2:Restart();break;case 3:MainMenu();break;case 4:ExitGame();break;default:break;}}
void ULunarPauseMenu::Resume(){if(CanAct()&&!ConfirmingRestart)if(auto* Pilot=PausePilot(this))Pilot->PauseMission();}
void ULunarPauseMenu::Restart(){if(CanAct()&&!ConfirmingRestart){Sync();ShowConfirmation(true);}}
void ULunarPauseMenu::ConfirmRestart(){if(CanAct()&&ConfirmingRestart)if(auto* Pilot=PausePilot(this))Pilot->RestartCurrentMission();}
void ULunarPauseMenu::CancelRestart(){if(CanAct()&&ConfirmingRestart)ShowConfirmation(false);}
void ULunarPauseMenu::Settings(){if(CanAct()&&!ConfirmingRestart)if(auto* Pilot=PausePilot(this))Pilot->OpenSettings();}
void ULunarPauseMenu::MainMenu(){if(CanAct()&&!ConfirmingRestart)if(auto* Pilot=PausePilot(this))Pilot->MissionMenu();}
void ULunarPauseMenu::ExitGame(){if(CanAct()&&!ConfirmingRestart)UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
