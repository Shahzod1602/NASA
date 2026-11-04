#include "LunarCrew.h"
#include "LunarGame.h"
#include "LunarDiagnostics.h"
#include "LunarDeparture.h"
#include "LunarCargoFinale.h"
#include "LunarPanels.h"
#include "LunarSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"

namespace {
const FName CrewVisualTag(TEXT("Lunar.Crew.Visual"));
FString CrewName(int32 Index){return Index==0?TEXT("PIP-3"):Index==1?TEXT("Dr. Mira Vale"):TEXT("Unknown guide");}
FString CrewAdvice(const ALunarGameMode* Game){const auto Snapshot=LunarDiagnostics::Build(Game);return Snapshot.Available?Snapshot.NextAction:TEXT("Follow the current mission briefing. No mission step is granted by this conversation.");}
FString ScienceNote(const ALunarGameMode* Game){
 if(Game&&Game->Mars)return TEXT("ARES has authored terrain and fictional ice-survey story data. A lunar LCROSS result is not a Mars measurement. Follow the recorded power, science and relay steps.");
 return TEXT("Modern science note: LCROSS studied Cabeus near the Moon's south pole in 2009. That site and date are separate from Apollo 12; this scene does not measure water at the current play area.");
}
FVector Focus(AActor* Actor,int32 Index){return Actor->GetActorLocation()+Actor->GetActorUpVector()*(Index==0?85.f:130.f);}
struct FCrewFacts {
 ELunarStage Stage;float Oxygen,Elapsed,Distance,Hold,PowerTime;uint8 Samples,Photos,Parts,Instruments;int32 Repair,Beacon;bool Online,Recorder,Drone;FGuid Run;
 explicit FCrewFacts(const ALunarGameMode* G):Stage(G->Stage),Oxygen(G->Oxygen),Elapsed(G->ApolloElapsed),Distance(G->ApolloDistance),Hold(G->HoldProgress),PowerTime(G->PowerSequenceSeconds),Samples(G->SamplesCollected),Photos(G->ApolloPhotos),Parts(G->ApolloParts),Instruments(G->ApolloInstruments),Repair(G->RepairStep),Beacon(G->BeaconLocks),Online(G->StationRebootComplete),Recorder(G->RecorderRecovered),Drone(G->DroneRecovered),Run(G->ApolloRunId){}
 bool Same(const ALunarGameMode* G) const{return Stage==G->Stage&&Oxygen==G->Oxygen&&Elapsed==G->ApolloElapsed&&Distance==G->ApolloDistance&&Hold==G->HoldProgress&&PowerTime==G->PowerSequenceSeconds&&Samples==G->SamplesCollected&&Photos==G->ApolloPhotos&&Parts==G->ApolloParts&&Instruments==G->ApolloInstruments&&Repair==G->RepairStep&&Beacon==G->BeaconLocks&&Online==G->StationRebootComplete&&Recorder==G->RecorderRecovered&&Drone==G->DroneRecovered&&Run==G->ApolloRunId;}
};
UStaticMeshComponent* CrewTerrain(ALunarGameMode* Game){
 const FString Wanted=Game->Apollo?TEXT("SM_Apollo12Terrain"):Game->Mars?TEXT("SM_MarsTerrain"):TEXT("SM_LunarTerrain");
 for(TActorIterator<AStaticMeshActor> It(Game->GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(C&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName()==Wanted)return C;}
 return nullptr;
}
struct FCrewBuilder {
 ALunarGameMode* Game;UStaticMeshComponent* Terrain=nullptr;UStaticMesh* Cube=nullptr;UStaticMesh* Cylinder=nullptr;UStaticMesh* Sphere=nullptr;UMaterialInterface* Materials[4]={};int32 Components=0;
 explicit FCrewBuilder(ALunarGameMode* G):Game(G){
  Terrain=CrewTerrain(G);Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
  const TCHAR* Paths[]={TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentPaint.M_EquipmentPaint"),TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentDark.M_EquipmentDark"),TEXT("/Game/Lunar/Suit/Materials/M_Metal.M_Metal"),TEXT("/Game/Lunar/Materials/M_Signal.M_Signal")};
  for(int32 I=0;I<4;++I)Materials[I]=LoadObject<UMaterialInterface>(nullptr,Paths[I]);
  for(const auto& A:G->CrewActors)if(IsValid(A)){TArray<UActorComponent*> Existing;A->GetComponents(Existing);Components+=Existing.Num();}
 }
 bool Ready() const{return Terrain&&Cube&&Cylinder&&Sphere&&Materials[0]&&Materials[1]&&Materials[2]&&Materials[3];}
 bool Place(AActor* Anchor,int32 Index,FVector& Position,FQuat& Rotation){
  if(!Anchor||!Terrain)return false;const FVector Choices[]={FVector(450,Index==0?300:-260,0),FVector(-450,Index==0?300:-260,0),FVector(450,Index==0?-300:260,0),FVector(-450,Index==0?-300:260,0)};
  for(const FVector& Offset:Choices){const FVector P=Anchor->GetActorLocation()+Offset;FHitResult Hit;FCollisionQueryParams Query;Query.bTraceComplex=true;
   if(!Terrain->LineTraceComponent(Hit,P+FVector(0,0,4000),P-FVector(0,0,4000),Query)||Hit.ImpactPoint.ContainsNaN()||Hit.ImpactNormal.ContainsNaN()||Hit.ImpactNormal.Z<.8f)continue;
   Position=Hit.ImpactPoint+Hit.ImpactNormal*.2f;FVector Facing=Anchor->GetActorLocation()-Position;if(auto* Pawn=UGameplayStatics::GetPlayerPawn(Game,0))Facing=Pawn->GetActorLocation()-Position;Facing.Z=0;if(!Facing.Normalize())Facing=FVector::ForwardVector;
   Rotation=FQuat::FindBetweenNormals(FVector::UpVector,Hit.ImpactNormal)*FRotator(0,Facing.Rotation().Yaw+180,0).Quaternion();return true;
  }return false;
 }
 AActor* Begin(AActor* Anchor,int32 Index){
  const int32 Needed=Index==0?17:14;if(Components+Needed>LunarCrew::MaxComponents)return nullptr;FVector Position;FQuat Rotation;if(!Place(Anchor,Index,Position,Rotation))return nullptr;
  auto* A=Game->GetWorld()->SpawnActor<AActor>();if(!A)return nullptr;A->SetOwner(Game);A->SetActorTickEnabled(false);A->PrimaryActorTick.bCanEverTick=false;
  auto* Root=NewObject<USceneComponent>(A);Root->SetMobility(EComponentMobility::Movable);Root->PrimaryComponentTick.bCanEverTick=false;A->SetRootComponent(Root);Root->RegisterComponent();++Components;
  A->SetActorLocation(Position);A->SetActorRotation(Rotation);A->SetActorEnableCollision(false);A->Tags.Add(TEXT("Lunar.FictionalCrew"));A->Tags.Add(Index==0?TEXT("Lunar.Crew.PIP3"):TEXT("Lunar.Crew.MiraCommunicator"));return A;
 }
 void Shape(AActor* A,UStaticMesh* Mesh,FVector P,FVector Size,int32 Material,FQuat Rotation=FQuat::Identity){
  if(!A||Components>=LunarCrew::MaxComponents)return;auto* C=NewObject<UStaticMeshComponent>(A);C->SetupAttachment(A->GetRootComponent());C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Mesh);C->SetRelativeLocation(P);C->SetRelativeScale3D(Size/100);C->SetRelativeRotation(Rotation);
  for(int32 I=0;I<FMath::Max(1,Mesh->GetStaticMaterials().Num());++I)C->SetMaterial(I,Materials[Material]);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetGenerateOverlapEvents(false);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->SetReceivesDecals(false);C->PrimaryComponentTick.bCanEverTick=false;C->ComponentTags.Add(CrewVisualTag);C->RegisterComponent();++Components;
 }
 void Box(AActor* A,FVector P,FVector Size,int32 M=0){Shape(A,Cube,P,Size,M);}
 void Rod(AActor* A,FVector From,FVector To,float Radius=1.5f,int32 M=2){const FVector D=To-From;Shape(A,Cylinder,(From+To)*.5f,FVector(Radius*2,Radius*2,D.Size()),M,FRotationMatrix::MakeFromZ(D).ToQuat());}
 void Label(AActor* A,const FString& Value,float Z,float Size){
  if(!A||Components>=LunarCrew::MaxComponents)return;auto* C=NewObject<UTextRenderComponent>(A);C->SetupAttachment(A->GetRootComponent());C->SetMobility(EComponentMobility::Movable);C->SetRelativeLocation(FVector(-30,0,Z));C->SetRelativeRotation(FRotator(0,180,0));C->SetText(FText::FromString(Value));C->SetWorldSize(Size);C->SetHorizontalAlignment(EHTA_Center);C->SetTextRenderColor(FColor(210,240,245));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->PrimaryComponentTick.bCanEverTick=false;C->ComponentTags.Add(CrewVisualTag);C->RegisterComponent();++Components;
 }
 AActor* Robot(AActor* Anchor){
  auto* A=Begin(Anchor,0);if(!A)return nullptr;
  Box(A,FVector(0,0,45),FVector(45,35,50));Box(A,FVector(0,0,86),FVector(51,39,28));Box(A,FVector(-26,0,86),FVector(2,31,13),1);
  for(float Side:{-1.f,1.f}){Shape(A,Sphere,FVector(-28,Side*8,87),FVector(6),3);Shape(A,Cylinder,FVector(0,Side*25,14),FVector(28,28,12),1,FRotationMatrix::MakeFromZ(FVector::RightVector).ToQuat());Rod(A,FVector(8,Side*22,60),FVector(-18,Side*29,48),2);Shape(A,Sphere,FVector(-20,Side*29,46),FVector(10),0);}
  Rod(A,FVector(7,0,100),FVector(7,0,120),1);Shape(A,Sphere,FVector(7,0,122),FVector(5),3);Shape(A,Sphere,FVector(-21,0,6),FVector(12),2);Box(A,FVector(-24,0,51),FVector(3,24,15),1);Label(A,TEXT("PIP-3 / FICTIONAL GUIDE"),109,6);return A;
 }
 AActor* Communicator(AActor* Anchor){
  auto* A=Begin(Anchor,1);if(!A)return nullptr;
  Shape(A,Cylinder,FVector(0,0,50),FVector(12,12,100),2);Box(A,FVector(0,0,5),FVector(60,45,10),1);Box(A,FVector(0,0,130),FVector(18,72,95));Box(A,FVector(-10,0,130),FVector(2,60,75),1);
  // A simple authored remote-person icon, not a scanned or skinned human.
  Shape(A,Sphere,FVector(-12,0,144),FVector(2,18,18),0);Shape(A,Sphere,FVector(-12,0,122),FVector(2,32,25),0);
  for(int32 I=0;I<3;++I)Box(A,FVector(-12,-17+I*17,103),FVector(2,8,4),3);
  Rod(A,FVector(5,20,178),FVector(5,20,196),1);Shape(A,Sphere,FVector(5,20,198),FVector(5),3);Label(A,TEXT("DR. MIRA VALE"),184,7);Label(A,TEXT("FICTIONAL REMOTE SCIENTIST"),173,4.5f);return A;
 }
};
UTextBlock* CrewText(ULunarPanel* P,const FString& Value,int32 Size,FLinearColor Color){
 auto* T=P->WidgetTree->ConstructWidget<UTextBlock>();auto Font=T->GetFont();const float Setting=ULunarSettings::Get()->TextScale;const float Scale=FMath::IsFinite(Setting)?FMath::Clamp(Setting,.75f,1.5f):1.f;Font.Size=FMath::RoundToInt(Size*Scale);Font.TypefaceFontName=TEXT("Regular");T->SetFont(Font);T->SetColorAndOpacity(Color);T->SetText(FText::FromString(Value));T->SetAutoWrapText(true);T->SetWrapTextAt(FMath::Max(180.f,P->EvidenceContentWidth()-16));T->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);return T;
}
}

void LunarCrew::Build(ALunarGameMode* Game){
 if(!Game||!Game->GetWorld()||Game->Menu)return;Game->CrewActors.SetNum(Count);FCrewBuilder B(Game);if(!B.Ready()){UE_LOG(LogTemp,Warning,TEXT("FICTIONAL_CREW missing retained terrain or presentation asset; no substitute data/model created"));return;}
 if(!IsValid(Game->CrewActors[0])&&IsValid(Game->HomeActor))Game->CrewActors[0]=B.Robot(Game->HomeActor);
 AActor* Link=IsValid(Game->PowerActor)?Game->PowerActor.Get():Game->HomeActor.Get();if(!IsValid(Game->CrewActors[1])&&Link)Game->CrewActors[1]=B.Communicator(Link);
 UE_LOG(LogTemp,Display,TEXT("FICTIONAL_CREW robot=%d communicator=%d components=%d cap=%d"),IsValid(Game->CrewActors[0]),IsValid(Game->CrewActors[1]),B.Components,MaxComponents);
}
int32 LunarCrew::Nearby(const ALunarGameMode* Game){
 if(!Game||!Game->GetWorld()||Game->Menu||!Game->IsActive()||Game->Paused||Game->Holding||Game->ApolloTool||Game->ApolloPhotoPending||ALunarDepartureScene::Active(Game)||ALunarCargoFinale::Active(Game)||Game->CanInteract())return INDEX_NONE;
 auto* Pawn=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Game,0));if(!Pawn||Pawn->ActivePanel)return INDEX_NONE;
 FVector Eye;FRotator Aim;Pawn->GetActorEyesViewPoint(Eye,Aim);int32 Best=INDEX_NONE;float Closest=260.f;
 for(int32 I=0;I<Count&&Game->CrewActors.IsValidIndex(I);++I){auto* A=Game->CrewActors[I].Get();if(!IsValid(A)||A->IsHidden())continue;const FVector At=Focus(A,I),D=At-Eye;const float Length=float(D.Size());if(Length>Closest||Length<1||FVector::DotProduct(Aim.Vector(),D/Length)<.68f)continue;
  FCollisionQueryParams Query;Query.bTraceComplex=true;Query.AddIgnoredActor(Pawn);for(const auto& Other:Game->CrewActors)if(IsValid(Other))Query.AddIgnoredActor(Other.Get());FHitResult Hit;
  if(Game->GetWorld()->LineTraceSingleByChannel(Hit,Eye,At,ECC_Visibility,Query)&&Hit.Distance<Length-2)continue;Best=I;Closest=Length;
 }return Best;
}
FString LunarCrew::Prompt(const ALunarGameMode* Game){const int32 Index=Nearby(Game);return Index==INDEX_NONE?FString():ULunarSettings::Get()->Key(TEXT("Interact")).GetDisplayName().ToString()+TEXT("  Talk to ")+CrewName(Index);}
bool LunarCrew::Talk(ALunarGameMode* Game,int32 Index){
 const int32 Available=Nearby(Game);if(Available==INDEX_NONE||(Index!=INDEX_NONE&&Index!=Available))return false;auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Game,0));if(!Pilot)return false;
 Game->CrewSpeaker=Available;Game->CrewTalkCount=FMath::Clamp(Game->CrewTalkCount,0,MAX_int32-1)+1;
 Game->Message(Available==0?TEXT("PIP-3 (fictional guide): ")+CrewAdvice(Game):TEXT("DR. MIRA VALE (fictional remote guide): ")+ScienceNote(Game)+TEXT(" ")+CrewAdvice(Game));Pilot->OpenCrewNotebook();return Pilot->ActivePanel!=nullptr;
}
void LunarCrew::BuildNotebook(ULunarPanel* Panel){
 if(!Panel||Panel->Settings||!Panel->WidgetTree||!Panel->Body)return;auto* Game=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(Panel));if(!Game||Game->Menu)return;
 const FLinearColor Ink(.91f,.96f,1),Muted(.70f,.79f,.84f),Accent(.05f,.82f,.94f);
 auto Add=[&](const FString& Value,int32 Size,FLinearColor Color,float Space=5.f){Panel->Body->AddChildToVerticalBox(CrewText(Panel,Value,Size,Color))->SetPadding(FMargin(4,Space));};
 Add(TEXT("CREW / FICTIONAL GUIDES"),24,Accent,9);Add(TEXT("Optional conversations. These characters do not perform work, grant cargo, refill resources or diagnose real hardware."),16,Muted);
 Add(Game->CrewTalkCount>0?TEXT("LAST CONTACT / ")+CrewName(FMath::Clamp(Game->CrewSpeaker,0,1)):TEXT("No conversation recorded in this session."),17,Ink);
 Add(TEXT("PIP-3 / FIELD ROBOT"),20,Accent,9);Add(TEXT("An authored companion display near the return point. PIP-3 reads the same recorded mission checklist as your equipment notebook; it cannot complete a task on your behalf."),17,Ink);
 Add(TEXT("DR. MIRA VALE / REMOTE SCIENCE GUIDE"),20,Accent,9);Add(TEXT("A fictional present-day scientist represented by a communicator icon. She is not an Apollo 12 crew member or a NASA employee. Her modern annotations are separate from the historical mission narrative."),17,Ink);
 Add(TEXT("CURRENT MISSION GUIDANCE"),19,Accent,9);Add(CrewAdvice(Game),18,Ink);Add(ScienceNote(Game),17,Ink,9);
 if(!Game->Mars)Add(TEXT("Scientific source: NASA Science / LCROSS. The 9 October 2009 impact studied Cabeus near the lunar south pole. https://science.nasa.gov/mission/lcross/"),15,Muted);
 else Add(TEXT("ARES source boundary: authored canyon and fictional survey-data recovery. Moon/Cabeus/LCROSS notes are not Mars data. See Science & Sources for planetary source distinctions."),15,Muted);
 Add(TEXT("Read-only / Close the notebook to resume. Approach, face and use the normal work control for actual mission tasks."),16,Muted,9);
}
void LunarCrew::ValidateChecks(ALunarGameMode* Game,TFunctionRef<void(bool,const TCHAR*)> Check){
 Check(Game&&Game->GetWorld()&&!Game->Menu&&ULunarSettings::IsAutomation(),TEXT("Crew checks use an isolated playable automation world"));if(!Game||!Game->GetWorld()||Game->Menu||!ULunarSettings::IsAutomation())return;
 const FCrewFacts Before(Game);Build(Game);Check(Game->CrewActors.Num()==Count&&IsValid(Game->CrewActors[0])&&IsValid(Game->CrewActors[1]),TEXT("Both fictional guides have visible retained-asset representations"));
 const auto Actors=Game->CrewActors;Build(Game);Check(Game->CrewActors==Actors&&Before.Same(Game),TEXT("Repeated crew build preserves guide actors and all mission facts"));
 int32 Total=0,Visible=0;bool Cosmetic=true;for(const auto& A:Game->CrewActors)if(IsValid(A)){TArray<UActorComponent*> All;A->GetComponents(All);Total+=All.Num();Cosmetic&=!A->IsActorTickEnabled();TArray<UStaticMeshComponent*> Meshes;A->GetComponents(Meshes);for(auto* C:Meshes){++Visible;Cosmetic&=C->GetStaticMesh()&&C->GetMaterial(0)&&C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!C->CanEverAffectNavigation()&&!C->IsComponentTickEnabled();}}
 Check(Total<=MaxComponents&&Visible>=20,TEXT("Two crew displays stay within the 40-total-component budget"));Check(Cosmetic,TEXT("Crew models have no collision navigation actor Tick or mesh Tick"));
 const bool Paused=Game->Paused;const int32 Speaker=Game->CrewSpeaker,Talks=Game->CrewTalkCount;Game->Paused=true;Check(Nearby(Game)==INDEX_NONE&&!Talk(Game)&&Game->CrewTalkCount==Talks,TEXT("Paused input cannot talk behind a menu or mutate conversation counts"));Game->Paused=Paused;
 Check(!Talk(Game,-2)&&!Talk(Game,Count)&&Game->CrewTalkCount==Talks,TEXT("Invalid explicit speaker indices cannot start a conversation"));
 auto* PC=UGameplayStatics::GetPlayerController(Game,0);auto* Panel=PC?CreateWidget<ULunarPanel>(PC):nullptr;if(Panel){Panel->WidgetTree=NewObject<UWidgetTree>(Panel);Panel->Body=Panel->WidgetTree->ConstructWidget<UVerticalBox>();Panel->Settings=false;BuildNotebook(Panel);}
 Check(Panel&&Panel->Body&&Panel->Body->GetChildrenCount()>=9,TEXT("Crew notebook builds wrapped English biographies and current-state advice"));Check(Before.Same(Game)&&Game->CrewSpeaker==Speaker&&Game->CrewTalkCount==Talks,TEXT("Reading crew biographies grants no work resources or conversation counts"));
 Check(CrewAdvice(Game)==LunarDiagnostics::Build(Game).NextAction,TEXT("Robot guidance reads the actual shared mission checklist"));
 Check(Game->Mars?!ScienceNote(Game).Contains(TEXT("current play area"))&&ScienceNote(Game).Contains(TEXT("not a Mars measurement")):ScienceNote(Game).Contains(TEXT("2009"))&&ScienceNote(Game).Contains(TEXT("separate from Apollo 12")),TEXT("Scientist keeps modern Moon science separate from Apollo history and Mars story data"));
 // Synthetic actual-world approach/LOS/dialogue fixtures. No human session is
 // inferred; all player transforms and mission facts are restored before exit.
 auto* Pilot=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Game,0));
 const bool CanFixture=Pilot&&PC&&!Pilot->ActivePanel&&IsValid(Game->CrewActors[0])&&!ALunarDepartureScene::Active(Game)&&!ALunarCargoFinale::Active(Game)&&!Game->Holding&&!Game->ApolloTool&&!Game->ApolloPhotoPending;
 Check(CanFixture,TEXT("Crew interaction fixture has an idle player and a built robot"));if(!CanFixture)return;
 const FTransform PlayerBefore=Pilot->GetActorTransform();const FRotator AimBefore=PC->GetControlRotation();auto* Move=Pilot->GetCharacterMovement();const auto ModeBefore=Move->MovementMode;const uint8 CustomBefore=Move->CustomMovementMode;const bool TickBefore=Move->IsComponentTickEnabled();const FVector VelocityBefore=Move->Velocity;
 const ELunarStage StageBefore=Game->Stage;const FString RadioBefore=Game->Radio;const float RadioTimeBefore=Game->RadioRemaining;
 Game->Paused=false;Game->Stage=Game->Apollo?ELunarStage::ApolloCheckout:ELunarStage::FindModule;Move->StopMovementImmediately();Move->DisableMovement();Move->SetComponentTickEnabled(false);
 auto* Robot=Game->CrewActors[0].Get();FVector Out=Robot->GetActorLocation()-(Game->HomeActor?Game->HomeActor->GetActorLocation():FVector::ZeroVector);Out.Z=0;if(!Out.Normalize())Out=FVector::ForwardVector;
 auto AimRobot=[&](float Range){Pilot->SetActorLocation(Robot->GetActorLocation()+Out*Range+FVector(0,0,95),false,nullptr,ETeleportType::TeleportPhysics);FVector Eye;FRotator Aim;Pilot->GetActorEyesViewPoint(Eye,Aim);PC->SetControlRotation((Focus(Robot,0)-Eye).Rotation());};
 AimRobot(900);Check(Nearby(Game)==INDEX_NONE,TEXT("Crew prompt is unavailable outside the actual approach range"));
 AimRobot(180);Check(Nearby(Game)==0&&Prompt(Game).Contains(TEXT("PIP-3")),TEXT("Facing the nearby robot exposes its real rebound-key talk prompt"));
 PC->SetControlRotation(PC->GetControlRotation()+FRotator(0,180,0));Check(Nearby(Game)==INDEX_NONE,TEXT("Looking away hides the crew interaction prompt"));AimRobot(180);
 FVector Eye;FRotator Aim;Pilot->GetActorEyesViewPoint(Eye,Aim);auto* Occluder=Game->GetWorld()->SpawnActor<AStaticMeshActor>();
 if(Occluder){auto* C=Occluder->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));C->SetCanEverAffectNavigation(false);C->SetCollisionProfileName(TEXT("BlockAll"));C->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Occluder->SetActorLocation((Eye+Focus(Robot,0))*.5f);Occluder->SetActorRotation((Focus(Robot,0)-Eye).Rotation());Occluder->SetActorScale3D(FVector(.10f,1,1));Occluder->SetActorHiddenInGame(true);}
 Check(Occluder&&Nearby(Game)==INDEX_NONE,TEXT("An actual opaque world collider blocks talking through geometry"));if(Occluder)Occluder->Destroy();
 const FCrewFacts BeforeTalk(Game);const int32 BeforeCount=Game->CrewTalkCount;const bool Contact=Talk(Game,0);
 Check(Contact&&Pilot->ActivePanel&&Game->CrewSpeaker==0&&Game->CrewTalkCount==BeforeCount+1,TEXT("Actual robot conversation opens the crew notebook and records one local contact"));
 Check(BeforeTalk.Same(Game),TEXT("Actual conversation and notebook opening grant no cargo work or resource changes"));const int32 ContactCount=Game->CrewTalkCount;
 Check(!Talk(Game)&&Game->CrewTalkCount==ContactCount,TEXT("An open crew notebook cannot receive a second hidden world interaction"));if(Pilot->ActivePanel)Pilot->ClosePanel();
 Game->Stage=StageBefore;Game->Paused=Paused;Game->CrewSpeaker=Speaker;Game->CrewTalkCount=Talks;Game->Radio=RadioBefore;Game->RadioRemaining=RadioTimeBefore;
 Pilot->SetActorTransform(PlayerBefore,false,nullptr,ETeleportType::TeleportPhysics);PC->SetControlRotation(AimBefore);Move->SetMovementMode(ModeBefore,CustomBefore);Move->Velocity=VelocityBefore;Move->SetComponentTickEnabled(TickBefore);
 Check(Before.Same(Game)&&Pilot->GetActorTransform().Equals(PlayerBefore,.001f),TEXT("Synthetic crew fixtures restore the original player and mission facts"));
}
