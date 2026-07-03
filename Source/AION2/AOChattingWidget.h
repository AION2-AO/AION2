// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AOChattingWidget.generated.h"

class UScrollBox;
class UEditableText;
class UWidgetAnimation;

/**
 * UAOChatLineWidget
 * 
 * 개별 채팅 한 줄의 UI 데이터를 설정하기 위한 C++ 베이스 클래스입니다.
 * 블루프린트에서 이를 상속받아 SetChatData 이벤트를 구현합니다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class AION2_API UAOChatLineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이름과 메세지를 받아 UI 컴포넌트(텍스트 블록 등)에 셋팅하는 블루프린트 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "AION2|Chatting")
	void SetChatData(const FString& SenderName, const FString& MessageText);
};

/**
 * UAOChattingWidget
 * 
 * 블루프린트 위젯(WBP_Chatting)의 부모 클래스로 지정하여 
 * 채팅 UI의 핵심 로직(스크롤 제어, 자동 페이드 아웃, 서버 전송 인터페이스)을 처리합니다.
 */
UCLASS()
class AION2_API UAOChattingWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	// 채팅 메시지 추가 (이름: 내용 형식)
	UFUNCTION(BlueprintCallable, Category = "AION2|Chatting")
	void AddChatMessage(const FString& SenderName, const FString& Message);

	// 입력창 활성화 / 비활성화
	UFUNCTION(BlueprintCallable, Category = "AION2|Chatting")
	void ActivateChatInput();

	UFUNCTION(BlueprintCallable, Category = "AION2|Chatting")
	void DeactivateChatInput();

	// 한국어(한글) 포함 여부 판단 유틸리티 (폰트 분기용)
	UFUNCTION(BlueprintPure, Category = "AION2|Chatting|Utils")
	static bool ContainsKorean(const FString& InString);

private:
	UFUNCTION(BlueprintCallable, Category = "AION2|Chatting")
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// 텍스트 입력 중 처리 (페이드아웃 타이머 갱신용)
	UFUNCTION()
	void OnChatTextChanged(const FText& Text);

	// 페이드아웃 타이머 관리
	void StartFadeTimer();
	void ResetChatVisibility();

	UFUNCTION()
	void PlayFadeOutAnimation();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ChatInputText;

	// 개별 채팅 한 줄을 나타내는 Widget Class
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AION2|Chatting")
	TSubclassOf<UAOChatLineWidget> ChatLineWidgetClass;

	// 10초 뒤 서서히 사라지게 만드는 Widget Animation
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeOutAnim;

private:
	FTimerHandle FadeTimerHandle;
	bool bIsInputActive = false;
};


