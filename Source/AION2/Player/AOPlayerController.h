#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AOPlayerController.generated.h"

extern TAutoConsoleVariable<int32> CVarDrawAttackTrace;

class AAOMonsterBase;
class AAOPlayerState;
class ADaeva;

class UUserWidget;
class UDungeonClearWidget;

class UAOMainHUDWidget;
class UAbilitySystemComponent;

UENUM()
enum class EInputType : uint8
{
	Game,
	UI
};

UCLASS()
class AION2_API AAOPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAOPlayerController();

public:
	UFUNCTION(Server, Reliable)
	void Server_SetShowColliderDebug();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void AcknowledgePossession(APawn* P) override;

private:
	void SetInputMappingContext(EInputType InNewInputType);


private:
	void ShowDebugCollider();
	void ShowDebugGAS();


public:
	// UI
	void HandlePawnASCReady();

private:
	// UI: Parameter is the verified playerstate in this PlayerController.s 
	void CreateOrBindMainHUD(AAOPlayerState* AOPlayerState);

public:
	// ! 신혜님 UI 코드에서 많이 사용 중이라 삭제하면 안됨 !: 추후 분리하든가 할 것.
	UAOMainHUDWidget* GetMainHUD() const { return MainHUD; }


public:
	// === Monster HUD. ===
	// Show Full-Screen Monster Stat visibility value & Bind ASC.
	void ShowTargetMonsterHUD(AAOMonsterBase* InMonster);

	// Hide Full-Screen Monster Stat visibility value & Unbind ASC.
	void HideTargetMonsterHUD();

public:
	// === MainHUD-> PlayerHUD -> SkillHUD/slot of inputID -> SkillSlot ===
	void PlaySkillPressedFeedback(int32 InputId);


private:
	bool bShowColliderDebug = false;
	bool bShowGASDebug = false;

private:
	UPROPERTY(VisibleAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	EInputType CurrentInputType;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TMap<EInputType, TObjectPtr<class UInputMappingContext>> InputMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ColliderDebugAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> GASDebugAction;

protected:
	// UI 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonHUD)
	TSubclassOf<UAOMainHUDWidget> MainHUDClass;

	TObjectPtr<UAOMainHUDWidget> MainHUD;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftClassPtr<class UMainMailWidget> MainMailWidgetClass;

	//07.09
	UFUNCTION(Client, Reliable)
	void Client_RefreshPlayerHUD();
	//

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleMailWidget();

	/*
	* 동일한 처리가 이미 성공했다면
	* (이미 Bound된 Daeva가 있고, 해당 Daeva의 ASC가 지금 Bind된 ASC와 같다면) return 해주는 용도.
	* Respawn이나 Pawn 교체가 있으면 새 Pawn에 다시 Binding 불가능할 수도 있음
	* 아래 부분은 원래 MainHUD에 맡겨야 하는데, 
	* Error 없이 보이는 구현이 우선이므로 여기서 진행
	*/
	UPROPERTY()
	TWeakObjectPtr<ADaeva> BoundHUDDaeva;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> BoundHUDASC;

private:
	// MainHUD의 Pawn Ready Tick 재시도 횟수 Count.
	int32 PawnASCReadyRetryCount = 0;

	// 일단 넉넉하게 180 => 3초로 잡기. 잘 되면 점점 줄여서 60을 목표로.
	int32 PawnASCMaxRetryCount = 180;

	//TEST.HY
public :
	UFUNCTION(Exec)
	void TestClearDungeon();

public:
	UFUNCTION(Client, Reliable)
	void ClientCreateDungeonClearWidget(int32 Gold);

	UFUNCTION(Server, Reliable)
	void ServerRequestDungeonComplete();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UDungeonClearWidget> DungeonClearWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UDungeonClearWidget> DungeonClearWidget;
};
