// Fill out your copyright notice in the Description page of Project Settings.

#include "AOChattingWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Animation/WidgetAnimation.h"
#include "Player/AOPlayerController.h"
#include "Game/AOGameInstance.h"

void UAOChattingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChatInputText)
	{
		ChatInputText->OnTextCommitted.AddDynamic(this, &UAOChattingWidget::OnChatTextCommitted);
		ChatInputText->OnTextChanged.AddDynamic(this, &UAOChattingWidget::OnChatTextChanged);
	}

	// 시작 시 입력 창 비활성화 상태
	DeactivateChatInput();

	// 자동 페이드아웃 타이머 작동 시작
	StartFadeTimer();
}

void UAOChattingWidget::AddChatMessage(const FString& SenderName, const FString& Message)
{
	if (!ChatScrollBox || !ChatLineWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	// 1. 개별 채팅 한 줄 위젯 동적 생성
	UAOChatLineWidget* LineWidget = CreateWidget<UAOChatLineWidget>(PC, ChatLineWidgetClass);
	if (LineWidget)
	{
		// 2. 데이터 전달 (블루프린트 이벤트 호출)
		LineWidget->SetChatData(SenderName, Message);

		// 3. 스크롤 박스에 추가하고 맨 밑으로 강제 스크롤
		ChatScrollBox->AddChild(LineWidget);
		ChatScrollBox->ScrollToEnd();
	}

	// 4. 새로운 메시지가 들어왔으므로 투명도를 100%로 재설정하고 타이머를 갱신
	ResetChatVisibility();
}

void UAOChattingWidget::ActivateChatInput()
{
	bIsInputActive = true;
	ResetChatVisibility();

	if (ChatInputText)
	{
		ChatInputText->SetVisibility(ESlateVisibility::Visible);
		ChatInputText->SetUserFocus(GetOwningPlayer());

		// 마우스 커서 없이 키보드 입력만 UI로 포커스 하도록 설정
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ChatInputText->GetCachedWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false; // 마우스 커서는 보이지 않게 처리
		}
	}
}

void UAOChattingWidget::DeactivateChatInput()
{
	bIsInputActive = false;

	if (ChatInputText)
	{
		ChatInputText->SetText(FText::GetEmpty());
		ChatInputText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 다시 게임 전용 입력 모드로 전환 (마우스 커서 숨김 유지)
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	// 입력 상태가 끝났으므로 10초 뒤에 페이드 아웃 되도록 타이머 다시 시작
	StartFadeTimer();
}

bool UAOChattingWidget::ContainsKorean(const FString& InString)
{
	if (InString.IsEmpty()) return false;

	for (int32 i = 0; i < InString.Len(); ++i)
	{
		TCHAR Char = InString[i];
		// 유니코드 범위: 한글 음절(AC00~D7A3), 자모(1100~11FF), 호환자모(3130~318F)
		if ((Char >= 0xAC00 && Char <= 0xD7A3) || 
			(Char >= 0x1100 && Char <= 0x11FF) || 
			(Char >= 0x3130 && Char <= 0x318F))
		{
			return true;
		}
	}
	return false;
}

void UAOChattingWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString SendStr = Text.ToString().TrimStartAndEnd();
		if (!SendStr.IsEmpty())
		{
			Protocol::C_ChatPacket chatPacket;
			UAOGameInstance* GameInstance = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());
			chatPacket.set_playerid(GameInstance->GetMyPlayerId());
			chatPacket.set_chat(TCHAR_TO_UTF8(*SendStr));
			GameInstance->SendPacket(chatPacket, PKT_C_CHAT);
		}

		DeactivateChatInput();
	}
	else if (CommitMethod == ETextCommit::OnCleared)
	{
		// 사용자가 Esc 키를 누르거나 포커스를 취소했을 때
		DeactivateChatInput();
	}
}

void UAOChattingWidget::OnChatTextChanged(const FText& Text)
{
	// 텍스트 입력 중에는 화면에 무조건 100% 노출되도록 유지 및 타이머 갱신
	ResetChatVisibility();
}

void UAOChattingWidget::StartFadeTimer()
{
	if (GetWorld() && !bIsInputActive)
	{
		// 10초 뒤 단발성으로 PlayFadeOutAnimation을 실행하도록 타이머 예약
		GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &UAOChattingWidget::PlayFadeOutAnimation, 10.0f, false);
	}
}

void UAOChattingWidget::ResetChatVisibility()
{
	if (GetWorld())
	{
		// 현재 돌고 있는 타이머 취소
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	}

	// 애니메이션이 돌고 있는 경우 중지하고 즉시 원상 복구
	if (FadeOutAnim && IsAnimationPlaying(FadeOutAnim))
	{
		StopAnimation(FadeOutAnim);
	}

	SetRenderOpacity(1.0f);

	// 입력 중이 아니라면 10초 쿨타임을 위해 새로 타이머 기동
	if (!bIsInputActive)
	{
		StartFadeTimer();
	}
}

void UAOChattingWidget::PlayFadeOutAnimation()
{
	if (FadeOutAnim)
	{
		PlayAnimation(FadeOutAnim);
	}
	else
	{
		// 애니메이션을 연동 안해두었을 경우 안전장치로 바로 투명 처리
		SetRenderOpacity(0.0f);
	}
}
