// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOStoreWidget.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAOStoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnHealPurchase();

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Slot")
	class UButton* Button_HealPurchase;
	
};
