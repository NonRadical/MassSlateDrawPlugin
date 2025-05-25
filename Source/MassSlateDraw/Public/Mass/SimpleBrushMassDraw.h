// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassDrawTraitBase.h"
#include "MassEntityTraitBase.h"
#include "UI/MassDrawLayer.h"
#include "VisualLogger/VisualLogger.h"
#include "SimpleBrushMassDraw.generated.h"

//Basic implementation of a FMassDrawSlateFragment. Draws a single icon.
USTRUCT()
struct MASSSLATEDRAW_API FSimpleBrushSlateFragment : public FMassFragment
{
	GENERATED_BODY()
	
	static void Draw(const FMassDrawStateFragment& DrawState, const FSimpleBrushSlateFragment& SimpleBrushData, const float DrawScale, FSlateClippingZone& ClippingZone, FPaintGeometry& PaintGeometry, FSlateWindowElementList& OutDrawElements, const FLinearColor& MasterTint, const int32 LayerId)
	{
		const FVector2f BrushSize = DrawScale * (SimpleBrushData.Brush.ImageSize / PaintGeometry.GetLocalSize());
		const FVector2f BrushDrawPosition = (FVector2f(DrawState.ScreenPosition.X, DrawState.ScreenPosition.Y) - (SimpleBrushData.Brush.ImageSize * 0.5f * DrawScale)) + (SimpleBrushData.DrawOffset * DrawScale);
		const FVector2f RoundedBackplateDrawPosition = FVector2f(FMath::RoundToInt(BrushDrawPosition.X), FMath::RoundToInt(BrushDrawPosition.Y));
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BrushSize), RoundedBackplateDrawPosition));

		const FSlateBrush DrawBrush = SimpleBrushData.Brush;
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &DrawBrush, ESlateDrawEffect::None, SimpleBrushData.Brush.TintColor.GetSpecifiedColor() * MasterTint);
	}
	
	UPROPERTY()
	FSimplifiedSlateBrush Brush = FSimplifiedSlateBrush();
	UPROPERTY()
	FVector2f DrawOffset = FVector2f(0.f);
};

class MASSSLATEDRAW_API FSimpleBrushDrawLayer final : public TMassDrawLayer<FSimpleBrushSlateFragment>
{
public:
	explicit FSimpleBrushDrawLayer(const FLocalPlayerContext& PlayerContext)
		: TMassDrawLayer(PlayerContext) {}
};

UCLASS(BlueprintType, EditInlineNew, meta=(DisplayName="Draw Simple Brush Trait"))
class MASSSLATEDRAW_API UMassDrawSimpleBrushTrait : public UMassDrawTraitBase
{
	GENERATED_UCLASS_BODY()

//~ Begin UMassEntityTraitBase Interface
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
//~ End UMassEntityTraitBase Interface

//~ Begin UMassDrawTraitBase Interface
public:
	virtual FVector2f GetBaseExtentHalfSize() const override { return Brush.ImageSize + DrawOffset; }
//~ End UMassDrawTraitBase Interface
	
protected:
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	FVector2f DrawOffset = FVector2f(0.f);
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	FSimplifiedSlateBrush Brush = FSimplifiedSlateBrush();
};