// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */

class UTexture2D;

class LOADINGSCREENMODULE_API SLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreen)
		: _BackgroundTexture(nullptr)	// Default constructor initializes texture to null
	{}
		SLATE_ARGUMENT(UTexture2D*, BackgroundTexture)	// Declares texture parameter for widget.

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	// Holds reference to background texture asset
	UTexture2D* BackgroundTexture;

	// Shared pointer to brush used for rendering background.
	TSharedPtr<FSlateBrush> BackgroundBrush;
};
