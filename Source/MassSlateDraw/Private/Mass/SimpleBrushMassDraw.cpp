// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/SimpleBrushMassDraw.h"
#include "MassEntityTemplateRegistry.h"
#include "VisualLogger/VisualLogger.h"

UMassDrawSimpleBrushTrait::UMassDrawSimpleBrushTrait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UMassDrawSimpleBrushTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	if (World.IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	Super::BuildTemplate(BuildContext, World);

	FSimpleBrushSlateFragment& SlateFragment = BuildContext.AddFragment_GetRef<FSimpleBrushSlateFragment>();
	SlateFragment.DrawOffset = DrawOffset;
	SlateFragment.Brush = Brush;
}