#include "LunarGame.h"
#include "LunarApolloText.h"
#include "LunarSettings.h"
#include "ApolloData.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ALunarHUD::DrawApollo(ALunarGameMode* G){
 const float W=Canvas->SizeX,H=Canvas->SizeY,S=FMath::Min(W/1280.f,H/720.f);
 const float SW=W/S,SH=H/S;
 const FLinearColor Ink(.92,.95,.94),Dim(.64,.72,.74),Gold(.98,.73,.35),Cyan(.39,.87,.84);
 auto T=[](const TCHAR* Key){return ApolloText::Get(Key);};
 auto Text=[&](const FString& Value,float X,float Y,float Size,FLinearColor Color){Label(Value,X*S,Y*S,Size*S,Color);};
 auto Box=[&](float X,float Y,float BW,float BH,float Alpha=.92f){Panel(X*S,Y*S,BW*S,BH*S,Alpha);};
 auto Wrap=[&](const FString& Value,float X,float Y,float Width,float Size,FLinearColor Color,bool Draw=true){
  Size*=ULunarSettings::Get()->TextScale;
  TArray<FString> Words;Value.ParseIntoArray(Words,TEXT(" "),true);FString Line;float TY=Y;
  float Unused,FontHeight;GetTextSize(TEXT("Ag"),Unused,FontHeight,GEngine->GetLargeFont(),Size*S*2.3f);
  const float LineHeight=FMath::Max(20.f,FontHeight/S+6.f);
  for(const FString& Word:Words){
   const FString Next=Line.IsEmpty()?Word:Line+TEXT(" ")+Word;float TW,TH;
   GetTextSize(Next,TW,TH,GEngine->GetLargeFont(),Size*S*2.3f);
   if(TW>Width*S&&!Line.IsEmpty()){if(Draw)Text(Line,X,TY,Size,Color);TY+=LineHeight;Line=Word;}else Line=Next;
  }
  if(!Line.IsEmpty()){if(Draw)Text(Line,X,TY,Size,Color);TY+=LineHeight;}
  return TY;
 };
 auto Time=[](float Seconds){const int32 Sec=FMath::Max(0,FMath::CeilToInt(Seconds));return FText::FromString(FString::Printf(TEXT("%02d:%02d"),Sec/60,Sec%60));};

 Box(0,0,SW,84,.97f);DrawRect(Gold,26*S,21*S,3*S,39*S);
 Text(TEXT("APOLLO 12"),43,14,.9,Ink);Text(T(TEXT("Subtitle")),44,51,.38,Dim);
 Text(G->ApolloTimed?ApolloText::Format(TEXT("Window"),{{TEXT("Time"),Time(G->Oxygen)}}):TEXT("HISTORICAL EXPLORATION"),SW-390,19,.52,G->ApolloTimed&&G->Oxygen<180?Gold:Cyan);
 Text(G->ApolloTimed?T(TEXT("WindowNote")):TEXT("Untimed - explore at your own pace"),SW-389,50,.36,Dim);
 if(G->ApolloSaveNoticeRemaining>0&&!G->ApolloSaveNotice.IsEmpty())Text(T(*G->ApolloSaveNotice),44,67,.40,G->ApolloSaveError?Gold:Cyan);
 Box(0,SH-42,SW,42,.97f);Text(T(TEXT("Footer")),26,SH-30,.48,Dim);

 auto* P=UGameplayStatics::GetPlayerPawn(this,0);auto* PC=GetOwningPlayerController();
 if(G->ApolloTool&&!G->Paused){
  const float X=24,Y=104,ToolWidth=G->ApolloTool==1?400.f:560.f,HelpEnd=Wrap(G->ApolloToolHelp(),44,161,ToolWidth-40,.57,Ink,false);
  Box(X,Y,ToolWidth,HelpEnd-Y+18,.97f);Text(G->ApolloToolTitle(),X+20,Y+16,.52,Gold);
  Wrap(G->ApolloToolHelp(),X+20,Y+57,ToolWidth-40,.57,Ink);
  if(G->ApolloTool==1){
   const float CX=W*.5f,CY=H*.5f,FX=W*.40f,FY=H*.39f;
   DrawLine(CX-FX,CY-FY,CX+FX,CY-FY,Cyan,2);DrawLine(CX-FX,CY+FY,CX+FX,CY+FY,Cyan,2);
   DrawLine(CX-FX,CY-FY,CX-FX,CY+FY,Cyan,2);DrawLine(CX+FX,CY-FY,CX+FX,CY+FY,Cyan,2);
   DrawLine(CX-8*S,CY,CX+8*S,CY,Ink,1);DrawLine(CX,CY-8*S,CX,CY+8*S,Ink,1);
  }
  if(G->ApolloTool==2){
   const float CX=W*.5f,CY=H*.40f,Size=80*S;
   Box(SW*.5f-110,SH*.40f-110,220,220,.95f);
   DrawLine(CX-Size,CY,CX+Size,CY,Dim,2);DrawLine(CX,CY-Size,CX,CY+Size,Dim,2);
   DrawRect(Cyan,CX-8*S,CY-8*S,16*S,16*S);
   DrawRect(G->ApolloLevel.Size()<.09f?Cyan:Gold,CX+G->ApolloLevel.X*Size-5*S,CY-G->ApolloLevel.Y*Size-5*S,10*S,10*S);
  }
  if(G->ApolloTool==3){
   const float Signal=1-FMath::Clamp(FMath::Abs(FMath::FindDeltaAngleDegrees(G->ApolloAntennaAngle,-7.f))/90.f,0.f,1.f);
   Box(SW*.5f-200,SH*.36f,400,100,.97f);Text(FString::Printf(TEXT("EARTH LINK  %.0f%%"),Signal*100),SW*.5f-178,SH*.36f+15,.60,Signal>.91?Cyan:Gold);
   DrawRect(Dim,W*.5f-178*S,(SH*.36f+65)*S,356*S,12*S);DrawRect(Cyan,W*.5f-178*S,(SH*.36f+65)*S,356*S*Signal,12*S);
  }
  if(G->RadioRemaining>0){const float RadioSize=.51f*ULunarSettings::Get()->SubtitleScale/ULunarSettings::Get()->TextScale;const float RadioHeight=Wrap(G->Radio,40,0,SW-80,RadioSize,Ink,false)+40;Box(24,SH-50-RadioHeight,SW-48,RadioHeight,.97f);Text(T(TEXT("RadioLabel")),40,SH-44-RadioHeight,.32,Gold);Wrap(G->Radio,40,SH-24-RadioHeight,SW-80,RadioSize,Ink);}
  return;
 }
 if(G->IsActive()&&!G->Journal&&!G->Paused){
  // One current task. The full checklist belongs to the paused journal.
  const float Width=560,ContentWidth=Width-32;
  const float InstructionEnd=Wrap(G->ApolloInstruction(),40,152,ContentWidth,.62,Cyan,false);
  const float PurposeEnd=Wrap(G->ApolloPurpose(),40,InstructionEnd+28,ContentWidth,.51,Dim,false);
  Box(24,104,Width,PurposeEnd-104+14,.93f);
  Text(G->ApolloObjective(),40,117,.62,Ink);
  Wrap(G->ApolloInstruction(),40,152,ContentWidth,.62,Cyan);
  Text(T(TEXT("Why")),40,InstructionEnd+9,.36,Gold);
  Wrap(G->ApolloPurpose(),40,InstructionEnd+28,ContentWidth,.51,Dim);

  if(auto* Target=G->Target();Target&&P){
   const int32 Distance=FMath::RoundToInt(FVector::Dist2D(P->GetActorLocation(),Target->GetActorLocation())/100);
   const FString Marker=ApolloText::Format(TEXT("Distance"),{{TEXT("Site"),FText::FromString(G->ApolloSiteName())},{TEXT("Distance"),Distance}});
   float MarkerWidth,MarkerHeight;GetTextSize(Marker,MarkerWidth,MarkerHeight,GEngine->GetLargeFont(),.52*S*2.3f);
   FVector2D Pos;
   const bool Front=PC&&FVector::DotProduct(PC->GetControlRotation().Vector(),Target->GetActorLocation()-P->GetActorLocation())>0;
   if(Front&&PC->ProjectWorldLocationToScreen(Target->GetActorLocation()+FVector(0,0,70),Pos)){
    Pos.X=FMath::Clamp(Pos.X,30*S,W-MarkerWidth-40*S);
    Pos.Y=FMath::Clamp(Pos.Y,108*S,H-250*S);
    if(Pos.X<610*S&&Pos.Y<(PurposeEnd+35)*S)Pos.Y=(PurposeEnd+35)*S;
    DrawLine(Pos.X,Pos.Y-7*S,Pos.X+7*S,Pos.Y,Gold,2);DrawLine(Pos.X+7*S,Pos.Y,Pos.X,Pos.Y+7*S,Gold,2);
    DrawLine(Pos.X,Pos.Y+7*S,Pos.X-7*S,Pos.Y,Gold,2);DrawLine(Pos.X-7*S,Pos.Y,Pos.X,Pos.Y-7*S,Gold,2);
    Panel(Pos.X+11*S,Pos.Y-13*S,MarkerWidth+12*S,MarkerHeight+10*S,.88f);
    Label(Marker,Pos.X+17*S,Pos.Y-8*S,.52*S,Ink);
   }else{
    const float Side=PC?FVector::DotProduct(FRotationMatrix(FRotator(0,PC->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),Target->GetActorLocation()-P->GetActorLocation()):1;
    const float X=Side>0?SW-218:24;
    Box(X,SH*.49f,194,62,.90f);Text(T(Side>0?TEXT("DirectionRight"):TEXT("DirectionLeft")),X+12,SH*.49f+9,.56,Gold);
    Text(FString::Printf(TEXT("%d m"),Distance),X+12,SH*.49f+35,.46,Ink);
   }
  }
  const FString Tutorial=G->ApolloTutorial();
  if(!Tutorial.IsEmpty()){
   Box(24,SH-313,560,77,.94f);Wrap(Tutorial,40,SH-297,528,.59,Ink);
  }
  DrawLine(W*.5f-5*S,H*.5f,W*.5f+5*S,H*.5f,Ink,1);DrawLine(W*.5f,H*.5f-5*S,W*.5f,H*.5f+5*S,Ink,1);
  if(G->CanInteract()&&G->Stage!=ELunarStage::ApolloTraverse){
   const float X=SW*.5f-380;Box(X,SH-214,760,65,.95f);
   Wrap(G->ApolloPrompt(),X+18,SH-201,724,.64,Gold);
  }
  if(G->Holding&&G->HoldDuration()>0){
   const float Progress=FMath::Clamp(G->HoldProgress/G->HoldDuration(),0.f,1.f);
   DrawRect(FLinearColor(.1,.15,.17),W*.5f-380*S,H-222*S,760*S,6*S);
   DrawRect(Cyan,W*.5f-380*S,H-222*S,760*S*Progress,6*S);
   Text(ApolloText::Format(TEXT("Working"),{{TEXT("Percent"),FMath::RoundToInt(Progress*100)}}),SW*.5f-362,SH-146,.40,Dim);
  }
  if(G->RadioRemaining>0){
   const float RadioSize=.51f*ULunarSettings::Get()->SubtitleScale/ULunarSettings::Get()->TextScale;
   const float RadioHeight=Wrap(G->Radio,40,0,SW-80,RadioSize,Ink,false)+40;
   Box(24,SH-50-RadioHeight,SW-48,RadioHeight,.95f);Text(T(TEXT("RadioLabel")),40,SH-44-RadioHeight,.32,Gold);
   Wrap(G->Radio,40,SH-24-RadioHeight,SW-80,RadioSize,Ink);
  }
 }

 if(G->Journal){
  Box(0,84,SW,SH-126,.80f);const float X=SW*.5f-580,Y=SH*.5f-260;
  Box(X,Y,1160,520,.98f);Text(T(TEXT("JournalTitle")),X+24,Y+18,.72,Ink);
  Text(T(TEXT("JournalControls")),X+690,Y+24,.44,Gold);
  const float MX=X+24,MY=Y+78,MW=510,MH=413;
  if(G->ApolloMapTexture)DrawTexture(G->ApolloMapTexture,MX*S,MY*S,MW*S,MH*S,0,0,1,1,FLinearColor(.72,.78,.77),BLEND_Opaque);
  auto MapPoint=[&](FVector Pos){return FVector2D((MX+(Pos.X+60025.366)/104800.f*MW)*S,(MY+(Pos.Y+39983.14)/84800.f*MH)*S);};
  for(const auto& Segment:ApolloData::ReferenceRoute){auto A=MapPoint(Segment[0]),B=MapPoint(Segment[1]);DrawLine(A.X,A.Y,B.X,B.Y,FLinearColor(.1,.65,.70,.7),1);}
  auto Dot=[&](FVector Pos,const FString& Name,FLinearColor C){auto M=MapPoint(Pos);DrawRect(C,M.X-3*S,M.Y-3*S,6*S,6*S);Label(Name,M.X+7*S,M.Y-8*S,.31*S,C);};
  Dot(ApolloData::Home,TEXT("INTREPID"),Ink);Dot(ApolloData::Surveyor,TEXT("SURVEYOR 3"),Gold);Dot(ApolloData::Instruments[0],TEXT("ALSEP"),Cyan);
  const TCHAR* Names[]={TEXT("HEAD"),TEXT("BENCH"),TEXT("SHARP")};
  for(int I=0;I<3;++I)Dot(ApolloData::Samples[I],Names[I],G->SamplesCollected&(1<<I)?Cyan:Gold);
  if(P)Dot(P->GetActorLocation(),T(TEXT("MapYou")),FLinearColor(1,.4,.25));
  if(auto* Target=G->Target()){auto A=MapPoint(Target->GetActorLocation());DrawLine(A.X-7*S,A.Y,A.X+7*S,A.Y,Gold,2);DrawLine(A.X,A.Y-7*S,A.X,A.Y+7*S,Gold,2);}
  Box(MX,MY,MW,29,.75f);Text(T(TEXT("MapNorth")),MX+10,MY+6,.35,Ink);
  const float TX=X+557;float TY=Y+82;
  if(G->JournalPage==0){
   Text(T(TEXT("JournalPlan")),TX,TY,.61,Gold);TY+=35;
   TY=Wrap(T(TEXT("JournalPlanText")),TX,TY,572,.53,Ink)+12;
   const TCHAR* Tasks[]={TEXT("TaskCheckout"),TEXT("TaskDeploy"),TEXT("TaskSamples"),TEXT("TaskTraverse"),TEXT("TaskRecover"),TEXT("TaskReturn")};
   for(int I=0;I<6;++I){Text(FString(I<G->ApolloStep()?TEXT("[+] "):I==G->ApolloStep()?TEXT(">  "):TEXT("    "))+T(Tasks[I]),TX,TY,.53,I<G->ApolloStep()?Cyan:I==G->ApolloStep()?Ink:Dim);TY+=25;}
   TY=Wrap(T(TEXT("JournalRouteText")),TX,TY+10,572,.46,Dim);
   Text(ApolloText::Format(TEXT("JournalStats"),{{TEXT("Distance"),FMath::RoundToInt(G->ApolloDistance)},{TEXT("Count"),G->SampleCount()}}),TX,Y+474,.44,Cyan);
  }else{
   const TCHAR* Titles[]={TEXT("JournalHistory"),TEXT("JournalEvidence"),TEXT("JournalSources")};
   const TCHAR* Paragraphs[][3]={{TEXT("HistoryCrew"),TEXT("HistoryLightning"),TEXT("HistorySurveyor")},{TEXT("EvidencePhoto"),TEXT("EvidenceSamples"),TEXT("EvidenceParts")},{TEXT("SourcesTerrain"),TEXT("SourcesAccuracy"),TEXT("SourcesAdaptation")}};
   const int Page=FMath::Clamp(G->JournalPage-1,0,2);Text(T(Titles[Page]),TX,TY,.61,Gold);TY+=40;
   for(const TCHAR* Key:Paragraphs[Page])TY=Wrap(T(Key),TX,TY,572,.56,Ink)+16;
   Text(G->JournalPage==3?TEXT("NASA / GSFC / ASU / LROC science team"):T(G->JournalPage==1?TEXT("HistorySources"):TEXT("EvidenceSources")),TX,Y+447,.40,Cyan);
   if(G->JournalPage==3)Text(T(TEXT("SourcesLinks")),TX,Y+476,.38,Dim);
  }
  return;
 }

 if(G->Stage==ELunarStage::Briefing||G->Stage==ELunarStage::Won||G->Stage==ELunarStage::Lost||G->Paused){
  Box(0,84,SW,SH-126,.67f);const float X=SW*.5f-460,Y=SH*.5f-235;
  Box(X,Y,920,470,.98f);DrawRect(Gold,X*S,Y*S,920*S,3*S);
  Text(T(TEXT("OverlayKicker")),X+30,Y+24,.40,Gold);Text(T(TEXT("MissionSelect")),X+742,Y+24,.40,Dim);
  Text(T(G->ApolloConfirmNew?TEXT("CheckpointConfirmTitle"):G->Paused?TEXT("PauseTitle"):G->Stage==ELunarStage::Won?TEXT("WinTitle"):G->Stage==ELunarStage::Lost?TEXT("LostTitle"):TEXT("BriefTitle")),X+30,Y+66,1.0,Ink);
  auto ContinueLabel=[&](){return T(G->ApolloCachedCheckpoint&&G->ApolloCachedCheckpoint->Step()==6?TEXT("CheckpointResult"):TEXT("CheckpointContinue"));};
  auto Summary=[&](){int Count=0;for(const TCHAR* Id:ApolloTasks::Samples)if(G->ApolloCachedCheckpoint->Has(Id))++Count;return ApolloText::Format(TEXT("CheckpointSummary"),{{TEXT("Step"),FMath::Min(6,G->ApolloCachedCheckpoint->Step()+1)},{TEXT("Count"),Count}});};
  if(G->ApolloConfirmNew){
   Wrap(T(TEXT("CheckpointConfirmBody")),X+30,Y+150,860,.67,Ink);
   if(G->ApolloCachedCheckpoint){Text(ContinueLabel(),X+30,Y+284,.64,Cyan);Text(Summary(),X+30,Y+325,.48,Dim);}
   Text(T(TEXT("CheckpointConfirmKeys")),X+30,Y+423,.63,Gold);
  }else if(G->Stage==ELunarStage::Briefing){
   Text(G->ApolloTimed?TEXT("T  Mode: timed exercise (30 minutes)"):TEXT("T  Mode: historical exploration (no time limit)"),X+30,Y+110,.46,Cyan);
   float TY=Wrap(T(TEXT("BriefStory")),X+30,Y+132,860,.67,Ink);
   TY=Wrap(T(TEXT("BriefRoute")),X+30,TY+17,860,.61,Ink);
   TY=Wrap(T(TEXT("BriefAdaptation")),X+30,TY+17,860,.49,Dim);
   if(G->ApolloCachedCheckpoint){Text(ContinueLabel(),X+30,Y+365,.62,Cyan);Text(Summary(),X+30,Y+398,.43,Dim);}
   else Text(T(G->ApolloSaveStatus==EApolloSaveStatus::Incompatible?TEXT("CheckpointIncompatible"):G->ApolloSaveStatus==EApolloSaveStatus::Corrupt?TEXT("CheckpointCorrupt"):TEXT("BriefHelp")),X+30,Y+369,.49,Cyan);
   Text(T(TEXT("Begin"))+TEXT("     F1 Settings"),X+30,Y+423,.60,Gold);
  }else if(G->Paused){
   Wrap(T(TEXT("PauseText")),X+30,Y+149,860,.67,Ink);
   Text(T(TEXT("PauseMap")),X+30,Y+231,.62,Cyan);
   if(G->ApolloCachedCheckpoint){Text(T(TEXT("CheckpointReload")),X+30,Y+282,.58,Gold);Text(Summary(),X+30,Y+320,.43,Dim);}
   Text(T(TEXT("Resume"))+TEXT("     F1 Settings  |  I Evidence"),X+30,Y+423,.52,Gold);
  }else if(G->Stage==ELunarStage::Won){
   Wrap(T(TEXT("WinText")),X+30,Y+132,860,.65,Ink);
   Text(T(TEXT("WinCargo")),X+30,Y+231,.47,Cyan);
   Text(ApolloText::Format(TEXT("WinStats"),{{TEXT("Time"),Time(G->ApolloElapsed)},{TEXT("Distance"),FMath::RoundToInt(G->ApolloDistance)}}),X+30,Y+274,.53,Ink);
   Wrap(T(TEXT("WinContext")),X+30,Y+322,860,.48,Dim);
   Text(TEXT("I  Cargo, photographs and debrief     ")+T(TEXT("Again")),X+30,Y+423,.49,Gold);
  }else{
   Wrap(G->FailureReason,X+30,Y+149,860,.65,Ink);
   Wrap(T(TEXT("LostHelp")),X+30,Y+259,860,.55,Dim);
   if(G->ApolloCachedCheckpoint)Text(ContinueLabel(),X+30,Y+368,.61,Cyan);
   Text(T(TEXT("Restart")),X+30,Y+423,.66,Gold);
  }
 }
}
