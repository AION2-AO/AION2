// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOSkillQuickSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Data/AOSkillSlotViewData.h"

void UAOSkillQuickSlotWidget::AddSkillSlotViewData(const FAOSkillSlotViewData& InViewData)
{
    ViewDataByComboIndex.Add(InViewData);
}

void UAOSkillQuickSlotWidget::ClearSkillSlotViewData()
{
    ViewDataByComboIndex.Empty();
}

void UAOSkillQuickSlotWidget::SetCurrentSkillIndex(int32 InNewIndex)
{
    if (!ViewDataByComboIndex.IsValidIndex(InNewIndex))
    {
        return;
    }

    CurrentSkillIndex = InNewIndex;

    SetSkillIcon(ViewDataByComboIndex[CurrentSkillIndex].Icon);
}

void UAOSkillQuickSlotWidget::SetSkillIcon(UTexture2D* Icon)
{
    if (!SkillImage || !Icon)
    {
        return;
    }

    // 두 번째 인자는 기존 ImageBrush의 Size를 
    // 새로 배정된 Texture의 Size에 맞출 것인지를 묻는 것.
    if (Icon)
    {
        SkillImage->SetBrushFromTexture(Icon, false);
        SkillImage->SetOpacity(1.0f);
    }

    // SizeBox로 감싸둬도 괜찮다!
}

const FAOSkillSlotViewData* UAOSkillQuickSlotWidget::GetCurrentSkillSlotViewData() const
{
    if (!ViewDataByComboIndex.IsValidIndex(CurrentSkillIndex))
    {
        return nullptr;
    }
    return &ViewDataByComboIndex[CurrentSkillIndex];
}

void UAOSkillQuickSlotWidget::PlaySkillPressedFeedback()
{
    if (!SlotButton)
    {
        return;
    }

    BP_PlayPressedFeedback();
}

void UAOSkillQuickSlotWidget::HandleComboInput()
{
    if (CurrentSkillIndex + 1  < ViewDataByComboIndex.Num())
    {
        CurrentSkillIndex++;
    }
    else
    {
        CurrentSkillIndex = 0;
    }


    UE_LOG(LogTemp, Warning, TEXT("%d"), CurrentSkillIndex);
    SetCurrentSkillIndex(CurrentSkillIndex);
}

void UAOSkillQuickSlotWidget::ResetComboInput()
{
    if (ViewDataByComboIndex.Num() > 0)
    {
        SetCurrentSkillIndex(0);
    }
}

void UAOSkillQuickSlotWidget::StartCooldown(float RemainingTime, float Duration)
{
    if (RemainingTime <= 0.0f || Duration <= 0.0f)
    {
        return;
    }

    RemainingTime = FMath::Clamp(RemainingTime, 0, Duration);

    // 실제 쿨다운 판정은 GAS가 담당한다.
    // 여기서는 ASC에서 조회한 남은 시간/전체 시간을 Blueprint UI에 넘겨 시각화만 한다.
    BP_StartCooldown(RemainingTime, Duration);
}

void UAOSkillQuickSlotWidget::StopCooldown()
{
    // CooldownTag가 ASC에서 제거되었으므로 슬롯의 쿨다운 표시를 종료한다.
    BP_StopCooldown();
}

