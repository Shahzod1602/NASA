#include "LunarLoadingTypography.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"

namespace {
const FLinearColor LoadingAccent(.025f,.85f,.9f),LoadingInk(.96f,.98f,1),LoadingDim(.64f,.73f,.78f);
FVector2D TextExtent(const FString& Text,const FSlateFontInfo& Font){
 if(FSlateApplication::IsInitialized())return FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Text,Font);
 // Startup fallback only. Normal MoviePlayer construction already has Slate.
 return FVector2D(Text.Len()*Font.Size*.82f,Font.Size*1.5f);
}
TArray<FString> WrapLoading(const FString& Text,const FSlateFontInfo& Font,float Width){
 TArray<FString> Words,Lines;Text.ParseIntoArray(Words,TEXT(" "),true);FString Current;
 for(const auto& Word:Words){const FString Next=Current.IsEmpty()?Word:Current+TEXT(" ")+Word;
  if(!Current.IsEmpty()&&TextExtent(Next,Font).X>Width){Lines.Add(Current);Current=Word;}else Current=Next;
 }
 if(!Current.IsEmpty())Lines.Add(Current);return Lines;
}
}
float LunarLoading::LoadingTextScale(float Requested){return FMath::IsFinite(Requested)?FMath::Clamp(Requested,1.f,1.5f):1.f;}
LunarLoading::FTypographyLayout LunarLoading::BuildTypography(const FString& Map,float RequestedScale){
 FTypographyLayout Layout;Layout.TextScale=LoadingTextScale(RequestedScale);Layout.SlateMeasured=FSlateApplication::IsInitialized();
 const bool Mars=Map==TEXT("MarsBase"),Menu=Map==TEXT("MissionSelect"),Legacy=Map==TEXT("MoonBase");
 auto Font=[&](int32 Size,bool Bold){return FCoreStyle::GetDefaultFontStyle(Bold?TEXT("Bold"):TEXT("Regular"),FMath::RoundToInt(Size*Layout.TextScale));};
 auto Add=[&](FName Role,const FString& Text,float X,float Y,int32 Size,FLinearColor Color,bool Bold=false){
  FTypographyLine Line;Line.Role=Role;Line.Text=Text;Line.Position=FVector2D(X,Y);Line.Font=Font(Size,Bold);Line.Extent=TextExtent(Text,Line.Font);Line.Color=Color;Layout.Lines.Add(Line);return Line.Extent.Y;
 };
 Add(TEXT("expedition"),Mars?TEXT("LUNARRESCUE  /  EXPEDITION 02"):TEXT("LUNARRESCUE  /  EXPEDITION 01"),52,31,9,LoadingAccent,true);
 Add(TEXT("destination"),Menu?TEXT("EXPEDITIONS"):Mars?TEXT("MARS"):TEXT("MOON"),50,55,32,LoadingInk,true);
 for(const auto& Line:Layout.Lines){Layout.HeaderWidth=FMath::Max(Layout.HeaderWidth,float(Line.Position.X+Line.Extent.X+38));Layout.HeaderHeight=FMath::Max(Layout.HeaderHeight,float(Line.Position.Y+Line.Extent.Y+20));}
 const FString Mission=Menu?TEXT("CHOOSE YOUR NEXT EXPEDITION"):Mars?TEXT("ARES / FICTIONAL PROTOTYPE"):Legacy?TEXT("SELENE / FICTIONAL RESCUE"):TEXT("APOLLO 12 / SURVEYOR'S LEGACY");
 const FString Guide=Menu?TEXT("Choose a destination, then select a mission and start when ready. Continue returns to your saved Apollo checkpoint."):
  Mars?TEXT("Recover the power cell and restore ARES station power. Download the ice survey, align the relay and return to the lander. Watch your oxygen budget; the crew recorder is optional."):
  Legacy?TEXT("Restore SELENE power and recover the research. Seal the lunar samples, recover NOMAD-7 memory and lock Earthlink. Return to the lander; the crew recorder is optional."):
  TEXT("Check your equipment, deploy ALSEP and document three samples. Recover Surveyor 3 hardware and return the cargo to Intrepid. Follow the objective marker; open the journal for the route.");
 // One measured left text column and one separate progress column. Logical
 // gutters stay fixed; accessibility text increases footer height, not overlap.
 const int32 FooterFirst=Layout.Lines.Num();float Y=24;
 for(const auto& Line:WrapLoading(Mission,Font(17,true),790)){Y+=Add(TEXT("mission"),Line,52,Y,17,LoadingInk,true)+4*Layout.TextScale;}
 Y+=10;Y+=Add(TEXT("guide-label"),TEXT("MISSION GUIDE"),53,Y,9,LoadingAccent,true)+10;
 for(const auto& Line:WrapLoading(Guide,Font(11,false),790)){Y+=Add(TEXT("guide"),Line,53,Y,11,LoadingInk)+4*Layout.TextScale;}
 Y+=20;Y+=Add(TEXT("source-label"),TEXT("Illustrated journey / fictional spacecraft / not to scale"),53,Y,9,LoadingDim);
 const float FooterHeight=FMath::Max(184.f+60*(Layout.TextScale-1),Y+22);Layout.FooterTop=720-FooterHeight;
 for(int32 Index=FooterFirst;Index<Layout.Lines.Num();++Index)Layout.Lines[Index].Position.Y+=Layout.FooterTop;
 Layout.RingRadius=18+6*(Layout.TextScale-1);
 // Anchor the whole progress column from its bottom edge using the same
 // measured fonts that will be painted. Fixed baselines let 150% text overlap.
 const float StatusHeight=TextExtent(TEXT("PREPARING DESTINATION"),Font(9,false)).Y;
 const float PercentHeight=TextExtent(TEXT("100 %"),Font(16,true)).Y;
 const float LoadingHeight=TextExtent(TEXT("LOADING"),Font(9,true)).Y;
 const float CaptionHeight=TextExtent(TEXT("Preparation progress"),Font(9,false)).Y;
 const float StatusY=704-StatusHeight;
 const float PercentY=StatusY-10-PercentHeight;
 const float LoadingY=PercentY-6-LoadingHeight;
 const float CaptionY=LoadingY-12-CaptionHeight;
 Layout.RingCenter.Y=(LoadingY+PercentY+PercentHeight)*.5f;
 Add(TEXT("progress-caption"),TEXT("Preparation progress"),944,CaptionY,9,LoadingDim);
 Add(TEXT("loading-label"),TEXT("LOADING"),1015,LoadingY,9,LoadingInk,true);
 Add(TEXT("percent"),TEXT("100 %"),1015,PercentY,16,LoadingInk,true);
 Add(TEXT("status"),TEXT("PREPARING DESTINATION"),944,StatusY,9,LoadingDim);
 return Layout;
}
