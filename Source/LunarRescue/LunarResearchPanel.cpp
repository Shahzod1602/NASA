#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarCrew.h"
#include "LunarDeparture.h"
#include "LunarLoading.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
namespace {
ALunarGameMode* ResearchGame(const UObject* Object){return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(Object));}
const FLinearColor ResearchInk(.92f,.96f,.98f),ResearchDim(.72f,.82f,.87f),ResearchCyan(.1f,.85f,.95f),ResearchGold(.99f,.79f,.44f);
UTextBlock* ResearchText(ULunarPanel* P,const FString& S,int32 Font=16,FLinearColor Color=ResearchInk){auto* T=P->WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(S));T->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Font*ULunarSettings::Get()->TextScale)));T->SetColorAndOpacity(Color);T->SetAutoWrapText(true);T->SetWrapTextAt(P->EvidenceContentWidth()-48);return T;}
void ResearchRow(ULunarPanel* P,UWidget* W){P->Body->AddChildToVerticalBox(W)->SetPadding(FMargin(4,8));}
UButton* ResearchButton(ULunarPanel* P,const FString& S){auto* B=P->WidgetTree->ConstructWidget<UButton>();B->SetContent(ResearchText(P,S,16));B->SetBackgroundColor(FLinearColor(.055f,.17f,.23f,.95f));B->SetColorAndOpacity(FLinearColor::White);return B;}
bool ResearchPanelOwned(const ULunarPanel* P){const auto* Pilot=P?Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(P,0)):nullptr;return Pilot&&Pilot->ActivePanel==P&&P->IsInViewport()&&P->IsVisible()&&!P->Settings;}
bool WaterPanelActive(const ULunarPanel* P,const ALunarGameMode* G){return ResearchPanelOwned(P)&&G&&P->ShowWaterStudy&&!G->Mars&&!G->Menu&&G->IsActive()&&G->Paused&&!ALunarDepartureScene::Active(G)&&!LunarLoading::IsBusy()&&LunarWater::Dataset().Available;}
void WaterAnswer(ULunarPanel* P,int32 Choice){auto* G=ResearchGame(P);if(!WaterPanelActive(P,G))return;LunarWater::RecordAnswer(G->WaterStudy,Choice);P->Rebuild();P->SetKeyboardFocus();}
void ResearchBar(ULunarPanel* P,const FString& Label,double Percent,FLinearColor Color){ResearchRow(P,ResearchText(P,FString::Printf(TEXT("%s  %.1f mass%%"),*Label,Percent),16,Color));auto* Size=P->WidgetTree->ConstructWidget<USizeBox>();Size->SetHeightOverride(8);auto* Bar=P->WidgetTree->ConstructWidget<UProgressBar>();Bar->SetPercent(float(FMath::Clamp(Percent/10.,0.,1.)));Bar->SetFillColorAndOpacity(Color);Size->SetContent(Bar);ResearchRow(P,Size);}
}
void ALunarCharacter::OpenCrewNotebook(){if(ActivePanel)return;auto* G=ResearchGame(this);if(!G||G->Menu||ALunarDepartureScene::Active(G))return;OpenEvidence();if(ActivePanel){ActivePanel->ShowCrew=true;ActivePanel->ShowWaterStudy=ActivePanel->ShowFieldLessons=ActivePanel->ShowExpeditionMap=false;ActivePanel->Rebuild();ActivePanel->SetKeyboardFocus();}}
void ALunarCharacter::OpenWaterNotebook(){if(ActivePanel)return;auto* G=ResearchGame(this);if(!G||G->Mars||G->Menu||ALunarDepartureScene::Active(G))return;OpenEvidence();if(ActivePanel){ActivePanel->ShowWaterStudy=true;ActivePanel->ShowCrew=ActivePanel->ShowFieldLessons=ActivePanel->ShowExpeditionMap=false;ActivePanel->Rebuild();ActivePanel->SetKeyboardFocus();}}
void ULunarPanel::ToggleCrew(){if(!ResearchPanelOwned(this))return;ShowCrew=!ShowCrew;ShowWaterStudy=ShowFieldLessons=ShowExpeditionMap=ShowWaterSources=false;Rebuild();SetKeyboardFocus();}
void ULunarPanel::ToggleWaterStudy(){auto* G=ResearchGame(this);if(!ResearchPanelOwned(this)||!G||G->Mars)return;ShowWaterStudy=!ShowWaterStudy;ShowCrew=ShowFieldLessons=ShowExpeditionMap=ShowWaterSources=false;Rebuild();SetKeyboardFocus();}
void ULunarPanel::ToggleWaterSources(){if(!ResearchPanelOwned(this)||!ShowWaterStudy)return;ShowWaterSources=!ShowWaterSources;Rebuild();SetKeyboardFocus();}
void ULunarPanel::WaterAnswerA(){WaterAnswer(this,0);}void ULunarPanel::WaterAnswerB(){WaterAnswer(this,1);}void ULunarPanel::WaterAnswerC(){WaterAnswer(this,2);}
void ULunarPanel::CommitWaterReport(){auto* G=ResearchGame(this);if(!WaterPanelActive(this,G)||G->WaterStudy.Recorded||!LunarWater::Commit(G->WaterStudy))return;G->Message(TEXT("MIRA / Remote LCROSS research report recorded. Historical cargo and mission resources are unchanged."));if(G->Apollo)G->SaveApolloCheckpoint();Rebuild();SetKeyboardFocus();}
void ULunarPanel::ResetWaterReport(){auto* G=ResearchGame(this);if(!ResearchPanelOwned(this)||!ShowWaterStudy||!G||G->Mars||G->Menu||ALunarDepartureScene::Active(G)||LunarLoading::IsBusy()||!LunarWater::Dataset().Available)return;const bool Invalid=!LunarWater::IsValidState(G->WaterStudy);if(!Invalid&&(!WaterPanelActive(this,G)||G->WaterStudy.Recorded))return;LunarWater::Reset(G->WaterStudy);Rebuild();SetKeyboardFocus();}
void ULunarPanel::BuildWaterStudy(){
 auto* G=ResearchGame(this);if(!G||Settings||!Body||!WidgetTree)return;
 ResearchRow(this,ResearchText(this,TEXT("WATER RESEARCH / REMOTE LCROSS EVIDENCE"),24,ResearchCyan));
 ResearchRow(this,ResearchText(this,TEXT("Optional modern research exercise / Cabeus, 2009. This is separate from the historical Apollo 12 reconstruction. You are analysing published observations, not detecting water at your landing site."),16,ResearchDim));
 if(G->Mars){ResearchRow(this,ResearchText(this,TEXT("This lunar evidence exercise is unavailable in the fictional ARES mission."),18,ResearchGold));return;}
 const auto& D=LunarWater::Dataset();if(!D.Available){ResearchRow(this,ResearchText(this,TEXT("The bundled LCROSS source record is unavailable or invalid. The main mission remains playable. No substitute measurements are generated."),18,ResearchGold));return;}
 if(!LunarWater::IsValidState(G->WaterStudy)){ResearchRow(this,ResearchText(this,TEXT("This optional research state is invalid. Reset only the exercise; mission cargo and resources stay unchanged."),18,ResearchGold));auto* Reset=ResearchButton(this,TEXT("Reset this optional exercise"));Reset->OnClicked.AddDynamic(this,&ULunarPanel::ResetWaterReport);ResearchRow(this,Reset);ResearchRow(this,ResearchText(this,D.Citation+TEXT("\n")+D.SourceURL,15,ResearchDim));return;}
 ResearchRow(this,ResearchText(this,LunarWater::StateSummary(G->WaterStudy),19,ResearchGold));
 ResearchRow(this,ResearchText(this,FString::Printf(TEXT("PUBLISHED RESULT / estimated water fraction %.1f +/- %.1f percentage points by mass. Observed field-of-view maximum: %.0f +/- %.0f kg water vapour and ice."),D.WaterPercent,D.UncertaintyPoints,D.ObservedWaterKg,D.ObservedWaterUncertaintyKg),17));
 ResearchRow(this,ResearchText(this,TEXT("Comparison scale: 0-10 mass%. The outer bars are simple estimate-minus/plus-uncertainty arithmetic; they are not guaranteed limits or a stated confidence interval."),15,ResearchDim));
 ResearchBar(this,TEXT("Estimate - published uncertainty"),D.WaterPercent-D.UncertaintyPoints,ResearchDim);ResearchBar(this,TEXT("Published estimate"),D.WaterPercent,ResearchCyan);ResearchBar(this,TEXT("Estimate + published uncertainty"),D.WaterPercent+D.UncertaintyPoints,ResearchGold);
 ResearchRow(this,ResearchText(this,TEXT("HYPOTHETICAL CALCULATION / Apply this fraction to an assumed 100 kg of regolith only to compare the numbers. No such game sample was measured; the observation cannot be generalized to every lunar site."),16,ResearchDim));
 if(!G->WaterStudy.Feedback.IsEmpty())ResearchRow(this,ResearchText(this,G->WaterStudy.Feedback,17,ResearchGold));
 if(G->WaterStudy.Recorded){const FString Persistence=G->Apollo?(G->ApolloSaveError?TEXT("The latest checkpoint write failed; persistence is not confirmed. "):TEXT("Successful Apollo checkpoint saves also retain this optional record. ")):TEXT("This mission keeps the report for this session only. ");ResearchRow(this,ResearchText(this,TEXT("REPORT RECORDED / Kept in this session. ")+Persistence+TEXT("No sample, repair, oxygen or primary objective was awarded."),18,G->Apollo&&G->ApolloSaveError?ResearchGold:ResearchCyan));}
 else if(!G->IsActive())ResearchRow(this,ResearchText(this,TEXT("Start the mission to conduct the optional exercise. Source information remains available here."),18,ResearchGold));
 else if(!LunarWater::ReadyToCommit(G->WaterStudy)){
  ResearchRow(this,ResearchText(this,FString::Printf(TEXT("DECISION %d / 3"),G->WaterStudy.Step+1),20,ResearchCyan));ResearchRow(this,ResearchText(this,LunarWater::Question(G->WaterStudy),19));const auto Options=LunarWater::Choices(G->WaterStudy);
  for(int32 I=0;I<Options.Num()&&I<3;++I){auto* B=ResearchButton(this,FString::Printf(TEXT("[ %d ]  %s"),I+1,*Options[I]));if(I==0)B->OnClicked.AddDynamic(this,&ULunarPanel::WaterAnswerA);else if(I==1)B->OnClicked.AddDynamic(this,&ULunarPanel::WaterAnswerB);else B->OnClicked.AddDynamic(this,&ULunarPanel::WaterAnswerC);ResearchRow(this,B);}
 }else{auto* Record=ResearchButton(this,TEXT("[ Enter ] Record remote research report"));Record->OnClicked.AddDynamic(this,&ULunarPanel::CommitWaterReport);ResearchRow(this,Record);}
 if(!G->WaterStudy.Recorded&&G->WaterStudy.Step>0){auto* Retry=ResearchButton(this,TEXT("Restart this optional exercise"));Retry->OnClicked.AddDynamic(this,&ULunarPanel::ResetWaterReport);ResearchRow(this,Retry);}
 auto* Sources=ResearchButton(this,ShowWaterSources?TEXT("Hide Science & Sources"):TEXT("Science & Sources / measured, inferred and illustrative"));Sources->OnClicked.AddDynamic(this,&ULunarPanel::ToggleWaterSources);ResearchRow(this,Sources);
 if(ShowWaterSources){ResearchRow(this,ResearchText(this,LunarWater::EvidenceText(),16));ResearchRow(this,ResearchText(this,D.Citation+TEXT("\n")+D.SourceURL+TEXT("\nDOI: ")+D.DOI+TEXT("\nData record: ")+D.Id,15,ResearchDim));}
 ResearchRow(this,ResearchText(this,LunarWater::ModelLimit()+TEXT("\nReading this notebook pauses the gameplay resource clock. This is a learning interaction, not an EVA physiology model."),15,ResearchDim));
}
