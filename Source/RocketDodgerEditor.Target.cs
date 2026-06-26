using UnrealBuildTool;
using System.Collections.Generic;

public class RocketDodgerEditorTarget : TargetRules
{
	public RocketDodgerEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bOverrideBuildEnvironment = true;
		ExtraModuleNames.Add("RocketDodger");
	}
}
