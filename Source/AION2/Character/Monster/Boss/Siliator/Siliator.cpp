#include "Character/Monster/Boss/Siliator/Siliator.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

void ASiliator::BeginPlay()
{
	Super::BeginPlay();

	InitAttributeSet();
}

void ASiliator::InitAttributeSet()
{
	AttributeSet->InitHealth(100000.f);
	AttributeSet->InitMaxHealth(100000.f);
	
	AttributeSet->InitStamina(100.f);
	AttributeSet->InitMaxStamina(100.f);
}
