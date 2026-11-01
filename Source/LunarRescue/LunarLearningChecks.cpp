#include "LunarLearning.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

void ALunarGameMode::NasaLearningChecks(){
 FString Report;int32 Failures=0;
 auto Check=[&](bool Good,const FString& Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failures;};
 auto Finish=[&](){const FString Folder=FPaths::ProjectDir()/TEXT("Reports");IFileManager::Get().MakeDirectory(*Folder,true);Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("field-learning-checks.txt")));FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);};
 auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 Check(Pilot&&!Menu,TEXT("Field lessons start in a playable mission with a player"));if(!Pilot||Menu){Finish();return;}
 const bool WasApollo=Apollo,WasMars=Mars;
 ResetFieldLessons();Check(FieldLessonAnswers.Num()==3&&LunarLearning::Reviewed(this)==0&&LunarLearning::Correct(this)==0&&LunarLearning::SessionSummary(this).IsEmpty(),TEXT("A new lesson session has three unanswered items and no debrief claim"));
 Check(LunarLearning::Reviewed(nullptr)==0&&LunarLearning::Correct(nullptr)==0&&LunarLearning::Question(nullptr,0).IsEmpty()&&LunarLearning::CorrectAnswer(nullptr,0)==INDEX_NONE,TEXT("Missing mission context has no question, score or correct answer"));
 for(int32 Mission=0;Mission<3;++Mission){
  Apollo=Mission==0;Mars=Mission==2;bool Complete=true;
  for(int32 Index=0;Index<3;++Index)Complete=Complete&&!LunarLearning::Question(this,Index).IsEmpty()&&!LunarLearning::Choice(this,Index,0).IsEmpty()&&!LunarLearning::Choice(this,Index,1).IsEmpty()&&LunarLearning::Choice(this,Index,0)!=LunarLearning::Choice(this,Index,1)&&!LunarLearning::Explanation(this,Index).IsEmpty()&&!LunarLearning::Source(this,Index).IsEmpty()&&LunarLearning::CorrectAnswer(this,Index)>=0&&LunarLearning::CorrectAnswer(this,Index)<=1;
  Check(Complete,FString::Printf(TEXT("Mission %d has three complete questions, distinct choices, explanations and sources"),Mission));
  Check(LunarLearning::Choice(this,0,LunarLearning::CorrectAnswer(this,0)).Contains(Apollo?TEXT("LROC"):TEXT("authored")),FString::Printf(TEXT("Mission %d identifies measured versus authored terrain correctly"),Mission));
  Check(Apollo?LunarLearning::Source(this,1).Contains(TEXT("https://www.nasa.gov/missions/apollo/apollo-12-the-pinpoint-mission/")):LunarLearning::Choice(this,1,LunarLearning::CorrectAnswer(this,1)).Contains(TEXT("fictional")),FString::Printf(TEXT("Mission %d distinguishes sourced history from the fictional data-recovery scenario"),Mission));
 }
 Apollo=WasApollo;Mars=WasMars;
 Check(!LunarLearning::RecordAnswer(this,-1,0)&&!LunarLearning::RecordAnswer(this,3,0)&&!LunarLearning::RecordAnswer(this,0,-1)&&!LunarLearning::RecordAnswer(this,0,2)&&LunarLearning::Reviewed(this)==0,TEXT("Invalid question and answer values cannot change session responses"));
 Check(LunarLearning::Question(this,3).IsEmpty()&&LunarLearning::Choice(this,0,2).IsEmpty()&&LunarLearning::CorrectAnswer(this,-1)==INDEX_NONE,TEXT("Invalid content requests have no fabricated question or answer"));
 LunarLearning::RecordAnswer(this,0,LunarLearning::CorrectAnswer(this,0));
 Check(LunarLearning::Reviewed(this)==1&&LunarLearning::Correct(this)==1,TEXT("One accepted response produces one answered item and one matching latest response"));
 LunarLearning::RecordAnswer(this,0,1-LunarLearning::CorrectAnswer(this,0));
 Check(LunarLearning::Reviewed(this)==1&&LunarLearning::Correct(this)==0,TEXT("Revising one response replaces its score rather than accumulating attempts"));
 FieldLessonAnswers[1]=42;Check(LunarLearning::Reviewed(this)==1&&LunarLearning::Correct(this)==0,TEXT("Out-of-range response data is not counted as reviewed or correct"));
 ResetFieldLessons();

 const ELunarStage BeforeStage=Stage;const bool BeforePaused=Paused;
 Stage=Apollo?ELunarStage::ApolloSamples:ELunarStage::FindModule;
 if(Pilot->ActivePanel)Pilot->ClosePanel();
 Pilot->OpenEvidence();auto* Panel=Pilot->ActivePanel.Get();
 Check(Panel&&Paused,TEXT("The existing notebook pauses active play before optional lessons"));
 if(!Panel){Stage=BeforeStage;Finish();return;}
 Panel->AnswerFieldLessonA();Check(LunarLearning::Reviewed(this)==0,TEXT("An answer callback outside the lesson view cannot change responses"));
 Panel->ToggleFieldLessons();Check(Panel->ShowFieldLessons&&!Panel->ShowExpeditionMap,TEXT("Field lessons open inside the notebook and do not leave the map active"));
 const float BeforeResource=Oxygen,BeforeElapsed=ApolloElapsed,BeforeDistance=ApolloDistance;const uint8 BeforeSamples=SamplesCollected,BeforePhotos=ApolloPhotos,BeforeParts=ApolloParts;
 auto* Inactive=NewObject<ULunarPanel>();Inactive->ShowFieldLessons=true;Inactive->AnswerFieldLessonA();
 Check(LunarLearning::Reviewed(this)==0,TEXT("An inactive panel cannot answer a background lesson"));
 Panel->Settings=true;Panel->AnswerFieldLessonA();Panel->AnswerFieldLessonB();Panel->Settings=false;
 Check(LunarLearning::Reviewed(this)==0,TEXT("Settings mode rejects lesson answer callbacks"));
 Panel->FieldLessonIndex=-1;Panel->AnswerFieldLessonB();Check(LunarLearning::Reviewed(this)==0,TEXT("An invalid panel lesson index cannot record an answer"));
 auto FindAnswer=[&](int32 Answer)->UButton*{TArray<UWidget*> Widgets;Panel->WidgetTree->GetAllWidgets(Widgets);for(auto* Widget:Widgets)if(auto* Button=Cast<UButton>(Widget))if(auto* Label=Cast<UTextBlock>(Button->GetContent()))if(Label->GetText().ToString().StartsWith(Answer==0?TEXT("1  "):TEXT("2  ")))return Button;return nullptr;};
 for(int32 Index=0;Index<3;++Index){
  Panel->FieldLessonIndex=Index;Panel->Rebuild();const int32 Answer=LunarLearning::CorrectAnswer(this,Index);auto* Button=FindAnswer(Answer);
  Check(Button!=nullptr,FString::Printf(TEXT("Lesson %d builds its bound answer button"),Index));
  if(Button)Button->OnClicked.Broadcast();
  Check(FieldLessonAnswers[Index]==Answer&&LunarLearning::Reviewed(this)==Index+1&&LunarLearning::Correct(this)==Index+1,FString::Printf(TEXT("Lesson %d UMG button records the latest response without granting mission work"),Index));
  bool HasNote=false;TArray<UWidget*> Widgets;Panel->WidgetTree->GetAllWidgets(Widgets);for(auto* Widget:Widgets)if(auto* Label=Cast<UTextBlock>(Widget))HasNote=HasNote||Label->GetText().ToString()==TEXT("FIELD NOTE");
  Check(HasNote,FString::Printf(TEXT("Lesson %d immediately displays its explanatory field note"),Index));
 }
 Panel->FieldLessonIndex=2;Panel->NextFieldLesson();Check(Panel->FieldLessonIndex==0,TEXT("Next lesson wraps from the third question to the first"));
 Panel->PreviousFieldLesson();Check(Panel->FieldLessonIndex==2,TEXT("Previous lesson wraps from the first question to the third"));
 const FString Summary=LunarLearning::SessionSummary(this);
 Check(Summary.Contains(TEXT("3/3"))&&Summary.Contains(TEXT("latest answers correct 3/3"))&&Summary.Contains(TEXT("this session")),TEXT("Lesson summary reports actual latest session responses"));
 Check(ApolloDebriefLines().Last()==Summary,TEXT("Debrief appends the observed answer summary when questions were answered"));
 AdvanceTime(30);if(Apollo)TickApollo(30);
 Check(Oxygen==BeforeResource&&ApolloElapsed==BeforeElapsed&&ApolloDistance==BeforeDistance&&SamplesCollected==BeforeSamples&&ApolloPhotos==BeforePhotos&&ApolloParts==BeforeParts,TEXT("Lesson choices and navigation do not change resources, telemetry, evidence or cargo"));
 Panel->ToggleFieldLessons();const TArray<int32> Answers=FieldLessonAnswers;Panel->AnswerFieldLessonA();Panel->NextFieldLesson();
 Check(!Panel->ShowFieldLessons&&FieldLessonAnswers==Answers,TEXT("Closing lessons blocks background answers and navigation"));
 ResetFieldLessons();Check(LunarLearning::Reviewed(this)==0&&LunarLearning::Correct(this)==0&&LunarLearning::SessionSummary(this).IsEmpty()&&!FString::Join(ApolloDebriefLines(),TEXT(" ")).Contains(TEXT("Field lessons answered")),TEXT("Reset clears session-only responses and removes the optional debrief line"));
 Pilot->ClosePanel();Stage=BeforeStage;if(BeforePaused&&!Paused)Pilot->PauseMission();
 Finish();
}
