// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGenerator.h"
#include "RecipeForTerrain.h"
#include "FileHelper.h"
#include "MultiBoxExtender.h"
#include "MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "LevelEditor.h"
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


FContentBrowserMenuExtender_SelectedAssets ContentBrowserContextMenuExtender;
FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors LevelEditorContextMenuExtender;

void FProceduralTerrainGeneratorModule::StartupModule()
{
	// Register Content Browser context menu extensions 
	ContentBrowserContextMenuExtender = FContentBrowserMenuExtender_SelectedAssets::CreateLambda([this](const TArray<FAssetData>& SelectedAssets) -> TSharedRef<FExtender>
	{
		TSharedPtr<FExtender> ExtensionForContentBrowser = MakeShareable(new FExtender);
		ExtensionForContentBrowser->AddMenuExtension(FName("CommonAssetActions"), EExtensionHook::After, TSharedPtr<FUICommandList>(), FMenuExtensionDelegate::CreateStatic(&FProceduralTerrainGeneratorModule::AddMenuEntry, SelectedAssets));
		return ExtensionForContentBrowser.ToSharedRef();
	});
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(ContentBrowserContextMenuExtender);

	// Register Level Editor context menu extensions 
	LevelEditorContextMenuExtender = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateLambda([this](const TSharedRef<FUICommandList> Commands, const TArray<AActor*>& SelectedAssets) -> TSharedRef<FExtender>
	{
		TSharedPtr<FExtender> ExtensionForLevelEditor = MakeShareable(new FExtender);
		ExtensionForLevelEditor->AddMenuExtension(FName("ActorControl"), EExtensionHook::After, TSharedPtr<FUICommandList>(), FMenuExtensionDelegate::CreateStatic(&FProceduralTerrainGeneratorModule::AddLevelMenuEntry, SelectedAssets));
		return ExtensionForLevelEditor.ToSharedRef();
	});
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetAllLevelViewportContextMenuExtenders().Add(LevelEditorContextMenuExtender);
}

void FProceduralTerrainGeneratorModule::ShutdownModule()
{
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
			MenuBuilder.AddMenuEntry(FText::FromString("Create BP Recipe from " + Filters[0]->GetName()), TAttribute<FText>(LOCTEXT("FProceduralTerrainGeneratorModule.CreateChildTooltip", "Make a data blueprint to customize filter params")), FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&FProceduralTerrainGeneratorModule::CreateChildBPFromFilter, Filters[0])));
		}
	}
	MenuBuilder.EndSection();
}

void FProceduralTerrainGeneratorModule::AddLevelMenuEntry(FMenuBuilder& MenuBuilder, TArray<AActor*> SelectedActors)
{
	// Create Section
	MenuBuilder.BeginSection("PTG_Section_LevelEditor", TAttribute<FText>(FText::FromString("Procedural Terrain Generator")));
	{
		TArray<ALandscape*> SelectedLandscapes;
		for (AActor* Actor : SelectedActors)
		{
			if (ALandscape* Landscape = Cast<ALandscape>(Actor))
			{
				SelectedLandscapes.Add(Landscape);
			}
		}

		if (SelectedLandscapes.Num() > 0)
		{
			MenuBuilder.AddSubMenu(FText::FromString("Apply Filters/Recipes"), FText::FromString("Apply these filters to the selected Landscape actors"), FNewMenuDelegate::CreateLambda([SelectedLandscapes](FMenuBuilder& MenuBuilder) {
				for (TObjectIterator<UClass> It; It; ++It)
				{    // Ignore deprecated
					if (It->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Abstract) || FKismetEditorUtilities::IsClassABlueprintSkeleton(*It) || !It->IsChildOf(ULandscapeFilter::StaticClass()))
					{
						continue;
					}

					TArray<ULandscapeFilter*> AuxFilters;
					AuxFilters.Add(Cast<ULandscapeFilter>(It->GetDefaultObject()));
					MenuBuilder.AddMenuEntry(FText::FromName(It->GetFName()), FText::FromString(It->GetName()), FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&FProceduralTerrainGeneratorModule::ApplyFiltersToLandscapes, AuxFilters, SelectedLandscapes)));
				}
			}));
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
		TArray<ALandscape*> AuxLandscapes;
		AuxLandscapes.Add(Landscape);
		MenuBuilder.AddMenuEntry(FText::FromString(Landscape->GetName()), FText::FromString(Landscape->GetName()), FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&FProceduralTerrainGeneratorModule::ApplyFiltersToLandscapes, LandscapeFilters, AuxLandscapes)));
	}
}

void FProceduralTerrainGeneratorModule::ApplyFiltersToLandscapes(TArray<ULandscapeFilter*> LandscapeFilters, TArray<ALandscape*> Landscapes)
{
	FRandomStream* Stream = new FRandomStream(0xCAFE1EAD);
	for (ULandscapeFilter* Filter : LandscapeFilters)
	{
		for (ALandscape* Landscape : Landscapes)
		{
			const FText TransactionTitle = LOCTEXT("FProceduralTerrainGeneratorModule.UndoRedoApplyFiltersName", "Apply Recipe");
			const FString TransactionNamespace = "ProceduralTerrainTool";
			GEditor->BeginTransaction(*TransactionNamespace, TransactionTitle, Landscape);
			Filter->ApplyFilter(Landscape, Stream);
			GEditor->EndTransaction();
		}
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