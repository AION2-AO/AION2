#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "DA_AbilitySet.generated.h"

class UTexture2D;
class UUserWidget;

USTRUCT(BlueprintType)
struct FGAData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    TSubclassOf<class UGameplayAbility> Ability = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    int32 AbilityLevel = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    int32 AbilityID = -1;

    // == Skill에 따른 UI 반응 ==
    // HUD에서 몇 번째 스킬 슬롯에 들어갈지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    int32 SlotIndex = -1;

    // 스킬 버튼 아이콘
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTexture2D> Icon = nullptr;

    // 이 스킬의 쿨타임 태그. 예: Cooldown.Assassin.Key1
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (Categories = "Cooldown"))
    FGameplayTag CooldownTag;

    // 스킬 사용 시 HUD에 띄울 효과 위젯 (만약 있다면)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> EffectWidgetClass;
};

UCLASS()
class AION2_API UDA_AbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Ability Set")
    TArray<FGAData> Abilities;

    void GiveToASC(class UAbilitySystemComponent* ASC, TArray<FGameplayAbilitySpecHandle>& OutHandles) const;

    // UI에서 Data를 가져갈 수 있도록 하는 함수
    UFUNCTION(BlueprintCallable, Category = "Ability Set")
    bool GetAbilityDataByInputID(int32 AbilityID, FGAData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Ability Set")
    const TArray<FGAData>& GetAbilities() const { return Abilities; }
};
