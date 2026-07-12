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
    /*
    * HUD에서 몇 번째 스킬 슬롯에 들어갈지.
    * 지금은 콤보 Skill을 통해 한 Slot에 여러 키가 들어가도록 하는 데 사용.
    * 추후 유저가 Skill을 자유 배치할 수 있도록 허용할 때도 아래 Index를 쓸 수 있음.
    */

    // 스킬 버튼 아이콘
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTexture2D> Icon = nullptr;
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
