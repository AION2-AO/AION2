#include "Character/Daeva/Templar.h"

ATemplar::ATemplar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetWeaponMesh()->SetupAttachment(GetMesh(), TEXT("GswordWeapon"));
	GetSubWeaponMesh()->SetupAttachment(GetMesh(), TEXT("ShieldWeapon"));
}
