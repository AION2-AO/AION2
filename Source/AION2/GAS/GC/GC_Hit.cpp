#include "GC_Hit.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

bool UGC_Hit::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
    const UWorld* World = Target ? Target->GetWorld() : nullptr;
    if (!World)
    {
        return false;
    }

    if (World->GetNetMode() == NM_DedicatedServer)
    {
        return false;
    }

    const FVector SpawnLocation = Parameters.Location.IsNearlyZero() ? Target->GetActorLocation() : FVector(Parameters.Location);

    const FRotator SpawnRotation = Parameters.Normal.IsNearlyZero() ? FRotator::ZeroRotator : Parameters.Normal.Rotation();

    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, HitEffect, SpawnLocation, SpawnRotation, FVector(EffectScale));
    }

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(World, HitSound, SpawnLocation);
    }

    return true;
}
