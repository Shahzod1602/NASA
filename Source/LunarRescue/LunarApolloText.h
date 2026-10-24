#pragma once
#include "CoreMinimal.h"

// Stable string-table keys shared by the Apollo HUD, radio and mission rules.
// The two shipped tables are code-defined so they are also present in offline builds.
namespace ApolloText {
 FText Text(const TCHAR* Key);
 FString Get(const TCHAR* Key);
 FString Format(const TCHAR* Key, const FFormatNamedArguments& Arguments);
 bool IsUzbek();
 void SetUzbek(bool Uzbek, bool SavePreference=true);
}
