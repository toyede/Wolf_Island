// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Wolf_Island : ModuleRules
{
	public Wolf_Island(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"UMG",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "AdvancedSessions" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file
	}
}
