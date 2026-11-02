#pragma once
#include "CoreMinimal.h"
class ALunarGameMode;
class ULunarPanel;

// A read-only view of existing mission state, not an electrical simulation.
namespace LunarDiagnostics {
 enum class EState : uint8 { Pending, Ready, Blocked, InProgress, Complete, Unavailable };
 struct FRow {
  FName Key;
  FString Label;
  EState State=EState::Pending;
  FString Detail;
 };
 struct FSnapshot {
  bool Available=false;
  FString Title,Summary,NextAction,ModelNote;
  TArray<FRow> Rows;
 };
 FSnapshot Build(const ALunarGameMode* Game);
 FString StateLabel(EState State);
 void BuildNotebook(ULunarPanel* Panel);
 void RunChecks(ALunarGameMode* Game);
}
