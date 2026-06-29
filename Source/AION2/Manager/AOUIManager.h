// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "AOUIManager.generated.h"

UENUM(BlueprintType)
enum class EUILayer : uint8
{
	Background UMETA(DisplayName = "Background"),
	PopUp UMETA(DisplayName = "PopUp"),
	System UMETA(DisplayName = "System"),
	Default UMETA(DisplayName = "Default"),
};


UCLASS()
class AION2_API UAOUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	UUserWidget* ShowWidget(TSoftClassPtr<UUserWidget> WidgetClass, EUILayer Layer = EUILayer::Default);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void HideWidget(UUserWidget* WidgetInstance);

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	UUserWidget* GetWidgetByClass(TSubclassOf<UUserWidget>WidgetClass) const;

	template<typename T>
	T* GetWidget() const
	{
		return Cast<T>(GetWidgetByClass(T::StaticClass()));
	}

private:
	int GetZOrderForLayer(EUILayer Layer) const;

private:
	UPROPERTY()
	TMap<TSoftClassPtr<UUserWidget>, UUserWidget*> WidgetCache;


};
