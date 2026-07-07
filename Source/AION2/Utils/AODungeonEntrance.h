// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "AODungeonEntrance.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class AION2_API AAODungeonEntrance : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAODungeonEntrance();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// declare overlap end function
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void EnterDungeon();
	void EnterDungeonWaitingRoom();

	UFUNCTION()
	void OnDungeonRoomWidgetClosed();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<class UUserWidget> DungeonWaitingRoomWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftClassPtr<class UUserWidget> DungeonWaitingRoomClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<class UUserWidget> DungeonRoomWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftClassPtr<class UUserWidget> DungeonRoomClass;
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> EffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> DungeonEffect;

private:
	UPROPERTY()
	class UBoxComponent* TriggerBox;

	UPROPERTY()
	AActor* OverlappedPlayer = nullptr;

	UPROPERTY()
	class AAOPlayerController* PC = nullptr;

	UPROPERTY()
	class UAOUIManager* UIManager = nullptr;
};
