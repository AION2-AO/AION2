// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Monster_TH_OrbGimmickAttack.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "Character/Daeva/Daeva.h"

void UGA_Monster_TH_OrbGimmickAttack::OnCheckAttackHitEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	ATalythra* pTalythra = Cast<ATalythra>(GetAvatarActorFromActorInfo());

	if (!pTalythra)
	{
		return;
	}

	
	
	
	TArray<class ADaeva*> ArrayDaeva = pTalythra->Get_ArrayOrbHittedDaeva();


	// 여기서 현재 탈리스라의 공격 색가져오고
	// 플레이어의 현재 matching color와 같은지 확인해서 
	// 같다면 공격x 틀리다면 공격 허용  o
	// 현재 그러면 color만 지정해주면됨 탈리스라에 

	for(auto& iter : ArrayDaeva)
	{
		if (iter->Get_CurrentDaevaHasSheildColor() == pTalythra->Get_AttackOrbColor())
		{
			continue;
		}

		else
			pTalythra->CheckAttackHit(AttackData);
	}


	//AOCharacter->CheckAttackHit(AttackData);
}