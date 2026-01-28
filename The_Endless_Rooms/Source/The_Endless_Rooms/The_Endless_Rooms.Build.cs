// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class The_Endless_Rooms : ModuleRules
{
	public The_Endless_Rooms(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"OnlineSubsystem",
			"OnlineSubsystemSteam",
			"OnlineSubsystemUtils",
            "GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"The_Endless_Rooms",
			"The_Endless_Rooms/Variant_Horror",
			"The_Endless_Rooms/Variant_Horror/UI",
			"The_Endless_Rooms/Variant_Shooter",
			"The_Endless_Rooms/Variant_Shooter/AI",
			"The_Endless_Rooms/Variant_Shooter/UI",
			"The_Endless_Rooms/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
