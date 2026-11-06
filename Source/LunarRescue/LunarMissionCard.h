#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "LunarMissionCard.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarMissionCardEvent,int32,Id);

// Keyboard selection is centrally owned by the menu; avoid a second Enter handler.
UCLASS()
class LUNARRESCUE_API ULunarMissionButton : public UButton {
 GENERATED_BODY()
public:
 ULunarMissionButton(const FObjectInitializer& ObjectInitializer);
};

// Action buttons retain one full-size glass surface and a visible keyboard-focus
// state. Mission card Enter handling remains centrally owned by the menu.
UCLASS()
class LUNARRESCUE_API ULunarFrontendButton : public UButton {
 GENERATED_BODY()
public:
 ULunarFrontendButton(const FObjectInitializer& ObjectInitializer);
 void SetGlassStyle(const FButtonStyle& Style);
 bool ValidateSurface(bool Arranged,FString& Reason) const;
 UPROPERTY() TObjectPtr<class UBorder> GlassSurface;
 UPROPERTY() TObjectPtr<class UTextBlock> Caption;
private:
 void FocusGlass();
 void UnfocusGlass();
 UPROPERTY() FButtonStyle UnfocusedStyle;
};

// Shared presentation for expedition entries and playable mission cards.
// Choosing a card only emits its ID; navigation and travel belong to the menu.
UCLASS()
class LUNARRESCUE_API ULunarMissionCard : public UUserWidget {
 GENERATED_BODY()
public:
 void Configure(int32 Id,const FString& Title,const FString& Subtitle,const FString& Description,bool Compact=false);
 void SetSelected(bool Selected);
 bool IsSelected() const {return SelectedState;}
 float SelectionBlend() const {return SelectionValue;}
 int32 PresentationUpdateCount() const {return PresentationUpdates;}
 void AdvancePresentation(float Delta);
 UPROPERTY() TObjectPtr<class UButton> Button;
 UPROPERTY() TObjectPtr<class UTextBlock> SelectionLabel;
 UPROPERTY() FLunarMissionCardEvent OnChosen;
 UPROPERTY() FLunarMissionCardEvent OnPreview;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
 UFUNCTION() void Choose();
 UFUNCTION() void Hover();
 UFUNCTION() void Unhover();
private:
 void Focus();
 void Unfocus();
 void RefreshText();
 void ApplyPresentation();
 int32 CardId=INDEX_NONE;
 FString CardTitle,CardSubtitle,CardDescription;
 bool CompactState=false,SelectedState=false,HoveredState=false,FocusedState=false,FullGlassBaseline=false;
 int32 PresentationUpdates=0;
 float SelectionValue=0,SelectionStart=0,SelectionElapsed=.28f;
 float HoverValue=0,CachedTextScale=1;
 UPROPERTY() TObjectPtr<class UBackgroundBlur> Frost;
 UPROPERTY() TObjectPtr<class UBorder> GlowOuter;
 UPROPERTY() TObjectPtr<class UBorder> GlowInner;
 UPROPERTY() TObjectPtr<class UBorder> Face;
 UPROPERTY() TObjectPtr<class UBorder> Edge;
 UPROPERTY() TObjectPtr<class UBorder> Glint;
 UPROPERTY() TObjectPtr<class UBorder> Accent;
 UPROPERTY() TObjectPtr<class UTextBlock> TitleLabel;
 UPROPERTY() TObjectPtr<class UTextBlock> SubtitleLabel;
 UPROPERTY() TObjectPtr<class UTextBlock> DescriptionLabel;
};
