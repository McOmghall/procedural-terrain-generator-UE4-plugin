#pragma once

#include "Landscape.h"
#include "UnrealType.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "LandscapeFilter.generated.h"

class SVerticalBox;

DECLARE_LOG_CATEGORY_EXTERN(LandscapeFilter, Log, All);

UCLASS(BlueprintType, Abstract)
class PROCEDURALTERRAINGENERATOR_API ULandscapeFilter : public UObject
{
	GENERATED_BODY()
public:
	virtual bool ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream) { return false; };
};

class SLandscapeFilterProperties : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapeFilterProperties)
		: _MetaClass(UObject::StaticClass())
	{}
		/** The meta class that the selected class must be a child-of (required) */
		SLATE_ATTRIBUTE(const UClass*, MetaClass)
	SLATE_END_ARGS()
};
