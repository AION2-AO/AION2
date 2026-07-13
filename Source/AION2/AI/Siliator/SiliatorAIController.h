#pragma once

#include "CoreMinimal.h"
#include "AI/AIMonsterControllerBase.h"
#include "SiliatorAIController.generated.h"

UCLASS()
class AION2_API ASiliatorAIController : public AAIMonsterControllerBase
{
	GENERATED_BODY()
	
public:
	virtual void TargetPerceptionOn(AActor* Actor, FAIStimulus  Stimlus) override;

protected:
	virtual bool RefreshOrReset() override;
};
