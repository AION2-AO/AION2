#include "Player/AOPlayerState.h"
#include "Data/DA_AbilitySet.h"

#include "AbilitySystemComponent.h"

AAOPlayerState::AAOPlayerState()
{
    SetNetUpdateFrequency(100.f);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
}

UAbilitySystemComponent* AAOPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAOPlayerState::GiveCommonAbilities()
{
    if (bCommonAbilitiesGiven)
    {
        return;
    }

    if (CommonAbilitySet)
    {
        CommonAbilitySet->GiveToASC(ASC, CommonAbilityHandles);

        bCommonAbilitiesGiven = true;
    }
}
