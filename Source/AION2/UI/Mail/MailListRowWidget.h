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
	UPROPERTY(meta = (BindWidget))
	UButton* MailSelectButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SenderName;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Title;

	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* ExpiredDate;

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

