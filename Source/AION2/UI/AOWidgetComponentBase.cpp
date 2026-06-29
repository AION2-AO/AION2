// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOWidgetComponentBase.h"

#include "UI/AOUserWidgetBase.h"

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"



UAOWidgetComponentBase::UAOWidgetComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAOWidgetComponentBase::InitWidget()
{
	Super::InitWidget();

	// Super::InitWidget() 상위 로직을 따라가보면, 
	// 함수 실행 과정에서 CreateWidget을 통해 Widget이 생성됨.
	// 그 이후에 여기가 실행됨. 따라서 Widget 초기화를 보장 받을 수 있음.
	UAOUserWidgetBase* AOUserWidget = Cast<UAOUserWidgetBase>(GetWidget());
	if (AOUserWidget)
	{
		// AbilitySystem이 있으면 Bind.
		AOUserWidget->BindToAbilitySystemActor(GetOwner());
	}
}
