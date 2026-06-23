#include "Character/Daeva/Assassin.h"

AAssassin::AAssassin(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetWeaponMesh()->SetupAttachment(GetMesh(), TEXT("DaggerWeapon"));
	GetSubWeaponMesh()->SetupAttachment(GetMesh(), TEXT("DaggerWeapon_Sub"));
}
