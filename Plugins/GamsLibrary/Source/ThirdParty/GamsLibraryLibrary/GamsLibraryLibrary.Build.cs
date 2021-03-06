// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class GamsLibraryLibrary : ModuleRules
{
  public GamsLibraryLibrary(ReadOnlyTargetRules Target) : base(Target)
  {
    Type = ModuleType.External;

    PublicDependencyModuleNames.AddRange(
      new string[]
      {
        "MadaraLibraryLibrary"
        // ... add other public dependencies that you statically link with here ...
      }
    );

    PublicIncludePaths.AddRange(
      new string[] {
              "ThirdParty",
      });

    PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS=1");
    PublicDefinitions.Add("MADARA_NO_THREAD_LOCAL=1");
    PublicDefinitions.Add("MADARA_FEATURE_SIMTIME=1");
    PublicDefinitions.Add("_USE_MATH_DEFINES=1");
    PublicDefinitions.Add("BOOST_ALL_NO_LIB=1");
    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS=1");

    bUseRTTI = false;
    bEnableExceptions = true;

    string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory,
      "..", "..", "..", "..", ".."));
    string BaseLibDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory,
      "..", "..", ".."));
    string SourceDirectory = Path.Combine(BaseLibDirectory, "Source",
      "ThirdParty");
    string GamsLibDirectory = Path.Combine(BaseLibDirectory, "ThirdParty",
      "gams", Target.Platform.ToString());

    System.Console.WriteLine("GamsLibrary paths ");
    System.Console.WriteLine("BaseDir = " + BaseDirectory);
    System.Console.WriteLine("Baselibdir = " + BaseLibDirectory);
    System.Console.WriteLine("SourceDirectory = " + SourceDirectory);
    System.Console.WriteLine("GamsLibDirectory = " + GamsLibDirectory);
    
    System.Console.WriteLine("Target.Type  " + Target.Type);
    System.Console.WriteLine("Target.Configuration  " + Target.Configuration);
    System.Console.WriteLine("Target.Platform  " + Target.Platform);

    if (Target.Platform == UnrealTargetPlatform.Win64)
    {
      string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "Win64");

      System.IO.File.Copy(Path.Combine(GamsLibDirectory, "gams.dll"),
        Path.Combine(BinariesDir, "gams.dll"), true);

      System.IO.File.Copy(Path.Combine(GamsLibDirectory, "gams.lib"),
        Path.Combine(BinariesDir, "gams.lib"), true);

      PublicAdditionalLibraries.Add(Path.Combine(GamsLibDirectory, "gams.lib"));
      PublicDelayLoadDLLs.Add(Path.Combine(GamsLibDirectory, "gams.dll"));
      RuntimeDependencies.Add(Path.Combine(BinariesDir, "gams.dll"));
    }
    else if (Target.Platform == UnrealTargetPlatform.Mac)
    {
      PublicAdditionalLibraries.Add(Path.Combine(GamsLibDirectory, "libgams.dylib"));

      string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "Mac");

      System.IO.File.Copy(Path.Combine(GamsLibDirectory, "libgams.dylib"),
        Path.Combine(BinariesDir, "libgams.dylib"), true);
      RuntimeDependencies.Add(Path.Combine(BinariesDir, "libgams.dylib"));
    }
    else if (Target.Platform == UnrealTargetPlatform.Linux)
    {
      PublicAdditionalLibraries.Add(Path.Combine(GamsLibDirectory, "libgams.so"));

      string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "Linux");

      System.IO.File.Copy(Path.Combine(GamsLibDirectory, "libgams.so"),
        Path.Combine(BinariesDir, "libgams.so"), true);
      RuntimeDependencies.Add(Path.Combine(BinariesDir, "libgams.so"));
    }
  }
}
