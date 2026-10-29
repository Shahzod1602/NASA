using UnrealBuildTool;
public class LunarRescue : ModuleRules {
 public LunarRescue(ReadOnlyTargetRules Target) : base(Target) {
  // The engine shared PCH exhausts virtual memory on the 16 GB development PC.
  // Keep this small game module independent of that multi-gigabyte reservation.
  PCHUsage = PCHUsageMode.NoPCHs;
  PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore","ImageWriteQueue","MoviePlayer"});
 }
}
