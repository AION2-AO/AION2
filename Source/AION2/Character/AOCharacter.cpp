#include "Character/AOCharacter.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Physics/Collision.h"
#include "GAS/AOGameplayTags.h"
#include "Player/AOPlayerController.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Actor/AOProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

AAOCharacter::AAOCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAOCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAOCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAOCharacter, bIsDead);
}

void AAOCharacter::Multicast_DrawDebugCapsuleCollider_Implementation(const FVector& CapsuleOrigin, const float CapsuleHalfHeight, const float AttackRadius, const FColor DrawColor)
{
	DrawDebugCapsuleCollider(CapsuleOrigin, CapsuleHalfHeight, AttackRadius, DrawColor);
}

void AAOCharacter::SearchTarget()
{
}

void AAOCharacter::TeleportBackToTarget()
{
}

void AAOCharacter::CheckAttackHit(const FAttackData& AttackData)
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<FHitResult> OutHitResults;

	const float AttackRange = AttackData.TraceData.Range;
	const float AttackRadius = AttackData.TraceData.Radius;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false, this);

	FVector SweepStart = GetActorTransform().TransformPosition(AttackData.TraceData.StartOffset);
	FVector SweepEnd = SweepStart + AttackData.TraceData.Direction.GetSafeNormal() * AttackRange;
	FVector CapsuleCenter = SweepStart + (SweepEnd - SweepStart) * 0.5f;

	bool bHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, SweepStart, SweepEnd, FQuat::Identity, CCHANNEL_ATTACK, FCollisionShape::MakeSphere(AttackRadius), Params);

	if (CVarDrawAttackTrace.GetValueOnGameThread())
	{
		const float CapsuleHalfHeight = AttackRange * 0.5f;
		FColor DrawColor = bHitDetected ? FColor::Green : FColor::Red;
		Multicast_DrawDebugCapsuleCollider(CapsuleCenter, CapsuleHalfHeight, AttackRadius, DrawColor);
	}

	if (!bHitDetected)
	{
		return;
	}

	bool bDidShakeCamera = false;

	for (const FHitResult& HitResult : OutHitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor))
		{
			continue;
		}
		
		//if (!IsEnemy(HitActor))
		//{
		//	continue;
		//}

		OnAttackSucceeded(AttackData, HitActor, HitResult, bDidShakeCamera);
	}
}

void AAOCharacter::OnAttackSucceeded(const FAttackData& AttackData, AActor* HitActor, const FHitResult& HitResult, bool& bDidShakeCamera)
{
	AAOCharacter* Target = Cast<AAOCharacter>(HitActor);
	if (!Target)
	{
		return;
	}

	Target->TakeDamageAO(AttackData, HitResult, this);
}

void AAOCharacter::TakeDamageAO(const FAttackData& AttackData, const FHitResult& HitResult, AAOCharacter* DamageCauser)
{
	UAbilitySystemComponent* SourceASC = DamageCauser->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = GetAbilitySystemComponent();
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	const float AttackPower = SourceASC->GetNumericAttribute(UAOAttributeSet::GetAttackPowerAttribute());
	const float Defense = TargetASC->GetNumericAttribute(UAOAttributeSet::GetDefenseAttribute());

	const float Multiplier = AttackData.DamageMultiplier;
	const float BaseDamage = AttackPower * Multiplier;

	const float FinalDamage = FMath::Max(1.0f, BaseDamage * (100.0f / (100.0f + Defense)));

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(DamageCauser);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, Context);

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Damage]"));
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(TEXT("Data.Damage")),
		-FinalDamage
	);

	const float OldHealth =
		TargetASC->GetNumericAttribute(
			UAOAttributeSet::GetHealthAttribute()
		);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	const float NewHealth =
		TargetASC->GetNumericAttribute(
			UAOAttributeSet::GetHealthAttribute()
		);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Damage] %s -> %s | ATK: %.1f | DEF: %.1f | Mult: %.2f | Final: %.2f | HP: %.1f -> %.1f"),
		*GetNameSafe(DamageCauser),
		*GetNameSafe(this),
		AttackPower,
		Defense,
		Multiplier,
		FinalDamage,
		OldHealth,
		NewHealth
	);

	if (AttackData.HitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		CueParams.Instigator = this;
		CueParams.EffectCauser = this;
		TargetASC->ExecuteGameplayCue(AttackData.HitGameplayCueTag, CueParams);
	}
}

void AAOCharacter::SpawnAttackProjectile(const FAttackData& AttackData, TSubclassOf<class AAOProjectile> ProjectileClass, const FName& SpawnSocket)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ProjectileClass)
	{
		return;
	}

	const FTransform SocketTransform = GetMesh()->GetSocketTransform(SpawnSocket);

	FVector Direction = GetActorForwardVector();
	if (IsValid(CurrentTarget))
	{
		Direction = (CurrentTarget->GetActorLocation() - SocketTransform.GetLocation()).GetSafeNormal();
	}

	FTransform SpawnTransform = SocketTransform;
	SpawnTransform.SetRotation(Direction.Rotation().Quaternion());

	AAOProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAOProjectile>(ProjectileClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		return;
	}

	Projectile->InitProjectile(AttackData, this, CurrentTarget, Direction);
	Projectile->FinishSpawning(SpawnTransform);
}

bool AAOCharacter::IsEnemy(AActor* TargetActor)
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(TargetActor);
	if (!AOCharacter)
	{
		return false;
	}

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = AOCharacter->GetAbilitySystemComponent();
	if (!MyASC || !TargetASC)
	{
		return false;
	}

	if (MyASC->HasMatchingGameplayTag(TEAM_DAEVA))
	{
		return TargetASC->HasMatchingGameplayTag(TEAM_MONSTER);
	}

	if (MyASC->HasMatchingGameplayTag(TEAM_MONSTER))
	{
		return TargetASC->HasMatchingGameplayTag(TEAM_DAEVA);
	}

	return false;
}

void AAOCharacter::DrawDebugCapsuleCollider(const FVector& CapsuleOrigin, const float CapsuleHalfHeight, const float AttackRadius, const FColor DrawColor)
{
#if ENABLE_DRAW_DEBUG
	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 1.0f);
#endif
}

void AAOCharacter::InitGAS()
{
}

void AAOCharacter::ClearGAS()
{
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

TArray<USkeletalMeshComponent*> AAOCharacter::GetAllMeshes()
{
	TArray<USkeletalMeshComponent*> Meshes;
	Meshes.Add(GetMesh());

	return Meshes;
}
