#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UStaticMeshComponent;

// Presentation-only derivative of the retained reference mesh. Source geometry,
// collision and mission coordinates remain authoritative and are never edited.
namespace LunarIntrepidCabin {
inline constexpr const TCHAR* SourcePath = TEXT("/Game/Lunar/Imported/ApolloLander/ApolloLander.ApolloLander");
inline constexpr const TCHAR* DerivedPath = TEXT("/Game/Lunar/Apollo12/IntrepidVisual/SM_IntrepidCabin.SM_IntrepidCabin");
inline constexpr const TCHAR* MaterialPath = TEXT("/Game/Lunar/Apollo12/IntrepidVisual/M_IntrepidCabin.M_IntrepidCabin");
inline constexpr const TCHAR* ComponentTag = TEXT("Apollo.IntrepidCabin");
inline constexpr const TCHAR* ApprovedSlots[] = {TEXT("blinn3SG_001"), TEXT("initialShadingGr_001")};
bool IsCabinSlot(FName Slot);
bool Compatible(const UStaticMesh* Source, const UStaticMesh* Derived);
bool Apply(UStaticMeshComponent* Component);
}
