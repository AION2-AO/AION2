#pragma once

#include "CoreMinimal.h"
#include "Game/AOGameMode.h"
#include "AODungeonGameMode.generated.h"

class AAOMonsterBase;

UENUM(BlueprintType)
enum class EDungeonPhase : uint8
{
	Ready,

	Boss1Combat,
	Boss1Defeated,

	Boss2Combat,
	Boss2Defeated,

	Boss3Combat,
	Boss3Defeated,

	Cleared,
	Failed
};

UCLASS()
class AION2_API AAODungeonGameMode : public AAOGameMode
{
	GENERATED_BODY()
	
public :
	AAODungeonGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void StartDungeon();

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void NotifyBossDefeated(AAOMonsterBase* DefeatedBoss);

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void ReturnToVillage();

protected:
	void FindPlacedBosses();
	void InitializePlacedBosses();

	void StartBossPhase(int32 BossNumber);
	void ActivateBoss(int32 BossNumber);
	void StartNextBoss();

	void ClearDungeon();

	void SetCombatPhase(int32 BossNumber);
	void SetDefeatedPhase(int32 BossNumber);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	EDungeonPhase CurrentPhase = EDungeonPhase::Ready;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentBossNumber = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	TObjectPtr<AAOMonsterBase> CurrentBoss;

	// 1 = Boss1, 2 = Boss2, 3 = Boss3
	UPROPERTY()
	TMap<int32, TObjectPtr<AAOMonsterBase>> PlacedBosses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon|Map")
	FString VillageMapPath = TEXT("/Game/Map/Village.Village");

	FTimerHandle NextBossTimerHandle;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Dungeon|Event")
	void OpenGateForNextBoss(int32 GateIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dungeon|Event")
	void GiveDungeonReward();
	
public :
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void RequestReturnToVillage();
	
};
