// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ServerCharacter/MMODaeva.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Player/AOPlayerController.h"
#include "AOChattingWidget.h"
#include "ui/AOMainHUDWidget.h"
#include "AION2.h"

void AMMODaeva::BeginPlay()
{
	Super::BeginPlay();
	LastLoc = GetActorLocation();
	LastRot = GetActorRotation();
	TargetLoc = LastLoc;
	TargetRot = LastRot;
	bWasMovingLastSend = false;
}

void AMMODaeva::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, 10.f);
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.f);

		SetActorLocation(NewLocation);
		SetActorRotation(NewRotation);

		GetCharacterMovement()->Velocity = TargetVel;
	}
}

void AMMODaeva::PossessedBy(AController* NewController)
{
	ACharacter::PossessedBy(NewController);

	if (!IsLocallyControlled()) return;
{
	UE_LOG(LogTemp, Log, TEXT(" ADaeva::BeginPlay() - SetTimer"));
	GetWorldTimerManager().SetTimer(SendMoveHandle, this, &AMMODaeva::SendMovePacket, SendMoveTimer, true);
}
}

void AMMODaeva::SendMovePacket()
{
	bool bCurrentMovement = HasMovement();

	bool bShouldSend = false;

	if (bCurrentMovement)
	{
		bShouldSend = true;

		bWasMovingLastSend = true;
	}

	else
	{
		if (bWasMovingLastSend)
		{
			bShouldSend = true;
			bWasMovingLastSend = false;
		}
		else
		{
			return;
		}
	}

	if (bShouldSend)
	{
		FVector CurrLoc = GetActorLocation();
		FRotator CurrRot = GetActorRotation();

		Protocol::C_MovePacket MovePacket;
		MovePacket.set_playerid(MyId);

		Protocol::Vector3* Location = MovePacket.mutable_playerlocation();
		Location->set_x(CurrLoc.X);
		Location->set_y(CurrLoc.Y);
		Location->set_z(CurrLoc.Z);

		FVector CurrVelocity = GetCharacterMovement()->Velocity;

		Protocol::Vector3* Velocity = MovePacket.mutable_playervelocity();
		Velocity->set_x(CurrVelocity.X);
		Velocity->set_y(CurrVelocity.Y);
		Velocity->set_z(CurrVelocity.Z);

		Protocol::Rotator3* Rotation = MovePacket.mutable_playerrotation();
		Rotation->set_pitch(CurrRot.Pitch);
		Rotation->set_yaw(CurrRot.Yaw);
		Rotation->set_roll(CurrRot.Roll);

		SEND_PACKET(MovePacket, PKT_C_MOVE);

		LastLoc = CurrLoc;
		LastRot = CurrRot;
	}
}

bool AMMODaeva::HasMovement()
{
	FVector CurrentLoc = GetActorLocation();
	FRotator CurrentRot = GetActorRotation();

	float Distance = FVector::DistSquared(CurrentLoc, LastLoc);

	float YawDiff = FRotator::NormalizeAxis(CurrentRot.Yaw - LastRot.Yaw);
	bool bRotated = FMath::Abs(YawDiff) >= 10.0f;

	bool bIsMoving = GetCharacterMovement()->Velocity.SizeSquared() > 0.f;

	return (Distance > 25.f) || bRotated || bIsMoving;
}

void AMMODaeva::InputMoveReleased()
{
	bHasMoveInput = false;
	RequestStopSprint();
}

void AMMODaeva::OnChatActivateTriggered()
{
	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetController());
	if(PlayerController)
	{
		auto HUD = PlayerController->GetMainHUD();
		if (HUD->ChattingWidget)
		{
			HUD->ChattingWidget->ActivateChatInput();
		}
	}
}

bool AMMODaeva::IsCurrentMoving()
{
	if (!GetCharacterMovement()) return false;
	bool bHasVelocity = GetCharacterMovement()->Velocity.SizeSquared() > 100.f;

	bool bHasInput = !GetPendingMovementInputVector().IsNearlyZero();

	return bHasVelocity || bHasInput;
}

void AMMODaeva::SendDungeonWait()
{
	UE_LOG(LogTemp, Log, TEXT("DunzeonWaitingRoom Enter"));

	Protocol::C_DungeonWaitingRoomEnterPacket EnterWaitPacket;
	SEND_PACKET(EnterWaitPacket, PKT_C_DUNGEONWAITINTROOM);
}

void AMMODaeva::SetHp(int32 Hp)
{

}

void AMMODaeva::ReceiveMovePacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel)
{
	TargetLoc = NewLoc;
	TargetRot = NewRot;
	TargetVel = NewVel;
}


void AMMODaeva::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	ACharacter::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMMODaeva::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMMODaeva::Look);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AMMODaeva::Zoom);
		EnhancedInputComponent->BindAction(ChatActivateAction, ETriggerEvent::Triggered, this, &AMMODaeva::OnChatActivateTriggered);

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&AMMODaeva::InputMoveReleased
		);	
		
		EnhancedInputComponent->BindAction(KeyXAction, ETriggerEvent::Triggered, this, &AMMODaeva::SendItem, 0);
		EnhancedInputComponent->BindAction(KeyBAction, ETriggerEvent::Triggered, this, &AMMODaeva::SendItem, 1);

	}
}

