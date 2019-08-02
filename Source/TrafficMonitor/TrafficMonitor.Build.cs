// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TrafficMonitor : ModuleRules
{
	public TrafficMonitor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"C:/Users/ak/Downloads/clingo/libclingo"
			}
			);

    PublicSystemIncludePaths.AddRange(
		new string[] {
			// ... add other private include paths required here ...
			//"C:/Users/ak/Downloads/clingo/libclingo"
		}
		);
		PublicLibraryPaths.AddRange(
		new string[] {
			// List of system/library paths (directory of .lib files)
			//"C:/Users/ak/Downloads/clingo-Build/lib/Debug"
		}
		);

    PublicAdditionalLibraries.Add(@"C:/Users/ak/Downloads/clingo-Build/lib/Debug/import_clingo.lib");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"Carla"
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...
				"Carla", "PhysXVehicles"
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		//PrivateIncludePathModuleNames.AddRange(new string[] { "Carla" });
		//PublicIncludePathModuleNames.AddRange(new string[] { "Carla" });
	}
}
