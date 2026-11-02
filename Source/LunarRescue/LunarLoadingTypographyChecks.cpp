#include "LunarLoading.h"
#include "LunarLoadingTypography.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Fonts/FontMeasure.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <limits>

namespace {
bool RectanglesOverlap(FVector2D A,FVector2D AS,FVector2D B,FVector2D BS){return A.X<B.X+BS.X-.1f&&A.X+AS.X>B.X+.1f&&A.Y<B.Y+BS.Y-.1f&&A.Y+AS.Y>B.Y+.1f;}
const LunarLoading::FTypographyLine* Role(const LunarLoading::FTypographyLayout& Layout,FName Name){return Layout.Lines.FindByPredicate([Name](const auto& Line){return Line.Role==Name;});}
}
void LunarLoading::RunTypographyChecks(const UObject* Context){
 FString Report;int32 Failures=0;auto Check=[&](bool Good,const FString& Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failures;};
 Check(Context&&FSlateApplication::IsInitialized(),TEXT("Loading layout checks have actual Slate font measurement"));
 Check(LoadingTextScale(-1)==1&&LoadingTextScale(9)==1.5f&&LoadingTextScale(std::numeric_limits<float>::quiet_NaN())==1&&LoadingTextScale(std::numeric_limits<float>::infinity())==1,TEXT("Accessibility scale rejects non-finite values and stays in its configured range"));
 for(const TCHAR* Map:{TEXT("Apollo12"),TEXT("MoonBase"),TEXT("MarsBase"),TEXT("MissionSelect")}){
  const auto Default=BuildTypography(Map,1),Large=BuildTypography(Map,1.5f);
  Check(Default.FooterTop>488&&Large.FooterTop<Default.FooterTop,FString::Printf(TEXT("%s gives the illustration more room by default and expands for 150%% text"),Map));
  for(float Scale:{1.f,1.5f}){
   const auto Layout=BuildTypography(Map,Scale);const FString Label=FString::Printf(TEXT("%s at %.0f%%"),Map,Scale*100);
   bool Within=true,Separate=true,Gutter=true;int32 GuideLines=0;
   for(int32 I=0;I<Layout.Lines.Num();++I){const auto& Line=Layout.Lines[I];Within&=!Line.Position.ContainsNaN()&&!Line.Extent.ContainsNaN()&&Line.Position.X>=40&&Line.Position.Y>=20&&Line.Position.X+Line.Extent.X<=1240&&Line.Position.Y+Line.Extent.Y<=710;
    if(Line.Role==TEXT("guide")){++GuideLines;Gutter&=Line.Position.X+Line.Extent.X<=844;}
    for(int32 J=I+1;J<Layout.Lines.Num();++J)Separate&=!RectanglesOverlap(Line.Position,Line.Extent,Layout.Lines[J].Position,Layout.Lines[J].Extent);
    Separate&=!RectanglesOverlap(Line.Position,Line.Extent,Layout.RingCenter-FVector2D(Layout.RingRadius),FVector2D(Layout.RingRadius*2));
   }
   Check(Layout.SlateMeasured&&Within,Label+TEXT(" measured text stays inside safe screen bounds"));
   Check(Separate&&Gutter&&GuideLines>0,Label+TEXT(" text and progress ring do not overlap and keep their column gutter"));
   Check(Layout.FooterTop>Layout.HeaderHeight+80,Label+TEXT(" retains an open illustration area between header and footer"));
   const auto* Planet=Role(Layout,TEXT("destination"));const auto* Mission=Role(Layout,TEXT("mission"));const auto* Guide=Role(Layout,TEXT("guide"));const auto* Source=Role(Layout,TEXT("source-label"));
   Check(Planet&&Mission&&Guide&&Planet->Font.Size==FMath::RoundToInt(32*Scale)&&Mission->Font.Size==FMath::RoundToInt(17*Scale)&&Guide->Font.Size==FMath::RoundToInt(11*Scale),Label+TEXT(" applies the smaller hierarchy while respecting the selected text scale"));
   Check(Source&&Source->Text.Contains(TEXT("Illustrated"))&&Source->Text.Contains(TEXT("fictional"))&&Source->Text.Contains(TEXT("not to scale")),Label+TEXT(" retains the illustration and fictional-spacecraft disclosure"));
   bool MeterFits=true;const auto* Percent=Role(Layout,TEXT("percent"));const auto* Status=Role(Layout,TEXT("status"));
   if(!Percent||!Status||!FSlateApplication::IsInitialized())MeterFits=false;
   else{
    const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    for(int32 Value=0;Value<=100;++Value)MeterFits&=Measure->Measure(FString::Printf(TEXT("%d %%"),Value),Percent->Font).X<=Percent->Extent.X+.1f;
    for(const TCHAR* Text:{TEXT("PREPARING DESTINATION"),TEXT("TERRAIN & EQUIPMENT"),TEXT("READY TO EXPLORE")})MeterFits&=Measure->Measure(FString(Text),Status->Font).X<=Status->Extent.X+.1f;
   }
   Check(MeterFits,Label+TEXT(" has room for every displayed percent and preparation stage caption"));
  }
 }
 Check(LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_LoadingMoon.T_LoadingMoon"))&&LoadObject<UTexture2D>(nullptr,TEXT("/Game/Lunar/Frontend/T_LoadingMars.T_LoadingMars")),TEXT("Both local illustrated loading backgrounds are available"));
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);const FString Folder=FPaths::ProjectDir()/TEXT("Reports");IFileManager::Get().MakeDirectory(*Folder,true);FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("loading-typography-checks.txt")));UE_LOG(LogTemp,Display,TEXT("LOADING_TYPOGRAPHY_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
}
