#include "Character/Daeva/Ranger.h"

ARanger::ARanger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetWeaponMesh()->SetupAttachment(GetMesh(), TEXT("BowWeapon"));
}
