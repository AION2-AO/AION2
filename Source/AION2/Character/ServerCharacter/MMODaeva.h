// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Daeva/Daeva.h"
#include "MMODaeva.generated.h"

/**
 *
 */
UCLASS()
class AION2_API AMMODaeva : public ADaeva
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	void SendMovePacket();
	void ReceiveMovePacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel);
	void ReceiveDashPacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel);
	void ToggleMailWidget();

	bool HasMovement();
	bool IsCurrentMoving();

	void SendDungeonWait();
	void SetDungeonId(int32 NewDungeonId) { DungeonId = NewDungeonId; }
	void SetHp(int32 Hp);

	uint64 GetMyId() { return MyId; }

	void PlayMontageWithSection(EMontageID MontageID, float PlayRate, FName SectionName);

	// PVP 상태 수신 함수
	void SetPvpState(bool bActive, float RemainingTime);

	void InputSpacePressed();
	void ReceiveJumpPacket(bool bIsGliding);

	// 클라이언트 피격 감지 함수
	virtual void CheckAttackHit(const FAttackData& AttackData);

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	bool bHasMoveInput = false;
private:
	void InputMoveReleased();

	FTimerHandle SendMoveHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Network")
	float SendMoveTimer = 0.1f;

	FVector LastLoc = FVector::ZeroVector;
	FRotator LastRot = FRotator::ZeroRotator;

	FVector TargetLoc = FVector::ZeroVector;
	FRotator TargetRot = FRotator::ZeroRotator;
	FVector TargetVel = FVector::ZeroVector;

	bool bWasMovingLastSend = false;

protected:
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChatActivateAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MailAction;

private:
	void OnChatActivateTriggered();
	void OnMailActivateTriggerd();

protected:
	bool CanDash() const;
	void PlayDash();

	UFUNCTION()
	void OnDashMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dash")
	float DashCooldown = 1.5f;

	float LastDashTime = -10.0f;

private:
	void MMOInputShiftPressed();

private:
	int32 DungeonId;
private:
	// PvP 상태 변수
	bool bIsPvpActive = false;
	float PvpRemainingTime = 0.0f;

	// LMB 연속 공격 확인
	bool bIsLMBHeld = false;
	int32 CurrentComboIndex = 1;
	float LastAttackTime = 0.0f;
	const float ComboWindow = 1.5f;

	// 스킬 쿨타임 관리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Cooldown", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, float> SkillCooldownDurations;

	// 기본공격 타격 시 마나 수급량 (에디터/블프에서 편집 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Mana", meta = (AllowPrivateAccess = "true"))
	float MMOHitManaRegenAmount = 10.0f;

	// 스킬 마나 소모량 관리 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Mana", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, float> SkillManaCosts;

	TMap<EMontageID, float> SkillLastUseTimes;

	// 내부 제어 함수
	void OnLMBPressed();
	void OnLMBReleased();
	void ExecuteBasicAttack();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnSkillActionTriggered(EMontageID SkillID);
	void SendSkillPacketToServer(EMontageID SkillID);

private:
	bool HasMontage(EMontageID MontageID) const;

	// 비전투 및 전투 상태 타이머
	FTimerHandle CombatStateTimerHandle;
	FTimerHandle ManaRegenTimerHandle;

	// 비전투 시 초당 마나 자동 회복량 (에디터/블프에서 편집 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Mana", meta = (AllowPrivateAccess = "true"))
	float ManaRegenRate = 10.0f;

	const float ManaRegenInterval = 0.5f;

	void EnterCombatState();
	void ExitCombatState();
	void RegenerateMana();
};