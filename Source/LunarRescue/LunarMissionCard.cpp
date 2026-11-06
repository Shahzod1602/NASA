#include "LunarMissionCard.h"
#include "LunarSettings.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"

namespace {
constexpr float SelectionDuration=.28f;
UTextBlock* CardText(UWidgetTree* Tree,int32 Size,FLinearColor Color){
 auto* Text=Tree->ConstructWidget<UTextBlock>();auto Font=Text->GetFont();
 Font.Size=FMath::RoundToInt(Size*ULunarSettings::Get()->TextScale);Font.TypefaceFontName=TEXT("Regular");
 Text->SetFont(Font);Text->SetColorAndOpacity(Color);Text->SetAutoWrapText(true);
 Text->SetVisibility(ESlateVisibility::HitTestInvisible);return Text;
}
UBorder* Layer(UWidgetTree* Tree,UOverlay* Overlay,float Inset,float Radius,float Outline=0){
 auto* Border=Tree->ConstructWidget<UBorder>();
 Border->SetBrush(Outline>0?FSlateRoundedBoxBrush(FLinearColor::Transparent,Radius,FLinearColor::White,Outline):FSlateRoundedBoxBrush(FLinearColor::White,Radius));
 Border->SetPadding(FMargin(0));Border->SetVisibility(ESlateVisibility::HitTestInvisible);
 auto* Slot=Overlay->AddChildToOverlay(Border);Slot->SetHorizontalAlignment(HAlign_Fill);Slot->SetVerticalAlignment(VAlign_Fill);Slot->SetPadding(FMargin(Inset));return Border;
}
}

ULunarMissionButton::ULunarMissionButton(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer){InitIsFocusable(false);}

ULunarFrontendButton::ULunarFrontendButton(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer){
 InitIsFocusable(true);OnReceivedFocus.BindUObject(this,&ULunarFrontendButton::FocusGlass);OnLostFocus.BindUObject(this,&ULunarFrontendButton::UnfocusGlass);
}
void ULunarFrontendButton::SetGlassStyle(const FButtonStyle& Style){UnfocusedStyle=Style;SetStyle(Style);}
void ULunarFrontendButton::FocusGlass(){auto Focused=UnfocusedStyle;Focused.Normal=Focused.Hovered;SetStyle(Focused);}
void ULunarFrontendButton::UnfocusGlass(){SetStyle(UnfocusedStyle);}
bool ULunarFrontendButton::ValidateSurface(bool Arranged,FString& Reason)const{
 auto* Layers=Cast<UOverlay>(GetContent());auto* ButtonSlot=Layers?Cast<UButtonSlot>(Layers->Slot):nullptr;
 auto* FaceSlot=GlassSurface?Cast<UOverlaySlot>(GlassSurface->Slot):nullptr;
 if(!GetIsFocusable()||!ButtonSlot||!FaceSlot||!Caption||ButtonSlot->GetHorizontalAlignment()!=HAlign_Fill||ButtonSlot->GetVerticalAlignment()!=VAlign_Fill||FaceSlot->GetHorizontalAlignment()!=HAlign_Fill||FaceSlot->GetVerticalAlignment()!=VAlign_Fill||GlassSurface->GetHorizontalAlignment()!=HAlign_Center||GlassSurface->GetVerticalAlignment()!=VAlign_Center){Reason=TEXT("Action glass, label or hit-area alignment is inconsistent.");return false;}
 if(Arranged){
  const auto& ButtonGeometry=GetCachedGeometry();const auto& FaceGeometry=GlassSurface->GetCachedGeometry();const auto& TextGeometry=Caption->GetCachedGeometry();
  const FVector2D ButtonSize=ButtonGeometry.GetLocalSize(),FaceSize=FaceGeometry.GetLocalSize();
  const FVector2D ButtonCentre=ButtonGeometry.LocalToAbsolute(ButtonSize*.5f),FaceCentre=FaceGeometry.LocalToAbsolute(FaceSize*.5f),TextCentre=TextGeometry.LocalToAbsolute(TextGeometry.GetLocalSize()*.5f);
  if(ButtonSize.X<=0||ButtonSize.Y<=0||!ButtonSize.Equals(FaceSize,.5f)||!ButtonCentre.Equals(FaceCentre,.5f)||!ButtonCentre.Equals(TextCentre,1.f)){Reason=TEXT("Arranged action fill or caption does not cover and centre on its outlined box.");return false;}
 }
 Reason.Reset();return true;
}

void ULunarMissionCard::Configure(int32 Id,const FString& Title,const FString& Subtitle,const FString& Description,bool Compact){
 CardId=Id;CardTitle=Title;CardSubtitle=Subtitle;CardDescription=Description;CompactState=Compact;RefreshText();
}
TSharedRef<SWidget> ULunarMissionCard::RebuildWidget(){
 FullGlassBaseline=FParse::Param(FCommandLine::Get(),TEXT("MissionFullGlassBaseline"));
 Button=WidgetTree->ConstructWidget<ULunarMissionButton>();auto Style=Button->GetStyle();
 Style.Normal.DrawAs=Style.Hovered.DrawAs=Style.Pressed.DrawAs=Style.Disabled.DrawAs=ESlateBrushDrawType::NoDrawType;
 Style.NormalPadding=Style.PressedPadding=FMargin(0);Button->SetStyle(Style);
 Button->OnClicked.AddDynamic(this,&ULunarMissionCard::Choose);Button->OnHovered.AddDynamic(this,&ULunarMissionCard::Hover);Button->OnUnhovered.AddDynamic(this,&ULunarMissionCard::Unhover);
 Button->OnReceivedFocus.BindUObject(this,&ULunarMissionCard::Focus);Button->OnLostFocus.BindUObject(this,&ULunarMissionCard::Unfocus);
 auto* Layers=WidgetTree->ConstructWidget<UOverlay>();Layers->SetVisibility(ESlateVisibility::SelfHitTestInvisible);Button->AddChild(Layers);
 if(auto* ButtonLayout=Cast<UButtonSlot>(Layers->Slot)){ButtonLayout->SetHorizontalAlignment(HAlign_Fill);ButtonLayout->SetVerticalAlignment(VAlign_Fill);ButtonLayout->SetPadding(FMargin(0));}
 GlowOuter=Layer(WidgetTree,Layers,0,8,5);GlowInner=Layer(WidgetTree,Layers,2,6,3);
 Frost=WidgetTree->ConstructWidget<UBackgroundBlur>();Frost->SetVisibility(ESlateVisibility::HitTestInvisible);Frost->SetPadding(FMargin(0));Frost->SetCornerRadius(FVector4(4));Frost->SetApplyAlphaToBlur(false);
 Frost->SetLowQualityFallbackBrush(FSlateRoundedBoxBrush(FLinearColor(.012f,.05f,.082f,.84f),4.f));
 auto* FrostSlot=Layers->AddChildToOverlay(Frost);FrostSlot->SetHorizontalAlignment(HAlign_Fill);FrostSlot->SetVerticalAlignment(VAlign_Fill);FrostSlot->SetPadding(FMargin(4));
 Face=Layer(WidgetTree,Layers,4,4);Edge=Layer(WidgetTree,Layers,4,4,1);
 Glint=WidgetTree->ConstructWidget<UBorder>();Glint->SetVisibility(ESlateVisibility::HitTestInvisible);
 auto* GlintSize=WidgetTree->ConstructWidget<USizeBox>();GlintSize->SetHeightOverride(1);GlintSize->AddChild(Glint);
 auto* GlintSlot=Layers->AddChildToOverlay(GlintSize);GlintSlot->SetHorizontalAlignment(HAlign_Fill);GlintSlot->SetVerticalAlignment(VAlign_Top);GlintSlot->SetPadding(FMargin(12,5,12,0));
 Accent=WidgetTree->ConstructWidget<UBorder>();Accent->SetVisibility(ESlateVisibility::HitTestInvisible);
 auto* AccentSize=WidgetTree->ConstructWidget<USizeBox>();AccentSize->SetWidthOverride(2);AccentSize->AddChild(Accent);
 auto* AccentSlot=Layers->AddChildToOverlay(AccentSize);AccentSlot->SetHorizontalAlignment(HAlign_Left);AccentSlot->SetVerticalAlignment(VAlign_Fill);AccentSlot->SetPadding(FMargin(4,CompactState?7:18,0,CompactState?7:18));
 const FLinearColor Ink(.91f,.96f,1),Dim(.61f,.75f,.84f);
 TitleLabel=CardText(WidgetTree,CompactState?12:23,Ink);
 if(CompactState){
  TitleLabel->SetAutoWrapText(false);auto* TitleSlot=Layers->AddChildToOverlay(TitleLabel);TitleSlot->SetHorizontalAlignment(HAlign_Fill);TitleSlot->SetVerticalAlignment(VAlign_Center);TitleSlot->SetPadding(FMargin(16,0,12,0));
 }else{
  auto* Stack=WidgetTree->ConstructWidget<UVerticalBox>();Stack->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
  auto* ContentSlot=Layers->AddChildToOverlay(Stack);ContentSlot->SetHorizontalAlignment(HAlign_Fill);ContentSlot->SetVerticalAlignment(VAlign_Fill);ContentSlot->SetPadding(FMargin(24,24,24,56));
  SubtitleLabel=CardText(WidgetTree,11,Dim);Stack->AddChildToVerticalBox(SubtitleLabel)->SetPadding(FMargin(0,0,0,12));
  Stack->AddChildToVerticalBox(TitleLabel)->SetPadding(FMargin(0,0,0,16));
  // Keep the title and selection state fixed while large text scrolls inside
  // the remaining card area; the footer never overlaps the description.
  auto* DescriptionScroll=WidgetTree->ConstructWidget<UScrollBox>();
  DescriptionScroll->SetIsFocusable(false);DescriptionScroll->SetAllowOverscroll(false);DescriptionScroll->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
  DescriptionScroll->SetScrollbarThickness(FVector2D(3,3));DescriptionScroll->SetScrollbarPadding(FMargin(4,0,0,0));
  Stack->AddChildToVerticalBox(DescriptionScroll)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  DescriptionLabel=CardText(WidgetTree,14,Ink);DescriptionScroll->AddChild(DescriptionLabel);
  SelectionLabel=CardText(WidgetTree,10,Dim);
  auto* StatusSlot=Layers->AddChildToOverlay(SelectionLabel);StatusSlot->SetHorizontalAlignment(HAlign_Fill);StatusSlot->SetVerticalAlignment(VAlign_Bottom);StatusSlot->SetPadding(FMargin(24,0,24,22));
 }
 WidgetTree->RootWidget=Button;RefreshText();ApplyPresentation();return Super::RebuildWidget();
}
void ULunarMissionCard::RefreshText(){
 CachedTextScale=ULunarSettings::Get()->TextScale;
 auto Resize=[&](UTextBlock* Text,int32 Size){if(Text){auto Font=Text->GetFont();Font.Size=FMath::RoundToInt(Size*CachedTextScale);Text->SetFont(Font);}};
 Resize(TitleLabel,CompactState?12:23);Resize(SubtitleLabel,11);Resize(DescriptionLabel,14);Resize(SelectionLabel,10);
 if(TitleLabel)TitleLabel->SetText(FText::FromString(CardTitle));
 if(SubtitleLabel)SubtitleLabel->SetText(FText::FromString(CardSubtitle));
 if(DescriptionLabel)DescriptionLabel->SetText(FText::FromString(CardDescription));
 if(SelectionLabel)SelectionLabel->SetText(FText::FromString(SelectedState?TEXT("SELECTED"):TEXT("SELECT MISSION")));
}
void ULunarMissionCard::SetSelected(bool Selected){
 if(SelectedState==Selected)return;SelectedState=Selected;SelectionStart=SelectionValue;SelectionElapsed=0;RefreshText();
}
void ULunarMissionCard::Choose(){if(GetIsEnabled()&&(!Button||Button->GetIsEnabled()))OnChosen.Broadcast(CardId);}
void ULunarMissionCard::Hover(){HoveredState=true;if(GetIsEnabled())OnPreview.Broadcast(CardId);}
void ULunarMissionCard::Unhover(){HoveredState=false;}
void ULunarMissionCard::Focus(){FocusedState=true;if(GetIsEnabled())OnPreview.Broadcast(CardId);}
void ULunarMissionCard::Unfocus(){FocusedState=false;}
void ULunarMissionCard::NativeTick(const FGeometry& Geometry,float Delta){
 Super::NativeTick(Geometry,Delta);
 if(!FMath::IsNearlyEqual(CachedTextScale,ULunarSettings::Get()->TextScale,.001f))RefreshText();
 AdvancePresentation(Delta);
}
void ULunarMissionCard::AdvancePresentation(float Delta){
 if(!FMath::IsFinite(Delta)||Delta<=0)return;
 const float OldSelection=SelectionValue,OldHover=HoverValue;
 SelectionElapsed=FMath::Min(SelectionDuration,SelectionElapsed+Delta);
 // Saturate explicitly: optimized packaged float division/lerp need not
 // produce the exact endpoint that persistent selection and idle state use.
 if(SelectionElapsed>=SelectionDuration)SelectionValue=SelectedState?1.f:0.f;
 else{
  const float T=FMath::Clamp(SelectionElapsed/SelectionDuration,0.f,1.f);
  const float Ease=T*T*(3-2*T);
  SelectionValue=FMath::Clamp(FMath::Lerp(SelectionStart,SelectedState?1.f:0.f,Ease),0.f,1.f);
 }
 const float HoverTarget=GetIsEnabled()&&(HoveredState||FocusedState)?1.f:0.f;
 HoverValue=FMath::FInterpConstantTo(HoverValue,HoverTarget,Delta,6.f);
 if(OldSelection!=SelectionValue||OldHover!=HoverValue)ApplyPresentation();
}
void ULunarMissionCard::ApplyPresentation(){
 if(!Face)return;++PresentationUpdates;
 // Three bounded glow layers, not a looping pulse. Once settled, no brush updates.
 const float Blue=FMath::Max(SelectionValue,HoverValue*.45f);
 Face->SetBrushColor(FMath::Lerp(FLinearColor(.012f,.024f,.035f,.8f),FLinearColor(.015f,.19f,.34f,.62f),Blue));
 Edge->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent,4.f,FMath::Lerp(FLinearColor(.17f,.28f,.35f,.5f),FLinearColor(.08f,.69f,1.f,.95f),Blue),1.f));
 GlowOuter->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent,8.f,FLinearColor(.025f,.34f,1.f,.12f*Blue),5.f));
 GlowInner->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent,6.f,FLinearColor(.025f,.46f,1.f,.22f*Blue),3.f));
 Glint->SetBrushColor(FLinearColor(.55f,.88f,1.f,.08f+.28f*Blue));
 Accent->SetBrushColor(FLinearColor(.035f,.8f,1.f,.14f+.86f*Blue));
 // Fixed frost keeps the selected glass while avoiding animated blur-kernel changes.
 const float BlurStrength=FullGlassBaseline?2.f+3.f*Blue:2.f;
 if(!FMath::IsNearlyEqual(Frost->GetBlurStrength(),BlurStrength))Frost->SetBlurStrength(BlurStrength);
 if(SelectionLabel)SelectionLabel->SetColorAndOpacity(FMath::Lerp(FLinearColor(.52f,.67f,.76f),FLinearColor(.28f,.87f,1.f),SelectionValue));
}
