#include "LunarGame.h"
#include "LunarApolloText.h"
#include "LunarSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

namespace {
int Find(const TArray<TObjectPtr<AActor>>& Actors,const AActor* Actor){return Actors.IndexOfByPredicate([Actor](const auto& A){return A.Get()==Actor;});}
FString Key(const TCHAR* Action){return ULunarSettings::Get()->Key(Action).GetDisplayName().ToString();}
}

void ALunarCharacter::ToggleApolloGuide(){
 if(ActivePanel)return;
 auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this));
 if(!G||!G->Apollo||!G->IsActive())return;
 if(G->Paused||G->Journal){G->Journal=true;G->JournalPage=4;return;}
 G->ApolloGuideEnabled=!G->ApolloGuideEnabled;
 G->ApolloInputHintRemaining=0;
}

AActor* ALunarGameMode::ApolloGuidanceTarget() const {
 if(ApolloTool&&ApolloWorkTarget)return ApolloWorkTarget;
 auto* Near=ApolloTarget(true);
 return CanInteractWith(Near)?Near:ApolloTarget();
}

FString ALunarGameMode::ApolloPrompt() const {
 if(ApolloDustMode)return FString::Printf(TEXT("[ %s ]  %s"),*Key(TEXT("Interact")),ApolloDustPhotos==3?TEXT("Compare coating evidence"):TEXT("Photograph the marked coating"));
 if(!ApolloToolsEnabled){
  if(Stage==ELunarStage::ApolloCheckout)return ApolloText::Get(ApolloAction==0?TEXT("PromptCheckout0"):ApolloAction==1?TEXT("PromptCheckout1"):TEXT("PromptCheckout2"));
  if(Stage==ELunarStage::ApolloDeploy)return ApolloText::Get(ApolloAction==0?TEXT("PromptSeismometer"):TEXT("PromptAntenna"));
  if(Stage==ELunarStage::ApolloSamples)return ApolloText::Get(ApolloHoldDuration()>0?TEXT("PromptSample"):TEXT("PromptPhoto"));
  if(Stage==ELunarStage::ApolloReturn)return ApolloText::Get(TEXT("PromptReturn"));
  if(Stage==ELunarStage::ApolloRecover){if(!SurveyorPhotographed)return ApolloText::Get(TEXT("PromptSurveyorPhoto"));const int Part=Find(ApolloPartActors,ApolloTarget(true));return ApolloText::Get(Part==0?TEXT("PromptCamera"):Part==1?TEXT("PromptScoop"):TEXT("PromptCable"));}
  return ApolloText::Get(TEXT("PromptWalk"));
 }
 const AActor* Focus=ApolloGuidanceTarget();
 const int I=Find(SampleActors,Focus),Part=Find(ApolloPartActors,Focus);
 FString Action;
 if(Stage==ELunarStage::ApolloCheckout)Action=TEXT("Open equipment check");
 else if(Stage==ELunarStage::ApolloDeploy)Action=ApolloAction==0?TEXT("Open level adjustment"):TEXT("Open antenna adjustment");
 else if(Stage==ELunarStage::ApolloSamples)Action=I>=0&&!(ApolloPhotos&(1<<I))?TEXT("Take a site photo"):I==2?TEXT("Open core-tube tool"):I==1?TEXT("Collect soil"):TEXT("Collect the rock");
 else if(Stage==ELunarStage::ApolloRecover)Action=!SurveyorPhotographed?TEXT("Photograph Surveyor 3"):Part==0?TEXT("Work on the TV camera"):Part==1?TEXT("Work on the soil scoop"):TEXT("Work on the cable");
 else if(Stage==ELunarStage::ApolloReturn)Action=TEXT("Open cargo list");
 return Action.IsEmpty()?FString():FString::Printf(TEXT("[ %s ]  %s"),*Key(TEXT("Interact")),*Action);
}

FString ALunarGameMode::ApolloInstruction() const {
 if(ApolloDustMode)return ApolloDustPhotos==3?TEXT("Compare the coating photos. Explain the pattern."):TEXT("Photograph exposed and shielded coating. K: main task.");
 if(Stage==ELunarStage::ApolloCheckout)return ApolloText::Get(ApolloAction==0?TEXT("InstructionCheckout0"):ApolloAction==1?TEXT("InstructionCheckout1"):TEXT("InstructionCheckout2"));
 if(Stage==ELunarStage::ApolloDeploy)return ApolloText::Get(ApolloAction==0?TEXT("InstructionSeismometer"):TEXT("InstructionAntenna"));
 if(Stage==ELunarStage::ApolloSamples){
  const int I=Find(SampleActors,ApolloGuidanceTarget());
  const bool Photo=I>=0&&(ApolloPhotos&(1<<I));
  return !Photo?TEXT("Photograph this sample before collecting it."):I==2?TEXT("Take a soil core. Cap and label the tube."):I==1?TEXT("Scoop the soil. Seal and label its pouch."):TEXT("Lift the rock. Seal and label its pouch.");
 }
 if(Stage==ELunarStage::ApolloTraverse)return TEXT("Walk to Surveyor 3. Follow the gold marker.");
 if(Stage==ELunarStage::ApolloRecover)return !SurveyorPhotographed?TEXT("Photograph the whole spacecraft first."):TEXT("Release two fittings, then pack the marked part.");
 if(Stage==ELunarStage::ApolloReturn)return TEXT("Return to Intrepid. Stow the samples and parts.");
 return FString();
}

FString ALunarGameMode::ApolloTutorial() const {
 if(!ApolloGuideEnabled||Stage!=ELunarStage::ApolloCheckout)return FString();
 if(ApolloTool)return FString::Printf(TEXT("FIRST CHECK %d / 3   Left click to confirm. Esc cancels."),ApolloAction+1);
 if(ApolloAction>0)return FString::Printf(TEXT("FIRST CHECK %d / 3   %s"),ApolloAction+1,*ApolloPrompt());
 if(!ApolloLookLearned)return TEXT("1 / 4   Move the mouse to look around.");
 if(!ApolloMoveLearned)return FString::Printf(TEXT("2 / 4   Walk toward the gold marker: %s %s %s %s."),*Key(TEXT("MoveForward")),*Key(TEXT("MoveLeft")),*Key(TEXT("MoveBack")),*Key(TEXT("MoveRight")));
 if(!CanInteract()){
  auto* P=UGameplayStatics::GetPlayerPawn(this,0);
  const bool Close=P&&ModuleActor&&FVector::Dist(P->GetActorLocation(),ModuleActor->GetActorLocation())<=300;
  return Close?TEXT("3 / 4   Look at the marked field kit."):TEXT("3 / 4   Keep walking to the marked field kit.");
 }
 return FString::Printf(TEXT("4 / 4   Press %s to open the equipment check."),*Key(TEXT("Interact")));
}

void ALunarGameMode::RequestApolloInputHint(){
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Apollo||!ApolloGuideEnabled||!IsActive()||Paused||Journal||ApolloTool||Holding||!P||P->ActivePanel)return;
 // Repeated input does not restart the hint or interrupt mission radio.
 if(ApolloInputHintRemaining<=0)ApolloInputHintRemaining=6.f;
}

FString ALunarGameMode::ApolloInputHint() const {
 if(ApolloInputHintRemaining<=0||!Apollo||!ApolloGuideEnabled||!IsActive()||Paused||Journal||ApolloTool)return FString();
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!P||P->ActivePanel)return FString();
 if(ApolloPhotoPending)return TEXT("Photo is saving. Wait for the next action.");
 if(Stage==ELunarStage::ApolloTraverse)return TEXT("Walk to Surveyor 3. Follow the gold marker; no button is needed on the way.");
 auto* Target=ApolloGuidanceTarget();if(!IsValid(Target))return FString();
 const float Reach=InteractionReach(Target);
 if(FVector::Dist(P->GetActorLocation(),Target->GetActorLocation())>Reach)
  return FString::Printf(TEXT("Move closer to the gold marker. Work within %.0f m."),Reach/100.f);
 FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);
 if(FVector::DotProduct(Aim.Vector(),(Target->GetActorLocation()-Eye).GetSafeNormal())<.35f)
  return TEXT("Face the marked work site. Aim the center crosshair at it.");
 if(!CanInteractWith(Target))return TEXT("Something blocks your reach. Step around it and face the work site.");
 return ApolloPrompt();
}

FString ALunarGameMode::ApolloFooter() const {
 if(Journal)return Key(TEXT("Route"))+TEXT(": next page     ")+Key(TEXT("Journal"))+TEXT(" / Esc: close     H: how to play     F1: settings");
 if(Paused)return TEXT("Esc: resume     H: how to play     F1: settings");
 if(ApolloTool==9)return Key(TEXT("Route"))+TEXT(": choose explanation     Left click: confirm     Esc: cancel");
 if(ApolloTool)return TEXT("Left click: work     Esc: cancel unfinished work     H: guide     F1: settings");
 if(!IsActive())return TEXT("F1 Settings     H Guide during play     Esc Pause");
 FString Result=Key(TEXT("Journal"))+TEXT(": map / help     H: ")+(ApolloGuideEnabled?TEXT("hide guide"):TEXT("show guide"));
 if(ApolloEvidence.Num()>0||SamplesCollected||ApolloParts)Result+=TEXT("     I: photos / cargo");
 if(Stage==ELunarStage::ApolloSamples||(Stage==ELunarStage::ApolloRecover&&SurveyorPhotographed))Result+=TEXT("     ")+Key(TEXT("Route"))+TEXT(": next site");
 if(CanStudyDust())if(auto* Player=UGameplayStatics::GetPlayerPawn(this,0);Player&&FVector::Dist2D(Player->GetActorLocation(),DroneActor->GetActorLocation())<=1000)Result+=TEXT("     K: dust study");
 return Result+TEXT("     Esc: pause     F1: settings");
}

FString ALunarGameMode::ApolloToolProgress() const {
 int Step=1,Count=1;
 if(ApolloTool==8){Step=ApolloAction+1;Count=3;}
 else if(ApolloTool==4){Step=ApolloToolPhase+1;Count=2;}
 else if(ApolloTool==5){Step=ApolloToolPhase+1;Count=7;}
 else if(ApolloTool==6){Step=ApolloToolPhase+1;Count=3;}
 return FString::Printf(TEXT("STEP %d / %d"),Step,Count);
}

void ALunarGameMode::ConfirmApolloWork(const FString& Value){ApolloConfirmation=Value;ApolloConfirmationRemaining=5.f;}

bool ALunarGameMode::ApolloWorkCue(FVector& Position,FString& Label) const {
 if(ApolloTool==4&&ApolloToolPhase>0)return false;
 if(Paused||Journal||!IsActive()||ApolloTool==1||ApolloTool==2||ApolloTool==3||ApolloTool==7||ApolloTool==8)return false;
 AActor* A=ApolloGuidanceTarget();if(!IsValid(A)||A->IsHidden()||!CanInteractWith(A))return false;
 const int Sample=Find(SampleActors,A),Part=Find(ApolloPartActors,A);
 if(Sample==INDEX_NONE&&Part==INDEX_NONE)return false;
 Position=A->GetActorLocation();
 const FName Tag=Part>=0&&ApolloTool==6&&ApolloVisualPhase()<2?FName(ApolloVisualPhase()==0?TEXT("work.0"):TEXT("work.1")):FName(TEXT("work.focus"));
 TArray<UStaticMeshComponent*> Meshes;A->GetComponents(Meshes);
 for(auto* C:Meshes)if(C->ComponentHasTag(Tag)){Position=C->Bounds.Origin;break;}
 if(Part>=0)Label=ApolloTool==6?(ApolloVisualPhase()==0?(Part==2?TEXT("FIRST CABLE END"):TEXT("FIRST FITTING")):ApolloVisualPhase()==1?(Part==2?TEXT("SECOND CABLE END"):TEXT("SECOND FITTING")):TEXT("PACK THIS PART")):TEXT("WORK ON THIS PART");
 else Label=Sample==0?TEXT("ROCK"):Sample==1?TEXT("SOIL"):TEXT("CORE SITE");
 return true;
}
