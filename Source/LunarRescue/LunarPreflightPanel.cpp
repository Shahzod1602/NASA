#include "LunarOperationsPanel.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "Misc/PackageName.h"
#include "Styling/CoreStyle.h"

namespace {
const FLinearColor Ink(.93f,.95f,.96f),Dim(.65f,.69f,.72f),Cyan(.02f,.83f,.88f),Amber(1.f,.73f,.23f);
float Scale(){const float Value=ULunarSettings::Get()->TextScale;return FMath::IsFinite(Value)?FMath::Clamp(Value,1.f,1.5f):1.f;}
FName Element(const TCHAR* Kind,int32 Generation,int32 Index=0){return FName(*FString::Printf(TEXT("EVA%s%d_%d"),Kind,Generation,Index));}
void Label(UTextBlock* Text,const FString& Value,FLinearColor Color){if(!Text)return;if(Text->GetText().ToString()!=Value)Text->SetText(FText::FromString(Value));Text->SetColorAndOpacity(Color);}
}

void ULunarOperationsPanel::UpdateCheckProgress(){
 auto* C=Operations();if(View!=EFieldPanelView::Preflight||!C||!WidgetTree)return;
 const int32 Step=FMath::Clamp(C->PreflightCursor,0,3);int32 Completed=0;
 for(int32 I=0;I<4;++I){
  const bool Done=(C->PreflightMask&(1<<I))!=0,Running=C->PendingCheck==I;
  const float Progress=Done?1.f:Running?C->ActionProgress():0.f;if(Done)++Completed;
  if(auto* Bar=Cast<UProgressBar>(WidgetTree->FindWidget(Element(TEXT("Progress"),Generation,I))))Bar->SetPercent(Progress);
  auto* State=Cast<UTextBlock>(WidgetTree->FindWidget(Element(TEXT("State"),Generation,I)));
  Label(State,Done?TEXT("Complete / 100%"):Running?FString::Printf(TEXT("Checking / %d%%"),FMath::FloorToInt(Progress*100.f+.001f)):I==Step?TEXT("Ready / 0%"):TEXT("Locked"),Done?Cyan:Running?Amber:I==Step?Ink:Dim);
  Label(Cast<UTextBlock>(WidgetTree->FindWidget(Element(TEXT("Mark"),Generation,I))),Done?TEXT("\u2713"):I==Step?TEXT("\u2022"):TEXT("-"),Done?Cyan:I==Step?Ink:Dim);
 }
 Label(Cast<UTextBlock>(WidgetTree->FindWidget(Element(TEXT("Completion"),Generation))),FString::Printf(TEXT("%d / 4 complete"),Completed),Ink);
}

void ULunarOperationsPanel::BuildExpeditionHeader(){
 auto* C=Operations();if(!C)return;
 const float S=Scale(),Width=ContentWidth(),D=FMath::Clamp(Width/1208.f,.7f,2.5f);const bool Wide=Width>950&&S<1.3f;
 auto Add=[D](UVerticalBox* Box,UWidget* Child,FMargin ItemPadding=FMargin(0,5)){auto* Slot=Box->AddChildToVerticalBox(Child);Slot->SetPadding(FMargin(ItemPadding.Left*D,ItemPadding.Top*D,ItemPadding.Right*D,ItemPadding.Bottom*D));return Slot;};
 auto Box=[&](UWidget* Child,float W,float H){auto* Size=WidgetTree->ConstructWidget<USizeBox>();if(W>0)Size->SetWidthOverride(W*D);if(H>0)Size->SetHeightOverride(H*D);Size->SetContent(Child);return Size;};
 auto Plain=[D,S](UButton* B,FLinearColor Normal,FLinearColor Hover,FLinearColor Disabled=FLinearColor(.12f,.13f,.14f,.95f)){
  auto Style=B->GetStyle();Style.Normal.DrawAs=Style.Hovered.DrawAs=Style.Pressed.DrawAs=Style.Disabled.DrawAs=ESlateBrushDrawType::Box;
  Style.Normal.TintColor=FSlateColor(Normal);Style.Hovered.TintColor=FSlateColor(Hover);Style.Pressed.TintColor=FSlateColor(Hover);Style.Disabled.TintColor=FSlateColor(Disabled);Style.NormalPadding=Style.PressedPadding=FMargin(8*D,6*D);B->SetStyle(Style);B->SetBackgroundColor(FLinearColor::White);if(auto* Caption=Cast<UTextBlock>(B->GetContent()))Caption->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(17*S*D*.8f)));
 };
 auto DisplayText=[&](const FString& Value,int32 FontSize,FLinearColor Color){return Text(Value,FMath::RoundToInt(FontSize*D*.8f),Color);};
 auto* Header=WidgetTree->ConstructWidget<UHorizontalBox>();auto* Brand=DisplayText(TEXT("LUNARRESCUE"),20,Ink);Brand->SetWrapTextAt(235*S*D);Brand->SetAutoWrapText(!Wide);auto* BrandSlot=Header->AddChildToHorizontalBox(Brand);BrandSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));BrandSlot->SetVerticalAlignment(VAlign_Center);
 UWrapBox* CompactTabs=nullptr;if(!Wide){CompactTabs=WidgetTree->ConstructWidget<UWrapBox>();CompactTabs->SetExplicitWrapSize(true);CompactTabs->SetWrapSize(Width-50);CompactTabs->SetInnerSlotPadding(FVector2D(6,4));}
 for(const auto& Item:TArray<TPair<FString,EFieldPanelView>>{{TEXT("EVA checks"),EFieldPanelView::Preflight},{TEXT("Map"),EFieldPanelView::Resources},{TEXT("Inventory"),EFieldPanelView::Inventory}}){
  auto* Tab=Button(Item.Key,TEXT("View"),int32(Item.Value),C->CanOpenView(Item.Value),Item.Value==View);Plain(Tab,FLinearColor::Transparent,FLinearColor(1,1,1,.07f));
  auto* TabBody=WidgetTree->ConstructWidget<UVerticalBox>();Add(TabBody,Box(Tab,160*S,46*S),FMargin(4,0));
  auto* Underline=WidgetTree->ConstructWidget<UBorder>();Underline->SetPadding(FMargin(0));Underline->SetBrushColor(Item.Value==View?Cyan:FLinearColor::Transparent);Add(TabBody,Box(Underline,0,3),FMargin(4,0));if(CompactTabs)CompactTabs->AddChild(TabBody);else Header->AddChildToHorizontalBox(TabBody)->SetVerticalAlignment(VAlign_Center);
 }
 auto* Close=Button(TEXT("Esc / Close"),TEXT("Close"));Plain(Close,FLinearColor::Transparent,FLinearColor(1,1,1,.07f));Header->AddChildToHorizontalBox(Box(Close,135*S,46*S))->SetVerticalAlignment(VAlign_Center);
 auto* HeaderPlate=WidgetTree->ConstructWidget<UBorder>();HeaderPlate->SetBrushColor(FLinearColor(.012f,.014f,.016f,.88f));HeaderPlate->SetPadding(FMargin(14*D,8*D));if(CompactTabs){auto* Stack=WidgetTree->ConstructWidget<UVerticalBox>();Add(Stack,Header);Add(Stack,CompactTabs);HeaderPlate->SetContent(Stack);}else HeaderPlate->SetContent(Header);Row(HeaderPlate,0);

}

void ULunarOperationsPanel::BuildExpeditionFooter(){
 auto* C=Operations();if(!C)return;
 const float S=Scale(),Width=ContentWidth(),D=FMath::Clamp(Width/1208.f,.7f,2.5f);const bool Wide=Width>950&&S<1.3f;
 auto Add=[D](UVerticalBox* Box,UWidget* Child,FMargin ItemPadding=FMargin(0,5)){auto* Slot=Box->AddChildToVerticalBox(Child);Slot->SetPadding(FMargin(ItemPadding.Left*D,ItemPadding.Top*D,ItemPadding.Right*D,ItemPadding.Bottom*D));return Slot;};
 auto Box=[&](UWidget* Child,float W,float H){auto* Size=WidgetTree->ConstructWidget<USizeBox>();if(W>0)Size->SetWidthOverride(W*D);if(H>0)Size->SetHeightOverride(H*D);Size->SetContent(Child);return Size;};
 auto Plain=[D,S](UButton* B,FLinearColor Normal,FLinearColor Hover,FLinearColor Disabled=FLinearColor(.12f,.13f,.14f,.95f)){
  auto Style=B->GetStyle();Style.Normal.DrawAs=Style.Hovered.DrawAs=Style.Pressed.DrawAs=Style.Disabled.DrawAs=ESlateBrushDrawType::Box;
  Style.Normal.TintColor=FSlateColor(Normal);Style.Hovered.TintColor=FSlateColor(Hover);Style.Pressed.TintColor=FSlateColor(Hover);Style.Disabled.TintColor=FSlateColor(Disabled);Style.NormalPadding=Style.PressedPadding=FMargin(8*D,6*D);B->SetStyle(Style);B->SetBackgroundColor(FLinearColor::White);if(auto* Caption=Cast<UTextBlock>(B->GetContent()))Caption->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(17*S*D*.8f)));
 };
 auto DisplayText=[&](const FString& Value,int32 FontSize,FLinearColor Color){return Text(Value,FMath::RoundToInt(FontSize*D*.8f),Color);};
 auto* ArtNote=DisplayText(TEXT("Training simulation / AI-generated illustration"),13,Ink);ArtNote->SetWrapTextAt(Wide?Width*.52f:Width-45);auto* Sources=Button(ShowSources?TEXT("Hide Science & Sources"):TEXT("Open Science & Sources"),TEXT("Sources"));Plain(Sources,FLinearColor(.015f,.02f,.025f,.75f),FLinearColor(.06f,.12f,.14f,.9f));if(Wide){auto* Footer=WidgetTree->ConstructWidget<UHorizontalBox>();Footer->AddChildToHorizontalBox(ArtNote)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Footer->AddChildToHorizontalBox(Sources);Row(Footer,14);}else{Row(ArtNote,8);Row(Sources,8);}
 auto* Hardware=Button(TEXT("Mission hardware"),TEXT("Hardware"));Plain(Hardware,FLinearColor(.015f,.02f,.025f,.75f),FLinearColor(.06f,.12f,.14f,.9f));Row(Hardware,4);
 if(ShowSources){auto* Anchor=Text(TEXT("Mission hardware and science sources"),20,Ink);Anchor->Rename(*FString::Printf(TEXT("HardwareAnchor%d"),Generation));Row(Anchor);auto* Detail=WidgetTree->ConstructWidget<UBorder>();Detail->SetBrushColor(FLinearColor(.015f,.02f,.025f,.98f));Detail->SetPadding(FMargin(18));Detail->SetContent(Text(C->SourcesText()+TEXT("\n\nPre-EVA cabin: AI-generated team reference with OpenAI imagegen cleanup. Supplied demo artwork/video: team-confirmed Higgsfield creations. The cinematic is a fictional portrayal, not NASA archive footage or institutional endorsement."),16,Ink));Row(Detail);}
}

void ULunarOperationsPanel::BuildPreflight(){
 auto* C=Operations();if(!C)return;const auto* Mission=C->Game();
 const float S=Scale(),Width=ContentWidth(),D=FMath::Clamp(Width/1208.f,.7f,2.5f);const bool Wide=Width>950&&S<1.3f;
 const bool CanMutate=C->Preparing&&C->CanUsePanel(this,true),Ready=CanMutate&&C->Prepared&&C->PreflightMask==15&&C->PendingCheck==INDEX_NONE;
 const int32 Step=FMath::Clamp(C->PreflightCursor,0,3);
 auto Add=[D](UVerticalBox* Box,UWidget* Child,FMargin ItemPadding=FMargin(0,5)){auto* Slot=Box->AddChildToVerticalBox(Child);Slot->SetPadding(FMargin(ItemPadding.Left*D,ItemPadding.Top*D,ItemPadding.Right*D,ItemPadding.Bottom*D));return Slot;};
 auto Box=[&](UWidget* Child,float W,float H){auto* Size=WidgetTree->ConstructWidget<USizeBox>();if(W>0)Size->SetWidthOverride(W*D);if(H>0)Size->SetHeightOverride(H*D);Size->SetContent(Child);return Size;};
 auto Plain=[D,S](UButton* B,FLinearColor Normal,FLinearColor Hover,FLinearColor Disabled=FLinearColor(.12f,.13f,.14f,.95f)){
  auto Style=B->GetStyle();Style.Normal.DrawAs=Style.Hovered.DrawAs=Style.Pressed.DrawAs=Style.Disabled.DrawAs=ESlateBrushDrawType::Box;
  Style.Normal.TintColor=FSlateColor(Normal);Style.Hovered.TintColor=FSlateColor(Hover);Style.Pressed.TintColor=FSlateColor(Hover);Style.Disabled.TintColor=FSlateColor(Disabled);Style.NormalPadding=Style.PressedPadding=FMargin(8*D,6*D);B->SetStyle(Style);B->SetBackgroundColor(FLinearColor::White);if(auto* Caption=Cast<UTextBlock>(B->GetContent()))Caption->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(17*S*D*.8f)));
 };
 auto DisplayText=[&](const FString& Value,int32 FontSize,FLinearColor Color){return Text(Value,FMath::RoundToInt(FontSize*D*.8f),Color);};
 BuildExpeditionHeader();

 auto* Overview=WidgetTree->ConstructWidget<UVerticalBox>();auto* Title=DisplayText(TEXT("Pre-EVA checks"),36,Ink);Title->SetWrapTextAt(Wide?Width*.46f:Width-40);Add(Overview,Title,FMargin(12,18,12,6));
 auto* Help=DisplayText(TEXT("Prepare for surface operations."),18,Ink);Help->SetWrapTextAt(Wide?Width*.43f:Width-40);Add(Overview,Help,FMargin(12,0,12,12));
 auto* MissionName=DisplayText(Mission&&Mission->Mars?TEXT("ARES / MARS"):Mission&&Mission->Apollo?TEXT("APOLLO 12 / MOON"):TEXT("SELENE / MOON"),15,Cyan);Add(Overview,MissionName,FMargin(12,0));
 auto* Left=WidgetTree->ConstructWidget<USizeBox>();if(Wide)Left->SetMinDesiredHeight(470*D);Left->SetContent(Overview);

 auto* Checklist=WidgetTree->ConstructWidget<UVerticalBox>();
 auto* CardTitle=WidgetTree->ConstructWidget<UHorizontalBox>();auto* Heading=DisplayText(TEXT("Suit checklist"),26,Ink);Heading->SetWrapTextAt(Wide?Width*.27f:Width*.55f);CardTitle->AddChildToHorizontalBox(Heading)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
 auto* Count=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Element(TEXT("Completion"),Generation));Count->SetFont(Heading->GetFont());Count->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(17*S*D*.8f)));Count->SetColorAndOpacity(Ink);CardTitle->AddChildToHorizontalBox(Count)->SetVerticalAlignment(VAlign_Center);Add(Checklist,CardTitle,FMargin(0,0,0,16));
 UTexture2D* IconAtlas=nullptr;if(FPackageName::DoesPackageExist(TEXT("/Game/Lunar/Frontend/Demo20261115/T_PreflightReference")))IconAtlas=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/Demo20261115/T_PreflightReference.T_PreflightReference"));
 const FBox2D Regions[]={FBox2D(FVector2D(741,209),FVector2D(799,249)),FBox2D(FVector2D(741,264),FVector2D(797,313)),FBox2D(FVector2D(741,325),FVector2D(799,377)),FBox2D(FVector2D(739,394),FVector2D(800,437))};
 const TCHAR* Names[]={TEXT("Suit seals"),TEXT("Life-support pack"),TEXT("Communications"),TEXT("Internal power")};
 for(int32 I=0;I<4;++I){
  const bool Done=(C->PreflightMask&(1<<I))!=0;auto* Line=WidgetTree->ConstructWidget<UHorizontalBox>();
  auto* Mark=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Element(TEXT("Mark"),Generation,I));Mark->SetFont(FCoreStyle::GetDefaultFontStyle("Bold",FMath::RoundToInt(24*S*D*.8f)));Line->AddChildToHorizontalBox(Box(Mark,28*S,0))->SetVerticalAlignment(VAlign_Center);
  if(IconAtlas){auto* Icon=WidgetTree->ConstructWidget<UImage>();FSlateBrush Brush;Brush.SetResourceObject(IconAtlas);Brush.DrawAs=ESlateBrushDrawType::Image;Brush.SetUVRegion(FBox2D(Regions[I].Min/FVector2D(1280,720),Regions[I].Max/FVector2D(1280,720)));Brush.ImageSize=FVector2D(48,44);Icon->SetBrush(Brush);Icon->SetVisibility(ESlateVisibility::HitTestInvisible);Line->AddChildToHorizontalBox(Box(Icon,48*S,44*S))->SetPadding(FMargin(0,0,10,0));}
  auto* Check=Button(FString::Printf(TEXT("%d / %s"),I+1,Names[I]),TEXT("Check"),I,CanMutate&&!Done&&C->PendingCheck==INDEX_NONE&&I==Step,I==Step);Plain(Check,FLinearColor::Transparent,FLinearColor(1,1,1,.08f),FLinearColor::Transparent);
  if(auto* Caption=Cast<UTextBlock>(Check->GetContent())){Caption->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(20*S*D*.8f)));Caption->SetWrapTextAt(Wide?Width*.22f:Width*.53f);Caption->SetColorAndOpacity(Done?Ink:I==Step?Ink:Dim);if(auto* ContentSlot=Cast<UButtonSlot>(Caption->Slot))ContentSlot->SetHorizontalAlignment(HAlign_Left);}
  auto* CheckSlot=Line->AddChildToHorizontalBox(Check);CheckSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));CheckSlot->SetVerticalAlignment(VAlign_Center);
  auto* State=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Element(TEXT("State"),Generation,I));State->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(14*S*D*.8f)));State->SetWrapTextAt(108*S*D);State->SetJustification(ETextJustify::Right);Line->AddChildToHorizontalBox(Box(State,108*S,0))->SetVerticalAlignment(VAlign_Center);Add(Checklist,Box(Line,0,55*S),FMargin(0,2));
  auto* Progress=WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(),Element(TEXT("Progress"),Generation,I));auto ProgressStyle=Progress->GetWidgetStyle();ProgressStyle.BackgroundImage.TintColor=FSlateColor(FLinearColor(.14f,.15f,.16f));Progress->SetWidgetStyle(ProgressStyle);Progress->SetFillColorAndOpacity(Cyan);Add(Checklist,Box(Progress,0,5*S),FMargin(0,0,0,10));
  if(I==Step){CheckProgress=Progress;CheckProgressLabel=State;}
 }
 auto* Hatch=Button(TEXT("Open hatch"),TEXT("Launch"),0,Ready,Ready);Plain(Hatch,Ready?Cyan:FLinearColor(.16f,.17f,.18f,.96f),FLinearColor(.14f,.95f,.98f),FLinearColor(.16f,.17f,.18f,.96f));
 if(auto* Caption=Cast<UTextBlock>(Hatch->GetContent())){Caption->SetColorAndOpacity(Ready?FLinearColor(.004f,.018f,.022f):Dim);Caption->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(22*S*D*.8f)));}Add(Checklist,Box(Hatch,0,54*S),FMargin(0,16,0,8));
 auto* Finish=DisplayText(Ready?TEXT("All checks complete. Ready for surface operations."):TEXT("Finish the checks to continue"),15,Ready?Cyan:Dim);Finish->SetJustification(ETextJustify::Center);Finish->SetWrapTextAt(Wide?Width*.45f-55:Width-90);Add(Checklist,Finish,FMargin(0,2));
 auto* Card=WidgetTree->ConstructWidget<UBorder>();Card->SetBrushColor(FLinearColor(.017f,.019f,.021f,.96f));Card->SetPadding(FMargin(26*D,24*D));Card->SetContent(Checklist);
 if(Wide){auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();FSlateChildSize L(ESlateSizeRule::Fill);L.Value=.5f;FSlateChildSize R(ESlateSizeRule::Fill);R.Value=.5f;Columns->AddChildToHorizontalBox(Left)->SetSize(L);auto* Right=Columns->AddChildToHorizontalBox(Card);Right->SetSize(R);Right->SetPadding(FMargin(10*D,30*D,0,10*D));Row(Columns,0);}else{Row(Left,0);Row(Card,12);}
 BuildExpeditionFooter();
 UpdateCheckProgress();
}
