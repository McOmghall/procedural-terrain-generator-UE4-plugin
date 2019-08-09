// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MultiBoxBuilder.h"
#include "Modules/ModuleManager.h"
#include "Landscape.h"
#include "LandscapeFilter.h"

DECLARE_LOG_CATEGORY_EXTERN(ProceduralTerrainGenerator, Log, All);

class FProceduralTerrainGeneratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/** end IModuleInterface implementation */

	static void AddMenuEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	static void FillSubmenuApplyFilters(FMenuBuilder& MenuBuilder, TArray<ULandscapeFilter*> LandscapeFilters);
	static void ApplyFiltersToLandscape(TArray<ULandscapeFilter*> LandscapeFilters, ALandscape* Landscape);
	static void CreateChildBPFromFilter(ULandscapeFilter* Filter);
};
