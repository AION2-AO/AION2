// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ServerCharacter/MMODaeva.h"
#include "UObject/UnrealType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Player/AOPlayerController.h"
#include "UI/AOChattingWidget.h"
#include "UI/AOMainHUDWidget.h"
#include "Manager/AOUIManager.h"
#include "UI/Mail/MainMailWidget.h"
#include "AION2.h"
#include "GAS/AOGameplayTags.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Physics/Collision.h"


void AMMODaeva::BeginPlay()
{
	Super::BeginPlay();
	LastLoc = GetActorLocation();
	LastRot = GetActorRotation();
	TargetLoc = LastLoc;
	TargetRot = LastRot;
	bWasMovingLastSend = false;

	// 블프에서 쿨타임 설정을 안 한 경우에만 디폴트값 채워넣음
	if (SkillCooldownDurations.Num() == 0)
	{
		SkillCooldownDurations.Add(EMontageID::Key1, 5.0f);  // 1번 스킬: 5초
		SkillCooldownDurations.Add(EMontageID::Key2, 8.0f);  // 2번 스킬: 8초
		SkillCooldownDurations.Add(EMontageID::Key3, 10.0f); // 3번 스킬: 10초
		SkillCooldownDurations.Add(EMontageID::Key4, 15.0f); // 4번 스킬: 15초
		SkillCooldownDurations.Add(EMontageID::KeyQ, 12.0f); // Q 스킬: 12초
		SkillCooldownDurations.Add(EMontageID::KeyE, 6.0f);  // E 스킬: 6초
		SkillCooldownDurations.Add(EMontageID::RB, 3.0f);    // 우클릭 스킬: 3초
	}

	// 블프에서 마나 소모량 설정을 안 한 경우에만 디폴트값 채워넣음
	if (SkillManaCosts.Num() == 0)
	{
		SkillManaCosts.Add(EMontageID::Key1, 15.0f);
		SkillManaCosts.Add(EMontageID::Key2, 20.0f);
		SkillManaCosts.Add(EMontageID::Key3, 25.0f);
		SkillManaCosts.Add(EMontageID::Key4, 30.0f);
		SkillManaCosts.Add(EMontageID::KeyQ, 10.0f);
		SkillManaCosts.Add(EMontageID::KeyE, 15.0f);
		SkillManaCosts.Add(EMontageID::RB, 20.0f);
	}

	// 0.5초 주기로 마나 자동 회복 타이머 가동
	GetWorldTimerManager().SetTimer(ManaRegenTimerHandle, this, &AMMODaeva::RegenerateMana, ManaRegenInterval, true);
}

void AMMODaeva::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		bool bIsDashing = ASC && ASC->HasMatchingGameplayTag(STATE_DASHING);
		if (!bIsDashing)
		{
			FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, 10.f);
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.f);

			SetActorLocation(NewLocation);
			SetActorRotation(NewRotation);

			GetCharacterMovement()->Velocity = TargetVel;
		}
	}
}

void AMMODaeva::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!IsLocallyControlled()) return;
	{
		UE_LOG(LogTemp, Log, TEXT(" AMMODaeva::PossessedBy() - SetTimer"));
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
	if (PlayerController)
	{
		auto HUD = PlayerController->GetMainHUD();
		if (HUD->ChattingWidget)
		{
			HUD->ChattingWidget->ActivateChatInput();
		}
	}
}

void AMMODaeva::OnMailActivateTriggerd()
{
	ToggleMailWidget();
}

void AMMODaeva::ToggleMailWidget()
{
	UAOUIManager* UIManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAOUIManager>() : nullptr;
	if (UIManager)
	{
		UMainMailWidget* MainMailWidget = UIManager->GetWidget<UMainMailWidget>();
		if (MainMailWidget && MainMailWidget->IsInViewport())
		{
			AAOPlayerController* PC = Cast<AAOPlayerController>(GetController());
			if (PC)
			{
				PC->ToggleMailWidget();
			}
			return;
		}
	}

	Protocol::C_MailListPacket ReqList;
	ReqList.set_playerid(MyId);
	SEND_PACKET(ReqList, PKT_C_MAIL_LIST);
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
	SEND_PACKET(EnterWaitPacket, PKT_C_DUNGEON_ENTER_WAITING_ROOM);
}

void AMMODaeva::SetHp(int32 Hp)
{

}

void AMMODaeva::ReceiveMovePacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel)
{
	TargetLoc = NewLoc;
	TargetRot = NewRot;
	TargetVel = NewVel;

	// 공중(낙하) 상태 중 지상 도달 시 (Z 속도가 0이 되면) 걷기 상태로 전환해 착지 애니메이션 유도
	if (GetCharacterMovement()->MovementMode == MOVE_Falling && FMath::IsNearlyZero(NewVel.Z, 0.1f))
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
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
		EnhancedInputComponent->BindAction(MailAction, ETriggerEvent::Started, this, &AMMODaeva::OnMailActivateTriggerd);

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&AMMODaeva::InputMoveReleased
		);

		EnhancedInputComponent->BindAction(KeyXAction, ETriggerEvent::Triggered, this, &AMMODaeva::SendItem, 0);
		EnhancedInputComponent->BindAction(KeyBAction, ETriggerEvent::Triggered, this, &AMMODaeva::SendItem, 1);

		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AMMODaeva::MMOInputShiftPressed);
		EnhancedInputComponent->BindAction(SpaceAction, ETriggerEvent::Started, this, &AMMODaeva::InputSpacePressed);

		// LMB 연속 공격(연계/홀딩) 바인딩
		EnhancedInputComponent->BindAction(LBAction, ETriggerEvent::Started, this, &AMMODaeva::OnLMBPressed);
		EnhancedInputComponent->BindAction(LBAction, ETriggerEvent::Completed, this, &AMMODaeva::OnLMBReleased);

		// 스킬 단축키 바인딩 (마나 소모량은 내부 Map에서 가져오도록 변경)
		EnhancedInputComponent->BindAction(Key1Action, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::Key1);
		EnhancedInputComponent->BindAction(Key2Action, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::Key2);
		EnhancedInputComponent->BindAction(Key3Action, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::Key3);
		EnhancedInputComponent->BindAction(Key4Action, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::Key4);
		EnhancedInputComponent->BindAction(KeyQAction, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::KeyQ);
		EnhancedInputComponent->BindAction(KeyEAction, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::KeyE);
		EnhancedInputComponent->BindAction(RBAction, ETriggerEvent::Started, this, &AMMODaeva::OnSkillActionTriggered, EMontageID::RB);
	}
}


void AMMODaeva::PlayMontageWithSection(EMontageID MontageID, float PlayRate, FName SectionName)
{
	// 몽타주가 맵에 등록되어 있지 않은 경우 크래시를 방지하기 위해 얼리 리턴
	if (!HasMontage(MontageID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Montages Map에 EMontageID::%d 에 해당하는 몽타주가 등록되어 있지 않습니다."), static_cast<int32>(MontageID));
		return;
	}

	if (!GetMesh() || !GetMontageByID(MontageID))
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (IsLocallyControlled())
		{
			AnimInstance->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
		}
		else
		{
			AnimInstance->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
		}

		AnimInstance->Montage_Play(GetMontageByID(MontageID), PlayRate);
		if (SectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(SectionName, GetMontageByID(MontageID));
		}
	}
}

bool AMMODaeva::CanDash() const
{
	if (IsDead())
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	// Check if falling, flying, or gliding (Custom movement mode)
	if (MovementComp->IsFalling() ||
		MovementComp->IsFlying() ||
		(MovementComp->MovementMode == MOVE_Custom &&
			MovementComp->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide)))
	{
		return false;
	}

	// Check if already dashing
	if (ASC && ASC->HasMatchingGameplayTag(STATE_DASHING))
	{
		return false;
	}

	// Cooldown check
	if (GetWorld()->GetTimeSeconds() - LastDashTime < DashCooldown)
	{
		return false;
	}

	// Stamina check
	if (ASC)
	{
		const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();
		if (AttributeSet && AttributeSet->GetStamina() < 20.0f)
		{
			return false;
		}
	}

	return true;
}

void AMMODaeva::PlayDash()
{
	EMontageID SelectedMontageID = EMontageID::Dash;
	float MontagePlayRate = 1.0f;

	UAnimMontage* DashMontage = GetMontageByID(SelectedMontageID);
	if (!DashMontage)
	{
		return;
	}

	bool bForward = HasMoveInput();
	FName SectionName = bForward ? FName("Forward") : FName("Back");

	PlayMontageWithSection(SelectedMontageID, MontagePlayRate, SectionName);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AMMODaeva::OnDashMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DashMontage);
	}

	if (ASC)
	{
		ASC->AddLooseGameplayTag(STATE_DASHING);

		const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();
		if (AttributeSet)
		{
			float CurrentStamina = AttributeSet->GetStamina();
			ASC->SetNumericAttributeBase(UAOAttributeSet::GetStaminaAttribute(), FMath::Max(0.0f, CurrentStamina - 20.0f));
		}
	}

	LastDashTime = GetWorld()->GetTimeSeconds();
}

void AMMODaeva::OnDashMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDashMontageEnded: Montage %s ended, bInterrupted = %s"),
		Montage ? *Montage->GetName() : TEXT("None"),
		bInterrupted ? TEXT("True") : TEXT("False"));

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(STATE_DASHING);
	}
}

void AMMODaeva::MMOInputShiftPressed()
{
	if (IsDead())
	{
		return;
	}

	// 대시 실행
	if (CanDash())
	{
		PlayDash();

		FVector CurrLoc = GetActorLocation();
		FRotator CurrRot = GetActorRotation();

		Protocol::C_DashPacket DashPacket;
		DashPacket.set_playerid(MyId);
		Protocol::Vector3* Location = DashPacket.mutable_playerlocation();
		Location->set_x(CurrLoc.X);
		Location->set_y(CurrLoc.Y);
		Location->set_z(CurrLoc.Z);

		FVector CurrVelocity = GetCharacterMovement()->Velocity;

		Protocol::Vector3* Velocity = DashPacket.mutable_playervelocity();
		Velocity->set_x(CurrVelocity.X);
		Velocity->set_y(CurrVelocity.Y);
		Velocity->set_z(CurrVelocity.Z);

		Protocol::Rotator3* Rotation = DashPacket.mutable_playerrotation();
		Rotation->set_pitch(CurrRot.Pitch);
		Rotation->set_yaw(CurrRot.Yaw);
		Rotation->set_roll(CurrRot.Roll);


		SEND_PACKET(DashPacket, PKT_C_DASH);


		LastLoc = CurrLoc;
		LastRot = CurrRot;

	}
}

void AMMODaeva::ReceiveDashPacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel)
{
	TargetLoc = NewLoc;
	TargetRot = NewRot;
	TargetVel = NewVel;

	EMontageID SelectedMontageID = EMontageID::Dash;
	float MontagePlayRate = 1.0f;

	UAnimMontage* DashMontage = GetMontageByID(SelectedMontageID);
	if (!DashMontage)
	{
		return;
	}

	FName SectionName = TargetVel.SizeSquared() > 10000.f ? FName("Forward") : FName("Back");

	PlayMontageWithSection(SelectedMontageID, MontagePlayRate, SectionName);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AMMODaeva::OnDashMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DashMontage);
	}

	if (ASC)
	{
		ASC->AddLooseGameplayTag(STATE_DASHING);
	}
}

// =========================================================================

void AMMODaeva::OnLMBPressed()
{
	if (IsDead()) return;
	if (!bIsPvpActive) return;
	bIsLMBHeld = true;
	ExecuteBasicAttack();
}

void AMMODaeva::OnLMBReleased()
{
	bIsLMBHeld = false;
}

void AMMODaeva::ExecuteBasicAttack()
{
	// PVP 비활성화 상태인 경우 공격 자체를 불허
	if (CurrentTarget && CurrentTarget->IsA(AMMODaeva::StaticClass()) && !bIsPvpActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("PVP 비활성화 기간에는 다른 플레이어를 공격할 수 없습니다."));
		return;
	}

	// 전투 상태 활성화 및 무기 장착 연출 타이머 작동
	EnterCombatState();

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	// 공중(낙하) 상태인지 확인하여 공중 공격(Jumping Attack) 처리
	bool bIsFalling = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
	if (bIsFalling)
	{
		if (HasMontage(EMontageID::Jump))
		{
			PlayMontageWithSection(EMontageID::Jump, 1.0f, NAME_None);

			uint64 TargetId = 0;
			if (CurrentTarget)
			{
				if (AMMODaeva* TargetDaeva = Cast<AMMODaeva>(CurrentTarget))
				{
					TargetId = TargetDaeva->GetMyId();
				}
			}

			Protocol::C_AttackPacket AttackPacket;
			AttackPacket.set_playerid(MyId);
			AttackPacket.set_targetid(TargetId);
			AttackPacket.set_damage(10.f);
			AttackPacket.set_isbasicattack(true);
			AttackPacket.set_skillid(static_cast<int32>(EMontageID::Jump));

			SEND_PACKET(AttackPacket, PKT_C_ATTACK);
			return;
		}
	}

	UAnimMontage* LBMontage = GetMontageByID(EMontageID::LB);
	if (!LBMontage) return;

	if (AnimInstance->Montage_IsPlaying(LBMontage)) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime > ComboWindow)
	{
		CurrentComboIndex = 1;
	}

	FName SectionName = *FString::Printf(TEXT("LB_%d"), CurrentComboIndex);
	PlayMontageWithSection(EMontageID::LB, 1.0f, SectionName);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AMMODaeva::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, LBMontage);

	LastAttackTime = CurrentTime;
	CurrentComboIndex = (CurrentComboIndex % 3) + 1;

	// 허공 공격 모션 동기화를 위해 기본 공격 재생 시점에 패킷을 한 번 날려줍니다. (데미지 0, 타겟 0)
	Protocol::C_AttackPacket AttackPacket;
	AttackPacket.set_playerid(MyId);
	AttackPacket.set_targetid(0);
	AttackPacket.set_damage(0);
	AttackPacket.set_isbasicattack(true);
	AttackPacket.set_skillid(static_cast<int32>(EMontageID::LB));

	SEND_PACKET(AttackPacket, PKT_C_ATTACK);
}

void AMMODaeva::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bIsLMBHeld && !bInterrupted)
	{
		ExecuteBasicAttack();
	}
}

void AMMODaeva::OnSkillActionTriggered(EMontageID SkillID)
{
	if (IsDead()) return;
	if (!bIsPvpActive) return;

	// 타겟이 플레이어인데 PVP 비활성화 상태인 경우 스킬 사용 자체를 불허
	if (CurrentTarget && CurrentTarget->IsA(AMMODaeva::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("PVP 비활성화 기간에는 다른 플레이어에게 스킬을 사용할 수 없습니다."));
		return;
	}

	// 전투 상태 활성화 및 무기 장착 연출 타이머 작동
	EnterCombatState();

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float Cooldown = SkillCooldownDurations.FindRef(SkillID);
	float LastUseTime = SkillLastUseTimes.FindRef(SkillID);

	if (CurrentTime - LastUseTime < Cooldown)
	{
		float Remaining = Cooldown - (CurrentTime - LastUseTime);
		UE_LOG(LogTemp, Warning, TEXT("스킬 재사용 대기 중! 남은 시간: %.1f초"), Remaining);
		return;
	}

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	if (!MyASC) return;

	const UAOAttributeSet* AttributeSet = MyASC->GetSet<UAOAttributeSet>();
	if (!AttributeSet) return;

	// 마나 소모량을 TMap(SkillManaCosts)에서 탐색하여 조회
	float ManaCost = SkillManaCosts.FindRef(SkillID);

	float CurrentMana = AttributeSet->GetMana();
	if (CurrentMana < ManaCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("마나가 부족합니다! (현재: %.1f / 필요: %.1f)"), CurrentMana, ManaCost);
		return;
	}

	float NewMana = CurrentMana - ManaCost;
	MyASC->SetNumericAttributeBase(UAOAttributeSet::GetManaAttribute(), NewMana);

	SkillLastUseTimes.Add(SkillID, CurrentTime);

	// 만약 개별 스킬 애니메이션이 등록되어 있지 않은 경우, 크래시 방지 및 최소한의 공격 연출을 위해 
	// 기본 공격 몽타주(LB)를 폴백(Fallback)으로 대신 재생합니다.
	EMontageID VisualMontageID = HasMontage(SkillID) ? SkillID : EMontageID::LB;
	PlayMontageWithSection(VisualMontageID, 1.0f, NAME_None);

	SendSkillPacketToServer(SkillID);
}

void AMMODaeva::SendSkillPacketToServer(EMontageID SkillID)
{
	Protocol::C_AttackPacket AttackPacket;
	AttackPacket.set_playerid(MyId);

	uint64 TargetId = 0;
	if (CurrentTarget)
	{
		if (AMMODaeva* TargetDaeva = Cast<AMMODaeva>(CurrentTarget))
		{
			TargetId = TargetDaeva->GetMyId();
		}
	}
	AttackPacket.set_targetid(TargetId);
	AttackPacket.set_damage(0); // 쿨타임/마나 소모 검증용
	AttackPacket.set_isbasicattack(false);
	AttackPacket.set_skillid(static_cast<int32>(SkillID));

	SEND_PACKET(AttackPacket, PKT_C_ATTACK);
}

void AMMODaeva::CheckAttackHit(const FAttackData& AttackData)
{
	TArray<FHitResult> OutHitResults;

	const float AttackRange = AttackData.TraceData.Range;
	const float AttackRadius = AttackData.TraceData.Radius;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false, this);
	FVector SweepStart = GetActorTransform().TransformPosition(AttackData.TraceData.StartOffset);
	FVector SweepEnd = SweepStart + AttackData.TraceData.Direction.GetSafeNormal() * AttackRange;

	bool bHitDetected = GetWorld()->SweepMultiByChannel(
		OutHitResults, SweepStart, SweepEnd, FQuat::Identity, 
		CCHANNEL_ATTACK, FCollisionShape::MakeSphere(AttackRadius), Params
	);

	if (!bHitDetected) return;

	for (const FHitResult& HitResult : OutHitResults)
	{
		AMMODaeva* HitActor = Cast<AMMODaeva>(HitResult.GetActor());
		if (!IsValid(HitActor) || HitActor->IsDead() || HitActor == this) continue;

		if (HitActor->IsA(AMMODaeva::StaticClass()) && !bIsPvpActive)
		{
			continue;
		}

		Protocol::C_AttackPacket AttackPacket;
		AttackPacket.set_playerid(MyId);
		AttackPacket.set_targetid(HitActor->GetMyId());
		AttackPacket.set_damage(10.f * AttackData.DamageMultiplier);
		AttackPacket.set_isbasicattack(AttackData.bRestoreManaOnHit);
		AttackPacket.set_skillid(static_cast<int32>(AttackData.bRestoreManaOnHit ? EMontageID::LB : EMontageID::Key1));

		SEND_PACKET(AttackPacket, PKT_C_ATTACK);

		// [마나 수급] 기본 공격(LMB) 타격 성공 시에만 마나 회복
		if (AttackData.bRestoreManaOnHit)
		{
			UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
			if (MyASC)
			{
				const UAOAttributeSet* AttributeSet = MyASC->GetSet<UAOAttributeSet>();
				if (AttributeSet)
				{
					float CurrentMana = AttributeSet->GetMana();
					float MaxMana = AttributeSet->GetMaxMana();
					float NewMana = FMath::Min(CurrentMana + MMOHitManaRegenAmount, MaxMana);
					
					MyASC->SetNumericAttributeBase(UAOAttributeSet::GetManaAttribute(), NewMana);
				}
			}
		}
	}
}

void AMMODaeva::SetPvpState(bool bActive, float RemainingTime)
{
	bIsPvpActive = bActive;
	PvpRemainingTime = RemainingTime;
	
	UE_LOG(LogTemp, Log, TEXT("PVP 모드 변경: %s, 남은 시간: %.1f초"), bActive ? TEXT("활성화") : TEXT("비활성화"), RemainingTime);
}

bool AMMODaeva::HasMontage(EMontageID MontageID) const
{
	FProperty* Property = ADaeva::StaticClass()->FindPropertyByName(TEXT("Montages"));
	if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		const void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(this);
		FScriptMapHelper MapHelper(MapProperty, MapPtr);
		
		for (int32 i = 0; i < MapHelper.Num(); ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				const uint8* KeyPtr = MapHelper.GetKeyPtr(i);
				if (KeyPtr && *reinterpret_cast<const EMontageID*>(KeyPtr) == MontageID)
				{
					const uint8* ValuePtr = MapHelper.GetValuePtr(i);
					if (ValuePtr)
					{
						UAnimMontage* Montage = *reinterpret_cast<UAnimMontage* const*>(ValuePtr);
						return Montage != nullptr;
					}
				}
			}
		}
	}
	return false;
}

void AMMODaeva::EnterCombatState()
{
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	if (MyASC)
	{
		if (!MyASC->HasMatchingGameplayTag(STATE_COMBAT))
		{
			MyASC->AddLooseGameplayTag(STATE_COMBAT);
		}
		
		// 5초 후에 비전투 상태로 전환 (이미 타이머가 돌고 있다면 리셋)
		GetWorldTimerManager().SetTimer(CombatStateTimerHandle, this, &AMMODaeva::ExitCombatState, 5.0f, false);
	}
}

void AMMODaeva::ExitCombatState()
{
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	if (MyASC && MyASC->HasMatchingGameplayTag(STATE_COMBAT))
	{
		MyASC->RemoveLooseGameplayTag(STATE_COMBAT);
	}
}

void AMMODaeva::RegenerateMana()
{
	if (IsDead()) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	// 최근 공격 시점으로부터 3초간 공격을 하지 않았고, 현재 마우스를 누르고 있지 않은 경우 마나 회복 진행
	if (!bIsLMBHeld && (CurrentTime - LastAttackTime > 3.0f))
	{
		UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
		if (MyASC)
		{
			const UAOAttributeSet* AttributeSet = MyASC->GetSet<UAOAttributeSet>();
			if (AttributeSet)
			{
				float CurrentMana = AttributeSet->GetMana();
				float MaxMana = AttributeSet->GetMaxMana();
				if (CurrentMana < MaxMana)
				{
					float NewMana = FMath::Min(CurrentMana + (ManaRegenRate * ManaRegenInterval), MaxMana);
					MyASC->SetNumericAttributeBase(UAOAttributeSet::GetManaAttribute(), NewMana);
				}
			}
		}
	}
}

void AMMODaeva::InputSpacePressed()
{
	if (GetCharacterMovement()->MovementMode == MOVE_Custom &&
		GetCharacterMovement()->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::StopGlide));
		
		if (IsLocallyControlled())
		{
			Protocol::C_JumpPacket JumpPacket;
			JumpPacket.set_playerid(MyId);
			JumpPacket.set_isgliding(false);

			SEND_PACKET(JumpPacket, PKT_C_JUMP);
		}
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		RequestStopSprint();
		GASInputPressed(static_cast<int32>(EAbilityID::Glide));

		if (IsLocallyControlled())
		{
			Protocol::C_JumpPacket JumpPacket;
			JumpPacket.set_playerid(MyId);
			JumpPacket.set_isgliding(true);

			SEND_PACKET(JumpPacket, PKT_C_JUMP);
		}
		return;
	}


	GASInputPressed(static_cast<int32>(EAbilityID::Jump));

	if (IsLocallyControlled())
	{
		Protocol::C_JumpPacket JumpPacket;
		JumpPacket.set_playerid(MyId);
		JumpPacket.set_isgliding(false);

		SEND_PACKET(JumpPacket, PKT_C_JUMP);
	}
}

void AMMODaeva::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (IsLocallyControlled())
	{
		bool bWasGliding = (PrevMovementMode == MOVE_Custom && PreviousCustomMode == static_cast<uint8>(EAOMovementMode::Glide));
		bool bIsGliding = (GetCharacterMovement()->MovementMode == MOVE_Custom && GetCharacterMovement()->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide));

		if (bWasGliding && !bIsGliding)
		{
			Protocol::C_JumpPacket JumpPacket;
			JumpPacket.set_playerid(MyId);
			JumpPacket.set_isgliding(false); // Stopped gliding

			SEND_PACKET(JumpPacket, PKT_C_JUMP);
		}
	}
}

static bool IsBodyMontageConfigured(const ADaeva* Daeva, EMontageID MontageID)
{
	if (!Daeva) return false;

	FMapProperty* MapProp = CastField<FMapProperty>(ADaeva::StaticClass()->FindPropertyByName(TEXT("Montages")));
	if (MapProp)
	{
		FScriptMapHelper MapHelper(MapProp, MapProp->ContainerPtrToValuePtr<void>(Daeva));
		for (int32 i = 0; i < MapHelper.Num(); ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				const uint8* KeyPtr = MapHelper.GetKeyPtr(i);
				if (KeyPtr && *KeyPtr == static_cast<uint8>(MontageID))
				{
					const uint8* ValuePtr = MapHelper.GetValuePtr(i);
					if (ValuePtr)
					{
						UAnimMontage* Montage = *reinterpret_cast<UAnimMontage* const*>(ValuePtr);
						return Montage != nullptr;
					}
				}
			}
		}
	}
	return false;
}

static bool IsWingMontageConfigured(const ADaeva* Daeva, EMontageID MontageID)
{
	if (!Daeva) return false;

	FMapProperty* MapProp = CastField<FMapProperty>(ADaeva::StaticClass()->FindPropertyByName(TEXT("WingMontages")));
	if (MapProp)
	{
		FScriptMapHelper MapHelper(MapProp, MapProp->ContainerPtrToValuePtr<void>(Daeva));
		for (int32 i = 0; i < MapHelper.Num(); ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				const uint8* KeyPtr = MapHelper.GetKeyPtr(i);
				if (KeyPtr && *KeyPtr == static_cast<uint8>(MontageID))
				{
					const uint8* ValuePtr = MapHelper.GetValuePtr(i);
					if (ValuePtr)
					{
						UAnimMontage* Montage = *reinterpret_cast<UAnimMontage* const*>(ValuePtr);
						return Montage != nullptr;
					}
				}
			}
		}
	}
	return false;
}

void AMMODaeva::ReceiveJumpPacket(bool bIsGliding)
{
	if (bIsGliding)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Custom, static_cast<uint8>(EAOMovementMode::Glide));
		
		if (USkeletalMeshComponent* WingMesh = GetWingMesh())
		{
			WingMesh->SetVisibility(true);
		}

		USkeletalMeshComponent* CapeMesh = nullptr;
		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
		for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
		{
			if (MeshComp && MeshComp->GetName() == TEXT("CapePart"))
			{
				CapeMesh = MeshComp;
				break;
			}
		}
		if (CapeMesh)
		{
			CapeMesh->SetVisibility(false);
		}

		if (IsBodyMontageConfigured(this, EMontageID::Glide))
		{
			Multicast_PlayMontage(EMontageID::Glide, 1.0f);
		}
		if (IsWingMontageConfigured(this, EMontageID::Glide))
		{
			Multicast_PlayWingMontage(EMontageID::Glide, 1.0f);
		}
	}
	else
	{
		if (GetCharacterMovement()->MovementMode == MOVE_Custom && 
		    GetCharacterMovement()->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide))
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			
			if (USkeletalMeshComponent* WingMesh = GetWingMesh())
			{
				WingMesh->SetVisibility(false);
			}

			USkeletalMeshComponent* CapeMesh = nullptr;
			TArray<USkeletalMeshComponent*> SkeletalMeshes;
			GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
			for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
			{
				if (MeshComp && MeshComp->GetName() == TEXT("CapePart"))
				{
					CapeMesh = MeshComp;
					break;
				}
			}
			if (CapeMesh)
			{
				CapeMesh->SetVisibility(true);
			}

			if (IsBodyMontageConfigured(this, EMontageID::GlideLand))
			{
				Multicast_PlayMontage(EMontageID::GlideLand, 2.0f);
			}
			if (IsWingMontageConfigured(this, EMontageID::GlideLand))
			{
				Multicast_PlayWingMontage(EMontageID::GlideLand, 2.0f);
			}
		}
		else
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			OnJumped();
		}
	}
}