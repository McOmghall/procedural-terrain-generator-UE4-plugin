// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class FProceduralTerrainGeneratorEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_ProceduralTerrainGeneratorEdModeId;
public:
	FProceduralTerrainGeneratorEdMode();
	virtual ~FProceduralTerrainGeneratorEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	//virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	//virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	//virtual void ActorSelectionChangeNotify() override;
	bool UsesToolkits() const override;
	// End of FEdMode interface
};
