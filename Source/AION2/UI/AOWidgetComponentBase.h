// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "AOWidgetComponentBase.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAOWidgetComponentBase : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UAOWidgetComponentBase();
	
protected:
	// Widget이 초기화될 때 호출되는 함수.
	virtual void InitWidget() override;

	
};
