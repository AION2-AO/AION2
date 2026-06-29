// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AODungeonEntranceWidget.generated.h"

/**
 *
 */
UCLASS()
class AION2_API UAODungeonEntranceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetLeaderClass(uint8 InLeaderClass);
	void SetLeaderName(FString InLeaderName);

	void SetMember1Class(uint8 InLeaderClass);
	void SetMember1Name(FString InLeaderName);

	void SetReady();

	void SetImage(class UImage* TargetImage, uint8 ClassType);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* EnterButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CreateButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ReadyButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	class UImage* LeaderClass;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LeaderName;

	UPROPERTY(meta = (BindWidget))
	class UImage* Member1Class;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Member1Name;

	UPROPERTY(meta = (BindWidget))
	class UImage* Member1Ready;

	UPROPERTY(meta = (BindWidget))
	class UImage* Member2Class;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Member2Name;

	UPROPERTY(EditAnywhere, Category = "Class")
	class UTexture2D* AssassinImage;


private:
	UFUNCTION()
	void OnEnterButtonClicked();

	UFUNCTION()
	void OnCreateButtonClicked();

	UFUNCTION()
	void OnStartButtonClicked();

};
