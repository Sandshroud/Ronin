/***
 * Demonstrike Core
 */

#include "StdAfx.h"

pSpellAura SpellAuraHandler[SPELL_AURA_TOTAL] = {
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_NONE = 0
    &Aura::SpellAuraBindSight,                                      //SPELL_AURA_BIND_SIGHT = 1
    &Aura::SpellAuraModPossess,                                     //SPELL_AURA_MOD_POSSESS = 2
    &Aura::SpellAuraPeriodicDamage,                                 //SPELL_AURA_PERIODIC_DAMAGE = 3
    &Aura::SpellAuraDummy,                                          //SPELL_AURA_DUMMY = 4
    &Aura::SpellAuraModConfuse,                                     //SPELL_AURA_MOD_CONFUSE = 5
    &Aura::SpellAuraModCharm,                                       //SPELL_AURA_MOD_CHARM = 6
    &Aura::SpellAuraModFear,                                        //SPELL_AURA_MOD_FEAR = 7
    &Aura::SpellAuraPeriodicHeal,                                   //SPELL_AURA_PERIODIC_HEAL = 8
    &Aura::SpellAuraModAttackSpeed,                                 //SPELL_AURA_MOD_ATTACKSPEED = 9
    &Aura::SpellAuraModThreatGenerated,                             //SPELL_AURA_MOD_THREAT = 10
    &Aura::SpellAuraModTaunt,                                       //SPELL_AURA_MOD_TAUNT = 11
    &Aura::SpellAuraModStun,                                        //SPELL_AURA_MOD_STUN = 12
    &Aura::SpellAuraModDamageDone,                                  //SPELL_AURA_MOD_DAMAGE_DONE = 13
    &Aura::SpellAuraModDamageTaken,                                 //SPELL_AURA_MOD_DAMAGE_TAKEN = 14
    &Aura::SpellAuraDamageShield,                                   //SPELL_AURA_DAMAGE_SHIELD = 15
    &Aura::SpellAuraModStealth,                                     //SPELL_AURA_MOD_STEALTH = 16
    &Aura::SpellAuraModDetect,                                      //SPELL_AURA_MOD_DETECT = 17
    &Aura::SpellAuraModInvisibility,                                //SPELL_AURA_MOD_INVISIBILITY = 18
    &Aura::SpellAuraModInvisibilityDetection,                       //SPELL_AURA_MOD_INVISIBILITY_DETECTION = 19
    &Aura::SpellAuraModTotalHealthRegenPct,                         //SPELL_AURA_MOD_TOTAL_HEALTH_REGEN_PCT = 20
    &Aura::SpellAuraModTotalManaRegenPct,                           //SPELL_AURA_MOD_TOTAL_MANA_REGEN_PCT = 21
    &Aura::SpellAuraUtilized,                                       //SPELL_AURA_MOD_RESISTANCE = 22
    &Aura::SpellAuraPeriodicTriggerSpell,                           //SPELL_AURA_PERIODIC_TRIGGER_SPELL = 23
    &Aura::SpellAuraPeriodicEnergize,                               //SPELL_AURA_PERIODIC_ENERGIZE = 24
    &Aura::SpellAuraModPacify,                                      //SPELL_AURA_MOD_PACIFY = 25
    &Aura::SpellAuraModRoot,                                        //SPELL_AURA_MOD_ROOT = 26
    &Aura::SpellAuraModSilence,                                     //SPELL_AURA_MOD_SILENCE = 27
    &Aura::SpellAuraReflectSpells,                                  //SPELL_AURA_REFLECT_SPELLS = 28
    &Aura::SpellAuraModStat,                                        //SPELL_AURA_MOD_STAT = 29
    &Aura::SpellAuraModSkill,                                       //SPELL_AURA_MOD_SKILL = 30
    &Aura::SpellAuraModIncreaseSpeed,                               //SPELL_AURA_MOD_INCREASE_SPEED = 31
    &Aura::SpellAuraModIncreaseMountedSpeed,                        //SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED = 32
    &Aura::SpellAuraModDecreaseSpeed,                               //SPELL_AURA_MOD_DECREASE_SPEED = 33
    &Aura::SpellAuraModIncreaseHealth,                              //SPELL_AURA_MOD_INCREASE_HEALTH = 34
    &Aura::SpellAuraModIncreaseEnergy,                              //SPELL_AURA_MOD_INCREASE_ENERGY = 35
    &Aura::SpellAuraModShapeshift,                                  //SPELL_AURA_MOD_SHAPESHIFT = 36
    &Aura::SpellAuraModEffectImmunity,                              //SPELL_AURA_EFFECT_IMMUNITY = 37
    &Aura::SpellAuraModStateImmunity,                               //SPELL_AURA_STATE_IMMUNITY = 38
    &Aura::SpellAuraModSchoolImmunity,                              //SPELL_AURA_SCHOOL_IMMUNITY = 39
    &Aura::SpellAuraModDmgImmunity,                                 //SPELL_AURA_DAMAGE_IMMUNITY = 40
    &Aura::SpellAuraModDispelImmunity,                              //SPELL_AURA_DISPEL_IMMUNITY = 41
    &Aura::SpellAuraProcTriggerSpell,                               //SPELL_AURA_PROC_TRIGGER_SPELL = 42
    &Aura::SpellAuraProcTriggerDamage,                              //SPELL_AURA_PROC_TRIGGER_DAMAGE = 43
    &Aura::SpellAuraTrackCreatures,                                 //SPELL_AURA_TRACK_CREATURES = 44
    &Aura::SpellAuraTrackResources,                                 //SPELL_AURA_TRACK_RESOURCES = 45
    &Aura::SpellAuraModParrySkill,                                  //SPELL_AURA_MOD_PARRY_SKILL = 46
    &Aura::SpellAuraModParryPerc,                                   //SPELL_AURA_MOD_PARRY_PERCENT = 47
    &Aura::SpellAuraModDodgeSkill,                                  //SPELL_AURA_MOD_DODGE_SKILL = 48
    &Aura::SpellAuraModDodgePerc,                                   //SPELL_AURA_MOD_DODGE_PERCENT = 49
    &Aura::SpellAuraModBlockSkill,                                  //SPELL_AURA_MOD_BLOCK_SKILL = 50
    &Aura::SpellAuraModBlockPerc,                                   //SPELL_AURA_MOD_BLOCK_PERCENT = 51
    &Aura::SpellAuraModCritPerc,                                    //SPELL_AURA_MOD_CRIT_PERCENT = 52
    &Aura::SpellAuraPeriodicLeech,                                  //SPELL_AURA_PERIODIC_LEECH = 53
    &Aura::SpellAuraModHitChance,                                   //SPELL_AURA_MOD_HIT_CHANCE = 54
    &Aura::SpellAuraModSpellHitChance,                              //SPELL_AURA_MOD_SPELL_HIT_CHANCE = 55
    &Aura::SpellAuraTransform,                                      //SPELL_AURA_TRANSFORM = 56
    &Aura::SpellAuraModSpellCritChance,                             //SPELL_AURA_MOD_SPELL_CRIT_CHANCE = 57
    &Aura::SpellAuraIncreaseSwimSpeed,                              //SPELL_AURA_MOD_INCREASE_SWIM_SPEED = 58
    &Aura::SpellAuraModCratureDmgDone,                              //SPELL_AURA_MOD_DAMAGE_DONE_CREATURE = 59
    &Aura::SpellAuraPacifySilence,                                  //SPELL_AURA_MOD_PACIFY_SILENCE = 60
    &Aura::SpellAuraModScale,                                       //SPELL_AURA_MOD_SCALE = 61
    &Aura::SpellAuraPeriodicHealthFunnel,                           //SPELL_AURA_PERIODIC_HEALTH_FUNNEL = 62
    &Aura::SpellAuraIgnore,                                         //SPELL_AURA_PERIODIC_MANA_FUNNEL = 63
    &Aura::SpellAuraPeriodicManaLeech,                              //SPELL_AURA_PERIODIC_MANA_LEECH = 64
    &Aura::SpellAuraModCastingSpeed,                                //SPELL_AURA_MOD_CASTING_SPEED = 65
    &Aura::SpellAuraFeignDeath,                                     //SPELL_AURA_FEIGN_DEATH = 66
    &Aura::SpellAuraModDisarm,                                      //SPELL_AURA_MOD_DISARM = 67
    &Aura::SpellAuraModStalked,                                     //SPELL_AURA_MOD_STALKED = 68
    &Aura::SpellAuraSchoolAbsorb,                                   //SPELL_AURA_SCHOOL_ABSORB = 69
    &Aura::SpellAuraIgnore,                                         //SPELL_AURA_EXTRA_ATTACKS = 70
    &Aura::SpellAuraModSpellCritChanceSchool,                       //SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71
    &Aura::SpellAuraModPowerCost,                                   //SPELL_AURA_MOD_POWER_COST = 72
    &Aura::SpellAuraModPowerCostSchool,                             //SPELL_AURA_MOD_POWER_COST_SCHOOL = 73
    &Aura::SpellAuraReflectSpellsSchool,                            //SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74
    &Aura::SpellAuraModLanguage,                                    //SPELL_AURA_MOD_LANGUAGE = 75
    &Aura::SpellAuraAddFarSight,                                    //SPELL_AURA_FAR_SIGHT = 76
    &Aura::SpellAuraMechanicImmunity,                               //SPELL_AURA_MECHANIC_IMMUNITY = 77
    &Aura::SpellAuraMounted,                                        //SPELL_AURA_MOUNTED = 78
    &Aura::SpellAuraModDamagePercDone,                              //SPELL_AURA_MOD_DAMAGE_PERCENT_DONE = 79
    &Aura::SpellAuraModPercStat,                                    //SPELL_AURA_MOD_PERCENT_STAT = 80
    &Aura::SpellAuraSplitDamage,                                    //SPELL_AURA_SPLIT_DAMAGE = 81
    &Aura::SpellAuraWaterBreathing,                                 //SPELL_AURA_WATER_BREATHING = 82
    &Aura::SpellAuraUtilized,                                       //SPELL_AURA_MOD_BASE_RESISTANCE = 83
    &Aura::SpellAuraModRegen,                                       //SPELL_AURA_MOD_REGEN = 84
    &Aura::SpellAuraModPowerRegen,                                  //SPELL_AURA_MOD_POWER_REGEN = 85
    &Aura::SpellAuraChannelDeathItem,                               //SPELL_AURA_CHANNEL_DEATH_ITEM = 86
    &Aura::SpellAuraModDamagePercTaken,                             //SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN = 87
    &Aura::SpellAuraModRegenPercent,                                //SPELL_AURA_MOD_PERCENT_REGEN = 88
    &Aura::SpellAuraPeriodicDamagePercent,                          //SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89
    &Aura::SpellAuraModResistChance,                                //SPELL_AURA_MOD_RESIST_CHANCE = 90
    &Aura::SpellAuraModDetectRange,                                 //SPELL_AURA_MOD_DETECT_RANGE = 91
    &Aura::SpellAuraPreventsFleeing,                                //SPELL_AURA_PREVENTS_FLEEING = 92
    &Aura::SpellAuraModUnattackable,                                //SPELL_AURA_MOD_UNATTACKABLE = 93
    &Aura::SpellAuraInterruptRegen,                                 //SPELL_AURA_INTERRUPT_REGEN = 94
    &Aura::SpellAuraGhost,                                          //SPELL_AURA_GHOST = 95
    &Aura::SpellAuraMagnet,                                         //SPELL_AURA_SPELL_MAGNET = 96
    &Aura::SpellAuraManaShield,                                     //SPELL_AURA_MANA_SHIELD = 97
    &Aura::SpellAuraSkillTalent,                                    //SPELL_AURA_MOD_SKILL_TALENT = 98
    &Aura::SpellAuraModAttackPower,                                 //SPELL_AURA_MOD_ATTACK_POWER = 99
    &Aura::SpellAuraVisible,                                        //SPELL_AURA_AURAS_VISIBLE = 100
    &Aura::SpellAuraUtilized,                                       //SPELL_AURA_MOD_RESISTANCE_PCT = 101
    &Aura::SpellAuraModCreatureAttackPower,                         //SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102
    &Aura::SpellAuraModTotalThreat,                                 //SPELL_AURA_MOD_TOTAL_THREAT = 103
    &Aura::SpellAuraWaterWalk,                                      //SPELL_AURA_WATER_WALK = 104
    &Aura::SpellAuraFeatherFall,                                    //SPELL_AURA_FEATHER_FALL = 105
    &Aura::SpellAuraHover,                                          //SPELL_AURA_HOVER = 106
    &Aura::SpellAuraAddFlatModifier,                                //SPELL_AURA_ADD_FLAT_MODIFIER = 107
    &Aura::SpellAuraAddPctMod,                                      //SPELL_AURA_ADD_PCT_MODIFIER = 108
    &Aura::SpellAuraAddTargetTrigger,                               //SPELL_AURA_ADD_TARGET_TRIGGER = 109
    &Aura::SpellAuraModPowerRegPerc,                                //SPELL_AURA_MOD_POWER_REGEN_PERCENT = 110
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_ADD_CASTER_HIT_TRIGGER = 111
    &Aura::SpellAuraOverrideClassScripts,                           //SPELL_AURA_OVERRIDE_CLASS_SCRIPTS = 112
    &Aura::SpellAuraModRangedDamageTaken,                           //SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN = 113
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT = 114
    &Aura::SpellAuraModHealing,                                     //SPELL_AURA_MOD_HEALING = 115
    &Aura::SpellAuraIgnoreRegenInterrupt,                           //SPELL_AURA_IGNORE_REGEN_INTERRUPT = 116
    &Aura::SpellAuraModMechanicResistance,                          //SPELL_AURA_MOD_MECHANIC_RESISTANCE = 117
    &Aura::SpellAuraModHealingPCT,                                  //SPELL_AURA_MOD_HEALING_PCT = 118
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_SHARE_PET_TRACKING = 119
    &Aura::SpellAuraUntrackable,                                    //SPELL_AURA_UNTRACKABLE = 120
    &Aura::SpellAuraEmphaty,                                        //SPELL_AURA_EMPATHY = 121
    &Aura::SpellAuraModOffhandDamagePCT,                            //SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT = 122
    &Aura::SpellAuraModPenetration,                                 //SPELL_AURA_MOD_POWER_COST_PCT = 123
    &Aura::SpellAuraModRangedAttackPower,                           //SPELL_AURA_MOD_RANGED_ATTACK_POWER = 124
    &Aura::SpellAuraModMeleeDamageTaken,                            //SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN = 125
    &Aura::SpellAuraModMeleeDamageTakenPct,                         //SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT = 126
    &Aura::SpellAuraRAPAttackerBonus,                               //SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS = 127
    &Aura::SpellAuraModPossessPet,                                  //SPELL_AURA_MOD_POSSESS_PET = 128
    &Aura::SpellAuraModIncreaseSpeedAlways,                         //SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS = 129
    &Aura::SpellAuraModIncreaseMountedSpeed,                        //SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS = 130
    &Aura::SpellAuraModCreatureRangedAttackPower,                   //SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER = 131
    &Aura::SpellAuraModIncreaseEnergyPerc,                          //SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT = 132
    &Aura::SpellAuraModIncreaseHealthPerc,                          //SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT = 133
    &Aura::SpellAuraModManaRegInterrupt,                            //SPELL_AURA_MOD_MANA_REGEN_INTERRUPT = 134
    &Aura::SpellAuraModHealingDone,                                 //SPELL_AURA_MOD_HEALING_DONE = 135
    &Aura::SpellAuraModHealingDonePct,                              //SPELL_AURA_MOD_HEALING_DONE_PERCENT = 136
    &Aura::SpellAuraModTotalStatPerc,                               //SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137
    &Aura::SpellAuraModHaste,                                       //SPELL_AURA_MOD_HASTE = 138
    &Aura::SpellAuraForceReaction,                                  //SPELL_AURA_FORCE_REACTION = 139
    &Aura::SpellAuraModRangedHaste,                                 //SPELL_AURA_MOD_RANGED_HASTE = 140
    &Aura::SpellAuraModRangedAmmoHaste,                             //SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141
    &Aura::SpellAuraUtilized,                                       //SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142
    &Aura::SpellAuraUtilized,                                       //SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143
    &Aura::SpellAuraSafeFall,                                       //SPELL_AURA_SAFE_FALL = 144
    &Aura::SpellAuraModPetTalentPoints,                             //SPELL_AURA_MOD_PET_TALENT_POINTS = 145
    &Aura::SpellAuraAllowTamePetType,                               //SPELL_AURA_ALLOW_TAME_PET_TYPE = 146
    &Aura::SpellAuraAddCreatureImmunity,                            //SPELL_AURA_ADD_CREATURE_IMMUNITY = 147
    &Aura::SpellAuraRetainComboPoints,                              //SPELL_AURA_RETAIN_COMBO_POINTS = 148
    &Aura::SpellAuraResistPushback,                                 //SPELL_AURA_RESIST_PUSHBACK = 149
    &Aura::SpellAuraModShieldBlockPCT,                              //SPELL_AURA_MOD_SHIELD_BLOCK_PCT = 150
    &Aura::SpellAuraTrackStealthed,                                 //SPELL_AURA_TRACK_STEALTHED = 151
    &Aura::SpellAuraModDetectedRange,                               //SPELL_AURA_MOD_DETECTED_RANGE = 152
    &Aura::SpellAuraSplitDamageFlat,                                //SPELL_AURA_SPLIT_DAMAGE_FLAT= 153
    &Aura::SpellAuraModStealthLevel,                                //SPELL_AURA_MOD_STEALTH_LEVEL = 154
    &Aura::SpellAuraModUnderwaterBreathing,                         //SPELL_AURA_MOD_WATER_BREATHING = 155
    &Aura::SpellAuraModReputationAdjust,                            //SPELL_AURA_MOD_REPUTATION_ADJUST = 156
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_PET_DAMAGE_MULTI = 157
    &Aura::SpellAuraModBlockValue,                                  //SPELL_AURA_MOD_SHIELD_BLOCKVALUE = 158
    &Aura::SpellAuraNoPVPCredit,                                    //SPELL_AURA_NO_PVP_CREDIT = 159
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_AOE_AVOIDANCE = 160
    &Aura::SpellAuraModHealthRegInCombat,                           //SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT = 161
    &Aura::SpellAuraPowerBurn,                                      //SPELL_AURA_POWER_BURN_MANA = 162
    &Aura::SpellAuraModCritDmgPhysical,                             //SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE = 163
    &Aura::SpellAuraNULL,                                           //missing = 164
    &Aura::SpellAuraAPAttackerBonus,                                //SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS = 165
    &Aura::SpellAuraModPAttackPower,                                //SPELL_AURA_MOD_ATTACK_POWER_PCT = 166
    &Aura::SpellAuraModRangedAttackPowerPct,                        //SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT = 167
    &Aura::SpellAuraIncreaseDamageTypePCT,                          //SPELL_AURA_MOD_DAMAGE_DONE_VERSUS = 168
    &Aura::SpellAuraIncreaseCricticalTypePCT,                       //SPELL_AURA_MOD_CRIT_PERCENT_VERSUS = 169
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_DETECT_AMORE = 170
    &Aura::SpellAuraIncreasePartySpeed,                             //SPELL_AURA_MOD_SPEED_NOT_STACK = 171
    &Aura::SpellAuraIncreaseMovementAndMountedSpeed,                //SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK = 172
    &Aura::SpellAuraNULL,                                           //missing = 173
    &Aura::SpellAuraIncreaseSpellDamageByAttribute,                 //SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT = 174
    &Aura::SpellAuraIncreaseHealingByAttribute,                     //SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT = 175
    &Aura::SpellAuraSpiritOfRedemption,                             //SPELL_AURA_SPIRIT_OF_REDEMPTION = 176
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_AOE_CHARM = 177
    &Aura::SpellAuraDispelDebuffResist,                             //SPELL_AURA_MOD_DEBUFF_RESISTANCE = 178
    &Aura::SpellAuraIncreaseAttackerSpellCrit,                      //SPELL_AURA_INCREASE_ATTACKER_SPELL_CRIT = 179
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS = 180
    &Aura::SpellAuraNULL,                                           //missing = 181
    &Aura::SpellAuraIncreaseArmorByPctInt,                          //SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT = 182
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_CRITICAL_THREAT = 183
    &Aura::SpellAuraReduceAttackerMHitChance,                       //SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE = 184
    &Aura::SpellAuraReduceAttackerRHitChance,                       //SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE = 185
    &Aura::SpellAuraReduceAttackerSHitChance,                       //SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE = 186
    &Aura::SpellAuraReduceEnemyMCritChance,                         //SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE = 187
    &Aura::SpellAuraReduceEnemyRCritChance,                         //SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE = 188
    &Aura::SpellAuraIncreaseRating,                                 //SPELL_AURA_MOD_RATING = 189
    &Aura::SpellAuraIncreaseRepGainPct,                             //SPELL_AURA_MOD_FACTION_REPUTATION_GAIN = 190
    &Aura::SpellAuraUseNormalMovementSpeed,                         //SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED = 191
    &Aura::SpellAuraModAttackSpeed,                                 //SPELL_AURA_MOD_MELEE_RANGED_HASTE = 192
    &Aura::SpellAuraIncreaseTimeBetweenAttacksPCT,                  //SPELL_AURA_MELEE_SLOW = 193
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL = 194
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_TARGET_ABILITY_ABSORB_SCHOOL = 195
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_COOLDOWN = 196
    &Aura::SpellAuraModAttackerCritChance,                          //SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE = 197
    &Aura::SpellAuraIncreaseAllWeaponSkill,                         // = 198
    &Aura::SpellAuraIncreaseHitRate,                                //SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT = 199
    &Aura::SpellAuraModMobKillXPGain,                               //SPELL_AURA_MOD_XP_PCT = 200
    &Aura::SpellAuraEnableFlight,                                   //SPELL_AURA_FLY = 201
    &Aura::SpellAuraFinishingMovesCannotBeDodged,                   //SPELL_AURA_IGNORE_COMBAT_RESULT = 202
    &Aura::SpellAuraReduceCritMeleeAttackDmg,                       //SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE = 203
    &Aura::SpellAuraReduceCritRangedAttackDmg,                      //SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE = 204
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_SCHOOL_CRIT_DMG_TAKEN = 205
    &Aura::SpellAuraIncreaseFlightSpeed,                            //SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED = 206
    &Aura::SpellAuraEnableFlight,                                   //SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED = 207
    &Aura::SpellAuraEnableFlightWithUnmountedSpeed,                 //SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED = 208
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS = 209
    &Aura::SpellAuraIncreaseFlightSpeed,                            //SPELL_AURA_MOD_VEHICLE_SPEED_ALWAYS = 210
    &Aura::SpellAuraIncreaseFlightSpeed,                            //SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK = 211
    &Aura::SpellAuraIncreaseRangedAPStatPCT,                        //SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_INTELLECT = 212
    &Aura::SpellAuraIncreaseRageFromDamageDealtPCT,                 //SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT = 213
    &Aura::SpellAuraNULL,                                           // = 214
    &Aura::SpellAuraNoReagentCost,                                  //SPELL_AURA_ARENA_PREPARATION = 215
    &Aura::SpellAuraModCastingSpeed,                                //SPELL_AURA_HASTE_SPELLS = 216
    &Aura::SpellAuraNULL,                                           // = 217
    &Aura::SpellAuraHasteRanged,                                    //SPELL_AURA_HASTE_RANGED = 218
    &Aura::SpellAuraRegenManaStatPCT,                               //SPELL_AURA_MOD_MANA_REGEN_FROM_STAT = 219
    &Aura::SpellAuraSpellHealingStatPCT,                            //SPELL_AURA_MOD_RATING_FROM_STAT = 220
    &Aura::SpellAuraIgnoreEnemy,                                    //SPELL_AURA_MOD_DETAUNT = 221
    &Aura::SpellAuraNULL,                                           // = 222
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_RAID_PROC_FROM_CHARGE = 223
    &Aura::SpellAuraNULL,                                           // = 224
    &Aura::SpellAuraHealAndJump,                                    //SPELL_AURA_RAID_PROC_FROM_CHARGE_WITH_VALUE = 225
    &Aura::SpellAuraDrinkNew,                                       //SPELL_AURA_PERIODIC_DUMMY = 226
    &Aura::SpellAuraPeriodicTriggerSpellWithValue,                  //SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE = 227
    &Aura::SpellAuraAuraModInvisibilityDetection,                   //SPELL_AURA_DETECT_STEALTH = 228
    &Aura::SpellAuraReduceAOEDamageTaken,                           //SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE = 229
    &Aura::SpellAuraIncreaseMaxHealth,                              //SPELL_AURA_MOD_INCREASE_MAX_HEALTH = 230
    &Aura::SpellAuraProcTriggerWithValue,                           //SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE = 231
    &Aura::SpellAuraReduceEffectDuration,                           //SPELL_AURA_MECHANIC_DURATION_MOD = 232
    &Aura::SpellAuraNULL,                                           // = 233
    &Aura::SpellAuraReduceEffectDuration,                           //SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK = 234
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_DISPEL_RESIST = 235
    &Aura::SpellAuraVehiclePassenger,                               //SPELL_AURA_CONTROL_VEHICLE = 236
    &Aura::SpellAuraModSpellDamageFromAP,                           //SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER = 237
    &Aura::SpellAuraModSpellHealingFromAP,                          //SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER = 238
    &Aura::SpellAuraModScale,                                       //SPELL_AURA_MOD_SCALE_2 = 239
    &Aura::SpellAuraExpertise,                                      //SPELL_AURA_MOD_EXPERTISE = 240
    &Aura::SpellAuraForceMoveFoward,                                //SPELL_AURA_FORCE_MOVE_FORWARD = 241
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING = 242
    &Aura::SpellAuraModFaction,                                     //SPELL_AURA_MOD_FACTION = 243
    &Aura::SpellAuraComprehendLanguage,                             //SPELL_AURA_COMPREHEND_LANGUAGE = 244
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL = 245
    &Aura::SpellAuraReduceEffectDuration,                           //SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK = 246
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_CLONE_CASTER = 247
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_COMBAT_RESULT_CHANCE = 248
    &Aura::SpellAuraConvertRune,                                    //SPELL_AURA_CONVERT_RUNE = 249
    &Aura::SpellAuraModIncreaseHealth,                              //SPELL_AURA_MOD_INCREASE_HEALTH_2 = 250
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_ENEMY_DODGE = 251
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_SPEED_SLOW_ALL = 252
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_BLOCK_CRIT_CHANCE = 253
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_DISARM_OFFHAND = 254
    &Aura::SpellAuraModDamageTakenByMechPCT,                        //SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT = 255
    &Aura::SpellAuraNoReagent,                                      //SPELL_AURA_NO_REAGENT_USE = 256
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_TARGET_RESIST_BY_SPELL_CLASS = 257
    &Aura::SpellAuraNULL,                                           // = 258
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_HOT_PCT = 259
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_SCREEN_EFFECT = 260
    &Aura::SpellAuraSetPhase,                                       //SPELL_AURA_PHASE = 261
    &Aura::SpellAuraSkipCanCastCheck,                               //SPELL_AURA_ABILITY_IGNORE_AURASTATE = 262
    &Aura::SpellAuraCastFilter,                                     //SPELL_AURA_ALLOW_ONLY_ABILITY = 263
    &Aura::SpellAuraNULL,                                           // = 264
    &Aura::SpellAuraNULL,                                           // = 265
    &Aura::SpellAuraNULL,                                           // = 266
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL = 267
    &Aura::SpellAuraIncreaseAPByAttribute,                          //SPELL_AURA_MOD_ATTACK_POWER_OF_STAT_PERCENT = 268
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_IGNORE_TARGET_RESIST = 269
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST = 270
    &Aura::SpellAuraModDamageTakenPctPerCaster,                     //SPELL_AURA_MOD_DAMAGE_FROM_CASTER = 271
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_IGNORE_MELEE_RESET = 272
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_X_RAY = 273
    &Aura::SpellAuraRequireNoAmmo,                                  //SPELL_AURA_ABILITY_CONSUME_NO_AMMO = 274
    &Aura::SpellAuraSkipCanCastCheck,                               //SPELL_AURA_MOD_IGNORE_SHAPESHIFT = 275
    &Aura::SpellAuraNULL,                                           // = 276
    &Aura::SpellAuraRedirectThreat,                                 //SPELL_AURA_MOD_MAX_AFFECTED_TARGETS = 277
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_DISARM_RANGED = 278
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_INITIALIZE_IMAGES = 279
    &Aura::SpellAuraModIgnoreArmorPct,                              //SPELL_AURA_MOD_ARMOR_PENETRATION_PCT = 280
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_HONOR_GAIN_PCT = 281
    &Aura::SpellAuraModBaseHealth,                                  //SPELL_AURA_MOD_BASE_HEALTH_PCT = 282
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_HEALING_RECEIVED = 283
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_LINKED = 284
    &Aura::SpellAuraModAttackPowerByArmor,                          //SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR = 285
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_ABILITY_PERIODIC_CRIT = 286
    &Aura::SpellAuraReflectInfront,                                 //SPELL_AURA_DEFLECT_SPELLS = 287
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_IGNORE_HIT_DIRECTION = 288
    &Aura::SpellAuraNULL,                                           // = 289
    &Aura::SpellAuraModCritChanceAll,                               //SPELL_AURA_MOD_CRIT_PCT = 290
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_XP_QUEST_PCT = 291
    &Aura::SpellAuraOpenStable,                                     //SPELL_AURA_OPEN_STABLE = 292
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_OVERRIDE_SPELLS = 293
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_PREVENT_REGENERATE_POWER = 294
    &Aura::SpellAuraNULL,                                           // = 295
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_SET_VEHICLE_ID = 296
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_BLOCK_SPELL_FAMILY = 297
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_STRANGULATE = 298
    &Aura::SpellAuraNULL,                                           // = 299
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_SHARE_DAMAGE_PCT = 300
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_SCHOOL_HEAL_ABSORB = 301
    &Aura::SpellAuraNULL,                                           // = 302
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE = 303
    &Aura::SpellAuraFakeInebriation,                                //SPELL_AURA_MOD_FAKE_INEBRIATE = 304
    &Aura::SpellAuraModWalkSpeed,                                   //SPELL_AURA_MOD_MINIMUM_SPEED = 305
    &Aura::SpellAuraNULL,                                           // = 306
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_HEAL_ABSORB_TEST = 307
    &Aura::SpellAuraNULL,                                           // = 308
    &Aura::SpellAuraNULL,                                           // = 309
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE = 310
    &Aura::SpellAuraNULL,                                           // = 311
    &Aura::SpellAuraNULL,                                           // = 312
    &Aura::SpellAuraNULL,                                           // = 313
    &Aura::SpellAuraPreventResurrection,                            //SPELL_AURA_PREVENT_RESSURECTION = 314
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_UNDERWATER_WALKING = 315
    &Aura::SpellAuraNULL,                                           //SPELL_AURA_PERIODIC_HASTE = 316
};

Unit* Aura::GetUnitCaster()
{
    if( m_target == NULL && m_casterGuid && m_casterGuid.getHigh() == HIGHGUID_TYPE_PLAYER)
    {
        if(Player* punit = objmgr.GetPlayer(m_casterGuid))
            if(punit != NULL)
                return punit;
    }
    if( m_target == NULL )
        return NULL;

    if( m_casterGuid && m_casterGuid == m_target->GetGUID() )
        return m_target;
    if( m_target->GetMapMgr() != NULL )
        return m_target->GetMapMgr()->GetUnit( m_casterGuid );
    return NULL;
}

Aura::Aura( SpellEntry* proto, WorldObject* caster, Unit* target )
    : m_target(target), m_spellProto(proto), m_auraFlags(0), m_auraLevel(MAXIMUM_ATTAINABLE_LEVEL), m_expirationTime(0), m_casterGuid(caster->GetGUID()),
    m_auraSlot(255), m_applied(false), m_deleted(false), m_dispelled(false), m_castInDuel(false), m_creatureAA(false), m_areaAura(false), m_interrupted(-1),
    m_positive(!proto->isNegativeSpell1())
{
    p_target = (target->IsPlayer() ? castPtr<Player>(target) : NULL);
    m_stackSizeorProcCharges = (m_spellProto->procCharges ? (-int16(m_spellProto->procCharges&0xFF)) : 1);
    if(!IsPassive())
    {
        m_auraFlags |= AFLAG_EFF_AMOUNT_SEND;
        if(m_positive && !m_spellProto->isForcedApplication())
            m_auraFlags |= AFLAG_POSITIVE|AFLAG_CANCELLABLE;
        else if(m_positive)
            m_auraFlags |= AFLAG_POSITIVE;
        else m_auraFlags |= AFLAG_NEGATIVE;
    }

    m_modcount = 0;
    memset(m_modList, 0, sizeof(Modifier)*3);
    CalculateDuration();

    if( caster->IsUnit() )
    {
        m_auraLevel = castPtr<Unit>(caster)->getLevel();
        if(m_stackSizeorProcCharges < 0 && m_spellProto->SpellGroupType)
        {
            int32 procCharges = m_stackSizeorProcCharges&0xFF;
            castPtr<Unit>(caster)->SM_FIValue(SMT_CHARGES, (int32*)&procCharges, m_spellProto->SpellGroupType);
            castPtr<Unit>(caster)->SM_PIValue(SMT_CHARGES, (int32*)&procCharges, m_spellProto->SpellGroupType);
            m_stackSizeorProcCharges = -(procCharges&0xFF);
        }

        if( sFactionSystem.isAttackable( castPtr<Unit>(caster), target, false ) )
        {
            if( target->InStealth() && target != caster )
                target->RemoveStealth();
        }

        if(m_target->IsPlayer() && castPtr<Player>(m_target)->DuelingWith)
        {
            if( castPtr<Player>(m_target)->DuelingWith->GetGUID() == m_casterGuid )
                m_castInDuel = true;
        }
    }

    m_castedItemId = 0;
    m_triggeredSpellId = 0;
    periodic_target = 0;
    sLog.Debug("Aura","Constructor %u (%s) from %u.", m_spellProto->Id, m_spellProto->Name, m_target->GetLowGUID());
}

Aura::Aura(Unit *target, SpellEntry *proto, uint16 auraFlags, uint8 auraLevel, int16 auraStackCharge, time_t expirationTime, WoWGuid casterGuid)
    : m_target(target), m_spellProto(proto), m_auraFlags(auraFlags), m_auraLevel(auraLevel), m_expirationTime(0), m_casterGuid(casterGuid),
    m_auraSlot(255), m_applied(false), m_deleted(false), m_dispelled(false), m_castInDuel(false), m_creatureAA(false), m_areaAura(false), m_interrupted(-1),
    m_positive(!proto->isNegativeSpell1())
{
    p_target = (target->IsPlayer() ? castPtr<Player>(target) : NULL);
    m_stackSizeorProcCharges = auraStackCharge;
    if(expirationTime)
    {
        CalculateDuration();
        m_expirationTime = expirationTime;
    } else m_duration = -1;

    m_modcount = 0;
    memset(m_modList, 0, sizeof(Modifier)*3);

    m_castedItemId = 0;
    m_triggeredSpellId = 0;
    periodic_target = 0;

    sLog.Debug("Aura","Constructor %u (%s) from %u.", m_spellProto->Id, m_spellProto->Name, m_target->GetLowGUID());
}

Aura::~Aura()
{

}

void Aura::Update(uint32 diff)
{

    if(m_expirationTime == 0)
        return;

    if(m_expirationTime <= UNIXTIME)
        Remove();
}

void Aura::CalculateDuration()
{
    if(IsPassive())
        m_duration = -1;
    else
    {
        int32 comboPoints = 0;
        if(m_casterGuid.getHigh() == HIGHGUID_TYPE_PLAYER)
            if(Player *plr = castPtr<Player>(GetUnitCaster()))
                comboPoints = plr->m_comboPoints;
        int32 Duration = m_spellProto->CalculateSpellDuration(m_auraLevel, comboPoints);
        if(!m_positive && !m_spellProto->isPassiveSpell())
            ::ApplyDiminishingReturnTimer(&Duration, m_target, GetSpellProto());
        uint32 mechanic = GetMechanic();
        if( m_target->IsPlayer() && mechanic < NUM_MECHANIC && Duration > 0 )
            Duration *= m_target->GetMechanicDurationPctMod(mechanic);
        if((m_duration = Duration) > 0)
        {
            m_auraFlags |= AFLAG_HAS_DURATION;
            m_expirationTime = UNIXTIME+(m_duration/1000);
        }
    }
}

void Aura::Remove()
{
    if( m_deleted )
        return;
    m_deleted = true;

    if( !IsPassive() )
        BuildAuraUpdate();

    m_target->m_AuraInterface.OnAuraRemove(this, m_auraSlot);

    ApplyModifiers( false );

    // reset diminishing return timer if needed
    ::UnapplyDiminishingReturnTimer( m_target, m_spellProto );

    Unit * m_caster = GetUnitCaster();
    if (m_caster != NULL)
    {
        m_caster->OnAuraRemove(m_spellProto->NameHash, m_target);
        if(m_spellProto->IsSpellChannelSpell())
        {
            if(m_caster->GetCurrentSpell() && m_caster->GetCurrentSpell()->GetSpellProto()->Id == m_spellProto->Id)
            {
                m_caster->GetCurrentSpell()->SendChannelUpdate(0);
                m_caster->GetCurrentSpell()->finish();
            }
        }
    }

    for( uint32 x = 0; x < 3; x++ )
    {
        if( !m_spellProto->Effect[x] )
            continue;

        if( m_spellProto->Effect[x] == SPELL_EFFECT_TRIGGER_SPELL && !m_spellProto->always_apply )
            m_target->RemoveAura(m_spellProto->EffectTriggerSpell[x]);
    }

    int32 proccharges = m_spellProto->procCharges;
    if( m_caster != NULL && m_spellProto->SpellGroupType )
    {
        m_caster->SM_FIValue(SMT_CHARGES,&proccharges, m_spellProto->SpellGroupType);
        m_caster->SM_PIValue(SMT_CHARGES,&proccharges, m_spellProto->SpellGroupType);
    }

    // remove attacker
    if( m_caster != NULL)
    {
        if( m_caster != m_target )
        {
            // try to remove
            m_caster->CombatStatus.RemoveAttackTarget(m_target);
            if( m_caster->isDead() )
                m_target->CombatStatus.ForceRemoveAttacker( m_caster->GetGUID() );
        }
    } else m_target->CombatStatus.ForceRemoveAttacker( m_casterGuid );

    if( m_spellProto->MechanicsType == MECHANIC_ENRAGED )
        m_target->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_ENRAGE );
    else if( m_spellProto->Id == 642 )
    {
        m_target->RemoveAura( 53523 );
        m_target->RemoveAura( 53524 );
    }

    m_target = NULL;
    p_target = NULL;
    m_casterGuid = 0;

    delete this;
}

void Aura::OnTargetChangeLevel(uint32 newLevel, uint64 targetGuid)
{
    // Get our unit target so we can test against the given guid
    Unit *target = GetUnitTarget();
    if(target == NULL)
        return;

    if(target->GetGUID() == targetGuid)
        RecalculateModBaseAmounts();
}

void Aura::AddMod(uint32 i, uint32 t, int32 a, uint32 b, int32 f, float ff )
{
    if( m_modcount >= 3 || m_target == NULL || m_target->GetMechanicDispels(GetMechanicOfEffect(i)))
        return;

    if(i == 0) m_auraFlags |= AFLAG_EFF_INDEX_0;
    else if(i == 1) m_auraFlags |= AFLAG_EFF_INDEX_1;
    else if(i == 2) m_auraFlags |= AFLAG_EFF_INDEX_2;
    int32 amount = a;
    if(m_stackSizeorProcCharges >= 0) amount *= m_stackSizeorProcCharges;

    m_modList[m_modcount].i = i;
    m_modList[m_modcount].m_type = t;
    m_modList[m_modcount].m_amount = amount;
    m_modList[m_modcount].m_baseAmount = a;
    m_modList[m_modcount].m_miscValue[0] = m_spellProto->EffectMiscValue[i];
    m_modList[m_modcount].m_miscValue[1] = m_spellProto->EffectMiscValueB[i];
    m_modList[m_modcount].m_bonusAmount = b;
    m_modList[m_modcount].fixed_amount = f;
    m_modList[m_modcount].fixed_float_amount = ff;
    m_modList[m_modcount].m_spellInfo = GetSpellProto();
    CalculateBonusAmount(GetUnitCaster(), m_modcount);
    m_modcount++;
}

void Aura::ResetExpirationTime()
{
    if(m_expirationTime == 0 || m_duration == -1)
        return;
    m_expirationTime = UNIXTIME+(m_duration/1000);
}

void Aura::RemoveIfNecessary()
{
    if( !m_applied )
        return; // already removed

    if( m_spellProto->CasterAuraState && m_target && !(m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraState-1)) ) )
    {
        ApplyModifiers(false);
        return;
    }
    if( m_spellProto->CasterAuraStateNot && m_target && m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraStateNot-1)) )
    {
        ApplyModifiers(false);
        return;
    }
}

void Aura::ApplyModifiers( bool apply )
{
    if(!m_applied && !apply)    // Don't want to unapply modifiers if they haven't been applied
        return;

    m_applied = apply;
    if( apply && m_spellProto->CasterAuraState && m_target && !(m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraState - 1) ) ) )
    {
        m_applied = false;
        return;
    }
    if( apply && m_spellProto->CasterAuraStateNot && m_target && m_target->GetUInt32Value(UNIT_FIELD_AURASTATE) & (uint32(1) << (m_spellProto->CasterAuraStateNot - 1) ) )
    {
        m_applied = false;
        return;
    }

    for( uint32 x = 0; x < m_modcount; x++ )
    {
        mod = &m_modList[x];
        if(mod->m_type >= SPELL_AURA_TOTAL)
        {
            sLog.Debug( "Aura","Unknown Aura id %d in spell %u", uint32(mod->m_type), GetSpellId());
            continue;
        }

        m_target->m_AuraInterface.UpdateModifier(GetAuraSlot(), x, mod, apply);
        sLog.Debug( "Aura","Known Aura id %d, value %d in spell %u", uint32(mod->m_type), uint32(mod->m_amount), GetSpellId());

        (*this.*SpellAuraHandler[mod->m_type])(apply);
    }
}

void Aura::UpdateModifiers( )
{
    for( uint32 x = 0; x < m_modcount; x++ )
    {
        mod = &m_modList[x];
        if(mod->m_type >= SPELL_AURA_TOTAL)
        {
            sLog.Debug( "Aura","Unknown Aura id %d", (uint32)mod->m_type);
            continue;
        }

        m_target->m_AuraInterface.UpdateModifier(GetAuraSlot(), x, mod, m_applied);
        sLog.Debug( "Aura","Updating Aura modifiers target = %u, slot = %u, Spell Aura id = %u, SpellId  = %u, i = %u, duration = %i, damage = %d",
            m_target->GetLowGUID(), m_auraSlot, mod->m_type, m_spellProto->Id, mod->i, GetDuration(),mod->m_amount);
    }
}

bool Aura::AddAuraVisual()
{
    uint8 slot = m_target->m_AuraInterface.GetFreeSlot(IsPositive());
    if (slot == 0xFF)
        return false;

    m_auraSlot = slot;
    BuildAuraUpdate();
    return true;
}

void Aura::BuildAuraUpdate()
{
    if( m_target == NULL || IsPassive() )
        return;

    WorldPacket data(SMSG_AURA_UPDATE, 50);
    FastGUIDPack(data, m_target->GetGUID());
    BuildAuraUpdatePacket(&data);
    m_target->SendMessageToSet(&data, true);
}

void Aura::BuildAuraUpdatePacket(WorldPacket *data)
{
    *data << uint8(m_auraSlot);
    if(m_stackSizeorProcCharges == 0)
    {
        *data << uint32(0);
        return;
    }

    uint16 flags = GetAuraFlags();
    *data << uint32(GetSpellProto()->Id) << uint16(flags);
    *data << uint8(GetAuraLevel());
    *data << uint8(m_stackSizeorProcCharges & 0xFF);

    if(!(flags & AFLAG_NOT_GUID))
        *data << GetCasterGUID().asPacked();
    if( flags & AFLAG_HAS_DURATION )
    {
        *data << GetDuration();
        *data << GetMSTimeLeft();
    }

    if (flags & AFLAG_EFF_AMOUNT_SEND)
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (flags & 1<<i)
            {
                Modifier *mod = GetMod(i);
                *data << uint32(mod ? mod->m_amount : 0);
            }
        }
    }
}

void Aura::EventRelocateRandomTarget()
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL || !m_caster->IsPlayer() || m_caster->isDead() || !m_caster->GetInRangeCount() )
        return;

    // Ok, let's do it. :D
    std::set<Unit* > enemies;
    
    WorldObject::InRangeMap::iterator itr = m_caster->GetInRangeMapBegin();
    for(; itr != m_caster->GetInRangeMapEnd(); itr++)
    {
        if( !itr->second->IsUnit() )
            continue;

        if( !sFactionSystem.isHostile( m_caster, itr->second ) )
            continue;

        // Too far away or dead, or I can't see him!
        if( castPtr<Unit>(itr->second)->isDead() || m_caster->GetDistance2dSq( itr->second ) > 100 || !castPtr<Player>(m_caster)->CanSee(itr->second) )
            continue;

        if (m_caster->GetMapMgr() && m_caster->GetMapMgr()->CanUseCollision(m_caster))
        {
            if( !sVMapInterface.CheckLOS( m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPhaseMask(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ() + 2.0f, itr->second->GetPositionX(), itr->second->GetPositionY(), itr->second->GetPositionZ() + 2.0f) )
                continue;
        }

        enemies.insert( castPtr<Unit>(itr->second) );
    }

    // Can't do anything w/o a target
    if( !enemies.size() )
        return;

    uint32 random = RandomUInt(uint32(enemies.size()) - 1);
    std::set<Unit* >::iterator it2 = enemies.begin();
    while( random-- )
        it2++;

    Unit* pTarget = (*it2);
    if(pTarget == NULL)
        return; // In case I did something horribly wrong.

    float ang = pTarget->GetOrientation();

    // avoid teleporting into the model on scaled models
    const static float killingspree_distance = 1.6f * GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( pTarget->GetUInt32Value(UNIT_FIELD_DISPLAYID)));
    float new_x = pTarget->GetPositionX() - (killingspree_distance * cosf(ang));
    float new_y = pTarget->GetPositionY() - (killingspree_distance * sinf(ang));
    float new_z = pTarget->GetCHeightForPosition(true, new_x, new_y);
    castPtr<Player>(m_caster)->SafeTeleport( pTarget->GetMapId(), pTarget->GetInstanceID(), new_x, new_y, new_z, pTarget->GetOrientation() );
    // void Unit::Strike( Unit pVictim, uint32 weapon_damage_type, SpellEntry* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check )
    castPtr<Player>(m_caster)->Strike( pTarget, MELEE, NULL, 0, 0, 0, false, false, true );
    castPtr<Player>(m_caster)->Strike( pTarget, OFFHAND, NULL, 0, 0, 0, false, false, true );
}

//------------------------- Aura Effects -----------------------------

void Aura::SpellAuraNULL(bool apply)
{
    sLog.Debug( "Aura","Unknown Aura id %d in spell %u", uint32(mod->m_type), GetSpellId());
}

void Aura::SpellAuraUtilized(bool apply)
{
    // We do nothing here
}

void Aura::SpellAuraBindSight(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster != NULL || !m_caster->IsPlayer())
        return;
    if(apply)
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, m_target->GetGUID());
    else m_caster->SetUInt64Value(PLAYER_FARSIGHT, 0 );
}

void Aura::SpellAuraModPossess(bool apply)
{

}

void Aura::SpellAuraPeriodicDamage(bool apply)
{

}

void Aura::EventPeriodicDamage(uint32 amount)
{

}

void Aura::SpellAuraDummy(bool apply)
{

}

void Aura::SpellAuraModConfuse(bool apply)
{

}

void Aura::SpellAuraModCharm(bool apply)
{

}

void Aura::SpellAuraModFear(bool apply)
{

}

void Aura::SpellAuraPeriodicHeal( bool apply )
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL)
        return;

}

void Aura::EventPeriodicHeal( uint32 amount )
{
    int32 add = amount; // IMPORTANT: target heals himself, but the packet says the caster does it. This is important, to allow for casters to log out and players still get healed.
    uint32 overheal = m_target->Heal(m_target, GetSpellId(), add, true);
    SendPeriodicAuraLog( m_casterGuid, m_target, GetSpellProto(), add, 0, overheal, FLAG_PERIODIC_HEAL );

    if( m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP )
        m_target->Emote( EMOTE_ONESHOT_EAT );

    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL || !m_caster->IsInWorld())
        return;

    if(m_target && m_target->IsInWorld() && m_caster != NULL && m_caster->IsInWorld())
        m_caster->CombatStatus.WeHealed(m_target);

    uint32 base_threat=Spell::GetBaseThreat(add);
    int count = 0;
    Unit* unit = NULL;
    std::vector< Unit* > target_threat;
    if( base_threat > 0 )
    {
        target_threat.reserve(m_caster->GetInRangeCount()); // this helps speed

        for(WorldObject::InRangeUnitSet::iterator itr = m_caster->GetInRangeUnitSetBegin(); itr != m_caster->GetInRangeUnitSetEnd(); itr++)
        {
            unit = *itr;
            if(unit->GetAIInterface()->GetNextTarget() == m_target)
            {
                target_threat.push_back(unit);
                ++count;
            }
        }
        if(count == 0)
            count = 1;  // division against 0 protection
        /*
        When a tank hold multiple mobs, the threat of a heal on the tank will be split between all the mobs.
        The exact formula is not yet known, but it is more than the Threat/number of mobs.
        So if a tank holds 5 mobs and receives a heal, the threat on each mob will be less than Threat(heal)/5.
        Current speculation is Threat(heal)/(num of mobs *2)
        */
        uint32 threat = base_threat / (count * 2);

        for(std::vector<Unit* >::iterator itr = target_threat.begin(); itr != target_threat.end(); itr++)
        {
            // for now we'll just use heal amount as threat.. we'll prolly need a formula though
            (castPtr<Unit>(*itr))->GetAIInterface()->HealReaction(m_caster, m_target, threat, m_spellProto);
        }
    }
}

void Aura::SpellAuraModAttackSpeed(bool apply)
{

}

void Aura::SpellAuraModThreatGenerated(bool apply)
{
    if(!m_target)
        return;

    //shaman spell 30672 needs to be based on spell schools
    if(m_target->GetGeneratedThreatModifier() == mod->m_amount)
    {
        apply ? m_target->ModGeneratedThreatModifier(mod->m_amount) : m_target->ModGeneratedThreatModifier(-(mod->m_amount));
        return;
    }
    else // if we this aura is better then the old one, replace the effect.
    {
        if(apply)
        {
            if(m_target->GetGeneratedThreatModifier() < mod->m_amount)
            {
                m_target->ModGeneratedThreatModifier(-m_target->GetGeneratedThreatModifier());
                m_target->ModGeneratedThreatModifier(mod->m_amount);
            }
        }
    }
}

void Aura::SpellAuraModTaunt(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL || !m_caster->isAlive() || m_target->IsPlayer())
        return;

    if(apply)
    {
        m_target->GetAIInterface()->AttackReaction(m_caster, 1, 0);
        m_target->GetAIInterface()->taunt(m_caster, true);
    }
    else if(m_target->GetAIInterface()->getTauntedBy() == m_caster)
        m_target->GetAIInterface()->taunt(m_caster, false);
}

void Aura::SpellAuraModStun(bool apply)
{

}

void Aura::SpellAuraModDamageDone(bool apply)
{

}

void Aura::SpellAuraModDamageTaken(bool apply)
{

}

void Aura::SpellAuraDamageShield(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModStealth(bool apply)
{
    if(apply)
    {
        if(m_target->IsPlayer() && castPtr<Player>(m_target)->m_bg != NULL && castPtr<Player>(m_target)->m_bgHasFlag )
        {
            switch( castPtr<Player>(m_target)->m_bg->GetType())
            {
            case BATTLEGROUND_WARSONG_GULCH:
                castPtr<WarsongGulch>(castPtr<Player>(m_target)->m_bg)->DropFlag(castPtr<Player>(m_target));
                break;
            case BATTLEGROUND_EYE_OF_THE_STORM:
                castPtr<EyeOfTheStorm>(castPtr<Player>(m_target)->m_bg)->DropFlag(castPtr<Player>(m_target));
                break;
            }
        }

        if(m_spellProto->NameHash != SPELL_HASH_VANISH)
            m_target->SetStealth(GetSpellId());

        // Stealth level (not for normal stealth... ;p)
        if( m_spellProto->NameHash == SPELL_HASH_STEALTH )
            m_target->SetFlag(UNIT_FIELD_BYTES_2, 0x1E000000);//sneak anim

        m_target->SetFlag(UNIT_FIELD_BYTES_1, 0x020000);
        if( m_target->IsPlayer() )
            m_target->SetFlag(PLAYER_FIELD_BYTES2, 0x2000);

        m_target->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_STEALTH, GetSpellId());
        m_target->m_stealthLevel += mod->m_amount;

        if( !m_target->InStealth() && m_target->HasDummyAura(SPELL_HASH_OVERKILL) )
            m_target->CastSpell(m_target, 58427, true);

        // hack fix for vanish stuff
        if(m_spellProto->NameHash == SPELL_HASH_VANISH && m_target->IsPlayer())     // Vanish
        {
            for(WorldObject::InRangeUnitSet::iterator iter = m_target->GetInRangeUnitSetBegin(); iter != m_target->GetInRangeUnitSetEnd(); ++iter)
            {
                Unit* _unit = *iter;
                if(!_unit->isAlive())
                    continue;

                if(_unit->GetCurrentSpell() && _unit->GetCurrentSpell()->GetUnitTarget() == m_target)
                    _unit->GetCurrentSpell()->cancel();

                if(_unit->GetAIInterface() != NULL)
                    _unit->GetAIInterface()->RemoveThreat(m_target->GetGUID());
            }

            m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ROOTED);
            m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ENSNARED);
            m_target->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);
            m_target->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_STALKED);
            m_target->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_DECREASE_SPEED);

            // check for stealth spells
            if(p_target != NULL)
            {
                uint32 stealth_id = 0;
                SpellSet::iterator itr = p_target->mSpells.begin();
                SpellSet::iterator end = p_target->mSpells.end();
                for(; itr != end; ++itr)
                {
                    if(((*itr) == 1787 || (*itr) == 1786 || (*itr) == 1785 || (*itr) == 1784) && stealth_id < (*itr))
                    {
                        stealth_id = *itr;
                    }
                }

                if(stealth_id != 0)
                    p_target->CastSpell(p_target, dbcSpell.LookupEntry(stealth_id), true);

                p_target->Dismount();
            }
        }
    }
    else
    {
        m_target->m_stealthLevel -= mod->m_amount;

        if(m_spellProto->NameHash != SPELL_HASH_VANISH)
        {
            m_target->SetStealth(0);
            m_target->RemoveFlag(UNIT_FIELD_BYTES_2, 0x1E000000);
            m_target->RemoveFlag(UNIT_FIELD_BYTES_1, 0x020000);

            if( m_target->IsPlayer() )
            {
                m_target->RemoveFlag(PLAYER_FIELD_BYTES2, 0x2000);

                WorldPacket data(SMSG_COOLDOWN_EVENT, 12);
                data << m_spellProto->Id;
                data << m_target->GetGUID();
                castPtr<Player>(m_target)->SendPacket(&data);
            }
        }

        if( (m_target->HasDummyAura(SPELL_HASH_MASTER_OF_SUBTLETY) || m_target->HasDummyAura(SPELL_HASH_OVERKILL)) && m_spellProto->NameHash == SPELL_HASH_STEALTH )
        {
            m_target->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_OVERKILL, false);
            m_target->m_AuraInterface.RemoveAllAurasByNameHash(SPELL_HASH_MASTER_OF_SUBTLETY, false);
        }
    }
    m_target->UpdateVisibility();
}

void Aura::SpellAuraModDetect(bool apply)
{
    if(apply)
        m_target->m_stealthDetectBonus += mod->m_amount;
    else m_target->m_stealthDetectBonus -= mod->m_amount;
}

void Aura::SpellAuraModInvisibility(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>(m_target)->m_bgFlagIneligible++;
        else
            castPtr<Player>(m_target)->m_bgFlagIneligible--;

        if(castPtr<Player>(m_target)->m_bg != NULL && castPtr<Player>(m_target)->m_bgHasFlag)
            castPtr<Player>(m_target)->m_bg->HookOnMount(castPtr<Player>(m_target));
        TRIGGER_INSTANCE_EVENT( m_target->GetMapMgr(), OnPlayerMount )( castPtr<Player>(m_target) );
    }

    if(m_spellProto->Effect[mod->i] == 128)
        return;

    if(apply) 
    {
        m_target->SetInvisibility(GetSpellId());
        m_target->m_invisFlag = mod->m_miscValue[0];
    }
    else 
    {
        m_target->SetInvisibility(0);
        m_target->m_invisFlag = INVIS_FLAG_NORMAL;
    }

    m_target->m_invisible = apply;
    m_target->UpdateVisibility();
}

void Aura::SpellAuraModInvisibilityDetection(bool apply)
{
    //Always Positive

    assert(mod->m_miscValue[0] < INVIS_FLAG_TOTAL);
    if(apply)
        m_target->m_invisDetect[mod->m_miscValue[0]] += mod->m_amount;
    else m_target->m_invisDetect[mod->m_miscValue[0]] -= mod->m_amount;

    if(m_target->IsPlayer())
        castPtr<Player>( m_target )->UpdateVisibility();
}

void Aura::SpellAuraModTotalHealthRegenPct(bool apply)
{

}

void Aura::EventPeriodicHealPct(float RegenPct)
{
    Unit* m_caster = GetUnitCaster();
    if(m_caster == NULL || !m_target->isAlive())
        return;

    uint32 add = float2int32(m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * (RegenPct / 100.0f));
    uint32 overheal = m_caster->Heal(m_target, GetSpellId(), add, true);
    SendPeriodicAuraLog( m_casterGuid, m_target, GetSpellProto(), add, 0, overheal, FLAG_PERIODIC_HEAL );

    if(m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
        m_target->Emote(EMOTE_ONESHOT_EAT);
}

void Aura::SpellAuraModTotalManaRegenPct(bool apply)
{

}

void Aura::EventPeriodicManaPct(float RegenPct)
{
    if(!m_target->isAlive())
        return;
    uint32 manaMax = m_target->GetMaxPower(POWER_TYPE_MANA);
    if(manaMax == 0)
        return;

    uint32 add = float2int32(manaMax * (RegenPct / 100.0f));
    uint32 newMana = m_target->GetPower(POWER_TYPE_MANA) + add;

    m_target->SetPower(POWER_TYPE_MANA, newMana <= manaMax ? newMana : manaMax);
    SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, add, 0, 0, FLAG_PERIODIC_ENERGIZE);

    if(m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
        m_target->Emote(EMOTE_ONESHOT_EAT);
    m_target->SendPowerUpdate();
}

void Aura::SpellAuraPeriodicTriggerSpell(bool apply)
{

}

void Aura::EventPeriodicTriggerSpell(SpellEntry* spellInfo, bool overridevalues, int32 overridevalue)
{
    Spell* spell = new Spell(m_target, spellInfo, true, this);
    /*if(overridevalues)
        for(uint32 i = 0; i < 3; ++i)
            spell->forced_basepoints[i] = overridevalue;*/
    SpellCastTargets targets;
    spell->GenerateTargets(&targets);
    if(spell->prepare(&targets) != SPELL_CANCAST_OK)
        Remove();
}

void Aura::SpellAuraPeriodicEnergize(bool apply)
{

}

void Aura::EventPeriodicEnergize(uint32 amount,uint32 type)
{
    m_target->Energize(m_target, m_spellProto->Id, amount, type );
    if((m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP) && type == 0)
        m_target->Emote(EMOTE_ONESHOT_EAT);

    m_target->SendPowerUpdate();
}

void Aura::SpellAuraModPacify(bool apply)
{
    // Can't Attack
    if( apply )
    {
        m_target->m_pacified++;
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }
    else
    {
        m_target->m_pacified--;
        if(m_target->m_pacified == 0)
        {
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
        }
    }
}

void Aura::SpellAuraModRoot(bool apply)
{

}

void Aura::SpellAuraModSilence(bool apply)
{
    if(apply)
    {
        m_target->m_silenced++;
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);

        // remove the current spell (for channelers)
        if(m_target->GetCurrentSpell() && m_target->GetGUID() != m_casterGuid &&
            m_target->GetCurrentSpell()->getState() == SPELL_STATE_CASTING )
        {
            m_target->GetCurrentSpell()->cancel();
            m_target->SetCurrentSpell(NULL);
        }
    }
    else
    {
        m_target->m_silenced--;

        if(m_target->m_silenced == 0)
        {
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        }
    }
}

void Aura::SpellAuraReflectSpells(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModStat(bool apply)
{

}

void Aura::SpellAuraModSkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->_ModifySkillBonus(mod->m_miscValue[0], mod->m_amount);
        else castPtr<Player>( m_target )->_ModifySkillBonus(mod->m_miscValue[0], -mod->m_amount);
    }
}

void Aura::SpellAuraModIncreaseSpeed(bool apply)
{

}

void Aura::SpellAuraModIncreaseMountedSpeed(bool apply)
{

}

void Aura::SpellAuraModCreatureRangedAttackPower(bool apply)
{

}

void Aura::SpellAuraModDecreaseSpeed(bool apply)
{

}

void Aura::UpdateAuraModDecreaseSpeed()
{

}

void Aura::SpellAuraModIncreaseHealth(bool apply)
{

}

void Aura::SpellAuraModIncreaseEnergy(bool apply)
{

}

void Aura::SpellAuraModShapeshift(bool apply)
{
    if( !m_target->IsPlayer())
        return;
    Player *p = castPtr<Player>(m_target);

    if( p->m_MountSpellId && p->m_MountSpellId != m_spellProto->Id )
        if( !(mod->m_miscValue[0] & FORM_BATTLESTANCE | FORM_DEFENSIVESTANCE | FORM_BERSERKERSTANCE ) )
            m_target->RemoveAura( p->m_MountSpellId ); // these spells are not compatible

    uint32 spellId = 0;
    uint32 spellId2 = 0;
    uint32 modelId = p->GenerateShapeshiftModelId(mod->m_miscValue[0]);

    bool freeMovements = false;

    switch( mod->m_miscValue[0] )
    {
    case FORM_CAT:
        {//druid
            freeMovements = true;
            spellId = 3025;
            if(apply)
            {
                m_target->SetByte(UNIT_FIELD_BYTES_0, 3, POWER_TYPE_ENERGY);
                if( m_target->HasDummyAura(SPELL_HASH_FUROR) )
                    m_target->ModPower(POWER_TYPE_ENERGY, m_target->GetDummyAura(SPELL_HASH_FUROR)->RankNumber * 20);
            }
            else
            {//turn back to mana
                //m_target->SetUInt32Value(UNIT_FIELD_BASEATTACKTIME,oldap);
                m_target->SetByte(UNIT_FIELD_BYTES_0,3,POWER_TYPE_MANA);
                if(m_target->m_stealth)
                {
                    uint32 sp = m_target->m_stealth;
                    m_target->m_stealth = 0;
                    m_target->RemoveAura(sp);//prowl
                }
                if(m_target->m_AuraInterface.HasActiveAura(1850))
                    m_target->RemoveAura(1850);//Dash rank1
                if(m_target->m_AuraInterface.HasActiveAura(9821))
                    m_target->RemoveAura(9821);//Dash rank2
                if(m_target->m_AuraInterface.HasActiveAura(33357))
                    m_target->RemoveAura(33357);//Dash rank3
            }
        }break;

    case FORM_TREE:
        {
            freeMovements=true;
            spellId = 34123;
        }break;

    case FORM_TRAVEL:
        {//druid
            freeMovements = true;
            spellId = 5419;
        }break;

    case FORM_AQUA:
        {//druid aqua
            freeMovements = true;
            spellId = 5421;
        }break;

    case FORM_BEAR:
        {//druid only
            freeMovements = true;
            spellId = 1178;
            spellId2 = 21178;
            if(apply)
            {
                m_target->SetPowerType(POWER_TYPE_RAGE);
                m_target->SetMaxPower(POWER_TYPE_RAGE, 1000);
            } else m_target->SetPowerType(POWER_TYPE_MANA);
        }break;

    case FORM_DIREBEAR:
        {//druid only
            freeMovements = true;
            spellId = 9635;
            spellId2 = 21178;
            if(apply)
            {
                m_target->SetPowerType(POWER_TYPE_RAGE);
                m_target->SetMaxPower(POWER_TYPE_RAGE, 1000);
            } else m_target->SetPowerType(POWER_TYPE_MANA);
        }break;

    case FORM_GHOSTWOLF:
        {
            spellId = 67116;
            if(m_target->IsPlayer())
                p->m_MountSpellId = apply ? m_spellProto->Id : 0;
        }break;

    case FORM_BATTLESTANCE:
        {
            spellId = 21156;
        }break;

    case FORM_DEFENSIVESTANCE:
        {
            spellId = 7376;
        }break;

    case FORM_BERSERKERSTANCE:
        {
            spellId = 7381;
        }break;

    case FORM_SHADOW:
        {
            spellId = 49868;
            spellId2 = 71167;
            if(apply)
            {
                WorldPacket data(SMSG_COOLDOWN_EVENT, 12);
                data << m_spellProto->Id;
                data << m_target->GetGUID();
                castPtr<Player>(m_target)->SendPacket(&data);
            }
        }break;

    case FORM_FLIGHT:
        {//druid
            freeMovements = true;
            spellId = 33948;
            spellId2 = 34764;
        }break;

    case FORM_STEALTH:
        {// rogue
            if (!m_target->m_can_stealth)
                return;
            //m_target->UpdateVisibility();
        } break;

    case FORM_MOONKIN:
        {//druid
            freeMovements = true;
            spellId = 24905;
            spellId2 = 69366;
        }break;

    case FORM_SWIFT: //not tested yet, right now going on trust
        {// druid
            freeMovements = true;
            spellId = 40121; //Swift Form Passive
            spellId2 = 40122;
        }break;

    case FORM_SPIRITOFREDEMPTION:
        {
            spellId = 27795;
            spellId2 = 27795;
        }break;

    case FORM_DEMON:
        {
            spellId  = 54817;
            spellId2 = 54879;
            //if(GetUnitCaster()->IsPlayer() && GetUnitCaster()->HasDummyAura(SPELL_HASH_GLYPH_OF_METAMORPHOSIS))
                //SetDuration(GetDuration() + 6000);
        }break;
    }

    if( apply )
    {
        if( spellId != GetSpellId() )
        {
            if( p->m_ShapeShifted )
                p->RemoveAura( p->m_ShapeShifted );

            p->m_ShapeShifted = GetSpellId();
        }

        if( modelId != 0 )
        {
            m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, modelId );
            m_target->EventModelChange();
        }

        p->SetShapeShift( mod->m_miscValue[0] );

        // check for spell id
        if( spellId != 0 )
        {
            SpellEntry* spellInfo = dbcSpell.LookupEntry(spellId );
            if(spellInfo->NameHash != GetSpellProto()->NameHash)
            {
                Spell* sp = NULL;
                sp = (new Spell( m_target, spellInfo, true, NULL ));
                SpellCastTargets tgt;
                tgt.m_unitTarget = m_target->GetGUID();
                sp->prepare( &tgt );
            }
        }

        if( spellId2 != 0 )
        {
            SpellEntry* spellInfo = dbcSpell.LookupEntry(spellId2);
            if(spellInfo->NameHash != GetSpellProto()->NameHash)
            {
                Spell* sp = NULL;
                sp = (new Spell( m_target, spellInfo, true, NULL ));
                SpellCastTargets tgt;
                tgt.m_unitTarget = m_target->GetGUID();
                sp->prepare(&tgt);
            }
        }

        // remove the caster from imparing movements
        if( freeMovements )
        {
            m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ROOTED);
            m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ENSNARED);
            m_target->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);
            m_target->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_DECREASE_SPEED);
        }
    }
    else
    {
        m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
        m_target->EventModelChange();

        if(spellId != 0)
        {
            m_target->RemoveAura(spellId);
            if( spellId == 27795 ) //Spirit Redemption
                m_target->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
        }

        if(spellId2 != 0)
            m_target->RemoveAura(spellId2);

        p->m_ShapeShifted = 0;
        p->SetShapeShift(0);

        if(m_target->HasAura(52610))
            m_target->RemoveAura(52610);
    }
}

void Aura::SpellAuraModEffectImmunity(bool apply)
{

}

void Aura::SpellAuraModStateImmunity(bool apply)
{
    //%50 chance to dispel 1 magic effect on target
    //23922
}

void Aura::SpellAuraModSchoolImmunity(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply) castPtr<Player>(m_target)->m_bgFlagIneligible++;
        else castPtr<Player>(m_target)->m_bgFlagIneligible--;
    }

    if( apply && ( m_spellProto->NameHash == SPELL_HASH_DIVINE_SHIELD || m_spellProto->NameHash == SPELL_HASH_ICE_BLOCK) ) // Paladin - Divine Shield
    {
        if( !m_target || !m_target->isAlive())
            return;

        m_target->m_AuraInterface.RemoveAllAurasWithAttributes(0, ATTRIBUTES_IGNORE_INVULNERABILITY);

        if(m_target->IsPlayer() && castPtr<Player>(m_target)->m_bg && castPtr<Player>(m_target)->m_bgHasFlag)
        {
            if(castPtr<Player>(m_target)->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
            {
                castPtr<WarsongGulch>(castPtr<Player>(m_target)->m_bg)->DropFlag(castPtr<Player>(m_target));
            }
            if(castPtr<Player>(m_target)->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
            {
                castPtr<EyeOfTheStorm>(castPtr<Player>(m_target)->m_bg)->DropFlag(castPtr<Player>(m_target));
            }
        }
    }

    if(apply)
    {
        for(uint32 i = 0; i < 7; i++)
        {
            if(mod->m_miscValue[0] & (1<<i))
            {
                m_target->SchoolImmunityList[i] ++;
                m_target->m_AuraInterface.RemoveAllAurasOfSchool(i, false, true);
            }
        }
    }
    else
    {
        for(int i = 0; i < 7; i++)
        {
            if(mod->m_miscValue[0] & (1<<i) &&
                m_target->SchoolImmunityList[i] > 0)
                m_target->SchoolImmunityList[i]--;
        }
    }
}

void Aura::SpellAuraModDmgImmunity(bool apply)
{

}

void Aura::SpellAuraModDispelImmunity(bool apply)
{
    assert(mod->m_miscValue[0] < 10);

    if(apply)
        m_target->m_AuraInterface.RemoveAllAurasWithDispelType((uint32)mod->m_miscValue[0]);
}

void Aura::SpellAuraProcTriggerSpell(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraProcTriggerDamage(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraTrackCreatures(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            if(castPtr<Player>( m_target )->TrackingSpell)
                m_target->RemoveAura( castPtr<Player>( m_target )->TrackingSpell);

            m_target->SetUInt32Value(PLAYER_TRACK_CREATURES,(uint32)1<< (mod->m_miscValue[0]-1));
            castPtr<Player>( m_target )->TrackingSpell = GetSpellId();
        }
        else
        {
            castPtr<Player>( m_target )->TrackingSpell = 0;
            m_target->SetUInt32Value(PLAYER_TRACK_CREATURES,0);
        }
    }
}

void Aura::SpellAuraTrackResources(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            if(castPtr<Player>( m_target )->TrackingSpell)
                m_target->RemoveAura(castPtr<Player>( m_target )->TrackingSpell);

        m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES,(uint32)1<< (mod->m_miscValue[0]-1));
        castPtr<Player>( m_target )->TrackingSpell = GetSpellId();
        }
        else
        {
            castPtr<Player>( m_target )->TrackingSpell = 0;
                m_target->SetUInt32Value(PLAYER_TRACK_RESOURCES,0);
        }
    }
}

void Aura::SpellAuraModParrySkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_PARRY, mod->m_amount);
        else castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_PARRY, -mod->m_amount);
    }
}

void Aura::SpellAuraModParryPerc(bool apply)
{
    if (m_target->IsPlayer())
    {
        int32 amt = apply ? mod->m_amount : -mod->m_amount;
        castPtr<Player>( m_target )->SetParryFromSpell(castPtr<Player>( m_target )->GetParryFromSpell() + amt );
    }
}

void Aura::SpellAuraModDodgeSkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_DODGE, mod->m_amount);
        else castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_DODGE, -mod->m_amount);
    }
}

void Aura::SpellAuraModDodgePerc(bool apply)
{
    if (m_target->IsPlayer())
    {
        int32 amt = apply ? mod->m_amount : -mod->m_amount;
        castPtr<Player>( m_target )->SetDodgeFromSpell(castPtr<Player>( m_target )->GetDodgeFromSpell() + amt );
    }
}

void Aura::SpellAuraModBlockSkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_BLOCK, mod->m_amount);
        else castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_BLOCK, -mod->m_amount);
    }
}

void Aura::SpellAuraModBlockPerc(bool apply)
{
    if (m_target->IsPlayer())
    {
        int32 amt = apply ? mod->m_amount : -mod->m_amount;
        castPtr<Player>( m_target )->SetBlockFromSpell(castPtr<Player>( m_target )->GetBlockFromSpell() + amt);
    }
}

void Aura::SpellAuraModCritPerc(bool apply)
{
    SpecialCases();
    if (m_target->IsPlayer())
    {
        if(apply)
        {
            WeaponModifier md;
            md.value = float(mod->m_amount);
            md.wclass = m_spellProto->EquippedItemClass;
            md.subclass = m_spellProto->EquippedItemSubClass;
            castPtr<Player>( m_target )->tocritchance.insert(std::make_pair(GetSpellId(), md));
        }
        else
        {
            /*std::list<WeaponModifier>::iterator i = castPtr<Player>( m_target )->tocritchance.begin();

            for(;i!=castPtr<Player>( m_target )->tocritchance.end();++i)
            {
                if((*i).spellid==GetSpellId())
                {
                    castPtr<Player>( m_target )->tocritchance.erase(i);
                    break;
                }
            }*/
            castPtr<Player>( m_target )->tocritchance.erase(GetSpellId());
        }
    }
}

void Aura::SpellAuraPeriodicLeech(bool apply)
{

}

void Aura::EventPeriodicLeech(uint32 amount, SpellEntry* sp)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL || m_target == NULL || !m_target->isAlive() || !m_caster->isAlive() )
        return;

    if( m_target->SchoolImmunityList[sp->School] )
        return;

    if( sp->NameHash == SPELL_HASH_DRAIN_LIFE && m_caster->HasDummyAura(SPELL_HASH_DEATH_S_EMBRACE) && m_caster->GetHealthPct() <= 20 )
        amount *= 1.3f;

    amount = m_caster->GetSpellBonusDamage(m_target, sp, mod->i, amount, false);

    uint32 Amount = std::min(amount, m_target->GetUInt32Value( UNIT_FIELD_HEALTH ));

    SendPeriodicAuraLog(m_casterGuid, m_target, sp, Amount, -1, 0, (uint32)FLAG_PERIODIC_DAMAGE);

    //deal damage before we add healing bonus to damage
    m_caster->DealDamage(m_target, Amount, 0, 0, sp->Id, true);
    if(sp)
    {
        float coef = sp->EffectValueMultiplier[mod->i]; // how much health is restored per damage dealt
        m_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, sp->SpellGroupType);
        m_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, sp->SpellGroupType);
        Amount = float2int32((float)Amount * coef);
    }

    uint32 newHealth = float2int32(m_caster->GetUInt32Value(UNIT_FIELD_HEALTH) + Amount);

    uint32 mh = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(newHealth <= mh)
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, newHealth);
    else
        m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, mh);

    SendPeriodicAuraLog(m_casterGuid, m_caster, sp, Amount, -1, 0, (uint32)FLAG_PERIODIC_HEAL);
}

void Aura::SpellAuraModHitChance(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    int32 amt = apply ? mod->m_amount : -mod->m_amount;
    castPtr<Player>( m_target )->SetHitFromMeleeSpell( castPtr<Player>( m_target )->GetHitFromMeleeSpell() + amt);
    if( castPtr<Player>( m_target )->GetHitFromMeleeSpell() < 0 ) castPtr<Player>( m_target )->SetHitFromMeleeSpell( 0 );
}

void Aura::SpellAuraModSpellHitChance(bool apply)
{
    if (m_target->IsPlayer())
    {
        if (apply)
        {
            castPtr<Player>( m_target )->SetHitFromSpell(castPtr<Player>( m_target )->GetHitFromSpell() + mod->m_amount);
        }
        else
        {
            castPtr<Player>( m_target )->SetHitFromSpell(castPtr<Player>( m_target )->GetHitFromSpell() - mod->m_amount);
            if(castPtr<Player>( m_target )->GetHitFromSpell() < 0)
            {
                castPtr<Player>( m_target )->SetHitFromSpell(0);
            }
        }
    }
}

void Aura::SpellAuraTransform(bool apply)
{
    uint32 displayId = 0;
    CreatureData* data = sCreatureDataMgr.GetCreatureData(mod->m_miscValue[0]);

    if(data == NULL)
        sLog.Debug("Aura","SpellAuraTransform cannot find CreatureData for id %d",mod->m_miscValue[0]);
    else displayId = data->DisplayInfo[0];

    if( m_target->IsPlayer() && castPtr<Player>(m_target)->IsMounted() )
        m_target->Dismount();

    Unit * m_caster = GetUnitCaster();
    switch( m_spellProto->Id )
    {
        case 47585: // Dispersion
        {
            if( apply && m_caster != NULL )
            {
                SpellEntry *spellInfo = dbcSpell.LookupEntry( 60069 );
                if(!spellInfo)
                    return;

                Spell* spell = new Spell(m_target, spellInfo ,true, NULL);
                SpellCastTargets targets(m_target->GetGUID());
                spell->prepare(&targets);
            }
        }break;
        case 20584://wisp
        {
            m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, apply ? 10045:m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }break;

        case 30167: // Red Ogre Costume
        {
            if( apply )
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, 11549 );
            else
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }break;

        case 41301: // Time-Lost Figurine
        {
            if( apply )
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, 18628 );
            else
                m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
        }break;

        case 16739: // Orb of Deception
        {
            if( apply )
            {
                switch(m_target->getRace())
                {
                    case RACE_ORC:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10139);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10140);
                    }break;
                    case RACE_TAUREN:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10136);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10147);
                    }break;
                    case RACE_TROLL:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10135);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10134);
                    }break;
                    case RACE_UNDEAD:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10146);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10145);
                    }break;
                    case RACE_BLOODELF:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17829);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17830);
                    }break;
                    case RACE_GNOME:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10148);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10149);
                    }break;
                    case RACE_DWARF:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10141);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10142);
                    }break;
                    case RACE_HUMAN:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10137);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10138);
                    }break;
                    case RACE_NIGHTELF:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10143);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 10144);
                    }break;
                    case RACE_DRAENEI:
                    {
                        if( m_target->getGender() == 0 )
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17827);
                        else
                            m_target->SetUInt32Value(UNIT_FIELD_DISPLAYID, 17828);
                    }break;
                    default:
                        m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
                }
            }
        }break;

        case 42365: // murloc costume
            m_target->SetUInt32Value( UNIT_FIELD_DISPLAYID, apply ? 21723 : m_target->GetUInt32Value( UNIT_FIELD_NATIVEDISPLAYID ) );
            break;

        case 19937:
            {
                if (apply)
                {
                    // TODO: Sniff the spell / item, we need to know the real displayID
                    // guessed this may not be correct
                    // human = 7820
                    // dwarf = 7819
                    // halfling = 7818
                    // maybe 7842 as its from a lesser npc
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, 7842);
                }
                else
                {
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
                }
            }break;

        default:
        {
            if(!displayId) return;

            if (apply)
                {
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, displayId);
                }
                else
                {
                    m_target->SetUInt32Value (UNIT_FIELD_DISPLAYID, m_target->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
                }
        }break;
    };

    m_target->EventModelChange();
}

void Aura::SpellAuraModSpellCritChance(bool apply)
{
    if (m_target->IsPlayer())
    {
        int32 amt = apply ? mod->m_amount : -mod->m_amount;
        m_target->spellcritperc += amt;
        castPtr<Player>( m_target )->SetSpellCritFromSpell( castPtr<Player>( m_target )->GetSpellCritFromSpell() + amt );
    }
}

void Aura::SpellAuraIncreaseSwimSpeed(bool apply)
{

}

void Aura::SpellAuraModCratureDmgDone(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            for(uint32 x = 0; x < 11; x++)
                if(mod->m_miscValue[0] & (((uint32)1)<<x))
                    castPtr<Player>( m_target )->IncreaseDamageByType[x+1] += mod->m_amount;
        }
        else
            for(uint32 x = 0; x < 11; x++)
                if(mod->m_miscValue[0] & (((uint32)1)<<x) )
                    castPtr<Player>( m_target )->IncreaseDamageByType[x+1] -= mod->m_amount;
    }
}

void Aura::SpellAuraPacifySilence(bool apply)
{
    // Can't Attack or Cast Spells
    if(apply)
    {
        m_target->m_pacified++;
        m_target->m_silenced++;
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);

        if(m_target->GetCurrentSpell() && m_target->GetGUID() != m_casterGuid &&
            m_target->GetCurrentSpell()->getState() == SPELL_STATE_CASTING )
            {
                m_target->GetCurrentSpell()->cancel();
                m_target->SetCurrentSpell(NULL);
            }
    }
    else
    {
        m_target->m_pacified--;
        if(m_target->m_pacified == 0)
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);

        m_target->m_silenced--;
        if(m_target->m_silenced == 0)
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
    }
}

void Aura::SpellAuraModScale(bool apply)
{
    float current = m_target->GetFloatValue(OBJECT_FIELD_SCALE_X);
    float delta = mod->m_amount/100.0f;

    m_target->SetFloatValue(OBJECT_FIELD_SCALE_X, apply ? (current+current*delta) : current/(1.0f+delta));
}

void Aura::SpellAuraPeriodicHealthFunnel(bool apply)
{

}

void Aura::EventPeriodicHealthFunnel(uint32 amount)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL  || m_target == NULL || !m_target->isAlive() || !m_caster->isAlive())
        return;
    if(m_target->isAlive() && m_caster->isAlive())
    {

        m_caster->DealDamage(m_target, amount, 0, 0, GetSpellId(),true);
        uint32 newHealth = m_caster->GetUInt32Value(UNIT_FIELD_HEALTH) + 1000;

        uint32 mh = m_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
        if(newHealth <= mh)
            m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, newHealth);
        else
            m_caster->SetUInt32Value(UNIT_FIELD_HEALTH, mh);

        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, amount, -1, 0, (uint32)FLAG_PERIODIC_LEECH);
    }
}

void Aura::SpellAuraPeriodicManaLeech(bool apply)
{

}

void Aura::EventPeriodicManaLeech(uint32 amount)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL  || m_target == NULL || !m_target->isAlive() || !m_caster->isAlive())
        return;

    int32 amt = amount;

    // Drained amount should be reduced by resilence
    if(m_target->IsPlayer())
    {
        float amt_reduction_pct = 2.2f * castPtr<Player>(m_target)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_RESILIENCE ) / 100.0f;
        if( amt_reduction_pct > 0.33f ) amt_reduction_pct = 0.33f; // 3.0.3
        amt = float2int32( amt - (amt * amt_reduction_pct) );
    }

    float coef = m_spellProto->EffectValueMultiplier[mod->i] > 0 ? m_spellProto->EffectValueMultiplier[mod->i] : 1; // how much mana is restored per mana leeched
    m_caster->SM_FFValue(SMT_MULTIPLE_VALUE, &coef, m_spellProto->SpellGroupType);
    m_caster->SM_PFValue(SMT_MULTIPLE_VALUE, &coef, m_spellProto->SpellGroupType);
    amt = float2int32((float)amt * coef);

    uint32 cm = m_caster->GetPower(POWER_TYPE_MANA) + amt;
    uint32 mm = m_caster->GetMaxPower(POWER_TYPE_MANA);
    if(cm <= mm)
    {
        m_caster->SetPower(POWER_TYPE_MANA, cm);
        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, amt, 0, 0, FLAG_PERIODIC_LEECH);
    }
    else
    {
        m_caster->SetPower(POWER_TYPE_MANA, mm);
        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, mm - cm, 0, 0, FLAG_PERIODIC_LEECH);
    }

    m_caster->SendPowerUpdate();
}

void Aura::SpellAuraModCastingSpeed(bool apply)
{

}

void Aura::SpellAuraFeignDeath(bool apply)
{
    if( m_target->IsPlayer() )
    {
        Player* pTarget = castPtr<Player>( m_target );
        WorldPacket data(50);
        if( apply )
        {
            pTarget->EventAttackStop();
            pTarget->SetFlag( UNIT_FIELD_FLAGS_2, 1 );
            pTarget->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH );
            //pTarget->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD );
            pTarget->SetFlag( UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD );
            //pTarget->SetUInt32Value( UNIT_NPC_EMOTESTATE, EMOTE_STATE_DEAD );

            data.SetOpcode( SMSG_START_MIRROR_TIMER );
            data << uint32( 2 );        // type
            data << int32( GetDuration() );
            data << int32( GetDuration() );
            data << uint32( 0xFFFFFFFF );
            data << uint8( 0 );
            data << uint32( m_spellProto->Id );     // ???
            pTarget->GetSession()->SendPacket( &data );

            data.Initialize(SMSG_CLEAR_TARGET);
            data << pTarget->GetGUID();

            //now get rid of mobs agro. pTarget->CombatStatus.AttackersForgetHate() - this works only for already attacking mobs
            WorldObject::InRangeUnitSet::iterator itr, itr2;
            for(itr = pTarget->GetInRangeUnitSetBegin(); itr != pTarget->GetInRangeUnitSetEnd();)
            {
                itr2 = itr++;
                WorldObject* pObject = (*itr2);

                if(pObject->IsUnit() && (castPtr<Unit>(pObject))->isAlive())
                {
                    if(pObject->GetTypeId()==TYPEID_UNIT)
                        (castPtr<Unit>( pObject ))->GetAIInterface()->RemoveThreat(pTarget->GetGUID());

                    //if this is player and targeting us then we interrupt cast
                    if( ( pObject )->IsPlayer() )
                    {   //if player has selection on us
                        if( castPtr<Player>( pObject )->GetSelection() == pTarget->GetGUID())
                        {
                            castPtr<Player>( (*itr) )->SetSelection(0); //lose selection
                            castPtr<Player>( (*itr) )->SetUInt64Value(UNIT_FIELD_TARGET, 0);

                            if( castPtr<Player>( pObject )->isCasting() && castPtr<Player>( pObject )->GetCurrentSpell() )
                                sEventMgr.AddEvent(castPtr<Unit>(pObject), &Unit::EventCancelSpell, castPtr<Player>( pObject )->GetCurrentSpell(), EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                        }

                        castPtr<Player>( pObject )->GetSession()->SendPacket( &data );
                    }
                }
            }
            pTarget->SetDeathState(ALIVE);
        }
        else
        {
            pTarget->RemoveFlag(UNIT_FIELD_FLAGS_2, 1);
            pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH);
            pTarget->RemoveFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD);
            //pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD);
            //pTarget->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
            data.SetOpcode(SMSG_STOP_MIRROR_TIMER);
            data << uint32(2);
            pTarget->GetSession()->SendPacket(&data);
        }
    }
}

void Aura::SpellAuraModDisarm(bool apply)
{
    if(apply)
    {
        if( m_target->IsPlayer() && castPtr<Player>(m_target)->IsInFeralForm())
            return;

        m_target->disarmed = true;
        m_target->disarmedShield = (m_spellProto->NameHash == SPELL_HASH_DISMANTLE);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    }
    else
    {
        m_target->disarmed = false;
        if( m_spellProto->NameHash == SPELL_HASH_DISMANTLE )
            m_target->disarmedShield = false;
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    }
}

void Aura::SpellAuraModStalked(bool apply)
{
    if(apply)
    {
        m_target->stalkedby = m_casterGuid;
        m_target->SetFlag(UNIT_DYNAMIC_FLAGS, 0x0002);
    }
    else
    {
        m_target->stalkedby = 0;
        m_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, 0x0002);
    }
}

void Aura::SpellAuraSchoolAbsorb(bool apply)
{
    if(apply)
        mod->fixed_amount = mod->m_amount;
}

void Aura::SpellAuraModSpellCritChanceSchool(bool apply)
{
    if(apply)
    {
        for(uint32 x=0;x<7;x++)
            if (mod->m_miscValue[0] & (((uint32)1)<<x))
                m_target->SpellCritChanceSchool[x] += mod->m_amount;
    }
    else
    {
        for(uint32 x=0;x<7;x++)
        {
            if (mod->m_miscValue[0] & (((uint32)1)<<x) )
            {
                m_target->SpellCritChanceSchool[x] -= mod->m_amount;
                /*if(m_target->SpellCritChanceSchool[x] < 0)
                    m_target->SpellCritChanceSchool[x] = 0;*/
            }
        }
    }
}

void Aura::SpellAuraModPowerCost(bool apply)
{
    int32 val = (apply) ? mod->m_amount : -mod->m_amount;
}

void Aura::SpellAuraModPowerCostSchool(bool apply)
{

}

void Aura::SpellAuraReflectSpellsSchool(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModLanguage(bool apply)
{
    if(apply)
        m_target->m_modlanguage = mod->m_miscValue[0];
    else m_target->m_modlanguage = -1;
}

void Aura::SpellAuraAddFarSight(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL || !m_caster->IsPlayer())
        return;

    if(apply)
    {
        float sightX = m_caster->GetPositionX() + 100.0f;
        float sightY = m_caster->GetPositionY() + 100.0f;
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, mod->m_miscValue[0]);
        m_caster->GetMapMgr()->ChangeFarsightLocation(castPtr<Player>(m_caster), sightX, sightY, true);

    }
    else
    {
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, 0);
        m_caster->GetMapMgr()->ChangeFarsightLocation(castPtr<Player>(m_caster), 0, 0, false);
    }
}

void Aura::SpellAuraMechanicImmunity(bool apply)
{
    if( m_target->IsPlayer())
    {
        switch(m_spellProto->Id)
        {
        case 49039:
            {
                if(apply && !m_target->HasAura(50397))
                    GetUnitCaster()->CastSpell(m_target,50397,true);
            }
        }
    }

    if(apply)
    {
        if(mod->m_miscValue[0] != MECHANIC_HEALING && mod->m_miscValue[0] != MECHANIC_INVULNARABLE && mod->m_miscValue[0] != MECHANIC_SHIELDED) // dont remove bandages, Power Word and protection effect
        {
            /* Supa's test run of Unit::RemoveAllAurasByMechanic */
            if( m_target ) // just to be sure?
            {
                m_target->m_AuraInterface.RemoveAllAurasByMechanic( (uint32)mod->m_miscValue[0] , -1 , false );
            }

            if(m_spellProto->Id == 42292 || m_spellProto->Id == 59752)  // PvP Trinket
            {
                // insignia of the A/H
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_CHARMED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_DISORIENTED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_FLEEING, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ROOTED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_PACIFIED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ASLEEP, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_STUNNED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_INCAPACIPATED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_POLYMORPHED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_SEDUCED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_FROZEN, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_ENSNARED, -1, false);
                m_target->m_AuraInterface.RemoveAllAurasByMechanic(MECHANIC_BANISHED, -1, false);
            }
        }
    }
}

void Aura::SpellAuraMounted(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    Player* pPlayer = castPtr<Player>(m_target);

    //Remove any previous mount if we had one
    if(pPlayer->IsMounted())
        pPlayer->Dismount();

    if(pPlayer->InStealth())
    {
        uint32 id = m_target->m_stealth;
        pPlayer->m_stealth = 0;
        pPlayer->RemoveAura(id);
    }

    bool warlockpet = false;
    if(pPlayer->GetSummon() && pPlayer->GetSummon()->IsWarlockPet() == true)
        warlockpet = true;

    if(apply)
    {
        pPlayer->m_bgFlagIneligible++;

        //Dismiss any pets
        if(pPlayer->GetSummon())
        {
            Pet* pPet = pPlayer->GetSummon();
            if((pPet->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0) && (warlockpet == false))
                pPet->Dismiss(false);               // Spell pet -> Dismiss
            else
            {
                pPet->Remove(false, true, true);    // hunter pet -> just remove for later re-call
                pPlayer->hasqueuedpet = true;
            }
        }

        if(pPlayer->m_bg)
            pPlayer->m_bg->HookOnMount(pPlayer);
        TRIGGER_INSTANCE_EVENT( pPlayer->GetMapMgr(), OnPlayerMount )( pPlayer );

        m_target->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_MOUNT, GetSpellId());

        CreatureData* ctrData = sCreatureDataMgr.GetCreatureData(mod->m_miscValue[0]);
        if(ctrData == NULL || ctrData->DisplayInfo[0] == 0)
            return;

        m_target->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID, ctrData->DisplayInfo[0]);
        pPlayer->m_MountSpellId = m_spellProto->Id;
        pPlayer->m_FlyingAura = 0;

        if( pPlayer->GetShapeShift() && pPlayer->m_ShapeShifted != m_spellProto->Id && 
            !(pPlayer->GetShapeShift() & FORM_BATTLESTANCE | FORM_DEFENSIVESTANCE | FORM_BERSERKERSTANCE ))
            m_target->RemoveAura( pPlayer->m_ShapeShifted );
    }
    else
    {
        pPlayer->m_bgFlagIneligible--;
        pPlayer->m_MountSpellId = 0;
        pPlayer->m_FlyingAura = 0;
        m_target->SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);

        uint8 petnum = pPlayer->GetUnstabledPetNumber();
        if(warlockpet && !petnum)
            petnum = pPlayer->GetFirstPetNumber();

        if( petnum && pPlayer->hasqueuedpet )
        {
            //unstable selected pet
            PlayerPet * pPet = NULL;
            pPet = pPlayer->GetPlayerPet(petnum);
            if( pPlayer != NULL && pPet != NULL )
            {
                pPlayer->SpawnPet(petnum);
                if(!warlockpet)
                    pPet->stablestate = STABLE_STATE_ACTIVE;
            }
            pPlayer->hasqueuedpet = false;
        }
        pPlayer->m_AuraInterface.RemoveAllAurasByInterruptFlagButSkip( AURA_INTERRUPT_ON_DISMOUNT, GetSpellId() );
    }
}

void Aura::SpellAuraModDamageTakenPctPerCaster(bool apply)
{
    if(!m_target->IsUnit())
        return;

    if(apply)
    {
        m_target->DamageTakenPctModPerCaster.insert(std::make_pair(m_casterGuid, std::make_pair(m_spellProto->EffectSpellClassMask[mod->i], mod->m_amount)));
    } 
    else
    {
        Unit::DamageTakenPctModPerCasterType::iterator it = m_target->DamageTakenPctModPerCaster.find(m_casterGuid);
        while(it != m_target->DamageTakenPctModPerCaster.end() && it->first == m_casterGuid)
        {
            if(it->second.first == m_spellProto->EffectSpellClassMask[mod->i])
            {
                it = m_target->DamageTakenPctModPerCaster.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
}

void Aura::SpellAuraRequireNoAmmo(bool apply)
{
    if(!m_target->IsPlayer())
        return;
}

void Aura::SpellAuraModDamagePercDone(bool apply)
{
    SpecialCases();
    float val = (apply) ? mod->m_amount/100.0f : -mod->m_amount/100.0f;

    switch (GetSpellId()) //dirty or mb not fix bug with wand specializations
    {
    case 6057:
    case 6085:
    case 14524:
    case 14525:
    case 14526:
    case 14527:
    case 14528:
        return;
    }

    for(uint32 x=0;x<7;x++)
    {
        if (mod->m_miscValue[0] & (((uint32)1)<<x) )
        {
            m_target->DamageDonePctMod[x] += val;
        }
    }
}

void Aura::SpellAuraModPercStat(bool apply)
{

}

void Aura::SpellAuraSplitDamage(bool apply)
{
    DamageSplitTarget *ds = NULL;
    Unit * m_caster = GetUnitCaster();
    if( m_target == NULL || !m_target->IsUnit() || m_caster == NULL)
        return;

    ds = m_spellProto->Id == 19028 ? &m_caster->m_damageSplitTarget : &m_target->m_damageSplitTarget;

    if(apply)
    {
        ds->m_flatDamageSplit = 0;
        ds->m_spellId = m_spellProto->Id;
        ds->m_pctDamageSplit = mod->m_amount / 100.0f;
        ds->m_target = m_spellProto->Id == 19028 ? m_target->GetGUID() : m_casterGuid;
    }

    ds->active = apply;
}

void Aura::SpellAuraModRegen(bool apply)
{

}

void Aura::SpellAuraIgnoreEnemy(bool apply)
{
    if (!apply)
    {
        //when unapplied, target will switches to highest threat
        Unit* caster=GetUnitCaster();
        if (caster == NULL || !caster->isAlive() || !caster->IsCreature() || !caster->IsInWorld())
            return;

        caster->GetAIInterface()->SetNextTarget(caster->GetAIInterface()->GetMostHated());
    }
}

void Aura::SpellAuraDrinkNew(bool apply)
{

}

void Aura::EventPeriodicSpeedModify(int32 modifier)
{

}

void Aura::EventPeriodicDrink(uint32 amount)
{
    uint32 v = m_target->GetPower(POWER_TYPE_MANA) + amount;
    if( v > m_target->GetMaxPower(POWER_TYPE_MANA) )
        v = m_target->GetMaxPower(POWER_TYPE_MANA);
    m_target->SetPower(POWER_TYPE_MANA, v);
    SendPeriodicAuraLog(amount, FLAG_PERIODIC_ENERGIZE);
}

void Aura::EventPeriodicHeal1(uint32 amount)
{
    if(m_target == NULL )
        return;

    if(!m_target->isAlive())
        return;

    uint32 ch = m_target->GetUInt32Value(UNIT_FIELD_HEALTH);
    ch+=amount;
    uint32 mh = m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH);

    if(ch>mh)
        m_target->SetUInt32Value(UNIT_FIELD_HEALTH,mh);
    else m_target->SetUInt32Value(UNIT_FIELD_HEALTH,ch);

    if(m_spellProto->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
        m_target->Emote(EMOTE_ONESHOT_EAT);
    else if(!m_spellProto->IsSpellAnyArmorBuffType())
        SendPeriodicAuraLog(amount, FLAG_PERIODIC_HEAL);
}

void Aura::SpellAuraModPowerRegen(bool apply)
{

}

void Aura::SpellAuraChannelDeathItem(bool apply)
{
    if(apply)
    {
        //dont need for now
    }
    else
    {
        if(m_target->GetTypeId() == TYPEID_UNIT && castPtr<Creature>(m_target)->GetCreatureData())
        {
            if(castPtr<Creature>(m_target)->GetCreatureData()->Type != CRITTER)
            {
                if(m_target->isDead())
                {
                    Player* pCaster = m_target->GetMapMgr()->GetPlayer(m_casterGuid);
                    if(!pCaster)
                        return;

                    uint32 itemid = m_spellProto->EffectItemType[mod->i];
                    if(itemid == 6265 && int32(pCaster->getLevel() - m_target->getLevel()) > 9)
                        return;

                    ItemPrototype *proto = sItemMgr.LookupEntry(itemid);
                    if(pCaster->GetInventory()->CalculateFreeSlots(proto) > 0)
                    {
                        if(Item* item = objmgr.CreateItem(itemid,pCaster))
                        {
                            item->SetUInt64Value(ITEM_FIELD_CREATOR,pCaster->GetGUID());
                            if(!pCaster->GetInventory()->AddItemToFreeSlot(item))
                            {
                                pCaster->GetInventory()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                                item->Destruct();
                                return;
                            }

                            SlotResult * lr = pCaster->GetInventory()->LastSearchResult();
                            pCaster->GetSession()->SendItemPushResult(item,true,false,true,true,lr->ContainerSlot,lr->Slot,1);
                        }
                    }
                }
            }
        }
    }
}

void Aura::SpellAuraModDamagePercTaken(bool apply)
{

}

void Aura::SpellAuraModRegenPercent(bool apply)
{
    if( apply )
        m_target->PctRegenModifier += mod->m_amount;
    else
        m_target->PctRegenModifier -= mod->m_amount;
}

void Aura::SpellAuraPeriodicDamagePercent(bool apply)
{

}

void Aura::EventPeriodicDamagePercent(uint32 amount)
{
    //DOT
    if(!m_target->isAlive())
        return;
    if(m_target->SchoolImmunityList[m_spellProto->School])
        return;

    uint32 damage = m_target->GetModPUInt32Value(UNIT_FIELD_MAXHEALTH, amount);


    Unit * m_caster = GetUnitCaster();
    if(m_caster!=NULL)
        m_caster->SpellNonMeleeDamageLog(m_target, m_spellProto->Id, damage, m_triggeredSpellId==0, true);
    else m_target->SpellNonMeleeDamageLog(m_target, m_spellProto->Id, damage, m_triggeredSpellId==0, true);
}

void Aura::SpellAuraModResistChance(bool apply)
{
    apply ? m_target->m_resistChance = mod->m_amount : m_target->m_resistChance = 0;
}

void Aura::SpellAuraModDetectRange(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL)
        return;
    if(apply)
        m_caster->setDetectRangeMod(m_target->GetGUID(), mod->m_amount);
    else m_caster->unsetDetectRangeMod(m_target->GetGUID());
}

void Aura::SpellAuraPreventsFleeing(bool apply)
{
    // Curse of Recklessness
}

void Aura::SpellAuraModUnattackable(bool apply)
{
/*
        Also known as Apply Aura: Mod Uninteractible
        Used by: Spirit of Redemption, Divine Intervention, Phase Shift, Flask of Petrification
        It uses one of the UNIT_FIELD_FLAGS, either UNIT_FLAG_NOT_SELECTABLE or UNIT_FLAG_NOT_ATTACKABLE_2
*/
}

void Aura::SpellAuraInterruptRegen(bool apply)
{
    if(apply)
        m_target->m_interruptRegen++;
    else
    {
        m_target->m_interruptRegen--;
        if(m_target->m_interruptRegen < 0)
            m_target->m_interruptRegen = 0;
    }
}

void Aura::SpellAuraGhost(bool apply)
{
    if(m_target->IsPlayer())
    {
        SpellAuraWaterWalk( apply );
        m_target->m_invisible = apply;
    }
    //m_target->SendPowerUpdate();
}

void Aura::SpellAuraMagnet(bool apply)
{
    if( !m_target->IsPlayer() )
        return;

    // grounding totem
    // redirects one negative aura to the totem

    if( apply )
        castPtr<Player>(m_target)->m_magnetAura = this;
    else
        castPtr<Player>(m_target)->m_magnetAura = NULL;
}

void Aura::SpellAuraManaShield(bool apply)
{
    if(apply)
    {
        m_target->m_manashieldamt = mod->m_amount ;
        m_target->m_manaShieldSpell = m_spellProto;
    }
    else
    {
        if(m_target->m_manaShieldSpell->Id == m_spellProto->Id)
        {
            m_target->m_manashieldamt = 0;
            m_target->m_manaShieldSpell = 0;
        }
    }
}

void Aura::SpellAuraSkillTalent(bool apply)
{
    if (m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->_ModifySkillBonus(mod->m_miscValue[0],mod->m_amount);
        else castPtr<Player>( m_target )->_ModifySkillBonus(mod->m_miscValue[0],-mod->m_amount);
    }
}

void Aura::SpellAuraModAttackPower(bool apply)
{
    m_target->ModUnsigned32Value(UNIT_FIELD_ATTACK_POWER_MOD_POS,apply? mod->m_amount : -mod->m_amount);
}

void Aura::SpellAuraVisible(bool apply)
{

}

void Aura::SpellAuraModCreatureAttackPower(bool apply)
{

}

void Aura::SpellAuraModTotalThreat( bool apply )
{
    if( apply )
    {
        if( m_target->GetThreatModifier() > mod->m_amount ) // replace old mod
        {
            m_target->ModThreatModifier( -m_target->GetThreatModifier() );
            m_target->ModThreatModifier( mod->m_amount );
        }
    }
    else
    {
        if( m_target->GetThreatModifier() == mod->m_amount ) // only remove it if it hasn't been replaced yet
        {
            m_target->ModThreatModifier(-(mod->m_amount));
        }
    }
}

void Aura::SpellAuraWaterWalk( bool apply )
{

}

void Aura::SpellAuraFeatherFall( bool apply )
{

}

void Aura::SpellAuraHover( bool apply )
{

}

void Aura::SpellAuraAddFlatModifier(bool apply)
{

}

void Aura::SpellAuraAddPctMod( bool apply )
{

}

void Aura::SpellAuraAddTargetTrigger(bool apply)
{

}

void Aura::SpellAuraModPowerRegPerc(bool apply)
{
    if(apply)
    {
        mod->fixed_float_amount = m_target->PctPowerRegenModifier[mod->m_miscValue[0]] * ((float)(mod->m_amount))/100.0f;
        m_target->PctPowerRegenModifier[mod->m_miscValue[0]] += mod->fixed_float_amount;
    }else m_target->PctPowerRegenModifier[mod->m_miscValue[0]] -= mod->fixed_float_amount;
}

void Aura::SpellAuraOverrideClassScripts(bool apply)
{

}

void Aura::SpellAuraModRangedDamageTaken(bool apply)
{

}

void Aura::SpellAuraModHealing(bool apply)
{
    m_target->HealTakenMod += apply ? mod->m_amount : -mod->m_amount;
}

void Aura::SpellAuraIgnoreRegenInterrupt(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    if(apply)
        castPtr<Player>( m_target )->PctIgnoreRegenModifier += ((float)(mod->m_amount))/100;
    else
        castPtr<Player>( m_target )->PctIgnoreRegenModifier -= ((float)(mod->m_amount))/100;
}

void Aura::SpellAuraModMechanicResistance(bool apply)
{

}

void Aura::SpellAuraModHealingPCT(bool apply)
{
    if( apply )
        m_target->HealTakenPctMod *= (mod->m_amount/100.0f + 1);
    else m_target->HealTakenPctMod /= (mod->m_amount/100.0f + 1);
}

void Aura::SpellAuraModRangedAttackPower(bool apply)
{
    if(apply)
        m_target->ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS,mod->m_amount);
    else m_target->ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS,-mod->m_amount);
}

void Aura::SpellAuraModMeleeDamageTaken(bool apply)
{

}

void Aura::SpellAuraModMeleeDamageTakenPct(bool apply)
{

}

void Aura::SpellAuraRAPAttackerBonus(bool apply)
{
    if(apply)
        m_target->RAPvModifier += mod->m_amount;
    else m_target->RAPvModifier -= mod->m_amount;
}

void Aura::SpellAuraModPossessPet(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL || !m_caster->IsPlayer())
        return;

    if(castPtr<Player>(m_caster)->GetSummon() != m_target)
        return;

    if(apply)
    {
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, m_target->GetGUID());
        m_target->SetFlag(UNIT_FIELD_FLAGS, 0x01000000);
    }
    else
    {
        m_caster->SetUInt64Value(PLAYER_FARSIGHT, 0);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, 0x01000000);
    }
}

void Aura::SpellAuraModIncreaseSpeedAlways(bool apply)
{

}

void Aura::SpellAuraModIncreaseEnergyPerc( bool apply )
{

}

void Aura::SpellAuraModIncreaseHealthPerc( bool apply )
{

}

void Aura::SpellAuraModManaRegInterrupt( bool apply )
{

}

void Aura::SpellAuraModTotalStatPerc(bool apply)
{

}

void Aura::SpellAuraModHaste( bool apply )
{
    //blade flurry - attack a nearby opponent
    if( m_spellProto->NameHash == SPELL_HASH_BLADE_FLURRY )
    {
        if( apply )
            m_target->AddExtraStrikeTarget(m_spellProto, mod->i, 0);
        else m_target->RemoveExtraStrikeTarget(m_spellProto);
    }
}

void Aura::SpellAuraForceReaction( bool apply )
{
    if( !m_target->IsPlayer() )
        return;

    std::map<uint32,uint32>::iterator itr;
    Player* p_target = castPtr<Player>(m_target);

    if( apply )
    {
        itr = p_target->m_forcedReactions.find( mod->m_miscValue[0] );
        if( itr != p_target->m_forcedReactions.end() )
            itr->second = mod->m_amount;
        else
            p_target->m_forcedReactions.insert( std::make_pair( mod->m_miscValue[0], mod->m_amount ) );
    }
    else
        p_target->m_forcedReactions.erase( mod->m_miscValue[0] );

    WorldPacket data( SMSG_SET_FORCED_REACTIONS, ( 8 * p_target->m_forcedReactions.size() ) + 4 );
    data << uint32(p_target->m_forcedReactions.size());
    for( itr = p_target->m_forcedReactions.begin(); itr != p_target->m_forcedReactions.end(); itr++ )
    {
        data << itr->first;
        data << itr->second;
    }

    p_target->GetSession()->SendPacket( &data );
}

void Aura::SpellAuraModRangedHaste(bool apply)
{

}

void Aura::SpellAuraModRangedAmmoHaste(bool apply)
{

}

void Aura::SpellAuraRetainComboPoints(bool apply)
{
    if( m_target->IsPlayer() )
        castPtr<Player>( m_target )->m_retainComboPoints = apply;
}

void Aura::SpellAuraResistPushback(bool apply)
{
    //DK:This is resist for spell casting delay
    //Only use on players for now

    if(m_target->IsPlayer())
    {
        int32 val = apply ? mod->m_amount : -mod->m_amount;
        for(uint32 x=0;x<7;x++)
        {
            if (mod->m_miscValue[0] & (((uint32)1)<<x) )
            {
                castPtr<Player>( m_target )->SpellDelayResist[x] += val;
            }
        }
    }
}

void Aura::SpellAuraModShieldBlockPCT( bool apply )
{
    if( m_target->IsPlayer() )
    {
        if( apply )
            castPtr<Player>(m_target)->m_modblockabsorbvalue += ( uint32 )mod->m_amount;
        else
            castPtr<Player>(m_target)->m_modblockabsorbvalue -= ( uint32 )mod->m_amount;
    }
}

void Aura::SpellAuraTrackStealthed(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster== NULL || !m_caster->IsPlayer() )
        return;

    //0x00000002 is track stealthed
    if( apply )
        m_caster->SetFlag(PLAYER_FIELD_BYTES, 0x00000002);
    else m_caster->RemoveFlag(PLAYER_FIELD_BYTES, 0x00000002);
}

void Aura::SpellAuraModDetectedRange(bool apply)
{
    if(!m_target->IsPlayer())
        return;
    if(apply)
        castPtr<Player>( m_target )->DetectedRange += mod->m_amount;
    else castPtr<Player>( m_target )->DetectedRange -= mod->m_amount;
}

void Aura::SpellAuraSplitDamageFlat(bool apply)
{
    DamageSplitTarget *ds;

    Unit * m_caster = GetUnitCaster();
    if( m_target == NULL || !m_target->IsUnit() || m_caster == NULL )
        return;

    ds = &m_caster->m_damageSplitTarget;
    if(apply)
    {
        ds->m_flatDamageSplit = mod->m_miscValue[0];
        ds->m_spellId = m_spellProto->Id;
        ds->m_pctDamageSplit = 0;
        ds->m_target = m_casterGuid;
//      printf("registering dmg split %u, amout= %u \n",ds->m_spellId, mod->m_amount, mod->m_miscValue[0], mod->m_type);
    }

    ds->active = apply;
}

void Aura::SpellAuraModStealthLevel(bool apply)
{
    if(apply)
        m_target->m_stealthLevel += mod->m_amount;
    else m_target->m_stealthLevel -= mod->m_amount;
}

void Aura::SpellAuraModUnderwaterBreathing(bool apply)
{
    if(m_target->IsPlayer())
    {
        uint32 m_UnderwaterMaxTimeSaved = castPtr<Player>( m_target )->m_UnderwaterMaxTime;
        if( apply )
            castPtr<Player>( m_target )->m_UnderwaterMaxTime *= (1 + mod->m_amount / 100 );
        else
            castPtr<Player>( m_target )->m_UnderwaterMaxTime /= (1 + mod->m_amount / 100 );
        castPtr<Player>( m_target )->m_UnderwaterTime *= castPtr<Player>( m_target )->m_UnderwaterMaxTime / m_UnderwaterMaxTimeSaved;
    }
}

void Aura::SpellAuraSafeFall(bool apply)
{

}

void Aura::SpellAuraModReputationAdjust(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->pctReputationMod += mod->m_amount;
        else castPtr<Player>( m_target )->pctReputationMod -= mod->m_amount;
    }
}

void Aura::SpellAuraNoPVPCredit(bool apply)
{

}

void Aura::SpellAuraModHealthRegInCombat(bool apply)
{

}

void Aura::EventPeriodicBurn(uint32 amount, uint32 misc)
{
    Unit * m_caster = GetUnitCaster();
    if( m_caster == NULL)
        return;

    if(m_target->isAlive() && m_caster->isAlive())
    {
        if(m_target->SchoolImmunityList[m_spellProto->School])
            return;

        uint32 Amount = std::min( amount, m_target->GetPower(misc) );
        SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, Amount, 0, 0, FLAG_PERIODIC_DAMAGE);
        m_target->DealDamage(m_target, Amount, 0, 0, m_spellProto->Id);
    }
}

void Aura::SpellAuraPowerBurn(bool apply)
{

}

void Aura::SpellAuraModCritDmgPhysical(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            castPtr<Player>( m_target )->m_modphyscritdmgPCT += (uint32)mod->m_amount;
        }
        else
        {
            castPtr<Player>( m_target )->m_modphyscritdmgPCT -= (uint32)mod->m_amount;
        }
    }
}


void Aura::SpellAuraWaterBreathing( bool apply )
{
    if( !m_target->IsPlayer() )
        return;

    castPtr<Player>( m_target )->m_bUnlimitedBreath = apply;
}

void Aura::SpellAuraAPAttackerBonus(bool apply)
{
    if(apply)
        m_target->APvModifier += mod->m_amount;
    else
        m_target->APvModifier -= mod->m_amount;
}


void Aura::SpellAuraModPAttackPower(bool apply)
{
    //!!probably there is a flag or something that will signal if randeg or melee attack power !!! (still missing)
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            m_target->ModFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER,(float)mod->m_amount/100);
        }
        else
            m_target->ModFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER,-(float)mod->m_amount/100);
    }
}

void Aura::SpellAuraModRangedAttackPowerPct(bool apply)
{
    if(m_target->IsPlayer())
    {
        m_target->ModFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER,((apply)?1:-1)*(float)mod->m_amount/100);
    }
}

void Aura::SpellAuraIncreaseDamageTypePCT(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            for(uint32 x = 0; x < 11; x++)
                if (mod->m_miscValue[0] & (((uint32)1)<<x) )
                    castPtr<Player>( m_target )->IncreaseDamageByTypePCT[x+1] += ((float)(mod->m_amount))/100;
        }
        else
        {
            for(uint32 x = 0; x < 11; x++)
            {
                if (mod->m_miscValue[0] & (((uint32)1)<<x) )
                    castPtr<Player>( m_target )->IncreaseDamageByTypePCT[x+1] -= ((float)(mod->m_amount))/100;
            }
        }
    }
}

void Aura::SpellAuraIncreaseCricticalTypePCT(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
        {
            for(uint32 x = 0; x < 11; x++)
                if (mod->m_miscValue[0] & (((uint32)1)<<x) )
                    castPtr<Player>( m_target )->IncreaseCricticalByTypePCT[x+1] += ((float)(mod->m_amount))/100;
        }
        else
        {
            for(uint32 x = 0; x < 11; x++)
            {
                if (mod->m_miscValue[0] & (((uint32)1)<<x) )
                    castPtr<Player>( m_target )->IncreaseCricticalByTypePCT[x+1] -= ((float)(mod->m_amount))/100;
            }
        }
    }
}

void Aura::SpellAuraIncreasePartySpeed(bool apply)
{

}

void Aura::SpellAuraIncreaseSpellDamageByAttribute(bool apply)
{

}

void Aura::SpellAuraIncreaseHealingByAttribute(bool apply)
{

}

void Aura::SpellAuraModHealingDone(bool apply)
{
    int32 val = apply ? mod->m_amount : -mod->m_amount;
    uint32 player_class = m_target->getClass();
    if(player_class == DRUID || player_class == PALADIN || player_class == SHAMAN || player_class == PRIEST)
        val = float2int32(val * 1.88f);

    m_target->HealDoneBase += val;
}

void Aura::SpellAuraModHealingDonePct(bool apply)
{
    SpecialCases();
    if( apply )
        m_target->HealDonePctMod *= (mod->m_amount/100.0f + 1);
    else m_target->HealDonePctMod /= (mod->m_amount/100.0f + 1);
}

void Aura::SpellAuraEmphaty(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if(m_caster == NULL || m_target == NULL || !m_caster->IsPlayer())
        return;

    // Show extra info about beast
    uint32 dynflags = m_target->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
    if(apply)
        dynflags |= U_DYN_FLAG_PLAYER_INFO;

    m_target->BuildFieldUpdatePacket(castPtr<Player>(m_caster), UNIT_DYNAMIC_FLAGS, dynflags);
}

void Aura::SpellAuraUntrackable(bool apply)
{
    if(apply)
        m_target->SetFlag(UNIT_FIELD_BYTES_1, 0x04000000);
    else
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, 0x04000000);
}

void Aura::SpellAuraModOffhandDamagePCT(bool apply)
{
    //Used only by talents of rogue and warrior;passive,positive
    if(m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>( m_target )->offhand_dmg_mod *= (100+mod->m_amount)/100.0f;
        else castPtr<Player>( m_target )->offhand_dmg_mod /= (100+mod->m_amount)/100.0f;
    }
}

void Aura::SpellAuraModPenetration(bool apply)
{

}

void Aura::SpellAuraIncreaseArmorByPctInt(bool apply)
{

}

void Aura::SpellAuraReduceAttackerMHitChance(bool apply)
{
    if (!m_target->IsPlayer())
        return;
    if(apply)
        castPtr<Player>( m_target )->m_resist_hit[0]+=mod->m_amount;
    else castPtr<Player>( m_target )->m_resist_hit[0]-=mod->m_amount;
}

void Aura::SpellAuraReduceAttackerRHitChance(bool apply)
{
    if (!m_target->IsPlayer())
        return;
    if(apply)
        castPtr<Player>( m_target )->m_resist_hit[1]+=mod->m_amount;
    else castPtr<Player>( m_target )->m_resist_hit[1]-=mod->m_amount;
}

void Aura::SpellAuraReduceAttackerSHitChance(bool apply)
{
    if (!m_target->IsPlayer())
        return;
    if(apply)
        castPtr<Player>( m_target )->m_resist_hit[2]-=mod->m_amount;
    else castPtr<Player>( m_target )->m_resist_hit[2]+=mod->m_amount;
}

void Aura::SpellAuraReduceEnemyMCritChance(bool apply)
{
    if(!m_target->IsPlayer())
        return;
    if(apply) // value is negative percent
        castPtr<Player>( m_target )->res_M_crit_set(castPtr<Player>( m_target )->res_M_crit_get()+mod->m_amount);
    else castPtr<Player>( m_target )->res_M_crit_set(castPtr<Player>( m_target )->res_M_crit_get()-mod->m_amount);
}

void Aura::SpellAuraReduceEnemyRCritChance(bool apply)
{
    if(!m_target->IsPlayer())
        return;
    if(apply) // value is negative percent
        castPtr<Player>( m_target )->res_R_crit_set(castPtr<Player>( m_target )->res_R_crit_get()+mod->m_amount);
    else castPtr<Player>( m_target )->res_R_crit_set(castPtr<Player>( m_target )->res_R_crit_get()-mod->m_amount);
}

void Aura::SpellAuraUseNormalMovementSpeed( bool apply )
{

}

void Aura::SpellAuraIncreaseTimeBetweenAttacksPCT(bool apply)
{

}

void Aura::SpellAuraModAttackerCritChance(bool apply)
{
    int32 val  = (apply) ? mod->m_amount : -mod->m_amount;
    m_target->AttackerCritChanceMod[0] += val;
}

void Aura::SpellAuraIncreaseAllWeaponSkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if(apply)
        {
//          castPtr<Player>( m_target )->ModSkillBonusType(SKILL_TYPE_WEAPON, mod->m_amount);
            //since the frikkin above line does not work we have to do it manually
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_SWORDS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_AXES, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_BOWS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_GUNS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_MACES, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_2H_SWORDS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_STAVES, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_2H_MACES, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_2H_AXES, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_DAGGERS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_CROSSBOWS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_SPEARS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_WANDS, mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_POLEARMS, mod->m_amount);
        }
        else
        {
//          castPtr<Player>( m_target )->ModSkillBonusType(SKILL_TYPE_WEAPON, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_SWORDS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_AXES, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_BOWS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_GUNS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_MACES, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_2H_SWORDS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_STAVES, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_2H_MACES, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_2H_AXES, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_DAGGERS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_CROSSBOWS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_SPEARS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_WANDS, -mod->m_amount);
            castPtr<Player>( m_target )->_ModifySkillBonus(SKILL_POLEARMS, -mod->m_amount);
        }
    }
}

void Aura::SpellAuraIncreaseHitRate( bool apply )
{
    if( !m_target->IsPlayer() )
        return;

}

void Aura::SpellAuraModMobKillXPGain( bool apply )
{
    if( !m_target->IsPlayer() )
        return;
    if( apply )
        castPtr<Player>( m_target )->MobXPGainRate += GetSpellProto()->EffectBasePoints[0]+1;
    else castPtr<Player>( m_target )->MobXPGainRate -= GetSpellProto()->EffectBasePoints[0]+1;
    if(castPtr<Player>( m_target )->MobXPGainRate <= (float)0.0f)
        castPtr<Player>( m_target )->MobXPGainRate = (float)0.0f;
}


void Aura::SpellAuraIncreaseRageFromDamageDealtPCT(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    castPtr<Player>( m_target )->rageFromDamageDealt += (apply) ? mod->m_amount : -mod->m_amount;
}

void Aura::SpellAuraNoReagentCost(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    castPtr<Player>( m_target )->NoReagentCost = apply;
}

void Aura::SpellAuraReduceCritMeleeAttackDmg(bool apply)
{

}

void Aura::SpellAuraReduceCritRangedAttackDmg(bool apply)
{

}

void Aura::SpellAuraEnableFlight(bool apply)
{

}

void Aura::SpellAuraEnableFlightWithUnmountedSpeed(bool apply)
{

}

void Aura::SpellAuraIncreaseMovementAndMountedSpeed( bool apply )
{

}

void Aura::SpellAuraIncreaseFlightSpeed( bool apply )
{

}

void Aura::SpellAuraIncreaseRating( bool apply )
{

}

void Aura::EventPeriodicRegenManaStatPct(uint32 perc, uint32 stat)
{
    if(m_target->isDead())
        return;

    uint32 spellId = m_triggeredSpellId ? m_triggeredSpellId : (m_spellProto ? m_spellProto->Id : 0);

    m_target->Energize(m_target, spellId, (m_target->GetUInt32Value(UNIT_FIELD_STATS + stat) * perc)/100, POWER_TYPE_MANA);
}


void Aura::SpellAuraRegenManaStatPCT(bool apply)
{

}

void Aura::SpellAuraSpellHealingStatPCT(bool apply)
{

}

void Aura::SpellAuraFinishingMovesCannotBeDodged(bool apply)
{
    if( !m_target->IsPlayer() )
        return;
    castPtr<Player>( m_target )->m_finishingmovesdodge = apply;
}

void Aura::SpellAuraAuraModInvisibilityDetection(bool apply)
{
    if( apply )
        m_target->m_stealthDetectBonus += mod->m_amount;
    else
        m_target->m_stealthDetectBonus -= mod->m_amount;
}

void Aura::SpellAuraIncreaseMaxHealth(bool apply)
{
    //should only be used by a player
    //and only ever target players
    if( !m_target->IsPlayer() )
        return;

    int32 amount;
    if( apply )
        amount = mod->m_amount;
    else
        amount = -mod->m_amount;
}

void Aura::SpellAuraSpiritOfRedemption(bool apply)
{
    if(!m_target->IsPlayer())
        return;

    if(apply)
    {
        //m_target->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);
        m_target->SetUInt32Value(UNIT_FIELD_HEALTH, 1);
        SpellEntry * sorInfo = dbcSpell.LookupEntry(27792);
        if(!sorInfo)
            return;
        Spell* sor = new Spell(m_target, sorInfo, true, NULL);
        SpellCastTargets targets;
        targets.m_unitTarget = m_target->GetGUID();
        sor->prepare(&targets);
    }
    else
    {
        //m_target->SetFloatValue(OBJECT_FIELD_SCALE_X, 1);
        m_target->RemoveAura(27792);
        m_target->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
    }
}

void Aura::SpellAuraDispelDebuffResist(bool apply)
{

}

void Aura::SpellAuraIncreaseAttackerSpellCrit(bool apply)
{
    int32 val = apply ? mod->m_amount : -mod->m_amount;
    for( uint32 x = 0; x < 7; x++ )
    {
        if( mod->m_miscValue[0] & (((uint32)1) << x) )
            m_target->AttackerCritChanceMod[x] += val;
    }
}

void Aura::SpellAuraIncreaseRepGainPct(bool apply)
{
    if(m_target->IsPlayer())
    {
        if(apply)
            castPtr<Player>(m_target)->pctReputationMod += mod->m_amount;//re use
        else castPtr<Player>(m_target)->pctReputationMod -= mod->m_amount;//re use
    }
}

void Aura::SpellAuraIncreaseRangedAPStatPCT(bool apply)
{

}

void Aura::SpellAuraModBlockValue(bool apply)
{
    if( m_target->IsPlayer())
    {
        int32 amt = apply ? mod->m_amount : -mod->m_amount;
        castPtr<Player>(m_target)->m_modblockvaluefromspells += amt;
    }
}

// Looks like it should make spells skip some can cast checks. Atm only affects TargetAuraState check
void Aura::SpellAuraSkipCanCastCheck(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if (m_caster == NULL || !m_target->IsPlayer())
        return;

    // Generic
    if(apply)
    {
        for(uint32 x=0;x<3;x++)
            castPtr<Player>(m_target)->m_skipCastCheck[x] |= m_spellProto->EffectSpellClassMask[mod->i][x];
    }
    else
        for(uint32 x=0;x<3;x++)
            castPtr<Player>(m_target)->m_skipCastCheck[x] &= ~m_spellProto->EffectSpellClassMask[mod->i][x];
}

void Aura::SpellAuraCastFilter(bool apply)
{
    // cannot perform any abilities (other than those in EffectMask)
    if (!m_target->IsPlayer())
        return; // only for players

    // Generic
    if(apply)
    {
        castPtr<Player>(m_target)->SetFlag(PLAYER_FLAGS,PLAYER_FLAG_ALLOW_ONLY_ABILITY);
        for(uint32 x=0;x<3;x++)
        {
            castPtr<Player>(m_target)->m_castFilter[x] |= m_spellProto->EffectSpellClassMask[mod->i][x];
        }
    }
    else
    {
        castPtr<Player>(m_target)->RemoveFlag(PLAYER_FLAGS,PLAYER_FLAG_ALLOW_ONLY_ABILITY);
        for(uint32 x=0;x<3;x++)
        {
            castPtr<Player>(m_target)->m_castFilter[x] &= ~m_spellProto->EffectSpellClassMask[mod->i][x];
        }
    }
}

void Aura::SendInterrupted(uint8 result, WorldObject* m_ocaster)
{
    if( !m_ocaster->IsInWorld() )
        return;

    WorldPacket data( SMSG_SPELL_FAILURE, 20 );
    if( m_ocaster->IsPlayer() )
    {
        data << m_ocaster->GetGUID();
        data << uint8(0); //extra_cast_number
        data << uint32(m_spellProto->Id);
        data << uint8( result );
        castPtr<Player>( m_ocaster )->GetSession()->SendPacket( &data );
    }

    data.Initialize( SMSG_SPELL_FAILED_OTHER );
    data << m_ocaster->GetGUID();
    data << uint8(0); //extra_cast_number
    data << uint32(m_spellProto->Id);
    data << uint8( result );
    m_ocaster->SendMessageToSet( &data, false );

    m_interrupted = (int16)result;
}

void Aura::SendChannelUpdate(uint32 time, WorldObject* m_ocaster)
{
    WorldPacket data(MSG_CHANNEL_UPDATE, 18);
    data << m_ocaster->GetGUID();
    data << time;

    m_ocaster->SendMessageToSet(&data, true);
}

void Aura::SpellAuraExpertise(bool apply)
{

}

void Aura::SpellAuraForceMoveFoward(bool apply)
{
    if(m_target == NULL || !m_target->IsPlayer())
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
}

void Aura::SpellAuraModFaction(bool apply)
{
    if(m_target == NULL)
        return;

    if(apply)
    {
        m_target->SetFaction(GetSpellProto()->EffectMiscValue[mod->i]);
        if(m_target->IsPlayer())
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    }
    else
    {
        m_target->ResetFaction();
        if(m_target->IsPlayer())
            m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    }
}

void Aura::SpellAuraComprehendLanguage(bool apply)
{
    if(m_target == NULL || !m_target->IsPlayer())
        return;

    if(apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
}

void Aura::SendPeriodicAuraLog(uint32 amt, uint32 Flags)
{
    SendPeriodicAuraLog(m_casterGuid, m_target, m_spellProto, amt, 0, 0, Flags, m_triggeredSpellId);
}

void Aura::SendPeriodicAuraLog(uint64 CasterGuid, Unit* Target, SpellEntry *sp, uint32 Amount, int32 abs_dmg, uint32 resisted_damage, uint32 Flags, uint32 pSpellId, bool crit)
{
    if(Target == NULL || !Target->IsInWorld())
        return;

    uint32 spellId = pSpellId ? pSpellId : sp->Id;
    uint8 isCritical = crit ? 1 : 0;
    if(abs_dmg == -1)
        abs_dmg = Amount;

    WorldPacket data(SMSG_PERIODICAURALOG, 46);
    data << Target->GetGUID();       // target guid
    data << WoWGuid(CasterGuid).asPacked(); // caster guid
    data << uint32(spellId);            // spellid
    data << uint32(1);                  // count of logs going next
    data << uint32(Flags);              // Log type
    switch(Flags)
    {
    case FLAG_PERIODIC_DAMAGE:
        {
            data << uint32(Amount);
            data << uint32(0);
            data << uint32(SchoolMask(sp->School));
            data << uint32(abs_dmg);
            data << uint32(resisted_damage);
            data << uint8(isCritical);
        }break;
    case FLAG_PERIODIC_HEAL:
        {
            data << uint32(Amount);
            data << uint32(resisted_damage);
            data << uint32(abs_dmg);
            data << uint8(isCritical);
        }break;
    case FLAG_PERIODIC_ENERGIZE:
    case FLAG_PERIODIC_LEECH:
        {
            data << uint32(sp->EffectMiscValue[mod->i]);
            data << uint32(Amount);
            if(Flags == FLAG_PERIODIC_LEECH)
                data << float(sp->EffectValueMultiplier[mod->i]);
        }break;
    default:
        {
            printf("Unknown type!");
            return;
        }break;
    }
    Target->SendMessageToSet(&data, true);
}

void Aura::AttemptDispel(Unit* pCaster, bool canResist)
{
    m_dispelled = true;
    Remove();
}

void Aura::SpellAuraModIgnoreArmorPct(bool apply)
{
    if( m_spellProto->NameHash == SPELL_HASH_MACE_SPECIALIZATION )
    {
        if(apply)
            m_target->m_ignoreArmorPctMaceSpec += (mod->m_amount / 100.0f);
        else
            m_target->m_ignoreArmorPctMaceSpec -= (mod->m_amount / 100.0f);
        return;
    }

    if(apply)
        m_target->m_ignoreArmorPct += (mod->m_amount / 100.0f);
    else m_target->m_ignoreArmorPct -= (mod->m_amount / 100.0f);
}

void Aura::SpellAuraSetPhase(bool apply)
{

}

void Aura::SpellAuraIncreaseAPByAttribute(bool apply)
{

}

void Aura::SpellAuraModSpellDamageFromAP(bool apply)
{

}

void Aura::SpellAuraModSpellHealingFromAP(bool apply)
{

}

void Aura::SpellAuraProcTriggerWithValue(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraVehiclePassenger(bool apply)
{
    if(!GetUnitCaster() || !GetUnitCaster()->IsVehicle())
        return;
    if(apply)
        castPtr<Vehicle>(GetCaster())->AddPassenger(m_target,GetSpellProto()->EffectMiscValue[mod->i],true);
    else castPtr<Vehicle>(GetCaster())->RemovePassenger(m_target);
}

void Aura::SpellAuraReduceEffectDuration(bool apply)
{

}


void Aura::SpellAuraNoReagent(bool apply)
{
    if( !m_target->IsPlayer() )
        return;

    uint32 ClassMask[3] = {0,0,0};
    for(uint32 x=0;x<3;x++)
        ClassMask[x] |= m_target->GetUInt32Value(PLAYER_NO_REAGENT_COST_1+x);

    for(uint32 x=0;x<3;x++)
    {
        if(apply)
            ClassMask[x] |= m_spellProto->EffectSpellClassMask[mod->i][x];
        else ClassMask[x] &= ~m_spellProto->EffectSpellClassMask[mod->i][x];
    }

    for(uint32 x=0;x<3;x++)
        m_target->SetUInt32Value(PLAYER_NO_REAGENT_COST_1+x, ClassMask[x]);
}

void Aura::SpellAuraModBaseHealth(bool apply)
{

}

uint8 Aura::GetMaxProcCharges(Unit* caster)
{
    uint32 charges = m_spellProto->procCharges;
    if(caster)
    {
        caster->SM_FIValue(SMT_CHARGES, (int32*)&charges, m_spellProto->SpellGroupType);
        caster->SM_PIValue(SMT_CHARGES, (int32*)&charges, m_spellProto->SpellGroupType);
    }
    return uint8(charges & 0xFF);
}

void Aura::RecalculateModBaseAmounts()
{
    Unit *unitCaster = GetUnitCaster(), *unitTarget = GetUnitTarget();
    Player *playerCaster = unitCaster ? unitCaster->IsPlayer() ? castPtr<Player>(unitCaster) : NULL : NULL;
    uint32 casterLevel = unitCaster ? unitCaster->getLevel() : 0, casterComboPoints = playerCaster ? playerCaster->m_comboPoints : 0;
    for(uint32 i = 0; i < m_modcount; i++)
    {
        int32 value = m_spellProto->CalculateSpellPoints(m_modList[i].i, casterLevel, casterComboPoints);
        if( playerCaster != NULL )
        {
            SpellOverrideMap::iterator itr = playerCaster->mSpellOverrideMap.find(GetSpellProto()->Id);
            if(itr != playerCaster->mSpellOverrideMap.end())
            {
                ScriptOverrideList::iterator itrSO;
                for(itrSO = itr->second->begin(); itrSO != itr->second->end(); itrSO++)
                    value += RandomUInt((*itrSO)->damage);
            }
        }

        if( unitCaster != NULL )
        {
            int32 spell_flat_modifers = 0, spell_pct_modifers = 0;
            unitCaster->SM_FIValue(SMT_MISC_EFFECT, &spell_flat_modifers,GetSpellProto()->SpellGroupType);
            unitCaster->SM_FIValue(SMT_MISC_EFFECT, &spell_pct_modifers, GetSpellProto()->SpellGroupType);

            if( m_modList[i].i == 0 )
            {
                unitCaster->SM_FIValue(SMT_FIRST_EFFECT_BONUS,&spell_flat_modifers,GetSpellProto()->SpellGroupType);
                unitCaster->SM_FIValue(SMT_FIRST_EFFECT_BONUS,&spell_pct_modifers,GetSpellProto()->SpellGroupType);
            }
            else if( m_modList[i].i == 1 )
            {
                unitCaster->SM_FIValue(SMT_SECOND_EFFECT_BONUS,&spell_flat_modifers,GetSpellProto()->SpellGroupType);
                unitCaster->SM_FIValue(SMT_SECOND_EFFECT_BONUS,&spell_pct_modifers,GetSpellProto()->SpellGroupType);
            }

            if( ( m_modList[i].i == 2 ) || ( m_modList[i].i == 1 && GetSpellProto()->Effect[2] == 0 ) || ( m_modList[i].i == 0 && GetSpellProto()->Effect[1] == 0 && GetSpellProto()->Effect[2] == 0 ) )
            {
                unitCaster->SM_FIValue(SMT_LAST_EFFECT_BONUS,&spell_flat_modifers,GetSpellProto()->SpellGroupType);
                unitCaster->SM_FIValue(SMT_LAST_EFFECT_BONUS,&spell_pct_modifers,GetSpellProto()->SpellGroupType);
            }
            value += float2int32(value * (float)(spell_pct_modifers / 100.0f)) + spell_flat_modifers;
        }

        m_modList[i].m_baseAmount = value;
    }
    UpdateModAmounts();
}

void Aura::UpdateModAmounts()
{
    Unit * m_caster = GetUnitCaster();
    for(uint8 i = 0; i < m_modcount; i++)
    {
        if( m_modList[i].m_bonusAmount == 0) CalculateBonusAmount(m_caster, i);
        if(m_modList[i].m_baseAmount >= 0)
            m_modList[i].m_amount = m_modList[i].m_baseAmount+m_modList[i].m_bonusAmount;
        else m_modList[i].m_amount = m_modList[i].m_baseAmount-m_modList[i].m_bonusAmount;
        if(m_stackSizeorProcCharges >= 0) m_modList[i].m_amount *= m_stackSizeorProcCharges;
        if(m_target) m_target->m_AuraInterface.SetModMaskBit(m_modList[i].m_type);
    }
}

void Aura::CalculateBonusAmount(Unit *caster, uint8 index)
{
    if(index >= m_modcount || index >= 3)
        return;
    if(caster == NULL || m_target == NULL)
        return;

    m_modList[index].m_bonusAmount = caster->GetSpellBonusDamage(m_target, m_spellProto, index, m_modList[index].m_baseAmount, m_spellProto->isSpellHealingEffect());
    if(m_modList[index].m_bonusAmount > m_modList[index].m_baseAmount)
        m_modList[index].m_bonusAmount -= m_modList[index].m_baseAmount;
    else m_modList[index].m_bonusAmount = 0;
}

void Aura::AddStackSize(uint8 mod)
{
    if(mod == 0 || m_stackSizeorProcCharges < 0)
        return;

    uint16 maxStack = (m_spellProto->maxstack&0xFFFF);
    if( maxStack && m_target->IsPlayer() && castPtr<Player>(m_target)->stack_cheat )
        maxStack = 0xFF;
    if(maxStack && m_stackSizeorProcCharges == maxStack)
        return;

    int16 newStack = m_stackSizeorProcCharges + mod;
    if(maxStack && newStack > maxStack)
        newStack = maxStack;

    m_stackSizeorProcCharges = newStack;
    BuildAuraUpdate();

    // now need to update amount and reapply modifiers
    ApplyModifiers(false);
    UpdateModAmounts();
    ApplyModifiers(true);
}

void Aura::RemoveStackSize(uint8 mod)
{
    if(mod == 0 || m_stackSizeorProcCharges < 0)
        return;

    if(m_stackSizeorProcCharges > mod)
    {
        m_stackSizeorProcCharges -= mod;
        BuildAuraUpdate();
        return;
    }

    m_stackSizeorProcCharges = 0;
    Remove();
}

void Aura::SetProcCharges(uint8 mod)
{
    if(m_stackSizeorProcCharges > 0)
        return;

    if(mod == 0)
        m_stackSizeorProcCharges = 0;
    else
    {
        m_stackSizeorProcCharges = -(mod&0xFF);
        BuildAuraUpdate();
    }
}

void Aura::RemoveProcCharges(uint8 mod)
{
    if(mod == 0 || m_stackSizeorProcCharges > 0)
        return;

    if(m_stackSizeorProcCharges < mod)
    {
        m_stackSizeorProcCharges += mod;
        BuildAuraUpdate();
        return;
    }

    m_stackSizeorProcCharges = 0;
    Remove();
}

void Aura::SpellAuraModDamageTakenByMechPCT(bool apply)
{

}

void Aura::SpellAuraAllowTamePetType(bool apply)
{
    if(m_target->IsPlayer())
    {
        castPtr<Player>(m_target)->m_BeastMaster = apply;
    }
}

void Aura::SpellAuraAddCreatureImmunity(bool apply)
{

}

void Aura::SpellAuraRedirectThreat(bool apply)
{
    Unit * m_caster = GetUnitCaster();
    if( m_target == NULL || m_caster == NULL )
        return;


    if( !m_caster->IsPlayer() || m_caster->isDead() ||
        !(m_target->IsPlayer() || m_target->IsPet()) ||
        m_target->isDead() )
        return;

    //Unapply is handled via function
    if( apply )
    {
        if( m_spellProto->Id == 50720 )
            m_target->SetRedirectThreat(m_caster,0.1f, GetDuration());
        else
            m_target->SetRedirectThreat(m_caster,1.0f, GetDuration());
    }
}

void Aura::SpellAuraReduceAOEDamageTaken(bool apply)
{

}

void Aura::SpecialCases()
{
    //We put all the special cases here, so we keep the code clean.
    switch(m_spellProto->Id)
    {
    case 12976:// Last Stand
    case 50322:// Survival Instincts
        {
            mod->m_amount = (uint32)(m_target->GetUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.3);
        }break;
    case 23782:// Gift of Life
        {
            mod->m_amount = 1500;
        }break;
    case 48418:// Master Shapeshifter Physical Damage
    case 48420:// Master Shapeshifter CritChance
    case 48421:// Master Shapeshifter SpellDamage
    case 48422:// Master Shapeshifter Healing
        {
            if(castPtr<Player>(m_target)->HasSpell(48411))
                mod->m_amount =  2;
            if(castPtr<Player>(m_target)->HasSpell(48412))
                mod->m_amount =  4;
        }break;
    }
}

void Aura::SpellAuraHasteRanged(bool apply)
{

}

void Aura::SpellAuraModAttackPowerByArmor( bool apply )
{

}

void Aura::SpellAuraReflectInfront(bool apply)
{
    // Todo:PROC
}

void Aura::SpellAuraModPetTalentPoints(bool apply)
{
    if( !m_target->IsPlayer() )
        return;

    Unit* unit = castPtr<Unit>(m_target);
    if( unit )
        unit->ChangePetTalentPointModifier(apply);
    Player * player = castPtr<Player>(m_target);
    if(player && player->GetSummon()!= NULL)
        player->GetSummon()->InitTalentsForLevel();

}

void Aura::SpellAuraPeriodicTriggerSpellWithValue(bool apply)
{

}

void Aura::SpellAuraModCritChanceAll(bool apply)
{
    if( !m_target || !m_target->IsPlayer() )
        return;

    Player* plr = castPtr<Player>(m_target);

    if( apply )
    {
        WeaponModifier md;
        md.value = float(mod->m_amount);
        md.wclass = m_spellProto->EquippedItemClass;
        md.subclass = m_spellProto->EquippedItemSubClass;
        plr->tocritchance.insert( std::make_pair(GetSpellId(), md) );

        plr->SetSpellCritFromSpell( plr->GetSpellCritFromSpell() + float(mod->m_amount) );
    }
    else
    {
        plr->tocritchance.erase( GetSpellId() );
        plr->SetSpellCritFromSpell( plr->GetSpellCritFromSpell() - float(mod->m_amount) );
    }
}

void Aura::SpellAuraOpenStable(bool apply)
{
    if( !m_target || !m_target->IsPlayer() )
        return;

    Player* _player = castPtr<Player>(m_target);

    if(apply)
    {
        if( _player->getClass() == HUNTER)
        {
            WorldPacket data(10 + (_player->m_Pets.size() * 25));
            data.SetOpcode(MSG_LIST_STABLED_PETS);
            data << uint64(0);
            data << uint8(_player->m_Pets.size());
            data << uint8(_player->m_StableSlotCount);
            _player->PetLocks.Acquire();
            for(std::map<uint32, PlayerPet*>::iterator itr = _player->m_Pets.begin(); itr != _player->m_Pets.end(); ++itr)
            {
                data << uint32( itr->first );           // pet no
                data << uint32( itr->second->entry );   // entryid
                data << uint32( itr->second->level );   // level
                data << itr->second->name;          // name
                if( itr->second->stablestate == STABLE_STATE_ACTIVE )
                    data << uint8(STABLE_STATE_ACTIVE);
                else
                {
                    data << uint8(STABLE_STATE_PASSIVE + 1);
                }
            }
            _player->PetLocks.Release();
            _player->GetSession()->SendPacket(&data);
        }
    }
}

void Aura::SpellAuraFakeInebriation(bool apply)
{
    if( !m_target || !m_target->IsPlayer() )
        return;

    Player* plr = castPtr<Player>(m_target);

    if( apply )
    {
        plr->m_invisDetect[INVIS_FLAG_DRUNK] += mod->m_amount;
        plr->ModSignedInt32Value(PLAYER_FAKE_INEBRIATION, mod->m_amount);
    }
    else
    {
        plr->m_invisDetect[INVIS_FLAG_DRUNK] -= mod->m_amount;
        plr->ModSignedInt32Value(PLAYER_FAKE_INEBRIATION, -mod->m_amount);
    }
    plr->UpdateVisibility();
}

void Aura::SpellAuraPreventResurrection(bool apply)
{
    if( !m_target || !m_target->IsPlayer() )
        return;

    Player* plr = castPtr<Player>(m_target);

    if( apply )
        plr->PreventRes = true;
    else plr->PreventRes = false;
}

void Aura::SpellAuraHealAndJump(bool apply)
{

}

void Aura::SpellAuraConvertRune(bool apply)
{
    if( !m_target || !m_target->IsPlayer() )
        return;

    Player *plr = castPtr<Player>(m_target);

    if( plr->getClass() != DEATHKNIGHT )
        return;
    uint32 runes = mod->m_amount;
    if( apply )
    {
        for(uint32 j = 0; j < 6 && runes; ++j)
        {
            if((uint8)GetSpellProto()->EffectMiscValue[mod->i] != plr->GetRune(j))
                continue;
            plr->ConvertRune((uint8)j,(uint8)GetSpellProto()->EffectMiscValueB[mod->i]);
            break;
        }
    }
    else
    {
        for(uint32 j = 0; j < 6 && runes; ++j)
        {
            if((uint8)GetSpellProto()->EffectMiscValueB[mod->i] != plr->GetRune(j))
                continue;
            plr->ConvertRune((uint8)j, (uint8)GetSpellProto()->EffectMiscValue[mod->i]);
            break;
        }
    }
}

void Aura::SpellAuraModWalkSpeed(bool apply)
{

}
