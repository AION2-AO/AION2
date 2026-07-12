// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOLoginUserWidget.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Network/PacketHeader.h"
#include "AION2.h"
#include "AOLoginUserWidget.h"

void UAOLoginUserWidget::SendNickname()
{
	FString Nickname = NicknameText ? NicknameText->GetText().ToString() : TEXT("");

	if (Nickname.IsEmpty())
	{
		ShowErrorMsg(FText::FromString(TEXT("닉네임을 적어 주세요.")));
	}

	Protocol::C_SetNicknamePacket SetNickPacket;
	SetNickPacket.set_nickname(TCHAR_TO_UTF8(*Nickname));
	SEND_PACKET(SetNickPacket, PKT_C_SET_NICNNAME);
}

void UAOLoginUserWidget::ReceiveNicknameResult(int8 ErrorCode)
{
	if (ErrorCode == -1)
	{
		ShowErrorMsg(FText::FromString(TEXT("중복된 닉네임입니다.")));
	}
}

void UAOLoginUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_NicknameSet)
	{
		Btn_NicknameSet->OnClicked.AddUniqueDynamic(this, &UAOLoginUserWidget::SendNickname);
	}
}
