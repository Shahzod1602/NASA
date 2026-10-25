#pragma once
#include "CoreMinimal.h"

// Only mission photographs use this path. Review/console screenshots retain
// the engine's normal handling. Compression and file I/O run on its write queue.
namespace LunarPhotos {
void Initialize();
bool Request(const FString& Filename,TFunction<void(bool)> Completed);
}
