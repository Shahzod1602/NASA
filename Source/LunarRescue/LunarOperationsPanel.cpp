#include "Misc/PackageName.h"
#include "LunarOperationsPanel.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "LunarWireBoard.h"
#include "LunarTrail.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ExpandableArea.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ButtonSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"

namespace {
const FLinearColor OpsInk(.91f,.96f,.98f),OpsDim(.68f,.79f,.85f),OpsCyan(.12f,.85f,.94f),OpsGold(.98f,.78f,.29f),OpsGreen(.52f,.9f,.69f);
FLinearColor WireColor(const UFieldOperationsComponent* C,const FString& Label){const FLinearColor Colors[]={FLinearColor(.99f,.58f,.47f),FLinearColor(.56f,.82f,1.f),FLinearColor(.99f,.83f,.37f),FLinearColor(.68f,.92f,.62f)};if(C)for(int32 I=0;I<FMath::Clamp(C->WirePairs(),2,4);++I)if(C->WireLeftLabel(I).Equals(Label,ESearchCase::IgnoreCase))return Colors[I];return OpsInk;}
float OpsScale(){const float S=ULunarSettings::Get()->TextScale;return FMath::IsFinite(S)?FMath::Clamp(S,1.f,1.5f):1.f;}
UImage* PlanetImage(const ULunarOperationsMap* Map){return Map&&Map->WidgetTree?Cast<UImage>(Map->WidgetTree->FindWidget(TEXT("PlanetContext"))):nullptr;}
bool PlanetVisible(const ULunarOperationsMap* Map){const auto* Image=PlanetImage(Map);return Map&&Map->IsPlanetOverview()&&Image&&Image->IsVisible();}
void UpdatePlanet(ULunarOperationsMap* Map,float Zoom,FVector2D Pan){if(auto* Image=PlanetImage(Map))if(auto* Material=Image->GetDynamicMaterial()){Material->SetScalarParameterValue(TEXT("Zoom"),Zoom);Material->SetScalarParameterValue(TEXT("Yaw"),float(Pan.X));Material->SetScalarParameterValue(TEXT("Pitch"),float(Pan.Y));}}
FVector2D LocalPoint(const FVector& P){return FVector2D(P.X,-P.Y);}
const TCHAR* ViewName(EFieldPanelView V){switch(V){case EFieldPanelView::Preflight:return TEXT("EVA checks");case EFieldPanelView::Briefing:return TEXT("Briefing");case EFieldPanelView::Resources:return TEXT("Resources & map");case EFieldPanelView::Vehicle:return TEXT("Vehicle");case EFieldPanelView::Wire:return TEXT("Wire repair");case EFieldPanelView::Inventory:return TEXT("Inventory");default:return TEXT("Sample data");}}
struct FBagItem{FString Name,Description,State;int32 Category=1,Quantity=1;FName Use=NAME_None;bool Ready=false;};
TArray<FBagItem> BagItems(UFieldOperationsComponent* C){
 TArray<FBagItem> Items;const auto* G=C?C->Game():nullptr;if(!C||!G)return Items;
 Items.Add({TEXT("Context work tool"),TEXT("Opens the mission's current tool at an eligible Apollo work site. Repairs and collection retain their normal mission order and range checks."),G->Apollo?TEXT("Approach the current work site"):TEXT("Use the work key at mission equipment"),1,1,TEXT("UseWorkTool"),G->Apollo&&G->CanInteract()});
 Items.Add({TEXT("Optical magnifier"),TEXT("Inspect labelled training cartridges. Hold left click to observe a label; this does not measure water in the current terrain."),C->ScannerUnlocked?TEXT("READY / equip magnifier"):TEXT("LOCKED / inspect training rover"),1,C->ScannerUnlocked?1:0,TEXT("UseScanner"),C->ScannerUnlocked&&!C->Riding()});
 Items.Add({TEXT("Life-support pack"),TEXT("Mission oxygen is a simplified gameplay budget. The pack cannot be refilled from this inventory."),FString::Printf(TEXT("%.0f / %.0f gameplay seconds"),G->Oxygen,G->OxygenCapacity),2});
 Items.Add({TEXT("Utility battery"),TEXT("Authored resource units power field support. Recharging requires the resource board and a working mission link."),FString::Printf(TEXT("%.1f / %.1f RU"),C->UtilityCharge,C->UtilityCapacity),2});
 Items.Add({TEXT("Training data buffer"),TEXT("Recorded training packets remain local until a downlink at the resource board."),FString::Printf(TEXT("%.2f / %.2f MB"),C->DataStoredMB,C->DataCapacityMB),2,C->DataStoredMB>0?1:0});
 for(int32 I=0;I<3;++I){const bool Packed=(C->CollectedMask&(1<<I))&&!(C->DeliveredMask&(1<<I));Items.Add({FString::Printf(TEXT("TRAINING-%02d"),I+1),TEXT("Authored remote-delivery teaching cartridge. It is not a historical Apollo sample or evidence of locally measured water."),C->DeliveredMask&(1<<I)?TEXT("DELIVERED"):Packed?TEXT("IN BACKPACK"):TEXT("NOT COLLECTED"),3,Packed?1:0});}
 Items.Add({TEXT("Mission samples"),TEXT("Actual collected sample count from the current mission. Inventory inspection never grants or consumes samples."),TEXT("Recorded mission cargo"),3,G->SampleCount()});
 return Items;
}
}

void ULunarOperationsMap::SetPoints(const TArray<FLunarOperationsMapPoint>& InPoints,UTexture2D* InTexture){
 Points.Reset();Bounds=FBox2D(ForceInit);MapTexture=InTexture;OrbitalMap=MapTexture!=nullptr;
 for(const auto& P:InPoints){if(Points.Num()>=64)break;if(P.World.ContainsNaN())continue;Points.Add(P);FVector2D UV;if(!LunarTrail::MapUV(P.World,UV))OrbitalMap=false;}
 for(const auto& P:Points){FVector2D At=LocalPoint(P.World);if(OrbitalMap)LunarTrail::MapUV(P.World,At);Bounds+=At;}
 if(!Bounds.bIsValid){Bounds=FBox2D(FVector2D(-500,-500),FVector2D(500,500));OrbitalMap=false;}
 const FVector2D Ext=Bounds.GetSize();const FVector2D Margin(OrbitalMap?FMath::Max(Ext.X*.10,.002):FMath::Max(Ext.X*.10,100.),OrbitalMap?FMath::Max(Ext.Y*.10,.002):FMath::Max(Ext.Y*.10,100.));
 Bounds.Min-=Margin;Bounds.Max+=Margin;
 if(OrbitalMap){Bounds.Min.X=FMath::Clamp(Bounds.Min.X,0.,1.);Bounds.Min.Y=FMath::Clamp(Bounds.Min.Y,0.,1.);Bounds.Max.X=FMath::Clamp(Bounds.Max.X,0.,1.);Bounds.Max.Y=FMath::Clamp(Bounds.Max.Y,0.,1.);}
 else{const FVector2D S=Bounds.GetSize(),Center=Bounds.GetCenter();const double Minimum=FMath::Max(S.X,S.Y)*.12;if(S.X<Minimum){Bounds.Min.X=Center.X-Minimum*.5;Bounds.Max.X=Center.X+Minimum*.5;}if(S.Y<Minimum){Bounds.Min.Y=Center.Y-Minimum*.5;Bounds.Max.Y=Center.Y+Minimum*.5;}}
 InvalidateLayoutAndVolatility();
}
TSharedRef<SWidget> ULunarOperationsMap::RebuildWidget(){
 SetVisibility(ESlateVisibility::Visible);SetClipping(EWidgetClipping::ClipToBounds);
 if(!WidgetTree)WidgetTree=NewObject<UWidgetTree>(this);
 auto* Root=WidgetTree->ConstructWidget<USizeBox>();Root->SetMinDesiredHeight(300);WidgetTree->RootWidget=Root;
 if(auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/UI/PlanetContext20261114/M_PlanetContext.M_PlanetContext"))){
  auto* Image=WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(),TEXT("PlanetContext"));auto* Material=UMaterialInstanceDynamic::Create(Base,this);
  const auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));const TCHAR* Path=G&&G->Mars?TEXT("/Game/Lunar/UI/PlanetContext20261114/T_MarsContext.T_MarsContext"):TEXT("/Game/Lunar/UI/PlanetContext20261114/T_MoonContext.T_MoonContext");
  if(auto* Texture=LoadObject<UTexture2D>(nullptr,Path))Material->SetTextureParameterValue(TEXT("Planet"),Texture);
  Image->SetBrushFromMaterial(Material);Image->SetVisibility(PlanetOverview?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);Root->SetContent(Image);UpdatePlanet(this,Zoom,Pan);
 }return UUserWidget::RebuildWidget();
}
FBox2D ULunarOperationsMap::ViewBounds()const{const FVector2D Half=Bounds.GetSize()/(2*Zoom),Center=Bounds.GetCenter()+Pan;return FBox2D(Center-Half,Center+Half);}
void ULunarOperationsMap::ZoomBy(float Steps){if(!FMath::IsFinite(Steps))return;Zoom=FMath::Clamp(Zoom*FMath::Pow(1.25f,FMath::Clamp(Steps,-8.f,8.f)),1.f,6.f);PanBy(FVector2D::ZeroVector);}
void ULunarOperationsMap::PanBy(FVector2D Fraction){if(Fraction.ContainsNaN())return;if(PlanetVisible(this)){Pan+=Fraction*180;Pan.X=FMath::Fmod(Pan.X,360.);Pan.Y=FMath::Fmod(Pan.Y,360.);UpdatePlanet(this,Zoom,Pan);}else{Pan+=Fraction*Bounds.GetSize()/Zoom;const FVector2D Margin=Bounds.GetSize()*(1-1/Zoom)*.5;Pan.X=FMath::Clamp(Pan.X,-Margin.X,Margin.X);Pan.Y=FMath::Clamp(Pan.Y,-Margin.Y,Margin.Y);}InvalidateLayoutAndVolatility();}
void ULunarOperationsMap::ResetView(){Zoom=1;Pan=FVector2D::ZeroVector;UpdatePlanet(this,Zoom,Pan);InvalidateLayoutAndVolatility();}
void ULunarOperationsMap::SetPlanetOverview(bool Value){PlanetOverview=Value;if(auto* Image=PlanetImage(this))Image->SetVisibility(Value?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);ResetView();}
FReply ULunarOperationsMap::NativeOnMouseWheel(const FGeometry&,const FPointerEvent& E){ZoomBy(E.GetWheelDelta());return FReply::Handled();}
FReply ULunarOperationsMap::NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent& E){if(E.GetEffectingButton()!=EKeys::LeftMouseButton)return FReply::Unhandled();Dragging=true;return FReply::Handled().CaptureMouse(TakeWidget());}
FReply ULunarOperationsMap::NativeOnMouseButtonUp(const FGeometry&,const FPointerEvent& E){if(E.GetEffectingButton()!=EKeys::LeftMouseButton)return FReply::Unhandled();Dragging=false;return FReply::Handled().ReleaseMouseCapture();}
FReply ULunarOperationsMap::NativeOnMouseMove(const FGeometry& G,const FPointerEvent& E){if(!Dragging||!E.IsMouseButtonDown(EKeys::LeftMouseButton)){Dragging=false;return FReply::Unhandled();}const FVector2D Size=G.GetLocalSize(),Delta=G.AbsoluteToLocal(E.GetScreenSpacePosition())-G.AbsoluteToLocal(E.GetScreenSpacePosition()-E.GetCursorDelta());if(Size.X>0&&Size.Y>0)PanBy(-Delta/Size);return FReply::Handled();}
int32 ULunarOperationsMap::NativePaint(const FPaintArgs& A,const FGeometry& G,const FSlateRect& C,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled)const{
 if(PlanetVisible(this)){if(auto* Image=PlanetImage(this))if(auto* Material=Image->GetDynamicMaterial()){const auto Size=G.GetLocalSize();Material->SetScalarParameterValue(TEXT("Aspect"),float(Size.X/FMath::Max(1.,Size.Y)));}return UUserWidget::NativePaint(A,G,C,Out,Layer,Style,Enabled);}
 Layer=UUserWidget::NativePaint(A,G,C,Out,Layer,Style,Enabled);const FVector2D Available=G.GetLocalSize();if(Available.X<20||Available.Y<20)return Layer;
 FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,FLinearColor(.008f,.025f,.035f));
 const FBox2D Visible=ViewBounds();const FVector2D Span=Visible.GetSize();if(Span.X<=UE_SMALL_NUMBER||Span.Y<=UE_SMALL_NUMBER)return Layer+1;
 const double Aspect=OrbitalMap?Span.X*104800./(Span.Y*84800.):Span.X/Span.Y;
 const double Height=FMath::Min(Available.Y-24.,(Available.X-24.)/Aspect);const FVector2D Size(Height*Aspect,Height),Origin((Available.X-Size.X)*.5,(Available.Y-Size.Y)*.5);
 auto Line=[&](FVector2D P,FVector2D Q,FLinearColor Color,float Width=1.f){const TArray<FVector2D> Ends{P,Q};FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),Ends,ESlateDrawEffect::None,Color,true,Width);};
 if(OrbitalMap&&MapTexture){FSlateBrush Brush;Brush.SetResourceObject(MapTexture);Brush.SetUVRegion(Visible);Brush.ImageSize=Size;FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(Size,FSlateLayoutTransform(Origin)),&Brush,ESlateDrawEffect::None,FLinearColor(.48f,.54f,.57f));}
 else for(int32 I=0;I<=4;++I){const double F=I/4.;Line(Origin+FVector2D(Size.X*F,0),Origin+FVector2D(Size.X*F,Size.Y),FLinearColor(.10f,.22f,.28f));Line(Origin+FVector2D(0,Size.Y*F),Origin+FVector2D(Size.X,Size.Y*F),FLinearColor(.10f,.22f,.28f));}
 int32 MarkerTotal=0;for(const auto& Point:Points)if(!Point.Player)++MarkerTotal;const double LabelWidth=34.*OpsScale(),LabelHeight=26.*OpsScale();const int32 LabelRows=FMath::Max(1,FMath::FloorToInt((Available.Y-12)/LabelHeight)),LabelColumns=FMath::Max(1,FMath::DivideAndRoundUp(MarkerTotal,LabelRows));const double LabelSpan=LabelColumns*(LabelWidth+6);double LabelStart=Origin.X+Size.X+12;if(LabelStart+LabelSpan>Available.X-4)LabelStart=Origin.X-LabelSpan-12;if(LabelStart<4)LabelStart=FMath::Max(4.,Available.X-LabelSpan-4);
 int32 Number=0;for(const auto& P:Points){FVector2D At=LocalPoint(P.World);if(OrbitalMap)LunarTrail::MapUV(P.World,At);if(!P.Player)++Number;if(!Visible.IsInsideOrOn(At))continue;At=Origin+(At-Visible.Min)/Span*Size;
  if(P.Player){Line(At+FVector2D(0,-9),At+FVector2D(8,7),OpsCyan,3);Line(At+FVector2D(8,7),At+FVector2D(-8,7),OpsCyan,3);Line(At+FVector2D(-8,7),At+FVector2D(0,-9),OpsCyan,3);continue;}
  TArray<FVector2D> Star;for(int32 I=0;I<=10;++I){const double Angle=-UE_PI*.5+I*UE_PI/5,Radius=(I%2?4.:9.);Star.Add(At+FVector2D(FMath::Cos(Angle),FMath::Sin(Angle))*Radius);}
  FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),Star,ESlateDrawEffect::None,OpsGold,true,P.Current?3.f:2.f);
  if(P.Completed){Line(At+FVector2D(-4,0),At+FVector2D(-1,4),OpsGreen,2);Line(At+FVector2D(-1,4),At+FVector2D(5,-4),OpsGreen,2);}
  const FVector2D Label(LabelStart+((Number-1)/LabelRows)*(LabelWidth+6),6+((Number-1)%LabelRows)*LabelHeight);Line(At,Label+FVector2D(LabelWidth*.5,LabelHeight*.5),FLinearColor(.70f,.59f,.26f,.7f),1);FSlateDrawElement::MakeBox(Out,Layer+3,G.ToPaintGeometry(FVector2D(LabelWidth,LabelHeight-3),FSlateLayoutTransform(Label)),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,FLinearColor(.004f,.012f,.02f,.96f));FSlateDrawElement::MakeText(Out,Layer+4,G.ToPaintGeometry(FVector2D(LabelWidth,LabelHeight),FSlateLayoutTransform(Label+FVector2D(4,1))),FString::FromInt(Number),FCoreStyle::GetDefaultFontStyle("Bold",FMath::RoundToInt(15*OpsScale())),ESlateDrawEffect::None,OpsInk);
 }
 return Layer+5;
}

void ULunarOperationsButton::Activate(){if(OwnerPanel&&GetIsEnabled())OwnerPanel->Dispatch(Action,Generation);}
UFieldOperationsComponent* ULunarOperationsPanel::Operations()const{const auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));return G?G->FieldOperations.Get():nullptr;}
float ULunarOperationsPanel::ContentWidth()const{const FVector2D Size=UWidgetLayoutLibrary::GetViewportSize(this);const float DPI=FMath::Max(.1f,UWidgetLayoutLibrary::GetViewportScale(this));return FMath::Max(300.f,float(Size.X>1?Size.X:1280)/DPI-72.f);}
UTextBlock* ULunarOperationsPanel::Text(const FString& Value,int32 Size,FLinearColor Color){auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));T->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Size*OpsScale())));T->SetColorAndOpacity(Color);T->SetAutoWrapText(false);T->SetWrapTextAt(ContentWidth()-32);return T;}
ULunarOperationsButton* ULunarOperationsPanel::Button(const FString& Caption,FName Action,int32 Argument,bool Enabled,bool Selected){
 auto* B=WidgetTree->ConstructWidget<ULunarOperationsButton>();B->OwnerPanel=this;B->Generation=Generation;B->Action=Actions.Add({Action,Argument});B->OnClicked.AddDynamic(B,&ULunarOperationsButton::Activate);B->SetIsEnabled(Enabled);B->SetBackgroundColor(Selected?FLinearColor(.035f,.33f,.43f,.98f):FLinearColor(.035f,.10f,.15f,.98f));
 auto Style=B->GetStyle();Style.Normal.TintColor=FSlateColor(Selected?FLinearColor(.035f,.33f,.43f):FLinearColor(.035f,.10f,.15f));Style.Hovered.TintColor=FSlateColor(FLinearColor(.075f,.28f,.35f));Style.Pressed.TintColor=FSlateColor(FLinearColor(.10f,.39f,.48f));B->SetStyle(Style);B->SetBackgroundColor(FLinearColor::White);auto* Label=Text(Caption,17,Enabled?OpsInk:OpsDim);Label->SetWrapTextAt(FMath::Max(120.f,ContentWidth()*.42f));B->SetContent(Label);return B;
}
void ULunarOperationsPanel::Row(UWidget* W,float SlotPadding){if(OperationsBody&&W)OperationsBody->AddChildToVerticalBox(W)->SetPadding(FMargin(4,SlotPadding));}
void ULunarOperationsPanel::Heading(const FString& S){Row(Text(S,23,OpsCyan),9);}
void ULunarOperationsPanel::Note(const FString& S){Row(Text(S,15,OpsDim));}
void ULunarOperationsPanel::Status(const FString& Name,const FString& Value,bool Good){Row(Text(Name+TEXT(" / ")+Value,17,Good?OpsInk:OpsGold));}
void ULunarOperationsPanel::Configure(EFieldPanelView InView){View=InView;ShowSources=false;Refresh(true);}
TSharedRef<SWidget> ULunarOperationsPanel::RebuildWidget(){
 SetIsFocusable(true);auto* Border=WidgetTree->ConstructWidget<UBorder>();Border->SetBrushColor(FLinearColor(.007f,.021f,.035f,.985f));Border->SetPadding(FMargin(24,18));WidgetTree->RootWidget=Border;
 OperationsScroll=WidgetTree->ConstructWidget<UScrollBox>();OperationsScroll->SetScrollBarVisibility(ESlateVisibility::Visible);Border->SetContent(OperationsScroll);OperationsBody=WidgetTree->ConstructWidget<UVerticalBox>();OperationsScroll->AddChild(OperationsBody);BuildPage();return UUserWidget::RebuildWidget();
}
void ULunarOperationsPanel::NativeConstruct(){UUserWidget::NativeConstruct();Refresh(true);SetKeyboardFocus();}
void ULunarOperationsPanel::NativeTick(const FGeometry& Geometry,float Delta){UUserWidget::NativeTick(Geometry,Delta);UpdateCheckProgress();if(FMath::IsFinite(Delta)&&Delta>0)RefreshElapsed+=Delta;if(RefreshElapsed>=.1f){RefreshElapsed=0;Refresh();}}
void ULunarOperationsPanel::CloseOwnedPanel(){if(auto* C=Operations()){if(C->CanUsePanel(this,false))if(auto* P=C->Pilot())P->ClosePanel();return;}auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(P&&P->ActivePanel==this&&IsInViewport())P->ClosePanel();}
FReply ULunarOperationsPanel::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event){
 if(Event.IsRepeat())return FReply::Handled();if(Event.GetKey()==EKeys::Escape||Event.GetKey()==EKeys::F1){CloseOwnedPanel();return FReply::Handled();}return UUserWidget::NativeOnKeyDown(Geometry,Event);
}
void ULunarOperationsPanel::Refresh(bool Force){
 if(!OperationsBody||!WidgetTree)return;auto* C=Operations();const float Scale=OpsScale(),Width=ContentWidth();FString Fingerprint=FString::Printf(TEXT("%d/%d/%.0f/%d/%d/%d"),int32(View),ShowSources,Width,FMath::RoundToInt(Scale*100),InventoryFilter,SelectedInventory);
 if(C){Fingerprint+=FString::Printf(TEXT("/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d"),C->Revision,C->PreflightMask,C->PendingCheck,C->Difficulty,C->WireMask,C->WireLeft,C->WireIsolated,C->CanUsePanel(this,false),C->CanUsePanel(this,true),C->PreflightCursor);Fingerprint+=C->Feedback;if(!C->Preparing)Fingerprint+=FString::Join(C->ResourceLines(),TEXT("|"))+FString::Join(C->InventoryLines(),TEXT("|"));}
 if(Force||Fingerprint!=LastFingerprint||FMath::Abs(LastScale-Scale)>.001f||FMath::Abs(LastWidth-Width)>1){LastFingerprint=Fingerprint;LastScale=Scale;LastWidth=Width;const float Offset=OperationsScroll?OperationsScroll->GetScrollOffset():0;BuildPage();if(OperationsScroll)OperationsScroll->SetScrollOffset(Offset);}
 UpdateCheckProgress();
}
void ULunarOperationsPanel::Dispatch(int32 Index,int32 SentGeneration){
 if(SentGeneration!=Generation||!Actions.IsValidIndex(Index))return;const FAction A=Actions[Index];if(A.Name==TEXT("Close")){CloseOwnedPanel();return;}auto* C=Operations();if(!C||!C->CanUsePanel(this,false))return;
 if(A.Name==TEXT("View")){if(A.Argument<0||A.Argument>int32(EFieldPanelView::Sample))return;const auto Next=EFieldPanelView(A.Argument);if(C->CanOpenView(Next)){if(OperationsScroll)OperationsScroll->ScrollToStart();Configure(Next);SetKeyboardFocus();}return;}
 if(A.Name==TEXT("Sources")){ShowSources=!ShowSources;Refresh(true);SetKeyboardFocus();return;}
 if(A.Name==TEXT("MapZoom")&&OperationsMap){if(A.Argument==0)OperationsMap->ResetView();else OperationsMap->ZoomBy(float(A.Argument));return;}
 if(A.Name==TEXT("MapPlanet")&&OperationsMap){OperationsMap->SetPlanetOverview(A.Argument!=0);return;}
 if(A.Name==TEXT("Waypoint")){if(C->PerformAction(A.Name,A.Argument)){if(OperationsMap)OperationsMap->SetPlanetOverview(false);Refresh(true);}return;}
 if(View==EFieldPanelView::Inventory&&A.Name==TEXT("FilterItems")&&A.Argument>=0&&A.Argument<=3){InventoryFilter=A.Argument;Refresh(true);return;}
 if(View==EFieldPanelView::Inventory&&A.Name==TEXT("SelectItem")&&BagItems(C).IsValidIndex(A.Argument)){SelectedInventory=A.Argument;Refresh(true);return;}
 if(!C->CanUsePanel(this,true))return;C->PerformAction(A.Name,A.Argument);if(C->CanUsePanel(this,false)){Refresh(true);SetKeyboardFocus();}
}
void ULunarOperationsPanel::BuildMap(){
 auto* C=Operations();if(!C)return;const auto Data=C->MapPoints();TArray<FLunarOperationsMapPoint> Points;int32 Count=0;
 for(const auto& M:Data){if(Count++>=63)break;Points.Add({M.Position,M.Label,false,M.Completed,M.Current});}if(auto* P=C->Pilot())Points.Add({P->GetActorLocation(),TEXT("YOU"),true,false,false});
 if(!CachedMap)CachedMap=CreateWidget<ULunarOperationsMap>(GetOwningPlayer());OperationsMap=CachedMap;OperationsMap->RemoveFromParent();const auto* G=C->Game();OperationsMap->SetPoints(Points,G&&G->Apollo?G->ApolloMapTexture.Get():nullptr);OperationsMap->PanBy(FVector2D::ZeroVector);
 const bool Wide=ContentWidth()>1000&&OpsScale()<1.3f;const float MapHeight=Wide?370.f:300.f;auto* Viewport=WidgetTree->ConstructWidget<UVerticalBox>();auto* Locations=WidgetTree->ConstructWidget<UVerticalBox>();
 Locations->AddChildToVerticalBox(Text(TEXT("LOCATIONS / select to track"),15,OpsCyan))->SetPadding(FMargin(0,0,8,8));
 int32 I=0;for(const auto& M:Data){if(I>=63)break;++I;const auto* P=C->Pilot();const float Meters=P?FVector::Dist2D(P->GetActorLocation(),M.Position)/100.f:0.f;
  auto* Pick=Button(FString::Printf(TEXT("%d. %s / %.0f m%s"),I,*M.Label,Meters,M.Completed?TEXT(" / DONE"):TEXT("")),TEXT("Waypoint"),M.Index,C->CanUsePanel(this,false),M.Current);
  if(auto* Caption=Cast<UTextBlock>(Pick->GetContent())){Caption->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(14*OpsScale())));Caption->SetWrapTextAt(Wide?ContentWidth()*.30f-30:ContentWidth()-50);}
  Locations->AddChildToVerticalBox(Pick)->SetPadding(FMargin(0,3,8,3));
 }
 auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetHeightOverride(MapHeight);Size->SetContent(OperationsMap);Viewport->AddChildToVerticalBox(Size);
 auto* Controls=WidgetTree->ConstructWidget<UWrapBox>();Controls->SetInnerSlotPadding(FVector2D(8,4));Controls->AddChild(Button(TEXT("Zoom +"),TEXT("MapZoom"),1));Controls->AddChild(Button(TEXT("Zoom -"),TEXT("MapZoom"),-1));Controls->AddChild(Button(TEXT("Reset map"),TEXT("MapZoom"),0));Viewport->AddChildToVerticalBox(Controls);
 auto* Views=WidgetTree->ConstructWidget<UWrapBox>();Views->SetInnerSlotPadding(FVector2D(8,4));Views->AddChild(Button(TEXT("Local mission map"),TEXT("MapPlanet"),0));Views->AddChild(Button(TEXT("Planet overview / drag to rotate"),TEXT("MapPlanet"),1));Viewport->AddChildToVerticalBox(Views);
 if(Wide){auto* Columns=WidgetTree->ConstructWidget<UHorizontalBox>();FSlateChildSize Left(ESlateSizeRule::Fill);Left.Value=.32f;FSlateChildSize Right(ESlateSizeRule::Fill);Right.Value=.68f;Columns->AddChildToHorizontalBox(Locations)->SetSize(Left);Columns->AddChildToHorizontalBox(Viewport)->SetSize(Right);Row(Columns);}else{Row(Viewport);Row(Locations);}
 Note(TEXT("Scroll to zoom / drag to pan or rotate. Local stars mark actual mission locations; the cyan triangle is you. Planet view provides colour context only. Distances belong to the local map. See Science & Sources for image credits and limits."));
}
void ULunarOperationsPanel::UpdateCheckProgress(){
 auto* C=Operations();if(View!=EFieldPanelView::Preflight||!C||!CheckProgress||!CheckProgressLabel)return;
 const int32 Step=FMath::Clamp(C->PreflightCursor,0,3);
 const float Progress=(C->PreflightMask&(1<<Step))?1.f:C->PendingCheck==Step?C->ActionProgress():0.f;
 CheckProgress->SetPercent(Progress);
 for(int32 I=0;I<4;++I)if(auto* Bar=Cast<UProgressBar>(WidgetTree->FindWidget(FName(*FString::Printf(TEXT("EVAProgress%d_%d"),Generation,I)))))Bar->SetPercent((C->PreflightMask&(1<<I))?1.f:C->PendingCheck==I?C->ActionProgress():0.f);
 const FString Value=FString::Printf(TEXT("%d%% / %s"),FMath::FloorToInt(Progress*100.f+.001f),(C->PreflightMask&(1<<Step))?TEXT("COMPLETE"):C->PendingCheck==Step?TEXT("CHECKING"):TEXT("READY"));
 if(CheckProgressLabel->GetText().ToString()!=Value)CheckProgressLabel->SetText(FText::FromString(Value));
}
void ULunarOperationsPanel::BuildPreflight(){
 auto* C=Operations();if(!C)return;const auto* Mission=C->Game();
 const bool CanMutate=C->Preparing&&C->CanUsePanel(this,true),Wide=ContentWidth()>950&&OpsScale()<1.3f;
 const int32 Step=FMath::Clamp(C->PreflightCursor,0,3);
 const TCHAR* Names[]={TEXT("Inspect suit seals"),TEXT("Attach PLSS life-support pack"),TEXT("Check communications"),TEXT("Select internal suit power")};
 const TCHAR* Help[]={TEXT("Inspect the suit seal indicator and record this training check."),TEXT("Secure the life-support pack before surface departure."),TEXT("Confirm the radio link for mission instructions and subtitles."),TEXT("Select internal suit power before opening the cabin hatch.")};
 auto* Content=WidgetTree->ConstructWidget<UHorizontalBox>();
 auto* Overview=WidgetTree->ConstructWidget<UVerticalBox>();auto* Checks=WidgetTree->ConstructWidget<UVerticalBox>();
 auto Add=[&](UVerticalBox* Box,UWidget* Child,float ItemGap=6.f){Box->AddChildToVerticalBox(Child)->SetPadding(FMargin(12,ItemGap));};
 auto Label=[&](const FString& Value,int32 Size,FLinearColor Color,float Width){auto* T=Text(Value,Size,Color);T->SetWrapTextAt(Width);return T;};
 const float LeftWidth=Wide?ContentWidth()*.35f-48:ContentWidth()-64,RightWidth=Wide?ContentWidth()*.65f-56:ContentWidth()-64;
 Add(Overview,Label(Mission&&Mission->Mars?TEXT("ARES / MARS"):Mission&&Mission->Apollo?TEXT("APOLLO 12 / MOON"):TEXT("SELENE / MOON"),30,OpsInk,LeftWidth));
 Add(Overview,Label(TEXT("PRE-EVA TRAINING"),15,OpsCyan,LeftWidth));
 if(Wide)if(auto* Portrait=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/UI/Expedition20261114/SuitPreview.SuitPreview"))){auto* Picture=WidgetTree->ConstructWidget<UImage>();Picture->SetBrushFromTexture(Portrait);auto* Frame=WidgetTree->ConstructWidget<USizeBox>();Frame->SetWidthOverride(160);Frame->SetHeightOverride(240);Frame->SetContent(Picture);auto* FrameSlot=Overview->AddChildToVerticalBox(Frame);FrameSlot->SetHorizontalAlignment(HAlign_Center);FrameSlot->SetPadding(FMargin(4));}
 Add(Overview,Label(FString::Printf(TEXT("SYSTEM %02d / 04"),Step+1),19,OpsGold,LeftWidth),16);
 Add(Overview,Label(Help[Step],19,OpsInk,LeftWidth));
 Add(Overview,Label(TEXT("Select each unlocked check in order. Your mission clock waits until surface handover."),15,OpsDim,LeftWidth),12);
 Add(Overview,Label(TEXT("Training simulation / authored checks, not real suit measurements."),14,OpsDim,LeftWidth));
 Add(Checks,Label(TEXT("SUIT CHECKLIST"),23,OpsInk,RightWidth));
 for(int32 I=0;I<4;++I){
  const bool Done=(C->PreflightMask&(1<<I))!=0,Running=C->PendingCheck==I;
  auto* B=Button(FString::Printf(TEXT("%d / %s / %s"),I+1,Names[I],Done?TEXT("100% / COMPLETE"):Running?TEXT("CHECKING"):I==Step?TEXT("START"):TEXT("LOCKED")),TEXT("Check"),I,CanMutate&&!Done&&C->PendingCheck==INDEX_NONE&&I==Step,I==Step);
  {auto PaddedStyle=B->GetStyle();PaddedStyle.NormalPadding=FMargin(12);PaddedStyle.PressedPadding=FMargin(12);B->SetStyle(PaddedStyle);}if(auto* T=Cast<UTextBlock>(B->GetContent())){T->SetWrapTextAt(RightWidth-28);if(Done)T->SetColorAndOpacity(OpsGreen);}Add(Checks,B,5);auto* Indicator=WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(),FName(*FString::Printf(TEXT("EVAProgress%d_%d"),Generation,I)));Indicator->SetFillColorAndOpacity(OpsCyan);Indicator->SetPercent(Done?1.f:0.f);auto* IndicatorSize=WidgetTree->ConstructWidget<USizeBox>();IndicatorSize->SetHeightOverride(5);IndicatorSize->SetContent(Indicator);Add(Checks,IndicatorSize,1);
 }
 CheckProgress=WidgetTree->ConstructWidget<UProgressBar>();CheckProgress->SetFillColorAndOpacity(OpsCyan);
 auto* ProgressSize=WidgetTree->ConstructWidget<USizeBox>();ProgressSize->SetHeightOverride(9);ProgressSize->SetContent(CheckProgress);Add(Checks,ProgressSize,8);
 CheckProgressLabel=Label(TEXT("0% / READY"),15,OpsCyan,RightWidth);Add(Checks,CheckProgressLabel,3);

 Add(Checks,Button(TEXT("Checks complete / Open Hatch"),TEXT("Launch"),0,CanMutate&&C->Prepared&&C->PreflightMask==15&&C->PendingCheck==INDEX_NONE,C->Prepared),12);
 if(Wide){
  FSlateChildSize LeftSize(ESlateSizeRule::Fill);LeftSize.Value=.35f;FSlateChildSize RightSize(ESlateSizeRule::Fill);RightSize.Value=.65f;
  auto* L=Content->AddChildToHorizontalBox(Overview);L->SetSize(LeftSize);L->SetPadding(FMargin(0,8,20,8));
  auto* R=Content->AddChildToHorizontalBox(Checks);R->SetSize(RightSize);R->SetPadding(FMargin(8));Row(Content);
 }else{Row(Overview);Row(Checks);}
 UpdateCheckProgress();
}
void ULunarOperationsPanel::BuildInventory(){
 auto* C=Operations();if(!C)return;const auto Items=BagItems(C);if(!Items.IsValidIndex(SelectedInventory))SelectedInventory=0;
 const bool Wide=ContentWidth()>950&&OpsScale()<1.3f,CanMutate=C->CanUsePanel(this,true);
 Heading(TEXT("INVENTORY"));Note(TEXT("Equipment and collected samples / actual mission state."));
 auto* Atlas=FPackageName::DoesPackageExist(TEXT("/Game/Lunar/Frontend/Startup20261114/T_InventoryReference"))?LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/Startup20261114/T_InventoryReference.T_InventoryReference")):nullptr;
 // Reference art is unchanged. UVs exclude its baked labels and quantities;
 // every interactive caption, quantity, lock and action comes from live state.
 const FBox2D Regions[]={FBox2D(FVector2D(461,216),FVector2D(556,298)),FBox2D(FVector2D(236,217),FVector2D(328,297)),FBox2D(FVector2D(350,331),FVector2D(442,410)),FBox2D(FVector2D(236,334),FVector2D(332,410)),FBox2D(FVector2D(568,331),FVector2D(660,409)),FBox2D(FVector2D(697,216),FVector2D(782,300)),FBox2D(FVector2D(697,216),FVector2D(782,300)),FBox2D(FVector2D(697,216),FVector2D(782,300)),FBox2D(FVector2D(694,337),FVector2D(785,412))};
 auto Icon=[&](int32 Index,float W,float H){auto* Image=WidgetTree->ConstructWidget<UImage>();if(Atlas&&Index>=0&&Index<UE_ARRAY_COUNT(Regions)){FSlateBrush Brush;Brush.SetResourceObject(Atlas);Brush.ImageSize=FVector2D(W,H);Brush.DrawAs=ESlateBrushDrawType::Image;Brush.SetUVRegion(FBox2D(Regions[Index].Min/FVector2D(1280,720),Regions[Index].Max/FVector2D(1280,720)));Image->SetBrush(Brush);}Image->SetVisibility(ESlateVisibility::HitTestInvisible);auto* Box=WidgetTree->ConstructWidget<USizeBox>();Box->SetWidthOverride(W);Box->SetHeightOverride(H);if(Atlas)Box->SetContent(Image);else {const TCHAR* Symbols[]={TEXT("TOOL"),TEXT("SCAN"),TEXT("PACK"),TEXT("CELL"),TEXT("DATA"),TEXT("01"),TEXT("02"),TEXT("03"),TEXT("ROCK")};auto* Symbol=Text(Symbols[FMath::Clamp(Index,0,8)],18,OpsCyan);Symbol->SetJustification(ETextJustify::Center);Box->SetContent(Symbol);}return Box;};
 auto* InventoryColumns=WidgetTree->ConstructWidget<UHorizontalBox>();auto* Sidebar=WidgetTree->ConstructWidget<UVerticalBox>();
 const TCHAR* Categories[]={TEXT("All"),TEXT("Tools"),TEXT("Supplies"),TEXT("Samples")};
 for(int32 I=0;I<4;++I)Sidebar->AddChildToVerticalBox(Button(Categories[I],TEXT("FilterItems"),I,true,InventoryFilter==I))->SetPadding(FMargin(4,6));
 if(Wide){auto* Side=WidgetTree->ConstructWidget<USizeBox>();Side->SetWidthOverride(135);Side->SetContent(Sidebar);InventoryColumns->AddChildToHorizontalBox(Side)->SetPadding(FMargin(0,0,14,0));}else Row(Sidebar);
 const float GridWidth=Wide?(ContentWidth()-149)*.62f:ContentWidth()-40,CellWidth=(GridWidth-(Wide?48:24))/(Wide?4:2);
 auto* Grid=WidgetTree->ConstructWidget<UWrapBox>();Grid->SetExplicitWrapSize(true);Grid->SetWrapSize(GridWidth);Grid->SetInnerSlotPadding(FVector2D(8,8));
 for(int32 I=0;I<Items.Num();++I){const auto& Item=Items[I];if(InventoryFilter&&Item.Category!=InventoryFilter)continue;
  auto* Cell=WidgetTree->ConstructWidget<USizeBox>();Cell->SetWidthOverride(CellWidth);Cell->SetHeightOverride(164*OpsScale());
  auto* Layers=WidgetTree->ConstructWidget<UOverlay>();Cell->SetContent(Layers);
  auto* B=Button(FString::Printf(TEXT("%s\nQTY %d"),*Item.Name,Item.Quantity),TEXT("SelectItem"),I,true,SelectedInventory==I);if(auto* CaptionLayout=Cast<UButtonSlot>(B->GetContent()->Slot))CaptionLayout->SetVerticalAlignment(VAlign_Bottom);
  {auto Style=B->GetStyle();Style.NormalPadding=FMargin(8);Style.PressedPadding=FMargin(8);B->SetStyle(Style);}if(auto* T=Cast<UTextBlock>(B->GetContent()))T->SetWrapTextAt(CellWidth-16);
  auto* ButtonSlot=Layers->AddChildToOverlay(B);ButtonSlot->SetHorizontalAlignment(HAlign_Fill);ButtonSlot->SetVerticalAlignment(VAlign_Fill);
  auto* PictureSlot=Layers->AddChildToOverlay(Icon(I,84,72));PictureSlot->SetHorizontalAlignment(HAlign_Center);PictureSlot->SetVerticalAlignment(VAlign_Top);PictureSlot->SetPadding(FMargin(0,7,0,0));Grid->AddChild(Cell);
 }
 auto* Detail=WidgetTree->ConstructWidget<UVerticalBox>();const auto& Selected=Items[SelectedInventory];const float DetailWidth=Wide?(ContentWidth()-149)*.38f-32:ContentWidth()-64;
 Detail->AddChildToVerticalBox(Icon(SelectedInventory,168,144))->SetHorizontalAlignment(HAlign_Center);
 auto Add=[&](const FString& Value,int32 Size,FLinearColor Color){auto* T=Text(Value,Size,Color);T->SetWrapTextAt(DetailWidth);Detail->AddChildToVerticalBox(T)->SetPadding(FMargin(12,8));};
 Add(Selected.Name,24,OpsInk);Add(Selected.State,17,OpsCyan);Add(FString::Printf(TEXT("QUANTITY / %d"),Selected.Quantity),15,OpsDim);Add(Selected.Description,17,OpsInk);
 if(!Selected.Use.IsNone())Detail->AddChildToVerticalBox(Button(Selected.Use==TEXT("UseScanner")?TEXT("Equip magnifier"):TEXT("Open current work tool"),Selected.Use,0,CanMutate&&Selected.Ready&&!C->Preparing));
 else Add(TEXT("INSPECT ONLY / use the existing mission controls or field board."),14,OpsDim);
 if(Wide){FSlateChildSize L(ESlateSizeRule::Fill);L.Value=.62f;FSlateChildSize R(ESlateSizeRule::Fill);R.Value=.38f;InventoryColumns->AddChildToHorizontalBox(Grid)->SetSize(L);InventoryColumns->AddChildToHorizontalBox(Detail)->SetSize(R);Row(InventoryColumns);}else{Row(Grid);Row(Detail);}
 Row(Button(TEXT("Deliver carried training packages"),TEXT("Deposit"),0,CanMutate&&(C->CollectedMask&~C->DeliveredMask)!=0));Row(Button(TEXT("Transmit stored training survey data"),TEXT("Transmit"),0,CanMutate&&C->DataStoredMB>0));
 Note(TEXT("Equipment symbols / resource amounts and item states use live gameplay data."));
}

void ULunarOperationsPanel::BuildPage(){
 if(!OperationsBody||!WidgetTree)return;bool ExpandedHelp=false;TArray<UWidget*> PreviousWidgets;WidgetTree->GetAllWidgets(PreviousWidgets);for(auto* Widget:PreviousWidgets)if(auto* Area=Cast<UExpandableArea>(Widget))ExpandedHelp|=Area->GetIsExpanded();++Generation;Actions.Reset();OperationsBody->ClearChildren();OperationsMap=nullptr;CheckProgress=nullptr;CheckProgressLabel=nullptr;auto* C=Operations();
 const bool CircuitScreen=C&&View==EFieldPanelView::Wire&&C->Game()&&!C->Game()->Apollo;
 if(auto* Background=Cast<UBorder>(WidgetTree->RootWidget))Background->SetBrushColor(CircuitScreen?FLinearColor(.003f,.008f,.011f,1):FLinearColor(.007f,.021f,.035f,.985f));
 if(CircuitScreen){
  auto* Header=WidgetTree->ConstructWidget<UHorizontalBox>();Header->AddChildToHorizontalBox(Text(TEXT("LUNARRESCUE"),19))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Header->AddChildToHorizontalBox(Button(TEXT("Esc / Close"),TEXT("Close")));Row(Header,8);
  Row(Text(TEXT("Connect systems"),32),13);const auto* Mission=C->Game();const bool Online=C->WireComplete&&!C->WirePendingCommit;int32 Count=0;for(int32 I=0;I<C->WirePairs();++I)if(C->WireMask&(1<<I))++Count;
  auto* StatusLine=WidgetTree->ConstructWidget<UHorizontalBox>();StatusLine->AddChildToHorizontalBox(Text(Online?TEXT("ONLINE / CABLE"):C->WireIsolated?TEXT("ISOLATED"):TEXT("ISOLATE CIRCUIT"),18,Online?OpsCyan:OpsDim))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));StatusLine->AddChildToHorizontalBox(Text(FString::Printf(TEXT("%d / %d"),Count,C->WirePairs()),20,OpsCyan));Row(StatusLine);
  const bool Work=C->CanUsePanel(this,true)&&Mission->Stage==ELunarStage::RestorePower&&Mission->RepairStep==1;
  if(!C->WireIsolated&&!Online)Row(Button(TEXT("Isolate training circuit"),TEXT("WireIsolate"),0,Work));
  auto* Board=CreateWidget<ULunarWireBoard>(GetOwningPlayer());Board->SetPanel(this);auto* BoardSize=WidgetTree->ConstructWidget<USizeBox>();BoardSize->SetHeightOverride(330*OpsScale());BoardSize->SetContent(Board);Row(BoardSize,0);
  if(!C->Feedback.IsEmpty()&&!Online)Row(Text(C->Feedback,14,OpsDim),3);
  auto* Footer=WidgetTree->ConstructWidget<UHorizontalBox>();Footer->AddChildToHorizontalBox(Text(Online?TEXT("Cable verified / station restart required"):TEXT("Drag a cable to its matching signal"),17,Online?OpsCyan:OpsDim))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));const bool Ready=Work&&C->WireComplete&&C->WirePendingCommit;auto* Verify=Button(Online?TEXT("Online"):TEXT("Verify"),TEXT("WireVerify"),0,Ready,true);if(Ready){auto VerifyStyle=Verify->GetStyle();VerifyStyle.Normal.TintColor=FSlateColor(OpsCyan);VerifyStyle.Hovered.TintColor=FSlateColor(FLinearColor(.35,.95,1));Verify->SetStyle(VerifyStyle);if(auto* Caption=Cast<UTextBlock>(Verify->GetContent()))Caption->SetColorAndOpacity(FLinearColor(.002,.008,.011));}auto* VerifySize=WidgetTree->ConstructWidget<USizeBox>();VerifySize->SetWidthOverride(220);VerifySize->SetHeightOverride(46*OpsScale());VerifySize->SetContent(Verify);Footer->AddChildToHorizontalBox(VerifySize);Row(Footer,12);
  auto* Alternative=WidgetTree->ConstructWidget<UExpandableArea>();Alternative->SetContentForSlot(TEXT("Header"),Text(TEXT("Keyboard / click controls and help"),14,OpsDim));auto* Controls=WidgetTree->ConstructWidget<UVerticalBox>();
  for(int32 I=0;I<C->WirePairs();++I){auto* Pair=WidgetTree->ConstructWidget<UHorizontalBox>();Pair->AddChildToHorizontalBox(Button(FString::Printf(TEXT("SOURCE %c / %s"),TCHAR('A'+I),*C->WireLeftLabel(I)),TEXT("WireLeft"),I,Work&&C->WireIsolated&&!C->WireComplete&&!(C->WireMask&(1<<I)),C->WireLeft==I))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Pair->AddChildToHorizontalBox(Button(FString::Printf(TEXT("SOCKET %c / %s"),TCHAR('A'+I),*C->WireRightLabel(I)),TEXT("WireRight"),I,Work&&C->WireIsolated&&!C->WireComplete&&C->WireLeft!=INDEX_NONE))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Controls->AddChildToVerticalBox(Pair);}
  Controls->AddChildToVerticalBox(Button(TEXT("Reset unfinished connections"),TEXT("WireReset"),0,Work&&!Online));Controls->AddChildToVerticalBox(Button(TEXT("Field operations"),TEXT("View"),int32(EFieldPanelView::Resources)));Controls->AddChildToVerticalBox(Text(TEXT("Select a source, then its matching socket. Verify applies only the cable step; the station still needs a restart. This isolated toy circuit is a gameplay abstraction, not real electrical repair. The resource clock is paused."),14,OpsDim));Alternative->SetContentForSlot(TEXT("Body"),Controls);Alternative->SetIsExpanded(ExpandedHelp);Row(Alternative,5);return;
 }
 Row(Text(TEXT("LUNARRESCUE / FIELD OPERATIONS"),15,OpsDim));Heading(C?C->Title(View):TEXT("FIELD OPERATIONS UNAVAILABLE"));
 auto* NavigationRow=WidgetTree->ConstructWidget<UWrapBox>();NavigationRow->SetInnerSlotPadding(FVector2D(8,7));NavigationRow->SetExplicitWrapSize(true);NavigationRow->SetWrapSize(ContentWidth()-16);NavigationRow->AddChild(Button(TEXT("Close / Esc / F1"),TEXT("Close")));
 if(C)for(int32 I: {0,2,5,1,3,4,6}){const auto Target=EFieldPanelView(I);if(C->Preparing&&Target!=EFieldPanelView::Preflight&&Target!=EFieldPanelView::Resources&&Target!=EFieldPanelView::Inventory)continue;NavigationRow->AddChild(Button(Target==EFieldPanelView::Resources&&C->Preparing?TEXT("Map"):ViewName(Target),TEXT("View"),I,C->CanOpenView(Target),View==Target));}Row(NavigationRow);
 if(!C){Note(TEXT("The field operations component is unavailable in this world. Close this panel to continue the mission."));return;}
 Note(C->Instructions(View));if(!C->Feedback.IsEmpty())Row(Text(C->Feedback,18,OpsGold));
 const bool CanMutate=C->CanUsePanel(this,true);
 if(View==EFieldPanelView::Preflight){
  BuildPreflight();
 }else if(View==EFieldPanelView::Briefing){
  Heading(TEXT("CHOOSE A GAMEPLAY RESOURCE PROFILE"));Note(TEXT("These are authored game budgets and costs. They do not predict real astronaut physiology or EVA endurance."));
  const TCHAR* Labels[]={TEXT("BEGINNER / extra planning margin"),TEXT("INTERMEDIATE / balanced resource budget"),TEXT("PROFESSIONAL / tighter gameplay budget")};
  for(int32 I=0;I<3;++I)Row(Button(Labels[I],TEXT("Difficulty"),I,CanMutate&&!C->Briefed,C->Difficulty==I));
  for(const auto& Line:C->ResourceLines())Row(Text(Line));BuildMap();Row(Button(C->Briefed?TEXT("RESOURCE PROFILE CONFIRMED"):TEXT("Confirm resource profile and mission plan"),TEXT("ConfirmBriefing"),0,CanMutate&&!C->Briefed,C->Briefed));
 }else if(View==EFieldPanelView::Resources){
  if(C->Preparing){Heading(TEXT("MISSION MAP / PRE-EVA"));BuildMap();}
  else{
  Heading(TEXT("LIVE LOCAL RESOURCE LEDGER"));for(const auto& Line:C->ResourceLines())Row(Text(Line));
  const auto* Mission=C->Game();const auto* Pilot=C->Pilot();const bool Linked=Mission&&(Mission->Apollo?(Mission->ApolloInstruments&2)!=0:Mission->StationRebootComplete);const bool AtBoard=Pilot&&IsValid(C->ResourceBoard)&&FVector::Dist(Pilot->GetActorLocation(),C->ResourceBoard->GetActorLocation())<=420.f;
  Row(Button(TEXT("Recharge utility / up to 20 RU / 3 seconds"),TEXT("Recharge"),0,CanMutate&&C->Briefed&&Linked&&AtBoard&&!C->Riding()&&C->PendingCheck==INDEX_NONE&&C->UtilityCharge<C->UtilityCapacity));
  Note(TEXT("Recharge requires a working mission communications/power link and proximity to the resource board. It closes this panel for three seconds; mission time and oxygen continue while it runs."));BuildMap();}
 }else if(View==EFieldPanelView::Vehicle){
  Heading(TEXT("SURFACE VEHICLE / INSPECTION"));Status(TEXT("INSPECTION"),C->VehicleInspected?TEXT("COMPLETE"):TEXT("REQUIRED"),C->VehicleInspected);for(const auto& Line:C->ResourceLines())Row(Text(Line));Row(Button(C->VehicleInspected?TEXT("Inspection complete"):TEXT("Inspect vehicle systems"),TEXT("InspectVehicle"),0,CanMutate&&!C->VehicleInspected));Row(Button(TEXT("Open wire matching panel"),TEXT("View"),int32(EFieldPanelView::Wire),C->CanOpenView(EFieldPanelView::Wire)));
 }else if(View==EFieldPanelView::Wire){
  const auto* Mission=C->Game();
  if(Mission&&Mission->Apollo){
   Heading(TEXT("RESCUE CABLE TRAINING / CONTEXT ONLY"));
   Note(TEXT("Cable matching belongs to the SELENE and ARES rescue missions. Historical Apollo 12 ALSEP deployment is a separate task. No cable-matching action is available in this mission."));
  }else{
  const bool WireActions=CanMutate&&Mission&&Mission->Stage==ELunarStage::RestorePower&&Mission->RepairStep==1;
  if(!WireActions)Note(TEXT("READ ONLY / Reach the station power-repair task and seat the power cell first. Cable matching becomes available at repair step 1; it does not replace any other mission stage."));
  const int32 Required=FMath::Clamp(C->WirePairs(),2,4);int32 Verified=0;for(int32 I=0;I<Required;++I)if(C->WireMask&(1<<I))++Verified;
  Heading(TEXT("ISOLATE / MATCH / VERIFY"));Status(TEXT("ELECTRICAL ISOLATION"),C->WireIsolated?TEXT("ISOLATED"):TEXT("ISOLATE BEFORE CONNECTING"),C->WireIsolated);Status(TEXT("VERIFIED CONNECTIONS"),FString::Printf(TEXT("%d / %d"),Verified,Required));
  Note(TEXT("Choose a source on the left, then the socket with the matching printed signal label on the right. Port letters and signal names are the rule; color is a secondary cue."));Row(Button(C->WireIsolated?TEXT("Circuit isolated"):TEXT("Isolate training circuit"),TEXT("WireIsolate"),0,WireActions&&!C->WireIsolated));
  auto* Ports=WidgetTree->ConstructWidget<UHorizontalBox>();auto* Left=WidgetTree->ConstructWidget<UVerticalBox>();auto* Right=WidgetTree->ConstructWidget<UVerticalBox>();auto* L=Ports->AddChildToHorizontalBox(Left);L->SetSize(FSlateChildSize(ESlateSizeRule::Fill));L->SetPadding(FMargin(0,0,8,0));auto* R=Ports->AddChildToHorizontalBox(Right);R->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  for(int32 I=0;I<Required;++I){const FString SourceLabel=C->WireLeftLabel(I),SocketLabel=C->WireRightLabel(I);const FString Source=FString::Printf(TEXT("SOURCE %c / %s%s"),TCHAR('A'+I),*SourceLabel,(C->WireMask&(1<<I))?TEXT(" / VERIFIED"):TEXT(""));auto* B=Button(Source,TEXT("WireLeft"),I,WireActions&&C->WireIsolated&&!C->WireComplete&&!(C->WireMask&(1<<I)),C->WireLeft==I);if(auto* Caption=Cast<UTextBlock>(B->GetContent()))Caption->SetColorAndOpacity(WireColor(C,SourceLabel));Left->AddChildToVerticalBox(B)->SetPadding(FMargin(0,5));const FString Socket=FString::Printf(TEXT("SOCKET %c / %s"),TCHAR('A'+I),*SocketLabel);auto* SocketButton=Button(Socket,TEXT("WireRight"),I,WireActions&&C->WireIsolated&&!C->WireComplete&&C->WireLeft!=INDEX_NONE);if(auto* Caption=Cast<UTextBlock>(SocketButton->GetContent()))Caption->SetColorAndOpacity(WireColor(C,SocketLabel));Right->AddChildToVerticalBox(SocketButton)->SetPadding(FMargin(0,5));}Row(Ports);if(C->WireComplete)Row(Text(TEXT("WIRE REPAIR VERIFIED / All required connections are recorded by gameplay."),18,OpsGreen));Row(Button(TEXT("Reset unfinished connections"),TEXT("WireReset"),0,WireActions&&!C->WireComplete));
  }
 }else if(View==EFieldPanelView::Inventory){
  BuildInventory();
 }else{
  Heading(TEXT("SAMPLE DATA / OBSERVE BEFORE RECORDING"));for(const auto& Line:C->InventoryLines())Row(Text(Line));Status(TEXT("SCANNER"),C->ScannerUnlocked?TEXT("AVAILABLE"):TEXT("FOLLOW THE FIELD TASK TO UNLOCK"),C->ScannerUnlocked);Note(TEXT("The world scanner uses the normal gameplay tool controls. This panel displays recorded results; opening it does not collect, scan, measure or award cargo."));
 }
 Row(Button(ShowSources?TEXT("Hide Science & Sources"):TEXT("Open Science & Sources"),TEXT("Sources"),0,true,ShowSources));if(ShowSources){Heading(TEXT("SCIENCE & SOURCES / MODEL LIMITS"));Row(Text(C->SourcesText(),16));}
 Note(C->Preparing?TEXT("Mission time has not started. A running suit check continues when you inspect Map or Inventory. Close / Esc / F1 returns to your current view."):TEXT("This panel pauses the gameplay resource clock. Close / Esc / F1 returns to the mission. All resource coefficients are gameplay tuning."));
}
bool ULunarOperationsPanel::ValidateLayout(FString& Reason)const{
 if(!OperationsBody||!OperationsScroll||!WidgetTree){Reason=TEXT("Operations panel is missing its scrollable body");return false;}
 const auto& Root=GetCachedGeometry();const auto& Scroll=OperationsScroll->GetCachedGeometry();const FVector2D Size=Root.GetLocalSize(),A=Root.AbsoluteToLocal(Scroll.LocalToAbsolute(FVector2D::ZeroVector)),B=Root.AbsoluteToLocal(Scroll.LocalToAbsolute(Scroll.GetLocalSize()));if(Size.X<1||A.X<-.5||A.Y<-.5||B.X>Size.X+.5||B.Y>Size.Y+.5||B.X<=A.X||B.Y<=A.Y){Reason=TEXT("Operations scroll viewport escapes the screen");return false;}
 TArray<UWidget*> Widgets;WidgetTree->GetAllWidgets(Widgets);for(auto* W:Widgets)if(auto* T=Cast<UTextBlock>(W)){bool InBody=false;for(const UWidget* P=T;P;P=P->GetParent())if(P==OperationsBody){InBody=true;break;}if(!InBody)continue;const auto& Geo=T->GetCachedGeometry();if(Geo.GetLocalSize().X<1)continue;const auto L=Scroll.AbsoluteToLocal(Geo.LocalToAbsolute(FVector2D::ZeroVector)),R=Scroll.AbsoluteToLocal(Geo.LocalToAbsolute(Geo.GetLocalSize()));if(L.X<-.5||R.X>Scroll.GetLocalSize().X+.5||Geo.GetLocalSize().Y+.5<T->GetDesiredSize().Y){Reason=FString::Printf(TEXT("Operations text allocation: '%s' arranged %.2fx%.2f desired %.2fx%.2f viewX %.2f..%.2f width %.2f scroll %.2f"),*T->GetText().ToString().Left(100),Geo.GetLocalSize().X,Geo.GetLocalSize().Y,T->GetDesiredSize().X,T->GetDesiredSize().Y,L.X,R.X,Scroll.GetLocalSize().X,OperationsScroll->GetScrollOffset());return false;}}
 Reason.Reset();return true;
}
