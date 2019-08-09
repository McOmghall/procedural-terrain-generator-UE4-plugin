// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGenerator.h"
#include "ProceduralTerrainGeneratorEdMode.h"
#include "RecipeForTerrain.h"
#include "FileHelper.h"
#include "MultiBoxExtender.h"
#include "MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "ModuleManager.h"
#include "IContentBrowserSingleton.h"
#include "KismetEditorUtilities.h"
#include "ContentBrowserModule.h"
#include "ComponentAssetBroker.h"
#include "BlueprintEditorUtils.h"
#include "AssetRegistryModule.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"

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
	// Get selected filters
	TArray<ULandscapeFilter*> Filters;
	for (FAssetData Asset : SelectedAssets)
	{
		UObject* AssetObject = Asset.GetAsset();
		if (AssetObject->IsA(ULandscapeFilter::StaticClass()))
		{
			Filters.Add(Cast<ULandscapeFilter>(AssetObject));
		}
		else if (AssetObject->IsA(UBlueprint::StaticClass()))
		{
			if (ULandscapeFilter* Filter = Cast<ULandscapeFilter>(Cast<UBlueprint>(AssetObject)->GeneratedClass->GetDefaultObject(true)))
			{
				Filters.Add(Filter);
			}
		}
		else if (TSubclassOf<ULandscapeFilter> AssetObjectClass = Cast<UClass>(AssetObject))
		{
			if (AssetObjectClass->IsChildOf(ULandscapeFilter::StaticClass()))
			{
				Filters.Add(Cast<ULandscapeFilter>(AssetObjectClass->ClassDefaultObject));
			}
		}
	}

	// Create Section
	MenuBuilder.BeginSection("PTG_Section", TAttribute<FText>(FText::FromString("Procedural Terrain Generator")));
	{
		if (Filters.Num() > 0)
		{
			// Apply filter to terrains in current opened world section
			MenuBuilder.AddSubMenu(FText::FromString("Apply Filters/Recipes"), FText::FromString("Apply these filters to the selected Landscape actors"), FNewMenuDelegate::CreateStatic(&FProceduralTerrainGeneratorModule::FillSubmenuApplyFilters, Filters));
			
			// Create new filter from first selected filter
			MenuBuilder.AddMenuEntry(FText::FromString("Create BP child from" + Filters[0]->GetName()), TAttribute<FText>(LOCTEXT("FProceduralTerrainGeneratorModule.CreateChildTooltip", "Make a data blueprint to customize filter params")), FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&FProceduralTerrainGeneratorModule::CreateChildBPFromFilter, Filters[0])));
		}
	}
	MenuBuilder.EndSection();
}

void FProceduralTerrainGeneratorModule::FillSubmenuApplyFilters(FMenuBuilder& MenuBuilder, TArray<ULandscapeFilter*> LandscapeFilters)
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
		MenuBuilder.AddMenuEntry(TAttribute<FText>(LOCTEXT("FProceduralTerrainGeneratorModule.NoLandscapes", "There are no landscapes in the opened world")), TAttribute<FText>(LOCTEXT("FProceduralTerrainGeneratorModule.NoLandscapes", "There are no landscapes in the opened world")), FSlateIcon(), FUIAction(), NAME_None, EUserInterfaceActionType::None);
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
		const FText TransactionTitle = LOCTEXT("FProceduralTerrainGeneratorModule.UndoRedoApplyFiltersName", "Apply Recipe");
		const FString TransactionNamespace = "ProceduralTerrainTool";
		GEditor->BeginTransaction(*TransactionNamespace, TransactionTitle, Landscape);
		Filter->ApplyFilter(Landscape, Stream);
		GEditor->EndTransaction();
	}
}

void FProceduralTerrainGeneratorModule::CreateChildBPFromFilter(ULandscapeFilter* Filter)
{
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();

	// Get path from dialog
	FSaveAssetDialogConfig SaveDialogConfig;
	SaveDialogConfig.DefaultPath = "/Game";
	SaveDialogConfig.DefaultAssetName = "NewRecipe";
	SaveDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;
	SaveDialogConfig.DialogTitleOverride = LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.SaveRecipeTitle", "Save New Terrain Recipe...");
	FString Path = ContentBrowser.CreateModalSaveAssetDialog(SaveDialogConfig);

	if (!Path.IsEmpty())
	{
		FString AbsolutePath;
		Path = FPaths::ChangeExtension(Path, ".uasset");
		FPackageName::TryConvertLongPackageNameToFilename(Path, AbsolutePath);

		UPackage* Package = CreatePackage(nullptr, *FEditorFileUtils::ExtractPackageName(Path));

		// Create new copies from filter objects to persist
		URecipeForTerrain* ToSave;
		FObjectDuplicationParameters DuplicationParams(Filter, Package);
		DuplicationParams.ApplyFlags = EObjectFlags::RF_Standalone;
		DuplicationParams.FlagMask = EObjectFlags::RF_Standalone;
		DuplicationParams.DuplicateMode = EDuplicateMode::Normal;
		ULandscapeFilter* ManagedFilter = Cast<ULandscapeFilter>(StaticDuplicateObjectEx(DuplicationParams));
		if (Filter->IsA(URecipeForTerrain::StaticClass()))
		{
			ToSave = Cast<URecipeForTerrain>(ManagedFilter);
		}
		else
		{
			ToSave = NewObject<URecipeForTerrain>(Package, URecipeForTerrain::StaticClass(), NAME_None, EObjectFlags::RF_Standalone);
			ToSave->Filters.Add(ManagedFilter);
		}
		ToSave->ApplyToLandscape = nullptr;
		ToSave->AddToRoot();

		// Create blueprint object to save
		UBlueprint* BlueprintToSave = FKismetEditorUtilities::CreateBlueprint(URecipeForTerrain::StaticClass(), Package, *FPaths::GetBaseFilename(Path), EBlueprintType::BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
		UObject* CDO = BlueprintToSave->GeneratedClass->GetDefaultObject();
		UEngine::FCopyPropertiesForUnrelatedObjectsParams CopyPropertiesParams;
		CopyPropertiesParams.bAggressiveDefaultSubobjectReplacement = true;
		CopyPropertiesParams.bClearReferences = true;
		CopyPropertiesParams.bDoDelta = false;
		CopyPropertiesParams.bReplaceObjectClassReferences = true;
		UEngine::CopyPropertiesForUnrelatedObjects(ToSave, CDO, CopyPropertiesParams);

		// Actually save the blueprint
		FAssetRegistryModule::AssetCreated(BlueprintToSave);
		FAssetRegistryModule::AssetCreated(ToSave);
		BlueprintToSave->MarkPackageDirty();
		TArray<UObject*> Focus;
		Focus.Add(BlueprintToSave);
		GEditor->SyncBrowserToObjects(Focus, true);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProceduralTerrainGeneratorModule, ProceduralTerrainGenerator)