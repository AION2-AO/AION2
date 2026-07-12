#include "GAS/GA/Monster/Siliator/GA_Siliator_PT4.h"
#include "Character/AOCharacter.h"
#include "Physics/Collision.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Actor/SiliatorShuriken.h"

void UGA_Siliator_PT4::OnCheckAttackHitEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		return;
	}

	// #1. Search Target
	TArray<FHitResult> OutHitResults;

	const float AttackRange = AttackData.TraceData.Range;
	const float AttackRadius = AttackData.TraceData.Radius;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false, AOCharacter);

	FVector SweepStart = AOCharacter->GetActorTransform().TransformPosition(AttackData.TraceData.StartOffset);
	FVector SweepEnd = SweepStart + AttackData.TraceData.Direction.GetSafeNormal() * AttackRange;
	FVector CapsuleCenter = SweepStart + (SweepEnd - SweepStart) * 0.5f;

	bool bHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, SweepStart, SweepEnd, FQuat::Identity, CCHANNEL_ATTACK, FCollisionShape::MakeSphere(AttackRadius), Params);

	if (!bHitDetected)
	{
		return;
	}

	TArray<AAOCharacter*> Candidates;
	for (const FHitResult& HitResult : OutHitResults)
	{
		AAOCharacter* HitCharacter = Cast<AAOCharacter>(HitResult.GetActor());
		if (!IsValid(HitCharacter))
		{
			continue;
		}

		if (HitCharacter->IsDead())
		{
			continue;
		}

		if (!AOCharacter->IsEnemy(HitCharacter))
		{
			continue;
		}

		Candidates.Add(HitCharacter);
	}

	if (Candidates.Num() == 0)
	{
		return;
	}

	AAOCharacter* TargetA = nullptr;
	AAOCharacter* TargetB = nullptr;

	if (Candidates.Num() == 1)
	{
		TargetA = Candidates[0];
		TargetB = Candidates[0];
	}
	else
	{
		const int32 FirstIndex = FMath::RandRange(0, Candidates.Num() - 1);
		TargetA = Candidates[FirstIndex];

		Candidates.RemoveAtSwap(FirstIndex);

		const int32 SecondIndex = FMath::RandRange(0, Candidates.Num() - 1);
		TargetB = Candidates[SecondIndex];
	}

	// #2. Spawn Shurikens
	AAOCharacter* Boss = AOCharacter;
	if (!IsValid(Boss) || !ShurikenClass)
	{
		return;
	}

	const FVector BossLocation = Boss->GetActorLocation();
	const FVector Forward = Boss->GetActorForwardVector();
	const FVector Right = Boss->GetActorRightVector();

	const FVector BaseLocation = BossLocation + Forward * ShurikenForwardOffset + FVector(0.f, 0.f, ShurikenHeightOffset);

	const FVector LeftSpawnLocation = BaseLocation - Right * ShurikenSideOffset;
	const FVector RightSpawnLocation = BaseLocation + Right * ShurikenSideOffset;
	const FRotator SpawnRotation = Boss->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Boss;
	SpawnParams.Instigator = Cast<APawn>(Boss);

	ASiliatorShuriken* LeftShuriken = GetWorld()->SpawnActor<ASiliatorShuriken>(ShurikenClass, LeftSpawnLocation, SpawnRotation, SpawnParams);

	ASiliatorShuriken* RightShuriken = GetWorld()->SpawnActor<ASiliatorShuriken>(ShurikenClass, RightSpawnLocation, SpawnRotation, SpawnParams);

	if (LeftShuriken)
	{
		LeftShuriken->SetTarget(TargetA, AttackData);
	}

	if (RightShuriken)
	{
		RightShuriken->SetTarget(TargetB, AttackData);
	}
}
