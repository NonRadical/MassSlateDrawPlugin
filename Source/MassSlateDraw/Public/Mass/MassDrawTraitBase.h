// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTraitBase.h"
#include "Styling/SlateBrush.h"
#include "VisualLogger/VisualLogger.h"
#include "MassSlateDraw.h"
#include "MassDrawTraitBase.generated.h"

DECLARE_CYCLE_STAT(TEXT("MassDraw - OnPaint"), STAT_MassDrawOnPaint, STATGROUP_MassDraw);

//Struct that represents a simplified FSlateBrush. Used instead of FSlateBrush to reduce struct size.
USTRUCT(BlueprintType)
struct MASSSLATEDRAW_API FSimplifiedSlateBrush
{
	GENERATED_BODY()

	operator FSlateBrush() const
	{
		FSlateBrush Brush = {};
		Brush.SetResourceObject(ResourceObject);
		Brush.TintColor = TintColor;
		Brush.ImageSize = ImageSize;
		return Brush;
	}

	void ApplyTo(FSlateBrush& Brush) const
	{
		Brush.SetResourceObject(ResourceObject);
		Brush.TintColor = TintColor;
		Brush.ImageSize = ImageSize;
	}
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Brush, meta=( AllowPrivateAccess="true", DisplayThumbnail="true", DisplayName="Image", AllowedClasses="/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	TObjectPtr<UObject> ResourceObject;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Brush, meta=( DisplayName="Tint", sRGB="true" ))
	FSlateColor TintColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Brush)
	FVector2f ImageSize = {SlateBrushDefs::DefaultImageSize, SlateBrushDefs::DefaultImageSize};
};

//Fragment containing the most basic draw state of a given icon.
USTRUCT()
struct MASSSLATEDRAW_API FMassDrawStateFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector WorldOffset = FVector(0.0);
	//Populated by draw traits. Used for preculling bound tests as well as scaling tests.
	UPROPERTY()
	FVector2f ExtentHalfSize = FVector2f(0.f);
	//Z represents the screen depth of the item in question.
	UPROPERTY()
	FVector3f ScreenPosition = FVector3f(-UE_MAX_FLT);
	UPROPERTY()
	float DistanceScale = -1.f;
	UPROPERTY()
	bool bIsEnabled = true;
};

/**
 * Base trait for any MassDraw trait. Abstract and meant to be subclassed.
 * See UMassDrawTraitBase or UMassDrawProgressBarTrait for example implementations of this trait.
 */
UCLASS(NotBlueprintable, NotBlueprintType, NotEditInlineNew, Abstract)
class MASSSLATEDRAW_API UMassDrawTraitBase : public UMassEntityTraitBase
{
	GENERATED_UCLASS_BODY()

//~ Begin UMassEntityTraitBase Interface
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
//~ End UMassEntityTraitBase Interface

public:
	virtual FVector2f GetBaseExtentHalfSize() const PURE_VIRTUAL(UMassDrawTraitBase, return FVector2f(););
	
protected:
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	FVector WorldOffset = FVector(0.0);
	UPROPERTY(Category="Draw", EditDefaultsOnly)
	float DistanceScaling = -1.f;
};