// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGeneratorEdMode.h"
#include "ProceduralTerrainGeneratorEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FProceduralTerrainGeneratorEdMode::EM_ProceduralTerrainGeneratorEdModeId = TEXT("EM_ProceduralTerrainGeneratorEdMode");

FProceduralTerrainGeneratorEdMode::FProceduralTerrainGeneratorEdMode()
{

}

FProceduralTerrainGeneratorEdMode::~FProceduralTerrainGeneratorEdMode()
{

}

void FProceduralTerrainGeneratorEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FProceduralTerrainGeneratorEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FProceduralTerrainGeneratorEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FProceduralTerrainGeneratorEdMode::UsesToolkits() const
{
	return true;
}




