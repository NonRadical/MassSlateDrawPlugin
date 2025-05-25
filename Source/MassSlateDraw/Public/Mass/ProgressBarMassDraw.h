// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassDrawTraitBase.h"
#include "MassEntityTraitBase.h"
#include "UI/MassDrawLayer.h"
#include "VisualLogger/VisualLogger.h"
#include "ProgressBarMassDraw.generated.h"

USTRUCT()
struct MASSSLATEDRAW_API FProgressBarSlateFragment : public FMassFragment
{
	GENERATED_BODY()
	
	static void Draw(const FMassDrawStateFragment& DrawState, const FProgressBarSlateFragment& ProgressSlateData, const float ViewportScale, FSlateClippingZone& ClippingZone, FPaintGeometry& PaintGeometry, FSlateWindowElementList& OutDrawElements, const FLinearColor& MasterTint, const int32 LayerId);

	UPROPERTY()
	float BarProgress = 1.f;
	UPROPERTY()
	FSimplifiedSlateBrush BackplateBrush = FSimplifiedSlateBrush();
	UPROPERTY()
	FSimplifiedSlateBrush BarBrush = FSimplifiedSlateBrush();
	
	UPROPERTY()
	FVector2f DrawOffset = FVector2f(0.f);
	UPROPERTY()
	bool bUseProgressClip = false;
};

class FProgressBarDrawLayer final : public TMassDrawLayer<FProgressBarSlateFragment>
{
public:
	explicit FProgressBarDrawLayer(const FLocalPlayerContext& PlayerContext)
		: TMassDrawLayer(PlayerContext) {}
};

UCLASS(BlueprintType, EditInlineNew, meta=(DisplayName="Draw Progress Bar Trait"))
class MASSSLATEDRAW_API UMassDrawProgressBarTrait : public UMassDrawTraitBase
{
	GENERATED_UCLASS_BODY()

//~ Begin UMassEntityTraitBase Interface
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
//~ End UMassEntityTraitBase Interface

//~ Begin UMassDrawTraitBase Interface
public:
	virtual FVector2f GetBaseExtentHalfSize() const override { return FVector2f(FMath::Max(BackplateBrush.ImageSize.X, BarBrush.ImageSize.X), FMath::Max(BackplateBrush.ImageSize.Y, BarBrush.ImageSize.Y)) + FMath::Abs(DrawOffset.Y); }
//~ End UMassDrawTraitBase Interface

protected:
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	bool bUseClippingForProgressBar = false;
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	FVector2f DrawOffset = FVector2f(0.f);
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	FSimplifiedSlateBrush BackplateBrush = FSimplifiedSlateBrush();
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	FSimplifiedSlateBrush BarBrush = FSimplifiedSlateBrush();
};