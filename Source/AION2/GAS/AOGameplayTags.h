#pragma once

#include "GameplayTagContainer.h"

#define TEAM_DAEVA FGameplayTag::RequestGameplayTag(TEXT("Team.Daeva"))
#define TEAM_MONSTER FGameplayTag::RequestGameplayTag(TEXT("Team.Monster"))

#define FACTION_ELYOS FGameplayTag::RequestGameplayTag(TEXT("Faction.Elyos"))
#define FACTION_ASMODIAN FGameplayTag::RequestGameplayTag(TEXT("Faction.Asmodian"))

#define STATE_COMBAT FGameplayTag::RequestGameplayTag(TEXT("State.Combat"))
#define STATE_DASHING FGameplayTag::RequestGameplayTag(TEXT("State.Dashing"))
#define STATE_JUMPING FGameplayTag::RequestGameplayTag(TEXT("State.Jumping"))
#define STATE_GLIDING FGameplayTag::RequestGameplayTag(TEXT("State.Gliding"))

#define COMBO_AVAILABLE_LB2 FGameplayTag::RequestGameplayTag(TEXT("Combo.Available.LB2"))
#define COMBO_AVAILABLE_LB3 FGameplayTag::RequestGameplayTag(TEXT("Combo.Available.LB3"))

#define COMBO_AVAILABLE_RB2 FGameplayTag::RequestGameplayTag(TEXT("Combo.Available.RB2"))
#define COMBO_AVAILABLE_RB3 FGameplayTag::RequestGameplayTag(TEXT("Combo.Available.RB3"))

#define EVENT_CHECKATTACKHIT FGameplayTag::RequestGameplayTag(TEXT("Event.CheckAttackHit"))

// Monster Game Tag

// Action 
#define ABILITY_MONSTER_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.ChargeAttack"))
#define ABILITY_MONSTER_TELEPORT FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.Teleport"))

// Cooldown 
#define COOLDOWN_MONSTER_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.ChargeAttack"))
#define COOLDOWN_MONSTER_TELEPORT FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.Teleport"))
