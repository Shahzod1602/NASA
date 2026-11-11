#include "LunarIntrepidCabin.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace LunarIntrepidCabin {
bool IsCabinSlot(FName Slot){
 for(const TCHAR* Name:ApprovedSlots)if(Slot==FName(Name))return true;
 return false;
}

bool Compatible(const UStaticMesh* Source,const UStaticMesh* Derived){
 if(!Source||!Derived||Source==Derived)return false;
 const auto A=Source->GetBounds(),B=Derived->GetBounds();
 if(A.Origin.ContainsNaN()||A.BoxExtent.ContainsNaN()||B.Origin.ContainsNaN()||B.BoxExtent.ContainsNaN()||
    A.BoxExtent.GetMin()<=0||!A.Origin.Equals(B.Origin,.001)||!A.BoxExtent.Equals(B.BoxExtent,.001))return false;
 const auto& Original=Source->GetStaticMaterials();const auto& Replacement=Derived->GetStaticMaterials();
 if(Original.Num()==0||Original.Num()!=Replacement.Num()||Source->GetNumLODs()!=Derived->GetNumLODs())return false;
 int32 CabinSlots=0;
 for(int32 I=0;I<Original.Num();++I){
  if(Original[I].MaterialSlotName!=Replacement[I].MaterialSlotName||Original[I].MaterialInterface!=Replacement[I].MaterialInterface)return false;
  CabinSlots+=IsCabinSlot(Original[I].MaterialSlotName)?1:0;
 }
 return CabinSlots==UE_ARRAY_COUNT(ApprovedSlots);
}

bool Apply(UStaticMeshComponent* Component){
 if(!Component||FParse::Param(FCommandLine::Get(),TEXT("IntrepidCabinBaseline")))return false;
 if(Component->ComponentHasTag(ComponentTag))return true;
 UStaticMesh* Source=Component->GetStaticMesh();
 if(!Source||Source->GetPathName()!=SourcePath||Component->GetCollisionEnabled()!=ECollisionEnabled::NoCollision)return false;
 auto* Derived=LoadObject<UStaticMesh>(nullptr,DerivedPath);
 auto* Material=LoadObject<UMaterialInterface>(nullptr,MaterialPath);
 if(!Compatible(Source,Derived)||!Material){
  UE_LOG(LogTemp,Warning,TEXT("INTREPID_CABIN optional derivative unavailable/incompatible; retained source appearance remains"));
  return false;
 }
 const FTransform Before=Component->GetComponentTransform();
 TArray<UMaterialInterface*> Materials;
 for(int32 I=0;I<Source->GetStaticMaterials().Num();++I)Materials.Add(Component->GetMaterial(I));
 if(!Component->SetStaticMesh(Derived))return false;
 int32 Changed=0;
 for(int32 I=0;I<Materials.Num();++I){
  const bool Cabin=IsCabinSlot(Source->GetStaticMaterials()[I].MaterialSlotName);
  if(Cabin){
   auto* Instance=UMaterialInstanceDynamic::Create(Material,Component);
   // Preserve the two retained GLB materials' distinct neutral base factors.
   // Weave/roughness in the new graph remain authored presentation tuning.
   const float Tint=Source->GetStaticMaterials()[I].MaterialSlotName==FName(TEXT("initialShadingGr_001"))?.498039216f:.678431392f;
   Instance->SetVectorParameterValue(TEXT("CabinTint"),FLinearColor(Tint,Tint,Tint,1));
   Component->SetMaterial(I,Instance);++Changed;
  }else Component->SetMaterial(I,Materials[I]);
 }
 Component->ComponentTags.Add(ComponentTag);
 UE_LOG(LogTemp,Display,TEXT("INTREPID_CABIN applied slots=%d source_geometry_unchanged=1 source_materials_unchanged=1 pose_unchanged=%d collision_unchanged=1"),Changed,Component->GetComponentTransform().Equals(Before,.0001));
 return true;
}
}
