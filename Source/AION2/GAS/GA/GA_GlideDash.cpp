#include "GAS/GA/GA_GlideDash.h"
#include "Character/Daeva/Daeva.h"
#include "Character/AOCharacterMovementComponent.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "UObject/UnrealType.h"

static bool IsBodyMontageConfigured(const ADaeva* Daeva, EMontageID MontageID)
{
	if (!Daeva) return false;

	FMapProperty* MapProp = CastField<FMapProperty>(ADaeva::StaticClass()->FindPropertyByName(TEXT("Montages")));
	if (MapProp)
	{
		FScriptMapHelper MapHelper(MapProp, MapProp->ContainerPtrToValuePtr<void>(Daeva));
		for (int32 i = 0; i < MapHelper.Num(); ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				const uint8* KeyPtr = MapHelper.GetKeyPtr(i);
				if (KeyPtr && *KeyPtr == static_cast<uint8>(MontageID))
				{
					const uint8* ValuePtr = MapHelper.GetValuePtr(i);
					if (ValuePtr)
					{
						UAnimMontage* Montage = *reinterpret_cast<UAnimMontage* const*>(ValuePtr);
						return Montage != nullptr;
					}
				}
			}
		}
	}
	return false;
}

static bool IsWingMontageConfigured(const ADaeva* Daeva, EMontageID MontageID)
{
	if (!Daeva) return false;

	FMapProperty* MapProp = CastField<FMapProperty>(ADaeva::StaticClass()->FindPropertyByName(TEXT("WingMontages")));
	if (MapProp)
	{
		FScriptMapHelper MapHelper(MapProp, MapProp->ContainerPtrToValuePtr<void>(Daeva));
		for (int32 i = 0; i < MapHelper.Num(); ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				const uint8* KeyPtr = MapHelper.GetKeyPtr(i);
				if (KeyPtr && *KeyPtr == static_cast<uint8>(MontageID))
				{
					const uint8* ValuePtr = MapHelper.GetValuePtr(i);
					if (ValuePtr)
					{
						UAnimMontage* Montage = *reinterpret_cast<UAnimMontage* const*>(ValuePtr);
						return Montage != nullptr;
					}
				}
			}
		}
	}
	return false;
}

void UGA_GlideDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    if (!Daeva)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAOCharacterMovementComponent* MoveComp = Cast<UAOCharacterMovementComponent>(Daeva->GetCharacterMovement());

    if (!MoveComp ||
        MoveComp->MovementMode != MOVE_Custom ||
        MoveComp->CustomMovementMode != static_cast<uint8>(EAOMovementMode::Glide))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MoveComp->StartGlideDash();

    UAnimMontage* BodyMontage = IsBodyMontageConfigured(Daeva, EMontageID::GlideDash) ? Daeva->GetMontageByID(EMontageID::GlideDash) : nullptr;
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, BodyMontage, 1.0f);
    if (Daeva->HasAuthority())
    {
        if (IsWingMontageConfigured(Daeva, EMontageID::GlideDash))
        {
            Daeva->Multicast_PlayWingMontage(EMontageID::GlideDash, 1.0f);
        }
    }

    MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_GlideDash::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GlideDash::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
