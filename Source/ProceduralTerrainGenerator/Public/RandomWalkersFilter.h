// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeFilter.h"
#include "RandomWalkersFilter.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALTERRAINGENERATOR_API URandomWalkersFilter : public ULandscapeFilter
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	uint32 WalkersToSpawn = 10;

	UPROPERTY(EditAnywhere)
	uint32 StepsPerWalker = 1000000;

	UPROPERTY(EditAnywhere)
	uint32 WeightPerWalk = 100;

	virtual bool ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream) override;
};
