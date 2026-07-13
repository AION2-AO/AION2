#include "Player/AOPlayerController.h"
#include "Player/AOPlayerState.h"

#include "Character/Monster/AOMonsterBase.h"
#include "Character/Daeva/Daeva.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputCoreTypes.h"
#include "Game/AODungeonGameMode.h"
#include "Game/AOGameInstance.h"
#include "AbilitySystemComponent.h"
#include "Game/AOGameInstance.h"
#include "Blueprint/UserWidget.h"

#include "UI/AOMainHUDWidget.h"

#include "Manager/AOUIManager.h"
#include "UI/Mail/MainMailWidget.h"
#include "UI/DungeonClearWidget.h"
#include "Components/Widget.h"


TAutoConsoleVariable<int32> CVarDrawAttackTrace(TEXT("ao.Debug.DrawAttackTrace"), 0, TEXT("Draw attack trace debug"), ECVF_Cheat);

AAOPlayerController::AAOPlayerController()
{
	CurrentInputType = EInputType::Game;
}

void AAOPlayerController::Server_SetShowColliderDebug_Implementation()
{
	bShowColliderDebug = !bShowColliderDebug;

	if (bShowColliderDebug)
	{
		ConsoleCommand(TEXT("ao.Debug.DrawAttackTrace 1"));
	}
	else
	{
		ConsoleCommand(TEXT("ao.Debug.DrawAttackTrace 0"));
	}
}

void AAOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 클라이언트일 때만 지정
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}

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
		EnhancedInputComponent->BindAction(ColliderDebugAction, ETriggerEvent::Started, this, &AAOPlayerController::ShowDebugCollider);
		EnhancedInputComponent->BindAction(GASDebugAction, ETriggerEvent::Started, this, &AAOPlayerController::ShowDebugGAS);
	}
}

// AOPlayerController.cpp
void AAOPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (IsLocalController())
	{
		HandlePawnASCReady();
	}
}

void AAOPlayerController::SetInputMappingContext(EInputType InNewInputType)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	if (!InputSystem)
	{
		return;
	}

	if (!InputMappingContexts.Contains(InNewInputType))
	{
		return;
	}

	InputSystem->ClearAllMappings();
	InputSystem->AddMappingContext(InputMappingContexts[InNewInputType], 0);
}


void AAOPlayerController::ShowDebugCollider()
{
	Server_SetShowColliderDebug();
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

void AAOPlayerController::HandlePawnASCReady()
{
	/* Suyeon: More strict validation Check => else: retry next Tick(26.07.07) */

	// Exception Handling 
	// => Validation Check: Is LocalPlayer && DedicatedServer => Can Show UI
	// Existed Validation Check
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}

	// === Validation Check below are the new ones ===

	// => Validation Check: Is the local pawn is daeva?
	ADaeva* Daeva = Cast<ADaeva>(GetPawn());
	if (!Daeva)
	{
		return;
	}

	// => Validation Check: The Daeva has ASC?
	AAOPlayerState* AOPlayerState = Daeva->GetPlayerState<AAOPlayerState>();
	UAbilitySystemComponent* ASC = Daeva->GetAbilitySystemComponent();


	// => Validation Check: The ASC of Dave is Ready?
	if (!AOPlayerState || !ASC)
	{
		// Then retry for 60 Tick.
		if (++PawnASCReadyRetryCount <= PawnASCMaxRetryCount)
		{
			GetWorldTimerManager().SetTimerForNextTick(
				this,
				&AAOPlayerController::HandlePawnASCReady
			);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HandlePawnASCReady retry exceeded. PlayerState=%s ASC=%s"),
				*GetNameSafe(AOPlayerState),
				*GetNameSafe(ASC));
		}

		// Leaving this block because of the upper retry block.
		return;
	}

	if (BoundHUDDaeva.Get() == Daeva && BoundHUDASC.Get() == ASC)
	{
		return;
	}


	/*
	* 동일한 처리가 이미 성공했다면
	* : 이미 Bound된 Daeva가 있고, 해당 Daeva의 ASC가 지금 Bind된 ASC와 같다면) return 해주는 용도.
	* Respawn이나 Pawn 교체가 있으면 새 Pawn에 다시 Binding 불가능할 수도 있음
	* 아래 부분은 원래 MainHUD에 bool 처리를 맡겨야 하는데,
	* Error 없이 보이는 구현이 우선이므로 여기서 진행
	*/
	BoundHUDDaeva = Daeva;
	BoundHUDASC = ASC;

	// 다음에 Pawn이 재생성되면 다시 시도될 수 있으므로 Initialize.
	PawnASCReadyRetryCount = 0;

	// Finally called.
	CreateOrBindMainHUD(AOPlayerState);
	Daeva->BindOverheadStatusWidget();
}

void AAOPlayerController::CreateOrBindMainHUD(AAOPlayerState* AOPlayerState)
{
	// Exception Handling
	if (GetNetMode() == NM_DedicatedServer || !IsLocalController())
	{
		return;
	}

	if (!MainHUD)
	{
		MainHUD = CreateWidget<UAOMainHUDWidget>(this, MainHUDClass);
		if (!MainHUD)
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerContoroller Is Not Vaild -  AAOPlayerController::CreateOrBindMainHUD"));
			return;
		}

		MainHUD->AddToViewport();
	}

	MainHUD->BindToPlayerState(AOPlayerState);
}

void AAOPlayerController::ShowTargetMonsterHUD(AAOMonsterBase* InMonster)
{
	if (!IsLocalController() || !MainHUD || !InMonster)
	{
		return;
	}

	MainHUD->ShowTargetMonsterHUD(InMonster);
}

void AAOPlayerController::HideTargetMonsterHUD()
{
	if (!IsLocalController() || !MainHUD)
	{
		return;
	}

	MainHUD->HideTargetMonsterHUD();
}

void AAOPlayerController::PlaySkillPressedFeedback(int32 InputId)
{
	if (!IsLocalController() || !MainHUD)
	{
		return;
	}

	MainHUD->PlaySkillPressedFeedback(InputId);
}

// 07.09
void AAOPlayerController::Client_RefreshPlayerHUD_Implementation()
{
	HandlePawnASCReady();
}
//

void AAOPlayerController::ToggleMailWidget()
{
	if (!IsLocalController()) return;

	UAOUIManager* UIManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAOUIManager>() : nullptr;
	if (!UIManager) return;

	UMainMailWidget* MainMailWidget = UIManager->GetWidget<UMainMailWidget>();
	if (MainMailWidget && MainMailWidget->IsInViewport())
	{
		UIManager->HideWidget(MainMailWidget);
		
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
	else
	{
		if (!MainMailWidgetClass.IsNull())
		{
			UUserWidget* Widget = UIManager->ShowWidget(MainMailWidgetClass, EUILayer::Default);
			if (Widget)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(Widget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);
				bShowMouseCursor = true;
			}
		}
	}
}

void AAOPlayerController::TestClearDungeon()
{
	//ServerTestClearDungeon();
}

void AAOPlayerController::ClientCreateDungeonClearWidget_Implementation(int32 Gold)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[DungeonClearUI] Client RPC received | PC=%s Local=%d NetMode=%d Gold=%d"),
		*GetNameSafe(this),
		IsLocalController(),
		static_cast<int32>(GetNetMode()),
		Gold);

	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Error,
			TEXT("[DungeonClearUI] This controller is not local"));
		return;
	}

	if (!DungeonClearWidgetClass)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[DungeonClearUI] DungeonClearWidgetClass is NULL"));
		return;
	}

	if (!DungeonClearWidget)
	{
		DungeonClearWidget =
			CreateWidget<UDungeonClearWidget>(
				this,
				DungeonClearWidgetClass
			);

		if (!DungeonClearWidget)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[DungeonClearUI] CreateWidget failed"));
			return;
		}

		DungeonClearWidget->AddToViewport(100);

		UE_LOG(LogTemp, Warning,
			TEXT("[DungeonClearUI] Widget created and added: %s"),
			*GetNameSafe(DungeonClearWidget));
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[DungeonClearUI] Existing widget found"));

		if (!DungeonClearWidget->IsInViewport())
		{
			DungeonClearWidget->AddToViewport(100);
		}

		DungeonClearWidget->SetVisibility(ESlateVisibility::Visible);
	}

	DungeonClearWidget->SetDungeonClearWidget(Gold);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(DungeonClearWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(
		EMouseLockMode::DoNotLock
	);

	SetInputMode(InputMode);
	bShowMouseCursor = true;

	UE_LOG(LogTemp, Warning,
		TEXT("[DungeonClearUI] Finished displaying widget"));
}

void AAOPlayerController::ServerRequestDungeonComplete_Implementation()
{
	AAODungeonGameMode* DungeonGameMode = GetWorld()->GetAuthGameMode<AAODungeonGameMode>();

	if (!DungeonGameMode)
	{
		return;
	}

	DungeonGameMode->SendDungeonCompleteRequest();
}
