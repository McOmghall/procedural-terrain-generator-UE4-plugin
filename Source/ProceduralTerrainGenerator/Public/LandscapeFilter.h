#pragma once

#include "Landscape.h"
#include "LandscapeFilter.generated.h"

UINTERFACE()
class PROCEDURALTERRAINGENERATOR_API ULandscapeFilter : public UInterface
{
	GENERATED_BODY()
};

class ILandscapeFilter
{
	GENERATED_BODY()
public:

	virtual bool ApplyFilter(ALandscape* Landscape) = 0;
};