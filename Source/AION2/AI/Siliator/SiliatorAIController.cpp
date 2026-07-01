#include "AI/Siliator/SiliatorAIController.h"
#include "Character/Monster/AOMonsterBase.h"
#include "GAS/AOGameplayTags.h"

#include "Perception/AIPerceptionTypes.h"

void ASiliatorAIController::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
{
	if (HasAuthority())
	{
		if (HasDetectedTarget == false)
		{
			HasDetectedTarget = true;

			PhaseTag = PHASE_MONSTER_COMBAT;
			ControlledMonster->Set_Phase(PHASE_MONSTER_COMBAT);

			ArrayTargetPlayers.Add(Actor);
			CurrentTargetPlayer = Actor;
		}
	}
}
