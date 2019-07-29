// Fill out your copyright notice in the Description page of Project Settings.

#include "NormalizeFilter.h"

// Normalize 2D array naïve implementation
bool UNormalizeFilter::ModifyHeightMap(TArray<uint16>* RawHeightMapData, FBounds Bounds, FRandomStream* RandomStream)
{
	double Avg = 0;

	for (int i = 0; i < RawHeightMapData->Num(); i++)
	{
		Avg += (double)(*RawHeightMapData)[i] / RawHeightMapData->Num();
	}

	for (int32 j = Bounds.MinY; j <= Bounds.MaxY; j++)
	{
		for (int32 i = Bounds.MinX; i <= Bounds.MaxX; i++)
		{
			int32 CurrentIndex = (j - Bounds.MinY) * (Bounds.MaxX - Bounds.MinX + 1) + (i - Bounds.MinX);
			(*RawHeightMapData)[CurrentIndex] = ((*RawHeightMapData)[CurrentIndex] - Avg) + ULandscapeFilter::ZeroInLandscape;
		}
	}

	return true;
}