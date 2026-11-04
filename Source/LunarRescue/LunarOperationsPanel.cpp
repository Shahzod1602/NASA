#include "LunarOperationsPanel.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "LunarTrail.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"

namespace {
const FLinearColor OpsInk(.91f,.96f,.98f),OpsDim(.68f,.79f,.85f),OpsCyan(.12f,.85f,.94f),OpsGold(.98f,.78f,.29f),OpsGreen(.52f,.9f,.69f);
FLinearColor WireColor(const UFieldOperationsComponent* C,const FString& Label){const FLinearColor Colors[]={FLinearColor(.99f,.58f,.47f),FLinearColor(.56f,.82f,1.f),FLinearColor(.99f,.83f,.37f),FLinearColor(.68f,.92f,.62f)};if(C)for(int32 I=0;I<FMath::Clamp(C->WirePairs(),2,4);++I)if(C->WireLeftLabel(I).Equals(Label,ESearchCase::IgnoreCase))return Colors[I];return OpsInk;}
float OpsScale(){const float S=ULunarSettings::Get()->TextScale;return FMath::IsFinite(S)?FMath::Clamp(S,1.f,1.5f):1.f;}
FVector2D LocalPoint(const FVector& P){return FVector2D(P.X,-P.Y);}
const TCHAR* ViewName(EFieldPanelView V){switch(V){case EFieldPanelView::Preflight:return TEXT("EVA checks");case EFieldPanelView::Briefing:return TEXT("Briefing");case EFieldPanelView::Resources:return TEXT("Resources & map");case EFieldPanelView::Vehicle:return TEXT("Vehicle");case EFieldPanelView::Wire:return TEXT("Wire repair");case EFieldPanelView::Inventory:return TEXT("Inventory");default:return TEXT("Sample data");}}
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
TSharedRef<SWidget> ULunarOperationsMap::RebuildWidget(){return SNew(SBox).MinDesiredHeight(300);}
int32 ULunarOperationsMap::NativePaint(const FPaintArgs& A,const FGeometry& G,const FSlateRect& C,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled)const{
 Layer=UUserWidget::NativePaint(A,G,C,Out,Layer,Style,Enabled);const FVector2D Available=G.GetLocalSize();if(Available.X<20||Available.Y<20)return Layer;
 FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,FLinearColor(.008f,.025f,.035f));
 const FVector2D Span=Bounds.GetSize();if(Span.X<=UE_SMALL_NUMBER||Span.Y<=UE_SMALL_NUMBER)return Layer+1;
 const double Aspect=OrbitalMap?Span.X*104800./(Span.Y*84800.):Span.X/Span.Y;
 const double Height=FMath::Min(Available.Y-24.,(Available.X-24.)/Aspect);const FVector2D Size(Height*Aspect,Height),Origin((Available.X-Size.X)*.5,(Available.Y-Size.Y)*.5);
 auto Line=[&](FVector2D P,FVector2D Q,FLinearColor Color,float Width=1.f){const TArray<FVector2D> Ends{P,Q};FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),Ends,ESlateDrawEffect::None,Color,true,Width);};
 if(OrbitalMap&&MapTexture){FSlateBrush Brush;Brush.SetResourceObject(MapTexture);Brush.SetUVRegion(Bounds);Brush.ImageSize=Size;FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(Size,FSlateLayoutTransform(Origin)),&Brush,ESlateDrawEffect::None,FLinearColor(.48f,.54f,.57f));}
 else for(int32 I=0;I<=4;++I){const double F=I/4.;Line(Origin+FVector2D(Size.X*F,0),Origin+FVector2D(Size.X*F,Size.Y),FLinearColor(.10f,.22f,.28f));Line(Origin+FVector2D(0,Size.Y*F),Origin+FVector2D(Size.X,Size.Y*F),FLinearColor(.10f,.22f,.28f));}
 int32 MarkerTotal=0;for(const auto& Point:Points)if(!Point.Player)++MarkerTotal;const double LabelWidth=34.*OpsScale(),LabelHeight=26.*OpsScale();const int32 LabelRows=FMath::Max(1,FMath::FloorToInt((Available.Y-12)/LabelHeight)),LabelColumns=FMath::Max(1,FMath::DivideAndRoundUp(MarkerTotal,LabelRows));const double LabelSpan=LabelColumns*(LabelWidth+6);double LabelStart=Origin.X+Size.X+12;if(LabelStart+LabelSpan>Available.X-4)LabelStart=Origin.X-LabelSpan-12;if(LabelStart<4)LabelStart=FMath::Max(4.,Available.X-LabelSpan-4);
 int32 Number=0;for(const auto& P:Points){FVector2D At=LocalPoint(P.World);if(OrbitalMap)LunarTrail::MapUV(P.World,At);At=Origin+(At-Bounds.Min)/Span*Size;
  if(P.Player){Line(At+FVector2D(0,-9),At+FVector2D(8,7),OpsCyan,3);Line(At+FVector2D(8,7),At+FVector2D(-8,7),OpsCyan,3);Line(At+FVector2D(-8,7),At+FVector2D(0,-9),OpsCyan,3);continue;}
  ++Number;TArray<FVector2D> Star;for(int32 I=0;I<=10;++I){const double Angle=-UE_PI*.5+I*UE_PI/5,Radius=(I%2?4.:9.);Star.Add(At+FVector2D(FMath::Cos(Angle),FMath::Sin(Angle))*Radius);}
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
void ULunarOperationsPanel::NativeTick(const FGeometry& Geometry,float Delta){UUserWidget::NativeTick(Geometry,Delta);if(FMath::IsFinite(Delta)&&Delta>0)RefreshElapsed+=Delta;if(RefreshElapsed>=.1f){RefreshElapsed=0;Refresh();}}
void ULunarOperationsPanel::CloseOwnedPanel(){if(auto* C=Operations()){if(C->CanUsePanel(this,false))if(auto* P=C->Pilot())P->ClosePanel();return;}auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(P&&P->ActivePanel==this&&IsInViewport())P->ClosePanel();}
FReply ULunarOperationsPanel::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event){
 if(Event.IsRepeat())return FReply::Handled();if(Event.GetKey()==EKeys::Escape||Event.GetKey()==EKeys::F1){CloseOwnedPanel();return FReply::Handled();}return UUserWidget::NativeOnKeyDown(Geometry,Event);
}
void ULunarOperationsPanel::Refresh(bool Force){
 if(!OperationsBody||!WidgetTree)return;auto* C=Operations();const float Scale=OpsScale(),Width=ContentWidth();FString Fingerprint=FString::Printf(TEXT("%d/%d/%.0f/%d"),int32(View),ShowSources,Width,FMath::RoundToInt(Scale*100));
 if(C){Fingerprint+=FString::Printf(TEXT("/%d/%d/%d/%d/%d/%d/%d/%d/%d/%.0f"),C->Revision,C->PreflightMask,C->PendingCheck,C->Difficulty,C->WireMask,C->WireLeft,C->WireIsolated,C->CanUsePanel(this,false),C->CanUsePanel(this,true),FMath::Clamp(C->ActionProgress(),0.f,1.f)*100.f);Fingerprint+=C->Feedback+FString::Join(C->ResourceLines(),TEXT("|"))+FString::Join(C->InventoryLines(),TEXT("|"));}
 if(Force||Fingerprint!=LastFingerprint||FMath::Abs(LastScale-Scale)>.001f||FMath::Abs(LastWidth-Width)>1){LastFingerprint=Fingerprint;LastScale=Scale;LastWidth=Width;const float Offset=OperationsScroll?OperationsScroll->GetScrollOffset():0;BuildPage();if(OperationsScroll)OperationsScroll->SetScrollOffset(Offset);}
}
void ULunarOperationsPanel::Dispatch(int32 Index,int32 SentGeneration){
 if(SentGeneration!=Generation||!Actions.IsValidIndex(Index))return;const FAction A=Actions[Index];if(A.Name==TEXT("Close")){CloseOwnedPanel();return;}auto* C=Operations();if(!C||!C->CanUsePanel(this,false))return;
 if(A.Name==TEXT("View")){if(A.Argument<0||A.Argument>int32(EFieldPanelView::Sample))return;const auto Next=EFieldPanelView(A.Argument);if(C->CanOpenView(Next)){if(OperationsScroll)OperationsScroll->ScrollToStart();Configure(Next);SetKeyboardFocus();}return;}
 if(A.Name==TEXT("Sources")){ShowSources=!ShowSources;Refresh(true);SetKeyboardFocus();return;}
 if(!C->CanUsePanel(this,true))return;C->PerformAction(A.Name,A.Argument);if(C->CanUsePanel(this,false)){Refresh(true);SetKeyboardFocus();}
}
void ULunarOperationsPanel::BuildMap(){
 auto* C=Operations();if(!C)return;const auto Data=C->MapPoints();TArray<FLunarOperationsMapPoint> Points;Points.Reserve(FMath::Min(Data.Num()+1,64));int32 Count=0;
 for(const auto& M:Data){if(Count++>=63)break;Points.Add({M.Position,M.Label,false,M.Completed,M.Current});}if(auto* P=C->Pilot())Points.Add({P->GetActorLocation(),TEXT("YOU"),true,false,false});
 OperationsMap=CreateWidget<ULunarOperationsMap>(GetOwningPlayer());const auto* G=C->Game();OperationsMap->SetPoints(Points,G&&G->Apollo?G->ApolloMapTexture.Get():nullptr);auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetHeightOverride(340);Size->SetContent(OperationsMap);Row(Size);
 Note(OperationsMap->IsOrbitalMap()?TEXT("NASA/LROC orbital basemap. Yellow numbered stars: current world mission locations. Cyan triangle: you. Check mark: completed. This is not a terrain safety rating."):TEXT("LOCAL WORLD MAP / Authored schematic from actual current world positions. Yellow numbered stars: mission locations. Cyan triangle: you. Check mark: completed. This is not a satellite image or terrain safety rating."));
 int32 I=0;for(const auto& M:Data){if(I>=63)break;++I;const auto* P=C->Pilot();const float Meters=P?FVector::Dist2D(P->GetActorLocation(),M.Position)/100.f:0.f;const FString State=M.Completed?TEXT("COMPLETE"):M.Current?TEXT("CURRENT"):TEXT("AVAILABLE");Row(Button(FString::Printf(TEXT("%d. %s / %s / %.0f m"),I,*M.Label,*State,Meters),TEXT("Waypoint"),M.Index,C->CanUsePanel(this,true),M.Current));}
}
void ULunarOperationsPanel::BuildPage(){
 if(!OperationsBody||!WidgetTree)return;++Generation;Actions.Reset();OperationsBody->ClearChildren();OperationsMap=nullptr;auto* C=Operations();
 Row(Text(TEXT("LUNARRESCUE / FIELD OPERATIONS"),15,OpsDim));Heading(C?C->Title(View):TEXT("FIELD OPERATIONS UNAVAILABLE"));
 auto* NavigationRow=WidgetTree->ConstructWidget<UWrapBox>();NavigationRow->SetInnerSlotPadding(FVector2D(8,7));NavigationRow->SetExplicitWrapSize(true);NavigationRow->SetWrapSize(ContentWidth()-16);NavigationRow->AddChild(Button(TEXT("Close / Esc / F1"),TEXT("Close")));
 if(C)for(int32 I=0;I<=int32(EFieldPanelView::Sample);++I){const auto Target=EFieldPanelView(I);NavigationRow->AddChild(Button(ViewName(Target),TEXT("View"),I,C->CanOpenView(Target),View==Target));}Row(NavigationRow);
 if(!C){Note(TEXT("The field operations component is unavailable in this world. Close this panel to continue the mission."));return;}
 Note(C->Instructions(View));if(!C->Feedback.IsEmpty())Row(Text(C->Feedback,18,OpsGold));
 const bool CanMutate=C->CanUsePanel(this,true);
 if(View==EFieldPanelView::Preflight){
  Note(TEXT("Authored training checks / simplified game actions, not a real EVA procedure. Complete each check before opening the hatch."));
  const TCHAR* Names[]={TEXT("Inspect suit seals"),TEXT("Attach PLSS life-support pack"),TEXT("Check communications"),TEXT("Select internal suit power")};
  for(int32 I=0;I<4;++I){const bool Done=(C->PreflightMask&(1<<I))!=0,Running=C->PendingCheck==I;const FString Caption=FString::Printf(TEXT("%d / %s / %s"),I+1,Names[I],Done?TEXT("COMPLETE"):Running?TEXT("CHECKING"):TEXT("CHECK"));Row(Button(Caption,TEXT("Check"),I,CanMutate&&!Done&&C->PendingCheck==INDEX_NONE&&C->PreflightStep()==I,Running));}
  if(C->PendingCheck!=INDEX_NONE){auto* Progress=WidgetTree->ConstructWidget<UProgressBar>();Progress->SetPercent(FMath::Clamp(C->ActionProgress(),0.f,1.f));Progress->SetFillColorAndOpacity(OpsCyan);auto* Size=WidgetTree->ConstructWidget<USizeBox>();Size->SetHeightOverride(14);Size->SetContent(Progress);Row(Size);Status(TEXT("ACTUAL CHECK PROGRESS"),FString::Printf(TEXT("%d%%"),FMath::RoundToInt(C->ActionProgress()*100)));}
  Row(Button(TEXT("Checks complete / open hatch and begin surface operations"),TEXT("Launch"),0,CanMutate&&(C->PreflightMask&15)==15&&C->PendingCheck==INDEX_NONE));
 }else if(View==EFieldPanelView::Briefing){
  Heading(TEXT("CHOOSE A GAMEPLAY RESOURCE PROFILE"));Note(TEXT("These are authored game budgets and costs. They do not predict real astronaut physiology or EVA endurance."));
  const TCHAR* Labels[]={TEXT("BEGINNER / extra planning margin"),TEXT("INTERMEDIATE / balanced resource budget"),TEXT("PROFESSIONAL / tighter gameplay budget")};
  for(int32 I=0;I<3;++I)Row(Button(Labels[I],TEXT("Difficulty"),I,CanMutate&&!C->Briefed,C->Difficulty==I));
  for(const auto& Line:C->ResourceLines())Row(Text(Line));BuildMap();Row(Button(C->Briefed?TEXT("RESOURCE PROFILE CONFIRMED"):TEXT("Confirm resource profile and mission plan"),TEXT("ConfirmBriefing"),0,CanMutate&&!C->Briefed,C->Briefed));
 }else if(View==EFieldPanelView::Resources){
  Heading(TEXT("LIVE LOCAL RESOURCE LEDGER"));for(const auto& Line:C->ResourceLines())Row(Text(Line));
  const auto* Mission=C->Game();const auto* Pilot=C->Pilot();const bool Linked=Mission&&(Mission->Apollo?(Mission->ApolloInstruments&2)!=0:Mission->StationRebootComplete);const bool AtBoard=Pilot&&IsValid(C->ResourceBoard)&&FVector::Dist(Pilot->GetActorLocation(),C->ResourceBoard->GetActorLocation())<=420.f;
  Row(Button(TEXT("Recharge utility / up to 20 RU / 3 seconds"),TEXT("Recharge"),0,CanMutate&&C->Briefed&&Linked&&AtBoard&&!C->Riding()&&C->PendingCheck==INDEX_NONE&&C->UtilityCharge<C->UtilityCapacity));
  Note(TEXT("Recharge requires a working mission communications/power link and proximity to the resource board. It closes this panel for three seconds; mission time and oxygen continue while it runs."));BuildMap();
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
  Heading(TEXT("LOCAL CARGO / DATA MANIFEST"));for(const auto& Line:C->InventoryLines())Row(Text(Line));Note(TEXT("Training water packages are authored cargo. Their presence does not indicate locally detected lunar or Martian water."));Row(Button(TEXT("Deliver carried training packages"),TEXT("Deposit"),0,CanMutate&&(C->CollectedMask&~C->DeliveredMask)!=0));Row(Button(TEXT("Transmit stored training survey data"),TEXT("Transmit"),0,CanMutate&&C->DataStoredMB>0));
 }else{
  Heading(TEXT("SAMPLE DATA / OBSERVE BEFORE RECORDING"));for(const auto& Line:C->InventoryLines())Row(Text(Line));Status(TEXT("SCANNER"),C->ScannerUnlocked?TEXT("AVAILABLE"):TEXT("FOLLOW THE FIELD TASK TO UNLOCK"),C->ScannerUnlocked);Note(TEXT("The world scanner uses the normal gameplay tool controls. This panel displays recorded results; opening it does not collect, scan, measure or award cargo."));
 }
 Row(Button(ShowSources?TEXT("Hide Science & Sources"):TEXT("Open Science & Sources"),TEXT("Sources"),0,true,ShowSources));if(ShowSources){Heading(TEXT("SCIENCE & SOURCES / MODEL LIMITS"));Row(Text(C->SourcesText(),16));}
 Note(View==EFieldPanelView::Preflight?TEXT("The departure clock waits while this preflight panel is open. Close / Esc / F1 returns to the cabin."):TEXT("This panel pauses the gameplay resource clock. Close / Esc / F1 returns to the mission. All resource coefficients are gameplay tuning."));
}
bool ULunarOperationsPanel::ValidateLayout(FString& Reason)const{
 if(!OperationsBody||!OperationsScroll||!WidgetTree){Reason=TEXT("Operations panel is missing its scrollable body");return false;}
 const auto& Root=GetCachedGeometry();const auto& Scroll=OperationsScroll->GetCachedGeometry();const FVector2D Size=Root.GetLocalSize(),A=Root.AbsoluteToLocal(Scroll.LocalToAbsolute(FVector2D::ZeroVector)),B=Root.AbsoluteToLocal(Scroll.LocalToAbsolute(Scroll.GetLocalSize()));if(Size.X<1||A.X<-.5||A.Y<-.5||B.X>Size.X+.5||B.Y>Size.Y+.5||B.X<=A.X||B.Y<=A.Y){Reason=TEXT("Operations scroll viewport escapes the screen");return false;}
 TArray<UWidget*> Widgets;WidgetTree->GetAllWidgets(Widgets);for(auto* W:Widgets)if(auto* T=Cast<UTextBlock>(W)){bool InBody=false;for(const UWidget* P=T;P;P=P->GetParent())if(P==OperationsBody){InBody=true;break;}if(!InBody)continue;const auto& Geo=T->GetCachedGeometry();if(Geo.GetLocalSize().X<1)continue;const auto L=Scroll.AbsoluteToLocal(Geo.LocalToAbsolute(FVector2D::ZeroVector)),R=Scroll.AbsoluteToLocal(Geo.LocalToAbsolute(Geo.GetLocalSize()));if(L.X<-.5||R.X>Scroll.GetLocalSize().X+.5||Geo.GetLocalSize().Y+.5<T->GetDesiredSize().Y){Reason=FString::Printf(TEXT("Operations text allocation: '%s' arranged %.2fx%.2f desired %.2fx%.2f viewX %.2f..%.2f width %.2f scroll %.2f"),*T->GetText().ToString().Left(100),Geo.GetLocalSize().X,Geo.GetLocalSize().Y,T->GetDesiredSize().X,T->GetDesiredSize().Y,L.X,R.X,Scroll.GetLocalSize().X,OperationsScroll->GetScrollOffset());return false;}}
 Reason.Reset();return true;
}
