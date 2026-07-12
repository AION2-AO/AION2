// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "AOQuickSkillHUD.generated.h"

class UAOSkillQuickSlotWidget;
class UHorizontalBox;
class UAOWidgetBase;

class ADaeva;

class UDA_AbilitySet;

struct FAOSkillSlotViewData;

/**
 * = PlayerHUDWidget 하위에서 =  
 * [Player의 각 Skill에 대해 시각적으로 반응하는 SkillSlot들을 소유 및 관리]
 * - 어떤 AbilityID가 어떤 Slot에 연결될지 && ASC/AbilitySet을 알고있음
 */
UCLASS()
class AION2_API UAOQuickSkillHUD : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
public:
	// 상위 Widget에서 호출
	virtual void BindToASC(UAbilitySystemComponent* InASC) override;

	virtual void UnbindFromASC() override;

protected:
	virtual void NativeOnInitialized() override;

public:
	// PlayerController로부터 타고 태려와 InputId에 맞는 slot에 전달할 함수.
	void PlaySkillPressedFeedback(int32 InputId);


private:
	void InitSkillSlots(const UDA_AbilitySet* InAbilitySet);

	// ASC combo tag에 Event 구독.
	void BindComboDelegates();

	// 재 Bind / Destruct 전에 같은 ASC에서 제거
	void UnbindComboDelegates();

private:
	// =============== Skill Combo Event =====================
	/*
	* BindComboDelegates에서 GameplayTag마다 묶어줄 Evnet.
	* parameter를 넣고는 있으나, 사용할 Delegate가 TwoParams를 전제로 하므로 갖는 것.
	* 지금은 RefreshComboSlots만 사용하고 있으므로 parameter의 값은 신경 안써도 됨.
	*
	* // NewCount는 GameplayTag의 stack count!
	* // => Count가 0이 됐다면 ComboTag가 떼어진 것이라고 보면 됨.
	*/
	void HandleLBComboTagChanged(FGameplayTag Tag, int32 NewCount);
	void HandleRBComboTagChanged(FGameplayTag Tag, int32 NewCount);

	// 등록한 Tag와 FDelegateHandle을 먼저 저장해서, 같은 ASC에서 제거 => 중복 바인딩 방지
	TArray<TPair<FGameplayTag, FDelegateHandle>> ComboTagDelegateHandles;



	// Combo가 끝나는 Delegate을 구독
	void BindComboInputCompletedDelegate(ADaeva* InDaeva);
	void UnbindComboInputCompletedDelegate();

	// ComboInput의 완료에 대한 Handler.
	void HandleComboInputCompleted(int32 SlotBaseInputId);

	TWeakObjectPtr<ADaeva> BoundComboInputDaeva;
	FDelegateHandle ComboInputCompletedDelegateHandle;

private:
	// =============== Skill CoolDown Event =====================
	void BindCooldownDelegates();
	void UnbindCooldownDelegates();

	// Cooldown이 시작되고, 떨어질 때를 가져옴.
	void HandleCooldownTagChanged(FGameplayTag CooldownTag, int32 NewCount);


	/*
	* Key Slot을 순회해서, 현재 Event로 받은 CooldownTag를 ViewData로 가진 Slot을 찾는다.
	* GAS Evnet는 Tag와 Count만 보내므로, 어떤 Slot에 Bind되어있는지 알 수 없음.
	* QuickSlot의 개수가 많지 않고, 슬롯 Array가 있으므로 이렇게 처리해줌.
	* [CooldownTag : SlotIndex]의 Map을 만들어줄 수는 있으나, 
	* 그러면 Event 시점에 현재 표시 중인지도 검사해야 함. => 시점이 엇갈리면 조건 검사가 애매해짐. 
	* + 지금 HUD에 있는 정보도 너무 많음.
	*/
	UAOSkillQuickSlotWidget* FindCurrentSlotByCooldownTag(FGameplayTag CooldownTag) const;


	// ASC에 이미 적용된 Cooldown GameplayEffect에서 남은 시간과 전체 시간을 조회.
	// 이 함수는 시간을 새로 세지 않고, GAS가 관리 중인 Active GameplayEffect 정보를 읽기만 함.
	bool GetCooldownTime(FGameplayTag CooldownTag, float& OutRemainingTime, float& OutDuration) const;

	
	// 등록한 Tag와 FDelegateHandle을 먼저 저장해서, 같은 ASC에서 제거 => 중복 바인딩 방지
	TArray<TPair<FGameplayTag, FDelegateHandle>> CooldownTagDelegateHandles;

protected:
	// === Skill Slots ===
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_1;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_2;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_3;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_4;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_Q;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_E;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_R;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOSkillQuickSlotWidget> Skill_T;

	// Binding된 위 Widget들을 편하게 관리하기 위해 Array에 넣음
	UPROPERTY()
	TArray<TObjectPtr<UAOSkillQuickSlotWidget>> SkillSlotArray;

	// Binding된 위 Widget들을 편하게 관리하기 위해 Map에 넣음
	UPROPERTY()
	TMap<int32, TObjectPtr<UAOSkillQuickSlotWidget>> SkillSlotByAbilityID;

	// 해당 Slot에서 현재 보여줄 Skill의 내부 Index. { SlotIndex : SkillID  }.
	// => Slot 내에서, viewData의 array를 들고 있도록 하고 입력을 받으면 변경.
	TMap<int32, int32> SkillSlotBySlotIndex;
	
	// === Buff Effect Area// 만약 생기면. ===
	TObjectPtr<UHorizontalBox> EffectArea;

	TObjectPtr<UAOWidgetBase> EffectSlotBase;
	

	UPROPERTY()
	TObjectPtr<const UDA_AbilitySet> BoundAbilitySet;
};
