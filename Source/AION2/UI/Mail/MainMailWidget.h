// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MailData.h"
#include "MainMailWidget.generated.h"

/**
 * 
 */

class UTextBlock;
class UButton;
class UAOClassSwitcherWidget;
class UMailListRowWidget;
UCLASS()
class AION2_API UMainMailWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MailSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Title ;		
	
	UPROPERTY(EditDefaultsOnly, Category = "MainUI")
	TSubclassOf<class UMailListRowWidget> RowWidgetClass;

	// 상세 정보 표시용 위젯들 (Optional 바인딩으로 컴파일 유연성 확보)
	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* DetailTitle;

	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* DetailContent;

	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* DetailSender;

	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* DetailGold;

	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* DetailItem;

	UPROPERTY(meta = (BindWidget, Optional = true))
	UTextBlock* DetailItemCount;

	UPROPERTY(meta = (BindWidget, Optional = true))
	class UWidget* GoldSection;

	UPROPERTY(meta = (BindWidget, Optional = true))
	class UWidget* ItemSection;
	
private:
	TArray<struct FMailData> MailListCache;
	
protected:
	virtual void NativeConstruct() override;
	
public:
	// UI 매니저 연동 ESC 제어 함수
    virtual bool HandleEscapeKey();
    
    UFUNCTION(BlueprintCallable, Category = "Mail UI")
    void ShowMailList();

    // 네트워크 매니저 패킷 수신 콜백 연동용 함수들
    void UpdateMailList(const TArray<FMailData>& InMailList);
    void UpdateMailContent(const FMailData& InDetailData);
    
    // 하위 Row에서 호출할 서버 요청 전스용 함수
    void RequestDetailContentFromServer(int64 MailUID);	
};
