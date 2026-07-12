#include "Player/AOPlayerState.h"
#include "Data/DA_AbilitySet.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

AAOPlayerState::AAOPlayerState()
{
    bReplicates = true;
    SetNetUpdateFrequency(100.f);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributeSet = CreateDefaultSubobject<UAOAttributeSet>(TEXT("AttributeSet"));
}

void AAOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAOPlayerState, MyId);
    DOREPLIFETIME(AAOPlayerState, MyClassType);
    DOREPLIFETIME(AAOPlayerState, MyName);
    //DOREPLIFETIME(AAOPlayerState, MyItem);
}

UAbilitySystemComponent* AAOPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

UAOAttributeSet* AAOPlayerState::GetAttributeSet() const
{
    return AttributeSet;
}

void AAOPlayerState::GiveCommonAbilities()
{
    if (bCommonAbilitiesGiven)
    {
        return;
    }

    if (!ASC)
    {
        return;
    }

    if (!CommonAbilitySet)
    {
        return;
    }

    CommonAbilitySet->GiveToASC(ASC, CommonAbilityHandles);
    bCommonAbilitiesGiven = true;
}

void AAOPlayerState::SetMyId(uint64 PlayerId)
{
    MyId = PlayerId;
}

void AAOPlayerState::SetMyClass(EDaevaClassType InClassType)
{
    MyClassType = InClassType;
}

void AAOPlayerState::SetMyName(FString InName)
{
    MyName = InName;
}

void AAOPlayerState::SetMyHealth(float InHealth)
{
}


void AAOPlayerState::SetMyItem(FString InItem)
{
    //MyItem = InItem;
}


void AAOPlayerState::SetPlayerInfo(uint64 InPlayerId, const FString& InPlayerName, uint8 InClassType,float InHP/*, FString InItem*/)
{
    if (!HasAuthority())
    {
        return;
    }

    MyId = InPlayerId;
    MyName = InPlayerName;
    MyClassType = (EDaevaClassType)InClassType;
    InitialHP = InHP;


    UE_LOG(LogTemp, Warning,TEXT("[Dungeon] PlayerInfo Set | Id: %llu | Name: %s | ClassType: %d | HP: %.1f"), MyId,*MyName, static_cast<int32>(MyClassType), InitialHP);
}
