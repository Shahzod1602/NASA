#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunarTrailMap.generated.h"

UCLASS()
class LUNARRESCUE_API ULunarTrailMap : public UUserWidget {
 GENERATED_BODY()
public:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& Geometry,const FSlateRect& CullingRect,FSlateWindowElementList& Elements,int32 Layer,const FWidgetStyle& Style,bool Enabled) const override;
};
