// Fill out your copyright notice in the Description page of Project Settings.

#include "BlurFilter.h"

// NaÏve gaussian blur impl
bool UBlurFilter::ModifyHeightMap(TArray<uint16>* RawHeightMapData, FBounds Bounds, FRandomStream* RandomStream)
{
	for (int32 j = Bounds.MinY; j <= Bounds.MaxY; j++)
	{
		for (int32 i = Bounds.MinX; i <= Bounds.MaxX; i++)
		{
			double CountPoints = 0;
			double SumPointHeight = 0;

			int32 RealDistanceOfInfluence = DistanceOfInfluence;
			for (int32 OffsetX = -RealDistanceOfInfluence; OffsetX <= RealDistanceOfInfluence; OffsetX++)
			{
				int32 NeededOffsetY = RealDistanceOfInfluence - FMath::Abs(OffsetX);
				for (int32 OffsetY = -NeededOffsetY; OffsetY <= NeededOffsetY; OffsetY++)
				{
					FIntPoint CurrentPoint(i + OffsetX, j + OffsetY);
					int32 CurrentIndex = (CurrentPoint.Y - Bounds.MinY) * (Bounds.MaxX - Bounds.MinX + 1) + (CurrentPoint.X - Bounds.MinX);
					if (CurrentPoint.X >= Bounds.MinX && CurrentPoint.X <= Bounds.MaxX && CurrentPoint.Y >= Bounds.MinY && CurrentPoint.Y <= Bounds.MaxY)
					{
						double Weight = FMath::Pow(InfluenceDecayPerDistanceUnit, FMath::Abs(OffsetX) + FMath::Abs(OffsetY));
						CountPoints += Weight;
						SumPointHeight += Weight * (*RawHeightMapData)[CurrentIndex];
					}
				}
			}

			int32 CurrentIndex = (j - Bounds.MinY) * (Bounds.MaxX - Bounds.MinX + 1) + (i - Bounds.MinX);
			(*RawHeightMapData)[CurrentIndex] = FMath::Clamp(FMath::RoundToInt(SumPointHeight / CountPoints), 0, (int32)UINT16_MAX);
		}
	}

	return true;
}
