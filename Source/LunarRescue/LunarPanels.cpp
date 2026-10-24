#include "LunarPanels.h"
#include "LunarSettings.h"
#include "LunarGame.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ScrollBox.h"
#include "Components/InputKeySelector.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "GameFramework/PlayerController.h"

namespace {
ALunarGameMode* Game(UObject* O){return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(O));}
ALunarCharacter* Player(UObject* O){return Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(O,0));}
UTextBlock* Text(UWidgetTree* W,const FString& T,int Size=22){auto* L=W->ConstructWidget<UTextBlock>();L->SetText(FText::FromString(T));auto F=L->GetFont();F.Size=FMath::RoundToInt(Size*ULunarSettings::Get()->TextScale);L->SetFont(F);L->SetAutoWrapText(true);L->SetColorAndOpacity(FSlateColor(FLinearColor(.9,.94,.93)));return L;}
UButton* Button(UWidgetTree* W,const FString& T){auto* B=W->ConstructWidget<UButton>();B->SetBackgroundColor(FLinearColor(.055,.14,.18));B->AddChild(Text(W,T,20));return B;}
FString Title(FName K){static const TMap<FName,FString> Titles={{TEXT("Sensitivity"),TEXT("Mouse sensitivity")},{TEXT("TextScale"),TEXT("Interface text size")},{TEXT("SubtitleScale"),TEXT("Subtitle size")},{TEXT("EffectsVolume"),TEXT("Suit and tool volume")},{TEXT("RadioVolume"),TEXT("Radio volume")},{TEXT("Brightness"),TEXT("Brightness")},{TEXT("FieldOfView"),TEXT("Field of view")},{TEXT("InvertY"),TEXT("Invert mouse Y")},{TEXT("CameraMotion"),TEXT("Hand motion")},{TEXT("ToggleSprint"),TEXT("Toggle fast stride")},{TEXT("Quality"),TEXT("Graphics quality")},{TEXT("MoveForward"),TEXT("Move forward")},{TEXT("MoveBack"),TEXT("Move backward")},{TEXT("MoveLeft"),TEXT("Move left")},{TEXT("MoveRight"),TEXT("Move right")},{TEXT("Interact"),TEXT("Open work tool")},{TEXT("Jump"),TEXT("Hop")},{TEXT("Route"),TEXT("Next site / journal page")},{TEXT("Journal"),TEXT("Field journal")},{TEXT("View"),TEXT("Camera view")},{TEXT("Sprint"),TEXT("Fast stride")},{TEXT("Zoom"),TEXT("Visor zoom")}};const FString* T=Titles.Find(K);return T?*T:K.ToString();}
}
TSharedRef<SWidget> ULunarSettingRow::RebuildWidget(){
 auto* Row=WidgetTree->ConstructWidget<UHorizontalBox>();WidgetTree->RootWidget=Row;
 auto* Label=Text(WidgetTree,Title(Setting),20);Row->AddChildToHorizontalBox(Label)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
 const TSet<FName> Sliders={TEXT("Sensitivity"),TEXT("TextScale"),TEXT("SubtitleScale"),TEXT("EffectsVolume"),TEXT("RadioVolume"),TEXT("Brightness"),TEXT("FieldOfView")};
 const TSet<FName> Toggles={TEXT("InvertY"),TEXT("CameraMotion"),TEXT("ToggleSprint"),TEXT("Quality")};
 ValueLabel=Text(WidgetTree,TEXT(""),19);
 if(Sliders.Contains(Setting)){
  auto* Slider=WidgetTree->ConstructWidget<USlider>();auto* S=ULunarSettings::Get();float V=0;
  if(Setting==TEXT("Sensitivity"))V=(S->Sensitivity-.15f)/1.85f;
  if(Setting==TEXT("TextScale"))V=(S->TextScale-1)*2;if(Setting==TEXT("SubtitleScale"))V=(S->SubtitleScale-1)*2;
  if(Setting==TEXT("EffectsVolume"))V=S->EffectsVolume;if(Setting==TEXT("RadioVolume"))V=S->RadioVolume;
  if(Setting==TEXT("Brightness"))V=(S->Brightness-.8f)/.6f;if(Setting==TEXT("FieldOfView"))V=(S->FieldOfView-70)/30;
  Slider->SetValue(FMath::Clamp(V,0.f,1.f));Slider->OnValueChanged.AddDynamic(this,&ULunarSettingRow::Changed);
  auto* RowLayout=Row->AddChildToHorizontalBox(Slider);RowLayout->SetSize(FSlateChildSize(ESlateSizeRule::Fill));RowLayout->SetPadding(FMargin(18,8));
  Row->AddChildToHorizontalBox(ValueLabel);
 }else if(Toggles.Contains(Setting)){
  auto* B=WidgetTree->ConstructWidget<UButton>();B->SetBackgroundColor(FLinearColor(.055,.14,.18));B->AddChild(ValueLabel);B->OnClicked.AddDynamic(this,&ULunarSettingRow::Clicked);Row->AddChildToHorizontalBox(B);
 }else{
  Selector=WidgetTree->ConstructWidget<UInputKeySelector>();Selector->SetSelectedKey(FInputChord(ULunarSettings::Get()->Key(Setting)));Selector->SetAllowModifierKeys(false);Selector->SetAllowGamepadKeys(false);Selector->OnKeySelected.AddDynamic(this,&ULunarSettingRow::Selected);Row->AddChildToHorizontalBox(Selector);Row->AddChildToHorizontalBox(ValueLabel);
  auto KeyStyle=Selector->GetTextStyle();KeyStyle.SetColorAndOpacity(FSlateColor(FLinearColor(.015f,.03f,.04f)));KeyStyle.SetFont(ValueLabel->GetFont());Selector->SetTextStyle(KeyStyle);
 }
 Refresh();
 return Super::RebuildWidget();
}
void ULunarSettingRow::Refresh(){auto* S=ULunarSettings::Get();FString V;
 if(Setting==TEXT("Sensitivity"))V=FString::Printf(TEXT("%.2f"),S->Sensitivity);
 if(Setting==TEXT("TextScale"))V=FString::Printf(TEXT("%.0f%%"),S->TextScale*100);
 if(Setting==TEXT("SubtitleScale"))V=FString::Printf(TEXT("%.0f%%"),S->SubtitleScale*100);
 if(Setting==TEXT("EffectsVolume"))V=FString::Printf(TEXT("%.0f%%"),S->EffectsVolume*100);
 if(Setting==TEXT("RadioVolume"))V=FString::Printf(TEXT("%.0f%%"),S->RadioVolume*100);
 if(Setting==TEXT("Brightness"))V=FString::Printf(TEXT("%.0f%%"),S->Brightness*100);
 if(Setting==TEXT("FieldOfView"))V=FString::Printf(TEXT("%.0f"),S->FieldOfView);
 if(Setting==TEXT("InvertY"))V=S->InvertY?TEXT("On"):TEXT("Off");
 if(Setting==TEXT("CameraMotion"))V=S->CameraMotion?TEXT("On"):TEXT("Off");
 if(Setting==TEXT("ToggleSprint"))V=S->ToggleSprint?TEXT("Toggle"):TEXT("Hold");
 if(Setting==TEXT("Quality"))V=S->Quality==0?TEXT("Low"):S->Quality==1?TEXT("Medium"):TEXT("High");
 ValueLabel->SetText(FText::FromString(V));
}
void ULunarSettingRow::Changed(float V){auto* S=ULunarSettings::Get();
 if(Setting==TEXT("Sensitivity"))S->Sensitivity=.15f+V*1.85f;
 if(Setting==TEXT("TextScale"))S->TextScale=1+FMath::RoundToFloat(V*2)*.25f;
 if(Setting==TEXT("SubtitleScale"))S->SubtitleScale=1+FMath::RoundToFloat(V*2)*.25f;
 if(Setting==TEXT("EffectsVolume"))S->EffectsVolume=V;if(Setting==TEXT("RadioVolume"))S->RadioVolume=V;
 if(Setting==TEXT("Brightness"))S->Brightness=.8f+V*.6f;if(Setting==TEXT("FieldOfView"))S->FieldOfView=70+V*30;
 S->Apply(Player(this));Refresh();
}
void ULunarSettingRow::Clicked(){auto* S=ULunarSettings::Get();if(Setting==TEXT("InvertY"))S->InvertY=!S->InvertY;if(Setting==TEXT("CameraMotion"))S->CameraMotion=!S->CameraMotion;if(Setting==TEXT("ToggleSprint"))S->ToggleSprint=!S->ToggleSprint;if(Setting==TEXT("Quality"))S->Quality=(S->Quality+1)%3;S->Apply(Player(this));Refresh();}
void ULunarSettingRow::Selected(FInputChord Chord){auto* S=ULunarSettings::Get();const bool Good=S->Rebind(Setting,Chord.Key);if(!Good)Selector->SetSelectedKey(FInputChord(S->Key(Setting)));ValueLabel->SetText(FText::FromString(Good?TEXT(" Saved"):TEXT(" Reserved / already used")));S->Apply(Player(this));}

TSharedRef<SWidget> ULunarPanel::RebuildWidget(){SetIsFocusable(true);Rebuild();return Super::RebuildWidget();}
void ULunarPanel::NativeConstruct(){Super::NativeConstruct();SetKeyboardFocus();}
void ULunarPanel::Rebuild(){
 Photo=nullptr;
 if(!Body){auto* Border=WidgetTree->ConstructWidget<UBorder>();Border->SetBrushColor(FLinearColor(.008,.02,.033,.99));Border->SetPadding(FMargin(32,20));WidgetTree->RootWidget=Border;auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Border->AddChild(Scroll);Body=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Body);}
 else Body->ClearChildren();
 auto Add=[&](UWidget* W){auto* RowLayout=Body->AddChildToVerticalBox(W);RowLayout->SetPadding(FMargin(4,7));};
 auto* G=Game(this);Add(Text(WidgetTree,Settings?TEXT("SETTINGS"):G&&G->Stage==ELunarStage::Won?TEXT("EXPEDITION DEBRIEF"):TEXT("EVIDENCE ALBUM"),30));
 auto* Top=WidgetTree->ConstructWidget<UHorizontalBox>();Add(Top);auto* Back=Button(WidgetTree,TEXT("Close / Esc / F1"));Back->OnClicked.AddDynamic(this,&ULunarPanel::Close);Top->AddChild(Back);
 if(Settings){
  auto* Reset=Button(WidgetTree,TEXT("Restore defaults"));Reset->OnClicked.AddDynamic(this,&ULunarPanel::ResetSettings);Top->AddChild(Reset);
  Add(Text(WidgetTree,TEXT("Changes save automatically. Text size is applied to the game when this panel closes. Scroll for key bindings. Esc and F1 always return to the game."),18));
  const TCHAR* Names[]={TEXT("Sensitivity"),TEXT("TextScale"),TEXT("SubtitleScale"),TEXT("Brightness"),TEXT("FieldOfView"),TEXT("EffectsVolume"),TEXT("RadioVolume"),TEXT("InvertY"),TEXT("CameraMotion"),TEXT("ToggleSprint"),TEXT("Quality"),TEXT("MoveForward"),TEXT("MoveBack"),TEXT("MoveLeft"),TEXT("MoveRight"),TEXT("Interact"),TEXT("Jump"),TEXT("Route"),TEXT("Journal"),TEXT("View"),TEXT("Sprint"),TEXT("Zoom")};
  for(auto* N:Names){auto* Row=CreateWidget<ULunarSettingRow>(GetOwningPlayer());Row->Setting=FName(N);Add(Row);}
  return;
 }
 if(!G)return;
 int Parts=0;for(int i=0;i<3;++i)if(G->ApolloParts&(1<<i))++Parts;
 Add(Text(WidgetTree,FString::Printf(TEXT("%s  |  %.0f m walked  |  %02d:%02d elapsed\nCargo: %d/3 samples, %d/3 Surveyor components. Context photographs: %d/4.\n%s"),G->ApolloTimed?TEXT("Timed exercise"):TEXT("Untimed historical exploration"),G->ApolloDistance,int(G->ApolloElapsed)/60,int(G->ApolloElapsed)%60,G->SampleCount(),Parts,G->ApolloEvidence.Num(),G->Stage==ELunarStage::Won?TEXT("Cargo stowed at Intrepid. The real crew reunited with Richard Gordon in Yankee Clipper and returned to Earth on 24 November 1969."):TEXT("Only committed work is listed. Cancelled work is never added to cargo.")),21));
 const TCHAR* Cargo[]={TEXT("12031 / Head: basalt rock; helps study lunar volcanic history."),TEXT("12032 / Bench: fine soil; records the surface regolith."),TEXT("12027 / Sharp: drive-tube core; preserves a shallow soil column."),TEXT("Surveyor TV camera: materials and mechanisms after lunar exposure."),TEXT("Surveyor scoop: returned hardware and adhering soil."),TEXT("Surveyor cable segment: returned material for laboratory examination.")};
 auto* Nav=WidgetTree->ConstructWidget<UHorizontalBox>();Add(Nav);auto* Prev=Button(WidgetTree,TEXT("< Previous photo"));auto* NextB=Button(WidgetTree,TEXT("Next photo >"));Prev->OnClicked.AddDynamic(this,&ULunarPanel::Previous);NextB->OnClicked.AddDynamic(this,&ULunarPanel::Next);Nav->AddChild(Prev);Nav->AddChild(NextB);
 EvidenceIndex=FMath::Clamp(EvidenceIndex,0,FMath::Max(0,G->ApolloEvidence.Num()-1));
 if(G->ApolloEvidence.IsValidIndex(EvidenceIndex)){
  const auto& E=G->ApolloEvidence[EvidenceIndex];FString Id=E.PhotoId.ToString(),Site=E.SiteId.ToString();Id.RemoveFromStart(TEXT("photo."));int Dot;if(Site.FindChar(TEXT('.'),Dot))Site=Site.Left(Dot);
  const FString Caption=E.PhotoId==FName(ApolloTasks::SurveyorPhoto)?TEXT("Surveyor 3 / before hardware removal"):FString::Printf(TEXT("Sample %s / %s Crater"),*Id,*Site.ToUpper());
  Add(Text(WidgetTree,FString::Printf(TEXT("%s  |  captured %02d:%02d"),*Caption,int(E.CapturedAt)/60,int(E.CapturedAt)%60),20));
  const FString Prefix=TEXT("Apollo12/Photos/")+G->ApolloRunId.ToString(EGuidFormats::Digits)+TEXT("/");
  if(E.PhotoPath.StartsWith(Prefix)&&!E.PhotoPath.Contains(TEXT("..")))Photo=FImageUtils::ImportFileAsTexture2D(FPaths::ProjectSavedDir()/E.PhotoPath);
  if(Photo){auto* Image=WidgetTree->ConstructWidget<UImage>();Image->SetBrushFromTexture(Photo,true);auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetHeightOverride(320);auto* Scale=WidgetTree->ConstructWidget<UScaleBox>();Scale->SetStretch(EStretch::ScaleToFit);Scale->AddChild(Image);Size->AddChild(Scale);Add(Size);}
  else Add(Text(WidgetTree,TEXT("Photo file unavailable. The saved evidence metadata and completed work are preserved."),20));
 }else Add(Text(WidgetTree,TEXT("No context photographs yet. At a sample site, open the camera and frame the target before releasing the shutter."),20));
 for(int i=0;i<6;++i)Add(Text(WidgetTree,FString(i<3?(G->SamplesCollected&(1<<i)?TEXT("[PACKED] "):TEXT("[PENDING] ")):(G->ApolloParts&(1<<(i-3))?TEXT("[PACKED] "):TEXT("[PENDING] ")))+Cargo[i],18));
 Add(Text(WidgetTree,TEXT("Historical note: this chapter combines selected work from two EVAs. Tool motions, single-player operation and simplified ALSEP alignment are authored gameplay. Core 12027 used a drive tube, not a powered drill. Terrain uses modern LROC orbital data. Sources: nasa.gov/history/alsj/a12/ and the bundled Credits/Apollo12 files."),17));
 if(!G->ApolloTranscript.IsEmpty())Add(Text(WidgetTree,TEXT("RECENT RADIO SUBTITLES\n")+FString::Join(G->ApolloTranscript,TEXT("\n")),18));
}
FReply ULunarPanel::NativeOnKeyDown(const FGeometry& Geo,const FKeyEvent& E){if(E.GetKey()==EKeys::Escape||E.GetKey()==EKeys::F1){Close();return FReply::Handled();}if(!Settings&&(E.GetKey()==EKeys::Right||E.GetKey()==EKeys::Q)){Next();return FReply::Handled();}if(!Settings&&E.GetKey()==EKeys::Left){Previous();return FReply::Handled();}return Super::NativeOnKeyDown(Geo,E);}
void ULunarPanel::Close(){if(auto* P=Player(this))P->ClosePanel();}
void ULunarPanel::Previous(){if(auto* G=Game(this))EvidenceIndex=(EvidenceIndex+FMath::Max(1,G->ApolloEvidence.Num())-1)%FMath::Max(1,G->ApolloEvidence.Num());Rebuild();}
void ULunarPanel::Next(){if(auto* G=Game(this))EvidenceIndex=(EvidenceIndex+1)%FMath::Max(1,G->ApolloEvidence.Num());Rebuild();}
void ULunarPanel::ResetSettings(){ULunarSettings::Get()->Reset();ULunarSettings::Get()->Apply(Player(this));Rebuild();}
