// Fill out your copyright notice in the Description page of Project Settings.

#include "AOMonsterBase.h"

/* 호영 */
#include "Game/AODungeonGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "AIController.h"
#include "BrainComponent.h"
/* ======   */

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "GAS/AOGameplayTags.h"
#include "Data/DA_AbilitySet.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Net/UnrealNetwork.h"

// Targeting UI
#include "UI/AOWidgetComponentBase.h"
#include "UI/AOUserWidgetBase.h"
#include "UI/AOMonsterHUDWidget_Targetable.h"

// Sets default values
AAOMonsterBase::AAOMonsterBase(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.


    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);


    AttributeSet = CreateDefaultSubobject<UAOAttributeSet>(TEXT("AttributeSet"));


    bReplicates = true;
    SetReplicateMovement(true);


	// TargetWidgetComponent
	TargetWidgetComponent = CreateDefaultSubobject<UAOWidgetComponentBase>(TEXT("TargetWidget"));
	TargetWidgetComponent->SetupAttachment(GetMesh(), TEXT("Bip001-Spine2"));
	TargetWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	TargetWidgetComponent->SetDrawSize(FVector2D(64.0f, 64.0f));
	TargetWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetWidgetComponent->bUseDistanceVisibility = false;
	TargetWidgetComponent->SetVisibility(false, true);

	static ConstructorHelpers::FClassFinder<UAOUserWidgetBase>
		TargetWidgetClass(
			TEXT("/Game/UI/Ingame/WBP_TargetMarker.WBP_TargetMarker_C"));

	if (TargetWidgetClass.Succeeded())
	{
		TargetWidgetComponent->SetWidgetClass(
			TargetWidgetClass.Class);
	}


#pragma region UI
	// Head-up UI
	OverheadStatusWidgetComponent = CreateDefaultSubobject<UAOWidgetComponentBase>(TEXT("OverheadStatusWidget"));
	OverheadStatusWidgetComponent->SetupAttachment(RootComponent);
	OverheadStatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadStatusWidgetComponent->SetDrawSize(FVector2D(150.0f, 80.0f));
	OverheadStatusWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 350.0f));
	OverheadStatusWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FClassFinder<UAOMonsterHUDWidget_Targetable>
		WidgetClass(
			TEXT("/Game/UI/Ingame/WBP_MonsterStatus_Head.WBP_MonsterStatus_Head_C"));

	if (WidgetClass.Succeeded())
	{
		OverheadStatusWidgetComponent->SetWidgetClass(
			WidgetClass.Class);
	}

	OverheadStatusWidgetComponent->SetMaxVisibleDistance(6000.0f);
#pragma endregion

}

// Called when the game starts or when spawned
void AAOMonsterBase::BeginPlay()
{
    Super::BeginPlay();

	// 클라에서 호출
	InitGAS();

	// UI
	if (TargetWidgetClass)
	{
		TargetWidgetComponent->SetWidgetClass(TargetWidgetClass);
	}

	TargetWidgetComponent->SetVisibility(false, true);
}

void AAOMonsterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

	// 서버에서 호출
    InitGAS();


	// 선환 추가 
	SetGenericTeamId(FGenericTeamId(TEAM_PERCEPTION_DAEVA)); // 플레이어 팀

}

void AAOMonsterBase::InitGAS()
{
    // Owner Actor란? : ASC를 논리적으로 소유한 주체 ( 해당 주체가 죽어도 유지되며, 레벨 및 스텟 보존 )
    // Avatar Actor란? : Character  (죽으면 바뀌는 물체, 실제로 데이터를 처리하지 않지만 비주얼만 수행해주는 엑터) 
    ASC->InitAbilityActorInfo(this, this);

    if (!ASC->HasMatchingGameplayTag(TEAM_MONSTER))
    {
        ASC->AddLooseGameplayTag(TEAM_MONSTER);
    }


    if (HasAuthority() == false)
        return;

    // 서버 로직 

    // ASC에 능력 넣어주는 작업  ( AbilitySet data는 몬스터의 Blueprint에서 설정 ) 
    AbilitySet->GiveToASC(ASC, AbilityHandles);

	// Delegate
	if (!HealthChangedDelegateHandle.IsValid())
	{
		HealthChangedDelegateHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(
				UAOAttributeSet::GetHealthAttribute()
			).AddUObject(this, &AAOMonsterBase::OnHealthChanged);
	}

	//H.Y
	if (!GroggyChangedDelegateHandle.IsValid())
	{
		GroggyChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UAOAttributeSet::GetGroggyAttribute()).AddUObject(this, &AAOMonsterBase::OnGroggyChanged);
	}
	//

	bIsDead = false;
	//H.Y
	bIsGroggy = false;
	//

}

void AAOMonsterBase::ClearGAS()
{
	if (ASC && HealthChangedDelegateHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UAOAttributeSet::GetHealthAttribute()
		).Remove(HealthChangedDelegateHandle);

		HealthChangedDelegateHandle.Reset();
	}

	// H.Y
	if (ASC && GroggyChangedDelegateHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UAOAttributeSet::GetGroggyAttribute()).Remove(GroggyChangedDelegateHandle);

		GroggyChangedDelegateHandle.Reset();
	}
	//

	Super::ClearGAS();
}

void AAOMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAOMonsterBase, Phase);
	DOREPLIFETIME(AAOMonsterBase, State);

	// H.Y
	DOREPLIFETIME(AAOMonsterBase, bIsGroggy);
	//
}


// Called every frame
void AAOMonsterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

UAnimMontage* AAOMonsterBase::GetMontageByTag(const FGameplayTag& MontageTag) const
{
    if (const TObjectPtr<UAnimMontage>* FoundMontage = MontageMap.Find(MontageTag))
    {
        return *FoundMontage;
    }

    return nullptr;
}

void AAOMonsterBase::InitAttributeSet()
{


}

// Boss Health 0 -> Call
// Only Server Call
void AAOMonsterBase::HandleBossDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	// 수연: 아직 시체만 남은 채로 Head-up Display를 Destroy. | 26.07.05
	if (OverheadStatusWidgetComponent)
	{
		OverheadStatusWidgetComponent->DestroyComponent();
		OverheadStatusWidgetComponent = nullptr;
	}


}

void AAOMonsterBase::HandleBossDeathMontageEnd()
{
	/* 선환 */
// 죽음 일시 해당 태그를 통해 확인하고 해당 ability가 끝나면 다음 구문을 이어서 호출하도록 하기 
	FGameplayTag PhaseDeadTag = FGameplayTag::RequestGameplayTag(FName("Phase.Monster.Dead"));
	if (Phase.MatchesTagExact(PhaseDeadTag) == false)
	{
		return;
	}


	UE_LOG(LogTemp, Warning, TEXT("[Monster Death] %s Died"), *GetName());

	// 클라이언트/서버 공통: 더 이상 이동, 충돌, 피격이 일어나지 않도록 처리
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ASC)
	{
		ASC->CancelAllAbilities();

		const FGameplayTag DeadTag =
			FGameplayTag::RequestGameplayTag(FName("State.Dead"));

		ASC->AddLooseGameplayTag(DeadTag);
	}

	// AI 중지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Monster Dead"));
		}
	}

	// 던전 진행은 서버에서만 결정
	if (!HasAuthority())
	{
		return;
	}

	// 여기서 사망 몽타주 재생
	// Multicast_PlayDeathMontage();

	// DungeonBossIndex가 1~3이면 던전 보스
	if (DungeonBossIndex >= 1 && DungeonBossIndex <= 3)
	{
		AAODungeonGameMode* DungeonGameMode =
			Cast<AAODungeonGameMode>(UGameplayStatics::GetGameMode(this));

		if (DungeonGameMode)
		{
			DungeonGameMode->NotifyBossDefeated(this);
		}
	}
}

// 호영 작성 
void AAOMonsterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Monster Health] %s : %.1f -> %.1f"),
		*GetName(),
		Data.OldValue,
		Data.NewValue
	);

	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		HandleBossDeath();
	}
}

void AAOMonsterBase::SetTargetWidgetVisible(bool bVisible)
{
	if (TargetWidgetComponent)
	{
		TargetWidgetComponent->SetVisibility(bVisible, true);
	}

	if (OverheadStatusWidgetComponent)
	{
		if (UAOMonsterHUDWidget_Targetable* StatusWidget =
			Cast<UAOMonsterHUDWidget_Targetable>(OverheadStatusWidgetComponent->GetUserWidgetObject()))
		{
			StatusWidget->SetMonsterHUDVisibility(bVisible);
		}
	}
}

// 호영 작성 
void AAOMonsterBase::OnGroggyChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("Monster Groggy %s : %.1f -> %.1f"), *GetName(), Data.OldValue, Data.NewValue);

	if (!HasAuthority())
	{
		return;
	}

	if (Data.NewValue <= 0.0f && !bIsGroggy && !bIsDead)
	{
		StartGroggy();
	}
}

void AAOMonsterBase::StartGroggy()
{
	if (!HasAuthority() || bIsGroggy || bIsDead)
	{
		return;
	}

	bIsGroggy = true;

	UE_LOG(LogTemp, Warning, TEXT("%s groggy start"), *GetName());

	if (ASC)
	{
		const FGameplayTag GroggyTag = FGameplayTag::RequestGameplayTag(FName("State.Groggy"));

		ASC->AddLooseGameplayTag(GroggyTag);
		ASC->CancelAbilities();
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Monster Groggy"));
		}
	}
	GetCharacterMovement()->StopMovementImmediately();
}

void AAOMonsterBase::EndGroggy()
{
	if (!HasAuthority() || !bIsGroggy || bIsDead)
	{
		return;
	}

	bIsGroggy = false;

	if (AttributeSet)
	{
		AttributeSet->SetGroggy(AttributeSet->GetMaxGroggy());
	}

	if (ASC)
	{
		const FGameplayTag GroggyTag = FGameplayTag::RequestGameplayTag(FName("State.Groggy"));

		ASC->RemoveLooseGameplayTag(GroggyTag);
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}
}

void AAOMonsterBase::SetDungeonBossActive(bool bActive)
{
	// 외형 표시 여부
	SetActorHiddenInGame(!bActive);

	// Capsule Collision도 함께 제어
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(
			bActive
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision
		);
	}

	// Tick을 꺼야 AI 관련 Tick이나 행동이 남지 않는다.
	SetActorTickEnabled(bActive);

	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Dungeon] AIController is null: %s"),
			*GetName()
		);
		return;
	}

	if (!AIController->BrainComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Dungeon] BrainComponent is null: %s"),
			*GetName()
		);
		return;
	}

	if (!bActive)
	{
		AIController->StopMovement();

		AIController->BrainComponent->StopLogic(
			TEXT("Dungeon Boss Disabled")
		);

		return;
	}

	AIController->BrainComponent->RestartLogic();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Dungeon] Boss AI Activated: %s"),
		*GetName()
	);
}

// 호영 작성 
void AAOMonsterBase::Die()
{
	HandleBossDeath();
}



bool AAOMonsterBase::CanMoveOnNavMesh(const FVector Direction, float Distance)
{
    const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

    if (!NavSys)
        return false;

    FVector Dir2D = Direction;
    Dir2D.Z = 0.f;

    if (!Dir2D.Normalize())
        return false;

    // Start를 발바닥 높이로 수정  
    const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    FVector Start = GetActorLocation();
    Start.Z -= HalfHeight;

    const FVector End = Start + Dir2D * Distance;

    FVector HitLocation;
    const bool bHit = NavSys->NavigationRaycast(GetWorld(), Start, End, HitLocation);


    return !bHit;
}
