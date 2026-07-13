// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PvpWidget.h"
#include "Components/TextBlock.h"

void UPvpWidget::UpdatePvpWidget(uint8 State, int32 RemainingSeconds)
{
	CurrentState = State;
	CurrentRemainingSeconds = RemainingSeconds;

	// 기존에 실행되던 타이머가 있다면 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	RefreshVisuals();

	if (CurrentRemainingSeconds > 0 && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UPvpWidget::TickTime, 1.0f, true);
	}
}

void UPvpWidget::NativeConstruct()
{
	if (PvpTimer)
	{
		PvpTimer->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPvpWidget::TickTime()
{
	if (CurrentRemainingSeconds > 0)
	{
		CurrentRemainingSeconds--;
		RefreshVisuals();
	}
	else
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		}
	}
}

void UPvpWidget::RefreshVisuals()
{
	int32 Min = CurrentRemainingSeconds / 60;
	int32 Sec = CurrentRemainingSeconds % 60;
	FString TimerStr = FString::Printf(TEXT("%02d:%02d"), Min, Sec);

	if (CurrentState == 1) // PVP_STATE_WARN_START
	{
		if (PvpText)
		{
			PvpText->SetText(FText::FromString(TEXT("시공의 균열이 곧 열립니다...")));
			PvpText->SetVisibility(ESlateVisibility::Visible);
		}
		if (PvpCountDown)
		{
			if (CurrentRemainingSeconds > 0)
			{
				PvpCountDown->SetText(FText::AsNumber(CurrentRemainingSeconds));
				PvpCountDown->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				PvpCountDown->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		if (PvpTimer)
		{
			PvpTimer->SetText(FText::FromString(TimerStr));
		}
	}
	else if (CurrentState == 3) // PVP_STATE_WARN_END
	{
		if (PvpText)
		{
			PvpText->SetText(FText::FromString(TEXT("시공의 균열이 곧 닫힙니다...")));
			PvpText->SetVisibility(ESlateVisibility::Visible);
		}
		if (PvpCountDown)
		{
			if (CurrentRemainingSeconds > 0)
			{
				PvpCountDown->SetText(FText::AsNumber(CurrentRemainingSeconds));
				PvpCountDown->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				PvpCountDown->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		if (PvpTimer)
		{
			PvpTimer->SetText(FText::FromString(TimerStr));
		}
	}
	else
	{
		// 비활성(0) 또는 활성(2) 상태인 경우 5초 경고 안내 및 카운트다운을 숨김
		if (PvpText)
		{
			PvpText->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (PvpCountDown)
		{
			PvpCountDown->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
