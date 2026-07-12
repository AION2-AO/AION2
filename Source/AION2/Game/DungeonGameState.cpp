// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DungeonGameState.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"      // UGameplayStatics
#include "Sound/SoundBase.h"              // USoundBase

void ADungeonGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(ADungeonGameState, Music, COND_None, REPNOTIFY_Always);
}

void ADungeonGameState::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
        SetMusic(EDungeonMusic::Dungeon);   // 던전 입장 시 기본 BGM
}

void ADungeonGameState::SetMusic(EDungeonMusic NewMusic)
{
    if (!HasAuthority() || Music == NewMusic) return;   // 중복 방지
    Music = NewMusic;
    OnRep_Music();   // 리슨서버/스탠드얼론 대응
}

void ADungeonGameState::OnRep_Music()
{
    if (GetNetMode() == NM_DedicatedServer) return;   // 서버는 소리 안 냄

    if (CurrentBGM)
    {
        CurrentBGM->FadeOut(1.5f, 0.f);
        CurrentBGM = nullptr;
    }

    USoundBase* Next = nullptr;
    switch (Music)
    {
    case EDungeonMusic::Dungeon: 
        Next = DungeonBGM; 
        break;
    case EDungeonMusic::Boss:    
        Next = BossBGM;   
        break;
    case EDungeonMusic::Talythra:    
        Next = TalythraBossBGM;   
        break;
    default: break;
    }
    if (!Next) return;

    CurrentBGM = UGameplayStatics::SpawnSound2D(this, Next);
    if (CurrentBGM)
        CurrentBGM->FadeIn(1.5f, 3.f);
}