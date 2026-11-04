#pragma once
#include "CoreMinimal.h"
#include "LunarPanels.h"
#include "LunarFieldOperations.h"
#include "Components/Button.h"
#include "LunarOperationsPanel.generated.h"

class UFieldOperationsComponent;
class UTextBlock;
class UScrollBox;
class UTexture2D;
class ULunarOperationsPanel;

struct FLunarOperationsMapPoint {
 FVector World=FVector::ZeroVector;
 FString Name;
 bool Player=false,Completed=false,Current=false;
};

// Bounded, cached map presentation. World locations are supplied by operations;
// painting never searches actors, changes gameplay, or downloads map data.
UCLASS()
class LUNARRESCUE_API ULunarOperationsMap : public UUserWidget {
 GENERATED_BODY()
public:
 void SetPoints(const TArray<FLunarOperationsMapPoint>& InPoints,UTexture2D* InTexture);
 bool IsOrbitalMap() const {return OrbitalMap;}
 int32 MarkerCount() const {return Points.Num();}
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& Geometry,const FSlateRect& Culling,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const override;
private:
 TArray<FLunarOperationsMapPoint> Points;
 UPROPERTY() TObjectPtr<UTexture2D> MapTexture;
 FBox2D Bounds=FBox2D(ForceInit);
 bool OrbitalMap=false;
};

UCLASS()
class LUNARRESCUE_API ULunarOperationsButton : public UButton {
 GENERATED_BODY()
public:
 UPROPERTY() TObjectPtr<ULunarOperationsPanel> OwnerPanel;
 int32 Action=INDEX_NONE;
 int32 Generation=0;
 UFUNCTION() void Activate();
};

// Lives in ALunarCharacter::ActivePanel, retaining the existing pause/close path.
// The operations component remains the authority for every action and guard.
UCLASS()
class LUNARRESCUE_API ULunarOperationsPanel : public ULunarPanel {
 GENERATED_BODY()
public:
 EFieldPanelView View=EFieldPanelView::Briefing;
 bool ShowSources=false;
 void Configure(EFieldPanelView InView);
 void Refresh(bool Force=false);
 void Dispatch(int32 Action,int32 Generation);
 bool ValidateLayout(FString& Reason) const;
 UPROPERTY() TObjectPtr<UVerticalBox> OperationsBody;
 UPROPERTY() TObjectPtr<UScrollBox> OperationsScroll;
 UPROPERTY() TObjectPtr<ULunarOperationsMap> OperationsMap;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeConstruct() override;
 virtual void NativeTick(const FGeometry& Geometry,float DeltaTime) override;
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
private:
 UFieldOperationsComponent* Operations() const;
 UTextBlock* Text(const FString& Value,int32 Size=17,FLinearColor Color=FLinearColor(.91f,.96f,.98f));
 ULunarOperationsButton* Button(const FString& Caption,FName Action,int32 Argument=0,bool Enabled=true,bool Selected=false);
 void Row(UWidget* Widget,float VerticalPadding=6);
 void Heading(const FString& Value);
 void Note(const FString& Value);
 void Status(const FString& Name,const FString& Value,bool Good=true);
 void BuildPage();
 void BuildMap();
 void CloseOwnedPanel();
 float ContentWidth() const;
 int32 Generation=0;
 FString LastFingerprint;
 struct FAction {FName Name;int32 Argument=0;};
 TArray<FAction> Actions;
 float LastScale=-1,LastWidth=-1,RefreshElapsed=0;
};
