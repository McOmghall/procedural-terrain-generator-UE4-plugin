// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGenerator.h"
#include "ProceduralTerrainGeneratorEdMode.h"
#include "MultiBoxExtender.h"
#include "MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "FProceduralTerrainGeneratorModule"

DEFINE_LOG_CATEGORY(ProceduralTerrainGenerator);


FContentBrowserMenuExtender_SelectedAssets ApplyFilterMenuExtender;

void FProceduralTerrainGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FProceduralTerrainGeneratorEdMode>(FProceduralTerrainGeneratorEdMode::EM_ProceduralTerrainGeneratorEdModeId, LOCTEXT("ProceduralTerrainGeneratorEdModeName", "ProceduralTerrainGeneratorEdMode"), FSlateIcon(), true);

	// Register Content Browser context menu extensions 
	ApplyFilterMenuExtender = FContentBrowserMenuExtender_SelectedAssets::CreateLambda([this](const TArray<FAssetData>& SelectedAssets) -> TSharedRef<FExtender>
	{
		TSharedPtr<FExtender> ExtensionForContentBrowser = MakeShareable(new FExtender);
		ExtensionForContentBrowser->AddMenuExtension(FName("CommonAssetActions"), EExtensionHook::After, TSharedPtr<FUICommandList>(), FMenuExtensionDelegate::CreateStatic(&FProceduralTerrainGeneratorModule::AddMenuEntry, SelectedAssets));
		return ExtensionForContentBrowser.ToSharedRef();
	});
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(ApplyFilterMenuExtender);
}

void FProceduralTerrainGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FProceduralTerrainGeneratorEdMode::EM_ProceduralTerrainGeneratorEdModeId);
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
}


void FProceduralTerrainGeneratorModule::AddMenuEntry(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	// Create Section
	MenuBuilder.BeginSection("CustomMenu", TAttribute<FText>(FText::FromString("Procedural Terrain Generator")));
	{
		TArray<ULandscapeFilter*> Filters;

		for (FAssetData Asset : SelectedAssets)
		{
			if (UBlueprint* BP = Cast<UBlueprint>(Asset.GetAsset()))
			{
				if (ULandscapeFilter* Filter = Cast<ULandscapeFilter>(BP->GeneratedClass->GetDefaultObject(true)))
				{
					Filters.Add(Filter);
				}
			}
		}

		if (Filters.Num() > 0)
		{
			MenuBuilder.AddSubMenu(FText::FromString("Apply Filters/Recipes"), FText::FromString("Apply these filters to the selected Landscape actors"), FNewMenuDelegate::CreateStatic(&FProceduralTerrainGeneratorModule::FillSubmenu, Filters));
		}
	}
	MenuBuilder.EndSection();
}

void FProceduralTerrainGeneratorModule::FillSubmenu(FMenuBuilder& MenuBuilder, TArray<ULandscapeFilter*> LandscapeFilters)
{
	TArray<ALandscape*> Landscapes;

	for (TActorIterator<ALandscape> It(GWorld, ALandscape::StaticClass()); It; ++It)
	{
		ALandscape* Actor = *It;
		if (Actor && !Actor->IsPendingKill())
		{
			Landscapes.Add(Actor);
		}
	}

	if (Landscapes.Num() <= 0)
	{
		MenuBuilder.AddMenuEntry(TAttribute<FText>(LOCTEXT("FProceduralTerrainGeneratorModule.NoLandscapes", "There are no landscapes in the opened world")), TAttribute<FText>(LOCTEXT("FProceduralTerrainGeneratorModule.NoLandscapes", "There are no landscapes in the opened world")), FSlateIcon(), FUIAction());
	}

	for (ALandscape* Landscape : Landscapes)
	{
		MenuBuilder.AddMenuEntry(FText::FromString(Landscape->GetName()), FText::FromString(Landscape->GetName()), FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&FProceduralTerrainGeneratorModule::ApplyFiltersToLandscape, LandscapeFilters, Landscape)));
	}
}

void FProceduralTerrainGeneratorModule::ApplyFiltersToLandscape(TArray<ULandscapeFilter*> LandscapeFilters, ALandscape* Landscape)
{
	FRandomStream* Stream = new FRandomStream(0xCAFE1EAD);
	for (ULandscapeFilter* Filter : LandscapeFilters)
	{
		Filter->ApplyFilter(Landscape, Stream);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProceduralTerrainGeneratorModule, ProceduralTerrainGenerator)