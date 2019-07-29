#include "LandscapeFilter.h"

bool ULandscapeFilter::ApplyFilter(ALandscape * Landscape, FRandomStream * RandomStream)
{
	if (!bApplyFilter)
	{
		UE_LOG(LandscapeFilter, Log, TEXT("%s: Not Applying filter to Terrain %s by config bApplyFilter"), *(GetClass()->GetName()), *(Landscape->GetName()));
		return false;
	}

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
