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



public:

	// 현재 LocalPlayer와의 거리를 계산해, 일정 거리 이상이면 보이지 않도록 함.
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	FORCEINLINE void SetMaxVisibleDistance(float InDistance) { MaxVisibleDistance = InDistance; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Visibility")
	float MaxVisibleDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Visibility")
	bool bIgnoreOwningLocalPlayer = true;

private:
	void UpdateDistanceVisibility();
};
