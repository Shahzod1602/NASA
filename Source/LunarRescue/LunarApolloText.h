#pragma once
#include "CoreMinimal.h"

// Stable string-table keys shared by the Apollo HUD, radio and mission rules.
// The English table is code-defined so it is present in offline builds.
namespace ApolloText {
 FText Text(const TCHAR* Key);
 FString Get(const TCHAR* Key);
 FString Format(const TCHAR* Key, const FFormatNamedArguments& Arguments);
}
