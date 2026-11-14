#include "LunarDeparture.h"
#include "LunarGame.h"
#include "LunarFieldOperations.h"
#include "LunarSurfaceVehicle.h"
#include "LunarCargoFinale.h"
#include "LunarLoading.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"

namespace {
float Ratio(float Value,float Length){return FMath::Clamp(Value/FMath::Max(.001f,Length),0.f,1.f);}
float Ease(float Value){const float T=FMath::Clamp(Value,0.f,1.f);return T*T*T*(T*(T*6-15)+10);}
}

ALunarDepartureScene::ALunarDepartureScene(){
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.bStartWithTickEnabled=false;
 auto* Root=CreateDefaultSubobject<USceneComponent>(TEXT("AuthoredDepartureCabin"));Root->SetMobility(EComponentMobility::Movable);SetRootComponent(Root);
 ShotCamera=CreateDefaultSubobject<UCameraComponent>(TEXT("DepartureCamera"));ShotCamera->SetupAttachment(Root);ShotCamera->bUsePawnControlRotation=false;
 SetActorEnableCollision(false);Tags.Add(TEXT("Lunar.DepartureScene"));
}
bool ALunarDepartureScene::Active(const ALunarGameMode* Mission){return Mission&&IsValid(Mission->DepartureScene)&&Mission->DepartureScene->IsPlaying();}
ALunarDepartureScene* ALunarDepartureScene::Begin(ALunarGameMode* Mission){
 if(!Mission||!Mission->GetWorld()||Mission->Menu||Mission->Stage!=ELunarStage::Briefing||Mission->Paused||LunarLoading::IsBusy()||ALunarCargoFinale::Active(Mission))return nullptr;
 if(Active(Mission))return Mission->DepartureScene;
 Reset(Mission);auto* Scene=Mission->GetWorld()->SpawnActor<ALunarDepartureScene>();if(!Scene)return nullptr;
 Scene->SetOwner(Mission);Mission->DepartureScene=Scene;
 if(!Scene->Initialize(Mission)){Mission->DepartureScene=nullptr;Scene->Destroy();return nullptr;}
 return Scene;
}
bool ALunarDepartureScene::Skip(ALunarGameMode* Mission){
 if(!Active(Mission))return false;
 auto* Scene=Mission->DepartureScene.Get();
 if(Mission->FieldOperations&&Mission->FieldOperations->Preparing)return true;
 if(!Mission->Paused&&!LunarLoading::IsBusy()&&Scene->Elapsed>=SkipDelay&&(!IsValid(Scene->Astronaut)||!Scene->Astronaut->ActivePanel))Scene->Finish(true);
 return true;
}
void ALunarDepartureScene::Abort(ALunarGameMode* Mission){
 if(!Mission)return;
 auto* Scene=Mission->DepartureScene.Get();Mission->DepartureScene=nullptr;
 if(IsValid(Scene)){Scene->Finish(false);Scene->Destroy();}
}
void ALunarDepartureScene::Reset(ALunarGameMode* Mission){Abort(Mission);}

bool ALunarDepartureScene::Initialize(ALunarGameMode* Mission){
 Game=Mission;Player=UGameplayStatics::GetPlayerController(Mission,0);Astronaut=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(Mission,0));
 if(!Player||!Astronaut||!Astronaut->Camera||Astronaut->ActivePanel||!Astronaut->GetCharacterMovement())return false;
 PreviousViewTarget=Player->GetViewTarget();PreviousPawnHidden=Astronaut->IsHidden();PreviousFOV=Astronaut->Camera->FieldOfView;
 Player->GetPlayerViewPoint(FinalEye,FinalAim);
 if(FinalEye.ContainsNaN()||FinalAim.ContainsNaN()||!FMath::IsFinite(PreviousFOV))return false;
 FVector Facing=FinalAim.Vector();Facing.Z=0;
 if(!Facing.Normalize())Facing=FVector::ForwardVector;
 const FRotator Heading(0,Facing.Rotation().Yaw,0);
 // Frame the cabin from the unchanged player's actual surface view.
 SetActorRotation(Heading);SetActorLocation(FinalEye-Heading.RotateVector(FVector(0,0,160)));
 if(!BuildCabin())return false;
 // Frame the cabin around the player's existing eye rather than flying a
 // camera 5.2 m forward after the equipment check. Surface handover is exact.
 TArray<USceneComponent*> CabinParts;GetComponents(CabinParts);
 for(auto* Part:CabinParts)if(Part&&Part!=GetRootComponent()&&Part!=ShotCamera)Part->AddLocalOffset(FVector(520,0,0));
 auto* Movement=Astronaut->GetCharacterMovement();PreviousMoveMode=uint8(Movement->MovementMode);PreviousCustomMode=Movement->CustomMovementMode;PreviousMovementTick=Movement->IsComponentTickEnabled();PreviousVelocity=Movement->Velocity;
 Movement->StopMovementImmediately();Movement->DisableMovement();Movement->SetComponentTickEnabled(false);
 Astronaut->SetActorHiddenInGame(true);ShotCamera->SetFieldOfView(PreviousFOV);
 // The GameMode sees Briefing before this actor can finish its frame. No
 // final cinematic delta is accidentally charged to active mission clocks.
 AddTickPrerequisiteActor(Mission);
 Playing=true;Committed=false;Elapsed=0;Pose();Player->SetViewTarget(this);SetActorTickEnabled(true);
 UE_LOG(LogTemp,Display,TEXT("DEPARTURE_SCENE begin map=%s visual_components=%d duration=%.1f authored_cabin=1 pawn_teleport=0"),*UGameplayStatics::GetCurrentLevelName(Mission,true),Visuals.Num(),Duration);
 return true;
}
UStaticMeshComponent* ALunarDepartureScene::Shape(UStaticMesh* Mesh,FVector Position,FVector Dimensions,int32 Material,FQuat Rotation){
 if(!Mesh||!Materials.IsValidIndex(Material)||!Materials[Material]||Visuals.Num()>=MaxVisualComponents||Position.ContainsNaN()||Dimensions.ContainsNaN()||Dimensions.GetMin()<=0)return nullptr;
 auto* C=NewObject<UStaticMeshComponent>(this);C->SetupAttachment(GetRootComponent());C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Mesh);
 C->SetRelativeLocation(Position);C->SetRelativeScale3D(Dimensions/100);C->SetRelativeRotation(Rotation);
 for(int32 I=0;I<FMath::Max(1,Mesh->GetStaticMaterials().Num());++I)C->SetMaterial(I,Materials[Material]);
 C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetGenerateOverlapEvents(false);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->SetReceivesDecals(false);C->PrimaryComponentTick.bCanEverTick=false;
 C->ComponentTags.Add(TEXT("Lunar.Departure.Visual"));C->RegisterComponent();Visuals.Add(C);return C;
}
UStaticMeshComponent* ALunarDepartureScene::Box(FVector Position,FVector Dimensions,int32 Material){return Shape(CubeMesh,Position,Dimensions,Material);}
UStaticMeshComponent* ALunarDepartureScene::Rod(FVector From,FVector To,float Radius,int32 Material){const FVector D=To-From;return D.SizeSquared()>.01?Shape(CylinderMesh,(From+To)*.5,FVector(Radius*2,Radius*2,D.Size()),Material,FRotationMatrix::MakeFromZ(D).ToQuat()):nullptr;}
bool ALunarDepartureScene::BuildCabin(){
 CubeMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));CylinderMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 const TCHAR* Paths[]={TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentPaint.M_EquipmentPaint"),TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentDark.M_EquipmentDark"),TEXT("/Game/Lunar/Suit/Materials/M_Metal.M_Metal"),TEXT("/Game/Lunar/Materials/M_Signal.M_Signal")};
 for(const auto* Path:Paths){auto* M=LoadObject<UMaterialInterface>(nullptr,Path);if(!M)return false;Materials.Add(M);}if(!CubeMesh||!CylinderMesh)return false;
 Box(FVector(-350,0,-6),FVector(540,360,12),1);Box(FVector(-350,0,270),FVector(540,360,12));Box(FVector(-620,0,132),FVector(12,360,276));
 for(float Side:{-1.f,1.f}){
  Box(FVector(-350,Side*177,132),FVector(540,12,276));Box(FVector(-80,Side*174,132),FVector(16,72,276));
  Box(FVector(-80,Side*144,132),FVector(22,10,264),2);Box(FVector(-350,Side*168,36),FVector(500,6,5),1);
  Rod(FVector(-585,Side*163,113),FVector(-190,Side*163,113),2.2f);
  Box(FVector(-465,Side*169,170),FVector(100,8,58),1);Box(FVector(-465,Side*163,170),FVector(78,3,35),3);
  for(int32 I=0;I<3;++I)Box(FVector(-491+I*25,Side*160,145),FVector(9,3,4),2);
  Box(FVector(-565,Side*115,40),FVector(60,55,8),1);Box(FVector(-588,Side*115,77),FVector(8,55,70),1);
  Box(FVector(-352,Side*166,245),FVector(370,4,3),3);
 }
 Box(FVector(-80,0,262),FVector(22,300,12),2);Box(FVector(-86,0,246),FVector(10,270,38),1);
 LeftDoor=Box(FVector(-80,-70,126),FVector(8,140,252),1);RightDoor=Box(FVector(-80,70,126),FVector(8,140,252),1);
 auto* Light=NewObject<UPointLightComponent>(this);Light->SetupAttachment(GetRootComponent());Light->SetMobility(EComponentMobility::Movable);Light->SetRelativeLocation(FVector(-345,0,225));Light->SetIntensity(2400);Light->SetAttenuationRadius(730);Light->SetCastShadows(false);Light->PrimaryComponentTick.bCanEverTick=false;Light->RegisterComponent();
 auto Label=[&](const FString& Text,float Z,float Size){auto* C=NewObject<UTextRenderComponent>(this);C->SetupAttachment(GetRootComponent());C->SetMobility(EComponentMobility::Movable);C->SetText(FText::FromString(Text));C->SetRelativeLocation(FVector(-94,0,Z));C->SetRelativeRotation(FRotator(0,180,0));C->SetWorldSize(Size);C->SetHorizontalAlignment(EHTA_Center);C->SetTextRenderColor(FColor(190,235,240));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCanEverAffectNavigation(false);C->SetCastShadow(false);C->PrimaryComponentTick.bCanEverTick=false;C->ComponentTags.Add(TEXT("Lunar.Departure.Label"));C->RegisterComponent();};
 Label(Game->Apollo?TEXT("APOLLO 12"):Game->Mars?TEXT("ARES / MARS"):TEXT("SELENE / MOON"),254,12);
 Label(TEXT("AUTHORED TRANSFER CABIN"),238,7);
 return LeftDoor&&RightDoor&&Visuals.Num()>0&&Visuals.Num()<=MaxVisualComponents;
}
void ALunarDepartureScene::Pose(){
 if(IsValid(Astronaut)&&Astronaut->Camera&&FMath::IsFinite(Astronaut->Camera->FieldOfView))ShotCamera->SetFieldOfView(Astronaut->Camera->FieldOfView);
 const float Door=HatchFraction();if(LeftDoor)LeftDoor->SetRelativeLocation(FVector(440,-70-145*Door,126));if(RightDoor)RightDoor->SetRelativeLocation(FVector(440,70+145*Door,126));
 ShotCamera->SetWorldLocationAndRotation(FinalEye,FinalAim);
}
void ALunarDepartureScene::Tick(float Delta){Super::Tick(Delta);AdvancePresentation(Delta);}
void ALunarDepartureScene::AdvancePresentation(float Delta){
 if(!Playing)return;
 if(!IsValid(Game)||Game->Menu||Game->Stage!=ELunarStage::Briefing||Game->DepartureScene!=this){Finish(false);return;}
 if(Game->FieldOperations&&Game->FieldOperations->Preparing)return;
 if(Game->Paused||LunarLoading::IsBusy()||(IsValid(Astronaut)&&Astronaut->ActivePanel)||!FMath::IsFinite(Delta)||Delta<=0)return;
 Elapsed=FMath::Min(Duration,Elapsed+FMath::Min(Duration,Delta));Pose();if(Elapsed>=Duration)Finish(true);
}
float ALunarDepartureScene::Progress()const{return FMath::Clamp(Elapsed/Duration,0.f,1.f);}
float ALunarDepartureScene::HatchFraction()const{return Ease(Ratio(Elapsed,.95f));}
FString ALunarDepartureScene::Title()const{return IsValid(Game)?(Game->Apollo?TEXT("APOLLO 12 / SURFACE DEPARTURE"):Game->Mars?TEXT("ARES / SURFACE DEPARTURE"):TEXT("SELENE / SURFACE DEPARTURE")):TEXT("SURFACE DEPARTURE");}
FString ALunarDepartureScene::Caption()const{
 if(Elapsed<.25f)return TEXT("Cabin systems checked. Prepare for surface operations.");
 if(Elapsed<.95f)return TEXT("Hatch opening. Keep the lander in sight.");
 return TEXT("Surface handover complete. Your mission begins now.");
}
FString ALunarDepartureScene::Guide()const{
 if(!IsValid(Game))return TEXT("Follow the objective marker on the surface.");
 if(Game->Apollo)return TEXT("Check your suit, deploy ALSEP, then collect the documented samples.");
 return Game->Mars?TEXT("Restore ARES power, recover survey data, align the relay, and return to the lander."):TEXT("Recover the rover power cell, restore SELENE, and return with the survey data.");
}
void ALunarDepartureScene::RestoreView(){
 if(IsValid(Astronaut)){
  Astronaut->SetActorHiddenInGame(PreviousPawnHidden);
  if(auto* Movement=Astronaut->GetCharacterMovement()){Movement->SetMovementMode(EMovementMode(PreviousMoveMode),PreviousCustomMode);Movement->Velocity=PreviousVelocity;Movement->SetComponentTickEnabled(PreviousMovementTick&&(!IsValid(Game)||!Game->Paused));}
 }
 if(IsValid(Player)&&Player->GetViewTarget()==this){AActor* Previous=PreviousViewTarget.Get();if(!IsValid(Previous))Previous=Astronaut;if(IsValid(Previous))Player->SetViewTarget(Previous);}
}
void ALunarDepartureScene::Finish(bool Commit){
 if(!Playing)return;Playing=false;SetActorTickEnabled(false);RestoreView();SetActorHiddenInGame(true);
 auto* Mission=Game.Get();const bool CanCommit=Commit&&!Committed&&IsValid(Mission)&&!Mission->Menu&&Mission->Stage==ELunarStage::Briefing&&!Mission->Paused&&!LunarLoading::IsBusy()&&(!IsValid(Astronaut)||!Astronaut->ActivePanel);
 UE_LOG(LogTemp,Display,TEXT("DEPARTURE_SCENE finish commit=%d time=%.3f pawn_teleport=0"),CanCommit,Elapsed);
 if(CanCommit){Committed=true;Mission->CommitMissionStart();}
}
void ALunarDepartureScene::EndPlay(const EEndPlayReason::Type Reason){Finish(false);Super::EndPlay(Reason);}
