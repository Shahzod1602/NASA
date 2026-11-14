#include "LunarWireBoard.h"
#include "LunarOperationsPanel.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace {
FLinearColor CableColor(const FString& Label){if(Label.Contains(TEXT("POWER")))return FLinearColor(.95,.66,.19);if(Label.Contains(TEXT("RADIO")))return FLinearColor(.12,.85,.9);if(Label.Contains(TEXT("UTILITY")))return FLinearColor(.61,.88,.42);return FLinearColor(.75,.83,.89);}
}
void ULunarWireBoard::SetPanel(ULunarOperationsPanel* Panel){OwnerPanel=Panel;SetVisibility(ESlateVisibility::Visible);SetClipping(EWidgetClipping::ClipToBounds);}
UFieldOperationsComponent* ULunarWireBoard::Operations() const{
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));auto* C=G?G->FieldOperations.Get():nullptr;
 return C&&OwnerPanel&&OwnerPanel->View==EFieldPanelView::Wire&&C->CanUsePanel(OwnerPanel,false)?C:nullptr;
}
TSharedRef<SWidget> ULunarWireBoard::RebuildWidget(){if(!WidgetTree)WidgetTree=NewObject<UWidgetTree>(this);auto* Root=WidgetTree->ConstructWidget<USizeBox>();Root->SetMinDesiredHeight(300);WidgetTree->RootWidget=Root;return UUserWidget::RebuildWidget();}
FVector2D ULunarWireBoard::PortPosition(bool Right,int32 Index,FVector2D Size) const{const auto* C=Operations();const int32 Count=C?C->WirePairs():3;return FVector2D(Size.X*(Right?.73:.27),Size.Y*(.18+.64*Index/FMath::Max(1,Count-1)));}
int32 ULunarWireBoard::HitPort(bool Right,FVector2D P,FVector2D Size) const{const auto* C=Operations();if(!C)return INDEX_NONE;const float Radius=FMath::Clamp(float(Size.X/30),22.f,34.f);for(int32 I=0;I<C->WirePairs();++I)if(FVector2D::Distance(P,PortPosition(Right,I,Size))<=Radius)return I;return INDEX_NONE;}
bool ULunarWireBoard::BeginCable(int32 Source){auto* C=Operations();const auto* G=C?C->Game():nullptr;if(!C||!G||G->Apollo||G->Stage!=ELunarStage::RestorePower||G->RepairStep!=1||!C->CanUsePanel(OwnerPanel,true)||!C->WireIsolated||C->WireComplete||Source<0||Source>=C->WirePairs()||(C->WireMask&(1<<Source)))return false;DragSource=Source;return true;}
bool ULunarWireBoard::ReleaseCable(int32 Socket){const int32 Source=DragSource;DragSource=INDEX_NONE;auto* C=Operations();if(!C||Source==INDEX_NONE||Socket<0||Socket>=C->WirePairs())return false;
 const bool Connected=C->PerformAction(TEXT("WireLeft"),Source)&&C->PerformAction(TEXT("WireRight"),Socket);if(!Connected)ErrorFade=.5f;return Connected;
}
void ULunarWireBoard::NativeTick(const FGeometry& G,float D){UUserWidget::NativeTick(G,D);if(!FMath::IsFinite(D)||D<=0)return;if(auto* C=Operations();C&&C->WireMask!=LastMask){LastMask=C->WireMask;Transition=0;}Transition=FMath::Min(1.f,Transition+D*5);ErrorFade=FMath::Max(0.f,ErrorFade-D);}
FReply ULunarWireBoard::NativeOnMouseButtonDown(const FGeometry& G,const FPointerEvent& E){if(E.GetEffectingButton()!=EKeys::LeftMouseButton)return FReply::Unhandled();Cursor=G.AbsoluteToLocal(E.GetScreenSpacePosition());if(!BeginCable(HitPort(false,Cursor,G.GetLocalSize())))return FReply::Unhandled();return FReply::Handled().CaptureMouse(TakeWidget());}
FReply ULunarWireBoard::NativeOnMouseButtonUp(const FGeometry& G,const FPointerEvent& E){if(E.GetEffectingButton()!=EKeys::LeftMouseButton||DragSource==INDEX_NONE)return FReply::Unhandled();Cursor=G.AbsoluteToLocal(E.GetScreenSpacePosition());ReleaseCable(HitPort(true,Cursor,G.GetLocalSize()));return FReply::Handled().ReleaseMouseCapture();}
FReply ULunarWireBoard::NativeOnMouseMove(const FGeometry& G,const FPointerEvent& E){if(DragSource==INDEX_NONE)return FReply::Unhandled();Cursor=G.AbsoluteToLocal(E.GetScreenSpacePosition());return FReply::Handled();}
void ULunarWireBoard::NativeOnMouseCaptureLost(const FCaptureLostEvent& E){DragSource=INDEX_NONE;UUserWidget::NativeOnMouseCaptureLost(E);}
int32 ULunarWireBoard::NativePaint(const FPaintArgs& A,const FGeometry& G,const FSlateRect& Clip,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled)const{
 Layer=UUserWidget::NativePaint(A,G,Clip,Out,Layer,Style,Enabled);const auto* C=Operations();const auto Size=G.GetLocalSize();if(!C||Size.X<20||Size.Y<20)return Layer;
 const auto* Brush=FCoreStyle::Get().GetBrush("WhiteBrush");const float TextScale=FMath::Clamp(float(Size.X/1100),.72f,1.25f)*FMath::Clamp(ULunarSettings::Get()->TextScale,1.f,1.5f);const float Scale=FMath::Min(TextScale,float(Size.Y*.64/(FMath::Max(1,C->WirePairs()-1)*70)));const auto Font=FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(15*TextScale));
 auto Line=[&](const TArray<FVector2D>& Points,int32 Z,FLinearColor Color,float Width){FSlateDrawElement::MakeLines(Out,Z,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Width);};
 auto Box=[&](FVector2D At,FVector2D Ext,int32 Z,FLinearColor Color){FSlateDrawElement::MakeBox(Out,Z,G.ToPaintGeometry(Ext,FSlateLayoutTransform(At)),Brush,ESlateDrawEffect::None,Color);};
 auto Ring=[&](FVector2D At,float Radius,int32 Z,FLinearColor Color,float Width){TArray<FVector2D> Points;for(int32 I=0;I<=32;++I){const double T=I*2*UE_PI/32;Points.Add(At+FVector2D(FMath::Cos(T),FMath::Sin(T))*Radius);}Line(Points,Z,Color,Width);};
 auto Cable=[&](FVector2D Start,FVector2D End,FLinearColor Color,float Progress){TArray<FVector2D> Points;const FVector2D Control1=Start+FVector2D((End.X-Start.X)*.4,18),Control2=End-FVector2D((End.X-Start.X)*.4,-18);for(int32 I=0;I<=36;++I){const double T=(I/36.)*Progress,U=1-T;Points.Add(Start*U*U*U+Control1*3*U*U*T+Control2*3*U*T*T+End*T*T*T);}Line(Points,Layer+1,FLinearColor(.002,.004,.006),13*Scale);Line(Points,Layer+2,Color*.48f,8*Scale);Line(Points,Layer+3,Color,4*Scale);for(int32 I=2;I<Points.Num()-1;I+=2){const auto N=(Points[I+1]-Points[I-1]).GetSafeNormal();const FVector2D Normal(-N.Y,N.X);Line({Points[I]-Normal*3*Scale,Points[I]+Normal*3*Scale},Layer+4,FLinearColor(.08,.12,.14,.55),1.3f*Scale);} };
 for(int32 I=0;I<C->WirePairs();++I){const auto Color=CableColor(C->WireLeftLabel(I));if(C->WireMask&(1<<I))for(int32 J=0;J<C->WirePairs();++J)if(C->WireLeftLabel(I)==C->WireRightLabel(J))Cable(PortPosition(false,I,Size),PortPosition(true,J,Size),Color,FMath::InterpEaseOut(0.f,1.f,Transition,3.f));}
 if(DragSource!=INDEX_NONE)Cable(PortPosition(false,DragSource,Size),Cursor,CableColor(C->WireLeftLabel(DragSource)),1);
 for(bool Right:{false,true})for(int32 I=0;I<C->WirePairs();++I){const auto At=PortPosition(Right,I,Size);const FString Label=Right?C->WireRightLabel(I):C->WireLeftLabel(I);auto Color=CableColor(Label);if(ErrorFade>0&&Right)Color=FLinearColor(1,.25,.2);const float R=14*Scale;Box(At-FVector2D(21,29)*Scale,FVector2D(42,58)*Scale,Layer+5,FLinearColor(.06,.085,.10));Ring(At,R+2,Layer+6,FLinearColor(.30,.38,.42),1.1f*Scale);Ring(At,R,Layer+7,Color,2.8f*Scale);Ring(At,2,Layer+7,Color,2*Scale);for(float Sign:{-1.f,1.f})Ring(At+FVector2D(0,Sign*23*Scale),1.5f*Scale,Layer+7,FLinearColor(.35,.4,.44),1);
  const FVector2D TextAt(Right?At.X+34*Scale:Size.X*.025,At.Y-10*Scale);FSlateDrawElement::MakeText(Out,Layer+8,G.ToPaintGeometry(FVector2D(190*Scale,30*Scale),FSlateLayoutTransform(TextAt)),Label,Font,ESlateDrawEffect::None,FLinearColor(.86,.91,.94));
  // A different shape accompanies each signal colour.
  const FVector2D Symbol=TextAt-FVector2D(15*Scale,-8*Scale);if(Label.Contains(TEXT("SENSOR")))Line({Symbol+FVector2D(0,-7)*Scale,Symbol+FVector2D(7,0)*Scale,Symbol+FVector2D(0,7)*Scale,Symbol+FVector2D(-7,0)*Scale,Symbol+FVector2D(0,-7)*Scale},Layer+8,Color,1.5f);else if(Label.Contains(TEXT("POWER")))Line({Symbol+FVector2D(3,-8)*Scale,Symbol+FVector2D(-3,0)*Scale,Symbol+FVector2D(3,0)*Scale,Symbol+FVector2D(-3,8)*Scale},Layer+8,Color,2.8f);else Ring(Symbol,6*Scale,Layer+8,Color,1.5f);
 }
 return Layer+9;
}
