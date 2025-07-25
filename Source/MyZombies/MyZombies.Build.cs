// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyZombies : ModuleRules
{
	public MyZombies(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" , "GameplayTasks", "AIModule", "Niagara", "NavigationSystem", "MultiplayerPlugin", "OnlineSubsystem", "OnlineSubsystemSteam", "Sockets"});
	
		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
