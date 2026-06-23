// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOLoginUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAOLoginUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowErrorMsg(const FText& Error);
	
	void HandleRegisterError()
	{
		ShowErrorMsg(FText::FromString(TEXT("중복된 아이디입니다.")));
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void HandleRegisterResult();

	void HandleLoginResult()
	{
		ShowErrorMsg(FText::FromString(TEXT("아이디나 비밀번호를 확인해 주세요.")));
	}

};
