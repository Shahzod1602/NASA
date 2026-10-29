#include "LunarGame.h"
#include "LunarSettings.h"
#include "ApolloData.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include <limits>

void ALunarGameMode::ApolloPresentationChecks(){
 FString Report;int32 Failures=0;auto Check=[&](bool Good,const TCHAR* Name){Report+=(Good?TEXT("PASS "):TEXT("FAIL "))+FString(Name)+TEXT("\n");if(!Good)++Failures;};
 auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0));auto* PC=UGameplayStatics::GetPlayerController(this,0);
 if(!P||!PC){FPlatformMisc::RequestExitWithStatus(false,1);return;}
 P->GetCharacterMovement()->DisableMovement();StartApollo(true);
 ApolloPhotos=5;SamplesCollected=1;ApolloParts=2;
 auto Lines=ApolloDebriefLines();
 Check(Lines[0].Contains(TEXT("2/3 sample sites documented, 1/3 samples packed and 1/3 Surveyor")),TEXT("Debrief counts documentation and packed cargo independently"));
 Check(Lines[2].Contains(TEXT("no exercise deadline"))&&!Lines[2].Contains(TEXT("remaining")),TEXT("Untimed debrief does not invent an oxygen countdown"));
 Check(Lines[1].Contains(TEXT("no coating comparison"))&&Lines[1].Contains(TEXT("not required")),TEXT("Optional study is not reported as a required or completed task"));
 ApolloDustPhotos=1;Check(ApolloDebriefLines()[1].Contains(TEXT("comparison was not completed")),TEXT("A coating photo does not claim a completed comparison"));
 ApolloDustPhotos=3;Check(ApolloDebriefLines()[1].Contains(TEXT("comparison was not completed")),TEXT("Two photographs still require an actual comparison"));
 ApolloDustCompared=true;Check(ApolloDebriefLines()[1].Contains(TEXT("comparison was recorded")),TEXT("Recorded optional comparison is reflected in debrief"));
 ApolloTimed=true;Oxygen=-2;Check(ApolloDebriefLines()[2].Contains(TEXT("00:00")),TEXT("Negative exercise balance is displayed as zero"));
 Oxygen=std::numeric_limits<float>::quiet_NaN();Check(ApolloDebriefLines()[2].Contains(TEXT("00:00")),TEXT("Non-finite exercise balance cannot corrupt the result panel"));
 Oxygen=99999;Check(ApolloDebriefLines()[2].Contains(TEXT("30:00")),TEXT("Displayed exercise balance is capped at the configured budget"));
 Oxygen=0;Stage=ELunarStage::Lost;Check(ApolloRetryAdvice().Contains(TEXT("required samples and hardware")),TEXT("Incomplete cargo produces a concrete next-attempt priority"));
 SamplesCollected=ApolloParts=7;Oxygen=100;Check(ApolloRetryAdvice().Contains(TEXT("larger exercise time margin")),TEXT("Low measured exercise balance produces a budget recommendation"));
 Stage=ELunarStage::Won;ApolloTimed=false;ApolloDustCompared=false;ApolloDustPhotos=0;
 Check(ApolloRetryAdvice().Contains(TEXT("optional exposed/shielded"))&&ApolloDebriefLines()[1].Contains(TEXT("not required")),TEXT("A successful required-only expedition remains valid without optional work"));
 ApolloDustCompared=true;Check(ApolloRetryAdvice().Contains(TEXT("timed exercise mode")),TEXT("Complete untimed work offers a concrete next challenge"));
 const FString Combined=FString::Join(ApolloDebriefLines(),TEXT(" "));
 Check(!Combined.Contains(TEXT("shorter route"))&&!Combined.Contains(TEXT("steep terrain slowed")),TEXT("Debrief never invents unrecorded route choices or slope causation"));
 Check(Combined.Contains(TEXT("LROC elevations"))&&Combined.Contains(TEXT("authored presentation")),TEXT("Measured terrain and authored visual detail are distinguished"));
 StartApollo(true);Stage=ELunarStage::ApolloSamples;ApolloPhotos=1;
 P->SetActorLocation(SampleActors[0]->GetActorLocation()+FVector(-170,0,50));FVector Eye;FRotator Aim;P->GetActorEyesViewPoint(Eye,Aim);PC->SetControlRotation((SampleActors[0]->GetActorLocation()-Eye).Rotation());
 P->Interact();P->PrimaryClick();
 Check(ApolloTool==4&&ApolloToolPhase==1&&!SamplesCollected&&ApolloToolFeedback.Contains(TEXT("Rock lifted")),TEXT("Accepted lift gives progress feedback without granting cargo"));
 const float FeedbackTime=ApolloToolFeedbackRemaining;P->PrimaryClick();
 Check(ApolloToolPhase==1&&ApolloToolFeedbackRemaining==FeedbackTime&&!SamplesCollected,TEXT("Rapid input cannot duplicate feedback, advance work or grant cargo"));
 const float Clock=ApolloElapsed,Balance=Oxygen;Paused=true;TickApollo(10);AdvanceTime(10);
 Check(ApolloToolFeedbackRemaining==FeedbackTime&&ApolloElapsed==Clock&&Oxygen==Balance,TEXT("Pause holds feedback, mission clock and exercise balance"));
 Paused=false;P->PauseMission();
 Check(!ApolloTool&&ApolloToolFeedbackRemaining==0&&!SamplesCollected,TEXT("Escape clears transient feedback without recording unfinished work"));
 P->Interact();P->PrimaryClick();TickApollo(.8f);P->PrimaryClick();
 Check((SamplesCollected&1)&&ApolloConfirmationLabel==TEXT("RECORDED")&&ApolloToolFeedbackRemaining==0,TEXT("Sealing records cargo and changes progress feedback to a recorded confirmation"));
 ConfirmApolloWork(TEXT("Saving photograph..."),false);
 Check(ApolloConfirmationLabel==TEXT("STATUS")&&(SamplesCollected&1),TEXT("Pending photo status cannot masquerade as recorded work"));
 ConfirmApolloWork(TEXT("Photo could not be saved."),false);Check(ApolloConfirmationLabel==TEXT("STATUS"),TEXT("Failed photo is shown as status rather than completed work"));
 Check(ApolloWorkStepLabel(5,1).Contains(TEXT("Tube positioned"))&&ApolloWorkStepLabel(5,5).Contains(TEXT("4 / 4"))&&ApolloWorkStepLabel(5,6).Contains(TEXT("Cap and label")),TEXT("Core feedback follows actual placement, hammer and cap steps"));
 SelectedSample=2;Check(ApolloWorkStepLabel(6,2).Contains(TEXT("Cable end 2 / 2")),TEXT("Cable feedback identifies the actual second end"));
 if(FParse::Param(FCommandLine::Get(),TEXT("PresentationAudioTest"))){
  auto* Settings=ULunarSettings::Get();const float OldMaster=Settings->MasterVolume,OldEffects=Settings->EffectsVolume;
  Settings->MasterVolume=.5f;Settings->EffectsVolume=.4f;ApolloTool=6;ApolloToolPhase=1;PulseApolloToolFeedback();
  auto* Cue=PowerSounds.IsEmpty()?nullptr:PowerSounds.Last().Get();
  Check(Cue&&FMath::IsNearlyEqual(Cue->VolumeMultiplier,.036f,.001f)&&FMath::IsNearlyEqual(Cue->PitchMultiplier,1.18f,.001f),TEXT("Actual contact audio applies master/effects gain and the configured cable pitch"));
  Settings->MasterVolume=OldMaster;Settings->EffectsVolume=OldEffects;
 }
 StartApollo(true);Check(ApolloToolFeedbackRemaining==0&&ApolloToolFeedback.IsEmpty()&&ApolloElapsed==0&&ApolloDistance==0&&!SamplesCollected&&!ApolloDustCompared,TEXT("New attempt clears feedback and all result facts"));
 int32 SurfaceBatches=0,Fragments=0;bool Cosmetic=true;
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("Apollo.SurfaceDetail"))){TArray<UHierarchicalInstancedStaticMeshComponent*> Meshes;It->GetComponents(Meshes);for(auto* C:Meshes){++SurfaceBatches;Fragments+=C->GetInstanceCount();Cosmetic&=C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&C->InstanceEndCullDistance<=4200;}}
 Check(SurfaceBatches==6&&Fragments>0&&Fragments<=9000&&Cosmetic,TEXT("Regolith detail stays in six bounded collision-free batches"));
 Check(ModuleActor&&FVector::Dist2D(ModuleActor->GetActorLocation(),ApolloData::Check)<1&&HomeActor==ModuleActor,TEXT("Presentation changes retain the authored check/return interaction anchor"));
 UStaticMeshComponent* Terrain=nullptr;AStaticMeshActor* Lander=nullptr;
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){auto* Mesh=It->GetStaticMeshComponent();if(Mesh&&Mesh->GetStaticMesh()&&Mesh->GetStaticMesh()->GetName()==TEXT("SM_Apollo12Terrain"))Terrain=Mesh;if(It->ActorHasTag(TEXT("apollo.reference-lander")))Lander=*It;}
 Check(Lander&&Lander->ActorHasTag(TEXT("Apollo.LanderDetailBuilt")),TEXT("Retained reference lander received the contact/detail pass"));
 if(Lander){auto* Mesh=Lander->GetStaticMeshComponent();const float Height=Mesh&&Mesh->GetStaticMesh()?Mesh->GetStaticMesh()->GetBounds().BoxExtent.Z*2*Lander->GetActorScale3D().Z:0;
  Check(FMath::IsNearlyEqual(Height,698.f,.1f)&&FMath::IsNearlyZero(Lander->GetActorLocation().X,.1)&&FMath::IsNearlyZero(Lander->GetActorLocation().Y,.1),TEXT("Lander support posing preserves normalized height and mission X/Y"));
  Check(!Lander->GetActorEnableCollision()||Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Lander visual posing cannot move an enabled gameplay collider"));
 }
 int32 ContactCount=0,LanderBatches=0,LanderInstances=0;bool ContactGeometry=true,LanderCosmetic=true;
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("Apollo.LanderDetail"))){TArray<UHierarchicalInstancedStaticMeshComponent*> Batches;It->GetComponents(Batches);
  for(auto* Batch:Batches){++LanderBatches;LanderInstances+=Batch->GetInstanceCount();LanderCosmetic&=Batch->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&Batch->InstanceEndCullDistance<=7000;
   if(Batch->ComponentHasTag(TEXT("Apollo.LanderContact")))for(int32 I=0;I<Batch->GetInstanceCount();++I){FTransform At;Batch->GetInstanceTransform(I,At,true);FHitResult Hit;FCollisionQueryParams Query;Query.bTraceComplex=true;
    const bool HitGround=Terrain&&Terrain->LineTraceComponent(Hit,At.GetLocation()+FVector(0,0,50),At.GetLocation()-FVector(0,0,150),Query);
    ContactGeometry&=HitGround&&!At.ContainsNaN()&&FVector::DotProduct(At.GetUnitAxis(EAxis::Z),Hit.ImpactNormal)>.995f&&FMath::Abs(FVector::DotProduct(At.GetLocation()-Hit.ImpactPoint,Hit.ImpactNormal)-.9f)<.3f;++ContactCount;
   }
  }
 }
 Check(ContactCount==4&&ContactGeometry,TEXT("Four authored contact soles align to and rest on the imported terrain plane"));
 Check(LanderBatches==6&&LanderInstances<=80&&LanderCosmetic,TEXT("Lander detail is bounded to six collision-free instanced batches"));
 const FTransform ReferencePose=Lander?Lander->GetActorTransform():FTransform::Identity;BuildApolloLanderDetail();
 Check(!Lander||Lander->GetActorTransform().Equals(ReferencePose,.001f),TEXT("Repeated detail initialization cannot accumulate lander pose offsets"));
 Report+=FString::Printf(TEXT("RESULT: %d failures\n"),Failures);FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectDir()/TEXT("Reports/presentation-checks.txt")));UE_LOG(LogTemp,Display,TEXT("APOLLO_PRESENTATION_CHECKS\n%s"),*Report);FPlatformMisc::RequestExitWithStatus(false,Failures?1:0);
}
