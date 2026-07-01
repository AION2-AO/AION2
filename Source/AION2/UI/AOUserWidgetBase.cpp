// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOUserWidgetBase.h"
#include "Player/AOPlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

void UAOUserWidgetBase::BindToASC(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		return;
	}

	if (BoundASC == InASC)
	{
		return;
	}

	UnbindFromASC();
	BoundASC = InASC;
}

void UAOUserWidgetBase::UnbindFromASC()
{
	BoundASC = nullptr;
}

void UAOUserWidgetBase::BindToPlayerState(AAOPlayerState* InPlayerState)
{
	if (!InPlayerState)
	{
		return;
	}

	BoundPlayerState = InPlayerState;
	BindToASC(BoundPlayerState->GetAbilitySystemComponent());
}

void UAOUserWidgetBase::BindToAbilitySystemActor(AActor* InActor)
{
	BoundAbilitySystemActor = InActor;
	BoundPlayerState = Cast<AAOPlayerState>(InActor);

	UAbilitySystemComponent* ASC = nullptr;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InActor))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}

	BindToASC(ASC);
}
