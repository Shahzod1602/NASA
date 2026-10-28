#include "LunarPanels.h"
#include "LunarSettings.h"
#include "LunarGame.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
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

namespace {
const FLinearColor Ink(.86f,.93f,.98f),Dim(.43f,.58f,.67f),Accent(.02f,.82f,.9f);
ALunarCharacter* Pilot(UObject* O){return Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(O,0));}
ALunarGameMode* Mission(UObject* O){return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(O));}
UTextBlock* Label(UWidgetTree* Tree,const FString& Text,int32 Size,FLinearColor Color=Ink){
 auto* W=Tree->ConstructWidget<UTextBlock>();W->SetText(FText::FromString(Text));auto Font=W->GetFont();Font.Size=Size;Font.TypefaceFontName=TEXT("Regular");W->SetFont(Font);W->SetColorAndOpacity(Color);W->SetAutoWrapText(true);return W;
}
void Place(UCanvasPanel* Canvas,UWidget* Widget,float X,float Y,float W,float H){auto* S=Canvas->AddChildToCanvas(Widget);S->SetPosition(FVector2D(X,Y));S->SetSize(FVector2D(W,H));}
UCanvasPanel* Frame(UWidgetTree* Tree){
 auto* Scale=Tree->ConstructWidget<UScaleBox>();Scale->SetStretch(EStretch::ScaleToFit);auto* Size=Tree->ConstructWidget<USizeBox>();Size->SetWidthOverride(1280);Size->SetHeightOverride(720);Scale->AddChild(Size);auto* C=Tree->ConstructWidget<UCanvasPanel>();Size->AddChild(C);Tree->RootWidget=Scale;return C;
}
UButton* Button(UWidgetTree* Tree,const FString& Text,bool Active=false){
 auto* B=Tree->ConstructWidget<UButton>();auto Style=B->GetStyle();
 Style.Normal.DrawAs=Style.Hovered.DrawAs=Style.Pressed.DrawAs=ESlateBrushDrawType::Box;
 Style.Normal.TintColor=FSlateColor(Active?FLinearColor(.04f,.2f,.26f,.96f):FLinearColor(.005f,.012f,.018f,.94f));
 Style.Hovered.TintColor=FSlateColor(FLinearColor(.08f,.23f,.3f));Style.Pressed.TintColor=FSlateColor(FLinearColor(.03f,.34f,.4f));B->SetStyle(Style);
 auto* T=Label(Tree,Text,12);T->SetAutoWrapText(false);B->AddChild(T);return B;
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
 BuiltSettings=true;auto* C=Frame(WidgetTree);Backdrop(WidgetTree,C);Shade(WidgetTree,C,0,0,1280,720,FLinearColor(0,.01f,.025f,.65f));
 Shade(WidgetTree,C,26,32,1228,656,FLinearColor(.002f,.006f,.011f,.98f));
 Place(C,Label(WidgetTree,TEXT("L U N A R R E S C U E"),10,Dim),58,49,800,20);
 Place(C,Label(WidgetTree,TEXT("S E T T I N G S"),22),58,72,900,40);
 auto* X=Button(WidgetTree,TEXT("X"));X->OnClicked.AddDynamic(this,&ULunarPanel::Close);Place(C,X,1214,42,28,28);
 const TCHAR* Names[]={TEXT("VIDEO OPTIONS"),TEXT("SOUND OPTIONS"),TEXT("GAMEPLAY"),TEXT("CONTROLS")};
 for(int32 I=0;I<4;++I){auto* B=Button(WidgetTree,Names[I]);Place(C,B,58+291*I,118,291,36);
  if(I==0)B->OnClicked.AddDynamic(this,&ULunarPanel::VideoTab);if(I==1)B->OnClicked.AddDynamic(this,&ULunarPanel::SoundTab);if(I==2)B->OnClicked.AddDynamic(this,&ULunarPanel::GameplayTab);if(I==3)B->OnClicked.AddDynamic(this,&ULunarPanel::ControlsTab);
 }
 SettingsBody=WidgetTree->ConstructWidget<UVerticalBox>();Place(C,SettingsBody,58,172,1164,423);Shade(WidgetTree,C,640,190,1,405,FLinearColor(.035f,.06f,.08f));
 Shade(WidgetTree,C,58,610,1164,1,FLinearColor(.2f,.32f,.4f));
 auto* Back=Button(WidgetTree,TEXT("BACK"));Back->OnClicked.AddDynamic(this,&ULunarPanel::Close);Place(C,Back,58,628,183,38);
 DisplayStatus=Label(WidgetTree,TEXT("Changes save automatically. Scroll each column for more."),10,Dim);Place(C,DisplayStatus,260,636,550,35);
 auto* Keep=Button(WidgetTree,TEXT("KEEP DISPLAY"));Keep->Rename(TEXT("KeepDisplayButton"));Keep->SetVisibility(ESlateVisibility::Collapsed);Keep->OnClicked.AddDynamic(this,&ULunarPanel::KeepDisplay);Place(C,Keep,810,628,190,38);
 auto* Reset=Button(WidgetTree,TEXT("RESTORE DEFAULTS"));Reset->OnClicked.AddDynamic(this,&ULunarPanel::ResetSettings);Place(C,Reset,1010,628,212,38);
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
  {{TEXT("TextScale"),TEXT("SubtitleScale")},{TEXT("ToggleSprint"),TEXT("CameraMotion")}},
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
  if(SettingsTab==1&&Col==1){Note(TEXT("Output: system default device. Speaker layout and spatial processing use the system configuration. No dynamic-range simulation is provided."));auto* Test=Button(WidgetTree,TEXT("TEST RADIO AUDIO"));Test->OnClicked.AddDynamic(this,&ULunarPanel::TestAudio);Content->AddChildToVerticalBox(Test)->SetPadding(FMargin(0,18,8,0));}
  if(SettingsTab==2&&Col==0)Note(TEXT("English interface. H toggles optional tutorial guidance; J opens the map. Mission-critical markers and prompts remain visible."));
  if(SettingsTab==2&&Col==1)Note(TEXT("Camera view: use the rebound View key in game. No crouch or head-bob mode is implemented. Progress saves after committed work, not on a timed interval."));
  if(SettingsTab==3&&Col==1)Note(TEXT("Select a binding, then press a new key. Reserved and duplicate keys are rejected. Esc / F1 always closes settings."));
 }
}
void ULunarPanel::NativeTick(const FGeometry& Geo,float Delta){Super::NativeTick(Geo,Delta);if(!Settings)return;auto* S=ULunarSettings::Get();if(auto* Keep=WidgetTree->FindWidget(TEXT("KeepDisplayButton")))Keep->SetVisibility(S->DisplayPending?ESlateVisibility::Visible:ESlateVisibility::Collapsed);if(S->DisplayPending&&FPlatformTime::Seconds()>=S->DisplayDeadline){S->RevertDisplay();FillSettings();}if(DisplayStatus)DisplayStatus->SetText(FText::FromString(S->DisplayPending?FString::Printf(TEXT("Keep display? Reverting in %d seconds."),FMath::Max(0,FMath::CeilToInt(S->DisplayDeadline-FPlatformTime::Seconds()))):TEXT("Changes save automatically. Scroll each column for more.")));}
void ULunarPanel::VideoTab(){SettingsTab=0;FillSettings();}
void ULunarPanel::SoundTab(){SettingsTab=1;FillSettings();}
void ULunarPanel::GameplayTab(){SettingsTab=2;FillSettings();}
void ULunarPanel::ControlsTab(){SettingsTab=3;FillSettings();}
void ULunarPanel::KeepDisplay(){ULunarSettings::Get()->ConfirmDisplay();}
void ULunarPanel::RevertDisplay(){ULunarSettings::Get()->RevertDisplay();FillSettings();}
void ULunarPanel::TestAudio(){auto* S=ULunarSettings::Get();if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/Lunar/Immersion/Audio/Radio_Intro.Radio_Intro")))UGameplayStatics::PlaySound2D(this,Sound,S->RadioVolume*S->MasterVolume);}

TSharedRef<SWidget> ULunarMainMenu::RebuildWidget(){
 auto* C=Frame(WidgetTree);Background=Backdrop(WidgetTree,C);Shade(WidgetTree,C,0,0,1280,720,FLinearColor(0,0,0,.12f));
 Place(C,Label(WidgetTree,TEXT("L U N A R R E S C U E"),31),84,84,650,48);Place(C,Label(WidgetTree,TEXT("E X P E D I T I O N S"),16,Dim),87,133,600,30);
 const TCHAR* Names[]={TEXT("Continue expedition"),TEXT("Explore the Moon"),TEXT("Explore Mars"),TEXT("Settings"),TEXT("Exit")};
 for(int32 I=0;I<5;++I){auto* B=Button(WidgetTree,Names[I]);if(auto* Layout=Cast<UButtonSlot>(B->GetContent()->Slot))Layout->SetHorizontalAlignment(HAlign_Left);auto MenuStyle=B->GetStyle();MenuStyle.NormalPadding=MenuStyle.PressedPadding=FMargin(16,4);B->SetStyle(MenuStyle);Place(C,B,87,195+48*I,268,42);
  if(I==0){B->OnClicked.AddDynamic(this,&ULunarMainMenu::Continue);B->SetIsEnabled(Mission(this)&&Mission(this)->ApolloCachedCheckpoint!=nullptr);}
  if(I==1){MoonButton=B;B->OnHovered.AddDynamic(this,&ULunarMainMenu::PreviewMoon);B->OnClicked.AddDynamic(this,&ULunarMainMenu::ExploreMoon);}
  if(I==2){MarsButton=B;B->OnHovered.AddDynamic(this,&ULunarMainMenu::PreviewMars);B->OnClicked.AddDynamic(this,&ULunarMainMenu::ExploreMars);}
  if(I==3)B->OnClicked.AddDynamic(this,&ULunarMainMenu::Settings);
  if(I==4)B->OnClicked.AddDynamic(this,&ULunarMainMenu::ExitGame);
 }
 Destination=Label(WidgetTree,TEXT(""),24);Place(C,Destination,410,190,700,45);
 Details=Label(WidgetTree,TEXT(""),13);Place(C,Details,410,249,460,280);
 Shade(WidgetTree,C,0,620,1280,100,FLinearColor(0,0,0,.7f));
 Place(C,Label(WidgetTree,TEXT("1  MOON     2  MARS     F1  SETTINGS"),13,Dim),87,642,700,24);
 Place(C,Label(WidgetTree,TEXT("Illustrated menu background / not a survey image"),11,Dim),87,672,700,22);
 PreviewMoon();
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
   S->MasterVolume=.37f;S->RenderScale=77;S->FrameLimit=120;S->VSync=true;S->Subtitles=false;S->TextureQuality=2;S->Rebind(TEXT("Interact"),EKeys::B);
   const FString Path=FPaths::ProjectSavedDir()/TEXT("AutomatedTests/FrontendSettings.ini");IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);S->SaveConfig(CPF_Config,*Path);auto* Loaded=NewObject<ULunarSettings>();Loaded->LoadConfig(nullptr,*Path);
   Check(FMath::IsNearlyEqual(Loaded->MasterVolume,.37f)&&Loaded->RenderScale==77&&Loaded->FrameLimit==120&&Loaded->VSync&&!Loaded->Subtitles&&Loaded->TextureQuality==2,TEXT("New graphics and sound settings survive serialization"));
   Check(Loaded->Key(TEXT("Interact"))==EKeys::B,TEXT("New settings retain rebound work key"));
   S->AntiAliasing=1;S->MotionBlur=true;S->Apply(P,false);
   Check(IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"))->GetInt()==1&&IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->GetInt()==1&&IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"))->GetInt()==120,TEXT("Player renderer choices override project AA, motion blur and FPS defaults"));
   S->PreviewDisplay(FIntPoint(640,480),2);Check(!S->DisplayPending,TEXT("Automation cannot change the user's display mode"));
   S->Reset();S->Apply(P,false);Check(S->MasterVolume==1&&S->Subtitles&&S->FrameLimit==60&&S->RenderScale==100,TEXT("Restore defaults resets new fields"));
   if(P){P->OpenSettings();Check(P->ActivePanel&&P->ActivePanel->SettingsBody,TEXT("Settings builds independently of evidence album"));if(P->ActivePanel){for(int32 I=0;I<4;++I){P->ActivePanel->SettingsTab=I;P->ActivePanel->FillSettings();Check(P->ActivePanel->SettingsBody->GetChildrenCount()==2,TEXT("Tab contains indicator and two-column content"));}P->ClosePanel();Check(!P->ActivePanel,TEXT("Settings closes without leaving input overlay"));}}
   Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectDir()/TEXT("Reports/frontend-checks.txt")));FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
  }),3.f,false);
 }
 return Super::RebuildWidget();
}
void ULunarMainMenu::PreviewMoon(){if(Background)Background->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMoon.T_MenuMoon")));Destination->SetText(FText::FromString(TEXT("01 / MOON")));Details->SetText(FText::FromString(TEXT("EXPEDITION\nAPOLLO 12 / SURVEYOR'S LEGACY\n\nTERRAIN & STRUCTURE\nLROC TERRAIN / 6 STAGES\n\nOBJECTIVE\nCollect lunar samples and Surveyor 3 hardware.")));MoonButton->SetBackgroundColor(Accent);MarsButton->SetBackgroundColor(FLinearColor::White);}
void ULunarMainMenu::PreviewMars(){if(Background)Background->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_MenuMars.T_MenuMars")));Destination->SetText(FText::FromString(TEXT("02 / MARS")));Details->SetText(FText::FromString(TEXT("EXPEDITION\nARES / FICTIONAL PROTOTYPE\n\nGAMEPLAY BUDGET\n8 MIN OXYGEN / 3.71 m/s2\n\nOBJECTIVE\nRestore the relay and recover ice survey data.")));MarsButton->SetBackgroundColor(Accent);MoonButton->SetBackgroundColor(FLinearColor::White);}
void ULunarMainMenu::ExploreMoon(){if(auto* G=Mission(this))G->SelectPlanet(false);}
void ULunarMainMenu::ExploreMars(){if(auto* G=Mission(this))G->SelectPlanet(true);}
void ULunarMainMenu::Continue(){if(auto* G=Mission(this))G->ContinueApollo();}
void ULunarMainMenu::Settings(){if(auto* P=Pilot(this))P->OpenSettings();}
void ULunarMainMenu::ExitGame(){UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
