#include "Character/Daeva/AOTrapZone.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/AOCharacter.h"
#include "Components/SphereComponent.h"
#include "GameplayEffect.h"
#include "TimerManager.h"


AAOTrapZone::AAOTrapZone()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DamageCollision = CreateDefaultSubobject<USphereComponent>("DamageCollision");
	DamageCollision->SetupAttachment(Root);

	DamageCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageCollision->SetCollisionObjectType(ECC_WorldDynamic);
	DamageCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAOTrapZone::BeginPlay()
{
	Super::BeginPlay();
	
	DamageCollision->SetSphereRadius(TrapRadius);

	if (!HasAuthority())
	{
		return;
	}

	SetLifeSpan(TrapDuration);

	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AAOTrapZone::ApplyTrapDamage, DamageInterval, true);
}

void AAOTrapZone::ApplyTrapDamage()
{
	if (!HasAuthority() || !TrapDamageEffect)
	{
		return;
	}

	AAOCharacter* DamageCauser = Cast<AAOCharacter>(GetOwner());

	if (!DamageCauser || DamageCauser->IsDead())
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(DamageCauser);

	if (!SourceASC)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;

	DamageCollision->GetOverlappingActors(OverlappingActors, AAOCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		AAOCharacter* TargetCharacter = Cast<AAOCharacter>(Actor);

		if (!TargetCharacter || TargetCharacter->IsDead())
		{
			continue;
		}

		if (!DamageCauser->IsEnemy(TargetCharacter))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetCharacter);

		if (!TargetASC)
		{
			continue;
		}


		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();

		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =	SourceASC->MakeOutgoingSpec(TrapDamageEffect,1.f,ContextHandle);

		if (!SpecHandle.IsValid())
		{
			continue;
		}

		SpecHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag,DamageAmount);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(),TargetASC);
	}
}
