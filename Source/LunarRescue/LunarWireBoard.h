#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunarWireBoard.generated.h"

class ULunarOperationsPanel;
class UFieldOperationsComponent;

// Bounded presentation and drag input; the operations component owns every
// match, isolation, resource and mission-stage decision.
UCLASS()
class LUNARRESCUE_API ULunarWireBoard : public UUserWidget {
 GENERATED_BODY()
public:
 void SetPanel(ULunarOperationsPanel* Panel);
 bool BeginCable(int32 Source);
 bool ReleaseCable(int32 Socket);
 FVector2D PortPosition(bool Right,int32 Index,FVector2D Size) const;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual int32 NativePaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool) const override;
 virtual void NativeTick(const FGeometry&,float Delta) override;
 virtual FReply NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&) override;
 virtual FReply NativeOnMouseButtonUp(const FGeometry&,const FPointerEvent&) override;
 virtual FReply NativeOnMouseMove(const FGeometry&,const FPointerEvent&) override;
 virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent&) override;
private:
 UFieldOperationsComponent* Operations() const;
 int32 HitPort(bool Right,FVector2D Position,FVector2D Size) const;
 UPROPERTY() TObjectPtr<ULunarOperationsPanel> OwnerPanel;
 int32 DragSource=INDEX_NONE,LastMask=0;
 FVector2D Cursor=FVector2D::ZeroVector;
 float Transition=1,ErrorFade=0;
};
