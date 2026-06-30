// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOWidgetComponentBase.h"

#include "UI/AOUserWidgetBase.h"

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"

#include "UI/AOMonsterHUDWidget.h"
#include "Character/Monster/AOMonsterBase.h"

UAOWidgetComponentBase::UAOWidgetComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetComponentTickInterval(0.2f);
}

void UAOWidgetComponentBase::InitWidget()
{
	Super::InitWidget();

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UAOUserWidgetBase* AOUserWidget = Cast<UAOUserWidgetBase>(GetWidget());
	if (!AOUserWidget)
	{
		return;
	}

	// Super::InitWidget() 상위 로직을 따라가보면, 
	// 함수 실행 과정에서 CreateWidget을 통해 Widget이 생성됨.
	// 그 이후에 여기가 실행됨. 따라서 Widget 초기화를 보장 받을 수 있음.
	// AbilitySystem이 있으면 Bind.
	AOUserWidget->BindToAbilitySystemActor(GetOwner());



	// 만약 몬스터 Widget이면 아래 과정을 추가.
	if (UAOMonsterHUDWidget* MonsterHUD = Cast<UAOMonsterHUDWidget>(AOUserWidget))
	{
		if (const AAOMonsterBase* Monster = Cast<AAOMonsterBase>(GetOwner()))
		{
			MonsterHUD->SetMonsterIndex(Monster->DungeonBossIndex);
		}
	}
}

void UAOWidgetComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UpdateDistanceVisibility();
}

void UAOWidgetComponentBase::UpdateDistanceVisibility()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		SetVisibility(false, true);
		return;
	}

	APlayerController* LocalPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	APawn* LocalPawn = LocalPC ? LocalPC->GetPawn() : nullptr;

	if (!LocalPawn)
	{
		SetVisibility(false, true);
		return;
	}

	// 내 캐릭터 머리 위 UI는 이 거리 판정에서 제외.
	if (bIgnoreOwningLocalPlayer && OwnerActor == LocalPawn)
	{
		SetVisibility(true, true);
		return;
	}

	const float MaxDistanceSq = FMath::Square(MaxVisibleDistance);
	const float CurrentDistanceSq = FVector::DistSquared(
		LocalPawn->GetActorLocation(),
		OwnerActor->GetActorLocation()
	);

	SetVisibility(CurrentDistanceSq < MaxDistanceSq, true);
}
