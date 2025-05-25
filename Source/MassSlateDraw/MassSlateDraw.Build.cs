// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MassSlateDraw : ModuleRules
{
	public MassSlateDraw(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        
		PublicDependencyModuleNames.AddRange(new string[] { "StructUtils", "MassCommon", "MassEntity", "MassSpawner" });

		PublicDependencyModuleNames.AddRange(new string[] { "SlateCore", "Slate", "UMG" });
		
		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
    }
}
