// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/AOCharacter.h"
#include "AI/Types/AIPhaseFlag.h"
#include "Talythra.generated.h"



UCLASS()
class AION2_API ATalythra : public AAOCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATalythra(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE void Set_Phase(ETalythraPhase _PhaseFlag) { Phase = _PhaseFlag; }
	FORCEINLINE void Set_State(ETalythraState _StateFlag) { State = _StateFlag; }

	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void Multicast_PlayAttackMontage(class UAnimMontage* MontageToPlay);

	void FireProjectile(); 
	void TurnToTarget(); 

	void Set_RotationAble(bool RotationOnOff)   { RotationAble = RotationOnOff; }
	void Set_AttackLineRenderOnOff(bool _OnOff) { AttackLineRenderOnOff = _OnOff; }


	// Charge Attack 관련 
	
	FORCEINLINE void Set_LockPevis(bool _bLock) { bLockPelvis = _bLock; }
	FORCEINLINE void Set_ChargeAttackMove(bool _bAttack) { bChargeAttack = _bAttack;}
	FORCEINLINE void Set_ChargeAttackDir(FVector _vector) { ChargeDirection = _vector; }
	void StartChargeMove();
	void EndChargeMove();

	

#pragma region
	void DoFireProjectile();
	void DoFireProjectile_2();
	void DoFireProjectile_3();

#pragma endregion 

	FORCEINLINE virtual class UAnimMontage* GetChargeAttackActionMontage() const { return ChargeAttackMontage; }


protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 투사체 발사 이펙트. 
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMuzzleEffect(FVector SpawnLocation, FRotator SpawnRotation);


#pragma region Decal Component 
	// Attack Line On/Off 기능. 
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AttackLine_Pattern_1(); 

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AttackLine_Pattern_1_Off();



	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AttackLine_Pattern_2();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AttackLine_Pattern_2_Off();


	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AttackLine_Pattern_3();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AttackLine_Pattern_3_Off();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SetChargeMovementParams(bool bChargeMode);
	// EFFECT 관련 
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<class ATalythraProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FName ProjectileSocketName = TEXT("WP_Center");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> LanchMuzzleEffect; 

#pragma region Decal_Attack_Line
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DecalEffect")
	TArray<TObjectPtr<class USceneComponent>> ArrayDecalSceneComponent;

#pragma endregion 


	// GAS 관련
protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TMap<FName, TSubclassOf<class UGameplayAbility>> HasAbilities; 




	// 몽타주 관련 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> ChargeAttackMontage; 





	// 페이즈 및 상태 관련 
public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Phase")
	ETalythraPhase Phase;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "State")
	ETalythraState State;


	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bLockPelvis = false;

private:
	UPROPERTY(VisibleAnywhere)
	int FireCount = 1;

	UPROPERTY(VisibleAnywhere)
	bool RotationAble = false; 

	UPROPERTY(VisibleAnywhere)
	bool AttackLineRenderOnOff = false; 


	UPROPERTY()
	FVector ChargeDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	bool bChargeAttack = false;

	UPROPERTY(VisibleAnywhere)
	bool bMovelAccel = false;


};
