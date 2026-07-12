#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interface/CombatInterface.h"
#include "AOCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDead, AActor*, DeadActor);

UCLASS(Abstract)
class AION2_API AAOCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAOCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DrawDebugCapsuleCollider(const FVector& CapsuleOrigin, const float CapsuleHalfHeight, const float AttackRadius, const FColor DrawColor);

public:
	virtual void SearchTarget();
	virtual void TeleportBackToTarget();

public:
	virtual void CheckAttackHit(const FAttackData& AttackData);
	virtual void CheckAttackHitSector(const FAttackData& AttackData, const float SafeAngle);
	virtual void CheckIsInSafeZone(const FAttackData& AttackData, uint8 SafeColor);
	virtual void OnAttackSucceeded(const FAttackData& AttackData, AActor* HitActor, const FHitResult& HitResult, bool& bDidShakeCamera);
	virtual void TakeDamageAO(const FAttackData& AttackData, const FHitResult& HitResult, AAOCharacter* DamageCauser);
	virtual void SpawnAttackProjectile(const FAttackData& AttackData, TSubclassOf<class AAOProjectile> ProjectileClass, const FName& SpawnSocket);
	void SetOwnedAttackCollidersCollisionEnabled(const FAttackData& InAttackData, bool bEnabled);
	void RefreshOwnedAttackColliderOverlaps();
	bool IsEnemy(AActor* TargetActor);

protected:
	void DrawDebugCapsuleCollider(const FVector& CapsuleOrigin, const float CapsuleHalfHeight, const float AttackRadius, const FColor DrawColor);

protected:
	virtual void InitGAS();
	virtual void ClearGAS();

public:
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE AAOCharacter* GetCurrentTarget() const { return CurrentTarget; }
	virtual TArray<class USkeletalMeshComponent*> GetAllMeshes();

public:
	FORCEINLINE void SetCurrentTarget(AAOCharacter* NewTarget) { CurrentTarget = NewTarget; }
	// 선환 추가 
	FOnCharacterDead OnPlayerDead;

protected:
	UFUNCTION()
	virtual void OnRep_IsDead();

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

public:
	bool IsDead() const { return bIsDead; }

private:
	void SetupOwnedAttackColliders();

protected:
	UPROPERTY()
	TObjectPtr<AAOCharacter> CurrentTarget;

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAbilitySystemComponent> ASC;

	//H.Y
	UPROPERTY(EditDefaultsOnly, Category="GAS|Combat")
	TSubclassOf<UGameplayEffect> GroggyDamageEffect;

	const float BaseGroggyDamage = 10.0f;
	//

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;

private:
	UPROPERTY()
	bool bIsRefreshOwnedAttackColliders = false;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	UPROPERTY()
	TArray<TObjectPtr<UPrimitiveComponent>> OwnedAttackColliders;

	UPROPERTY()
	FAttackData CurrentOwendAttackCollidersAttackData;
};