#pragma once
#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

namespace LunarLoading {
// Logical 1280x720 space is uniformly fitted to the viewport. Text extents are
// measured once before movie playback, never from UObjects on the loading thread.
struct FTypographyLine {
 FName Role;FString Text;FVector2D Position=FVector2D::ZeroVector,Extent=FVector2D::ZeroVector;
 FSlateFontInfo Font;FLinearColor Color=FLinearColor::White;
};
struct FTypographyLayout {
 float TextScale=1,FooterTop=536,HeaderWidth=560,HeaderHeight=124;
 FVector2D RingCenter=FVector2D(975,647);float RingRadius=18;
 bool SlateMeasured=false;
 TArray<FTypographyLine> Lines;
};
float LoadingTextScale(float Requested);
FTypographyLayout BuildTypography(const FString& Map,float RequestedScale);
}
