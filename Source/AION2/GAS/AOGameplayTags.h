#pragma once

#include "GameplayTagContainer.h"

#define STATE_COMBAT FGameplayTag::RequestGameplayTag(TEXT("State.Combat"))
#define STATE_DASHING FGameplayTag::RequestGameplayTag(TEXT("State.Dashing"))
#define STATE_JUMPING FGameplayTag::RequestGameplayTag(TEXT("State.Jumping"))
#define STATE_GLIDING FGameplayTag::RequestGameplayTag(TEXT("State.Gliding"))

#define COMBO_AVAILABLE_LB2 FGameplayTag::RequestGameplayTag(TEXT("Combo.Available.LB2"))
#define COMBO_AVAILABLE_LB3 FGameplayTag::RequestGameplayTag(TEXT("Combo.Available.LB3"))



// Monster Game Tag

// Action 
#define ABILITY_MONSTER_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.ChargeAttack"))

// Cooldown 
#define COOLDOWN_MONSTER_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.ChargeAttack"))
