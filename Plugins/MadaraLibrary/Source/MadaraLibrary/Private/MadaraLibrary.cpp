// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MadaraLibrary.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include "MadaraIncludes.h"

#define LOCTEXT_NAMESPACE "FMadaraLibraryModule"

void FMadaraLibraryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	const FString BaseDir = IPluginManager::Get().FindPlugin(
    "MadaraLibrary")->GetBaseDir();
  const FString MadaraDir = FPaths::Combine (
    *BaseDir, TEXT ("ThirdParty"), TEXT ("madara"));

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*MadaraDir, TEXT("Win64"), TEXT("MADARA.dll"));
#elif PLATFORM_MAC
  LibraryPath = FPaths::Combine (*MadaraDir, TEXT ("Mac"), TEXT ("MADARA.dylib"));
#endif // PLATFORM_WINDOWS

	ExampleLibraryHandle = !LibraryPath.IsEmpty() ?
    FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (ExampleLibraryHandle)
	{
    //FMessageDialog::Open (EAppMsgType::Ok,
    //  LOCTEXT ("MADARA LOAD SUCCESS",
    //    "MadaraLibrary: Found MADARA library"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok,
      LOCTEXT("MADARA LOAD FAILURE",
        "MadaraLibrary: Couldn't find MADARA library :("));
	}

  //madara::knowledge::KnowledgeBase kb;

  //kb.set ("bob", "bobtext");

  //std::string kb_text ("MadaraLibrary: KB LOAD TEST\nbob: ");
  //kb_text += kb.get ("bob").to_string ().c_str ();
  //FString kb_text_f (kb_text.c_str());

  //FMessageDialog::Open (EAppMsgType::Ok,
  //  FText::FromString (kb_text_f));
}

void FMadaraLibraryModule::ShutdownModule()
{
	// Free the dll handle
	FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
	ExampleLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMadaraLibraryModule, MadaraLibrary)
