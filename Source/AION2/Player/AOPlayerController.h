#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AOPlayerController.generated.h"

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

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void SetInputMappingContext(EInputType InNewInputType);

private:
	void ShowDebugGAS();
	bool bShowGASDebug = false;

private:
	UPROPERTY(VisibleAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	EInputType CurrentInputType;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TMap<EInputType, TObjectPtr<class UInputMappingContext>> InputMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> GASDebugAction;
};
