// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyProjectEditorTarget : TargetRules
{
	public MyProjectEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		bOverrideBuildEnvironment = true;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("MyProject");
	}
}
