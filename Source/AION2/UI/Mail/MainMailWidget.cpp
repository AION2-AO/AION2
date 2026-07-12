// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Mail/MainMailWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "MailListRowWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Types/SlateEnums.h"
#include "Components/EditableText.h"
#include "Components/MultiLineEditableText.h"
#include "GameFramework/PlayerState.h"
#include "Character/Daeva/Daeva.h"
#include "Network/PacketHeader.h"
#include "Manager/AOUIManager.h"
#include "UI/AOQuickSlotComponent.h"
#include  "AION2.h"


void UMainMailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MailSwitcher)
	{
		MailSwitcher->SetActiveWidgetIndex(0);
	}
	if (ScrollBox)
	{
		ScrollBox->ClearChildren();
	}

	if (DetailGold)
	{
		DetailGold->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnGoldClicked);
	}
	if (DetailItem)
	{
		DetailItem->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnItemClicked);
	}
	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnBackClicked);
		Btn_Back->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Tab_List)
	{
		Tab_List->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnTabListClicked);
	}
	if (Tab_Write)
	{
		Tab_Write->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnTabWriteClicked);
	}
	if (Btn_Send)
	{
		Btn_Send->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnSendClicked);
	}
	if (Btn_Reset)
	{
		Btn_Reset->OnClicked.AddUniqueDynamic(this, &UMainMailWidget::OnResetClicked);
	}
}

bool UMainMailWidget::HandleEscapeKey()
{
	if (!MailSwitcher) return false;

	int32 ActiveIndex = MailSwitcher->GetActiveWidgetIndex();

	if (ActiveIndex == 1)
	{
		MailSwitcher->SetActiveWidgetIndex(0);
		if (Btn_Back)
		{
			Btn_Back->SetVisibility(ESlateVisibility::Collapsed);
		}
		return true;
	}

	UAOUIManager* UIManager = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UAOUIManager>() : nullptr;
	if (UIManager)
	{
		UIManager->HideWidget(this);
	}
	else
	{
		RemoveFromParent();
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	return true;
}

void UMainMailWidget::ShowMailList()
{
	if (MailSwitcher)
	{
		MailSwitcher->SetActiveWidgetIndex(0);
	}
	if (Btn_Back)
	{
		Btn_Back->SetVisibility(ESlateVisibility::Collapsed);
	}
	UpdateMailList(MailListCache);
}

void UMainMailWidget::UpdateMailList(const TArray<FMailData>& InMailList)
{
	MailListCache = InMailList;
	if (!ScrollBox || !RowWidgetClass) return;

	ScrollBox->ClearChildren();

	if (Btn_Back)
	{
		Btn_Back->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (MailListCache.Num() == 0)
	{
		UTextBlock* EmptyTextBlock = NewObject<UTextBlock>(this);
		if (EmptyTextBlock)
		{
			EmptyTextBlock->SetText(FText::FromString(TEXT("메일이 없습니다")));

			FSlateFontInfo FontInfo = EmptyTextBlock->GetFont();
			FontInfo.Size = 20;
			EmptyTextBlock->SetFont(FontInfo);
			EmptyTextBlock->SetJustification(ETextJustify::Center);

			UPanelSlot* PanelSlot = ScrollBox->AddChild(EmptyTextBlock);
			UScrollBoxSlot* ScrollBoxSlot = Cast<UScrollBoxSlot>(PanelSlot);
			if (ScrollBoxSlot)
			{
				ScrollBoxSlot->SetHorizontalAlignment(HAlign_Center);
				ScrollBoxSlot->SetVerticalAlignment(VAlign_Center);
				ScrollBoxSlot->SetPadding(FMargin(0.f, 20.f, 0.f, 0.f));
			}
		}
	}
	else
	{
		for (const FMailData& Data : MailListCache)
		{
			UMailListRowWidget* NewRow = CreateWidget<UMailListRowWidget>(this, RowWidgetClass);
			if (NewRow)
			{
				NewRow->InitRowData(Data, this);
				ScrollBox->AddChild(NewRow);
			}
		}
	}
}

void UMainMailWidget::UpdateMailContent(const FMailData& InDetailData)
{
	CurrentMailData = InDetailData;

	// 캐시 데이터 업데이트
	for (FMailData& Data : MailListCache)
	{
		if (Data.MailUID == InDetailData.MailUID)
		{
			Data = InDetailData;
			break;
		}
	}

	// 상세 UI 컴포넌트 갱신
	if (DetailTitle)
	{
		DetailTitle->SetText(FText::FromString(InDetailData.Title));
	}

	if (DetailContent)
	{
		DetailContent->SetText(FText::FromString(InDetailData.Content));
	}

	if (DetailSender)
	{
		DetailSender->SetText(FText::FromString(InDetailData.SenderName));
	}

	// 골드 표시 제어
	if (DetailGold || GoldSection)
	{
		if (InDetailData.Gold > 0)
		{
			if (DetailGoldText)
			{
				DetailGoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), InDetailData.Gold)));
			}

			if (GoldSection)
			{
				GoldSection->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			else if (DetailGold)
			{
				DetailGold->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			if (GoldSection)
			{
				GoldSection->SetVisibility(ESlateVisibility::Collapsed);
			}
			else if (DetailGold)
			{
				DetailGold->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// 아이템 표시 제어
	if (DetailItem || ItemSection)
	{
		if (InDetailData.ItemId > 0 && InDetailData.ItemCount > 0)
		{
			FString DispItemName = FString::Printf(TEXT("Item: %d"), InDetailData.ItemId);
			UTexture2D* IconTexture = nullptr;

			if (APlayerController* PC = GetOwningPlayer())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					UAOQuickSlotComponent* QuickSlotComp = Pawn->FindComponentByClass<UAOQuickSlotComponent>();
					if (QuickSlotComp)
					{
						FItemData ItemTemplateData;
						if (QuickSlotComp->FindItemTemplateData(InDetailData.ItemId, ItemTemplateData))
						{
							DispItemName = ItemTemplateData.ItemName.ToString();
							IconTexture = ItemTemplateData.ItemIcon;
						}
					}
				}
			}

			if (IconTexture)
			{
				if (DetailItem)
				{
					FButtonStyle Style = DetailItem->GetStyle();
					Style.Normal.SetResourceObject(IconTexture);
					Style.Hovered.SetResourceObject(IconTexture);
					Style.Pressed.SetResourceObject(IconTexture);
					DetailItem->SetStyle(Style);
				}
			}

			if (DetailItemCount)
			{
				DetailItemCount->SetText(FText::FromString(FString::Printf(TEXT("x%d"), InDetailData.ItemCount)));
				DetailItemCount->SetVisibility(ESlateVisibility::Visible);
			}

			if (ItemSection)
			{
				ItemSection->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			else if (DetailItem)
			{
				DetailItem->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			if (DetailItemCount)
			{
				DetailItemCount->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (ItemSection)
			{
				ItemSection->SetVisibility(ESlateVisibility::Collapsed);
			}
			else if (DetailItem)
			{
				DetailItem->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	if (MailSwitcher)
	{
		MailSwitcher->SetActiveWidgetIndex(1);
	}
	if (Btn_Back)
	{
		Btn_Back->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMainMailWidget::RequestDetailContentFromServer(int64 MailUID)
{
	Protocol::C_MailContentPacket ReqContent;
	ReqContent.set_mailid(MailUID);
	SEND_PACKET(ReqContent, PKT_C_MAIL_CONTENT);
}

void UMainMailWidget::ShowErrorMessage(int8 Reason)
{
	if (Reason == 0)
	{
		OnResetClicked();
		OnTabListClicked();
		return;
	}
	if (ErrorMessage)
	{
		FText ErrorMsgText = FText::GetEmpty();

		if (Reason == 1)
		{
			ErrorMsgText = FText::FromString(TEXT("받을 사람이 없습니다"));
		}
		else if (Reason == 2)
		{
			ErrorMsgText = FText::FromString(TEXT("골드가 부족합니다"));
		}
		else if (Reason == 3)
		{
			ErrorMsgText = FText::FromString(TEXT("제목을 입력하세요"));
		}
		else if (Reason == 4)
		{
			ErrorMsgText = FText::FromString(TEXT("받을 사람을 입력하세요"));
		}
		else if (Reason == 5)
		{
			ErrorMsgText = FText::FromString(TEXT("내용을 입력하세요"));
		}

		ErrorMessage->SetText(ErrorMsgText);
		ErrorMessage->SetVisibility(ESlateVisibility::Visible);
	GetWorld()->GetTimerManager().ClearTimer(ErrorMessageTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(ErrorMessageTimerHandle, this, &UMainMailWidget::ClearErrorMessage, 3.0f, false);

	}
}

void UMainMailWidget::ClearErrorMessage()
{
	if (ErrorMessage)
	{
		ErrorMessage->SetText(FText::GetEmpty());
		ErrorMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMainMailWidget::OnGoldClicked()
{
	OnClaimGold(CurrentMailData.Gold);
}

void UMainMailWidget::OnItemClicked()
{
	OnClaimItem(CurrentMailData.ItemId, CurrentMailData.ItemCount);
}

void UMainMailWidget::OnBackClicked()
{
	ShowMailList();
}

FReply UMainMailWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::M)
	{
		UAOUIManager* UIManager = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UAOUIManager>() : nullptr;
		if (UIManager)
		{
			UIManager->HideWidget(this);
		}
		else
		{
			RemoveFromParent();
		}

		if (APlayerController* PC = GetOwningPlayer())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UMainMailWidget::OnTabListClicked()
{
	if (MailSwitcher)
	{
		MailSwitcher->SetActiveWidgetIndex(0);
	}
	if (Btn_Back)
	{
		Btn_Back->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADaeva* Daeva = Cast<ADaeva>(PC->GetPawn()))
		{
			Protocol::C_MailListPacket ReqList;
			ReqList.set_playerid(Daeva->GetMy());
			SEND_PACKET(ReqList, PKT_C_MAIL_LIST);
		}
	}
}

void UMainMailWidget::OnTabWriteClicked()
{
	if (MailSwitcher)
	{
		MailSwitcher->SetActiveWidgetIndex(2);
	}
	if (Btn_Back)
	{
		Btn_Back->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMainMailWidget::OnResetClicked()
{
	if (Write_Recipient)
	{
		Write_Recipient->SetText(FText::GetEmpty());
	}
	if (Write_Title)
	{
		Write_Title->SetText(FText::GetEmpty());
	}
	if (Write_Content)
	{
		Write_Content->SetText(FText::GetEmpty());
	}
	if (Write_Gold)
	{
		Write_Gold->SetText(FText::GetEmpty());
	}
}

void UMainMailWidget::OnSendClicked()
{
	FString Recipient = Write_Recipient ? Write_Recipient->GetText().ToString() : TEXT("");
	FString TitleText = Write_Title ? Write_Title->GetText().ToString() : TEXT("");
	FString ContentText = Write_Content ? Write_Content->GetText().ToString() : TEXT("");

	int32 GoldVal = 0;
	if (Write_Gold)
	{
		GoldVal = FCString::Atoi(*Write_Gold->GetText().ToString());
	}

	if (Recipient.IsEmpty())
	{
		ShowErrorMessage(4); 
		return;
	}

	if (TitleText.IsEmpty())
	{
		ShowErrorMessage(3); 
		return;
	}
	if (ContentText.IsEmpty())
	{
		ShowErrorMessage(5); 
		return;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADaeva* Daeva = Cast<ADaeva>(PC->GetPawn()))
		{
			Protocol::C_SendMailPacket SendPacket;
			SendPacket.set_senderid(Daeva->GetMy());

			FString SenderNameStr = Daeva->GetName();
			if (Daeva->GetPlayerState())
			{
				SenderNameStr = Daeva->GetPlayerState()->GetPlayerName();
			}
			SendPacket.set_sendername(TCHAR_TO_UTF8(*SenderNameStr));

			SendPacket.set_receivername(TCHAR_TO_UTF8(*Recipient));
			SendPacket.set_title(TCHAR_TO_UTF8(*TitleText));
			SendPacket.set_content(TCHAR_TO_UTF8(*ContentText));
			SendPacket.set_gold(GoldVal);
			SendPacket.set_itemid(0);
			SendPacket.set_itemcount(0);

			SEND_PACKET(SendPacket, PKT_C_MAIL_SEND);
		}
	}

	//OnResetClicked();
	//OnTabListClicked();
}
