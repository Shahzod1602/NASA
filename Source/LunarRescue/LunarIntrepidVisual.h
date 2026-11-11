#pragma once

#include "CoreMinimal.h"

class ALunarGameMode;
class UStaticMeshComponent;

// An authored rendering layer over the existing LROC collision surface.
// It never represents additional measured terrain or changes mission coordinates.
namespace LunarIntrepidVisual {
inline constexpr float HalfExtentCM = 2500.f;
inline constexpr float EdgeFadeCM = 350.f;
bool Inside(const FVector& Position);
bool Available(UStaticMeshComponent* Terrain);
void Build(ALunarGameMode* Game, UStaticMeshComponent* Terrain);
}
