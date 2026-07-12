// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/AT/Monster/AT_Monster_ChargeHitTick.h"

UAT_Monster_ChargeHitTick::UAT_Monster_ChargeHitTick()
{
	bTickingTask = true;  // 매 틱마다 task 하도록 설정 
	bSimulatedTask = false;  // 서버에서만 돌리도록 설정
}

UAT_Monster_ChargeHitTick* UAT_Monster_ChargeHitTick::CreateChargeHitTick(UGameplayAbility* OwningAbility)
{
	UAT_Monster_ChargeHitTick* Task = NewAbilityTask<UAT_Monster_ChargeHitTick>(OwningAbility);

	return Task;
}


void UAT_Monster_ChargeHitTick::Activate()
{
	Super::Activate(); // 여기서 특별히 할 건 없음. 틱은 TickTask에서 처리. 


}

void UAT_Monster_ChargeHitTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast();
	}


}

void UAT_Monster_ChargeHitTick::OnDestroy(bool bInOwnerFinished)
{
	// 정리 로직이 필요하면 여기에
	Super::OnDestroy(bInOwnerFinished);
}
