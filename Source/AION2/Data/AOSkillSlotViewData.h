#pragma once


#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AOSkillSlotViewData.generated.h"

/*
* UI에 불필요한 Data를 분리하기 위해, 새로운 struct 생성
* Widget 입장에서는 ASC 원본 구조체를 알 필요가 없다.
* 받아온 FGAData에서의 변환은 이 class에서 전담하도록 함.
*/
USTRUCT(BlueprintType)
struct FAOSkillSlotViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SkillSlotView")
	int32 AbilityID = -1;

	UPROPERTY(BlueprintReadOnly, Category = "SkillSlotView")
	int32 AbilityLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "SkillSlotView")
	TObjectPtr<UTexture2D> Icon = nullptr;


	// === Struct 내부의, CooldownTag 기준의 중앙 상태 ===
	UPROPERTY(BlueprintReadOnly, Category = "SkillSlotView")
	FGameplayTag CooldownTag;

	UPROPERTY(BlueprintReadOnly, Category = "SkillSlotView")
	float RemainingTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SkillSlotView")
	float Duration = 0.0f;
	// === Cool이 돌고 있다면 해당 Slot 정보를 복사해 다른 곳에 생성해줘야 하므로 추가 ===

};