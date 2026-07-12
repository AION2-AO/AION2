#pragma once

#include "CoreMinimal.h"
#include "Game/AOGameMode.h"
#include "Player/AOPlayerState.h"
#include "Network/PacketHeader.h"
#include "AODungeonGameMode.generated.h"

class AAOMonsterBase;
class APlayerController;
class APlayerStart;
class Pawn;

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
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer) override;
public:
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void StartDungeon();

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void NotifyBossDefeated(AAOMonsterBase* DefeatedBoss);

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void ReturnToVillage();

	// Player Health 0 or Died Call
	void NotifyPlayerDied(APlayerController* DeadPlayerController, bool bIsFallDeath = false);
	void NotifyPlayerRespawnImmediately(APlayerController* DeadPlayerController);

	TMap<TObjectPtr<APlayerController>, int32> PendingRespawnBossIndices;

protected:
	void FindPlacedBosses();
	void InitializePlacedBosses();

	void StartBossPhase(int32 BossNumber);
	void ActivateBoss(int32 BossNumber);
	void StartNextBoss();

	void ClearDungeon();
	void FailDungeon();

	void SetCombatPhase(int32 BossNumber);
	void SetDefeatedPhase(int32 BossNumber);


	void StartPlayerRespawnTimer(APlayerController* DeadPlayerController, const FTransform& RespawnTransform);
	void RespawnPlayer(APlayerController* PlayerController);
	AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	int32 GetActiveDungeonPlayerCount() const;
	int32 GetAliveDungeonPlayerCount() const;

	APlayerStart* FindDungeonRespawnPoint() const;
	void ClearAllRespawnTimers();

	void StartWipeRespawn();
	void RespawnAllDeadPlayersAtBossCheckpoint();
	TArray<APlayerStart*> FindBossRespawnPoint(int32 CurrentBossNumber) const;

	// 캐릭터 직업
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	UPROPERTY(EditDefaultsOnly, Category = "Dungeon|Class")
	TMap<EDaevaClassType, TSubclassOf<APawn>> JobClassMap;

protected :
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,const FTransform& SpawnTransform) override;

private :
	APawn* SpawnPawnByPlayerClass(AController* NewPlayer, const FTransform& SapwnTransform);

protected:
	TMap<TObjectPtr<APlayerController>, FTransform> PendingRespawnTransforms;

	FTimerHandle WipeRespawnTimerHandle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	EDungeonPhase CurrentPhase = EDungeonPhase::Ready;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentBossNumber = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	TObjectPtr<AAOMonsterBase> CurrentBoss;

	UPROPERTY()
	TMap<int32, TObjectPtr<AAOMonsterBase>> PlacedBosses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon|Map")
	FString VillageMapPath = TEXT("/Game/Map/Village.Village");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Respawn")
	FName RespawnPlayerStartTag = TEXT("DungeonRespawn");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Respawn")
	float RespawnDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Respawn")
	FName Boss1RespawnTag = TEXT("Boss1Respawn");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Respawn")
	FName Boss2RespawnTag = TEXT("Boss2Respawn");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Respawn")
	FName Boss3RespawnTag = TEXT("Boss3Respawn");

	UPROPERTY(EditDefaultsOnly, Category = "Dungeon|Spawn")
	FName DungeonStartTag = TEXT("DungeonStart");

private :
	int32 NextDungeonStartIndex = 0;

	// CurrentDeadPlayer
	UPROPERTY()
	TSet<TObjectPtr<APlayerController>> DeadPlayerControllers;

	// RespawnTimer
	TMap<TObjectPtr<APlayerController>, FTimerHandle> RespawnTimerHandles;

	FTimerHandle NextBossTimerHandle;

protected:
	/*UFUNCTION(BlueprintImplementableEvent, Category = "Dungeon|Event")
	void OpenGateForNextBoss(int32 GateIndex);*/

	UFUNCTION(BlueprintImplementableEvent, Category = "Dungeon|Event")
	void GiveDungeonReward();
	
public:
	void SetDungeonId(int32 DungeonId) { MyDungeonId = DungeonId; }
	void SetPrePlayerInfo(const Protocol::S_DungeonStartDediPacket& PlayerInfo);

	int32 GetDungeonId() { return MyDungeonId; }
	
public :
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void RequestReturnToVillage();

	// 서버 전송	 
	// 미리 스폰된 플레이어들
	UPROPERTY()
	TArray<class APawn*> SpawnedPlayers;

	// Test
	UFUNCTION(Exec)
	void ForceClearDungeon();

	void SendDungeonCompleteRequest();

	void CreateDungeonClearWidget();

private:
	int32 MyDungeonId = 0;
	Protocol::DPlayerInfo* ValidateToken(FString Token);

	// 서버에서 받은 클라이언트 인증 토큰
	TMap<FString, Protocol::DPlayerInfo> PrePlayers;

	// 로그인 토큰 인증용
	TMap<int32, Protocol::DPlayerInfo> PendingPlayers;

private :
	bool bDungeonResultSent = false;
	int32 DungeonPrice = 1000;

};
