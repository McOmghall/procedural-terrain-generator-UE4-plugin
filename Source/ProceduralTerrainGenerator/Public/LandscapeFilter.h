#pragma once

#include "Landscape.h"
#include "UnrealType.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "LandscapeEdit.h"
#include "LandscapeComponent.h"
#include "LandscapeFilter.generated.h"

class SVerticalBox;

DECLARE_LOG_CATEGORY_EXTERN(LandscapeFilter, Log, All);

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class PROCEDURALTERRAINGENERATOR_API ULandscapeFilter : public UObject
{
	GENERATED_BODY()
public:
	struct FBounds
	{
		int32 MinX = 0;
		int32 MinY = 0;
		int32 MaxX = 0;
		int32 MaxY = 0;

		FBounds(int32 InMinX = 0, int32 InMinY = 0, int32 InMaxX = 0, int32 InMaxY = 0)
			: MinX(InMinX),
			MinY(InMinY), 
			MaxX(InMaxX),
			MaxY(InMaxY)
		{
		};
	};

	const static uint16 ZeroInLandscape = 0x8000;

	UPROPERTY(EditAnywhere)
	bool bApplyFilter = true;

	UPROPERTY(EditAnywhere)
	bool bOverrideSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RandomSeed = 0xCAFE1EAD;

	virtual bool ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream);
	virtual bool ModifyHeightMap(TArray<uint16>* RawHeightMapData, FBounds Bounds, FRandomStream* RandomStream) { return false; };

	static uint16 SafeOverflowAdd(int32 A, int32 B);

	static bool ApplyFilterStatic(ULandscapeFilter* Filter, ALandscape* Landscape);
	static void ApplyFilterAssetToLandscapeStatic(UObject* FilterAsset, AActor* Landscape);
};
