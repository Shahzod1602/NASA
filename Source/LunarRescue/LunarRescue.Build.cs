using UnrealBuildTool;
public class LunarRescue : ModuleRules {
 public LunarRescue(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore"});
 }
}
