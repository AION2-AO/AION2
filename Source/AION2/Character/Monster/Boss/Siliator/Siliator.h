#pragma once

#include "CoreMinimal.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Siliator.generated.h"

UCLASS()
class AION2_API ASiliator : public AAOMonsterBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void InitAttributeSet() override;
};
