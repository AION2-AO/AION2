// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "AOItemDataBase.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 ItemId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	// 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UTexture2D* ItemIcon;

	// 최대 스택 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxStack = 99;

	// 회복량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 HealAcount = 50;
};

USTRUCT(BlueprintType)
struct FAOSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 ItemInstancedId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 ItemTemplateId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;

	bool IsValid() const { return ItemTemplateId > 0 && Count > 0; }
};

UCLASS(BlueprintType)
class AION2_API UAOItemDataBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 아이템 id를 key로 함
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TMap<int32, FItemData> ItemMap;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool FindItemTemplate(int32 ItemId, FItemData& OutItemData) const;
};
