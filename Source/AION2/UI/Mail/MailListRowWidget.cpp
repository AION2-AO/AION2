// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Mail/MailListRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Network/Struct.pb.h"
#include  "MainMailWidget.h"

void UMailListRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MailSelectButton)
	{
		MailSelectButton->OnClicked.AddUniqueDynamic(this, &UMailListRowWidget::OnRowClicked);
	}
}

void UMailListRowWidget::InitRowData(const FMailData& InData, class UMainMailWidget* InMainMailWidget)
{
	RowData = InData;
	MainMailWidget = InMainMailWidget;

	if (Title)
	{
		Title->SetText(FText::FromString(RowData.Title));
	}
	if (SenderName)
	{
		SenderName->SetText(FText::FromString(RowData.SenderName));
	}
	if (ExpiredDate)
	{
		ExpiredDate->SetText(FText::FromString(RowData.ExpiredDate));
	}
}

void UMailListRowWidget::OnRowClicked()
{
	if (MainMailWidget)
	{
		MainMailWidget->RequestDetailContentFromServer(RowData.MailUID);
	}
}
