// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DungeonGameState.generated.h"


UENUM(BlueprintType)
enum class EDungeonMusic : uint8 { None, Dungeon, Boss, Talythra };

/**
 * 
 */
UCLASS()
class AION2_API ADungeonGameState : public AGameState
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Music")
    TObjectPtr<USoundBase> DungeonBGM;

    UPROPERTY(EditDefaultsOnly, Category = "Music")
    TObjectPtr<USoundBase> BossBGM;

    UPROPERTY(EditDefaultsOnly, Category = "Music")
    TObjectPtr<USoundBase> TalythraBossBGM;

    UPROPERTY(ReplicatedUsing = OnRep_Music)
    EDungeonMusic Music = EDungeonMusic::None;

    void SetMusic(EDungeonMusic NewMusic);   // 서버에서만 호출

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
    virtual void BeginPlay() override;


    UFUNCTION()
    void OnRep_Music();

    UPROPERTY()
    TObjectPtr<UAudioComponent> CurrentBGM;
};
