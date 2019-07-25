// Copyright (C) 2019 Pedro Montoto García - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LandscapeFilter.h"
#include "FlattenFilter.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURALTERRAINGENERATOR_API UFlattenFilter : public ULandscapeFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 Height = 0x0FFF;

	// Inherited via ILandscapeFilter
	virtual bool ApplyFilter(ALandscape *Landscape, FRandomStream* InRandomStream) override;
};
