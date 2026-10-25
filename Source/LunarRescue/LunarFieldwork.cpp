#include "LunarGame.h"
#include "LunarSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace {
UStaticMeshComponent* Detail(AActor* A,const TCHAR* Shape,FVector P,FVector Scale,const TCHAR* Surface=TEXT("M_EquipmentPaint"),FRotator R=FRotator::ZeroRotator){
 auto* C=NewObject<UStaticMeshComponent>(A);C->SetupAttachment(A->GetRootComponent());C->SetMobility(EComponentMobility::Movable);
 C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
 C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/Lunar/Apollo12/Realism/%s.%s"),Surface,Surface)));
 C->SetRelativeLocation(P);C->SetRelativeScale3D(Scale);C->SetRelativeRotation(R);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->RegisterComponent();return C;
}
void Beam(AActor* A,FVector From,FVector To,float Radius=1.5f){const FVector D=To-From;Detail(A,TEXT("Cylinder"),(From+To)*.5,FVector(Radius/50,Radius/50,D.Size()/100),TEXT("M_EquipmentPaint"),FRotationMatrix::MakeFromZ(D).Rotator());}
}

void ALunarGameMode::BuildApolloFieldwork(){
 // NASA's existing Carbajal model is retained. Normalize the authored asset to
 // the published ~6.98 m LM height, with its lowest foot at the terrain datum.
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){
  auto* C=It->GetStaticMeshComponent();UStaticMesh* Mesh=C?C->GetStaticMesh():nullptr;
  if(!Mesh||!Mesh->GetName().Contains(TEXT("ApolloLander")))continue;
  C->SetMobility(EComponentMobility::Movable);const auto B=Mesh->GetBounds();const float Scale=698.f/FMath::Max(1.f,float(B.BoxExtent.Z*2));
  It->SetActorScale3D(FVector(Scale));FVector P=It->GetActorLocation();P.Z=-(B.Origin.Z-B.BoxExtent.Z)*Scale;It->SetActorLocation(P);
  It->Tags.Add(TEXT("apollo.reference-lander"));
  // Thin blanket seams and fasteners, scaled from the normalized descent body.
  // These are authored visual detail, not a claim to reproduce every LM-6 panel.
  const float Inv=1/Scale;
  for(float Y:{-140.f,140.f})for(float X:{-120.f,0.f,120.f}){
   auto* Seam=Detail(*It,TEXT("Cube"),FVector(X,Y,180)*Inv,FVector(.025,.025,1.35)*Inv,TEXT("M_EquipmentDark"));Seam->SetCastShadow(false);
  }
 }
 // Open triangular truss, equipment shelves, harnesses and cell dividers follow
 // the visible arrangement in NASA photograph AS12-48-7136.
 for(int I=0;I<3;++I){
  const float A=I*2*PI/3,B=(I+1)*2*PI/3;
  FVector U(63*FMath::Cos(A),63*FMath::Sin(A),15),V(63*FMath::Cos(B),63*FMath::Sin(B),15);
  Beam(DroneActor,U,V,2.3f);Beam(DroneActor,U,U+FVector(0,0,53),1.6f);Beam(DroneActor,U,V+FVector(0,0,53),1.2f);
  Detail(DroneActor,TEXT("Cube"),(U+V)*.35f+FVector(0,0,19),FVector(.38,.26,.36),TEXT("M_LunarFoil"),FRotator(0,FMath::RadiansToDegrees(A),0));
 }
 Detail(DroneActor,TEXT("Cylinder"),FVector(0,0,-31),FVector(.58,.58,.12),TEXT("M_EquipmentDark"));
 const FRotator PanelRotation(0,20,22);const FTransform PanelFrame(PanelRotation,FVector(10,0,209));
 for(int I=0;I<11;++I){auto* C=Detail(DroneActor,TEXT("Cube"),PanelFrame.TransformPosition(FVector(-60+I*12,0,1.4f)),FVector(.007,.76,.003),TEXT("M_EquipmentPaint"),PanelRotation);C->SetCastShadow(false);}
 for(int I=0;I<5;++I){auto* C=Detail(DroneActor,TEXT("Cube"),PanelFrame.TransformPosition(FVector(0,-30+I*15,1.4f)),FVector(1.29,.007,.003),TEXT("M_EquipmentPaint"),PanelRotation);C->SetCastShadow(false);}
 // The paired coating panels illustrate the returned-camera analysis. They are
 // fixed on the authored inspection stand, so taking hardware cannot erase them.
 auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Lunar/Apollo12/Fieldwork/M_SurveyorCoating.M_SurveyorCoating"));
 for(int I=0;I<2;++I){
  auto* A=GetWorld()->SpawnActor<AActor>();auto* Root=NewObject<USceneComponent>(A);A->SetRootComponent(Root);Root->RegisterComponent();
  A->SetActorLocation(DroneActor->GetActorLocation()+FVector(-150,I?272.f:208.f,30));A->Tags.Add(FName(ApolloTasks::DustPhotos[I]));ApolloDustActors.Add(A);
  auto* Panel=Detail(A,TEXT("Cube"),FVector::ZeroVector,FVector(.025,.50,.42));
  if(Base){auto* Mat=UMaterialInstanceDynamic::Create(Base,this);Mat->SetVectorParameterValue(TEXT("CoatingColor"),I?FLinearColor(.38,.35,.29):FLinearColor(.64,.62,.56));DustMaterials.Add(Mat);Panel->SetMaterial(0,Mat);}
  if(I){auto* Stripe=Detail(A,TEXT("Cube"),FVector(-1.5f,0,0),FVector(.012,.08,.42),TEXT("M_EquipmentDark"));Stripe->SetCastShadow(false);Beam(A,FVector(-8,0,-95),FVector(-8,0,36),1.3f);}
 }
 auto* Stand=ApolloDustActors[0].Get();
 Beam(Stand,FVector(-8,0,-95),FVector(-8,0,-21));
 Beam(Stand,FVector(-8,0,-21),FVector(-8,64,-21));
 for(float Y:{0.f,64.f}){Beam(Stand,FVector(-8,Y,-21),FVector(0,Y,-21));Detail(Stand,TEXT("Cylinder"),FVector(-8,Y,-93),FVector(.18,.18,.03));}
 if(auto* P=Cast<ALunarCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))P->PrepareApolloWorkAssets();
}

int32 ALunarGameMode::ApolloVisualPhase() const {
 return ApolloToolCooldown>0&&(ApolloTool==4||ApolloTool==5||ApolloTool==6)?FMath::Max(0,ApolloToolPhase-1):ApolloToolPhase;
}

FVector ALunarGameMode::ApolloContactPoint() const {
 auto* A=ApolloWorkTarget.Get();if(!A)return FVector::ZeroVector;
 const FName Tag=ApolloTool==6?FName(ApolloVisualPhase()==0?TEXT("work.0"):ApolloVisualPhase()==1?TEXT("work.1"):TEXT("work.focus")):FName(TEXT("work.focus"));
 TArray<UStaticMeshComponent*> Parts;A->GetComponents(Parts);for(auto* C:Parts)if(C->ComponentHasTag(Tag))return C->Bounds.Origin;
 return A->GetActorLocation();
}

bool ALunarGameMode::CanStudyDust() const {return Apollo&&SurveyorPhotographed&&(Stage==ELunarStage::ApolloRecover||Stage==ELunarStage::ApolloReturn)&&ApolloDustActors.Num()==2;}

void ALunarCharacter::ToggleDustStudy(){if(!ActivePanel)if(auto* G=Cast<ALunarGameMode>(UGameplayStatics::GetGameMode(this)))G->ToggleDustStudy();}

void ALunarGameMode::ToggleDustStudy(){
 if(Paused||!CanStudyDust())return;
 auto* P=UGameplayStatics::GetPlayerPawn(this,0);if(!P||FVector::Dist2D(P->GetActorLocation(),DroneActor->GetActorLocation())>1000)return;
 CancelApolloTool();ApolloDustMode=!ApolloDustMode;
 if(ApolloDustMode)Message(ApolloDustCompared?TEXT("Dust study recorded. I opens your evidence and interpretation."):TEXT("Optional investigation: photograph exposed and shielded coating, then compare the evidence. K returns to the main task."));
}

bool ALunarGameMode::CaptureDustPhoto(){
 if(ApolloPhotoPending||!ApolloDustMode||!CanStudyDust()||ApolloDustCompared)return false;
 const int I=ApolloDustActors.IndexOfByPredicate([&](const auto& A){return A==ApolloWorkTarget;});
 if(I<0||ApolloDustPhotos&(1<<I))return false;
 const FString Filename=I?TEXT("Surveyor-shielded-coating"):TEXT("Surveyor-exposed-coating");FString Relative;
 if(FApp::CanEverRender()){
  Relative=TEXT("Apollo12/Photos/")+ApolloRunId.ToString(EGuidFormats::Digits)+TEXT("/")+Filename+TEXT(".png");
 }
 if(!RequestApolloPhoto(4+I,Relative))return false;
 CancelApolloTool();return true;
}

FString ALunarGameMode::ApolloDustExplanation() const {
 return TEXT("NASA's returned-camera study found a pattern consistent with dust driven by Intrepid's landing exhaust. A strut shielded some coating. These panels illustrate that comparison; their colors and geometry are authored, not measured lunar reflectance.");
}
