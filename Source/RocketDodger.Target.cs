using UnrealBuildTool;
using System.Collections.Generic;

public class RocketDodgerTarget : TargetRules
{
	public RocketDodgerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bOverrideBuildEnvironment = true;
		ExtraModuleNames.Add("RocketDodger");
	}
}
