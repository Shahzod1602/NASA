#include "LunarLearning.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarMissionCard.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Kismet/GameplayStatics.h"

namespace {
bool LessonIndex(int32 Index){return Index>=0&&Index<LunarLearning::Count;}
bool LessonAnswer(int32 Answer){return Answer==0||Answer==1;}
ALunarGameMode* LessonGame(const UObject* Context){return Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(Context));}
bool LessonPanelActive(const ULunarPanel* Panel){
 const auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Panel,0));const auto* Game=LessonGame(Panel);
 return Pilot&&Pilot->ActivePanel==Panel&&!Panel->Settings&&Game&&!Game->Menu&&(!Game->IsActive()||Game->Paused);
}
UTextBlock* LessonText(ULunarPanel* Panel,const FString& Value,int32 Size,FLinearColor Color){
 auto* Label=Panel->WidgetTree->ConstructWidget<UTextBlock>();auto Font=Label->GetFont();Font.Size=FMath::RoundToInt(Size*ULunarSettings::Get()->TextScale);Font.TypefaceFontName=TEXT("Regular");Label->SetFont(Font);Label->SetColorAndOpacity(Color);Label->SetText(FText::FromString(Value));Label->SetAutoWrapText(true);Label->SetWrapTextAt(Panel->EvidenceContentWidth()-36);Label->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);return Label;
}
UButton* LessonButton(ULunarPanel* Panel,const FString& Value,bool Selected=false,float WrapWidth=0){
 auto* Control=Panel->WidgetTree->ConstructWidget<ULunarMissionButton>();auto Style=Control->GetStyle();
 const FLinearColor Outline(.12f,.55f,.69f,.8f),Accent(.05f,.82f,.94f);
 Style.Normal=FSlateRoundedBoxBrush(Selected?FLinearColor(.025f,.18f,.29f,.95f):FLinearColor(.012f,.036f,.055f,.94f),3.f,Selected?Accent:Outline,1.f);
 Style.Hovered=FSlateRoundedBoxBrush(FLinearColor(.025f,.23f,.33f,.97f),3.f,Accent,1.f);
 Style.Pressed=FSlateRoundedBoxBrush(FLinearColor(.015f,.3f,.39f,.98f),3.f,Accent,1.f);
 Style.NormalPadding=Style.PressedPadding=FMargin(14,12);Control->SetStyle(Style);auto* Caption=LessonText(Panel,Value,19,FLinearColor(.91f,.96f,1));if(WrapWidth>0)Caption->SetWrapTextAt(WrapWidth);Control->AddChild(Caption);return Control;
}
void AnswerFromPanel(ULunarPanel* Panel,int32 Answer){
 if(!LessonPanelActive(Panel)||!Panel->ShowFieldLessons)return;
 if(LunarLearning::RecordAnswer(LessonGame(Panel),Panel->FieldLessonIndex,Answer))Panel->Rebuild();
}
}

FString LunarLearning::Question(const ALunarGameMode* Game,int32 Index){
 if(!Game||!LessonIndex(Index))return FString();
 if(Index==0)return Game->Apollo?TEXT("What controls the large-scale ground elevations in this Apollo 12 chapter?"):FString::Printf(TEXT("Is the %s terrain a measured NASA elevation map?"),Game->Mars?TEXT("ARES canyon"):TEXT("SELENE rescue"));
 if(Index==1)return Game->Apollo?TEXT("Why does this game ask for a photograph before collecting a sample or removing hardware?"):FString::Printf(TEXT("Does recovering the %s establish a new NASA scientific result?"),Game->Mars?TEXT("ice survey"):TEXT("station science data"));
 return TEXT("What does an active oxygen or exercise countdown represent here?");
}
FString LunarLearning::Choice(const ALunarGameMode* Game,int32 Index,int32 Answer){
 if(!Game||!LessonIndex(Index)||!LessonAnswer(Answer))return FString();
 if(Index==0){
  if(Game->Apollo)return Answer==0?TEXT("Every elevation was shaped by hand."):TEXT("Heights from the LROC NAC Apollo 12 terrain product.");
  return Answer==0?TEXT("No. It is authored gameplay terrain."):TEXT("Yes. Every ground point is a NASA measurement.");
 }
 if(Index==1){
  if(Game->Apollo)return Answer==0?TEXT("To keep a record of the item's original context."):TEXT("To refill the oxygen budget.");
  return Answer==0?TEXT("Yes. Mission completion verifies a real NASA discovery."):TEXT("No. Data recovery belongs to the fictional scenario.");
 }
 return Answer==0?TEXT("An adjustable gameplay resource budget."):TEXT("Verified real-world spacesuit endurance.");
}
int32 LunarLearning::CorrectAnswer(const ALunarGameMode* Game,int32 Index){if(!Game||!LessonIndex(Index))return INDEX_NONE;return Index==0?(Game->Apollo?1:0):Index==1?(Game->Apollo?0:1):0;}
FString LunarLearning::Explanation(const ALunarGameMode* Game,int32 Index,int32 Answer){
 if(!Game||!LessonIndex(Index))return FString();
 FString Note;
 if(Index==0)Note=Game->Apollo?TEXT("Modern LROC NAC terrain heights shape the ground and work-site elevations. The source post spacing is 2 metres. Small fragments, equipment and tool motion are authored; extra visual detail does not add measured terrain resolution."):TEXT("This rescue map is authored terrain. Using a NASA-based planetary gravity value does not make the canyon or station a measured NASA map. Apollo 12 is a separate chapter with imported LROC elevations.");
 else if(Index==1)Note=Game->Apollo?TEXT("A before-work photograph records appearance and location before an item is disturbed. NASA's Apollo 12 account describes Surveyor 3 being photographed before parts were removed. Requiring photographs at the game's selected sample sites is an authored adaptation."):TEXT("ARES and SELENE are fictional rescue scenarios. Their terminal downloads are story objectives, not a measured dataset or a new discovery. Real planetary facts and any measured terrain must be attributed separately.");
 else Note=TEXT("Countdown values are tuned game limits, not a breathing model or a real suit endurance rating. Apollo 12 defaults to untimed exploration; its timed exercise is optional. Rescue oxygen is also a gameplay budget.");
 return (LessonAnswer(Answer)?FString(Answer==CorrectAnswer(Game,Index)?TEXT("This matches the field note. "):TEXT("Review the field note. ")):FString())+Note;
}
FString LunarLearning::Source(const ALunarGameMode* Game,int32 Index){
 if(!Game||!LessonIndex(Index))return FString();
 if(Index==0&&Game->Apollo)return TEXT("Product: NAC_DTM_APOLLO12; NASA/GSFC/ASU, LROC science team. Bundled product note: NAC_DTM_APOLLO12_README.TXT. Official archive: https://pds.mcp.nasa.gov/data/store/img/lunar_reconnaissance_orbiter/pds4/lroc/lro-l-lroc-5-rdr/LROLRC_2001/DATA/SDP/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_README.TXT");
 if(Index==1&&Game->Apollo)return TEXT("Historical support: NASA, Apollo 12: The Pinpoint Mission, Mission Highlights (Surveyor photography before hardware retrieval). https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/");
 if(Index==0)return Game->Mars?TEXT("Project source: Tools/build_mars.py creates the authored canyon. See SCIENCE_AND_SOURCES and MARS_UPDATE for the fiction/data boundary."):TEXT("Project source: Tools/build_moon.py creates the authored SELENE map. See SCIENCE_AND_SOURCES for the fiction/data boundary.");
 if(Index==1)return TEXT("Project source: LunarGame.cpp mission narrative and objectives; README and SCIENCE_AND_SOURCES describe the rescue prototype. This note is about the game's authored content, not a NASA research claim.");
 return TEXT("Gameplay source: LunarGame.cpp (AdvanceTime), LunarApollo.cpp (exercise mode and budget), and SCIENCE_AND_SOURCES. These rules are authored gameplay tuning, not NASA suit specifications.");
}
bool LunarLearning::RecordAnswer(ALunarGameMode* Game,int32 Index,int32 Answer){
 if(!Game||!LessonIndex(Index)||!LessonAnswer(Answer)||!Game->FieldLessonAnswers.IsValidIndex(Index))return false;
 Game->FieldLessonAnswers[Index]=Answer;return true;
}
int32 LunarLearning::Reviewed(const ALunarGameMode* Game){int32 Total=0;if(Game)for(int32 Index=0;Index<Count;++Index)if(Game->FieldLessonAnswers.IsValidIndex(Index)&&LessonAnswer(Game->FieldLessonAnswers[Index]))++Total;return Total;}
int32 LunarLearning::Correct(const ALunarGameMode* Game){int32 Total=0;if(Game)for(int32 Index=0;Index<Count;++Index)if(Game->FieldLessonAnswers.IsValidIndex(Index)&&LessonAnswer(Game->FieldLessonAnswers[Index])&&Game->FieldLessonAnswers[Index]==CorrectAnswer(Game,Index))++Total;return Total;}
FString LunarLearning::SessionSummary(const ALunarGameMode* Game){const int32 Answered=Reviewed(Game);return Answered?FString::Printf(TEXT("Field lessons answered %d/3; latest answers correct %d/%d (this session)."),Answered,Correct(Game),Answered):FString();}
void ALunarGameMode::ResetFieldLessons(){FieldLessonAnswers.Init(INDEX_NONE,LunarLearning::Count);}

void ULunarPanel::BuildFieldLessons(){
 if(!Body||Settings||!ShowFieldLessons)return;auto* Game=LessonGame(this);if(!Game)return;
 FieldLessonIndex=FMath::Clamp(FieldLessonIndex,0,LunarLearning::Count-1);const int32 Answer=Game->FieldLessonAnswers.IsValidIndex(FieldLessonIndex)?Game->FieldLessonAnswers[FieldLessonIndex]:INDEX_NONE;
 const FLinearColor Ink(.91f,.96f,1),Dim(.59f,.71f,.8f),Cyan(.08f,.83f,.94f),Gold(.98f,.72f,.34f);
 auto Add=[&](UWidget* Widget){Body->AddChildToVerticalBox(Widget)->SetPadding(FMargin(4,8));};
 Add(LessonText(this,TEXT("FIELD LESSONS"),26,Cyan));
 Add(LessonText(this,TEXT("Optional. Answers stay in this session. They do not change cargo or mission resources."),17,Dim));
 Add(LessonText(this,FString::Printf(TEXT("QUESTION %d / 3   |   ANSWERED %d / 3"),FieldLessonIndex+1,LunarLearning::Reviewed(Game)),18,Dim));
 Add(LessonText(this,LunarLearning::Question(Game,FieldLessonIndex),23,Ink));
 auto* First=LessonButton(this,FString(Answer==0?TEXT("1  [SELECTED]  "):TEXT("1  "))+LunarLearning::Choice(Game,FieldLessonIndex,0),Answer==0);First->OnClicked.AddDynamic(this,&ULunarPanel::AnswerFieldLessonA);Add(First);
 auto* Second=LessonButton(this,FString(Answer==1?TEXT("2  [SELECTED]  "):TEXT("2  "))+LunarLearning::Choice(Game,FieldLessonIndex,1),Answer==1);Second->OnClicked.AddDynamic(this,&ULunarPanel::AnswerFieldLessonB);Add(Second);
 if(LessonAnswer(Answer)){
  Add(LessonText(this,TEXT("FIELD NOTE"),20,Answer==LunarLearning::CorrectAnswer(Game,FieldLessonIndex)?Cyan:Gold));
  Add(LessonText(this,LunarLearning::Explanation(Game,FieldLessonIndex,Answer),18,Ink));
  Add(LessonText(this,LunarLearning::Source(Game,FieldLessonIndex),15,Dim));
  Add(LessonText(this,TEXT("You can revise your answer. The summary records your latest choice."),16,Dim));
 }else Add(LessonText(this,TEXT("Choose an answer to read the field note and its source."),17,Dim));
 auto* LessonNavigation=WidgetTree->ConstructWidget<UHorizontalBox>();auto* PreviousControl=LessonButton(this,TEXT("< Previous lesson"),false,EvidenceContentWidth()*.5f-42);auto* NextControl=LessonButton(this,TEXT("Next lesson >"),false,EvidenceContentWidth()*.5f-42);
 PreviousControl->OnClicked.AddDynamic(this,&ULunarPanel::PreviousFieldLesson);NextControl->OnClicked.AddDynamic(this,&ULunarPanel::NextFieldLesson);
 auto* PreviousLayout=LessonNavigation->AddChildToHorizontalBox(PreviousControl);PreviousLayout->SetSize(FSlateChildSize(ESlateSizeRule::Fill));PreviousLayout->SetPadding(FMargin(0,0,8,0));LessonNavigation->AddChildToHorizontalBox(NextControl)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Add(LessonNavigation);
 Add(LessonText(this,TEXT("1 / 2: answer   |   Left / Right: question   |   Esc: close"),16,Dim));
 const FString Summary=LunarLearning::SessionSummary(Game);if(!Summary.IsEmpty())Add(LessonText(this,Summary,17,Cyan));
}
void ULunarPanel::ToggleFieldLessons(){if(!LessonPanelActive(this))return;ShowFieldLessons=!ShowFieldLessons;if(ShowFieldLessons)ShowExpeditionMap=false;Rebuild();}
void ULunarPanel::AnswerFieldLessonA(){AnswerFromPanel(this,0);}
void ULunarPanel::AnswerFieldLessonB(){AnswerFromPanel(this,1);}
void ULunarPanel::NextFieldLesson(){if(LessonPanelActive(this)&&ShowFieldLessons){FieldLessonIndex=(FMath::Clamp(FieldLessonIndex,0,LunarLearning::Count-1)+1)%LunarLearning::Count;Rebuild();}}
void ULunarPanel::PreviousFieldLesson(){if(LessonPanelActive(this)&&ShowFieldLessons){FieldLessonIndex=(FMath::Clamp(FieldLessonIndex,0,LunarLearning::Count-1)+LunarLearning::Count-1)%LunarLearning::Count;Rebuild();}}
