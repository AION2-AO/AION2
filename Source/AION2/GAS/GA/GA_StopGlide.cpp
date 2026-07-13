#include "GAS/GA/GA_StopGlide.h"
#include "GAS/GA/AT/AT_WaitLanding.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Character/Daeva/Daeva.h"

#include "GameFramework/Character.h"
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

void UGA_StopGlide::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    UAnimMontage* BodyMontage = IsBodyMontageConfigured(Daeva, EMontageID::StopGlide) ? Daeva->GetMontageByID(EMontageID::StopGlide) : nullptr;
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, BodyMontage, 1.5f);
    if (Daeva->HasAuthority())
    {
        Daeva->SetWingVisibilityOnServer(true);
        if (IsWingMontageConfigured(Daeva, EMontageID::Glide))
        {
            Daeva->Multicast_PlayWingMontage(EMontageID::Glide, 1.8f);
        }
    }

    MontageTask->OnCompleted.AddDynamic(this, &UGA_StopGlide::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_StopGlide::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_StopGlide::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_StopGlide::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_StopGlide::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get()))
    {
        if (Daeva->HasAuthority())
        {
            Daeva->SetWingVisibilityOnServer(false);
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_StopGlide::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_StopGlide::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
