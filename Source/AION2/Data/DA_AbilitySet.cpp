#include "Data/DA_AbilitySet.h"

#include "AbilitySystemComponent.h"

void UDA_AbilitySet::GiveToASC(UAbilitySystemComponent* ASC, TArray<FGameplayAbilitySpecHandle>& OutHandles) const
{
    if (!ASC || !ASC->IsOwnerActorAuthoritative())
    {
        return;
    }

    for (const auto& Entry : Abilities)
    {
        if (!Entry.Ability)
        {
            continue;
        }

        FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel, Entry.AbilityID);
        FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
        OutHandles.Add(Handle);
    }
}
