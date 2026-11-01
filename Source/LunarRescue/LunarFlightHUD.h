#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunarFlightHUD.generated.h"

class ALunarGameMode;
class ALunarCharacter;
class UTextBlock;
class UBorder;
class UCanvasPanel;
class UProgressBar;

// A read-only view of gameplay. Holds use game time; tool phases remain click actions.
struct FLunarFlightHUDState {
 bool Visible=false,Apollo=false,BoundedResource=true,ResourceWarning=false;
 bool Holding=false,ShowGuide=false,HasNavigation=false,Crosshair=true,PhotoFrame=false;
 bool DustAnswers=false,Beacon=false;
 int32 Tool=0,Completed=0,Total=0,DustChoice=0;
 float ResourceFraction=0,HoldFraction=0,AntennaSignal=0,BeaconPhase=0;
 FVector2D Level=FVector2D::ZeroVector;
 FVector NavigationWorld=FVector::ZeroVector;
 float NavigationDistance=0;
 FString Expedition,Mission,Stage,Objective,Subtask,Counter;
 FString ResourceLabel,ResourceTime,Gravity,Warning;
 FString GuideCaption,GuideTitle,GuideBody,GuideAction,Notice,Subtitle;
 FString WorkKey,RouteKey,JournalKey,MoveKeys,NavigationName,HoldTitle,RouteHint;
};

UCLASS()
class LUNARRESCUE_API ULunarFlightHUD : public UUserWidget {
 GENERATED_BODY()
public:
 static bool ShouldShow(const ALunarGameMode* Game,const ALunarCharacter* Player);
 static float HoldFraction(const ALunarGameMode* Game);
 static FLunarFlightHUDState ReadState(const ALunarGameMode* Game,const ALunarCharacter* Player);
 void Refresh();
 const FLunarFlightHUDState& State() const {return Current;}
 UPROPERTY() TObjectPtr<UTextBlock> ObjectiveLabel;
 UPROPERTY() TObjectPtr<UTextBlock> ResourceLabel;
 UPROPERTY() TObjectPtr<UTextBlock> GuideLabel;
 UPROPERTY() TObjectPtr<UTextBlock> SubtitleLabel;
 UPROPERTY() TObjectPtr<UTextBlock> CounterLabel;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeTick(const FGeometry& Geometry,float DeltaTime) override;
 virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& Geometry,const FSlateRect& Culling,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const override;
private:
 UTextBlock* MakeText(float Size,FLinearColor Color,bool Subtitle=false);
 FLunarFlightHUDState Current;
 UPROPERTY() TObjectPtr<UCanvasPanel> Content;
 UPROPERTY() TObjectPtr<UWidget> MissionPanel;
 UPROPERTY() TObjectPtr<UWidget> GuidePanel;
 UPROPERTY() TObjectPtr<UWidget> RadioPanel;
 UPROPERTY() TObjectPtr<UWidget> ToolsPanel;
 UPROPERTY() TObjectPtr<class UVerticalBox> ToolAnswers;
 UPROPERTY() TObjectPtr<UTextBlock> ExpeditionLabel;
 UPROPERTY() TObjectPtr<UTextBlock> MissionLabel;
 UPROPERTY() TObjectPtr<UTextBlock> StageLabel;
 UPROPERTY() TObjectPtr<UTextBlock> SubtaskLabel;
 UPROPERTY() TObjectPtr<UTextBlock> ResourceCaption;
 UPROPERTY() TObjectPtr<UTextBlock> GravityLabel;
 UPROPERTY() TObjectPtr<UTextBlock> WarningLabel;
 UPROPERTY() TObjectPtr<UTextBlock> GuideCaption;
 UPROPERTY() TObjectPtr<UTextBlock> GuideTitle;
 UPROPERTY() TObjectPtr<UTextBlock> GuideAction;
 UPROPERTY() TObjectPtr<UTextBlock> NoticeLabel;
 UPROPERTY() TObjectPtr<UProgressBar> OxygenBar;
 UPROPERTY() TArray<TObjectPtr<UTextBlock>> AnswerLabels;
 UPROPERTY() TArray<TObjectPtr<UTextBlock>> FontLabels;
 UPROPERTY() TArray<TObjectPtr<UBorder>> Faces;
 TArray<float> FontSizes;
 TArray<bool> SubtitleFonts;
 float CachedTextScale=-1,CachedSubtitleScale=-1,CachedOpacity=-1,Fade=0;
 float RadioTop=710,GuideTop=540;
 int32 CachedTool=INDEX_NONE;
};

