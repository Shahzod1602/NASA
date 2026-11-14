#include "LunarEquipmentDetail.h"
#include "LunarGame.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include <limits>

namespace {
const FName DetailTag(TEXT("Lunar.EquipmentDetail")),BuiltTag(TEXT("Lunar.EquipmentDetailBuilt"));
enum ESurface {Paint,Dark,Foil,Metal,Rock};
struct FFrame {
 AActor* Owner=nullptr;USceneComponent* Parent=nullptr;FQuat Rotation=FQuat::Identity;FVector Origin=FVector::ZeroVector;
 FVector Point(FVector Local) const {return Origin+Rotation.RotateVector(Local);}
};
FFrame Local(AActor* A,USceneComponent* Parent=nullptr){FFrame F;F.Owner=A;F.Parent=Parent?Parent:A?A->GetRootComponent():nullptr;if(F.Parent){F.Origin=F.Parent->GetComponentLocation();F.Rotation=F.Parent->GetComponentQuat();}return F;}
struct FAssetFrame {FFrame Frame;FVector Size=FVector::ZeroVector;bool Valid=false;};
FAssetFrame AssetFrame(AActor* A){
 FAssetFrame F;if(!A)return F;auto* C=A->FindComponentByClass<UStaticMeshComponent>();if(!C||!C->GetStaticMesh())return F;
 const auto B=C->GetStaticMesh()->GetBounds();F.Frame=Local(A,C);F.Frame.Origin=C->GetComponentTransform().TransformPosition(B.Origin-FVector(0,0,B.BoxExtent.Z));F.Size=B.BoxExtent*2*C->GetComponentScale().GetAbs();F.Valid=!F.Size.ContainsNaN()&&F.Size.GetMin()>UE_SMALL_NUMBER;return F;
}
// Coordinates below come from the retained converted OBJ meshes, not NASA measurements.
FVector SourcePoint(const FAssetFrame& Asset,FVector Point){
 auto* C=Cast<UStaticMeshComponent>(Asset.Frame.Parent);if(!C||!C->GetStaticMesh())return FVector::ZeroVector;
 return Asset.Frame.Rotation.UnrotateVector(C->GetComponentTransform().TransformPosition(Point)-Asset.Frame.Origin);
}
float SourceScale(const FAssetFrame& Asset){return float(Asset.Frame.Parent->GetComponentScale().GetAbs().GetMin());}
struct FBuilder {
 ALunarGameMode* Game;UStaticMeshComponent* Terrain=nullptr;TMap<FString,UInstancedStaticMeshComponent*> Batches;TMap<FString,UStaticMesh*> Meshes;UMaterialInterface* Materials[5]={};
 int32 Instances=0,Missing=0,Skipped=0;
 explicit FBuilder(ALunarGameMode* G):Game(G){
  const TCHAR* Paths[]={TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentPaint.M_EquipmentPaint"),TEXT("/Game/Lunar/Apollo12/Realism/M_EquipmentDark.M_EquipmentDark"),TEXT("/Game/Lunar/Apollo12/Realism/M_LunarFoil.M_LunarFoil"),TEXT("/Game/Lunar/Suit/Materials/M_Metal.M_Metal"),TEXT("/Game/Lunar/Apollo12/Realism/M_RegolithRock.M_RegolithRock")};
  for(int32 I=0;I<5;++I)Materials[I]=LoadObject<UMaterialInterface>(nullptr,Paths[I]);
  for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(C&&C->GetStaticMesh()){const FString N=C->GetStaticMesh()->GetName();if(N==(G->Apollo?TEXT("SM_Apollo12Terrain"):G->Mars?TEXT("SM_MarsTerrain"):TEXT("SM_LunarTerrain"))){Terrain=C;break;}}}
 }
 UStaticMesh* Mesh(const FString& Path){if(auto** M=Meshes.Find(Path))return *M;auto* M=LoadObject<UStaticMesh>(nullptr,*Path);Meshes.Add(Path,M);if(!M)++Missing;return M;}
 UInstancedStaticMeshComponent* Batch(const FFrame& F,const FString& Path,ESurface Surface){
  if(!IsValid(F.Owner)||!IsValid(F.Parent)||!Materials[Surface]){++Missing;return nullptr;}auto* M=Mesh(Path);if(!M)return nullptr;
  const FString Key=FString::Printf(TEXT("%u:%u:%s:%d"),F.Owner->GetUniqueID(),F.Parent->GetUniqueID(),*Path,int32(Surface));if(auto** B=Batches.Find(Key))return *B;
  if(Batches.Num()>=LunarEquipmentDetail::MaxComponents){++Skipped;return nullptr;}
  auto* B=NewObject<UInstancedStaticMeshComponent>(F.Owner);F.Owner->AddInstanceComponent(B);B->SetupAttachment(F.Parent);B->SetMobility(F.Parent->GetMobility());B->SetStaticMesh(M);for(int32 Slot=0;Slot<FMath::Max(1,M->GetStaticMaterials().Num());++Slot)B->SetMaterial(Slot,Materials[Surface]);
  B->SetCollisionEnabled(ECollisionEnabled::NoCollision);B->SetGenerateOverlapEvents(false);B->SetCanEverAffectNavigation(false);B->SetCastShadow(false);B->SetCullDistances(Surface==Rock?1800:3500,Surface==Rock?2800:6500);B->PrimaryComponentTick.bCanEverTick=false;B->ComponentTags.Add(DetailTag);B->RegisterComponent();Batches.Add(Key,B);return B;
 }
 void Add(const FFrame& F,const FString& Path,FVector P,FVector Dimensions,ESurface Surface=Paint,FQuat Rotation=FQuat::Identity){
  if(Instances>=LunarEquipmentDetail::MaxInstances||P.ContainsNaN()||Dimensions.ContainsNaN()||Dimensions.GetMin()<=0){++Skipped;return;}
  if(auto* B=Batch(F,Path,Surface)){B->AddInstance(FTransform(F.Rotation*Rotation,F.Point(P),Dimensions/100.f),true);++Instances;}
 }
 void Shape(const FFrame& F,const TCHAR* Name,FVector P,FVector Dimensions,ESurface Surface=Paint,FQuat Rotation=FQuat::Identity){Add(F,FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Name,Name),P,Dimensions,Surface,Rotation);}
 void Box(const FFrame& F,FVector P,FVector Dimensions,ESurface Surface=Paint){Shape(F,TEXT("Cube"),P,Dimensions,Surface);}
 void Rod(const FFrame& F,FVector A,FVector B,float Radius=1.2f,ESurface Surface=Metal){const FVector D=B-A;if(D.SizeSquared()<.01f)return;Shape(F,TEXT("Cylinder"),(A+B)*.5f,FVector(Radius*2,Radius*2,D.Size()),Surface,FRotationMatrix::MakeFromZ(D).ToQuat());}
 void Ring(const FFrame& F,FVector P,float Radius,float Thickness=1.1f,ESurface Surface=Metal,FVector Normal=FVector::UpVector){const FQuat Align=FRotationMatrix::MakeFromZ(Normal).ToQuat();for(int32 I=0;I<12;++I){const float A=I*2*PI/12,B=(I+1)*2*PI/12;Rod(F,P+Align.RotateVector(FVector(FMath::Cos(A)*Radius,FMath::Sin(A)*Radius,0)),P+Align.RotateVector(FVector(FMath::Cos(B)*Radius,FMath::Sin(B)*Radius,0)),Thickness,Surface);}}
 void Cable(const FFrame& F,FVector A,FVector B,FVector Sag,float Radius=.7f){for(int32 I=0;I<8;++I){auto Point=[&](float T){return FMath::Lerp(A,B,T)+Sag*4*T*(1-T);};Rod(F,Point(I/8.f),Point((I+1)/8.f),Radius,Dark);}}
 bool Ground(FVector P,FHitResult& Hit) const {if(!Terrain)return false;FCollisionQueryParams Q;Q.bTraceComplex=true;return Terrain->LineTraceComponent(Hit,P+FVector(0,0,4000),P-FVector(0,0,4000),Q)&&!Hit.ImpactPoint.ContainsNaN()&&!Hit.ImpactNormal.ContainsNaN()&&Hit.ImpactNormal.Z>.7f;}
 void Bolts(const FFrame& F,FVector Center,FVector U,FVector V){for(float X:{-1.f,1.f})for(float Y:{-1.f,1.f})Shape(F,TEXT("Cylinder"),Center+U*X+V*Y,FVector(1.7,1.7,1),Metal);}
};

void Surveyor(FBuilder& B){
 auto* G=B.Game;if(!G->DroneActor)return;const auto F=Local(G->DroneActor);
 // Authored boxes, suspension collars and harnesses add definition without
 // replacing the existing camera framing target or any work.* marker.
 for(int32 I=0;I<3;++I){const float T=I*2*PI/3;const FVector Foot(185*FMath::Cos(T),185*FMath::Sin(T),-55),Upper(60*FMath::Cos(T),60*FMath::Sin(T),27);
  const FVector Shock=FMath::Lerp(Upper,Foot,.67f);B.Rod(F,FMath::Lerp(Upper,Foot,.55f),FMath::Lerp(Upper,Foot,.78f),4.1f,Dark);B.Ring(F,Shock,4.2f,.55f,Metal,Foot-Upper);
  B.Cable(F,Upper+FVector(0,0,10),Foot+FVector(0,0,4),FVector(0,0,-12));
  const FVector Shelf(45*FMath::Cos(T),45*FMath::Sin(T),37);B.Box(F,Shelf,FVector(27,20,22),Foil);B.Bolts(F,Shelf+FVector(0,0,11.5f),FVector(10,0,0),FVector(0,7,0));
  for(int32 J=0;J<4;++J)B.Box(F,Shelf+FVector(-8+J*5,0,12),FVector(1.2f,15,1.5f),Dark);
 }
 B.Cable(F,FVector(-20,15,48),FVector(-30,23,96),FVector(-10,5,0));
 B.Cable(F,FVector(22,-30,25),FVector(-110,-65,-27),FVector(0,-12,-12));
 for(int32 I=0;I<G->ApolloPartActors.Num()&&I<3;++I){auto* Part=G->ApolloPartActors[I].Get();if(!Part)continue;const auto P=Local(Part);
  if(I==0){
   B.Shape(P,TEXT("Cylinder"),FVector(0,0,42.8f),FVector(26,26,2),Metal);B.Shape(P,TEXT("Cylinder"),FVector(0,0,-43),FVector(24,24,2),Dark);
   B.Ring(P,FVector(-15,0,24),6.3f,.8f,Metal,FVector::ForwardVector);B.Shape(P,TEXT("Cylinder"),FVector(-15.2f,0,24),FVector(10,10,2),Dark,FRotationMatrix::MakeFromZ(FVector::ForwardVector).ToQuat());
   B.Box(P,FVector(11,0,1),FVector(4,12,27),Foil);for(float Z:{-32.f,32.f})B.Ring(P,FVector(-10,0,Z),4.4f,.6f,Metal);
   B.Rod(P,FVector(13,-8,-22),FVector(13,8,-22),1.5f);B.Cable(P,FVector(11,4,-30),FVector(11,5,-40),FVector(5,4,0),.6f);
  }else if(I==1){
   for(float Y:{-10.f,10.f})B.Box(P,FVector(0,Y,7),FVector(29,1.2f,7),Metal);B.Box(P,FVector(13,0,7),FVector(1.2f,19,7),Metal);
   for(int32 J=0;J<5;++J)B.Box(P,FVector(-14,-8+J*4,1.1f),FVector(4,2.1f,2),Metal);
   B.Rod(P,FVector(8,0,5),FVector(25,0,15),1.5f);for(float X:{-12.f,12.f})B.Ring(P,FVector(X,0,3),3.9f,.6f,Dark);
  }else{
   B.Cable(P,FVector(0,4.5f,-18),FVector(0,4.5f,18),FVector(8,1,0),1.1f);
   for(float Z:{-18.f,18.f}){B.Ring(P,FVector(0,4.5f,Z),3.7f,.65f,Metal);B.Shape(P,TEXT("Cylinder"),FVector(0,4.5f,Z),FVector(5,5,4),Dark);}
   for(float Z:{-11.f,0.f,11.f})B.Box(P,FVector(4,4.5f,Z),FVector(2,14,2),Paint);
  }
 }
 for(int32 I=0;I<G->ApolloInstrumentActors.Num()&&I<2;++I){auto* A=G->ApolloInstrumentActors[I].Get();if(!A)continue;const auto P=Local(A);
  if(I==0){B.Ring(P,FVector(0,0,-17.7f),24,1.2f,Metal);B.Box(P,FVector(-13,0,-17),FVector(12,19,1.7f),Dark);
   for(int32 J=0;J<7;++J)B.Box(P,FVector(-18+J*2,0,-15.8f),FVector(.55f,14,1),Paint);
   for(float X:{-17.f,17.f})B.Shape(P,TEXT("Cylinder"),FVector(X,18,-18),FVector(5,5,6),Metal);
   B.Cable(P,FVector(22,0,-33),FVector(42,18,-62),FVector(18,0,-8));
  }else{
   B.Box(P,FVector(-40.7f,0,-30),FVector(2,51,29),Dark);for(int32 J=0;J<7;++J)B.Box(P,FVector(-42,-20+J*7,-28),FVector(1.5f,2.5f,22),Metal);
   for(float Y:{-24.f,24.f}){B.Ring(P,FVector(-42,Y,-35),4,.7f,Metal,FVector::ForwardVector);B.Cable(P,FVector(-43,Y,-35),FVector(-65,Y,-62),FVector(-8,0,-7));}
   B.Bolts(P,FVector(0,0,-4.5f),FVector(35,0,0),FVector(0,27,0));
   if(G->ApolloAntenna){const auto Dish=Local(A,G->ApolloAntenna);B.Rod(Dish,FVector(-18,0,-10),FVector(18,0,-10),1.1f);B.Rod(Dish,FVector(0,-16,0),FVector(0,16,0),1.1f);for(float Y:{-12.f,12.f})B.Rod(Dish,FVector(-19,Y,0),FVector(19,Y,0),.8f);B.Ring(Dish,FVector(0,0,-9),5,.9f,Dark);}
  }
 }
 // Reuse existing close-up tool meshes in a separate retained storage rack.
 // The player's articulated tool meshes and exact grip transforms are untouched.
 if(G->ModuleActor){const auto K=Local(G->ModuleActor);const TCHAR* Names[]={TEXT("SM_SampleTongs"),TEXT("SM_SoilScoop"),TEXT("SM_DriveTube"),TEXT("SM_GeologyHammer"),TEXT("SM_ContextCamera"),TEXT("SM_SamplePouch")};
  B.Box(K,FVector(0,0,23),FVector(72,51,2),Dark);
  for(int32 I=0;I<6;++I){const FVector At(-25+(I%3)*24,-17+(I/3)*30,26);const FString Path=FString::Printf(TEXT("/Game/Lunar/Apollo12/Tools/%s.%s"),Names[I],Names[I]);B.Add(K,Path,At,FVector(65),I==5?Paint:Metal,FRotator(0,I<3?90.f:0.f,0).Quaternion());for(float Y:{-5.f,5.f})B.Box(K,At+FVector(0,Y,-1),FVector(8,2,3),Foil);}
 }
}

void Lander(FBuilder& B,AActor* A){
 const auto Asset=AssetFrame(A);if(!Asset.Valid)return;auto* C=Cast<UStaticMeshComponent>(Asset.Frame.Parent);if(!C||C->GetStaticMesh()->GetName()!=TEXT("ApolloLander"))return;const auto F=Asset.Frame;const float Scale=SourceScale(Asset);
 // The blanket's retained side faces lie at source x +/-152.36 cm. Bounding
 // extrema include landing gear and cannot be used as equipment mounting faces.
 for(float Side:{-1.f,1.f})for(float Y:{-10.f,10.f}){
  const FVector Low(Side*152.36f,Y,128),High(Side*152.36f,Y,167),Out(Side*8,0,0);
  B.Rod(F,SourcePoint(Asset,Low),SourcePoint(Asset,Low+Out),.9f*Scale,Metal);B.Rod(F,SourcePoint(Asset,High),SourcePoint(Asset,High+Out),.9f*Scale,Metal);B.Rod(F,SourcePoint(Asset,Low+Out),SourcePoint(Asset,High+Out),1.1f*Scale,Dark);
 }
}
void Battery(FBuilder& B,AActor* A){
 const auto Asset=AssetFrame(A);if(!Asset.Valid)return;const auto F=Asset.Frame;const FVector S=Asset.Size;
 for(float Y:{-S.Y*.22f,S.Y*.22f})B.Rod(F,FVector(0,Y,S.Z),FVector(0,Y,S.Z+7),1.4f);B.Rod(F,FVector(0,-S.Y*.22f,S.Z+7),FVector(0,S.Y*.22f,S.Z+7),1.4f,Dark);
 for(float Y:{-S.Y*.18f,S.Y*.18f}){B.Ring(F,FVector(-S.X*.5f-1,Y,S.Z*.55f),3.6f,.7f,Metal,FVector::ForwardVector);B.Shape(F,TEXT("Cylinder"),FVector(-S.X*.5f-2,Y,S.Z*.55f),FVector(3.5f,3.5f,4),Dark,FRotationMatrix::MakeFromZ(FVector::ForwardVector).ToQuat());}
 for(float Y:{-S.Y*.35f,S.Y*.35f})B.Box(F,FVector(0,Y,3),FVector(S.X+3,2.2f,3.5f),Foil);
}
void Port(FBuilder& B,AActor* A){if(!A)return;const auto F=Local(A);
 // Support the existing panel at its saved mission anchor. Moving that anchor
 // would detach the installed cell and cable; terrain-tested legs close the gap.
 const auto Asset=AssetFrame(A);if(Asset.Valid){const auto Base=Asset.Frame;const FFrame World{A,A->GetRootComponent(),FQuat::Identity,FVector::ZeroVector};
  for(float X:{-.35f,.35f})for(float Y:{-.35f,.35f}){const FVector Top=Base.Point(FVector(Asset.Size.X*X,Asset.Size.Y*Y,2));FHitResult Hit;
   if(B.Ground(Top,Hit)&&Top.Z-Hit.ImpactPoint.Z>2&&Top.Z-Hit.ImpactPoint.Z<500){B.Rod(World,Hit.ImpactPoint+FVector(0,0,1),Top,3.2f,Metal);B.Shape(World,TEXT("Cylinder"),Hit.ImpactPoint+FVector(0,0,1.5f),FVector(20,20,3),Dark);}
  }
 }
 // These exact positions match the existing installed-module and cable visual
 // anchors in BuildStoryProps; cosmetic collars never become interaction targets.
 for(const FVector P:{FVector(-77,0,-25),FVector(-57,28,35)}){B.Ring(F,P,6.2f,1.2f,Metal,FVector::ForwardVector);B.Ring(F,P+FVector(-2,0,0),7.6f,.7f,Foil,FVector::ForwardVector);}
 for(float Y:{-20.f,20.f})B.Rod(F,FVector(-70,Y,-65),FVector(-70,Y,-25),1.5f,Metal);
 B.Box(F,FVector(-72,0,-68),FVector(5,45,3),Dark);B.Box(F,FVector(-72,0,-22),FVector(5,45,3),Foil);
 for(int32 I=0;I<4;++I)B.Box(F,FVector(-51,-32+I*10,17),FVector(2,5,13),Dark);
 B.Rod(F,FVector(15,15,80),FVector(15,15,116),1.2f);B.Box(F,FVector(15,15,118),FVector(12,16,7),Dark);
}
void Terminal(FBuilder& B,AActor* A,bool Recorder=false){
 const auto Asset=AssetFrame(A);if(!Asset.Valid)return;const auto F=Asset.Frame;const float Scale=SourceScale(Asset);
 // The screen/control fascia is inset far from the footplate bounding extrema.
 for(float Y:{-25.f,25.f}){
  const FVector Low(-23.0644f,Y,150),High(-23.0644f,Y,195),Out(-8,0,0);
  B.Rod(F,SourcePoint(Asset,Low),SourcePoint(Asset,Low+Out),1.1f*Scale);B.Rod(F,SourcePoint(Asset,High),SourcePoint(Asset,High+Out),1.1f*Scale);B.Rod(F,SourcePoint(Asset,Low+Out),SourcePoint(Asset,High+Out),1.2f*Scale,Dark);
 }
 for(int32 I=0;I<5;++I)B.Box(F,SourcePoint(Asset,FVector(-24.5644f,-20+I*10,150)),FVector(3,3.4f,2.4f)*Scale,Metal);
 if(Recorder)B.Rod(F,SourcePoint(Asset,FVector(-23.0644f,20,195)),SourcePoint(Asset,FVector(-23.0644f,20,240)),.8f*Scale,Metal);
}
void Dish(FBuilder& B,AActor* A){const auto Asset=AssetFrame(A);if(!Asset.Valid)return;const auto F=Asset.Frame;const FVector S=Asset.Size;const float Radius=FMath::Min(float(S.X),float(S.Y))*.46f;
 B.Ring(F,FVector(0,0,S.Z*.5f),Radius,1.6f,Metal);B.Ring(F,FVector(0,0,-6),12,2,Dark);
 for(int32 I=0;I<3;++I){const float T=I*2*PI/3;B.Rod(F,FVector(Radius*.8f*FMath::Cos(T),Radius*.8f*FMath::Sin(T),S.Z*.5f),FVector(0,0,S.Z+25),1.2f);}
 B.Shape(F,TEXT("Cylinder"),FVector(0,0,S.Z+25),FVector(10,10,8),Dark);
}
void Habitat(FBuilder& B,AActor* A){
 const auto Asset=AssetFrame(A);if(!Asset.Valid)return;auto* C=Cast<UStaticMeshComponent>(Asset.Frame.Parent);const bool Main=C->GetStaticMesh()->GetName()==TEXT("HabitatMain");const auto F=Asset.Frame;const float Scale=SourceScale(Asset);
 const FVector Low=Main?FVector(-293.7382f,150,130):FVector(-221.9322f,55,115);
 const FVector High=Main?FVector(-293.7382f,150,230):FVector(-224.7778f,55,215),Out(-9,0,0);
 B.Rod(F,SourcePoint(Asset,Low),SourcePoint(Asset,Low+Out),2*Scale);B.Rod(F,SourcePoint(Asset,High),SourcePoint(Asset,High+Out),2*Scale);B.Rod(F,SourcePoint(Asset,Low+Out),SourcePoint(Asset,High+Out),2.2f*Scale,Dark);
 // Support tops use actual retained source contact vertices. Generic bounding
 // corners are not attachment points on the asymmetric HDU reference geometry.
 const FVector MainFeet[]={FVector(-624.9237f,358.0693f,0),FVector(309.6579f,206.9218f,.0763f),FVector(-505.3638f,413.1569f,0),FVector(304.1644f,311.7561f,.0763f)};
 FFrame World=F;World.Origin=FVector::ZeroVector;World.Rotation=FQuat::Identity;
 for(int32 I=0;I<(Main?4:1);++I){const FVector Contact=Main?MainFeet[I]:FVector(-88.0821f,0,0);const FVector Top=F.Point(SourcePoint(Asset,Contact));FHitResult Hit;if(B.Ground(Top,Hit)){const float Gap=float(Top.Z-Hit.ImpactPoint.Z);if(Gap>=1&&Gap<=180){B.Rod(World,Hit.ImpactPoint+FVector(0,0,2),Top,6,Metal);B.Shape(World,TEXT("Cylinder"),Hit.ImpactPoint+FVector(0,0,1.3f),FVector(27,27,2.6f),Dark);}}}
}
void Rover(FBuilder& B,AActor* A){const auto Asset=AssetFrame(A);if(!Asset.Valid)return;const auto F=Asset.Frame;const float Scale=SourceScale(Asset);
 // Retained Lunokhod OBJ wheel housings: longitudinal axis Y, axle axis X.
 const float CentersX[]={-70.0101f,78.9235f},OuterX[]={-84.3f,93.2f},AxleY[]={71.6832f,9.0526f,-40.9407f,-103.5709f};
 for(int32 Side=0;Side<2;++Side)for(float Y:AxleY){
  B.Ring(F,SourcePoint(Asset,FVector(OuterX[Side],Y,22.2865f)),8*Scale,.8f*Scale,Metal,FVector::ForwardVector);
  B.Rod(F,SourcePoint(Asset,FVector(CentersX[Side],Y,22.2865f)),SourcePoint(Asset,FVector(Side==0?-44.f:48.f,Y,59)),1.8f*Scale,Dark);
 }
 const FVector Mast(-94.18749f,-13.8985f,172.81773f),Top=Mast+FVector(0,0,40);
 B.Rod(F,SourcePoint(Asset,Mast),SourcePoint(Asset,Top),2*Scale);B.Box(F,SourcePoint(Asset,Top+FVector(0,0,2)),FVector(22,32,12)*Scale,Paint);for(float Y:{-9.f,9.f})B.Shape(F,TEXT("Sphere"),SourcePoint(Asset,Top+FVector(-11,Y,2)),FVector(6)*Scale,Dark);
 B.Cable(F,SourcePoint(Asset,Mast+FVector(0,0,5)),SourcePoint(Asset,FVector(42.26778f,-.6576f,119.89215f)),FVector(0,14,-12)*Scale,1.1f*Scale);
}
void Solar(FBuilder& B,AActor* A){const auto Asset=AssetFrame(A);if(!Asset.Valid)return;const auto F=Asset.Frame;const float Scale=SourceScale(Asset);
 // Runtime source-triangle probes establish z + y = 95.000818 cm.
 // The negative normal points beneath that plane; the former mixed sign
 // placed the junction/cable direction along the panel instead of beneath it.
 const FVector N(0,-.70710678f,-.70710678f),Base(145,0,95.000818f);
 B.Shape(F,TEXT("Cube"),SourcePoint(Asset,Base+N*5.5f),FVector(23,17,11)*Scale,Dark,FRotationMatrix::MakeFromZX(N,FVector::ForwardVector).ToQuat());
 for(float X:{-125.f,125.f}){const FVector At(X,0,95.000818f);B.Rod(F,SourcePoint(Asset,At+N*2.5f-FVector(7,0,0)),SourcePoint(Asset,At+N*2.5f+FVector(7,0,0)),2.5f*Scale,Metal);}
 B.Cable(F,SourcePoint(Asset,Base+N*11),SourcePoint(Asset,FVector(-60,-35,130.000818f)+N*2),N*(8*Scale),.9f*Scale);
}
bool IsGroundedStructure(const FString& Name){return Name==TEXT("HabitatMain")||Name==TEXT("HabitatUtility")||Name==TEXT("SolarPanel");}
bool StructureGround(FBuilder& B,AStaticMeshActor* Actor,TArray<FHitResult>& Hits,double& Highest){
 Hits.Reset();Highest=-TNumericLimits<double>::Max();auto* C=Actor?Actor->GetStaticMeshComponent():nullptr;if(!C||!C->GetStaticMesh())return false;
 const auto Bounds=C->Bounds;const FVector Center=Bounds.Origin,Extent=Bounds.BoxExtent;
 for(const FVector2D Offset:{FVector2D::ZeroVector,FVector2D(-.7,-.7),FVector2D(-.7,.7),FVector2D(.7,-.7),FVector2D(.7,.7)}){
  FHitResult Hit;if(!B.Ground(Center+FVector(Extent.X*Offset.X,Extent.Y*Offset.Y,0),Hit)||Hit.ImpactNormal.Z<.65f||!FMath::IsFinite(Hit.ImpactPoint.Z))return false;
  Highest=FMath::Max(Highest,Hit.ImpactPoint.Z);Hits.Add(Hit);
 }return Hits.Num()==5;
}
void GroundStructure(FBuilder& B,AStaticMeshActor* Actor){
 TArray<FHitResult> Hits;double Highest;if(!StructureGround(B,Actor,Hits,Highest))return;
 auto* C=Actor->GetStaticMeshComponent();const double Bottom=C->Bounds.Origin.Z-C->Bounds.BoxExtent.Z,Shift=Highest-Bottom;
 if(!FMath::IsFinite(Shift)||FMath::Abs(Shift)>1200)return;
 C->SetMobility(EComponentMobility::Movable);Actor->AddActorWorldOffset(FVector(0,0,Shift),false,nullptr,ETeleportType::TeleportPhysics);C->UpdateBounds();Actor->Tags.AddUnique(TEXT("Lunar.StructureGrounded"));
 if(C->GetStaticMesh()->GetName()==TEXT("SolarPanel")){
  // An authored conservative obstacle envelope prevents walking/driving
  // through the non-colliding display mesh. It is not a precision contact
  // mesh or a physics simulation of the solar-array structure.
  auto* Collision=NewObject<UBoxComponent>(Actor);Collision->SetupAttachment(C);Collision->SetMobility(EComponentMobility::Movable);const auto Source=C->GetStaticMesh()->GetBounds();Collision->SetRelativeLocation(Source.Origin);Collision->SetBoxExtent(Source.BoxExtent);Collision->SetCollisionProfileName(TEXT("BlockAll"));Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Collision->SetCanEverAffectNavigation(false);Collision->ComponentTags.Add(TEXT("Lunar.StructureCollision"));Collision->RegisterComponent();Actor->SetActorEnableCollision(true);
 }
 // Small authored adjustable feet span remaining uneven-ground gaps. Main
 // structure collision remains intact. Measured terrain and mission anchors
 // are not moved, regenerated, flattened or made to simulate physics.
 const auto F=Local(Actor);for(int32 I=1;I<Hits.Num();++I){const FVector Ground=Hits[I].ImpactPoint;const double Gap=Highest-Ground.Z;if(Gap>3&&Gap<100){const FVector Top(Ground.X,Ground.Y,Highest+4);B.Rod(F,F.Rotation.UnrotateVector(Ground-F.Origin),F.Rotation.UnrotateVector(Top-F.Origin),5,Metal);}}
 UE_LOG(LogTemp,Display,TEXT("STRUCTURE_GROUNDED mesh=%s dz_cm=%.2f terrain_samples=5 physics_simulation=0"),*C->GetStaticMesh()->GetName(),Shift);
}
void Rescue(FBuilder& B){auto* G=B.Game;Battery(B,G->ModuleActor);Port(B,G->PowerActor);Terminal(B,G->DataActor);Terminal(B,G->RecorderActor,true);if(G->RelayActor)Terminal(B,G->RelayActor);if(G->RelayDish)Dish(B,G->RelayDish);if(G->PowerAntenna)Dish(B,G->PowerAntenna);
 if(!G->Mars){
  for(const auto& A:G->SampleActors){const auto F=Local(A);B.Ring(F,FVector(0,0,31),20.8f,.8f,Metal);B.Box(F,FVector(0,0,31.8f),FVector(20,3,2.5f),Dark);for(float X:{-15.f,15.f})B.Box(F,FVector(X,0,24),FVector(4,7,12),Metal);}
  if(G->DroneActor){const auto F=Local(G->DroneActor);B.Box(F,FVector(30,-67,5),FVector(27,3,39),Dark);for(float Z:{-7.f,17.f})B.Rod(F,FVector(22,-69,Z),FVector(38,-69,Z),1.2f);for(int32 I=0;I<4;++I)B.Box(F,FVector(24+I*4,-69,0),FVector(1.5f,2,10),Metal);}
  if(G->BeaconActor){const auto F=Local(G->BeaconActor);B.Ring(F,FVector(0,0,-22),43,1.2f,Metal);for(int32 I=0;I<3;++I)B.Box(F,FVector(-34,-16+I*16,0),FVector(3,5,12),Foil);}
 }
 TArray<AStaticMeshActor*> Existing;for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It)Existing.Add(*It);int32 PoleCount=0;
 for(auto* A:Existing){auto* C=A->GetStaticMeshComponent();const FString Name=C&&C->GetStaticMesh()?C->GetStaticMesh()->GetName():FString();
  if(IsGroundedStructure(Name))GroundStructure(B,A);
  if(Name==TEXT("ApolloLander"))Lander(B,A);else if(Name==TEXT("HabitatMain")||Name==TEXT("HabitatUtility"))Habitat(B,A);else if(Name==TEXT("Lunokhod"))Rover(B,A);else if(Name==TEXT("SolarPanel"))Solar(B,A);
  else if(Name==TEXT("Cylinder")&&PoleCount<16){const auto AF=AssetFrame(A);if(AF.Valid&&AF.Size.X<15&&AF.Size.Y<15&&AF.Size.Z>60&&AF.Size.Z<230){B.Ring(AF.Frame,FVector(0,0,AF.Size.Z*.86f),float(AF.Size.X)*.7f,.8f,Foil);++PoleCount;}}
  if(G->Mars&&G->HomeActor&&Name==TEXT("Cylinder")&&FVector::Dist2D(A->GetActorLocation(),G->HomeActor->GetActorLocation())<500){const auto AF=AssetFrame(A);if(AF.Valid&&AF.Size.Z>400&&AF.Size.X>200)Lander(B,A);}
 }
}
// Bounds are source centimetres, not an assumed 100 cm primitive. This pure
// pose function keeps the source bottom centre at -20% of the desired height
// relative to the measured tangent plane, including asymmetric mesh origins.
bool RockPose(const FBox& Bounds,FVector Dimensions,FVector Ground,FVector Normal,double Yaw,FVector& Scale,FVector& Origin,FQuat& Rotation){
 Scale=FVector::ZeroVector;Origin=FVector::ZeroVector;Rotation=FQuat::Identity;
 if(!Bounds.IsValid||Bounds.Min.ContainsNaN()||Bounds.Max.ContainsNaN()||Dimensions.ContainsNaN()||Ground.ContainsNaN()||Normal.ContainsNaN()||!FMath::IsFinite(Yaw)||Dimensions.GetMin()<=UE_SMALL_NUMBER)return false;
 const FVector SourceSize=Bounds.GetSize();if(SourceSize.ContainsNaN()||SourceSize.GetMin()<=UE_SMALL_NUMBER||!Normal.Normalize())return false;
 Scale=Dimensions/SourceSize;Rotation=FQuat::FindBetweenNormals(FVector::UpVector,Normal)*FRotator(0,Yaw,0).Quaternion();
 const FVector Bottom(Bounds.GetCenter().X,Bounds.GetCenter().Y,Bounds.Min.Z);
 Origin=Ground-Normal*(Dimensions.Z*.2)-Rotation.RotateVector(Bottom*Scale);
 return !Scale.ContainsNaN()&&!Origin.ContainsNaN()&&!Rotation.ContainsNaN();
}
bool GroundRock(FBuilder& B,const FFrame& F,const FString& Path,const FHitResult& Hit,FVector Dimensions,double Yaw){
 auto* Mesh=B.Mesh(Path);if(!Mesh){++B.Skipped;return false;}
 FVector Scale,Origin;FQuat Rotation;if(!RockPose(Mesh->GetBoundingBox(),Dimensions,Hit.ImpactPoint,Hit.ImpactNormal,Yaw,Scale,Origin,Rotation)){++B.Skipped;return false;}
 const int32 Before=B.Instances;
 // Add divides its dimensions by 100 for Engine primitives. Feed the actual
 // source scale through that adapter; existing batching/material/caps remain.
 B.Add(F,Path,F.Rotation.UnrotateVector(Origin-F.Origin),Scale*100,Rock,F.Rotation.Inverse()*Rotation);
 return B.Instances>Before;
}
void Surface(FBuilder& B){if(!B.Terrain)return;auto* G=B.Game;auto* A=G->GetWorld()->SpawnActor<AActor>();if(!A)return;auto* Root=NewObject<USceneComponent>(A);A->SetRootComponent(Root);Root->RegisterComponent();A->Tags.Add(TEXT("Lunar.EquipmentSurface"));const auto F=Local(A);FRandomStream Rand(G->Apollo?196912:G->Mars?203501:202601);
 TArray<FVector> Sites;if(G->ModuleActor)Sites.Add(G->ModuleActor->GetActorLocation());if(G->PowerActor)Sites.Add(G->PowerActor->GetActorLocation());if(G->HomeActor)Sites.Add(G->HomeActor->GetActorLocation());if(G->Apollo&&G->DroneActor)Sites.Add(G->DroneActor->GetActorLocation());
 for(int32 I=0;I<192&&Sites.Num();++I){const float Angle=Rand.FRand()*2*PI,Radius=Rand.FRandRange(320,850);FVector P=Sites[I%Sites.Num()]+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);bool Clear=true;for(const auto& S:Sites)Clear&=FVector::DistSquared2D(S,P)>180.f*180.f;if(!Clear)continue;FHitResult Hit;if(!B.Ground(P,Hit))continue;
  const float Size=Rand.FRandRange(3,14);const FString Name=FString::Printf(TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_%d.SM_Basalt_%d"),I%6,I%6);const double Yaw=Rand.FRandRange(0,360);GroundRock(B,F,Name,Hit,FVector(Size,Size*Rand.FRandRange(.55f,1.2f),Size*.55f),Yaw);
 }
 if(G->Mars){int32 Mesa=0;for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It&&Mesa<12;++It){auto* C=It->GetStaticMeshComponent();if(!C||!C->GetStaticMesh()||C->GetStaticMesh()->GetName()!=TEXT("SM_MarsMesa"))continue;const auto AF=AssetFrame(*It);if(!AF.Valid)continue;++Mesa;for(int32 I=0;I<4;++I){const float T=I*HALF_PI+.23f;FVector P=AF.Frame.Point(FVector(FMath::Cos(T)*AF.Size.X*.42f,FMath::Sin(T)*AF.Size.Y*.42f,0));FHitResult Hit;if(!B.Ground(P,Hit))continue;const FString Name=FString::Printf(TEXT("/Game/Lunar/V3/Meshes/SM_Basalt_%d.SM_Basalt_%d"),(I+Mesa)%6,(I+Mesa)%6);GroundRock(B,F,Name,Hit,FVector(70,45,25),I*87.f+Mesa*11.f);}}
 }
}
}

void LunarEquipmentDetail::Build(ALunarGameMode* G){
 if(!G||G->Menu||!G->GetWorld()||G->ActorHasTag(BuiltTag))return;
 FBuilder B(G);if(G->Apollo){Surveyor(B);for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It)if(It->ActorHasTag(TEXT("apollo.reference-lander")))Lander(B,*It);}else Rescue(B);Surface(B);G->Tags.Add(BuiltTag);
 UE_LOG(LogTemp,Display,TEXT("LUNAR_EQUIPMENT_DETAIL planet=%s batches=%d instances=%d cap_batches=%d cap_instances=%d missing=%d skipped=%d terrain_collision_unchanged=1 mission_anchors_unchanged=1"),G->Apollo?TEXT("APOLLO12"):G->Mars?TEXT("MARS"):TEXT("SELENE"),B.Batches.Num(),B.Instances,MaxComponents,MaxInstances,B.Missing,B.Skipped);
}
void LunarEquipmentDetail::AppendChecks(ALunarGameMode* G,FString& Report,int32& Failed){
 auto Check=[&](bool Good,const TCHAR* Name){Report+=FString(Good?TEXT("PASS "):TEXT("FAIL "))+Name+TEXT("\n");if(!Good)++Failed;};
 Check(G&&G->GetWorld()&&!G->Menu,TEXT("Equipment detail checks have a playable world"));if(!G||!G->GetWorld()||G->Menu)return;
 if(!G->Apollo){FBuilder B(G);int32 Structures=0;bool Grounded=true;for(TActorIterator<AStaticMeshActor> It(G->GetWorld());It;++It){auto* C=It->GetStaticMeshComponent();if(!C||!C->GetStaticMesh()||!IsGroundedStructure(C->GetStaticMesh()->GetName()))continue;++Structures;TArray<FHitResult> Hits;double Highest;Grounded&=It->ActorHasTag(TEXT("Lunar.StructureGrounded"))&&StructureGround(B,*It,Hits,Highest)&&FMath::Abs(C->Bounds.Origin.Z-C->Bounds.BoxExtent.Z-Highest)<1.0&&!C->IsSimulatingPhysics();}Check(Structures>0&&Grounded,TEXT("Imported base modules and solar supports rest on sampled terrain without unstable physics"));}
 auto Count=[&](int32& Instances,bool& Cosmetic){int32 Components=0;Instances=0;Cosmetic=true;for(TActorIterator<AActor> It(G->GetWorld());It;++It){TArray<UInstancedStaticMeshComponent*> Meshes;It->GetComponents(Meshes);for(auto* C:Meshes)if(C->ComponentHasTag(DetailTag)){++Components;Instances+=C->GetInstanceCount();Cosmetic&=C->GetStaticMesh()!=nullptr&&C->GetMaterial(0)!=nullptr&&C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!C->CanEverAffectNavigation()&&!C->IsComponentTickEnabled()&&C->InstanceEndCullDistance<=6500;}}return Components;};
 int32 Instances=0;bool Cosmetic=false;const int32 Components=Count(Instances,Cosmetic);Check(G->ActorHasTag(BuiltTag)&&Components>0&&Instances>0,TEXT("Mission received visible authored equipment detail"));Check(Components<=MaxComponents&&Instances<=MaxInstances,TEXT("Equipment detail remains within 128 component and 1400 instance caps"));Check(Cosmetic,TEXT("Added equipment uses existing meshes/materials with no collision, navigation or Tick"));
 // These checks exercise geometric requirements independently of any mesh
 // origin convention, then verify actual spawned source boxes against terrain.
 const FBox Asymmetric(FVector(-17,-9,-4),FVector(5,11,19));const FVector Desired(70,45,25),Plane(11,-23,42);
 auto PoseCheck=[&](FVector Normal){FVector Scale,Origin;FQuat Rotation;if(!RockPose(Asymmetric,Desired,Plane,Normal,37,Scale,Origin,Rotation))return false;Normal.Normalize();double Low=TNumericLimits<double>::Max(),High=-TNumericLimits<double>::Max();for(int32 I=0;I<8;++I){const FVector Corner((I&1)?Asymmetric.Max.X:Asymmetric.Min.X,(I&2)?Asymmetric.Max.Y:Asymmetric.Min.Y,(I&4)?Asymmetric.Max.Z:Asymmetric.Min.Z);const double D=FVector::DotProduct(Origin+Rotation.RotateVector(Corner*Scale)-Plane,Normal);Low=FMath::Min(Low,D);High=FMath::Max(High,D);}return FMath::IsNearlyEqual(Low,-5.,.0001)&&FMath::IsNearlyEqual(High,20.,.0001)&&FVector::DotProduct(Rotation.RotateVector(FVector::UpVector),Normal)>.99999;};
 Check(PoseCheck(FVector::UpVector),TEXT("Asymmetric source rock box on flat ground spans -20% to +80% desired height"));
 Check(PoseCheck(FVector(.6,0,.8)),TEXT("Asymmetric source rock box follows known terrain normal without changing burial depth"));
 FVector InvalidScale,InvalidOrigin;FQuat InvalidRotation;const double NaN=std::numeric_limits<double>::quiet_NaN(),Infinity=std::numeric_limits<double>::infinity();
 auto Invalid=[&](const FBox& Box,FVector Size,FVector Point,FVector Normal,double Yaw){return !RockPose(Box,Size,Point,Normal,Yaw,InvalidScale,InvalidOrigin,InvalidRotation);};
 Check(Invalid(Asymmetric,FVector(0,1,1),Plane,FVector::UpVector,0)&&Invalid(Asymmetric,FVector(-1,1,1),Plane,FVector::UpVector,0)&&Invalid(FBox(ForceInit),Desired,Plane,FVector::UpVector,0)&&Invalid(Asymmetric,Desired,Plane,FVector::ZeroVector,0)&&Invalid(Asymmetric,FVector(NaN,1,1),Plane,FVector::UpVector,0)&&Invalid(Asymmetric,Desired,FVector(Infinity,0,0),FVector::UpVector,0)&&Invalid(Asymmetric,Desired,Plane,FVector(0,0,Infinity),0)&&Invalid(Asymmetric,Desired,Plane,FVector::UpVector,NaN),TEXT("Rock pose rejects zero negative invalid and nonfinite inputs"));
 FBuilder TerrainProof(G);int32 RockCount=0;bool Exposed=true,Grounded=true,CosmeticRocks=true;
 for(TActorIterator<AActor> It(G->GetWorld());It;++It)if(It->ActorHasTag(TEXT("Lunar.EquipmentSurface"))){TArray<UInstancedStaticMeshComponent*> Meshes;It->GetComponents(Meshes);for(auto* C:Meshes)if(C->ComponentHasTag(DetailTag)&&C->GetStaticMesh()&&C->GetStaticMesh()->GetName().StartsWith(TEXT("SM_Basalt_"))){const FBox Box=C->GetStaticMesh()->GetBoundingBox();CosmeticRocks&=C->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!C->IsComponentTickEnabled()&&!C->CanEverAffectNavigation();for(int32 I=0;I<C->GetInstanceCount();++I){++RockCount;FTransform T;if(!C->GetInstanceTransform(I,T,true)){Exposed=Grounded=false;continue;}const FVector Up=T.GetRotation().RotateVector(FVector::UpVector),Bottom(Box.GetCenter().X,Box.GetCenter().Y,Box.Min.Z);const double Height=Box.GetSize().Z*T.GetScale3D().Z;const FVector Contact=T.TransformPosition(Bottom)+Up*(Height*.2);FHitResult Hit;if(!TerrainProof.Ground(Contact,Hit)){Exposed=Grounded=false;continue;}double Low=TNumericLimits<double>::Max(),High=-TNumericLimits<double>::Max();for(int32 Corner=0;Corner<8;++Corner){const FVector P((Corner&1)?Box.Max.X:Box.Min.X,(Corner&2)?Box.Max.Y:Box.Min.Y,(Corner&4)?Box.Max.Z:Box.Min.Z);const double D=FVector::DotProduct(T.TransformPosition(P)-Hit.ImpactPoint,Hit.ImpactNormal);Low=FMath::Min(Low,D);High=FMath::Max(High,D);}Exposed&=High>.1;Grounded&=Low<=1&&FMath::IsNearlyEqual(Low,-Height*.2,.1)&&FMath::IsNearlyEqual(High,Height*.8,.1);}}}
 Check(RockCount>0&&Exposed,TEXT("Every actual authored Basalt instance bbox extends above its measured terrain plane"));
 Check(RockCount>0&&Grounded,TEXT("Actual authored Basalt source boxes are grounded with 20 percent burial and no floating base"));
 Check(RockCount>0&&CosmeticRocks,TEXT("Grounded Basalt remains cosmetic with no collision navigation or Tick"));
 const auto Stage=G->Stage;const float Oxygen=G->Oxygen,Hold=G->HoldProgress;const uint8 Samples=G->SamplesCollected,Parts=G->ApolloParts,Photos=G->ApolloPhotos;TArray<TPair<AActor*,FTransform>> Anchors;for(AActor* A:{G->ModuleActor.Get(),G->PowerActor.Get(),G->DataActor.Get(),G->HomeActor.Get(),G->DroneActor.Get(),G->RecorderActor.Get()})if(A)Anchors.Add({A,A->GetActorTransform()});
 Build(G);int32 AfterInstances=0;bool AfterCosmetic=false;const int32 After=Count(AfterInstances,AfterCosmetic);bool Same=true;for(const auto& A:Anchors)Same&=A.Key->GetActorTransform().Equals(A.Value,.0001f);
 Check(After==Components&&AfterInstances==Instances&&Same,TEXT("Repeating equipment build adds no geometry and preserves mission anchor transforms"));Check(G->Stage==Stage&&G->Oxygen==Oxygen&&G->HoldProgress==Hold&&G->SamplesCollected==Samples&&G->ApolloParts==Parts&&G->ApolloPhotos==Photos,TEXT("Equipment presentation does not advance mission work or change resources"));
 if(G->Apollo){int32 Markers=0;for(const auto& A:G->ApolloPartActors){TArray<UStaticMeshComponent*> Meshes;A->GetComponents(Meshes);for(auto* C:Meshes)Markers+=C->ComponentHasTag(TEXT("work.0"))||C->ComponentHasTag(TEXT("work.1"));}Check(Markers==6,TEXT("Surveyor retains exactly its six original removable-part work fittings"));Check(G->ApolloAntenna!=nullptr,TEXT("ALSEP retains its original separately adjustable antenna component"));}
}
