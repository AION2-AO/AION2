#include "Player/AOPlayerState.h"
#include "Data/DA_AbilitySet.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "AbilitySystemComponent.h"

AAOPlayerState::AAOPlayerState()
{
    SetNetUpdateFrequency(100.f);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributeSet = CreateDefaultSubobject<UAOAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAOPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

UAOAttributeSet* AAOPlayerState::GetAttributeSet() const
{
    return AttributeSet;
}

void AAOPlayerState::GiveCommonAbilities()
{
    if (bCommonAbilitiesGiven)
    {
        UE_LOG(LogTemp, Warning, TEXT("Common abilities already given."));
        return;
    }

    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("ASC is null."));
        return;
    }

    if (!CommonAbilitySet)
    {
        UE_LOG(LogTemp, Error, TEXT("CommonAbilitySet is null. Check BP_AOPlayerState setting."));
        return;
    }

    CommonAbilitySet->GiveToASC(ASC, CommonAbilityHandles);
    bCommonAbilitiesGiven = true;

    UE_LOG(LogTemp, Warning, TEXT("Common abilities given. Count: %d"), CommonAbilityHandles.Num());
}
