#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/DungeonRoomTypes.h"

#include "DA_DungeonRoom.generated.h"


UCLASS()
class AION2_API UDA_DungeonRoom : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DungeonRoom")
	ERaidBoss RaidBoss = ERaidBoss::Arkanis;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DungeonRoom")
	int32 MaxPlayers = 4;
};