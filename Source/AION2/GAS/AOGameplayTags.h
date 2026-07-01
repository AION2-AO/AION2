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

// Event 
#define EVENT_CHECKATTACKHIT FGameplayTag::RequestGameplayTag(TEXT("Event.CheckAttackHit"))
#define EVENT_CHECKATTACKHIT_BEGIN FGameplayTag::RequestGameplayTag(TEXT("Event.CheckAttackHit.Begin"))
#define EVENT_CHECKATTACKHIT_END FGameplayTag::RequestGameplayTag(TEXT("Event.CheckAttackHit.End"))



// GameplayCue
#define CUE_GHOSTTRAIL FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.GhostTrail"))

// Monster Game Tag

// Action 
#define ABILITY_MONSTER_TH_CHARGEATTACK   FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.ChargeAttack"))
#define ABILITY_MONSTER_TH_TELEPORTATTACK FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.TeleportAttack"))
#define ABILITY_MONSTER_TH_RANGEDATTACK   FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.RangedAttack"))
#define ABILITY_MONSTER_TH_MELEEATTACK    FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.MELEEAttack"))


// Cooldown 
#define COOLDOWN_MONSTER_TH_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.TH.ChargeAttack"))
#define COOLDOWN_MONSTER_TH_TELEPORTATTACK FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.TH.TeleportAttack"))



// Phase 
#define PHASE_MONSTER_OUTOFCOMBAT FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.OutofCombat"))
#define PHASE_MONSTER_PRECOMBAT FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.PreCombat"))
#define PHASE_MONSTER_COMBAT FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.Combat"))
#define PHASE_MONSTER_DEAD FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.Dead"))


// State
#define STATE_MONSTER_TH_IDLE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.Idle"))
#define STATE_MONSTER_TH_CHASE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.Chase"))
#define STATE_MONSTER_TH_RANGEDSHOT FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.RangedShot"))
#define STATE_MONSTER_TH_MELEEATTACK FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.MeleeAttack"))
#define STATE_MONSTER_TH_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.ChargeAttack"))
#define STATE_MONSTER_TH_TELEPORTATTACK FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.TeleportAttack"))
#define STATE_MONSTER_TH_DEAD FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.Dead"))