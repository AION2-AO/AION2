// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOSlotWidgetBase.h"
#include "Data/DA_AbilitySet.h"
#include "AOSkillQuickSlotWidget.generated.h"


class UBtton;
class UImage;
class UTextBlock;
class UUserWidget;


/**
 * [각 SkillSlot에 대한 Widget]
 * 표시 / 상태만 담당
 *  (이 Slot의 담당 AbilityID는 상위인 QuickSlotSkillHUD가 이미 알고 있으므로)
 * 
 * [이 Widget이 표시/가져야 할 것] => 여러 AbilityID를 등록 가능하도록.
 * => Icon, Level, CooldownTag, EffectWidgetClass, ASC(Bound)
 * 
 * [이 Widget이 시행해야 할 것]
 * 아이콘/레벨/쿨타임/이펙트 표시, 눌림 피드백 재생
 */

UCLASS()
class AION2_API UAOSkillQuickSlotWidget : public UAOSlotWidgetBase
{
	GENERATED_BODY()
	
public:
	void InitSkillSlot(const FGAData& InAbilityData);

	// Pressed Feedback (Not Effect)
	void PlayPressedFeedback();


	void StartCooldown(float RemainingTime, float Duration);
	void StopCooldown();
	
	
	void ShowEffectWidget();

private:
	// Init the Icon
	void SetSkillIcon(UTexture2D* Icon);

	// Set Skill Level Text
	void SetSkillLevel(int32 InLevel);

protected:
	// Button은 상위 class를 물려받아 "SlotButton"으로 존재.

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UImage> SkillImage;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TB_SkillLevel;

protected:
	ESlotType slotType = ESlotType::Skill_Quick;
	
	
	//매번 인자를 받기보다, 내부 멤버를 쓰도록 함: Play Effect용 private member.
private:

	// UClass 계열 UObject 참조라서 /UPROPERTY()/를 붙이는 게 안전.
	UPROPERTY()
	TSubclassOf<UUserWidget> EffectWidgetClass;

	// FGameplayTag는 GC 대상이 아니기 때문에 일반 private으로 둬도 된다. => 나중에 더 알아보기
	FGameplayTag CooldownTag;
};
