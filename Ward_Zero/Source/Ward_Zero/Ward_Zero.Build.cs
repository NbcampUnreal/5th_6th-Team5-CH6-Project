// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ward_Zero : ModuleRules
{
    public Ward_Zero(ReadOnlyTargetRules Target) : base(Target)
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
            "GameplayStateTreeModule",
            "UMG",
            "Slate",
            "Niagara",
            "GameplayTags",
            "PhysicsCore",
            "NavigationSystem",
            "AnimGraphRuntime",
            "AnimationBlueprintLibrary",
            "MotionWarping",

            //Level
            "GeometryCollectionEngine",
            "FieldSystemEngine",         
            "ChaosSolverEngine",
            "ChaosNiagara",       
            "Chaos",
            
            //GAS
            "GameplayAbilities",  
            "GameplayTags",  
            "GameplayTasks",
            
            
            //UI
            "ImageWrapper", 
            "RenderCore", 
            "SlateCore",
            "MoviePlayer"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] {
            "Ward_Zero",
            "Ward_Zero/Variant_Platforming",
            "Ward_Zero/Variant_Platforming/Animation",
            "Ward_Zero/Variant_Combat",
            "Ward_Zero/Variant_Combat/AI",
            "Ward_Zero/Variant_Combat/Animation",
            "Ward_Zero/Variant_Combat/Gameplay",
            "Ward_Zero/Variant_Combat/Interfaces",
            "Ward_Zero/Variant_Combat/UI",
            "Ward_Zero/Variant_SideScrolling",
            "Ward_Zero/Variant_SideScrolling/AI",
            "Ward_Zero/Variant_SideScrolling/Gameplay",
            "Ward_Zero/Variant_SideScrolling/Interfaces",
            "Ward_Zero/Variant_SideScrolling/UI"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
