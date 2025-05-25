// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/ProgressBarMassDraw.h"
#include "MassEntityTemplateRegistry.h"
#include "VisualLogger/VisualLogger.h"

namespace MassSlateDraw::ProgressBar
{
	static bool bEnableProgressBarClipping = true;
	static FAutoConsoleVariableRef CVarEnableProgressBarClipping(
		TEXT("MassDraw.ProgressBar.EnableProgressBarClipping"),
		bEnableProgressBarClipping,
		TEXT("If true, allows progress bars to use clipping instead of scaling."),
		ECVF_Default);
}

void FProgressBarSlateFragment::Draw(const FMassDrawStateFragment& DrawState, const FProgressBarSlateFragment& ProgressSlateData, const float ViewportScale, FSlateClippingZone& ClippingZone, FPaintGeometry& PaintGeometry, FSlateWindowElementList& OutDrawElements, const FLinearColor& MasterTint, const int32 LayerId)
{
	const FVector2f ScreenPosition = FVector2f(DrawState.ScreenPosition.X, DrawState.ScreenPosition.Y);
	const FVector2f BackplateSize = ViewportScale * (ProgressSlateData.BackplateBrush.ImageSize / PaintGeometry.GetLocalSize());
	const FVector2f BackplateDrawPosition = (ScreenPosition - (ProgressSlateData.BackplateBrush.ImageSize * 0.5f * ViewportScale)) + (ProgressSlateData.DrawOffset * ViewportScale);
	const FVector2f RoundedBackplateDrawPosition = FVector2f(FMath::RoundToInt(BackplateDrawPosition.X), FMath::RoundToInt(BackplateDrawPosition.Y));
	PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BackplateSize), RoundedBackplateDrawPosition));

	FSlateBrush DrawBrush = ProgressSlateData.BackplateBrush;
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &DrawBrush, ESlateDrawEffect::None, ProgressSlateData.BackplateBrush.TintColor.GetSpecifiedColor() * MasterTint);
	
	if(ProgressSlateData.BarProgress == 0.f)
	{
		return;
	}
	
	const FVector2f BarDrawPosition = (ScreenPosition - (ProgressSlateData.BarBrush.ImageSize * 0.5f * ViewportScale)) + (ProgressSlateData.DrawOffset * ViewportScale);
	const FVector2f RoundedBarDrawPosition = FVector2f(FMath::RoundToInt(BarDrawPosition.X), FMath::RoundToInt(BarDrawPosition.Y));
	
	const FVector2f BarScale = ViewportScale * (ProgressSlateData.BarBrush.ImageSize / PaintGeometry.GetLocalSize());
	
	ProgressSlateData.BarBrush.ApplyTo(DrawBrush);
	
	if(ProgressSlateData.BarProgress >= 1.f)
	{
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BarScale), RoundedBarDrawPosition));
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &DrawBrush, ESlateDrawEffect::None, ProgressSlateData.BarBrush.TintColor.GetSpecifiedColor() * MasterTint);
		return;
	}
	
	const bool bIconShouldPerformProgressClipping = ProgressSlateData.bUseProgressClip;

	if(bIconShouldPerformProgressClipping)
	{
		const float BarSizeY = FMath::CeilToFloat(ProgressSlateData.BarBrush.ImageSize.Y * ViewportScale);
		ClippingZone.TopLeft = FVector2f(RoundedBarDrawPosition);
		ClippingZone.BottomLeft = ClippingZone.TopLeft;
		ClippingZone.BottomLeft.Y += BarSizeY;
	
		ClippingZone.TopRight = FVector2f(RoundedBarDrawPosition);
		ClippingZone.TopRight.X += (ProgressSlateData.BarBrush.ImageSize.X * ProgressSlateData.BarProgress * ViewportScale);
		ClippingZone.BottomRight = ClippingZone.TopRight;
		ClippingZone.BottomRight.Y += BarSizeY;
		OutDrawElements.PushClip(ClippingZone);		
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BarScale), RoundedBarDrawPosition));
	}
	else
	{
		PaintGeometry.SetRenderTransform(FSlateRenderTransform(FScale2D(BarScale.X * ProgressSlateData.BarProgress, BarScale.Y), RoundedBarDrawPosition));
	}

	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &DrawBrush, ESlateDrawEffect::None, ProgressSlateData.BarBrush.TintColor.GetSpecifiedColor() * MasterTint);

	if(bIconShouldPerformProgressClipping)
	{
		OutDrawElements.PopClip();
	}
}

UMassDrawProgressBarTrait::UMassDrawProgressBarTrait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UMassDrawProgressBarTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	if (World.IsNetMode(NM_DedicatedServer))
	{
		return;
	}
	
	Super::BuildTemplate(BuildContext, World);

	FProgressBarSlateFragment& SlateFragment = BuildContext.AddFragment_GetRef<FProgressBarSlateFragment>();
	SlateFragment.DrawOffset = DrawOffset;
	SlateFragment.BackplateBrush = BackplateBrush;
	SlateFragment.BarBrush = BarBrush;
	SlateFragment.bUseProgressClip = MassSlateDraw::ProgressBar::bEnableProgressBarClipping && bUseClippingForProgressBar;
}