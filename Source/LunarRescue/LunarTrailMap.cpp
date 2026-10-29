#include "LunarTrailMap.h"
#include "LunarTrail.h"
#include "LunarGame.h"
#include "LunarSettings.h"
#include "ApolloData.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"

TSharedRef<SWidget> ULunarTrailMap::RebuildWidget(){return SNew(SBox).MinDesiredHeight(480);}
int32 ULunarTrailMap::NativePaint(const FPaintArgs& Args,const FGeometry& Geo,const FSlateRect& Culling,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const {
 Layer=Super::NativePaint(Args,Geo,Culling,Out,Layer,Style,Enabled);
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));if(!G)return Layer;
 const auto Trail=G->ApolloTrailSnapshot();FBox2D Crop(ForceInit);
 auto Include=[&](FVector P){FVector2D UV;if(LunarTrail::MapUV(P,UV))Crop+=UV;};
 Include(ApolloData::Home);Include(ApolloData::Surveyor);for(const auto& P:ApolloData::Samples)Include(P);for(const auto& P:ApolloData::Instruments)Include(P);
 for(const auto& Segment:ApolloData::ReferenceRoute){Include(Segment[0]);Include(Segment[1]);}for(const auto& P:Trail)Include(P.Position);
 Crop.Min=FVector2D(FMath::Max(0.,Crop.Min.X-.04),FMath::Max(0.,Crop.Min.Y-.04));Crop.Max=FVector2D(FMath::Min(1.,Crop.Max.X+.04),FMath::Min(1.,Crop.Max.Y+.04));
 const FVector2D Span=Crop.GetSize(),Available=Geo.GetLocalSize();const float Aspect=float(Span.X*104800./(Span.Y*84800.));
 const float Height=FMath::Min(float(Available.Y),float(Available.X)/Aspect);
 const FVector2D Size(Height*Aspect,Height),Origin((Available.X-Size.X)*.5f,0);
 auto Position=[&](FVector P){FVector2D UV;LunarTrail::MapUV(P,UV);return Origin+(UV-Crop.Min)/Span*Size;};
 auto Line=[&](FVector2D A,FVector2D B,FLinearColor Color,float Width){TArray<FVector2D> Points{A,B};FSlateDrawElement::MakeLines(Out,Layer+1,Geo.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Width);};
 const FLinearColor Gold(.98,.73,.35),Cyan(.20,.74,.83),Ink(.94,.97,.95);
 if(G->ApolloMapTexture){FSlateBrush Brush;Brush.SetResourceObject(G->ApolloMapTexture);Brush.ImageSize=Size;Brush.SetUVRegion(Crop);FSlateDrawElement::MakeBox(Out,Layer,Geo.ToPaintGeometry(Size,FSlateLayoutTransform(Origin)),&Brush,ESlateDrawEffect::None,FLinearColor(.42,.49,.50));}
 for(const auto& Segment:ApolloData::ReferenceRoute){auto A=Position(Segment[0]),B=Position(Segment[1]);Line(A,B,Cyan,.9f);}
 for(int32 I=1;I<Trail.Num();++I)if(!Trail[I].BreakBefore)Line(Position(Trail[I-1].Position),Position(Trail[I].Position),Gold,2.f);
 auto Mark=[&](FVector P,const FString& Name,FLinearColor C,bool Packed){const auto M=Position(P);const float R=Packed?5:3;
  Line(M+FVector2D(-R,-R),M+FVector2D(R,-R),C,2);Line(M+FVector2D(R,-R),M+FVector2D(R,R),C,2);Line(M+FVector2D(R,R),M+FVector2D(-R,R),C,2);Line(M+FVector2D(-R,R),M+FVector2D(-R,-R),C,2);
  if(Packed){Line(M-FVector2D(R,0),M+FVector2D(R,0),C,2);Line(M-FVector2D(0,R),M+FVector2D(0,R),C,2);}
  const FVector2D Label(FMath::Clamp(float(M.X+8),float(Origin.X+3),float(Origin.X+Size.X-120)),FMath::Clamp(float(M.Y+5),4.f,float(Size.Y-30)));
  FSlateDrawElement::MakeText(Out,Layer+2,Geo.ToPaintGeometry(FVector2D(150,30),FSlateLayoutTransform(Label)),Name,FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(16*ULunarSettings::Get()->TextScale)),ESlateDrawEffect::None,C);
 };
 Mark(ApolloData::Home,TEXT("INTREPID"),Ink,false);
 const TCHAR* Names[]={TEXT("12031"),TEXT("12032"),TEXT("12027")};
 for(int32 I=0;I<3;++I)if((G->ApolloPhotos|G->SamplesCollected)&(1<<I))Mark(ApolloData::Samples[I],Names[I],(G->SamplesCollected&(1<<I))?Gold:Cyan,(G->SamplesCollected&(1<<I))!=0);
 if(G->SurveyorPhotographed||G->ApolloParts)Mark(ApolloData::Surveyor,TEXT("SURVEYOR 3"),G->ApolloParts?Gold:Cyan,G->ApolloParts!=0);
 if(!Trail.IsEmpty()){const auto End=Position(Trail.Last().Position);Line(End-FVector2D(0,7),End+FVector2D(0,7),Gold,2);Line(End-FVector2D(7,0),End+FVector2D(7,0),Gold,2);}
 return Layer+3;
}
