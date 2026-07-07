#include "Character/Daeva/Cleric.h"

ACleric::ACleric(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetWeaponMesh()->SetupAttachment(GetMesh(), TEXT("MaceWeapon"));
}
