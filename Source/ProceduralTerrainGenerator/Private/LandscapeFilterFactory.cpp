// Fill out your copyright notice in the Description page of Project Settings.

#include "LandscapeFilterFactory.h"
#include "KismetEditorUtilities.h"

ULandscapeFilterFactory::ULandscapeFilterFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = URecipeForTerrain::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* ULandscapeFilterFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FKismetEditorUtilities::CreateBlueprint(InClass, InParent, InName, EBlueprintType::BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}