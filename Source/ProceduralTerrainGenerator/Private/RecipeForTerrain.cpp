// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeForTerrain.h"
#include "FlattenFilter.h"

#define LOCTEXT_NAMESPACE "FProceduralTerrainGeneratorEdModeToolkit"

URecipeForTerrain::URecipeForTerrain(const FObjectInitializer& Init)
{
}

void URecipeForTerrain::ApplyRecipe()
{
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

		UE_LOG(ProceduralTerrainGenerator, Log, TEXT("Applying filter %s to Landscape %s"), *FilterName, *(ApplyToLandscape->GetName()));
		Filter->ApplyFilter(ApplyToLandscape, RandomStream);
	}
}

bool URecipeForTerrain::ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream)
{
	ApplyToLandscape = Landscape;
	RandomSeed = RandomStream->GetCurrentSeed();
	ApplyRecipe();
	return true;
}
