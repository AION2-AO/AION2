#include "Character/Daeva/Daeva.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"

ADaeva::ADaeva()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetVisibility(false);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	CreatePart(EDaevaPartType::Helmet, TEXT("HelmetPart"));
	CreatePart(EDaevaPartType::Hair, TEXT("HairPart"));
	CreatePart(EDaevaPartType::Head, TEXT("HeadPart"));
	CreatePart(EDaevaPartType::Shoulder, TEXT("ShoulderPart"));
	CreatePart(EDaevaPartType::Cape, TEXT("CapePart"));
	CreatePart(EDaevaPartType::Body, TEXT("BodyPart"));
	CreatePart(EDaevaPartType::Glove, TEXT("GlovePart"));
	CreatePart(EDaevaPartType::Pants, TEXT("PantsPart"));
	CreatePart(EDaevaPartType::Boots, TEXT("BootsPart"));
}

void ADaeva::CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName)
{
	auto* PartMesh = CreateDefaultSubobject<USkeletalMeshComponent>(ComponentName);

	PartMesh->SetupAttachment(GetMesh());
	PartMesh->SetLeaderPoseComponent(GetMesh());

	Parts.Add(PartType, PartMesh);
}

void ADaeva::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADaeva::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADaeva::Look);
	}
}

void ADaeva::Move(const FInputActionValue& Value)
{
	FVector2D Movement = Value.Get<FVector2D>();

	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	FVector NewMoveInputDirection = ForwardVector * Movement.Y + RightVector * Movement.X;
	if (NewMoveInputDirection.IsNearlyZero())
	{
		return;
	}

	bHasCurrentMoveInput = true;
	CurrentMoveInputDirection = NewMoveInputDirection.GetSafeNormal();
	AddMovementInput(CurrentMoveInputDirection);
}

void ADaeva::Look(const FInputActionValue& Value)
{
	FVector2D RotationValue = Value.Get<FVector2D>();

	AddControllerYawInput(RotationValue.X);
	AddControllerPitchInput(-RotationValue.Y);
}
