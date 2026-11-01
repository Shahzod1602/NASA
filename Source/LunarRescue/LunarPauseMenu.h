#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunarPauseMenu.generated.h"

// This view only presents pause actions. The character owns pause physics,
// input mode and full-restart/checkpoint semantics.
UCLASS()
class LUNARRESCUE_API ULunarPauseMenu : public UUserWidget {
 GENERATED_BODY()
public:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeConstruct() override;
 virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
 void NotifyOpened();
 void Sync();
 void AdvancePresentation(float Delta);
 float EntryBlend() const {return OpenBlend;}
 bool IsConfirmingRestart() const {return ConfirmingRestart;}
 bool HandleBack();
 void Navigate(int32 Direction);
 void Activate();
 UFUNCTION() void Resume();
 UFUNCTION() void Restart();
 UFUNCTION() void ConfirmRestart();
 UFUNCTION() void CancelRestart();
 UFUNCTION() void Settings();
 UFUNCTION() void MainMenu();
 UFUNCTION() void ExitGame();
 UFUNCTION() void ChooseAction(int32 Index);
 UFUNCTION() void PreviewAction(int32 Index);
 UPROPERTY() TArray<TObjectPtr<class ULunarMissionCard>> ActionCards;
 UPROPERTY() TObjectPtr<class UTextBlock> MissionText;
 UPROPERTY() TObjectPtr<class UTextBlock> ObjectiveText;
 UPROPERTY() TObjectPtr<class UTextBlock> ObjectiveDetail;
 UPROPERTY() TObjectPtr<class UTextBlock> ResourceText;
 UPROPERTY() TObjectPtr<class UTextBlock> ConfirmationBody;
 UPROPERTY() TObjectPtr<class UCanvasPanel> ConfirmationLayer;
 UPROPERTY() TObjectPtr<class UButton> RestartButton;
 UPROPERTY() TObjectPtr<class UButton> CancelButton;
private:
 bool CanAct() const;
 void ShowConfirmation(bool Show);
 UPROPERTY() TObjectPtr<class UBackgroundBlur> WorldBlur;
 UPROPERTY() TObjectPtr<class UBorder> WorldDim;
 UPROPERTY() TObjectPtr<class UScaleBox> Presentation;
 UPROPERTY() TArray<TObjectPtr<class UTextBlock>> Labels;
 TArray<int32> LabelSizes;
 bool ConfirmingRestart=false;
 int32 FocusedAction=0;
 float OpenElapsed=0,OpenBlend=0,RefreshElapsed=0,CachedTextScale=1;
};
