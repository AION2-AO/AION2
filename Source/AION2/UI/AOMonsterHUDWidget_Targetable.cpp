// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOMonsterHUDWidget_Targetable.h"

#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"

void UAOMonsterHUDWidget_Targetable::SetHiddenUIVisibliblity(bool bInVisiblity)
{
	ESlateVisibility NewVisibility = bInVisiblity ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (Pb_HpBar)
	{
		Pb_HpBar->SetVisibility(NewVisibility);
	}

	if (Pb_StaminaBar)
	{
		Pb_StaminaBar->SetVisibility(NewVisibility);
	}

	if (Image_NameBG)
	{
		Image_NameBG->SetVisibility(NewVisibility);
	}

	if (CanvasPanel_TargetArrowRight)
	{
		CanvasPanel_TargetArrowRight->SetVisibility(NewVisibility);
	}

	if (CanvasPanel_TargetArrowLeft)
	{
		CanvasPanel_TargetArrowLeft->SetVisibility(NewVisibility);
	}
}
