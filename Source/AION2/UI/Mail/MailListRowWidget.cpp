// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Mail/MailListRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"
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

	FSlateColor TextColor = RowData.bIsRead ? ReadColor : UnreadColor;

	if (Title)
	{
		Title->SetText(FText::FromString(RowData.Title));
		Title->SetColorAndOpacity(TextColor);
	}
	if (SenderName)
	{
		SenderName->SetText(FText::FromString(RowData.SenderName));
		SenderName->SetColorAndOpacity(TextColor);
	}
	if (ExpiredDate)
	{
		ExpiredDate->SetText(FText::FromString(RowData.ExpiredDate));
		ExpiredDate->SetColorAndOpacity(TextColor);
	}
}

void UMailListRowWidget::OnRowClicked()
{
	if (MainMailWidget)
	{
		MainMailWidget->RequestDetailContentFromServer(RowData.MailUID);
	}
}
