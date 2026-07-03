#include "Animation/Daeva/DaevaAnimInstance.h"
#include "Character/AOCharacter.h"
#include "GAS/AOGameplayTags.h"

#include "AbilitySystemComponent.h"

void UDaevaAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn)
    {
        return;
    }

    AAOCharacter* AOCharacter = Cast<AAOCharacter>(TryGetPawnOwner());
    if (!AOCharacter)
    {
        return;
    }

    if (UAbilitySystemComponent* ASC = AOCharacter->GetAbilitySystemComponent())
    {
        bIsGliding = ASC->HasMatchingGameplayTag(STATE_GLIDING);
        bIsCombat = ASC->HasMatchingGameplayTag(STATE_COMBAT);
    }
}
