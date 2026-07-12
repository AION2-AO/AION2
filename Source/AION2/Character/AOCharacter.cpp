#include "Character/AOCharacter.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Physics/Collision.h"
#include "GAS/AOGameplayTags.h"
#include "Player/AOPlayerController.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Actor/AOProjectile.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Character/Daeva/Daeva.h"


#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"

AAOCharacter::AAOCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAOCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AAOCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetupOwnedAttackColliders();
	}
}

void AAOCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RefreshOwnedAttackColliderOverlaps();
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
		AAOCharacter* HitActor = Cast<AAOCharacter>(HitResult.GetActor());
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor->IsDead())
		{
			continue;
		}

		if (!IsEnemy(HitActor))
		{
			continue;
		}

		OnAttackSucceeded(AttackData, HitActor, HitResult, bDidShakeCamera);
	}
}

void AAOCharacter::CheckAttackHitSector(const FAttackData& AttackData, const float SafeAngle)
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

	FVector BackVector = -GetActorForwardVector();
	BackVector.Z = 0.f;
	BackVector.Normalize();

	for (const FHitResult& HitResult : OutHitResults)
	{
		AAOCharacter* HitActor = Cast<AAOCharacter>(HitResult.GetActor());
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor->IsDead())
		{
			continue;
		}

		if (!IsEnemy(HitActor))
		{
			continue;
		}

		FVector ToTarget = HitActor->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;
		ToTarget.Normalize();

		const float Dot = FVector::DotProduct(BackVector, ToTarget);
		if (Dot > FMath::Cos(FMath::DegreesToRadians(SafeAngle)))
		{
			continue;
		}

		OnAttackSucceeded(AttackData, HitActor, HitResult, bDidShakeCamera);
	}
}

void AAOCharacter::CheckIsInSafeZone(const FAttackData& AttackData, uint8 SafeColor)
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
		ADaeva* HitActor = Cast<ADaeva>(HitResult.GetActor());
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor->IsDead())
		{
			continue;
		}

		if (!IsEnemy(HitActor))
		{
			continue;
		}

		uint8 CurrentColor = static_cast<uint8>(HitActor->Get_CurrentDaevaHasSheildColor());

		if (CurrentColor == SafeColor)
		{
			continue;
		}

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


	// H.Y
	AAOMonsterBase* MonsterTarget = Cast<AAOMonsterBase>(this);

	if (MonsterTarget && MonsterTarget->DungeonBossIndex >= 1 && MonsterTarget->DungeonBossIndex <= 3 && GroggyDamageEffect)
	{
		FGameplayEffectSpecHandle GroggySpecHandle = SourceASC->MakeOutgoingSpec(GroggyDamageEffect, 1.0f, Context);

		FGameplayTagContainer OwnedTags;	
		MonsterTarget->GetAbilitySystemComponent()->GetOwnedGameplayTags(OwnedTags);	

		// 보스가 기믹상태일시에는 데미지 x 
		if (GroggySpecHandle.IsValid() && OwnedTags.HasTagExact(GIMMICK_MONSTER) == false)
		{
			const FGameplayTag GroggyDamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.GroggyDamage"));
			float GroggyDamageAmount = BaseGroggyDamage;
			if (AttackData.bRestoreManaOnHit)
			{
				GroggyDamageAmount *= 0.5;
			}

			GroggySpecHandle.Data->SetSetByCallerMagnitude(GroggyDamageTag, -GroggyDamageAmount);
			SourceASC->ApplyGameplayEffectSpecToTarget(*GroggySpecHandle.Data.Get(), TargetASC);
			
		}
	}

	//

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Damage]"));
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Damage")),	-FinalDamage);

	const float OldHealth =	TargetASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	const float NewHealth =	TargetASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());

	const  float NewGroggy = TargetASC->GetNumericAttribute(UAOAttributeSet::GetGroggyAttribute());

	UE_LOG(LogTemp, Warning, TEXT("[Damage] %s -> %s | ATK: %.1f | DEF: %.1f | Mult: %.2f | Final: %.2f | HP: %.1f -> %.1f | Groggy : %.1f"),
		*GetNameSafe(DamageCauser),
		*GetNameSafe(this),
		AttackPower,
		Defense,
		Multiplier,
		FinalDamage,
		OldHealth,
		NewHealth,
		NewGroggy
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

void AAOCharacter::SetOwnedAttackCollidersCollisionEnabled(const FAttackData& InAttackData, bool bEnabled)
{
	if (bEnabled)
	{
		HitActors.Reset();
	}

	bIsRefreshOwnedAttackColliders = bEnabled;
	CurrentOwendAttackCollidersAttackData = InAttackData;
}

void AAOCharacter::RefreshOwnedAttackColliderOverlaps()
{
	if (!HasAuthority() || !bIsRefreshOwnedAttackColliders)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	for (UPrimitiveComponent* Collider : OwnedAttackColliders)
	{
		if (!Collider)
		{
			continue;
		}

		TArray<FOverlapResult> Results;

		const FVector Location = Collider->GetComponentLocation();
		const FQuat Rotation = Collider->GetComponentQuat();

		bool bHit = false;

		if (UBoxComponent* Box = Cast<UBoxComponent>(Collider))
		{
			bHit = World->OverlapMultiByObjectType(Results, Location, Rotation, ObjectParams, FCollisionShape::MakeBox(Box->GetScaledBoxExtent()), QueryParams);

			if (CVarDrawAttackTrace.GetValueOnGameThread())
			{
				DrawDebugBox(World, Location, Box->GetScaledBoxExtent(), Rotation, FColor::Red, false, 0.1f, 0, 2.f);
			}
		}
		else if (USphereComponent* Sphere = Cast<USphereComponent>(Collider))
		{
			const float Radius = Sphere->GetScaledSphereRadius();

			bHit = World->OverlapMultiByObjectType(Results, Location, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Radius), QueryParams);

			if (CVarDrawAttackTrace.GetValueOnGameThread())
			{
				DrawDebugSphere(World, Location, Radius, 16, FColor::Red, false, 0.1f, 0, 2.f);
			}
		}
		else if (UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Collider))
		{
			const float Radius = Capsule->GetScaledCapsuleRadius();
			const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

			bHit = World->OverlapMultiByObjectType(Results, Location, Rotation, ObjectParams, FCollisionShape::MakeCapsule(Radius, HalfHeight), QueryParams);

			if (CVarDrawAttackTrace.GetValueOnGameThread())
			{
				DrawDebugCapsule(World, Location, HalfHeight, Radius, Rotation, FColor::Red, false, 0.1f, 0, 2.f);
			}
		}

		if (!bHit)
		{
			continue;
		}

		for (const FOverlapResult& Result : Results)
		{
			AAOCharacter* HitCharacter = Cast<AAOCharacter>(Result.GetActor());
			if (!HitCharacter || HitCharacter == this)
				continue;

			if (HitCharacter->IsDead())
				continue;

			if (!IsEnemy(HitCharacter))
				continue;

			if (HitActors.Contains(HitCharacter))
				continue;

			HitActors.Add(HitCharacter);

			bool bDidCameraShake = false;
			OnAttackSucceeded(CurrentOwendAttackCollidersAttackData, HitCharacter, FHitResult(), bDidCameraShake);
		}
	}
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

void AAOCharacter::OnRep_IsDead()
{
}

void AAOCharacter::SetupOwnedAttackColliders()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		if (!Comp || !Comp->ComponentHasTag(TEXT("OwnedAttackCollider")))
		{
			continue;
		}

		OwnedAttackColliders.Add(Comp);
	}
}
