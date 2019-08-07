// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGeneratorEdModeToolkit.h"
#include "LandscapeFilter.h"
#include "RecipeForTerrain.h"
#include "LandscapeFilterFactory.h"
#include "ModuleManager.h"
#include "IContentBrowserSingleton.h"
#include "KismetEditorUtilities.h"
#include "ContentBrowserModule.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Engine/Selection.h"
#include "Engine/StaticMeshActor.h"
#include "ComponentAssetBroker.h"
#include "BlueprintEditorUtils.h"
#include "AssetRegistryModule.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "FProceduralTerrainGeneratorEdModeToolkit"

FProceduralTerrainGeneratorEdModeToolkit::FProceduralTerrainGeneratorEdModeToolkit()
{
	FDetailsViewArgs DetailArgs;
	DetailArgs.bShowPropertyMatrixButton = false;
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	CurrentFilterDetails = PropertyModule.CreateDetailView(DetailArgs);
	OnFilterClassChanged(FilterClass);
}

void FProceduralTerrainGeneratorEdModeToolkit::OnFilterClassChanged(const UClass* NewClass)
{
	FilterClass = NewClass;
	CurrentManagedFilter = NewObject<ULandscapeFilter>(GetTransientPackage(), FilterClass->ClassDefaultObject->GetClass());
	CurrentFilterDetails->SetObject(CurrentManagedFilter);
}

TArray<ALandscape*> FProceduralTerrainGeneratorEdModeToolkit::GetSelectedLandscapeActors()
{
	TArray<ALandscape*> rval;
	GEditor->GetSelectedActors()->GetSelectedObjects<ALandscape>(rval);
	return rval;
}

void FProceduralTerrainGeneratorEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{	
	FModeToolkit::Init(InitToolkitHost);
}


TSharedPtr<class SWidget> FProceduralTerrainGeneratorEdModeToolkit::GetInlineContent() const
{
	TSharedPtr<SWidget> Content;
	SAssignNew(Content, SBorder)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(10.0f, 12.0f))
	[
		SNew(SScaleBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Stretch(EStretch::ScaleToFitX)
		.StretchDirection(EStretchDirection::DownOnly)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(0)
			[
				SNew(STextBlock)
				.AutoWrapText(false)
				.Justification(ETextJustify::Center)
				.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14, EFontHinting::AutoLight))
				.Text(LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.Title", "Landscape Procedural Terrain Generator"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(10, 10, 10, 0)
			[
				SNew(SClassPropertyEntryBox)
				.AllowAbstract(false)
				.AllowNone(false)
				.MetaClass(ULandscapeFilter::StaticClass())
				.SelectedClass_Lambda([this]
				{
					return FilterClass;
				})
				.OnSetClass(this, &FProceduralTerrainGeneratorEdModeToolkit::OnFilterClassChanged)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(0)
			[
				CurrentFilterDetails->AsShared()
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(10)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]
					{
						return CurrentManagedFilter != nullptr;
					})
					.OnClicked_Lambda([this]() -> FReply
					{
						IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();

						// Get path from dialog
						FSaveAssetDialogConfig SaveDialogConfig;
						SaveDialogConfig.DefaultPath = "/Game";
						SaveDialogConfig.DefaultAssetName = "NewRecipe";
						SaveDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;
						SaveDialogConfig.DialogTitleOverride = LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.SaveRecipeTitle", "Save New Terrain Recipe...");
						FString Path = ContentBrowser.CreateModalSaveAssetDialog(SaveDialogConfig);
						FString AbsolutePath;
						Path = FPaths::ChangeExtension(Path, ".uasset");
						FPackageName::TryConvertLongPackageNameToFilename(Path, AbsolutePath);
						
						UPackage* Package = CreatePackage(nullptr, *FEditorFileUtils::ExtractPackageName(Path));

						// Create new copies from filter objects to persist
						URecipeForTerrain* ToSave;
						if (CurrentManagedFilter->IsA(URecipeForTerrain::StaticClass()))
						{
							ToSave = Cast<URecipeForTerrain>(NewObject<URecipeForTerrain>(Package, CurrentManagedFilter->GetClass(), NAME_None, EObjectFlags::RF_Standalone, CurrentManagedFilter));
						}
						else
						{
							ToSave = NewObject<URecipeForTerrain>(Package, URecipeForTerrain::StaticClass(), NAME_None, EObjectFlags::RF_Standalone);
							ToSave->Filters.Add(NewObject<ULandscapeFilter>(Package, CurrentManagedFilter->GetClass(), NAME_None, EObjectFlags::RF_Standalone, CurrentManagedFilter));
						}
						ToSave->ApplyToLandscape = nullptr;

						// Create blueprint object to save
						UBlueprint* BlueprintToSave = FKismetEditorUtilities::CreateBlueprint(URecipeForTerrain::StaticClass(), Package, *FPaths::GetBaseFilename(Path), EBlueprintType::BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
						UObject* BlueprintCDO = BlueprintToSave->GeneratedClass->GetDefaultObject();
						const auto CopyOptions = (EditorUtilities::ECopyOptions::Type)(EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties | EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances);
						for (TFieldIterator<UProperty> PropIt(URecipeForTerrain::StaticClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
						{
							UProperty* Property = *PropIt;
							EditorUtilities::CopySingleProperty(ToSave, BlueprintCDO, Property);
						}

						// Actually save the blueprint
						FAssetRegistryModule::AssetCreated(BlueprintToSave);
						FAssetRegistryModule::AssetCreated(ToSave);
						BlueprintToSave->MarkPackageDirty();

						return FReply::Handled();
					})
					.Text(LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.SaveRecipe", "Save Recipe"))
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]
					{
						return GetSelectedLandscapeActors().Num() > 0 && CurrentManagedFilter != nullptr;
					})
					.OnClicked_Lambda([this]() -> FReply
					{
						FString FilterName = CurrentManagedFilter->GetClass()->GetName();
						UE_LOG(ProceduralTerrainGenerator, Log, TEXT("Applying filter %s"), *FilterName);
						for (auto Landscape : GetSelectedLandscapeActors())
						{
							ULandscapeFilter::ApplyFilterStatic(CurrentManagedFilter, Landscape);
						}

						return FReply::Handled();
					})
					.Text(LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.ActivateFilter", "Activate Filter"))
				]
			]
		]
	];

	return Content;
}

#undef LOCTEXT_NAMESPACE
