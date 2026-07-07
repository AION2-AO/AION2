#pragma once

#include "GameplayTagContainer.h"

#define TEAM_DAEVA FGameplayTag::RequestGameplayTag(TEXT("Team.Daeva"))
#define TEAM_MONSTER FGameplayTag::RequestGameplayTag(TEXT("Team.Monster"))
#define TEAM_PERCEPTION_DAEVA 1
#define TEAM_PERCEPTION_MONSTER 2 


#define FACTION_ELYOS FGameplayTag::RequestGameplayTag(TEXT("Faction.Elyos"))
#define FACTION_ASMODIAN FGameplayTag::RequestGameplayTag(TEXT("Faction.Asmodian"))

#define STATE_ATTACKING FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"))
#define STATE_COMBAT FGameplayTag::RequestGameplayTag(TEXT("State.Combat"))
#define STATE_DASHING FGameplayTag::RequestGameplayTag(TEXT("State.Dashing"))
#define STATE_DEAD FGameplayTag::RequestGameplayTag(TEXT("State.Dead"))
#define STATE_JUMPING FGameplayTag::RequestGameplayTag(TEXT("State.Jumping"))
#define STATE_GLIDING FGameplayTag::RequestGameplayTag(TEXT("State.Gliding"))
#define STATE_SPRINT FGameplayTag::RequestGameplayTag(TEXT("State.Sprint"))

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
#define ABILITY_MONSTER_LT_BASEATTACK1    FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.LT.BaseAttack1"))
#define ABILITY_MONSTER_LT_DEAD           FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.LT.Dead"))
#define ABILITY_MONSTER_LT_PT1            FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.LT.PT1"))
#define ABILITY_MONSTER_TH_CHARGEATTACK   FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.ChargeAttack"))
#define ABILITY_MONSTER_TH_TELEPORTATTACK FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.TeleportAttack"))
#define ABILITY_MONSTER_TH_RANGEDATTACK   FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.RangedAttack"))
#define ABILITY_MONSTER_TH_MELEEATTACK    FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.TH.MELEEAttack"))

#define ABILITY_MONSTER_WE_CANNON FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.WE.Cannon"))


// Cooldown 
#define COOLDOWN_MONSTER_TH_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.TH.ChargeAttack"))
#define COOLDOWN_MONSTER_TH_TELEPORTATTACK FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.TH.TeleportAttack"))

#define COOLDOWN_MONSTER_WE_RELOADING FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Monster.WE.Reloading"))



// Phase 
#define PHASE_MONSTER_OUTOFCOMBAT FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.OutofCombat"))
#define PHASE_MONSTER_PRECOMBAT FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.PreCombat"))
#define PHASE_MONSTER_COMBAT FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.Combat"))
#define PHASE_MONSTER_GROGGY FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.Groggy"))
#define PHASE_MONSTER_DEAD FGameplayTag::RequestGameplayTag(TEXT("Phase.Monster.Dead"))


// State - SH
#define STATE_MONSTER_TH_IDLE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.Idle"))
#define STATE_MONSTER_TH_CHASE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.Chase"))
#define STATE_MONSTER_TH_RANGEDSHOT FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.RangedShot"))
#define STATE_MONSTER_TH_MELEEATTACK FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.MeleeAttack"))
#define STATE_MONSTER_TH_CHARGEATTACK FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.ChargeAttack"))
#define STATE_MONSTER_TH_TELEPORTATTACK FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.TeleportAttack"))
#define STATE_MONSTER_TH_DEAD FGameplayTag::RequestGameplayTag(TEXT("State.Monster.TH.Dead"))

#define STATE_MONSTER_WE_IDLE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.WE.Idle"))
#define STATE_MONSTER_WE_CHASE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.WE.Chase"))
#define STATE_MONSTER_WE_RELOADING FGameplayTag::RequestGameplayTag(TEXT("State.Monster.WE.Reloading"))
#define STATE_MONSTER_WE_CANNON FGameplayTag::RequestGameplayTag(TEXT("State.Monster.WE.Cannon"))
#define STATE_MONSTER_WE_TARGETMOVING FGameplayTag::RequestGameplayTag(TEXT("State.Monster.WE.TargetMoving"))
#define STATE_MONSTER_WE_DEAD FGameplayTag::RequestGameplayTag(TEXT("State.Monster.WE.Dead"))

// State - LT
#define STATE_MONSTER_LT_IDLE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.LT.Idle"))
#define STATE_MONSTER_LT_CHASE FGameplayTag::RequestGameplayTag(TEXT("State.Monster.LT.Chase"))
#define STATE_MONSTER_LT_BASEATTACK1 FGameplayTag::RequestGameplayTag(TEXT("State.Monster.LT.BaseAttack1"))
#define STATE_MONSTER_LT_DEAD FGameplayTag::RequestGameplayTag(TEXT("State.Monster.LT.Dead"))

// Montage - LT
#define MONTAGE_LUTALIS_BASEATTACK1 FGameplayTag::RequestGameplayTag(TEXT("Montage.Lutalis.BaseAttack1"))
#define MONTAGE_LUTALIS_PT1 FGameplayTag::RequestGameplayTag(TEXT("Montage.Lutalis.PT1"))



// Gimmick 
// 기믹 실행 트리거 (붙었다가 기믹 끝나면 제거됨)
//GIMMICK_MONSTER_TH_HP65_PENDING
#define GIMMICK_MONSTER_TH_HP70_DONE FGameplayTag::RequestGameplayTag(TEXT("Gimmick.Monster.TH.HP70.Done"))

// 실행 이력 (한 번 발동했으면 영구히 남아 재발동 차단)
//GIMMICK_MONSTER_TH_HP65_DONE
#define GIMMICK_MONSTER_TH_HP70_PENDING FGameplayTag::RequestGameplayTag(TEXT("Gimmick.Monster.TH.HP70.Pending"))


#define GIMMICK_MONSTER_TH_HP35_DONE FGameplayTag::RequestGameplayTag(TEXT("Gimmick.Monster.TH.HP35.Done"))
#define GIMMICK_MONSTER_TH_HP35_PENDING FGameplayTag::RequestGameplayTag(TEXT("Gimmick.Monster.TH.HP35.Pending"))

