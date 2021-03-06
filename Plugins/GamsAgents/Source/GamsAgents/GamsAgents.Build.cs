// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GamsAgents : ModuleRules
{
  public GamsAgents(ReadOnlyTargetRules Target) : base(Target)
  {
    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    bUseRTTI = false;
    bEnableExceptions = true;

    PublicIncludePaths.AddRange(
      new string[] {
        // ... add public include paths required here ...
      }
    );
        
    
    PrivateIncludePaths.AddRange(
      new string[] {
        // ... add other private include paths required here ...
      }
    );
      
    
    PublicDependencyModuleNames.AddRange(
      new string[]
      {
        "Core",
        "CoreUObject",
        "Engine",
        "Slate",
        "SlateCore",
        "GamsLibrary",
        "GamsLibraryLibrary",
        "MadaraLibraryLibrary",
        "MoviePlayer",
        "InputCore",
        "RenderCore",
        "ImageWrapper",
        "CinematicCamera",
        "Projects", // Support IPluginManager
        // ... add other public dependencies that you statically link with here ...
      }
    );
      
    
    PrivateDependencyModuleNames.AddRange(
      new string[]
      {
        // ... add private dependencies that you statically link with here ...  
      }
    );
    
    
    DynamicallyLoadedModuleNames.AddRange(
      new string[]
      {
        "Renderer"
      }
    );
  }
}
