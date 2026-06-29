// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AION2 : ModuleRules
{
    public AION2(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        IWYUSupport = IWYUSupport.Full;
        bUseUnity = false;
        bUseRTTI = true;
        bEnableExceptions = true;

        PublicIncludePaths.AddRange(new string[] { "AION2" });

        PrivateDependencyModuleNames.AddRange(new string[] { "ProtobufCore" });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "Networking",
            "Sockets",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "Niagara",
            "UMG",
            "NavigationSystem",
            "Navmesh",
            "HTTP",
            "StateTreeModule",
            "GameplayStateTreeModule"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore"
        });

        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Off;

        // PrivateDependencyModuleNames.Add("OnlineSubsystem");
        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}