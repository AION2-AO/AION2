// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "AOMonsterBase.generated.h"


class UAOWidgetComponentBase;
class UAOMonsterHUDWidget_Targetable;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeadMontageEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGroggyMontageEnd);

UCLASS()
class AION2_API AAOMonsterBase : public AAOCharacter , public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAOMonsterBase(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void InitGAS() override;
	virtual void ClearGAS() override;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// Navigation Mesh의 범위 밖에 이동인지 아닌지를 판단하는 함수 
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	bool CanMoveOnNavMesh(const FVector Direction, float Distance);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Montage")
	UAnimMontage* GetMontageByTag(const FGameplayTag& MontageTag) const;


	// 몬스터 AI 컨트롤러에서 Phase 설정 뒤 Replicate. 
	FORCEINLINE void Set_Phase(FGameplayTag _PhaseFlag) { Phase = _PhaseFlag; }
	FORCEINLINE void Set_State(FGameplayTag _StateFlag) { State = _StateFlag; }

	TObjectPtr<class UAOAttributeSet> GetAttributeSet() { return AttributeSet; }

	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(TeamID); }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { TeamID = NewTeamID.GetId(); }


protected:
	// AttributeSet
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAOAttributeSet> AttributeSet;

	virtual void InitAttributeSet();


private:
	// GAS 관련 
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> MontageMap;




	// 몬스터 페이즈 및 상태 설정 

	//  EditAnywhere 에디터 Details 패널에서 값을 수정할 수 있냐를 정합니다.
	//  BlueprintReadWrite 루프린트 그래프에서 Get/Set 노드로 접근할 수 있냐를 정합니다.

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "AI State", meta = (AllowPrivateAccess = "true"))
	FGameplayTag Phase;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "AI State", meta = (AllowPrivateAccess = "true"))
	FGameplayTag State;



	// AODungeonGameMode -> TEST
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void HandleBossDeath();

	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDA_AbilitySet> AbilitySet;



public:
	UPROPERTY(BlueprintAssignable, Category = "Monster|Death")
	FOnDeadMontageEnd OnDeadMontageEnd;

	UPROPERTY(BlueprintAssignable, Category = "Monster|Groggy")
	FOnGroggyMontageEnd OnGroggyMontageEnd;


	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void HandleBossDeathMontageEnd();


public:
	/*
	* 0 = 일반 몬스터
	* 1 = b1
	* 2 = b2
	* 3 = b3
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	int32 DungeonBossIndex = 0;


	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void SetDungeonBossActive(bool bActive);

	void Die();

protected :
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

protected :
	FDelegateHandle HealthChangedDelegateHandle;

	// Seohwan ( aicontroller에서 적 및 동료 판별 기준 ) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	uint8 TeamID = 1;

	// Targeting UI
public:
	void SetTargetWidgetVisible(bool bVisible);

	// Stat UI
protected:
	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAOWidgetComponentBase> TargetWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> TargetWidgetClass;

	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAOWidgetComponentBase> OverheadStatusWidgetComponent;

	// K.H
protected :
	void OnGroggyChanged(const FOnAttributeChangeData& Data);

	void StartGroggy();

public :
	UFUNCTION(BlueprintCallable, Category = "Groggy")
	virtual void EndGroggy();

	UFUNCTION(BlueprintPure, Category = "Groggy")
	bool IsGroggy() const { return bIsGroggy; }

protected :
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Groggy")
	bool bIsGroggy = false;

	FDelegateHandle GroggyChangedDelegateHandle;

};
