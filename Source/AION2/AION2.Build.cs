// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AION2 : ModuleRules
{
    public AION2(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        string ProjectRoot = Target.ProjectFile.Directory.ToString();
        string ProtobufPath = Path.Combine(ProjectRoot, "Protobuf");
        string ProtobufIncludePath = Path.Combine(ProtobufPath, "include");
        string ProtobufLibPath = Path.Combine(ProtobufPath, "lib");

        PublicSystemIncludePaths.Add(ProtobufIncludePath);
        PublicIncludePaths.Add(ProtobufIncludePath);
        PublicAdditionalLibraries.Add(Path.Combine(ProtobufLibPath, "libprotobuf.lib"));
        if (Directory.Exists(ProtobufLibPath))
        {
            string[] LibFiles = Directory.GetFiles(ProtobufLibPath, "*.lib");
            foreach (string LibFile in LibFiles)
            {
                PublicAdditionalLibraries.Add(LibFile);
            }
        }

        PublicIncludePaths.AddRange(new string[] { "AION2" });
        PublicDefinitions.AddRange(
           new string[]
           {
                "GOOGLE_PROTOBUF_NO_RTTI=1",
                "ABSL_FLAGS_STRIP_NAMES=1"
           });

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
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore"
        });


        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // 1. �ҽ� �ڵ尡 �ִ� Common ���� ��� ����
            string CommonBinariesDir = Path.Combine(ProjectRoot, "Common", "bin");

            string TargetBinariesDir = Path.Combine(ModuleDirectory, "..", "..", "Binaries", "Win64");
            if (Directory.Exists(CommonBinariesDir))
            {
                string[] DllFiles = Directory.GetFiles(CommonBinariesDir, "*.dll");

                foreach (string SrcDllPath in DllFiles)
                {
                    string DllName = Path.GetFileName(SrcDllPath);
                    string TgtDllPath = Path.Combine(TargetBinariesDir, DllName);

                    RuntimeDependencies.Add(TgtDllPath, SrcDllPath);

                    if (!Directory.Exists(TargetBinariesDir))
                    {
                        Directory.CreateDirectory(TargetBinariesDir);
                    }

                    if (!File.Exists(TgtDllPath) || File.GetLastWriteTime(SrcDllPath) != File.GetLastWriteTime(TgtDllPath))
                    {
                        File.Copy(SrcDllPath, TgtDllPath, true);
                    }
                }
            }
        }

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}