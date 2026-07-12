// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/AOItemDataBase.h"
#include "AOQuickSlotComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AION2_API UAOQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAOQuickSlotComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 슬롯 초기화 및 업데이트 (서버 패킷 수신 시 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void InitializeQuickSlot(int32 SlotIndex, int32 ItemTemplateId, int32 ItemInstancedId, int32 Count);

	// 슬롯에서 런타임 데이터 및 기획 템플릿 데이터 조회
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool GetItemDataFromSlot(int32 SlotIndex, FAOSlotData& OutSlotData, FItemData& OutTemplateData);

	// UI 갱신 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
	FOnQuickSlotChanged OnQuickSlotChanged;

	// 퀵슬롯 최대 크기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	int32 MaxQuickSlots = 2;

protected:
	// 아이템 데이터베이스 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QuickSlot")
	TSoftObjectPtr<UAOItemDataBase> ItemDataBaseAsset;

public:
	bool FindItemTemplateData(int32 ItemTemplateId, FItemData& OutTemplateData);

private:
	// 퀵슬롯 슬롯 배열 (런타임 데이터)
	UPROPERTY()
	TArray<FAOSlotData> QuickSlots;
	
};
