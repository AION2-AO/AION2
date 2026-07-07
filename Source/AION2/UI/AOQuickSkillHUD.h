// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AOQuickSkillHUD.generated.h"

class UAOSkillQuickSlotWidget;
class UHorizontalBox;
class UAOWidgetBase;

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

protected:
	virtual void NativeConstruct() override;

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


	// Binding된 위 Widget들을 편하게 관리하기 위해 Map에 넣음
	UPROPERTY()
	TMap<int32, TObjectPtr<UAOSkillQuickSlotWidget>> SkillSlotByAbilityID;

	// === Effect Area ===
	TObjectPtr<UHorizontalBox> EffectArea;

	TObjectPtr<UAOWidgetBase> EffectSlotBase;
	
	
};
