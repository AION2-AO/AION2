// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOUserWidgetBase.generated.h"

/**
 * 
 */

class AAOPlayerState;
class UAbilitySystemComponent;

UCLASS()
class AION2_API UAOUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	/*
	* 공통 Bind/Unbind 흐름 제공
	* Server에서는 절대 생성되지 않도록 함.
	*/
	virtual void BindToASC(UAbilitySystemComponent* InASC);
	virtual void UnbindFromASC();
	virtual void ClearBinding();

	// 기존 Player 코드 호환용. 내부에서 BindToASC 호출. 
	virtual void BindToPlayerState(AAOPlayerState* InPlayerState);

	virtual void BindToAbilitySystemActor(AActor* InActor);

protected:
	// Player State 저장 (Player 전용 보조 참조로, Monster Binding 시 null).
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AAOPlayerState> BoundPlayerState;

	// Ability System의 Owning Actor.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> BoundAbilitySystemActor;

	// ASC 저장
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> BoundASC;
	
};
