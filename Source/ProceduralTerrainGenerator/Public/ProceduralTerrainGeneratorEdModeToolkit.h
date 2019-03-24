// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ProceduralTerrainGeneratorEdMode.h"
#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "EditorModeManager.h"
#include "Landscape.h"

class FProceduralTerrainGeneratorEdModeToolkit : public FModeToolkit
{
public:

	FProceduralTerrainGeneratorEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;
	/** end of FModeToolkit interface */

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override
	{
		return FName("ProceduralTerrainGeneratorEdMode");
	}

	virtual FText GetBaseToolkitName() const override
	{
		return NSLOCTEXT("ProceduralTerrainGeneratorEdModeToolkit", "DisplayName", "ProceduralTerrainGeneratorEdMode Tool");
	}

	virtual class FEdMode* GetEditorMode() const override
	{
		return GLevelEditorModeTools().GetActiveMode(FProceduralTerrainGeneratorEdMode::EM_ProceduralTerrainGeneratorEdModeId);
	}

	virtual TSharedPtr<class SWidget> GetInlineContent() const override;
	/** end of IToolkit interface */

private:
	const UClass* FilterClass = nullptr;

	void OnFilterClassChanged(const UClass* NewClass) 
	{
		FilterClass = NewClass;
	}

	static TArray<ALandscape*> GetSelectedLandscapeActors();
};
