// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGenerator.h"
#include "ProceduralTerrainGeneratorEdMode.h"

#define LOCTEXT_NAMESPACE "FProceduralTerrainGeneratorModule"

DEFINE_LOG_CATEGORY(ProceduralTerrainGenerator);

void FProceduralTerrainGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FProceduralTerrainGeneratorEdMode>(FProceduralTerrainGeneratorEdMode::EM_ProceduralTerrainGeneratorEdModeId, LOCTEXT("ProceduralTerrainGeneratorEdModeName", "ProceduralTerrainGeneratorEdMode"), FSlateIcon(), true);
}

void FProceduralTerrainGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FProceduralTerrainGeneratorEdMode::EM_ProceduralTerrainGeneratorEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProceduralTerrainGeneratorModule, ProceduralTerrainGenerator)