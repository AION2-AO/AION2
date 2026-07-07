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

	bool HasMovement();
	bool IsCurrentMoving();

	void SendDungeonWait();
	void SetDungeonId(int32 NewDungeonId) { DungeonId = NewDungeonId; }

	void SetHp(int32 Hp);

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

private:
	void OnChatActivateTriggered();

private:
	int32 DungeonId;
};