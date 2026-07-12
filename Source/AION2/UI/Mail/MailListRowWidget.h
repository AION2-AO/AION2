// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MailData.h"
#include "MailListRowWidget.generated.h"

/**
 * 
 */

class UButton;
class UTextBlock;

UCLASS()
class AION2_API UMailListRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MailSelectButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SenderName;
	
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Title;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ExpiredDate;

	UPROPERTY(EditDefaultsOnly, Category = "Mail UI")
	FSlateColor UnreadColor = FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));

	UPROPERTY(EditDefaultsOnly, Category = "Mail UI")
	FSlateColor ReadColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

private:
	FMailData RowData;
	
	UPROPERTY()
	class UMainMailWidget* MainMailWidget;
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnRowClicked();
	
public:
	void InitRowData(const FMailData& InData, class UMainMailWidget* InMainMailWidget);

};

