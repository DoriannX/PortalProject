// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PortalProject : ModuleRules
{
	public PortalProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
