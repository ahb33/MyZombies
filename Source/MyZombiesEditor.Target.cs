// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyZombiesEditorTarget : TargetRules
{
	public MyZombiesEditorTarget(TargetInfo Target) : base(Target)
	{
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5; // Use latest build settings
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new[] { "MyZombies", "CCDebug" });
    }
}
