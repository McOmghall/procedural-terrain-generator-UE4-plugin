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

UCLASS(BlueprintType, Abstract)
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

	virtual bool ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream) 
	{ 
		UE_LOG(LandscapeFilter, Log, TEXT("%s: Applying filter to Terrain %s"), *(GetClass()->GetName()), *(Landscape->GetName()));

		ULandscapeInfo* LandscapeInfo = Landscape->CreateLandscapeInfo();
		FLandscapeEditDataInterface Editor(LandscapeInfo);
		int32 MinX, MinY, MaxX, MaxY;
		if (LandscapeInfo && LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
		{
			FBounds Bounds(MinX, MinY, MaxX, MaxY);

			// TODO: Partition in chunks to manage big terrain objects
			TArray<uint16>* HeightData = new TArray<uint16>();
			HeightData->SetNum((MaxX - MinX + 1) * (MaxY - MinY + 1), false);
			Editor.GetHeightData(MinX, MinY, MaxX, MaxY, HeightData->GetData(), 0);
			bool bModifiedHeightMap = ModifyHeightMap(HeightData, Bounds, RandomStream);
			if (bModifiedHeightMap)
			{
				Editor.SetHeightData(MinX, MinY, MaxX, MaxY, HeightData->GetData(), 0, true);
				Editor.Flush();
			}
			return bModifiedHeightMap;
		}
		return false;
	};

	virtual bool ModifyHeightMap(TArray<uint16>* RawHeightMapData, FBounds Bounds, FRandomStream* RandomStream) { return false; };
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
