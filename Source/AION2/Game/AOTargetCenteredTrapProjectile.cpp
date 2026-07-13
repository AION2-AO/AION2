#include "Game/AOTargetCenteredTrapProjectile.h"
#include "Character/AOCharacter.h"

FVector AAOTargetCenteredTrapProjectile::GetTrapSpawnLocation(AAOCharacter* HitCharacter, const FHitResult& SweepResult) const
{
	if (!IsValid(HitCharacter))
	{
		return GetActorLocation();
	}

	return HitCharacter->GetActorLocation();
}
