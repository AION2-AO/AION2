// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Utils/AOSoundRow.h"
#include "AOSoundManager.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAOSoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UAOSoundManager();
	virtual ~UAOSoundManager()= default;

	UFUNCTION(BlueprintPure, Category = "FZFAudio", meta = (WorldContext = "WorldContextObject"))
	static UAOSoundManager* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// BGM 재생 함수 (데이터 테이블의 Row Name을 받아 재생)
	UFUNCTION(BlueprintCallable, Category = "FZFAudio")
	void PlayBGM(FName RowName);

	// BGM 정지 함수
	UFUNCTION(BlueprintCallable, Category = "FZFAudio")
	void StopBGM(float FadeOutTime = 1.0f);

	// SFX 재생 함수 (2D)
	UFUNCTION(BlueprintCallable, Category = "FZFAudio")
	void PlaySFX(FName RowName);

	// SFX 재생 함수 (3D - 특정 위치)
	UFUNCTION(BlueprintCallable, Category = "FZFAudio")
	void PlaySFXAtLocation(FName RowName, FVector Location);
	
protected:
	// 로드된 데이터 테이블을 보관할 포인터
	UPROPERTY()
	UDataTable* SoundDataTable;

	// 현재 재생 중인 오디오 컴포넌트 추적용 (BGM용)
	UPROPERTY(Transient)
	UAudioComponent* CurrentAudioComponent;
	
};
