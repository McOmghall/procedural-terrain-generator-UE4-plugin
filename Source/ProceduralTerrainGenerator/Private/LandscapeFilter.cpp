#include "LandscapeFilter.h"
#include "ProceduralTerrainGenerator.h"

#undef LOCTEXT_NAMESPACE
#define LOCTEXT_NAMESPACE "ULandscapeFilter"

bool ULandscapeFilter::ApplyFilter(ALandscape * Landscape, FRandomStream* RandomStream)
{
	if (!bApplyFilter)
	{
		UE_LOG(LandscapeFilter, Log, TEXT("%s: Not Applying filter to Terrain %s by config bApplyFilter"), *(GetName()), *(Landscape->GetName()));
		return false;
	}

	FRandomStream* InternalRandomStream = RandomStream;
	if (bOverrideSeed) 
	{
		UE_LOG(LandscapeFilter, Log, TEXT("%s: Overriding seed from %x to %x"), *(GetName()), RandomStream->GetCurrentSeed(), RandomSeed);
		InternalRandomStream = new FRandomStream(RandomSeed);
	}

	UE_LOG(LandscapeFilter, Log, TEXT("%s: Applying filter to Terrain %s with seed %x"), *(GetName()), *(Landscape->GetName()), InternalRandomStream->GetCurrentSeed());

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
			Editor.RecalculateNormals();
		}
		return bModifiedHeightMap;
	}
	return false;
}

uint16 ULandscapeFilter::SafeOverflowAdd(int32 A, int32 B)
{
	int32 SafeRangeBoundaryMax = ((int32)UINT16_MAX) - B;
	int32 SafeRangeBoundaryMin = 0 - B;
	if (A > SafeRangeBoundaryMax) {
		// Overflow
		return UINT16_MAX;
	}
	else if (A < SafeRangeBoundaryMin)
	{
		// Underflow
		return 0;
	}
	else
	{
		return A + B;
	}
};


const FText TransactionTitle = LOCTEXT("ULandscapeFilter.UndoRedoApplyFiltersName", "Apply Filter");
const FString TransactionNamespace = "ProceduralTerrainTool";

bool ULandscapeFilter::ApplyFilterStatic(ULandscapeFilter* Filter, ALandscape* Landscape)
{
	GEditor->BeginTransaction(*TransactionNamespace, TransactionTitle, Landscape);
	UE_LOG(ProceduralTerrainGenerator, Log, TEXT("Applying filter %s to Landscape %s"), *Filter->GetName(), *Landscape->GetName());
	bool bRval = Filter->ApplyFilter(Landscape, new FRandomStream(FDateTime::Now().GetMillisecond()));
	GEditor->EndTransaction();
	return bRval;
}


void ULandscapeFilter::ApplyFilterAssetToLandscapeStatic(UObject* FilterAsset, AActor* Landscape)
{
	ULandscapeFilter* InternalFilter = Cast<ULandscapeFilter>(FilterAsset);
	ALandscape* InternalLandscape = Cast<ALandscape>(Landscape);

	if (InternalFilter && InternalLandscape)
	{
		ApplyFilterStatic(InternalFilter, InternalLandscape);
	}
}

#undef LOCTEXT_NAMESPACE
