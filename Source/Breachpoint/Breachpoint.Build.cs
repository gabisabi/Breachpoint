// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Breachpoint : ModuleRules
{
	public Breachpoint(ReadOnlyTargetRules Target) : base(Target)
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
			"SlateCore",
			"HeadMountedDisplay",
			"NavigationSystem",
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Breachpoint",
			"Breachpoint/Variant_Horror",
			"Breachpoint/Variant_Horror/UI",
			"Breachpoint/Variant_Shooter",
			"Breachpoint/Variant_Shooter/AI",
			"Breachpoint/Variant_Shooter/UI",
			"Breachpoint/Variant_Shooter/Weapons",
			"Breachpoint/Variant_Shooter/Modes",
			"Breachpoint/Variant_Shooter/Movement",
			"Breachpoint/VR",
			"Breachpoint/Lobby",
			"Breachpoint/BodyCam"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
