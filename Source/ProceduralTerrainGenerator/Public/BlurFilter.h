// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeFilter.h"
#include "BlurFilter.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALTERRAINGENERATOR_API UBlurFilter : public ULandscapeFilter
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	uint32 DistanceOfInfluence = 10;

	UPROPERTY(EditAnywhere)
	double InfluenceDecayPerDistanceUnit = 1.0f;

	virtual bool ModifyHeightMap(TArray<uint16>* RawHeightMapData, FBounds Bounds, FRandomStream* RandomStream) override;
};
