#include "Player/AOPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AAOPlayerController::AAOPlayerController()
{
	CurrentInputType = EInputType::Game;
}

void AAOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	SetInputMappingContext(CurrentInputType);

	bShowGASDebug = false;
}

void AAOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(GASDebugAction, ETriggerEvent::Started, this, &AAOPlayerController::ShowDebugGAS);
	}
}

void AAOPlayerController::SetInputMappingContext(EInputType InNewInputType)
{
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSystem)
	{
		InputSystem->ClearAllMappings();
		InputSystem->AddMappingContext(InputMappingContexts[InNewInputType], 0);
	}
}

void AAOPlayerController::ShowDebugGAS()
{
	bShowGASDebug = !bShowGASDebug;

	if (bShowGASDebug)
	{
		ConsoleCommand(TEXT("showdebug abilitysystem"));
	}
	else
	{
		ConsoleCommand(TEXT("showdebug none"));
	}
}
