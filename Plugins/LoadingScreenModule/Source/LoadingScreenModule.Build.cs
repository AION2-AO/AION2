// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LoadingScreenModule : ModuleRules
{
    public LoadingScreenModule(ReadOnlyTargetRules Target) : base(Target)
    {
        // Specify the primary header file for precompiled headers.
        PrivatePCHHeaderFile = "Private/LoadingScreenModule.h";

        // Use shared precompiled headers for better compile times.
        PCHUsage = PCHUsageMode.UseSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",         // Core UE functionality
            "CoreUObject",  // Base UObject system.
            "MoviePlayer",  // Movie player for loading screens.
            "Engine",       // Main UE Engine module for UITexture2D
            "Slate",        // UI framework
            "SlateCore",     // Core Slate functionality
			"DeveloperSettings" // For use it easily in the Project Setting
        });
    }
}
