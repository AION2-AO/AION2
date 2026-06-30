#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AOPlayerController.generated.h"

extern TAutoConsoleVariable<int32> CVarDrawAttackTrace;

class UAOMainHUDWidget;

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
	// UI
	void CreateOrBindMainHUD();

public:
	UAOMainHUDWidget* GetMainHUD() const { return MainHUD; }

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
	// UI ฐüทร
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RaidHUD)
	TSubclassOf<UAOMainHUDWidget> MainHUDClass;

	TObjectPtr<UAOMainHUDWidget> MainHUD;
};
