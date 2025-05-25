// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/MassDrawProjectionProcessor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "MassCommonFragments.h"
#include "Mass/MassDrawTraitBase.h"
#include "MassExecutionContext.h"
#include "MassSlateDraw.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

namespace MassSlateDraw::ProjectionProcessor
{
	//This preculling should be faster than Slate's. But requires the implementation of
	//GetBaseExtentHalfSize in subclasses of UMassDrawTraitBase to express extent at max size.
	static bool bPerformPreculling = true;
	static FAutoConsoleVariableRef CVarPerformPreculling(
		TEXT("MassSlateDraw.ProjectionProcessor.PerformPreculling"),
		bPerformPreculling,
		TEXT("If true, the mass draw projection processor will attempt to precull Mass Draw Fragments "
		"(instead of letting only Slate handle it)."),
		ECVF_Default);

};

UMassDrawProjectionProcessor::UMassDrawProjectionProcessor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProcessingPhase = EMassProcessingPhase::FrameEnd; //Icon screen position processing needs to run after camera updates to be accurate to the current frame.
	
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
}

void UMassDrawProjectionProcessor::ConfigureQueries()
{	
	DrawProjectionQuery.RegisterWithProcessor(*this);
	DrawProjectionQuery.AddRequirement<FMassDrawStateFragment>(EMassFragmentAccess::ReadWrite);
	DrawProjectionQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

inline FIntRect::IntPointType ToIntPoint(const FVector2f& VectorPoint)
{
	return FIntRect::IntPointType(VectorPoint.X, VectorPoint.Y);
}

//This function is pretty much the same as FSceneView::ProjectWorldToScreen but;
// - ViewRect is now a FVector4f instead of a FIntRect to skip some float casts.
// - OutScreenPosition is now a FVector instead of a FVector2D since we intend on using scene depth.
FORCEINLINE bool ProjectWorldToScreen(const FVector& WorldPosition, const FVector4f& ViewRect, const FMatrix& ViewProjectionMatrix, FVector3f& OutScreenPosition)
{
	const FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
	if (Result.W <= 0.f)
	{
		return false;
	}

	// the result of this will be x and y coords in -1..1 projection space
	const float RHW = 1.f / Result.W;
	const FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

	// Move from projection space to normalized 0..1 UI space
	const float NormalizedX = ( PosInScreenSpace.X / 2.f ) + 0.5f;
	const float NormalizedY = 1.f - ( PosInScreenSpace.Y / 2.f ) - 0.5f;
	
	const FVector2D RayStartViewRectSpace = {(NormalizedX * (ViewRect.Z - ViewRect.X)),(NormalizedY * (ViewRect.W - ViewRect.Y))};
	const FVector2D OutScreenPosition2D = RayStartViewRectSpace + FVector2D(ViewRect.X, ViewRect.Y);
	OutScreenPosition = FVector3f(OutScreenPosition2D.X, OutScreenPosition2D.Y, Result.W);
	return true;
}

DECLARE_CYCLE_STAT(TEXT("MassDraw - ProjectionProcessor"), STAT_MassDrawProjectionProcessor, STATGROUP_MassDraw);
void UMassDrawProjectionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_MassDrawProjectionProcessor);
	
	const UWorld* World = EntityManager.GetWorld();
	
	if(!World)
	{
		return;
	}
	
	const APlayerController* LocalPlayerController = World->GetFirstPlayerController();

	if(!LocalPlayerController)
	{
		return;
	}

	const ULocalPlayer* const LocalPlayer = LocalPlayerController->GetLocalPlayer();

	if(!LocalPlayer || !LocalPlayer->ViewportClient)
	{
		return;
	}
	
	FSceneViewProjectionData ProjectionData;
	LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData);

	if (!ProjectionData.IsValidViewRectangle())
	{
		return;
	}
	
	const bool bPerformPreculling = MassSlateDraw::ProjectionProcessor::bPerformPreculling;
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(LocalPlayer->ViewportClient);
	
	DrawProjectionQuery.ForEachEntityChunk(EntityManager, Context, [ProjectionData, bPerformPreculling, ViewportScale](FMassExecutionContext& LocalContext)
	{
		const TArrayView<FMassDrawStateFragment> DrawStateList = LocalContext.GetMutableFragmentView<FMassDrawStateFragment>();
		const TConstArrayView<FTransformFragment> TransformList = LocalContext.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = LocalContext.GetNumEntities();

		const FIntRect& ViewRectangle = ProjectionData.GetConstrainedViewRect();
		const FVector4f ViewRectangleFloat(ViewRectangle.Min.X, ViewRectangle.Min.Y, ViewRectangle.Max.X, ViewRectangle.Max.Y);
		FMatrix const ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
		FVector3f EntityScreenPosition = FVector3f(-UE_MAX_FLT);

		for(int32 Index = NumEntities - 1; Index >= 0; Index--)
		{
			FMassDrawStateFragment& DrawState = DrawStateList[Index];

			if (!DrawState.bIsEnabled)
			{
				DrawState.ScreenPosition = FVector3f(-UE_MAX_FLT);
				continue;
			}
			
			const FVector TransformedPosition = TransformList[Index].GetTransform().TransformPosition(DrawState.WorldOffset);
			
			if(!ProjectWorldToScreen(TransformedPosition, ViewRectangleFloat, ViewProjectionMatrix, EntityScreenPosition))
			{
				DrawState.ScreenPosition = FVector3f(-UE_MAX_FLT);
				continue;
			}

			const float DrawScale = DrawState.DistanceScale != -1.f ? ViewportScale * (1.f - ((EntityScreenPosition.Z - DrawState.DistanceScale) / DrawState.DistanceScale)) : ViewportScale;

			if (DrawScale <= 0.f)
			{
				DrawState.ScreenPosition = FVector3f(-UE_MAX_FLT);
				continue;
			}
			
			if(bPerformPreculling)
			{
				const FVector2f TotalIconHalfSize = DrawState.ExtentHalfSize * DrawScale;

				if (TotalIconHalfSize.X < 0.5f || TotalIconHalfSize.Y < 0.5f)
				{
					DrawState.ScreenPosition = FVector3f(-UE_MAX_FLT);
					continue;
				}
				
			#if WITH_EDITOR
				//Editor has an issue where the given rectangle is not 100% accurate.
				const FVector2f UsedIconHalfSize = (DrawState.ExtentHalfSize * DrawScale) + (GEditor ? FVector2f(16.f, 32.f) : FVector2f(0.f));
			#else
				const FVector2f UsedIconHalfSize = TotalIconHalfSize;
			#endif
							
				const FIntRect::IntPointType DrawItemTopLeft = FIntRect::IntPointType(ToIntPoint(FVector2f(EntityScreenPosition) - UsedIconHalfSize));
				const FIntRect::IntPointType DrawItemBottomRight = FIntRect::IntPointType(ToIntPoint(FVector2f(EntityScreenPosition) + UsedIconHalfSize));
				if(!ViewRectangle.Intersect(FIntRect(DrawItemTopLeft, DrawItemBottomRight)))
				{
					DrawState.ScreenPosition = FVector3f(-UE_MAX_FLT);
					continue;
				}
			}

			DrawState.ScreenPosition = EntityScreenPosition;
		}
	});
}