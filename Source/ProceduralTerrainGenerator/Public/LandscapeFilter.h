#pragma once

#include "Landscape.h"
#include "LandscapeFilter.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LandscapeFilter, Log, All);

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
