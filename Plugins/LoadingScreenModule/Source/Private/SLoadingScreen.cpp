// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingScreen.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h" // For SThrobber
#include "Widgets/Layout/SScaleBox.h"	// For ScaleBox
#include "Widgets/Layout/SBorder.h"		// For Border
#include "Widgets/SOverlay.h"
#include "Styling/CoreStyle.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoadingScreen::Construct(const FArguments& InArgs)
{
	// Store texture from arguments
	BackgroundTexture = InArgs._BackgroundTexture;

	if (BackgroundTexture)
	{
		BackgroundBrush = MakeShared<FSlateBrush>();
		BackgroundBrush->SetResourceObject(BackgroundTexture);
		BackgroundBrush->ImageSize = FVector2D(
			BackgroundTexture->GetSizeX(),
			BackgroundTexture->GetSizeY()
		);
	}


	// Begin constructing the widget hierarchy
	// ChildSlot is the root slot of our compound widget.

	ChildSlot
		[
			SNew(SOverlay)

				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					// Create an overlay widget which allows stacking multiple elements.
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor::Black)
						[
							SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								.StretchDirection(EStretchDirection::Both)	// Stretch Both to fill space.
								.HAlign(HAlign_Center)		// Stretch horizontally to fill space.
								.VAlign(VAlign_Center)		// Stretch vertically to fill space.
								[
									// Create an image widget using our background brush
									SNew(SImage)
										.Image(BackgroundTexture ? BackgroundBrush.Get() : nullptr)
										.ColorAndOpacity(FLinearColor::White)
								]
						]
				]

			// Add second slot to the overlay for the loading indicator
			+ SOverlay::Slot()
				.HAlign(HAlign_Right)	// Align to right
				.VAlign(VAlign_Bottom)	// Align to bottom
				.Padding(10.0f)			// Add 10 units of padding
				[
					// Create a throbber (loading spinner) widget
					SNew(SThrobber)
						.Visibility(EVisibility::HitTestInvisible)	// Visible but doesn't block clicks
						.NumPieces(10)								// Number of spinning pieces
				]
		];

	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
