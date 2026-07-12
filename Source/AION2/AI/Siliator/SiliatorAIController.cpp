#include "AI/Siliator/SiliatorAIController.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Character/Daeva/Daeva.h"
#include "GAS/AOGameplayTags.h"

#include "Perception/AIPerceptionTypes.h"
#include "Game/DungeonGameState.h"

void ASiliatorAIController::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
{
	if (HasAuthority() == false) // 타겟 인식은 서버에서만 할 수 있게 설정. 
		return;

	ADaeva* pPlayer = Cast<ADaeva>(Actor);
	if (pPlayer == nullptr || pPlayer->IsDead() == true)
	{
		return;
	}

	// 감지 범위 안에 들어오거나 감지 상태가 갱신될때 마다 호출됨 
	if (Stimlus.WasSuccessfullySensed())
	{
		// 처음 보스를 마주할 때 보스가 전투준비 페이즈로 갈 수 있게 설정. 
		if (HasDetectedTarget == false)
		{
			PhaseTag = PHASE_MONSTER_COMBAT;
			ControlledMonster->Set_Phase(PHASE_MONSTER_COMBAT);
			CurrentTargetPlayer = Actor;
			HasDetectedTarget = true;

			// 보스 브금 재생 
			if (auto* GS = GetWorld()->GetGameState<ADungeonGameState>())
			{
				GS->SetMusic(EDungeonMusic::Boss);
			}
		}

		// 해당 플레이어가 시야에 처음 인식되었을 때 
		if (ArrayTargetPlayers.Find(Actor) == -1)
		{
			pPlayer->OnPlayerDead.AddUniqueDynamic(this, &AAIMonsterControllerBase::OnTargetDead);
			ArrayTargetPlayers.Add(Actor);
		}
	}
}
