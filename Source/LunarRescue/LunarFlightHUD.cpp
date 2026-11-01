#include "LunarFlightHUD.h"
#include "LunarGame.h"
#include "LunarApolloText.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"
#include "UnrealClient.h"

namespace {
 const FLinearColor Ink(.93f,.97f,.98f),Dim(.64f,.73f,.78f),Cyan(.05f,.88f,.96f),Gold(.99f,.77f,.39f);
 int32 Bits(uint8 Value){int32 N=0;for(;Value;Value>>=1)N+=Value&1;return N;}
 float Safe(float Value){return FMath::IsFinite(Value)?FMath::Max(0.f,Value):0.f;}
 FString Clock(float Value){const int32 Sec=FMath::CeilToInt(FMath::Clamp(Safe(Value),0.f,359999.f));return FString::Printf(TEXT("%02d:%02d"),Sec/60,Sec%60);}
 FString Key(const TCHAR* Action){return ULunarSettings::Get()->Key(Action).GetDisplayName().ToString();}
 bool Set(UTextBlock* Label,const FString& Value){if(!Label||Label->GetText().ToString()==Value)return false;Label->SetText(FText::FromString(Value));return true;}
 bool Show(UWidget* Widget,bool Visible){if(!Widget)return false;const auto V=Visible?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed;if(Widget->GetVisibility()==V)return false;Widget->SetVisibility(V);return true;}
 UCanvasPanelSlot* Place(UCanvasPanel* Canvas,UWidget* Widget,float X,float Y,float W,float H){auto* Slot=Canvas->AddChildToCanvas(Widget);Slot->SetPosition(FVector2D(X,Y));Slot->SetSize(FVector2D(W,H));return Slot;}
 UWidget* Glass(UWidgetTree* Tree,UWidget* Child,TArray<TObjectPtr<UBorder>>& Faces){
  auto* Overlay=Tree->ConstructWidget<UOverlay>();Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
  auto* Blur=Tree->ConstructWidget<UBackgroundBlur>();Blur->SetBlurStrength(3.5f);Blur->SetCornerRadius(FVector4(6,6,6,6));
  FSlateBrush Fallback;Fallback.TintColor=FSlateColor(FLinearColor(.025f,.065f,.095f,.96f));Blur->SetLowQualityFallbackBrush(Fallback);Overlay->AddChild(Blur);
  auto* Face=Tree->ConstructWidget<UBorder>();
  FSlateRoundedBoxBrush Brush(FLinearColor::White,6.f,FLinearColor(.15f,.42f,.51f,.55f),1.f);
  Face->SetBrush(Brush);Face->SetBrushColor(FLinearColor(.025f,.075f,.11f,.80f));Face->SetPadding(FMargin(15,12));Face->SetContent(Child);Overlay->AddChild(Face);Faces.Add(Face);return Overlay;
 }
 FString LegacyName(const ALunarGameMode* G){
  switch(G->Stage){case ELunarStage::FindModule:return TEXT("POWER MODULE");case ELunarStage::RestorePower:return G->Mars?TEXT("ARES RELAY"):TEXT("SELENE STATION");
  case ELunarStage::CollectData:return TEXT("SCIENCE TERMINAL");case ELunarStage::AlignRelay:return TEXT("ORBITAL ANTENNA");case ELunarStage::SurveySamples:return TEXT("LUNAR SAMPLE");
  case ELunarStage::RecoverDrone:return TEXT("SURVEY DRONE");case ELunarStage::ActivateBeacon:return TEXT("RESCUE BEACON");default:return G->TrackRecorder&&!G->RecorderRecovered?TEXT("CREW RECORDER"):TEXT("LANDER");}
 }
}

bool ULunarFlightHUD::ShouldShow(const ALunarGameMode* G,const ALunarCharacter* P){
 return G&&P&&!G->Menu&&G->IsActive()&&!G->Paused&&!G->Journal&&!P->ActivePanel&&P->PhotoCaptureFrames<=0&&!(FScreenshotRequest::IsScreenshotRequested()&&!FScreenshotRequest::ShouldShowUI());
}
float ULunarFlightHUD::HoldFraction(const ALunarGameMode* G){
 if(!G||!G->Holding)return 0;const float Duration=G->HoldDuration();
 return FMath::IsFinite(Duration)&&Duration>UE_SMALL_NUMBER&&FMath::IsFinite(G->HoldProgress)?FMath::Clamp(G->HoldProgress/Duration,0.f,1.f):0.f;
}
FLunarFlightHUDState ULunarFlightHUD::ReadState(const ALunarGameMode* G,const ALunarCharacter* P){
 FLunarFlightHUDState R;if(!G)return R;const auto* S=ULunarSettings::Get();
 R.Visible=ShouldShow(G,P);R.Apollo=G->Apollo;R.Tool=G->Apollo?G->ApolloTool:0;R.WorkKey=Key(TEXT("Interact"));R.RouteKey=Key(TEXT("Route"));R.JournalKey=Key(TEXT("Journal"));
 R.MoveKeys=Key(TEXT("MoveForward"))+TEXT(" ")+Key(TEXT("MoveLeft"))+TEXT(" ")+Key(TEXT("MoveBack"))+TEXT(" ")+Key(TEXT("MoveRight"));R.Crosshair=S->Crosshair;
 R.Expedition=G->Mars?TEXT("EXPEDITION 02 / MARS"):TEXT("EXPEDITION 01 / MOON");R.Mission=G->Apollo?TEXT("APOLLO 12"):G->Mars?TEXT("ARES / RESCUE"):TEXT("SELENE / RESCUE");
 R.BoundedResource=!G->Apollo||G->ApolloTimed;R.ResourceLabel=G->Apollo?(G->ApolloTimed?TEXT("EXERCISE BUDGET"):TEXT("EXPLORATION TIME")):TEXT("GAMEPLAY OXYGEN");
 R.ResourceTime=Clock(R.BoundedResource?G->Oxygen:G->ApolloElapsed);R.ResourceFraction=FMath::IsFinite(G->OxygenCapacity)&&G->OxygenCapacity>UE_SMALL_NUMBER?FMath::Clamp(Safe(G->Oxygen)/G->OxygenCapacity,0.f,1.f):0.f;
 R.ResourceWarning=R.BoundedResource&&Safe(G->Oxygen)<(G->Apollo?180.f:60.f);R.Warning=R.ResourceWarning?TEXT("LOW RESOURCE / RETURN TO LANDER"):R.BoundedResource?TEXT("Simplified mission resource"):TEXT("Untimed / explore at your own pace");
 R.Gravity=FString::Printf(TEXT("SURFACE GRAVITY  %.2f m/s²"),Safe(G->SurfaceGravity)/100.f);
 const float Duration=G->HoldDuration();R.Holding=G->Holding&&FMath::IsFinite(Duration)&&Duration>UE_SMALL_NUMBER;R.HoldFraction=HoldFraction(G);
 R.ShowGuide=G->Apollo?(G->ApolloGuideEnabled||R.Tool!=0):S->TutorialHints;R.GuideCaption=TEXT("FIELD GUIDE");
 if(G->Apollo){
  const int32 Step=FMath::Clamp(G->ApolloStep(),0,5);R.Stage=FString::Printf(TEXT("STAGE %d / 6"),Step+1);
  R.Objective=G->ApolloObjective();R.GuideTitle=G->ApolloInstruction();
  const FString Tutorial=G->ApolloTutorial(),Hint=G->ApolloInputHint();R.GuideBody=!Hint.IsEmpty()?Hint:!Tutorial.IsEmpty()?Tutorial:G->ApolloPurpose();
  R.GuideAction=G->ApolloPrompt();R.Total=1;
  const auto* Focus=G->ApolloGuidanceTarget();const int32 FocusSample=G->SampleActors.IndexOfByPredicate([Focus](const auto& A){return A.Get()==Focus;});
  switch(G->Stage){
   case ELunarStage::ApolloCheckout:R.Mission=TEXT("EQUIPMENT CHECK");R.Completed=FMath::Clamp(G->ApolloAction,0,3);R.Total=3;R.Subtask=TEXT("Check the suit and record your equipment.");break;
   case ELunarStage::ApolloDeploy:R.Mission=TEXT("SCIENCE DEPLOYMENT");R.Completed=Bits(G->ApolloInstruments&3);R.Total=2;R.Subtask=TEXT("Place, level and align the instruments.");break;
   case ELunarStage::ApolloSamples:R.Mission=TEXT("LUNAR SAMPLES");R.Completed=G->SampleCount();R.Total=3;R.Subtask=TEXT("Photograph each site before collecting its sample.");
    if(FocusSample>=0&&FocusSample<3){const bool Photo=(G->ApolloPhotos&(1<<FocusSample))!=0;R.Objective=Photo?TEXT("Collect the photographed sample"):TEXT("Photograph the sample");R.Subtask=Photo?TEXT("Collect and seal the sample using its work tool."):TEXT("Then collect and seal the photographed sample.");}break;
   case ELunarStage::ApolloTraverse:R.Mission=TEXT("SURVEYOR TRAVERSE");R.Subtask=TEXT("Follow the terrain toward Surveyor 3.");R.Completed=0;break;
   case ELunarStage::ApolloRecover:R.Mission=TEXT("SURVEYOR HARDWARE");R.Completed=Bits(G->ApolloParts&7);R.Total=3;R.Subtask=G->SurveyorPhotographed?TEXT("Recover and pack the marked hardware."):TEXT("Record the Surveyor context photo first.");break;
   case ELunarStage::ApolloReturn:R.Mission=TEXT("RETURN TO INTREPID");R.Completed=0;R.Subtask=TEXT("Bring your samples and hardware to the lander.");break;
   default:break;
  }
  R.Counter=FString::Printf(TEXT("%s  %d / %d"),G->Stage==ELunarStage::ApolloSamples?TEXT("SAMPLES COLLECTED"):G->Stage==ELunarStage::ApolloRecover?TEXT("HARDWARE PACKED"):TEXT("TASKS COMPLETE"),R.Completed,R.Total);
  if(G->Stage==ELunarStage::ApolloSamples)R.Counter+=FString::Printf(TEXT("  /  PHOTOS %d / 3"),Bits(G->ApolloPhotos&7));
  if(G->ApolloDustMode){R.Mission=TEXT("OPTIONAL / DUST INVESTIGATION");R.Completed=Bits(G->ApolloDustPhotos&3);R.Total=2;R.Counter=FString::Printf(TEXT("COATING PHOTOS %d / 2"),R.Completed);R.Subtask=G->ApolloDustCompared?TEXT("Explanation recorded in your evidence journal."):TEXT("Compare exposed and protected coating.");}
  if(G->Stage==ELunarStage::ApolloSamples||(G->Stage==ELunarStage::ApolloRecover&&G->SurveyorPhotographed))R.RouteHint=TEXT("Next site");if(R.Tool==9)R.RouteHint=TEXT("Choose answer");
  if(R.Tool){R.GuideCaption=TEXT("WORK TOOL / CLICK TO PERFORM EACH STEP");R.GuideTitle=G->ApolloToolTitle();R.GuideBody=G->ApolloToolHelp();R.GuideAction=G->ApolloToolProgress()+TEXT("  /  Left click: perform step  /  Esc: pause and cancel unfinished work");
   if(G->ApolloToolFeedbackRemaining>0&&!G->ApolloToolFeedback.IsEmpty())R.GuideAction+=TEXT("\nIN PROGRESS: ")+G->ApolloToolFeedback;}
  R.PhotoFrame=R.Tool==1;R.Level=G->ApolloLevel;R.DustAnswers=R.Tool==9&&!G->ApolloDustCompared;R.DustChoice=FMath::Clamp(G->ApolloDustChoice,0,2);
  R.AntennaSignal=1-FMath::Clamp(FMath::Abs(FMath::FindDeltaAngleDegrees(G->ApolloAntennaAngle,-7.f))/90.f,0.f,1.f);
  if(G->ApolloPhotoPending)R.Notice=TEXT("PHOTO CAPTURE / waiting for the image to be saved");
  else if(G->ApolloSaveError&&G->ApolloSaveNoticeRemaining>0)R.Notice=ApolloText::Get(*G->ApolloSaveNotice);
  else if(G->ApolloConfirmationRemaining>0&&!G->ApolloConfirmation.IsEmpty())R.Notice=G->ApolloConfirmationLabel+TEXT(": ")+G->ApolloConfirmation;
  else if(G->ApolloSaveNoticeRemaining>0&&!G->ApolloSaveNotice.IsEmpty())R.Notice=ApolloText::Get(*G->ApolloSaveNotice);
 }else{
  int32 Step=1;switch(G->Stage){case ELunarStage::RestorePower:Step=2;break;case ELunarStage::CollectData:Step=3;break;case ELunarStage::AlignRelay:Step=4;break;case ELunarStage::ReturnHome:Step=G->Mars?5:7;break;case ELunarStage::SurveySamples:Step=4;break;case ELunarStage::RecoverDrone:Step=5;break;case ELunarStage::ActivateBeacon:Step=6;break;default:break;}
  R.Stage=FString::Printf(TEXT("STAGE %02d / %02d"),Step,G->Mars?5:7);R.Objective=G->Objective();R.GuideTitle=G->Objective();R.GuideAction=G->Prompt();R.Total=1;
  switch(G->Stage){
   case ELunarStage::FindModule:R.Subtask=TEXT("Locate and collect the replacement power module.");R.GuideBody=TEXT("Follow the gold marker. Approach the module, then use your work key.");break;
   case ELunarStage::RestorePower:R.Completed=FMath::Clamp(G->RepairStep,0,3);R.Total=3;R.Subtask=G->PowerSequenceActive?TEXT("Station startup in progress."):TEXT("Install the module, connect the cable and restart power.");R.GuideBody=R.Subtask;if(G->PowerSequenceActive)R.GuideAction=TEXT("Automatic startup / wait for system checks.");break;
   case ELunarStage::CollectData:R.Subtask=TEXT("Download the research archive before returning.");R.GuideBody=TEXT("Approach the science terminal. Hold the work key to download; releasing cancels the current hold.");break;
   case ELunarStage::AlignRelay:R.Subtask=TEXT("Align the relay with the orbital link.");R.GuideBody=TEXT("Approach the marked antenna and hold the work key to align the relay.");break;
   case ELunarStage::SurveySamples:R.Completed=G->SampleCount();R.Total=3;R.Subtask=TEXT("Inspect the marked sample sites.");R.GuideBody=TEXT("Choose a site with your route key, then approach and hold the work key.");break;
   case ELunarStage::RecoverDrone:R.Subtask=TEXT("Recover the survey drone.");R.GuideBody=TEXT("Approach the drone and hold the work key to recover it.");break;
   case ELunarStage::ActivateBeacon:R.Completed=G->BeaconLocks;R.Total=3;R.Subtask=TEXT("Lock the moving signal inside the target window.");R.GuideBody=TEXT("Press the work key while the indicator is in the cyan window.");R.Beacon=true;R.BeaconPhase=G->BeaconPhase();break;
   case ELunarStage::ReturnHome:R.Subtask=TEXT("Return to the lander. Crew recorder recovery is optional.");R.GuideBody=TEXT("Check your remaining resource before taking an optional detour.");break;
   default:break;
  }
  R.Counter=FString::Printf(TEXT("OBJECTIVES COMPLETE  %d / %d"),R.Completed,R.Total);
  if(G->Stage==ELunarStage::SurveySamples)R.RouteHint=TEXT("Next site");else if(G->Stage==ELunarStage::ReturnHome&&!G->RecorderRecovered)R.RouteHint=TEXT("Change destination");
 }
 // Hints may be hidden, but a nearby usable object must retain its work key.
 if(!R.ShowGuide&&!R.Holding&&G->CanInteract()&&!R.GuideAction.IsEmpty()){R.ShowGuide=true;R.GuideCaption=TEXT("INTERACTION");R.GuideTitle.Empty();R.GuideBody.Empty();}
 if(R.Holding){R.ShowGuide=false;R.HoldTitle=G->Stage==ELunarStage::CollectData?TEXT("DOWNLOADING SCIENCE DATA"):G->Stage==ELunarStage::AlignRelay?TEXT("ALIGNING ORBITAL RELAY"):G->Stage==ELunarStage::RestorePower?TEXT("RESTARTING STATION"):TEXT("MISSION WORK IN PROGRESS");}
 if(S->Subtitles&&G->RadioRemaining>0)R.Subtitle=G->Radio;
 const auto* Target=G->Apollo?G->ApolloGuidanceTarget():G->Target();
 if(IsValid(Target)&&P&&!R.DustAnswers){R.HasNavigation=true;R.NavigationWorld=Target->GetActorLocation()+FVector(0,0,70);R.NavigationDistance=FVector::Dist2D(P->GetActorLocation(),Target->GetActorLocation())/100.f;R.NavigationName=G->Apollo?G->ApolloSiteName():LegacyName(G);}
 return R;
}


UTextBlock* ULunarFlightHUD::MakeText(float Size,FLinearColor Color,bool Subtitle){
 auto* Label=WidgetTree->ConstructWidget<UTextBlock>();auto Font=FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Size));Font.OutlineSettings.OutlineSize=1;Font.OutlineSettings.OutlineColor=FLinearColor(0,0,0,.45f);Label->SetFont(Font);Label->SetColorAndOpacity(Color);Label->SetShadowOffset(FVector2D(.7,.7));Label->SetShadowColorAndOpacity(FLinearColor(0,0,0,.6f));Label->SetAutoWrapText(true);Label->SetVisibility(ESlateVisibility::HitTestInvisible);FontLabels.Add(Label);FontSizes.Add(Size);SubtitleFonts.Add(Subtitle);return Label;
}
TSharedRef<SWidget> ULunarFlightHUD::RebuildWidget(){
 WidgetTree->RootWidget=nullptr;Faces.Reset();FontLabels.Reset();FontSizes.Reset();SubtitleFonts.Reset();AnswerLabels.Reset();
 auto* Scale=WidgetTree->ConstructWidget<UScaleBox>();Scale->SetStretch(EStretch::ScaleToFit);WidgetTree->RootWidget=Scale;
 auto* Frame=WidgetTree->ConstructWidget<USizeBox>();Frame->SetWidthOverride(1280);Frame->SetHeightOverride(720);Scale->AddChild(Frame);
 Content=WidgetTree->ConstructWidget<UCanvasPanel>();Frame->AddChild(Content);
 auto Stack=[&](){return WidgetTree->ConstructWidget<UVerticalBox>();};
 auto Row=[](UVerticalBox* Parent,UWidget* Child,float Gap=4.f){auto* Slot=Parent->AddChildToVerticalBox(Child);Slot->SetPadding(FMargin(0,0,0,Gap));};
 auto Panel=[&](UVerticalBox* StackWidget,float Width,float X,float Y,FVector2D Alignment){auto* Sized=WidgetTree->ConstructWidget<USizeBox>();Sized->SetWidthOverride(Width);Sized->AddChild(StackWidget);auto* Pane=Glass(WidgetTree,Sized,Faces);auto* Slot=Place(Content,Pane,X,Y,Width+30,100);Slot->SetAutoSize(true);Slot->SetAlignment(Alignment);return Pane;};
 auto* MissionStack=Stack();ExpeditionLabel=MakeText(10,Cyan);MissionLabel=MakeText(20,Ink);StageLabel=MakeText(10,Dim);ObjectiveLabel=MakeText(14,Gold);SubtaskLabel=MakeText(12,Ink);CounterLabel=MakeText(11,Dim);NoticeLabel=MakeText(11,Cyan);
 Row(MissionStack,ExpeditionLabel);Row(MissionStack,MissionLabel);Row(MissionStack,StageLabel,10);Row(MissionStack,ObjectiveLabel);Row(MissionStack,SubtaskLabel,10);Row(MissionStack,CounterLabel);Row(MissionStack,NoticeLabel,0);MissionPanel=Panel(MissionStack,334,26,24,FVector2D::ZeroVector);
 auto* ResourceStack=Stack();ResourceCaption=MakeText(10,Dim);ResourceLabel=MakeText(24,Ink);GravityLabel=MakeText(10,Dim);WarningLabel=MakeText(10,Dim);OxygenBar=WidgetTree->ConstructWidget<UProgressBar>();OxygenBar->SetFillColorAndOpacity(Cyan);auto* BarSize=WidgetTree->ConstructWidget<USizeBox>();BarSize->SetHeightOverride(4);BarSize->AddChild(OxygenBar);
 Row(ResourceStack,ResourceCaption);Row(ResourceStack,ResourceLabel,8);Row(ResourceStack,BarSize,8);Row(ResourceStack,GravityLabel);Row(ResourceStack,WarningLabel,0);Panel(ResourceStack,188,1254,24,FVector2D(1,0));
 auto* GuideStack=Stack();GuideCaption=MakeText(10,Cyan);GuideTitle=MakeText(13,Ink);GuideLabel=MakeText(12,Dim);GuideAction=MakeText(11,Gold);Row(GuideStack,GuideCaption);Row(GuideStack,GuideTitle);Row(GuideStack,GuideLabel);Row(GuideStack,GuideAction,0);GuidePanel=Panel(GuideStack,450,640,648,FVector2D(.5,1));
 auto* RadioStack=Stack();Row(RadioStack,MakeText(9,Gold),3);Cast<UTextBlock>(RadioStack->GetChildAt(0))->SetText(FText::FromString(TEXT("RADIO / MISSION UPDATE")));SubtitleLabel=MakeText(14,Ink,true);Row(RadioStack,SubtitleLabel,0);RadioPanel=Panel(RadioStack,1000,640,700,FVector2D(.5,1));
 ToolAnswers=Stack();auto* Question=MakeText(13,Gold);Question->SetText(FText::FromString(TEXT("WHAT EXPLAINS THE COATING DIFFERENCE?")));Row(ToolAnswers,Question,12);
 for(int32 I=0;I<3;++I){auto* Label=MakeText(14,Ink);AnswerLabels.Add(Label);Row(ToolAnswers,Label,12);}ToolsPanel=Glass(WidgetTree,ToolAnswers,Faces);Place(Content,ToolsPanel,190,200,900,260);
 // Only the inner canvas collapses: the outer widget keeps ticking to observe resume/close.
 SetVisibility(ESlateVisibility::SelfHitTestInvisible);ForceVolatile(true);CachedTextScale=CachedSubtitleScale=CachedOpacity=-1;CachedTool=INDEX_NONE;Refresh();return Super::RebuildWidget();
}
void ULunarFlightHUD::Refresh(){
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));Current=ReadState(G,P);if(!Content)return;
 const auto* S=ULunarSettings::Get();bool LayoutDirty=false;
 const float TextScale=FMath::Clamp(S->TextScale,.8f,1.5f),SubtitleScale=FMath::Clamp(S->SubtitleScale,.8f,1.5f);
 if(CachedTextScale!=TextScale||CachedSubtitleScale!=SubtitleScale){for(int32 I=0;I<FontLabels.Num();++I){auto Font=FontLabels[I]->GetFont();Font.Size=FMath::RoundToInt(FontSizes[I]*(SubtitleFonts[I]?SubtitleScale:TextScale));FontLabels[I]->SetFont(Font);}CachedTextScale=TextScale;CachedSubtitleScale=SubtitleScale;LayoutDirty=true;}
 if(CachedOpacity!=S->HudOpacity){for(const auto& Face:Faces)if(Face)Face->SetBrushColor(FLinearColor(.025f,.075f,.11f,S->HudPanelAlpha(.80f,true)));CachedOpacity=S->HudOpacity;}
 LayoutDirty|=Show(Content,Current.Visible);if(!Current.Visible)return;
 LayoutDirty|=Set(ExpeditionLabel,Current.Expedition);LayoutDirty|=Set(MissionLabel,Current.Mission);LayoutDirty|=Set(StageLabel,Current.Stage);LayoutDirty|=Set(ObjectiveLabel,TEXT("<>  ")+Current.Objective);LayoutDirty|=Set(SubtaskLabel,Current.Subtask);LayoutDirty|=Set(CounterLabel,Current.Counter);LayoutDirty|=Set(NoticeLabel,Current.Notice);LayoutDirty|=Show(NoticeLabel,!Current.Notice.IsEmpty());
 LayoutDirty|=Set(ResourceCaption,Current.ResourceLabel);LayoutDirty|=Set(ResourceLabel,Current.ResourceTime);LayoutDirty|=Set(GravityLabel,Current.Gravity);LayoutDirty|=Set(WarningLabel,Current.Warning);WarningLabel->SetColorAndOpacity(Current.ResourceWarning?Gold:Dim);ResourceLabel->SetColorAndOpacity(Current.ResourceWarning?Gold:Ink);
 OxygenBar->SetPercent(Current.ResourceFraction);OxygenBar->SetFillColorAndOpacity(Current.ResourceWarning?Gold:Cyan);LayoutDirty|=Show(OxygenBar,Current.BoundedResource);
 LayoutDirty|=Set(GuideCaption,Current.GuideCaption);LayoutDirty|=Set(GuideTitle,Current.GuideTitle);LayoutDirty|=Show(GuideTitle,!Current.GuideTitle.IsEmpty());LayoutDirty|=Set(GuideLabel,Current.GuideBody);LayoutDirty|=Show(GuideLabel,!Current.GuideBody.IsEmpty());LayoutDirty|=Set(GuideAction,Current.GuideAction);LayoutDirty|=Show(GuidePanel,Current.ShowGuide);LayoutDirty|=Set(SubtitleLabel,Current.Subtitle);LayoutDirty|=Show(RadioPanel,!Current.Subtitle.IsEmpty());
 const bool ToolPanel=Current.Tool==2||Current.Tool==3||Current.DustAnswers;LayoutDirty|=Show(ToolsPanel,ToolPanel);LayoutDirty|=Show(ToolAnswers,Current.DustAnswers);
 if(CachedTool!=Current.Tool){if(auto* HUDSlot=Cast<UCanvasPanelSlot>(ToolsPanel->Slot)){HUDSlot->SetPosition(Current.Tool==2?FVector2D(520,210):Current.Tool==3?FVector2D(440,240):FVector2D(190,200));HUDSlot->SetSize(Current.Tool==2?FVector2D(240,220):Current.Tool==3?FVector2D(400,120):FVector2D(900,260));}CachedTool=Current.Tool;LayoutDirty=true;}
 static const TCHAR* Answers[]={TEXT("Intrepid landing exhaust moved the dust."),TEXT("Rain washed the coating."),TEXT("Earth-like wind eroded the surface.")};
 if(Current.DustAnswers)for(int32 I=0;I<3;++I){LayoutDirty|=Set(AnswerLabels[I],FString(I==Current.DustChoice?TEXT(">  "):TEXT("   "))+Answers[I]);AnswerLabels[I]->SetColorAndOpacity(I==Current.DustChoice?Cyan:Ink);}
 if(LayoutDirty){Content->ForceLayoutPrepass();RadioTop=Current.Subtitle.IsEmpty()?710.f:700.f-RadioPanel->GetDesiredSize().Y;const float Bottom=Current.Subtitle.IsEmpty()?648.f:RadioTop-12.f;if(auto* HUDSlot=Cast<UCanvasPanelSlot>(GuidePanel->Slot))HUDSlot->SetPosition(FVector2D(640,Bottom));GuideTop=Current.ShowGuide?Bottom-GuidePanel->GetDesiredSize().Y:Bottom;}
}
void ULunarFlightHUD::NativeTick(const FGeometry& Geometry,float DeltaTime){Super::NativeTick(Geometry,DeltaTime);Refresh();Fade=Current.Visible?FMath::Min(1.f,Fade+FMath::Clamp(DeltaTime,0.f,.1f)/.16f):0.f;if(Content)Content->SetRenderOpacity(Fade);}

int32 ULunarFlightHUD::NativePaint(const FPaintArgs& Args,const FGeometry& Geo,const FSlateRect& Culling,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const {
 Layer=Super::NativePaint(Args,Geo,Culling,Out,Layer,Style,Enabled);if(!Current.Visible||Fade<=0)return Layer;
 const auto* S=ULunarSettings::Get();const float TextScale=FMath::Clamp(S->TextScale,.8f,1.5f);const FVector2D Available=Geo.GetLocalSize();const float Scale=FMath::Min(float(Available.X)/1280.f,float(Available.Y)/720.f);if(Scale<=UE_SMALL_NUMBER)return Layer;
 const FVector2D Origin=(Available-FVector2D(1280,720)*Scale)*.5f;const int32 PaintLayer=Layer+1;
 auto Tint=[&](FLinearColor Color){Color.A*=Fade*Style.GetColorAndOpacityTint().A;return Color;};
 auto Line=[&](FVector2D A,FVector2D B,FLinearColor Color,float Width=1.f){TArray<FVector2D> Points{Origin+A*Scale,Origin+B*Scale};FSlateDrawElement::MakeLines(Out,PaintLayer,Geo.ToPaintGeometry(),Points,ESlateDrawEffect::None,Tint(Color),true,Width*Scale);};
 auto Rect=[&](FVector2D At,FVector2D Size,FLinearColor Color){const auto* Brush=FCoreStyle::Get().GetBrush("WhiteBrush");FSlateDrawElement::MakeBox(Out,PaintLayer,Geo.ToPaintGeometry(Size*Scale,FSlateLayoutTransform(Origin+At*Scale)),Brush,ESlateDrawEffect::None,Tint(Color));};
 auto Text=[&](const FString& Value,FVector2D At,float Size,FLinearColor Color,bool Center=false){
  const auto Font=FCoreStyle::GetDefaultFontStyle("Regular",FMath::Max(8,FMath::RoundToInt(Size*TextScale*Scale)));
  FVector2D Measured=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Value,Font);FVector2D Point=Origin+At*Scale;if(Center)Point.X-=Measured.X*.5;
  FSlateDrawElement::MakeText(Out,PaintLayer,Geo.ToPaintGeometry(Measured+FVector2D(4,4),FSlateLayoutTransform(Point+FVector2D(1,1)*Scale)),Value,Font,ESlateDrawEffect::None,Tint(FLinearColor(0,0,0,.75f)));
  FSlateDrawElement::MakeText(Out,PaintLayer+1,Geo.ToPaintGeometry(Measured+FVector2D(4,4),FSlateLayoutTransform(Point)),Value,Font,ESlateDrawEffect::None,Tint(Color));
 };
 auto Ring=[&](FVector2D Center,float Radius,float Fraction,FLinearColor Color,float Width){TArray<FVector2D> Points;const int32 Segments=FMath::Clamp(FMath::CeilToInt(64*FMath::Clamp(Fraction,0.f,1.f)),1,64);for(int32 I=0;I<=Segments;++I){const float A=-HALF_PI+2*PI*Fraction*I/Segments;Points.Add(Origin+(Center+FVector2D(FMath::Cos(A),FMath::Sin(A))*Radius)*Scale);}FSlateDrawElement::MakeLines(Out,PaintLayer,Geo.ToPaintGeometry(),Points,ESlateDrawEffect::None,Tint(Color),true,Width*Scale);};
 auto Keycap=[&](const FString& Value,FVector2D Center){const float Half=FMath::Clamp(float(Value.Len())*4.f+9.f,15.f,88.f);Rect(Center-FVector2D(Half,12),FVector2D(Half*2,24),FLinearColor(.025f,.09f,.13f,.84f));Line(Center-FVector2D(Half,12),Center+FVector2D(Half,-12),Cyan);Line(Center-FVector2D(Half,12),Center+FVector2D(-Half,12),Cyan);Line(Center+FVector2D(Half,-12),Center+FVector2D(Half,12),Cyan);Line(Center+FVector2D(-Half,12),Center+FVector2D(Half,12),Cyan);Text(Value,Center-FVector2D(0,8),11,Ink,true);};
 if(Current.HasNavigation){
  auto* PC=GetOwningPlayer();APawn* Pawn=PC?PC->GetPawn():nullptr;FVector2D Screen;int32 VW=0,VH=0;if(PC)PC->GetViewportSize(VW,VH);const float PixelScale=FMath::Min(VW/1280.f,VH/720.f);
  const bool Front=PC&&Pawn&&FVector::DotProduct(PC->GetControlRotation().Vector(),Current.NavigationWorld-Pawn->GetActorLocation())>0;
  if(Front&&PixelScale>UE_SMALL_NUMBER&&PC->ProjectWorldLocationToScreen(Current.NavigationWorld,Screen,true)){
   Screen=(Screen-(FVector2D(VW,VH)-FVector2D(1280,720)*PixelScale)*.5f)/PixelScale;const float NavBottom=FMath::Max(205.f,GuideTop-24.f);
   Screen.X=FMath::Clamp(float(Screen.X),38.f,1050.f);Screen.Y=FMath::Clamp(float(Screen.Y),190.f,NavBottom);if(Screen.X<410&&Screen.Y<330)Screen.X=420;
   Line(Screen-FVector2D(0,8),Screen+FVector2D(8,0),Gold,1.5f);Line(Screen+FVector2D(8,0),Screen+FVector2D(0,8),Gold,1.5f);Line(Screen+FVector2D(0,8),Screen-FVector2D(8,0),Gold,1.5f);Line(Screen-FVector2D(8,0),Screen-FVector2D(0,8),Gold,1.5f);
   Text(Current.NavigationName,Screen+FVector2D(16,-13),11,Ink);Text(FString::Printf(TEXT("%.0f m"),Safe(Current.NavigationDistance)),Screen+FVector2D(16,4),11,Gold);
  }else{
   const float Side=PC&&Pawn?FVector::DotProduct(FRotationMatrix(FRotator(0,PC->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),Current.NavigationWorld-Pawn->GetActorLocation()):1.f;
   Text(Side>=0?TEXT("TARGET >"):TEXT("< TARGET"),FVector2D(Side>=0?1120:38,352),12,Gold);Text(FString::Printf(TEXT("%.0f m"),Safe(Current.NavigationDistance)),FVector2D(Side>=0?1120:38,374),11,Ink);
  }
 }
 if(Current.PhotoFrame){for(float X:{128.f,1152.f})for(float Y:{95.f,625.f}){const float DX=X<640?1.f:-1.f,DY=Y<360?1.f:-1.f;Line(FVector2D(X,Y),FVector2D(X+DX*34,Y),Cyan,1.5f);Line(FVector2D(X,Y),FVector2D(X,Y+DY*34),Cyan,1.5f);}}
 if((Current.Crosshair&&Current.Tool==0&&!Current.Holding)||Current.PhotoFrame){Line(FVector2D(635,360),FVector2D(645,360),Ink);Line(FVector2D(640,355),FVector2D(640,365),Ink);}
 if(Current.Holding){
  const FVector2D Center(640,353);Ring(Center,48,1,FLinearColor(.20f,.31f,.37f,.75f),3);if(Current.HoldFraction>0)Ring(Center,48,Current.HoldFraction,Cyan,3);
  // Download glyph and the player's actual work binding are deliberately separate.
  Line(Center+FVector2D(0,-26),Center+FVector2D(0,-9),Cyan,2);Line(Center+FVector2D(-7,-16),Center+FVector2D(0,-9),Cyan,2);Line(Center+FVector2D(7,-16),Center+FVector2D(0,-9),Cyan,2);Line(Center+FVector2D(-9,-4),Center+FVector2D(9,-4),Cyan,2);Keycap(Current.WorkKey,Center+FVector2D(0,17));
  Text(Current.HoldTitle,FVector2D(640,416),15,Ink,true);Text(TEXT("Hold to continue"),FVector2D(640,442),12,Dim,true);Text(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(Current.HoldFraction*100)),FVector2D(640,466),16,Cyan,true);
  Text(TEXT("Release [ ")+Current.WorkKey+TEXT(" ] to cancel"),FVector2D(32,FMath::Min(675.f,RadioTop-38.f)),12,Ink);
 }
 if(Current.Tool==2){const FVector2D Center(640,305);Line(Center-FVector2D(74,0),Center+FVector2D(74,0),Dim);Line(Center-FVector2D(0,74),Center+FVector2D(0,74),Dim);Ring(Center,8,1,Cyan,2);const FVector2D Bubble=Center+FVector2D(Current.Level.X,-Current.Level.Y)*74;Ring(Bubble,6,1,Current.Level.Size()<.09f?Cyan:Gold,2);Text(TEXT("CENTER THE BUBBLE"),FVector2D(640,395),11,Dim,true);}
 if(Current.Tool==3){Text(FString::Printf(TEXT("EARTH LINK  %.0f%%"),Current.AntennaSignal*100),FVector2D(640,260),17,Current.AntennaSignal>.91f?Cyan:Gold,true);Rect(FVector2D(460,315),FVector2D(360,8),FLinearColor(.18f,.28f,.33f,.9f));Rect(FVector2D(460,315),FVector2D(360*Current.AntennaSignal,8),Cyan);}
 if(Current.Beacon){Rect(FVector2D(460,313),FVector2D(360,8),FLinearColor(.18f,.28f,.33f,.9f));Rect(FVector2D(460+360*.34f,313),FVector2D(360*.32f,8),Cyan*.7f);const float Phase=FMath::Clamp(Safe(Current.BeaconPhase),0.f,1.f);Rect(FVector2D(460+360*Phase-2,309),FVector2D(4,16),Gold);Text(TEXT("LOCK INSIDE THE CYAN WINDOW"),FVector2D(640,335),11,Ink,true);}
 if(Current.Subtitle.IsEmpty()&&!Current.Holding){Text(Current.Apollo?TEXT("[ ")+Current.JournalKey+TEXT(" ] Journal / [ I ] Lessons / [ H ] Guide"):TEXT("[ ")+Current.MoveKeys+TEXT(" ] Move  /  [ I ] Field lessons"),FVector2D(28,682),10,Dim);Text((Current.RouteHint.IsEmpty()?FString():TEXT("[ ")+Current.RouteKey+TEXT(" ] ")+Current.RouteHint+TEXT("  /  "))+TEXT("[ Esc ] Pause"),FVector2D(934,682),10,Dim);}
 return PaintLayer+2;
}
