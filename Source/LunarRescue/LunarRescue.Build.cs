using UnrealBuildTool;
public class LunarRescue : ModuleRules {
 public LunarRescue(ReadOnlyTargetRules Target) : base(Target) {
  // The engine shared PCH exhausts virtual memory on the 16 GB development PC.
  // Keep this small game module independent of that multi-gigabyte reservation.
  PCHUsage = PCHUsageMode.NoPCHs;
  // File-local helpers must remain isolated in clean and adaptive builds.
  bUseUnity = false;
  PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore","ImageWriteQueue","MoviePlayer","Json"});
  if (Target.bBuildEditor) {
   PrivateDependencyModuleNames.AddRange(new string[]{"MeshDescription","StaticMeshDescription","AssetRegistry"});
  }
 }
}
