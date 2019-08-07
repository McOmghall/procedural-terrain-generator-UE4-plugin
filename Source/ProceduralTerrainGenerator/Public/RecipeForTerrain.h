// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralTerrainGenerator.h"
#include "LandscapeFilter.h"
#include "UnrealEd.h"
#include "RecipeForTerrain.generated.h"


UCLASS(Blueprintable, BlueprintType)
class PROCEDURALTERRAINGENERATOR_API URecipeForTerrain : public ULandscapeFilter
{
	GENERATED_BODY()
public:
	URecipeForTerrain(const FObjectInitializer& Init);

	UPROPERTY(BlueprintReadWrite)
	ALandscape* ApplyToLandscape = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<ULandscapeFilter*> Filters;

	UFUNCTION(BlueprintCallable)
	void ApplyRecipe();

	virtual bool ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream) override;
};
