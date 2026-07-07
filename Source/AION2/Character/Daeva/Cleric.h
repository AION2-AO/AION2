#pragma once

#include "CoreMinimal.h"
#include "Character/Daeva/Daeva.h"
#include "Cleric.generated.h"

UCLASS()
class AION2_API ACleric : public ADaeva
{
	GENERATED_BODY()

public:
	ACleric(const FObjectInitializer& ObjectInitializer);
};
