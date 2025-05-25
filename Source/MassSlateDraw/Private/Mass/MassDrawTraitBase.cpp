// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/MassDrawTraitBase.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "VisualLogger/VisualLogger.h"

UMassDrawTraitBase::UMassDrawTraitBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

inline FVector2D MaxVector2D(const FVector2D& A, const FVector2D& B)
{
	return FVector2D(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
}

void UMassDrawTraitBase::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	if (World.IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	FMassDrawStateFragment& StateFragment = BuildContext.AddFragment_GetRef<FMassDrawStateFragment>();
	StateFragment.WorldOffset = WorldOffset;
	StateFragment.DistanceScale = DistanceScaling;
	StateFragment.ExtentHalfSize = GetBaseExtentHalfSize();
	
	BuildContext.RequireFragment<FTransformFragment>();
}