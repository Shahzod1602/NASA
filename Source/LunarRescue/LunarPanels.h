#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Commands/InputChord.h"
#include "LunarIntro.h"
#include "LunarPanels.generated.h"

struct FLunarFrontendLabel {
 TWeakObjectPtr<class UTextBlock> Widget;
 int32 BaseSize=12;
 bool Scales=true;
};

UCLASS()
class LUNARRESCUE_API ULunarSettingRow : public UUserWidget {
 GENERATED_BODY()
public:
 FName Setting;
 float RowTextScale=-1;
 virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
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
 TArray<FLunarFrontendLabel> FrontendLabels;
 float FrontendTextScale=-1;
 bool ShowCrew=false,ShowWaterStudy=false,ShowWaterSources=false;
 void BuildWaterStudy();
 UFUNCTION() void ToggleCrew();
 UFUNCTION() void ToggleWaterStudy();
 UFUNCTION() void ToggleWaterSources();
 UFUNCTION() void WaterAnswerA();
 UFUNCTION() void WaterAnswerB();
 UFUNCTION() void WaterAnswerC();
 UFUNCTION() void CommitWaterReport();
 UFUNCTION() void ResetWaterReport();
 bool ShowExpeditionMap=false;
 bool ShowFieldLessons=false;
 int32 FieldLessonIndex=0;
 void BuildFieldLessons();
 UFUNCTION() void ToggleFieldLessons();
 UFUNCTION() void AnswerFieldLessonA();
 UFUNCTION() void AnswerFieldLessonB();
 UFUNCTION() void NextFieldLesson();
 UFUNCTION() void PreviousFieldLesson();
 UFUNCTION() void ToggleExpeditionMap();
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
 float EvidenceLayoutWidth=0;
 float EvidenceContentWidth() const;
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
 TArray<FLunarFrontendLabel> FrontendLabels;
 float FrontendTextScale=-1;
 virtual void NativeTick(const FGeometry& Geometry,float Delta) override;
 UPROPERTY() TObjectPtr<class UImage> Background;
 UPROPERTY() TObjectPtr<class UTextBlock> Destination;
 UPROPERTY() TObjectPtr<class UTextBlock> Details;
 UPROPERTY() TArray<TObjectPtr<class ULunarMissionCard>> HomeCards;
 UPROPERTY() TArray<TObjectPtr<class ULunarMissionCard>> ExtraHomeCards;
 UPROPERTY() TObjectPtr<class UCanvasPanel> IntroLayer;
 UPROPERTY() TObjectPtr<class UImage> IntroImage;
 FLunarIntroState IntroState;
 bool IsIntroActive() const {return IntroState.Active;}
 void BeginIntro(bool Force=false);
 void AdvanceIntro(float Delta);
 UPROPERTY() TArray<TObjectPtr<class ULunarMissionCard>> MissionCards;
 UPROPERTY() TObjectPtr<class UCanvasPanel> HomePage;
 UPROPERTY() TObjectPtr<class UCanvasPanel> SelectionPage;
 UPROPERTY() TObjectPtr<class UTextBlock> SelectionTitle;
 UPROPERTY() TObjectPtr<class UTextBlock> SelectionPrompt;
 UPROPERTY() TObjectPtr<class UButton> LaunchButton;
 UPROPERTY() TObjectPtr<class UButton> SelectionBackButton;
 UPROPERTY() TObjectPtr<class UButton> SelectionSettingsButton;
 UPROPERTY() TObjectPtr<class UTexture2D> MoonBackdrop;
 UPROPERTY() TObjectPtr<class UTexture2D> MarsBackdrop;
 int32 DestinationTextureLoads=0,DestinationPresentationUpdates=0,FrontendBuilds=0;
 bool DestinationPreviewReady=false,SelectionProfileRequested=false;
 int32 SelectionProfileStep=0;
 double NextSelectionProfileAt=0;

 bool MissionSelection=false,SelectionMars=false,PreviewingMars=false;
 int32 SelectedMission=INDEX_NONE;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 void OpenSelection(bool Mars);
 void ConfirmSelection();
 void MoveSelection(int32 Direction);
 UFUNCTION() void Back();
 UFUNCTION() void SelectMission(int32 Index);
 UFUNCTION() void LaunchMission();
 UFUNCTION() void PreviewHome(int32 Index);
 UFUNCTION() void ChooseHome(int32 Index);
 UFUNCTION() void PreviewMoon();
 UFUNCTION() void PreviewMars();
 UFUNCTION() void ExploreMoon();
 UFUNCTION() void ExploreMars();
 UFUNCTION() void Continue();
 UFUNCTION() void Settings();
 UFUNCTION() void ExitGame();
};
