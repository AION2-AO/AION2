// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOMonsterHUDWidget_Targetable.h"

#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"

#include "Character/Monster/AOMonsterBase.h"
#include "UI/AODistanceDisplayWidget.h"

void UAOMonsterHUDWidget_Targetable::NativeConstruct()
{
	Super::NativeConstruct();

	if (Overlay_Root)
	{
		Overlay_Root->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAOMonsterHUDWidget_Targetable::BindToAbilitySystemActor(AActor* InActor)
{
	Super::BindToAbilitySystemActor(InActor);

	if (DistanceDisplayWidget)
	{
		DistanceDisplayWidget->BindToAbilitySystemActor(InActor);
	}
}

void UAOMonsterHUDWidget_Targetable::ClearBinding()
{
	Super::ClearBinding();

	if (DistanceDisplayWidget)
	{
		DistanceDisplayWidget->ClearBinding();
	}
}

void UAOMonsterHUDWidget_Targetable::SetMonsterHUDVisibility(bool bInVisiblity)
{
	ESlateVisibility NewVisibility = bInVisiblity ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (TargetableWidgetType== ETargetableWidgetType::FullScreen && Overlay_Root)
	{
		Overlay_Root->SetVisibility(NewVisibility);
		return;
	}

	if (Pb_HpBar)
	{
		Pb_HpBar->SetVisibility(NewVisibility);
	}

	if (Pb_GroggyBar)
	{
		Pb_GroggyBar->SetVisibility(NewVisibility);
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
