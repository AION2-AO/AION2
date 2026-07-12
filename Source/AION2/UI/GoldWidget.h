// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoldWidget.generated.h"

/**
 *
 */
UCLASS()
class AION2_API UGoldWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetGold(FString InGold);

protected:
	virtual void NativeConstruct() override;

private:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Gold;

};
