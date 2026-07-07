// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/Boss/Lutalis/Lutalis.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

ALutalis::ALutalis(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DungeonBossIndex = 2;
}

void ALutalis::BeginPlay()
{
	Super::BeginPlay();

	InitAttributeSet();
}

void ALutalis::InitAttributeSet()
{
	Super::InitAttributeSet();

	AttributeSet->InitHealth(5000.f);
	AttributeSet->InitMaxHealth(5000.f);

	AttributeSet->InitStamina(100.f);
	AttributeSet->InitMaxStamina(100.f);
}