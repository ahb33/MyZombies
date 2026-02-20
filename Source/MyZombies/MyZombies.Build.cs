// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MyZombies : ModuleRules
{
	public MyZombies(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "UMG",
			"GameplayTags", "GameplayTasks", "AIModule", "Niagara",
			"NavigationSystem", "MultiplayerPlugin", "OnlineSubsystem", "OnlineSubsystemSteam", "Sockets"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "CCDebug", "Slate", "SlateCore", "CCGameplayDebugger" });

		// Use PrivateIncludePaths unless other modules need to include these headers.
		PrivateIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "UI"),
			Path.Combine(ModuleDirectory, "GameModes"),
			Path.Combine(ModuleDirectory, "Pickups"),
			Path.Combine(ModuleDirectory, "EnemyAI"),
			Path.Combine(ModuleDirectory, "PlayerState"),
			Path.Combine(ModuleDirectory, "GameState"),
			Path.Combine(ModuleDirectory, "Weapons"),
			Path.Combine(ModuleDirectory, "Controllers"),
			Path.Combine(ModuleDirectory, "MainCharacter"),
			Path.Combine(ModuleDirectory, "Combat")
		});
	}
}
