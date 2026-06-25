// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/AOGameInstance.h"

#define SEND_PACKET(pkt, pktId)\
UAOGameInstance* GameInstance = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());\
if(!GameInstance)return;\
GameInstance->SendPacket(pkt, pktId);
