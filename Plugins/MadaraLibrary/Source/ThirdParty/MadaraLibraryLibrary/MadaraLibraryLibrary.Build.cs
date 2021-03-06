// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class MadaraLibraryLibrary : ModuleRules
{
  public MadaraLibraryLibrary(ReadOnlyTargetRules Target) : base(Target)
  {
    Type = ModuleType.External;
    
    string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory,
      "..", "..", "..", "..", ".."));
    string BaseLibDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory,
      "..", "..", ".."));
    string SourceDirectory = Path.Combine(BaseLibDirectory, "Source",
      "ThirdParty");
    string MadaraLibDirectory = Path.Combine(BaseLibDirectory, "ThirdParty",
      "madara", Target.Platform.ToString());

    System.Console.WriteLine("MadaraLibrary paths ");
    System.Console.WriteLine("BaseDir = " + BaseDirectory);
    System.Console.WriteLine("Baselibdir = " + BaseLibDirectory);
    System.Console.WriteLine("SourceDirectory = " + SourceDirectory);
    System.Console.WriteLine("MadaraLibDirectory = " + MadaraLibDirectory);
    
    System.Console.WriteLine("Target.Type  " + Target.Type);
    System.Console.WriteLine("Target.Configuration  " + Target.Configuration);
    System.Console.WriteLine("Target.Platform  " + Target.Platform);

    PublicIncludePaths.AddRange(
      new string[] {
              SourceDirectory,
      });

    //PublicLibraryPaths.Add(MadaraLibDirectory);

    PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS=1");
    PublicDefinitions.Add("MADARA_NO_THREAD_LOCAL=1");
    PublicDefinitions.Add("MADARA_FEATURE_SIMTIME=1");
    PublicDefinitions.Add("_USE_MATH_DEFINES=1");
    PublicDefinitions.Add("BOOST_ALL_NO_LIB=1");
    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS=1");

    bUseRTTI = false;
    bEnableExceptions = true;

    if (Target.Platform == UnrealTargetPlatform.Win64)
    {
      string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "Win64");

      System.IO.File.Copy(Path.Combine(MadaraLibDirectory, "madara.dll"),
        Path.Combine(BinariesDir, "madara.dll"), true);

      System.IO.File.Copy(Path.Combine(MadaraLibDirectory, "madara.lib"),
        Path.Combine(BinariesDir, "madara.lib"), true);

      PublicAdditionalLibraries.Add(Path.Combine(MadaraLibDirectory, "madara.lib"));
      PublicDelayLoadDLLs.Add(Path.Combine(MadaraLibDirectory, "madara.dll"));
      RuntimeDependencies.Add(Path.Combine(BinariesDir, "madara.dll"));
    }
    else if (Target.Platform == UnrealTargetPlatform.Mac)
    {
      string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "Mac");

      System.IO.File.Copy(Path.Combine(MadaraLibDirectory, "libmadara.dylib"),
        Path.Combine(BinariesDir, "libmadara.dylib"), true);
      PublicAdditionalLibraries.Add(Path.Combine(MadaraLibDirectory, "libmadara.dylib"));
      RuntimeDependencies.Add(Path.Combine(BinariesDir, "libmadara.dylib"));
    }
    else if (Target.Platform == UnrealTargetPlatform.Linux)
    {
      string BinariesDir = Path.Combine(BaseDirectory, "Binaries", "Linux");

      System.IO.File.Copy(Path.Combine(MadaraLibDirectory, "libmadara.so"),
        Path.Combine(BinariesDir, "libmadara.so"), true);
      PublicAdditionalLibraries.Add(Path.Combine(MadaraLibDirectory, "libmadara.so"));
      RuntimeDependencies.Add(Path.Combine(BinariesDir, "libmadara.so"));
    }
  }
}
