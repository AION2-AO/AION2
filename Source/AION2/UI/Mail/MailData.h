#pragma once

#include "CoreMinimal.h"
#include "MailData.generated.h"

USTRUCT(BlueprintType)
struct FMailData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int64 MailUID = 0;

    UPROPERTY(BlueprintReadOnly)
    FString SenderName;

    UPROPERTY(BlueprintReadOnly)
    FString Title;

    UPROPERTY(BlueprintReadOnly)
    FString Content;

    UPROPERTY(BlueprintReadOnly)
    FString ExpiredDate;

    UPROPERTY(BlueprintReadOnly)
    int32 Gold = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ItemId = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ItemCount = 0;

    UPROPERTY(BlueprintReadOnly)
    bool bIsReceived = false;

    UPROPERTY(BlueprintReadOnly)
    bool bIsRead = false;

    FMailData() = default;
    
    FMailData(int64 InMailId, FString InSenderName, FString InTitle, FString InContent, FString InExpiredDate, int32 InGold, int32 InItemId, int32 InItemCount, bool InIsReceived, bool InIsRead)
        : MailUID(InMailId)
        , SenderName(InSenderName)
        , Title(InTitle)
        , Content(InContent)
        , ExpiredDate(InExpiredDate)
        , Gold(InGold)
        , ItemId(InItemId)
        , ItemCount(InItemCount)
        , bIsReceived(InIsReceived)
        , bIsRead(InIsRead)
    {}
};
