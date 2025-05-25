// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "Mass/MassDrawTraitBase.h"
#include "Slate/SGameLayerManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"

//Template class for a game layer representing a specific MassDrawFragment.
//This is the same as UWidgetComponent's IGameLayer implementation but templated for easier creation of layers.
template<typename MassDrawFragment>
class TMassDrawLayer : public IGameLayer
{	
public:
	//SWidget we'll use to draw to for a given mass draw game layer.
	class SMassDrawScreenLayer final : public SCompoundWidget
	{
		SLATE_BEGIN_ARGS(SMassDrawScreenLayer) {}
		SLATE_END_ARGS()
	

	public:
		void Construct(const FArguments& InArgs, const FLocalPlayerContext& InPlayerContext)
		{
			PlayerContext = InPlayerContext;
			bCanSupportFocus = false;
			SetVisibility(EVisibility::HitTestInvisible);
		}
		
		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const UWorld* World = PlayerContext.GetWorld();

			if (!World)
			{
				return LayerId;
			}
	
			FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry();
			const FLinearColor MasterTint = FLinearColor::White;

			UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(PlayerContext.GetWorld());
			if (!EntitySubsystem)
			{
				return LayerId;
			}
			
			FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	
			const UGameViewportClient* ViewportClient = World->GetGameViewport();
			if(!ViewportClient)
			{
				return LayerId;
			}
	
			const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(ViewportClient);
			
			FMassExecutionContext ExecutionContext(EntityManager);
			FMassEntityQuery Query;
			Query.AddRequirement<FMassDrawStateFragment>(EMassFragmentAccess::ReadOnly);
			Query.AddRequirement<MassDrawFragment>(EMassFragmentAccess::ReadOnly);
	
			Query.ForEachEntityChunk(EntityManager, ExecutionContext, [&OutDrawElements, PaintGeometry, MyCullingRect, ViewportScale, MasterTint, LayerId](FMassExecutionContext& Context)
			{
				SCOPE_CYCLE_COUNTER(STAT_MassDrawOnPaint);
				const TConstArrayView<FMassDrawStateFragment> StateDataList = Context.GetFragmentView<FMassDrawStateFragment>();
				const TConstArrayView<MassDrawFragment> DrawDataList = Context.GetFragmentView<MassDrawFragment>();

				FPaintGeometry CurrentPaintGeometry = FPaintGeometry(PaintGeometry);
				FSlateClippingZone ClippingZone(MyCullingRect);

				const int32 NumEntities = Context.GetNumEntities();
				for (int32 Index = NumEntities - 1; Index >= 0; Index--)
				{
					const FMassDrawStateFragment& StateData = StateDataList[Index];

					if(!StateData.bIsEnabled || StateData.ScreenPosition == FVector3f(-UE_MAX_FLT))
					{
						continue;
					}

					const float DrawScale = StateData.DistanceScale != -1.f ? ViewportScale * (1.f - ((StateData.ScreenPosition.Z - StateData.DistanceScale) / StateData.DistanceScale)) : ViewportScale;					
					MassDrawFragment::Draw(StateData, DrawDataList[Index], DrawScale, ClippingZone, CurrentPaintGeometry, OutDrawElements, MasterTint, LayerId);
				}
			});
			
			return LayerId;
		}

		virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(0, 0); }
	
	protected:
		FLocalPlayerContext PlayerContext;
	};
	
	static TSharedPtr<IGameLayer> CreateLayerForLocalPlayer(ULocalPlayer* LocalPlayer, UWorld* WorldContext, const FName& LayerName)
	{
		if (!LocalPlayer || !LocalPlayer->ViewportClient)
		{
			return nullptr;
		}
		
		const TSharedPtr<IGameLayerManager> LayerManager = LocalPlayer->ViewportClient->GetGameLayerManager();

		if (!LayerManager.IsValid())
		{
			return nullptr;
		}
		
		const TSharedPtr<IGameLayer> Layer = LayerManager->FindLayerForPlayer(LocalPlayer, LayerName);
		if (!Layer.IsValid())
		{
			TSharedRef<IGameLayer> NewScreenLayer = MakeShareable(new TMassDrawLayer(FLocalPlayerContext(LocalPlayer, WorldContext)));
			LayerManager->AddLayerForPlayer(LocalPlayer, LayerName, NewScreenLayer, -100);
			return NewScreenLayer;
		}

		return Layer;
	}

	TMassDrawLayer(const FLocalPlayerContext& PlayerContext)
	{
		OwningPlayer = PlayerContext;
		ScreenLayerPtr = nullptr;
	}
	
	virtual ~TMassDrawLayer()
	{
		// empty virtual destructor to help clang warning
	}
	
protected:
	virtual TSharedRef<SWidget> AsWidget() override
	{
		if (TSharedPtr<SMassDrawScreenLayer> ScreenLayer = ScreenLayerPtr.Pin())
		{
			return ScreenLayer.ToSharedRef();
		}

		TSharedRef<SMassDrawScreenLayer> NewScreenLayer = SNew(SMassDrawScreenLayer, OwningPlayer);
		ScreenLayerPtr = NewScreenLayer;
		return NewScreenLayer;
	}
	
private:
	FLocalPlayerContext OwningPlayer;
	TWeakPtr<SMassDrawScreenLayer> ScreenLayerPtr;
};
