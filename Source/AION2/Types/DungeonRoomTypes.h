#pragma once

#include "CoreMinimal.h"
#include "Player/AOPlayerState.h"
#include "DungeonRoomTypes.generated.h"

// [방 설정 데이터에 대한 공용 Type 분리].

// 방에 이미 참가한 상태인지 => 방생성/빠른참가 || 나가기/시작 버튼 구분
UENUM(BlueprintType)
enum class EJoinState : uint8
{
	None,
	Joined
};

// [각 Player의 Ready 상태]
UENUM(BlueprintType)
enum class EReadyState : uint8
{
	None,
	Preparing,
	Ready
};

// [이 방의 목표 RaidBoss]: 일단 넣어봤음
UENUM(BlueprintType)
enum class ERaidBoss : uint8
{
	None,
	Arkanis
};

// 방 하나의 상태를 저장하는 Struct.
USTRUCT(BlueprintType)
struct FPlayerReadyState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DungeonRoom")
	EDaevaClassType PlayerClass = EDaevaClassType::None;

	UPROPERTY(BlueprintReadOnly, Category = "DungeonRoom")
	FString PlayerName = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "DungeonRoom")
	EReadyState ReadyState = EReadyState::None;
};