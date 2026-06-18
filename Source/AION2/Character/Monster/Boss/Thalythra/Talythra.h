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

	void Set_RotationAble(bool _bRotationOnOff) { bRotationAble = _bRotationOnOff; }

#pragma region
	void DoFireProjectile();
	void DoFireProjectile_2();
	void DoFireProjectile_3();

#pragma endregion 

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


public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Phase")
	ETalythraPhase Phase;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "State")
	ETalythraState State;




private:
	UPROPERTY(VisibleAnywhere)
	int FireCount = 1;

	UPROPERTY(VisibleAnywhere)
	bool bRotationAble = false; 

};
