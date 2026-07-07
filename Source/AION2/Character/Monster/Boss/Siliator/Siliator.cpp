#include "Character/Monster/Boss/Siliator/Siliator.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "UI/AOWidgetComponentBase.h"

#include "AI/AIMonsterControllerBase.h"
#include "GAS/AOGameplayTags.h"



ASiliator::ASiliator(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	DungeonBossIndex = 1;
}

void ASiliator::BeginPlay()
{
	Super::BeginPlay();

	InitAttributeSet();
}

void ASiliator::InitAttributeSet()
{
	AttributeSet->InitHealth(1000.f);
	AttributeSet->InitMaxHealth(1000.f);
	
	AttributeSet->InitStamina(100.f);
	AttributeSet->InitMaxStamina(100.f);
}


void ASiliator::EndGroggy()
{
	if (!HasAuthority() || !bIsGroggy || bIsDead)
	{
		return;
	}

	bIsGroggy = false;

	if (AttributeSet)
	{
		AttributeSet->SetGroggy(AttributeSet->GetMaxGroggy());
	}



	AAIMonsterControllerBase* pMonsterController = Cast<AAIMonsterControllerBase>(GetController());
	if (pMonsterController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("pMonsterController nullptr"));
	}

	// 만약 PreCombat 페이즈를 안쓰신다면 EndGroggy를 virtual 함수로 선언하신 뒤 
	// Set_Phase를 다른걸로 사용하시면 될 거 같습니다.
	pMonsterController->Set_Phase(PHASE_MONSTER_COMBAT);
}