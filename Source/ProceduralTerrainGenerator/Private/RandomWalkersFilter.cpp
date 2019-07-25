// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomWalkersFilter.h"


bool URandomWalkersFilter::ModifyHeightMap(TArray<uint16>* RawHeightMapData, FBounds Bounds, FRandomStream* RandomStream)
{
	uint32 AuxWalkersToSpawn = WalkersToSpawn;
	while (AuxWalkersToSpawn > 0)
	{
		AuxWalkersToSpawn--;
		uint32 AuxStepsPerWalker = StepsPerWalker;
		FIntPoint CurrentPoint(RandomStream->RandRange(Bounds.MinX, Bounds.MaxX), RandomStream->RandRange(Bounds.MinY, Bounds.MaxY));

		while (AuxStepsPerWalker > 0)
		{
			AuxStepsPerWalker--;
			int32 X = CurrentPoint.X;
			int32 Y = CurrentPoint.Y;
			if (X >= Bounds.MinX && X <= Bounds.MaxX && Y >= Bounds.MinY && Y <= Bounds.MaxY)
			{
				int CurrentIndex = (CurrentPoint.Y - Bounds.MinY) * (Bounds.MaxX - Bounds.MinX + 1) + (CurrentPoint.X - Bounds.MinX);
				(*RawHeightMapData)[CurrentIndex] += WeightPerWalk;
			}

			CurrentPoint = FIntPoint(CurrentPoint.X + RandomStream->RandRange(-1, 1), CurrentPoint.Y + RandomStream->RandRange(-1, 1));
		}
	}

	return true;
}
