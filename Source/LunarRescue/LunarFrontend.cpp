#include "Misc/PackageName.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "LunarGame.h"
#include "LunarMissionCard.h"
#include "LunarLoading.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/BackgroundBlur.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UnrealType.h"
#include "Sound/SoundBase.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "HAL/IConsoleManager.h"
#include <limits>

namespace {
const FLinearColor Ink(.86f,.93f,.98f),Dim(.43f,.58f,.67f),Accent(.02f,.82f,.9f);
bool IntroShownThisProcess=false;
ALunarCharacter* Pilot(UObject* O){return Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(O,0));}
ALunarGameMode* Mission(UObject* O){return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(O));}
UTextBlock* Label(UWidgetTree* Tree,const FString& Text,int32 Size,FLinearColor Color=Ink){
 auto* W=Tree->ConstructWidget<UTextBlock>();W->SetText(FText::FromString(Text));const bool Scales=!Text.StartsWith(TEXT("L U N A R R E S C U E"));
 auto Font=W->GetFont();Font.Size=FMath::RoundToInt(Size*(Scales?FMath::Clamp(ULunarSettings::Get()->TextScale,1.f,1.5f):1.f));Font.TypefaceFontName=TEXT("Regular");W->SetFont(Font);W->SetColorAndOpacity(Color);W->SetAutoWrapText(true);
 if(auto* Main=Tree->GetTypedOuter<ULunarMainMenu>())Main->FrontendLabels.Add({W,Size,Scales});else if(auto* Panel=Tree->GetTypedOuter<ULunarPanel>())Panel->FrontendLabels.Add({W,Size,Scales});return W;
}
void Place(UCanvasPanel* Canvas,UWidget* Widget,float X,float Y,float W,float H){auto* S=Canvas->AddChildToCanvas(Widget);S->SetPosition(FVector2D(X,Y));S->SetSize(FVector2D(W,H));}
UCanvasPanel* Frame(UWidgetTree* Tree){
 auto* Scale=Tree->ConstructWidget<UScaleBox>();Scale->SetStretch(EStretch::ScaleToFit);auto* Size=Tree->ConstructWidget<USizeBox>();Size->SetWidthOverride(1280);Size->SetHeightOverride(720);Scale->AddChild(Size);auto* C=Tree->ConstructWidget<UCanvasPanel>();Size->AddChild(C);Tree->RootWidget=Scale;return C;
}
UButton* FrontendButton(UWidgetTree* Tree,const FString& Text,bool Active=false){
 auto* B=Tree->ConstructWidget<ULunarFrontendButton>();auto Style=B->GetStyle();
 Style.Normal=FSlateRoundedBoxBrush(Active?FLinearColor(.025f,.16f,.28f,.7f):FLinearColor(.009f,.035f,.06f,.65f),4.f,FLinearColor(.16f,.40f,.53f,.85f),1.f);
 Style.Hovered=FSlateRoundedBoxBrush(FLinearColor(.04f,.36f,.58f,.78f),4.f,FLinearColor(.05f,.85f,1.f,.95f),1.f);
 Style.Pressed=FSlateRoundedBoxBrush(FLinearColor(.03f,.27f,.46f,.9f),4.f,FLinearColor(.1f,.9f,1.f),1.f);
 Style.Disabled=FSlateRoundedBoxBrush(FLinearColor(.01f,.03f,.05f,.65f),4.f,FLinearColor(.14f,.23f,.29f,.5f),1.f);
 Style.NormalPadding=Style.PressedPadding=FMargin(0);B->SetGlassStyle(Style);
 auto* Layers=Tree->ConstructWidget<UOverlay>();Layers->SetVisibility(ESlateVisibility::HitTestInvisible);B->AddChild(Layers);if(auto* Slot=Cast<UButtonSlot>(Layers->Slot)){Slot->SetPadding(0);Slot->SetHorizontalAlignment(HAlign_Fill);Slot->SetVerticalAlignment(VAlign_Fill);}
 // Small action surfaces use the translucent glass tint directly. Keep the old
 // independent blur pass only for an explicitly labelled same-build cost test.
 if(FParse::Param(FCommandLine::Get(),TEXT("MissionFullGlassBaseline"))){
  auto* Blur=Tree->ConstructWidget<UBackgroundBlur>();Blur->SetBlurStrength(2.f);Blur->SetCornerRadius(FVector4(4,4,4,4));FSlateBrush Fallback;Fallback.TintColor=FSlateColor(FLinearColor(.01f,.035f,.06f,.85f));Blur->SetLowQualityFallbackBrush(Fallback);Blur->SetVisibility(ESlateVisibility::HitTestInvisible);auto* Slot=Layers->AddChildToOverlay(Blur);Slot->SetHorizontalAlignment(HAlign_Fill);Slot->SetVerticalAlignment(VAlign_Fill);
 }
 auto* Face=Tree->ConstructWidget<UBorder>();Face->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White,4.f));Face->SetBrushColor(FLinearColor(.015f,.08f,.13f,.18f));Face->SetPadding(Text==TEXT("X")?FMargin(0):FMargin(12,5));Face->SetHorizontalAlignment(HAlign_Center);Face->SetVerticalAlignment(VAlign_Center);Face->SetVisibility(ESlateVisibility::HitTestInvisible);auto* FaceSlot=Layers->AddChildToOverlay(Face);FaceSlot->SetHorizontalAlignment(HAlign_Fill);FaceSlot->SetVerticalAlignment(VAlign_Fill);
 auto* T=Label(Tree,Text,12);T->SetAutoWrapText(false);T->SetJustification(ETextJustify::Center);Face->SetContent(T);B->GlassSurface=Face;B->Caption=T;return B;
}
void RefreshFrontendLabels(UWidgetTree* Tree,TArray<FLunarFrontendLabel>& Labels,float& Cached){
 const float Scale=FMath::Clamp(ULunarSettings::Get()->TextScale,1.f,1.5f);if(FMath::IsNearlyEqual(Cached,Scale))return;Cached=Scale;
 Labels.RemoveAll([&](const FLunarFrontendLabel& Entry){auto* W=Entry.Widget.Get();if(!W)return true;UWidget* Top=W;while(Top->GetParent())Top=Top->GetParent();return Top!=Tree->RootWidget;});
 for(const auto& Entry:Labels)if(auto* W=Entry.Widget.Get()){auto Font=W->GetFont();Font.Size=FMath::RoundToInt(Entry.BaseSize*(Entry.Scales?Scale:1.f));W->SetFont(Font);}
}
UImage* Backdrop(UWidgetTree* Tree,UCanvasPanel* C){auto* I=Tree->ConstructWidget<UImage>();I->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMoon.T_MenuMoon")));Place(C,I,0,0,1280,720);return I;}
void Shade(UWidgetTree* Tree,UCanvasPanel* C,float X,float Y,float W,float H,FLinearColor Color){auto* B=Tree->ConstructWidget<UBorder>();B->SetBrushColor(Color);B->SetVisibility(ESlateVisibility::HitTestInvisible);Place(C,B,X,Y,W,H);}
}

void ULunarSettingRow::ChoiceChanged(FString Choice,ESelectInfo::Type Type){
 if(Type==ESelectInfo::Direct)return;auto* S=ULunarSettings::Get();const int32 Index=Choices->GetSelectedIndex();
 if(Setting==TEXT("Resolution")||Setting==TEXT("DisplayMode")){
  if(auto* GS=UGameUserSettings::GetGameUserSettings()){
   FIntPoint R=GS->GetScreenResolution();int32 Mode=GS->GetFullscreenMode()==EWindowMode::Windowed?0:GS->GetFullscreenMode()==EWindowMode::WindowedFullscreen?1:2;
   if(Setting==TEXT("Resolution")){FString X,Y;if(Choice.Split(TEXT(" x "),&X,&Y)){R=FIntPoint(FCString::Atoi(*X),FCString::Atoi(*Y));}}
   else Mode=Index;
   if(R.X>=640&&R.Y>=480)S->PreviewDisplay(R,Mode);
  }return;
 }
 if(auto* Prop=FindFProperty<FBoolProperty>(S->GetClass(),Setting))Prop->SetPropertyValue_InContainer(S,Index==1);
 else if(auto* IntProp=FindFProperty<FIntProperty>(S->GetClass(),Setting)){
  const int32 Limits[]={30,60,120,0};IntProp->SetPropertyValue_InContainer(S,Setting==TEXT("FrameLimit")?Limits[FMath::Clamp(Index,0,3)]:Index);
 }
 if(Setting==TEXT("Quality")){S->TextureQuality=S->ShadowQuality=S->ViewDistance=S->Quality;}
 S->Apply(Pilot(this));
 if(Setting==TEXT("Quality"))if(auto* P=Pilot(this);P&&P->ActivePanel)P->ActivePanel->FillSettings();
}

void ULunarPanel::BuildSettings(){
 if(BuiltSettings){FillSettings();return;}
 BuiltSettings=true;FrontendLabels.Reset();FrontendTextScale=-1;auto* C=Frame(WidgetTree);Backdrop(WidgetTree,C);Shade(WidgetTree,C,0,0,1280,720,FLinearColor(0,.01f,.025f,.65f));
 Shade(WidgetTree,C,26,32,1228,656,FLinearColor(.002f,.006f,.011f,.98f));
 Place(C,Label(WidgetTree,TEXT("L U N A R R E S C U E"),10,Dim),58,49,800,20);
 Place(C,Label(WidgetTree,TEXT("S E T T I N G S"),22),58,66,900,46);
 auto* X=FrontendButton(WidgetTree,TEXT("X"));X->OnClicked.AddDynamic(this,&ULunarPanel::Close);Place(C,X,1214,42,28,28);
 const TCHAR* Names[]={TEXT("VIDEO OPTIONS"),TEXT("SOUND OPTIONS"),TEXT("GAMEPLAY"),TEXT("CONTROLS")};
 for(int32 I=0;I<4;++I){auto* B=FrontendButton(WidgetTree,Names[I]);Place(C,B,58+291*I,118,291,36);
  if(I==0)B->OnClicked.AddDynamic(this,&ULunarPanel::VideoTab);if(I==1)B->OnClicked.AddDynamic(this,&ULunarPanel::SoundTab);if(I==2)B->OnClicked.AddDynamic(this,&ULunarPanel::GameplayTab);if(I==3)B->OnClicked.AddDynamic(this,&ULunarPanel::ControlsTab);
 }
 SettingsBody=WidgetTree->ConstructWidget<UVerticalBox>();Place(C,SettingsBody,58,172,1164,423);Shade(WidgetTree,C,640,190,1,405,FLinearColor(.035f,.06f,.08f));
 Shade(WidgetTree,C,58,610,1164,1,FLinearColor(.2f,.32f,.4f));
 auto* Back=FrontendButton(WidgetTree,TEXT("BACK"));Back->OnClicked.AddDynamic(this,&ULunarPanel::Close);Place(C,Back,58,628,183,38);
 DisplayStatus=Label(WidgetTree,TEXT("Changes save automatically. Scroll each column for more."),10,Dim);Place(C,DisplayStatus,260,636,550,35);
 auto* Keep=FrontendButton(WidgetTree,TEXT("KEEP DISPLAY"));Keep->Rename(TEXT("KeepDisplayButton"));Keep->SetVisibility(ESlateVisibility::Collapsed);Keep->OnClicked.AddDynamic(this,&ULunarPanel::KeepDisplay);Place(C,Keep,810,628,190,38);
 auto* Reset=FrontendButton(WidgetTree,TEXT("RESTORE DEFAULTS"));Reset->OnClicked.AddDynamic(this,&ULunarPanel::ResetSettings);Place(C,Reset,1010,628,212,38);
 FillSettings();
}
void ULunarPanel::FillSettings(){
 SettingsBody->ClearChildren();
 auto* Bar=WidgetTree->ConstructWidget<UBorder>();Bar->SetBrushColor(Accent);auto* BarSize=WidgetTree->ConstructWidget<USizeBox>();BarSize->SetHeightOverride(3);BarSize->AddChild(Bar);auto* BS=SettingsBody->AddChildToVerticalBox(BarSize);BS->SetPadding(FMargin(SettingsTab*291,0,(3-SettingsTab)*291,12));
 auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();SettingsBody->AddChildToVerticalBox(Columns)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
 const TCHAR* Headings[][2]={{TEXT("D I S P L A Y"),TEXT("G R A P H I C S")},{TEXT("V O L U M E"),TEXT("A U D I O   S E T T I N G S")},{TEXT("I N T E R F A C E"),TEXT("M O V E M E N T  &  C A M E R A")},{TEXT("M O U S E  &  M O V E M E N T"),TEXT("A C T I O N S  &  C A M E R A")}};
 const TArray<FName> Rows[4][2]={
  {{TEXT("Resolution"),TEXT("DisplayMode"),TEXT("RenderScale"),TEXT("FrameLimit"),TEXT("VSync"),TEXT("Brightness"),TEXT("FieldOfView")},{TEXT("Quality"),TEXT("TextureQuality"),TEXT("ShadowQuality"),TEXT("ViewDistance"),TEXT("AntiAliasing"),TEXT("AmbientOcclusion"),TEXT("Bloom"),TEXT("MotionBlur")}},
  {{TEXT("MasterVolume"),TEXT("EffectsVolume"),TEXT("RadioVolume")},{TEXT("Subtitles")}},
  {{TEXT("TextScale"),TEXT("SubtitleScale"),TEXT("HudOpacity"),TEXT("TutorialHints"),TEXT("Crosshair")},{TEXT("ToggleSprint"),TEXT("CameraMotion")}},
  {{TEXT("Sensitivity"),TEXT("InvertY"),TEXT("MouseSmoothing"),TEXT("MoveForward"),TEXT("MoveBack"),TEXT("MoveLeft"),TEXT("MoveRight"),TEXT("Sprint"),TEXT("Jump")},{TEXT("Interact"),TEXT("Route"),TEXT("Journal"),TEXT("View"),TEXT("Zoom")}}
 };
 for(int32 Col=0;Col<2;++Col){
  auto* Stack=WidgetTree->ConstructWidget<UVerticalBox>();auto* ColumnLayout=Columns->AddChildToHorizontalBox(Stack);ColumnLayout->SetSize(FSlateChildSize(ESlateSizeRule::Fill));ColumnLayout->SetPadding(FMargin(Col?30:0,0,Col?0:30,0));
  Stack->AddChildToVerticalBox(Label(WidgetTree,Headings[SettingsTab][Col],14))->SetPadding(FMargin(0,0,0,12));
  auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Stack->AddChildToVerticalBox(Scroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  auto* Content=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Content);
  for(FName Name:Rows[SettingsTab][Col]){auto* Row=CreateWidget<ULunarSettingRow>(GetOwningPlayer());Row->Setting=Name;auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetMinDesiredHeight(40*ULunarSettings::Get()->TextScale);Size->AddChild(Row);Content->AddChildToVerticalBox(Size)->SetPadding(FMargin(0,0,8,3));}
  auto Note=[&](const FString& Text){Content->AddChildToVerticalBox(Label(WidgetTree,Text,11,Dim))->SetPadding(FMargin(0,18,8,0));};
  if(SettingsTab==0&&Col==0)Note(TEXT("Display changes revert after 15 seconds unless kept. Close / Esc also reverts an unconfirmed change."));
  if(SettingsTab==0&&Col==1)Note(TEXT("Terrain geometry is fixed to the imported dataset. Presets change rendering, not scientific elevations."));
  if(SettingsTab==1&&Col==0)Note(TEXT("Suit and tool volume includes footsteps and station effects. No separate music, ambient or UI tracks are installed."));
  if(SettingsTab==1&&Col==1){Note(TEXT("Output: system default device. Speaker layout and spatial processing use the system configuration. No dynamic-range simulation is provided."));auto* Test=FrontendButton(WidgetTree,TEXT("TEST RADIO AUDIO"));Test->OnClicked.AddDynamic(this,&ULunarPanel::TestAudio);Content->AddChildToVerticalBox(Test)->SetPadding(FMargin(0,18,8,0));}
  if(SettingsTab==2&&Col==0)Note(TEXT("H also saves the Apollo tutorial choice. HUD backgrounds adjust from 50 to 100%; menus retain full contrast. Crosshair hides the walking reticle; camera framing and work cues stay visible."));
  if(SettingsTab==2&&Col==1)Note(TEXT("Camera view: use the rebound View key in game. No crouch or head-bob mode is implemented. Progress saves after committed work, not on a timed interval."));
  if(SettingsTab==3&&Col==1)Note(TEXT("Select a binding, then press a new key. Reserved and duplicate keys are rejected. Esc / F1 always closes settings."));
 }
}
void ULunarPanel::NativeTick(const FGeometry& Geo,float Delta){Super::NativeTick(Geo,Delta);RefreshFrontendLabels(WidgetTree,FrontendLabels,FrontendTextScale);if(!Settings){if(Body&&FMath::Abs(EvidenceLayoutWidth-EvidenceContentWidth())>1.f)Rebuild();return;}auto* S=ULunarSettings::Get();if(auto* Keep=WidgetTree->FindWidget(TEXT("KeepDisplayButton")))Keep->SetVisibility(S->DisplayPending?ESlateVisibility::Visible:ESlateVisibility::Collapsed);if(S->DisplayPending&&FPlatformTime::Seconds()>=S->DisplayDeadline){S->RevertDisplay();FillSettings();}if(DisplayStatus)DisplayStatus->SetText(FText::FromString(S->DisplayPending?FString::Printf(TEXT("Keep display? Reverting in %d seconds."),FMath::Max(0,FMath::CeilToInt(S->DisplayDeadline-FPlatformTime::Seconds()))):TEXT("Changes save automatically. Scroll each column for more.")));}
void ULunarPanel::VideoTab(){SettingsTab=0;FillSettings();}
void ULunarPanel::SoundTab(){SettingsTab=1;FillSettings();}
void ULunarPanel::GameplayTab(){SettingsTab=2;FillSettings();}
void ULunarPanel::ControlsTab(){SettingsTab=3;FillSettings();}
void ULunarPanel::KeepDisplay(){ULunarSettings::Get()->ConfirmDisplay();}
void ULunarPanel::RevertDisplay(){ULunarSettings::Get()->RevertDisplay();FillSettings();}
void ULunarPanel::TestAudio(){auto* S=ULunarSettings::Get();if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/Lunar/Immersion/Audio/Radio_Intro.Radio_Intro")))UGameplayStatics::PlaySound2D(this,Sound,S->RadioVolume*S->MasterVolume);}

TSharedRef<SWidget> ULunarMainMenu::RebuildWidget(){
 ++FrontendBuilds;FrontendLabels.Reset();FrontendTextScale=-1;DestinationPreviewReady=false;DestinationTextureLoads=0;DestinationPresentationUpdates=0;NextSelectionProfileAt=0;SelectionProfileStep=0;SelectionProfileRequested=FParse::Param(FCommandLine::Get(),TEXT("MissionSelectionProfile"));
 MoonBackdrop=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMoon.T_MenuMoon"));++DestinationTextureLoads;MarsBackdrop=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMars.T_MenuMars"));++DestinationTextureLoads;
 auto* C=Frame(WidgetTree);Background=WidgetTree->ConstructWidget<UImage>();Background->SetBrushFromTexture(MoonBackdrop);Place(C,Background,0,0,1280,720);Shade(WidgetTree,C,0,0,1280,720,FLinearColor(0,0,0,.2f));
 HomeCards.Reset();ExtraHomeCards.Reset();MissionCards.Reset();MissionSelection=false;SelectedMission=INDEX_NONE;IntroState=FLunarIntroState();
 HomePage=WidgetTree->ConstructWidget<UCanvasPanel>();Place(C,HomePage,0,0,1280,720);
 Place(HomePage,Label(WidgetTree,TEXT("L U N A R R E S C U E"),31),84,84,650,48);Place(HomePage,Label(WidgetTree,TEXT("E X P E D I T I O N S"),16,Dim),87,133,600,30);
 const TCHAR* Names[]={TEXT("Continue expedition"),TEXT("Explore the Moon"),TEXT("Explore Mars")};
 for(int32 I=0;I<3;++I){auto* Card=CreateWidget<ULunarMissionCard>(GetOwningPlayer());Card->Configure(I,Names[I],TEXT(""),TEXT(""),true);Card->OnChosen.AddDynamic(this,&ULunarMainMenu::ChooseHome);Card->OnPreview.AddDynamic(this,&ULunarMainMenu::PreviewHome);Place(HomePage,Card,87,195+48*I,268,42);HomeCards.Add(Card);if(I==0)Card->SetIsEnabled(Mission(this)&&Mission(this)->ApolloCachedCheckpoint!=nullptr);}
 for(int32 I=3;I<5;++I){auto* Card=CreateWidget<ULunarMissionCard>(GetOwningPlayer());Card->Configure(I,I==3?TEXT("Settings"):TEXT("Exit"),TEXT(""),TEXT(""),true);Card->OnChosen.AddDynamic(this,&ULunarMainMenu::ChooseHome);Place(HomePage,Card,87,195+48*I,268,42);ExtraHomeCards.Add(Card);}
 Destination=Label(WidgetTree,TEXT(""),24);Place(HomePage,Destination,410,184,700,60);
 Details=Label(WidgetTree,TEXT(""),13);Place(HomePage,Details,410,249,520,390);
 SelectionPage=WidgetTree->ConstructWidget<UCanvasPanel>();Place(C,SelectionPage,0,0,1280,720);
 Shade(WidgetTree,SelectionPage,0,0,1280,720,FLinearColor(.001f,.006f,.014f,.42f));
 Place(SelectionPage,Label(WidgetTree,TEXT("L U N A R R E S C U E  /  E X P E D I T I O N S"),11,Accent),70,48,1000,24);
 SelectionTitle=Label(WidgetTree,TEXT(""),32);Place(SelectionPage,SelectionTitle,70,80,1100,64);
 Place(SelectionPage,Label(WidgetTree,TEXT("Choose a mission, then start when you are ready."),15),70,151,1100,40);
 const TCHAR* Titles[]={TEXT("APOLLO 12"),TEXT("SELENE"),TEXT("ARES")};
 const TCHAR* Tags[]={TEXT("MOON / HISTORICAL EXPLORATION"),TEXT("MOON / FICTIONAL RESCUE"),TEXT("MARS / FICTIONAL PROTOTYPE")};
 const TCHAR* Descriptions[]={TEXT("Deploy ALSEP, collect three samples and recover Surveyor 3 hardware.\n\nLROC terrain / 6 stages\nUntimed exploration"),TEXT("Restore SELENE power and recover its science data.\n\nAuthored terrain / optional recorder\n8-minute gameplay oxygen budget"),TEXT("Restore ARES power, recover the ice survey and align the orbital antenna.\n\nAuthored terrain / optional recorder\n8-minute gameplay oxygen budget")};
 for(int32 I=0;I<3;++I){auto* Card=CreateWidget<ULunarMissionCard>(GetOwningPlayer());Card->Configure(I,Titles[I],Tags[I],Descriptions[I]);Card->OnChosen.AddDynamic(this,&ULunarMainMenu::SelectMission);Place(SelectionPage,Card,I==0?268:I==1?656:462,216,356,312);MissionCards.Add(Card);}
 Shade(WidgetTree,SelectionPage,0,558,1280,162,FLinearColor(.001f,.004f,.009f,.91f));
 SelectionPrompt=Label(WidgetTree,TEXT("Select a mission to continue."),15);Place(SelectionPage,SelectionPrompt,70,574,780,64);
 LaunchButton=FrontendButton(WidgetTree,TEXT("START MISSION"),true);LaunchButton->OnClicked.AddDynamic(this,&ULunarMainMenu::LaunchMission);Place(SelectionPage,LaunchButton,922,580,288,50);
 SelectionBackButton=FrontendButton(WidgetTree,TEXT("BACK"));SelectionBackButton->OnClicked.AddDynamic(this,&ULunarMainMenu::Back);Place(SelectionPage,SelectionBackButton,70,650,150,36);
 SelectionSettingsButton=FrontendButton(WidgetTree,TEXT("SETTINGS"));SelectionSettingsButton->OnClicked.AddDynamic(this,&ULunarMainMenu::Settings);Place(SelectionPage,SelectionSettingsButton,1060,650,150,36);
 Place(SelectionPage,Label(WidgetTree,TEXT("LEFT / RIGHT  SELECT     ENTER  START     ESC  BACK     1 / 2  DESTINATION"),11,Dim),250,654,800,30);
 SelectionPage->SetVisibility(ESlateVisibility::Collapsed);LaunchButton->SetIsEnabled(false);PreviewMoon();
 IntroLayer=WidgetTree->ConstructWidget<UCanvasPanel>();Place(C,IntroLayer,0,0,1280,720);
 Shade(WidgetTree,IntroLayer,0,0,1280,720,FLinearColor::Black);
 IntroImage=WidgetTree->ConstructWidget<UImage>();
 auto* IntroTexture=FPackageName::DoesPackageExist(TEXT("/Game/Lunar/Frontend/Startup20261114/T_AstroKid"))?LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/Startup20261114/T_AstroKid.T_AstroKid")):nullptr;
 if(IntroTexture){IntroImage->SetBrushFromTexture(IntroTexture);Place(IntroLayer,IntroImage,0,26,1280,668);Shade(WidgetTree,IntroLayer,1136,606,64,64,FLinearColor::Black);}
 else{auto* Wordmark=Label(WidgetTree,TEXT("AstroKid"),44);Wordmark->Rename(TEXT("PublicStartupWordmark"));Place(IntroLayer,Wordmark,500,295,640,85);Place(IntroLayer,Label(WidgetTree,TEXT("E X P E D I T I O N S"),19,Dim),452,372,700,60);UE_LOG(LogTemp,Display,TEXT("NASA_INTRO original text presentation; optional reference artwork absent"));}
 BeginIntro();
 if(FParse::Param(FCommandLine::Get(),TEXT("FrontendReview"))){
  FTimerHandle Handle;GetWorld()->GetTimerManager().SetTimer(Handle,FTimerDelegate::CreateWeakLambda(this,[this,Step=0]()mutable{
   auto Capture=[&](const TCHAR* Name){const FString Folder=FPaths::ProjectDir()/TEXT("Reports/Frontend20261028");IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/TEXT("frontend-")+Name+TEXT(".png"),true,false);};
   auto* P=Pilot(this);if(!P){FPlatformMisc::RequestExitWithStatus(false,1);return;}
   switch(Step++){
    case 0:Capture(TEXT("moon"));break;case 1:PreviewMars();break;case 2:Capture(TEXT("mars"));break;
    case 3:P->OpenSettings();break;case 4:Capture(TEXT("video"));break;
    case 5:P->ActivePanel->SoundTab();break;case 6:Capture(TEXT("sound"));break;
    case 7:P->ActivePanel->GameplayTab();break;case 8:Capture(TEXT("gameplay"));break;
    case 9:P->ActivePanel->ControlsTab();break;case 10:Capture(TEXT("controls"));break;
    case 11:ULunarSettings::Get()->TextScale=1.5f;P->ActivePanel->FillSettings();break;case 12:Capture(TEXT("controls150"));break;
    case 13:P->ClosePanel();break;default:FPlatformMisc::RequestExitWithStatus(false,0);break;
   }
  }),2.f,true,4.f);
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("FrontendTest"))){
  FTimerHandle Handle;GetWorld()->GetTimerManager().SetTimer(Handle,FTimerDelegate::CreateWeakLambda(this,[this](){
   FString Report;int32 Failures=0;auto Check=[&](bool Good,const TCHAR* Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+FString(Name)+TEXT("\n");if(!Good)++Failures;};
   auto* P=Pilot(this);auto* S=ULunarSettings::Get();Check(P!=nullptr,TEXT("Menu player exists"));
   Check(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMoon.T_MenuMoon"))&&LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMars.T_MenuMars")),TEXT("Both local menu art assets load"));
   PreviewMars();Check(Destination->GetText().ToString().Contains(TEXT("MARS")),TEXT("Mars preview identifies fictional destination"));PreviewMoon();Check(Destination->GetText().ToString().Contains(TEXT("MOON")),TEXT("Moon preview restores destination"));
   S->TutorialHints=false;S->Crosshair=false;S->HudOpacity=.65f;S->MasterVolume=.37f;S->RenderScale=77;S->FrameLimit=120;S->VSync=true;S->Subtitles=false;S->TextureQuality=2;S->Rebind(TEXT("Interact"),EKeys::U);
   const FString Path=FPaths::ProjectSavedDir()/TEXT("AutomatedTests/FrontendSettings.ini");IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);S->SaveConfig(CPF_Config,*Path);auto* Loaded=NewObject<ULunarSettings>();Loaded->LoadConfig(nullptr,*Path);
   Check(FMath::IsNearlyEqual(Loaded->MasterVolume,.37f)&&Loaded->RenderScale==77&&Loaded->FrameLimit==120&&Loaded->VSync&&!Loaded->Subtitles&&Loaded->TextureQuality==2,TEXT("New graphics and sound settings survive serialization"));
   Check(Loaded->Key(TEXT("Interact"))==EKeys::U,TEXT("New settings retain rebound work key"));
   Check(!Loaded->TutorialHints&&!Loaded->Crosshair&&FMath::IsNearlyEqual(Loaded->HudOpacity,.65f),TEXT("Accessibility preferences survive independent config reload"));
   Check(FMath::IsNearlyEqual(S->HudPanelAlpha(.8f,true),.52f)&&FMath::IsNearlyEqual(S->HudPanelAlpha(.8f,false),.8f),TEXT("HUD opacity changes active backgrounds and preserves menu contrast"));
   S->HudOpacity=-1;S->Apply(P,false);Check(S->HudOpacity==.5f,TEXT("HUD opacity cannot fall below readable lower bound"));
   S->HudOpacity=2;S->Apply(P,false);Check(S->HudOpacity==1.f,TEXT("HUD opacity cannot exceed full opacity"));
   S->HudOpacity=std::numeric_limits<float>::quiet_NaN();S->Apply(P,false);Check(S->HudOpacity==1.f,TEXT("Non-finite HUD opacity falls back to full contrast"));
   S->AntiAliasing=1;S->MotionBlur=true;S->Apply(P,false);
   Check(IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"))->GetInt()==1&&IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->GetInt()==1&&IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"))->GetInt()==120,TEXT("Player renderer choices override project AA, motion blur and FPS defaults"));
   S->PreviewDisplay(FIntPoint(640,480),2);Check(!S->DisplayPending,TEXT("Automation cannot change the user's display mode"));
   S->Reset();S->Apply(P,false);Check(S->MasterVolume==1&&S->Subtitles&&S->FrameLimit==60&&S->RenderScale==100&&S->TutorialHints&&S->Crosshair&&S->HudOpacity==1.f,TEXT("Restore defaults resets new fields"));
   if(P){P->OpenSettings();Check(P->ActivePanel&&P->ActivePanel->SettingsBody,TEXT("Settings builds independently of evidence album"));if(P->ActivePanel){for(int32 I=0;I<4;++I){P->ActivePanel->SettingsTab=I;P->ActivePanel->FillSettings();Check(P->ActivePanel->SettingsBody->GetChildrenCount()==2,TEXT("Tab contains indicator and two-column content"));}P->ClosePanel();Check(!P->ActivePanel,TEXT("Settings closes without leaving input overlay"));}}
   Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectDir()/TEXT("Reports/frontend-checks.txt")));FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
  }),3.f,false);
 }
 return Super::RebuildWidget();
}
void ULunarMainMenu::NativeTick(const FGeometry& Geometry,float Delta){
 Super::NativeTick(Geometry,Delta);RefreshFrontendLabels(WidgetTree,FrontendLabels,FrontendTextScale);
 // A bounded, screenshot-free profiling workload. It only browses the menu;
 // capture startup/warmup and full-glass comparison are documented by the tool.
 if(SelectionProfileRequested&&!IsIntroActive()&&GetWorld()&&GetWorld()->GetTimeSeconds()>=NextSelectionProfileAt){
  auto* P=Pilot(this);if(P&&P->ActivePanel)return;
  switch(SelectionProfileStep++%4){
   case 0:Back();ExploreMoon();SelectMission(0);break;
   case 1:SelectMission(1);break;
   case 2:Back();ExploreMars();SelectMission(2);break;
   default:Back();ExploreMoon();SelectMission(1);break;
  }
  NextSelectionProfileAt=GetWorld()->GetTimeSeconds()+2;
  UE_LOG(LogTemp,Display,TEXT("MISSION_SELECTION_PROFILE step=%d destination=%s mission=%d loads=%d builds=%d full_glass_baseline=%d"),SelectionProfileStep,SelectionMars?TEXT("Mars"):TEXT("Moon"),SelectedMission,DestinationTextureLoads,FrontendBuilds,FParse::Param(FCommandLine::Get(),TEXT("MissionFullGlassBaseline")));
 }
}

void ULunarMainMenu::PreviewHome(int32 Index){if(!IsIntroActive()&&!MissionSelection){if(Index==1)PreviewMoon();else if(Index==2)PreviewMars();}}
void ULunarMainMenu::ChooseHome(int32 Index){if(IsIntroActive())return;if(Index==0)Continue();else if(Index==1)ExploreMoon();else if(Index==2)ExploreMars();else if(Index==3)Settings();else if(Index==4)ExitGame();}
void ULunarMainMenu::PreviewMoon(){if(MissionSelection||IsIntroActive()||(DestinationPreviewReady&&!PreviewingMars))return;DestinationPreviewReady=true;++DestinationPresentationUpdates;PreviewingMars=false;if(Background)Background->SetBrushFromTexture(MoonBackdrop);Destination->SetText(FText::FromString(TEXT("01 / MOON")));Details->SetText(FText::FromString(FString(TEXT("EXPEDITIONS\nAPOLLO 12 / HISTORICAL EXPLORATION\nSELENE / FICTIONAL RESCUE\n\nCHOOSE YOUR MISSION\nReview the mission guide before departure.\n\nOBJECTIVE\nExplore lunar worksites or restore the rescue station.")).ToUpper()));for(int32 I=0;I<HomeCards.Num();++I)HomeCards[I]->SetSelected(I==1);}
void ULunarMainMenu::PreviewMars(){if(MissionSelection||IsIntroActive()||(DestinationPreviewReady&&PreviewingMars))return;DestinationPreviewReady=true;++DestinationPresentationUpdates;PreviewingMars=true;if(Background)Background->SetBrushFromTexture(MarsBackdrop);Destination->SetText(FText::FromString(TEXT("02 / MARS")));Details->SetText(FText::FromString(FString(TEXT("EXPEDITION\nARES / FICTIONAL PROTOTYPE\n\nGAMEPLAY BUDGET\n8 MIN OXYGEN / 3.71 m/s2\n\nOBJECTIVE\nRestore the relay and recover ice survey data.")).ToUpper().Replace(TEXT("M/S2"),TEXT("m/s²"))));for(int32 I=0;I<HomeCards.Num();++I)HomeCards[I]->SetSelected(I==2);}
void ULunarMainMenu::OpenSelection(bool Mars){
 auto* G=Mission(this);auto* P=Pilot(this);if(!G||!G->Menu||IsIntroActive()||LunarLoading::IsBusy()||(P&&P->ActivePanel))return;
 MissionSelection=false;if(Mars)PreviewMars();else PreviewMoon();MissionSelection=true;SelectionMars=Mars;SelectedMission=INDEX_NONE;
 HomePage->SetVisibility(ESlateVisibility::Collapsed);SelectionPage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
 SelectionTitle->SetText(FText::FromString(Mars?TEXT("MARS / MISSION SELECTION"):TEXT("MOON / MISSION SELECTION")));
 SelectionPrompt->SetText(FText::FromString(TEXT("Select a mission to continue.")));LaunchButton->SetIsEnabled(false);
 for(int32 I=0;I<MissionCards.Num();++I){MissionCards[I]->SetSelected(false);MissionCards[I]->SetVisibility((Mars?I==2:I<2)?ESlateVisibility::Visible:ESlateVisibility::Collapsed);}
}
void ULunarMainMenu::ExploreMoon(){OpenSelection(false);}
void ULunarMainMenu::ExploreMars(){OpenSelection(true);}
void ULunarMainMenu::SelectMission(int32 Index){
 auto* P=Pilot(this);if(IsIntroActive()||!MissionSelection||LunarLoading::IsBusy()||(P&&P->ActivePanel)||Index<0||Index>=MissionCards.Num()||(SelectionMars?Index!=2:Index==2))return;
 SelectedMission=Index;for(int32 I=0;I<MissionCards.Num();++I)MissionCards[I]->SetSelected(I==Index);
 const TCHAR* Names[]={TEXT("Apollo 12 / Surveyor's Legacy"),TEXT("SELENE / fictional rescue"),TEXT("ARES / fictional prototype")};
 SelectionPrompt->SetText(FText::FromString(FString(TEXT("SELECTED: "))+Names[Index]+TEXT("\nStart mission when you are ready.")));LaunchButton->SetIsEnabled(true);
}
void ULunarMainMenu::MoveSelection(int32 Direction){
 if(!MissionSelection||!Direction)return;
 const int32 First=SelectionMars?2:0,Count=SelectionMars?1:2;
 const int32 Offset=SelectedMission==INDEX_NONE?(Direction>0?0:Count-1):(SelectedMission-First+(Direction>0?1:Count-1))%Count;
 SelectMission(First+Offset);
}
void ULunarMainMenu::ConfirmSelection(){if(IsIntroActive())return;if(MissionSelection)LaunchMission();else OpenSelection(PreviewingMars);}
void ULunarMainMenu::LaunchMission(){
 auto* G=Mission(this);auto* P=Pilot(this);if(!G||!G->Menu||IsIntroActive()||!MissionSelection||LunarLoading::IsBusy()||(P&&P->ActivePanel)||SelectedMission<0||SelectedMission>=3||(SelectionMars?SelectedMission!=2:SelectedMission==2))return;
 const TCHAR* Maps[]={TEXT("/Game/Lunar/Maps/Apollo12"),TEXT("/Game/Lunar/Maps/MoonBase"),TEXT("/Game/Mars/Maps/MarsBase")};
 if(LunarLoading::Open(this,Maps[SelectedMission]))LaunchButton->SetIsEnabled(false);
}
void ULunarMainMenu::Back(){auto* P=Pilot(this);if(IsIntroActive()||!MissionSelection||LunarLoading::IsBusy()||(P&&P->ActivePanel))return;MissionSelection=false;SelectionPage->SetVisibility(ESlateVisibility::Collapsed);HomePage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);if(SelectionMars)PreviewMars();else PreviewMoon();}
void ULunarMainMenu::Continue(){if(IsIntroActive())return;if(auto* P=Pilot(this);P&&P->ActivePanel)return;if(!LunarLoading::IsBusy())if(auto* G=Mission(this))G->ContinueApollo();}
void ULunarMainMenu::Settings(){if(IsIntroActive())return;if(auto* P=Pilot(this))P->OpenSettings();}
void ULunarMainMenu::ExitGame(){if(IsIntroActive())return;UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}

void ULunarMainMenu::BeginIntro(bool Force){
 if(!IntroLayer)return;
 if(IntroShownThisProcess&&!Force){IntroState.Elapsed=FLunarIntroState::Duration;IntroState.Active=false;IntroLayer->SetVisibility(ESlateVisibility::Collapsed);return;}
 IntroShownThisProcess=true;IntroState.Begin();MissionSelection=false;SelectedMission=INDEX_NONE;
 HomePage->SetVisibility(ESlateVisibility::Collapsed);SelectionPage->SetVisibility(ESlateVisibility::Collapsed);
 IntroLayer->SetVisibility(ESlateVisibility::Visible);IntroLayer->SetRenderOpacity(1);if(IntroImage)IntroImage->SetRenderOpacity(1);
}
void ULunarMainMenu::AdvanceIntro(float Delta){
 if(!IntroState.Active)return;
 IntroState.Advance(Delta);
 if(IntroImage)IntroImage->SetRenderOpacity(1);
 IntroLayer->SetRenderOpacity(IntroState.LayerAlpha());
 if(IntroState.RevealingMenu())HomePage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
 if(!IntroState.Active){IntroLayer->SetVisibility(ESlateVisibility::Collapsed);UE_LOG(LogTemp,Display,TEXT("NASA_INTRO completed before menu input enabled"));}
}
