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
 UPROPERTY() TObjectPtr<class UComboBoxString> Choices;
 UFUNCTION() void ChoiceChanged(FString Choice,ESelectInfo::Type Type);
 UFUNCTION() UWidget* OptionWidget(FString Option);
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
 int32 SettingsTab=0;
 bool BuiltSettings=false;
 UPROPERTY() TObjectPtr<class UTextBlock> DisplayStatus;
 UPROPERTY() TObjectPtr<class UVerticalBox> SettingsBody;
 virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
 void BuildSettings();
 void FillSettings();
 UFUNCTION() void VideoTab();
 UFUNCTION() void SoundTab();
 UFUNCTION() void GameplayTab();
 UFUNCTION() void ControlsTab();
 UFUNCTION() void KeepDisplay();
 UFUNCTION() void RevertDisplay();
 UFUNCTION() void TestAudio();
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

UCLASS()
class LUNARRESCUE_API ULunarMainMenu : public UUserWidget {
 GENERATED_BODY()
public:
 UPROPERTY() TObjectPtr<class UImage> Background;
 UPROPERTY() TObjectPtr<class UTextBlock> Destination;
 UPROPERTY() TObjectPtr<class UTextBlock> Details;
 UPROPERTY() TObjectPtr<class UButton> MoonButton;
 UPROPERTY() TObjectPtr<class UButton> MarsButton;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 UFUNCTION() void PreviewMoon();
 UFUNCTION() void PreviewMars();
 UFUNCTION() void ExploreMoon();
 UFUNCTION() void ExploreMars();
 UFUNCTION() void Continue();
 UFUNCTION() void Settings();
 UFUNCTION() void ExitGame();
};
