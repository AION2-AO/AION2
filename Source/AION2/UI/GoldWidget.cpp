// Fill out your copyright notice in the Description page of Project Settings.


#include "GoldWidget.h"
#include "Components/TextBlock.h"

void UGoldWidget::SetGold(FString InGold)
{
	if (Gold)
	{
		Gold->SetText(FText::FromString(InGold));
	}
}

void UGoldWidget::NativeConstruct()
{
	if (Gold)
	{
		Gold->SetText(FText::FromString(TEXT("0")));
	}
}
