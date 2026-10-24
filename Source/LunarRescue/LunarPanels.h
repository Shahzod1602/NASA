#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Commands/InputChord.h"
#include "LunarPanels.generated.h"

UCLASS()
class LUNARRESCUE_API ULunarSettingRow : public UUserWidget {
 GENERATED_BODY()
public:
 FName Setting;
 UPROPERTY() TObjectPtr<class UTextBlock> ValueLabel;
 UPROPERTY() TObjectPtr<class UInputKeySelector> Selector;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 UFUNCTION() void Changed(float V);
 UFUNCTION() void Clicked();
 UFUNCTION() void Selected(FInputChord Chord);
 void Refresh();
};
UCLASS()
class LUNARRESCUE_API ULunarPanel : public UUserWidget {
 GENERATED_BODY()
public:
 bool Settings=false;
 int32 EvidenceIndex=0;
 UPROPERTY() TObjectPtr<class UVerticalBox> Body;
 UPROPERTY() TObjectPtr<class UTexture2D> Photo;
 virtual void NativeConstruct() override;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 void Rebuild();
 UFUNCTION() void Close();
 UFUNCTION() void Previous();
 UFUNCTION() void Next();
 UFUNCTION() void ResetSettings();
};
