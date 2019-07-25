// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWalkersFilter.h"
#include "LandscapeEdit.h"
#include "LandscapeComponent.h"

bool URandomWalkersFilter::ApplyFilter(ALandscape* Landscape, FRandomStream* RandomStream)
{
	UE_LOG(LandscapeFilter, Log, TEXT("RandomWalkersFilter: Applying filter to Terrain %s"), *(Landscape->GetName()));

	ULandscapeInfo* LandscapeInfo = Landscape->CreateLandscapeInfo();
	FLandscapeEditDataInterface Editor(LandscapeInfo);
	int32 MinX, MinY, MaxX, MaxY;
	if (LandscapeInfo && LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
	{
		TArray<uint16> CurrentData;
		CurrentData.SetNum((MaxX - MinX + 1) * (MaxY - MinY + 1), false);
		Editor.GetHeightData(MinX, MinY, MaxX, MaxY, CurrentData.GetData(), 0);

		TArray<uint16> Data(CurrentData);
		uint32 AuxWalkersToSpawn = WalkersToSpawn;
		while (AuxWalkersToSpawn > 0)
		{
			AuxWalkersToSpawn--;
			uint32 AuxStepsPerWalker = StepsPerWalker;
			FIntPoint CurrentPoint(RandomStream->RandRange(MinX, MaxX), RandomStream->RandRange(MinY, MaxY));

			while (AuxStepsPerWalker > 0)
			{
				AuxStepsPerWalker--;
				int32 X = CurrentPoint.X;
				int32 Y = CurrentPoint.Y;
				if (X >= MinX && X <= MaxX && Y >= MinY && Y <= MaxY)
				{
					int CurrentIndex = (CurrentPoint.Y - MinY) * (MaxX - MinX + 1) + (CurrentPoint.X - MinX);
					Data[CurrentIndex] += WeightPerWalk;
				}

				CurrentPoint = FIntPoint(CurrentPoint.X + RandomStream->RandRange(-1, 1), CurrentPoint.Y + RandomStream->RandRange(-1, 1));
			}
		}

		Editor.SetHeightData(MinX, MinY, MaxX, MaxY, Data.GetData(), 0, true);

		Editor.Flush();

		return true;
	}

	return false;
}