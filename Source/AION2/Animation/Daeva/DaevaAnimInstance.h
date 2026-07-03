#pragma once

#include "CoreMinimal.h"
#include "Animation/AOCharacterAnimInstance.h"
#include "DaevaAnimInstance.generated.h"

UCLASS()
class AION2_API UDaevaAnimInstance : public UAOCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	bool bIsGliding = false;

	UPROPERTY(BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	bool bIsCombat = false;
};
