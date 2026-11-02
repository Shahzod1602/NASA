#include "LunarDiagnostics.h"
#include "LunarGame.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"

namespace {
using EDiagnosticState=LunarDiagnostics::EState;
int32 DiagnosticBits(uint8 Mask){int32 Count=0;for(int32 I=0;I<3;++I)if(Mask&(1<<I))++Count;return Count;}
bool DiagnosticAfterCheckout(ELunarStage Stage){return (Stage>=ELunarStage::ApolloDeploy&&Stage<=ELunarStage::ApolloReturn)||Stage==ELunarStage::Won;}
bool DiagnosticAfterDownload(ELunarStage Stage){return Stage==ELunarStage::AlignRelay||Stage==ELunarStage::SurveySamples||Stage==ELunarStage::RecoverDrone||Stage==ELunarStage::ActivateBeacon||Stage==ELunarStage::ReturnHome||Stage==ELunarStage::Won;}
bool DiagnosticAfterPickup(ELunarStage Stage){return Stage==ELunarStage::RestorePower||Stage==ELunarStage::CollectData||DiagnosticAfterDownload(Stage);}
void DiagnosticRow(LunarDiagnostics::FSnapshot& Out,const TCHAR* Key,const TCHAR* Label,EDiagnosticState State,const FString& Detail){Out.Rows.Add({FName(Key),FString(Label),State,Detail});}
LunarDiagnostics::FSnapshot RescueDiagnostics(const ALunarGameMode* Game){
 LunarDiagnostics::FSnapshot Out;Out.Available=true;
 Out.Title=FString(Game->Mars?TEXT("ARES"):TEXT("SELENE"))+TEXT(" / EQUIPMENT DIAGNOSTICS");
 const bool Closed=Game->Stage==ELunarStage::Lost||Game->Stage==ELunarStage::Won;
 const bool Started=Game->Stage!=ELunarStage::Briefing;
 const bool RepairValid=Game->RepairStep>=0&&Game->RepairStep<=2;
 const bool CellSecured=DiagnosticAfterPickup(Game->Stage)||(RepairValid&&Game->RepairStep>0)||Game->StationRebootComplete||Game->PowerSequenceActive||(Game->Stage==ELunarStage::Lost&&IsValid(Game->ModuleActor.Get())&&Game->ModuleActor->IsHidden());
 const bool Seated=RepairValid&&Game->RepairStep>=1;
 const bool Cable=RepairValid&&Game->RepairStep==2;
 const bool Online=Game->StationRebootComplete;
 const bool Downloaded=DiagnosticAfterDownload(Game->Stage);
 const bool Lost=Game->Stage==ELunarStage::Lost;
 const auto Waiting=Started?EDiagnosticState::Blocked:EDiagnosticState::Pending;
 DiagnosticRow(Out,TEXT("cell.recovered"),TEXT("1  Power cell recovered"),CellSecured?EDiagnosticState::Complete:!IsValid(Game->ModuleActor.Get())?EDiagnosticState::Unavailable:Started&&!Closed?EDiagnosticState::Ready:EDiagnosticState::Pending,
  CellSecured?TEXT("The mission has recorded the power-cell pickup."):TEXT("Recover the rover cell before working on the station port."));
 DiagnosticRow(Out,TEXT("cell.seated"),TEXT("2  Cell seated in station port"),!RepairValid?EDiagnosticState::Unavailable:Seated?EDiagnosticState::Complete:CellSecured&&!Closed?EDiagnosticState::Ready:Waiting,
  !RepairValid?TEXT("Repair-step state is outside the supported range; no seating result is inferred."):Seated?TEXT("The port installation step is recorded. A seated cell alone has not restarted the station."):TEXT("Requires the recovered cell. Use the station power-port interaction."));
 DiagnosticRow(Out,TEXT("cable.connected"),TEXT("3  Power cable connected"),!RepairValid?EDiagnosticState::Unavailable:Cable?EDiagnosticState::Complete:Seated&&!Closed?EDiagnosticState::Ready:Waiting,
  Cable?TEXT("The cable-connection step is recorded. The startup command is still a separate action."):TEXT("Requires a seated cell. Connect the cable at the station power port."));
 const auto StartupState=Online?EDiagnosticState::Complete:Game->PowerSequenceActive?(Closed?EDiagnosticState::Unavailable:EDiagnosticState::InProgress):Cable&&!Closed?EDiagnosticState::Ready:Waiting;
 DiagnosticRow(Out,TEXT("station.startup"),TEXT("4  Station startup"),StartupState,
  Online?TEXT("The station reboot completed; the science terminal has power."):Game->PowerSequenceActive?(Closed?TEXT("The restart sequence had begun when the mission ended; completion is not recorded."):TEXT("The existing startup sequence is running. It and the resource clock wait while this notebook is open.")):TEXT("Requires both the seated cell and connected cable. Hold the normal work control at the power port."));
 DiagnosticRow(Out,TEXT("science.download"),TEXT("5  Science data recovered"),Downloaded?EDiagnosticState::Complete:Lost?EDiagnosticState::Unavailable:Online&&!Closed?EDiagnosticState::Ready:Waiting,
  Downloaded?TEXT("The mission advanced beyond the terminal download."):Lost?TEXT("The ended mission does not retain a separate download flag. No completion result is inferred."):Online?TEXT("Power is available. Approach the science terminal and hold the normal work control to recover the story data."):TEXT("Blocked until the station reboot completes; installing a cell does not complete the download."));
 if(Game->Mars){
  const bool Aligned=Game->Stage==ELunarStage::ReturnHome||Game->Stage==ELunarStage::Won;
  DiagnosticRow(Out,TEXT("relay.aligned"),TEXT("6  Orbital relay aligned"),Aligned?EDiagnosticState::Complete:Lost?EDiagnosticState::Unavailable:Downloaded&&!Closed?EDiagnosticState::Ready:Waiting,
   Aligned?TEXT("Relay alignment is recorded; return to the lander."):Lost?TEXT("No separate alignment flag is retained after mission failure."):TEXT("Requires the recovered science data. Align the marked orbital antenna before returning."));
 }
 Out.Summary=TEXT("DEPENDENCIES / Cell -> port -> cable -> startup -> science terminal. Status comes from this mission's recorded steps.");
 if(Game->Stage==ELunarStage::Briefing)Out.NextAction=TEXT("NEXT / Close the notebook and begin the mission from the briefing.");
 else if(Closed)Out.NextAction=Game->Stage==ELunarStage::Won?TEXT("NEXT / Mission complete. Review the recorded steps, or start another expedition."):TEXT("NEXT / This mission has ended. Restart to try the restoration sequence again.");
 else if(!RepairValid)Out.NextAction=TEXT("NEXT / Repair state is unavailable. Restart the mission rather than treating this display as a successful repair.");
 else if(!CellSecured)Out.NextAction=IsValid(Game->ModuleActor.Get())?TEXT("NEXT / Follow the power-cell marker and recover the rover cell."):TEXT("NEXT / Power-cell actor is unavailable in this map. Restart the mission; no pickup is assumed.");
 else if(!IsValid(Game->PowerActor.Get())&&!Online)Out.NextAction=TEXT("NEXT / Station power-port actor is unavailable in this map. Restart the mission; no repair is assumed.");
 else if(!Seated)Out.NextAction=TEXT("NEXT / Take the recovered cell to the station power port and insert it.");
 else if(!Cable)Out.NextAction=TEXT("NEXT / At the same power port, connect the power cable.");
 else if(Game->PowerSequenceActive&&!Online)Out.NextAction=TEXT("NEXT / Close the notebook and let the station startup finish.");
 else if(!Online)Out.NextAction=TEXT("NEXT / Hold the work control at the power port to command the station restart.");
 else if(!Downloaded)Out.NextAction=IsValid(Game->DataActor.Get())?TEXT("NEXT / Approach the powered science terminal and hold the work control to download its data."):TEXT("NEXT / Science-terminal actor is unavailable in this map. No download is assumed.");
 else if(Game->Mars&&Game->Stage==ELunarStage::AlignRelay)Out.NextAction=TEXT("NEXT / Follow the orbital-antenna marker and hold the work control to align the relay.");
 else Out.NextAction=TEXT("NEXT / The power-and-data chain is complete. Follow the current mission objective; this notebook does not complete it for you.");
 Out.ModelNote=TEXT("MODEL LIMIT / Read-only gameplay checklist, not a voltmeter or real equipment diagnosis. READY means the preceding mission step is recorded; range, facing and a clear view are still required after closing the notebook. SELENE/ARES equipment, cables and data recovery are fictional authored content. No electrical units, NASA test results or real suit endurance are inferred.");
 return Out;
}
LunarDiagnostics::FSnapshot ApolloDiagnostics(const ALunarGameMode* Game){
 LunarDiagnostics::FSnapshot Out;Out.Available=true;Out.Title=TEXT("APOLLO 12 / FIELDWORK CHECKLIST");
 const bool Closed=Game->Stage==ELunarStage::Won||Game->Stage==ELunarStage::Lost;
 const bool Checkout=DiagnosticAfterCheckout(Game->Stage);
 const bool Leveled=(Game->ApolloInstruments&1)!=0,Linked=(Game->ApolloInstruments&2)!=0;
 const int32 Photos=DiagnosticBits(Game->ApolloPhotos),Samples=DiagnosticBits(Game->SamplesCollected),Parts=DiagnosticBits(Game->ApolloParts);
 const bool AllCargo=Game->SamplesCollected==7&&Game->ApolloParts==7&&Game->ApolloInstruments==3;
 DiagnosticRow(Out,TEXT("apollo.checkout"),TEXT("1  Intrepid checkout"),Checkout?EDiagnosticState::Complete:Game->Stage==ELunarStage::Lost?EDiagnosticState::Unavailable:Game->Stage==ELunarStage::ApolloCheckout?EDiagnosticState::Ready:EDiagnosticState::Pending,
  Checkout?TEXT("The mission advanced past its three checkout confirmations."):Game->Stage==ELunarStage::ApolloCheckout?FString::Printf(TEXT("%d / 3 confirmations recorded. Check the suit, camera/tools, then the return point."),FMath::Clamp(Game->ApolloAction,0,2)):TEXT("Checkout is separate from the fictional SELENE/ARES power repair; this chapter has no rescue power-cell task."));
 DiagnosticRow(Out,TEXT("apollo.level"),TEXT("2  Seismometer setup"),Leveled?EDiagnosticState::Complete:Game->Stage==ELunarStage::ApolloDeploy&&Game->ApolloAction==0?EDiagnosticState::Ready:EDiagnosticState::Blocked,
  Leveled?TEXT("Seismometer setup was committed."):TEXT("After checkout, approach the seismometer. Center its bubble and confirm the existing work action."));
 DiagnosticRow(Out,TEXT("apollo.link"),TEXT("3  Earth-link setup"),Linked?EDiagnosticState::Complete:Game->Stage==ELunarStage::ApolloDeploy&&Leveled?EDiagnosticState::Ready:EDiagnosticState::Blocked,
  Linked?TEXT("Earth-link setup was committed."):TEXT("The mission asks for seismometer setup first. At the central station, find the strongest authored signal and confirm."));
 DiagnosticRow(Out,TEXT("apollo.samples"),TEXT("4  Context and sample cargo"),Game->SamplesCollected==7?EDiagnosticState::Complete:Game->Stage==ELunarStage::ApolloSamples?EDiagnosticState::Ready:EDiagnosticState::Blocked,
  FString::Printf(TEXT("Context photos %d / 3; packed samples %d / 3. Each sample requires its own successfully recorded photo before collection. Tool motion alone is not packed cargo."),Photos,Samples));
 DiagnosticRow(Out,TEXT("apollo.surveyor.photo"),TEXT("5  Surveyor before-work photograph"),Game->SurveyorPhotographed?EDiagnosticState::Complete:Game->Stage==ELunarStage::ApolloRecover?EDiagnosticState::Ready:EDiagnosticState::Blocked,
  Game->SurveyorPhotographed?TEXT("The Surveyor context photograph was committed; hardware work is available at the marked fittings."):TEXT("After the sample traverse, photograph the whole Surveyor spacecraft before removing its hardware."));
 DiagnosticRow(Out,TEXT("apollo.hardware"),TEXT("6  Surveyor hardware cargo"),Game->ApolloParts==7?EDiagnosticState::Complete:Game->Stage==ELunarStage::ApolloRecover&&Game->SurveyorPhotographed?EDiagnosticState::Ready:EDiagnosticState::Blocked,
  FString::Printf(TEXT("Packed parts %d / 3. Release both marked fittings or cable ends, then pack each part. The before-work Surveyor photo is a prerequisite."),Parts));
 DiagnosticRow(Out,TEXT("apollo.return"),TEXT("7  Return and stow cargo"),Game->Stage==ELunarStage::Won?EDiagnosticState::Complete:Game->Stage==ELunarStage::ApolloReturn&&AllCargo?EDiagnosticState::Ready:EDiagnosticState::Blocked,
  Game->Stage==ELunarStage::Won?TEXT("The mission recorded cargo stowage at Intrepid."):TEXT("Requires both instrument setups, all three samples and all three hardware parts. Return to Intrepid and confirm cargo stowage."));
 Out.Summary=TEXT("DEPENDENCIES / Setup -> context before disturbance -> packed evidence -> return. This is the existing Apollo fieldwork sequence, separate from rescue power restoration.");
 if(Game->Stage==ELunarStage::Briefing)Out.NextAction=TEXT("NEXT / Close the notebook and begin the Apollo expedition from the briefing.");
 else if(Closed)Out.NextAction=Game->Stage==ELunarStage::Won?TEXT("NEXT / Cargo stowage is recorded. Review your photos and expedition results."):TEXT("NEXT / The expedition has ended. Continue the saved checkpoint or restart from the mission menu.");
 else if(Game->ApolloPhotoPending)Out.NextAction=TEXT("NEXT / Wait for the pending photograph to finish saving; no new context evidence is assumed yet.");
 else if(Game->Stage==ELunarStage::ApolloCheckout)Out.NextAction=FString(TEXT("NEXT / At the Intrepid field kit, "))+(Game->ApolloAction==0?TEXT("confirm the suit check."):Game->ApolloAction==1?TEXT("confirm the camera and tools."):TEXT("confirm Intrepid as the return point."));
 else if(Game->Stage==ELunarStage::ApolloDeploy)Out.NextAction=!Leveled?TEXT("NEXT / Approach the seismometer, center its bubble and confirm setup."):TEXT("NEXT / At the central station, turn toward the strongest signal and confirm the Earth link.");
 else if(Game->Stage==ELunarStage::ApolloSamples){
  int32 Index=FMath::Clamp(Game->SelectedSample,0,2);if(Game->SamplesCollected&(1<<Index))for(int32 I=0;I<3;++I)if(!(Game->SamplesCollected&(1<<I))){Index=I;break;}
  const TCHAR* Names[]={TEXT("12031 rock"),TEXT("12032 soil"),TEXT("12027 core")};
  Out.NextAction=FString::Printf(TEXT("NEXT / At the %s marker, %s"),Names[Index],Game->ApolloPhotos&(1<<Index)?TEXT("complete collection and seal or cap the sample to record cargo."):TEXT("take and successfully save its context photograph before collection."));
 }
 else if(Game->Stage==ELunarStage::ApolloTraverse)Out.NextAction=TEXT("NEXT / Walk to the Surveyor 3 marker. The mapped traverse is a reference, not an invisible sequence lock.");
 else if(Game->Stage==ELunarStage::ApolloRecover)Out.NextAction=Game->SurveyorPhotographed?TEXT("NEXT / At an uncollected Surveyor part, release both marked fittings or cable ends, then pack the part."):TEXT("NEXT / Step back, frame the whole Surveyor spacecraft and take its before-work photograph.");
 else if(Game->Stage==ELunarStage::ApolloReturn)Out.NextAction=AllCargo?TEXT("NEXT / Return to the Intrepid field kit and confirm cargo stowage."):TEXT("NEXT / Cargo or instrument records are incomplete; this display cannot grant the missing work.");
 else Out.NextAction=TEXT("NEXT / Follow the current mission marker. No additional action is granted by this checklist.");
 Out.ModelNote=TEXT("MODEL LIMIT / Recorded gameplay actions, not hardware health measurements. Bubble centering, signal alignment and single-player tool steps are authored simplifications. Modern LROC data controls the terrain; it does not validate the equipment model. History and terrain sources remain in Science & Sources and bundled Credits/Apollo12. Opening this optional notebook grants no work, cargo or resources.");
 return Out;
}
UTextBlock* DiagnosticText(ULunarPanel* Panel,const FString& Value,int32 Size,FLinearColor Color){
 auto* Label=Panel->WidgetTree->ConstructWidget<UTextBlock>();auto Font=Label->GetFont();Font.Size=FMath::RoundToInt(Size*ULunarSettings::Get()->TextScale);Font.TypefaceFontName=TEXT("Regular");Label->SetFont(Font);Label->SetColorAndOpacity(Color);Label->SetText(FText::FromString(Value));Label->SetAutoWrapText(true);Label->SetWrapTextAt(FMath::Max(180.f,Panel->EvidenceContentWidth()-16.f));Label->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);return Label;
}
}

LunarDiagnostics::FSnapshot LunarDiagnostics::Build(const ALunarGameMode* Game){
 if(!Game||Game->Menu)return FSnapshot();
 return Game->Apollo?ApolloDiagnostics(Game):RescueDiagnostics(Game);
}
FString LunarDiagnostics::StateLabel(EState State){
 switch(State){case EState::Ready:return TEXT("READY");case EState::Blocked:return TEXT("BLOCKED");case EState::InProgress:return TEXT("IN PROGRESS");case EState::Complete:return TEXT("RECORDED");case EState::Unavailable:return TEXT("UNAVAILABLE");default:return TEXT("PENDING");}
}
void LunarDiagnostics::BuildNotebook(ULunarPanel* Panel){
 if(!Panel||Panel->Settings||!Panel->WidgetTree||!Panel->Body)return;
 const auto* Game=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(Panel));const auto Snapshot=Build(Game);if(!Snapshot.Available)return;
 const FLinearColor TextColor(.90f,.94f,.96f),Muted(.70f,.79f,.84f),Accent(.05f,.82f,.94f),Warning(1.f,.80f,.48f);
 auto Add=[&](const FString& Value,int32 Size,FLinearColor Color,float Space=4.f){auto* Slot=Panel->Body->AddChildToVerticalBox(DiagnosticText(Panel,Value,Size,Color));Slot->SetPadding(FMargin(4,Space));};
 Add(Snapshot.Title,22,TextColor,9);Add(Snapshot.Summary,16,Muted);Add(Snapshot.NextAction,18,Accent,7);
 Add(TEXT("Read-only / Close the notebook to resume. READY does not bypass approach, facing or normal work input."),15,Muted);
 for(const auto& Row:Snapshot.Rows){
  const FLinearColor StatusColor=Row.State==EState::Complete||Row.State==EState::Ready?Accent:Row.State==EState::Blocked||Row.State==EState::Unavailable?Warning:TextColor;
  Add(FString::Printf(TEXT("%s  /  %s"),*Row.Label,*StateLabel(Row.State)),17,StatusColor,7);Add(Row.Detail,15,TextColor);
 }
 Add(Snapshot.ModelNote,15,Muted,8);
}
