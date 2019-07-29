// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeForTerrain.h"

void URecipeForTerrain::ApplyRecipe()
{

	const FText TransactionTitle = LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.UndoRedoApplyFiltersName", "Apply Filter");
	const FString TransactionNamespace = "ProceduralTerrainTool";
	FScopedSlowTask Progress(Filters.Num(), LOCTEXT("FProceduralTerrainGeneratorEdModeToolkit.ProgressDialogTitle", "Appliying filters..."));
	Progress.MakeDialog(true, true);

	FRandomStream* RandomStream = new FRandomStream(RandomSeed);

	for (ULandscapeFilter* Filter : Filters)
	{
		if (Filter == nullptr)
		{
			Progress.EnterProgressFrame();
			UE_LOG(ProceduralTerrainGenerator, Warning, TEXT("Found null filter in recipe"));
			continue;
		}
		FString FilterName = Filter->GetClass()->GetName();
		Progress.EnterProgressFrame(1, FText::FromString(FilterName));
		UE_LOG(ProceduralTerrainGenerator, Log, TEXT("Applying filter %s"), *FilterName);

		GEditor->BeginTransaction(*TransactionNamespace, TransactionTitle, ApplyToLandscape);
		UE_LOG(ProceduralTerrainGenerator, Log, TEXT("Applying filter %s to Landscape %s"), *FilterName, *(ApplyToLandscape->GetName()));
		Filter->ApplyFilter(ApplyToLandscape, RandomStream);
		GEditor->EndTransaction();
	}
}

bool URecipeForTerrain::ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream)
{
	ApplyToLandscape = Landscape;
	ApplyRecipe();
	return true;
}
