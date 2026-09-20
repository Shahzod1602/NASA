using UnrealBuildTool;
public class LunarRescueTarget : TargetRules {
 public LunarRescueTarget(TargetInfo Target) : base(Target) {
  Type = TargetType.Game; DefaultBuildSettings = BuildSettingsVersion.V7;
  IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
  ExtraModuleNames.Add("LunarRescue");
 }
}
