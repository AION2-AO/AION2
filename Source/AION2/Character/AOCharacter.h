#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interface/CombatInterface.h"
#include "AOCharacter.generated.h"


UCLASS(Abstract)
class AION2_API AAOCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAOCharacter(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DrawDebugCapsuleCollider(const FVector& CapsuleOrigin, const float CapsuleHalfHeight, const float AttackRadius, const FColor DrawColor);

public:
	virtual void SearchTarget();

public:
	virtual void CheckAttackHit(const FAttackData& AttackData) override;

protected:
	virtual void InitGAS();
	virtual void ClearGAS();

protected:
	virtual void OnAttackSucceeded(const FAttackData& AttackData, AActor* HitActor, const FHitResult& HitResult, bool& bDidShakeCamera);
	virtual void TakeDamageAO(const FAttackData& AttackData, AAOCharacter* DamageCauser);

protected:
	bool IsEnemy(AActor* TargetActor);
	void DrawDebugCapsuleCollider(const FVector& CapsuleOrigin, const float CapsuleHalfHeight, const float AttackRadius, const FColor DrawColor);

public:
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE AAOCharacter* GetCurrentTarget() const { return CurrentTarget; }

public:
	FORCEINLINE void SetCurrentTarget(AAOCharacter* NewTarget) { CurrentTarget = NewTarget; }

protected:
	UPROPERTY()
	TObjectPtr<AAOCharacter> CurrentTarget;

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAbilitySystemComponent> ASC;

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffect;
};