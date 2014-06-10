/***
 * Demonstrike Core
 */

#include "StdAfx.h"

createFileSingleton(FactionSystem);

FactionSystem::FactionSystem()
{

}

FactionSystem::~FactionSystem()
{

}

bool FactionSystem::AC_GetAttackableStatus(Player *plr, Unit *target)
{
    return (intisAttackable(plr, target, false) == 0);
}

int FactionSystem::GetFactionsInteractStatus(Unit *unitA, Unit* unitB)
{
    if(unitA == NULL || unitB == NULL)
        return 0;
    FactionTemplateEntry *factionTemplateA = unitA->m_factionTemplate, *factionTemplateB = unitB->m_factionTemplate;
    FactionEntry *factionA = unitA->m_faction, *factionB = unitB->m_faction;
    if(factionTemplateA == NULL || factionTemplateB == NULL || factionA == NULL || factionB == NULL)
        return 0;
    if(factionTemplateA == factionTemplateB || factionA == factionB)
        return 0; // Same faction, we can skip the rest of the checks
    if(factionA->ID == 35 || factionB->ID == 35)
        return 0; // 35 is forced friendly to all

    // Check hostile masks
    if(factionTemplateA->HostileMask & factionTemplateB->FactionMask)
        return 1;
    if(factionTemplateB->HostileMask & factionTemplateA->FactionMask)
        return 1;

    // check friend/enemy list
    for(uint32 i = 0; i < 4; i++)
    {
        if(factionTemplateA->EnemyFactions[i] && factionTemplateA->EnemyFactions[i] == factionTemplateB->Faction)
            return 1;
        if(factionTemplateB->EnemyFactions[i] && factionTemplateB->EnemyFactions[i] == factionTemplateA->Faction)
            return 1;
    }

    // Reputation System Checks
    if(unitA->IsPlayer() && !unitB->IsPlayer())
    {
        if(factionB->RepListId >= 0)
            if(TO_PLAYER(unitA)->IsHostileBasedOnReputation(factionB))
                return 1;

        if(factionB->RepListId == -1 && factionTemplateB->HostileMask == 0 && factionTemplateB->FriendlyMask == 0)
            return 1;
    }
    else if(unitB->IsPlayer() && !unitA->IsPlayer())
    {
        if(factionA->RepListId >= 0)
            if(TO_PLAYER(unitB)->IsHostileBasedOnReputation(factionA))
                return 1;

        if(factionA->RepListId == -1 && factionTemplateA->HostileMask == 0 && factionTemplateA->FriendlyMask == 0)
            return 1;
    }

    return 0;
}

int FactionSystem::GetAreaInteractionStatus(Unit *unitA, Unit *unitB)
{
    // Do not let units attack each other in sanctuary
    if(unitA->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY) || unitB->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
    {
        bool allowedCombat = true;
        if(unitA->IsPlayer() && unitB->IsPlayer())
            allowedCombat = false;
        else if((unitA->IsPet() || unitA->IsSummon()) && unitB->IsPlayer())
            allowedCombat = false;
        else if((unitB->IsPet() || unitB->IsSummon()) && unitA->IsPlayer())
            allowedCombat = false;
        if(!allowedCombat)
            return 0;
    }
    return 1;
}

/// Where we check if we object A can attack object B. This is used in many feature's
/// Including the spell class, the player class, and the AI interface class.
int FactionSystem::intisAttackable(Object* objA, Object* objB, bool CheckStealth)// A can attack B?
{
    // can't attack self.. this causes problems with buffs if we don't have it :p
    if( !objA || !objB || objA == objB )
        return 0;

    if( !objA->IsInWorld() )
        return 0;

    // can't attack corpses neither...
    if( objA->GetTypeId() == TYPEID_CORPSE || objB->GetTypeId() == TYPEID_CORPSE )
        return 0;

    // Dead people can't attack anything.
    if( (objA->IsUnit() && !TO_UNIT(objA)->isAlive()) || (objB->IsUnit() && !TO_UNIT(objB)->isAlive()) )
        return 0;

    // Checks for untouchable, unattackable
    if( objA->IsUnit() && (objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) ||
        objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) || objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
        return 0;

    if( objB->IsUnit() && (objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) ||
        objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) || objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
        return 0;

    if(!objA->PhasedCanInteract(objB))
        return 0;

    // we cannot attack sheathed units. Maybe checked in other places too ?
    // !! warning, this presumes that objA is attacking ObjB
    if( CheckStealth && objB->IsUnit() && TO_UNIT(objB)->InStealth() )
        if(objA->CalcDistance(objB) > 5.0f)
            return 0;

    // Get players (or owners of pets/totems)
    Player* player_objA = GetPlayerFromObject(objA);
    Player* player_objB = GetPlayerFromObject(objB);
    if(objA->IsUnit() && objB->IsVehicle())
        if(TO_VEHICLE(objB)->GetPassengerSlot(TO_UNIT(objA)) != -1)
            return 0;
    else if(objB->IsUnit() && objA->IsVehicle())
        if(TO_VEHICLE(objA)->GetPassengerSlot(TO_UNIT(objB)) != -1)
            return 0;

    // Always kill critters
    if(!player_objB && objB->IsCreature() && TO_CREATURE(objB)->GetCreatureType() == CRITTER)
        if(player_objA)
            return 1;

    // Disable GM attacking.
    if(player_objA && player_objB && player_objA->bGMTagOn)
        return -1;

    // Disable GM attacking.
    if(player_objA && !player_objB && player_objA->bGMTagOn)
        return -1;

    // Don't allow players to attack GMs
    if(player_objA && player_objB && player_objB->bGMTagOn)
        return -1;

    // Creatures cannot attack a GM with tag on.
    if(!player_objA && player_objB && player_objB->bGMTagOn)
        return -1;

    if(objA->IsCreature() && isTargetDummy(objA->GetEntry()))
        return 0; // Bwahahaha

    if( player_objA && player_objB )
    {
        if(player_objA->DuelingWith == player_objB && player_objA->GetDuelState() == DUEL_STATE_STARTED )
            return 1;
    }
    else if(player_objA)
    {
        if(objB->IsPet() && TO_PET(objB)->GetOwner()->DuelingWith == player_objA)
            return 1;
        if(objB->IsSummon())
        {
            Object* summoner = TO_SUMMON(objB)->GetSummonOwner();
            if(summoner && summoner->IsPlayer())
            {
                if(TO_PLAYER(summoner)->DuelingWith == player_objA)
                    return 1;
            }
        }
    }
    else if(player_objB)
    {
        if(objA->IsPet() && TO_PET(objA)->GetOwner()->DuelingWith == player_objB)
            return 1;
        if(objA->IsSummon())
        {
            Object* summoner = TO_SUMMON(objA)->GetSummonOwner();
            if(summoner && summoner->IsPlayer())
            {
                if(TO_PLAYER(summoner)->DuelingWith == player_objB)
                    return 1;
            }
        }
    }

    // Do not let units attack each other in sanctuary
    if(objA->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY) || objB->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY) )
    {
        bool allowedCombat = true;
        if(objA->IsPlayer() && objB->IsPlayer())
            allowedCombat = false;
        else if(objA->IsPet() || objB->IsPet())
            allowedCombat = false;
        else if(objA->IsSummon() || objB->IsSummon())
            allowedCombat = false;
        else if(objA->m_factionTemplate == NULL || objB->m_factionTemplate == NULL || objA->m_factionTemplate == objB->m_factionTemplate)
            allowedCombat = false;
        else if(objA->m_factionTemplate->ID == 35 || objB->m_factionTemplate->ID == 35)
            allowedCombat = false;
        if(!allowedCombat)
            return 0;
    }

    if(objA->IsCreature())
    {
        if(objA->IsPet())
        {
            if(player_objB && !player_objB->IsPvPFlagged())
                return 0;

            if(player_objB)
            {
                if(TO_PET(objA)->GetOwner())
                {
                    if(!TO_PET(objA)->GetOwner()->IsPvPFlagged())
                        return 0;
                    // the target is PvP, its okay.
                }
                else
                    return 0;
            }
        }
        else if(objA->IsSummon())
        {
            if(player_objB && !player_objB->IsPvPFlagged())
                return 0;

            if(player_objB)
            {
                Object* summonownerA = TO_SUMMON(objA)->GetSummonOwner();
                if(summonownerA && summonownerA->IsPlayer())
                {
                    if(!TO_UNIT(summonownerA)->IsPvPFlagged())
                        return 0;
                    // the target is PvP, its okay.
                }
                else
                    return 0;
            }
        }
    }

    if( player_objA && player_objB )
    {
        if(player_objA->IsPvPFlagged() && !player_objB->IsPvPFlagged() && player_objA->DuelingWith != player_objB)
            return 0;
        if(!player_objA->IsPvPFlagged() && !player_objB->IsPvPFlagged() && player_objA->DuelingWith != player_objB)
            return 0;
        if(player_objA->IsFFAPvPFlagged() && player_objB->IsFFAPvPFlagged())
        {
            if( player_objA->GetGroup() && player_objA->GetGroup() == player_objB->GetGroup() )
                return 0;

            if( player_objA == player_objB ) // Totems...
                return 0;

            return 1;       // can hurt each other in FFA pvp
        }

        //Handle BG's
        if( player_objA->m_bg != NULL)
        {
            //Handle Arenas
            if( player_objA->GetTeam() != player_objB->GetTeam() )
                return 1;
        }

        // same faction can't kill each other.
        if(player_objA->m_faction == player_objB->m_faction)
            return 0;

        return 1; // Skip the rest of this, it's all faction shit.
    }

    return GetFactionsInteractStatus(TO_UNIT(objA), TO_UNIT(objB));
}

bool FactionSystem::CanEitherUnitAttack(Object* objA, Object* objB, bool CheckStealth)// A can attack B?
{
    // can't attack self.. this causes problems with buffs if we don't have it :p
    if( !objA || !objB || objA == objB )
        return false;

    if( !objA->IsInWorld() || !objB->IsInWorld() )
        return false;

    // can't attack corpses neither...
    if( objA->GetTypeId() == TYPEID_CORPSE || objB->GetTypeId() == TYPEID_CORPSE )
        return false;

    // Dead people can't attack anything.
    if( (objA->IsUnit() && !TO_UNIT(objA)->isAlive()) || (objB->IsUnit() && !TO_UNIT(objB)->isAlive()) )
        return false;

    // Checks for untouchable, unattackable
    if( objA->IsUnit() && (objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) ||
        objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) || objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
        return false;

    if( objB->IsUnit() && (objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) ||
        objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) || objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
        return false;

    if(!objA->PhasedCanInteract(objB) || !objB->PhasedCanInteract(objA))
        return false;

    // we cannot attack sheathed units. Maybe checked in other places too ?
    if(CheckStealth)
    {
        if( objB->IsUnit() && TO_UNIT(objB)->InStealth() )
            if(objA->CalcDistance(objB) > 5.0f)
                return false;
        if( objA->IsUnit() && TO_UNIT(objA)->InStealth() )
            if(objB->CalcDistance(objA) > 5.0f)
                return false;
    }

    // Get players (or owners of pets/totems)
    Player* player_objA = GetPlayerFromObject(objA);
    Player* player_objB = GetPlayerFromObject(objB);
    if(objA->IsUnit() && objB->IsVehicle())
        if(TO_VEHICLE(objB)->GetPassengerSlot(TO_UNIT(objA)) != -1)
            return false;
    if(objB->IsUnit() && objA->IsVehicle())
        if(TO_VEHICLE(objA)->GetPassengerSlot(TO_UNIT(objB)) != -1)
            return false;

    // Disable GM attacking.
    if(player_objA && player_objB && player_objA->bGMTagOn)
        return false;

    // Disable GM attacking.
    if(player_objA && !player_objB && player_objA->bGMTagOn)
        return false;

    // Don't allow players to attack GMs
    if(player_objA && player_objB && player_objB->bGMTagOn)
        return false;

    // Creatures cannot attack a GM with tag on.
    if(!player_objA && player_objB && player_objB->bGMTagOn)
        return false;

    if(objA->IsCreature() && isTargetDummy(objA->GetEntry()))
        return false; // Bwahahaha

    if( player_objA && player_objB )
    {
        if(player_objA->DuelingWith == player_objB && player_objA->GetDuelState() == DUEL_STATE_STARTED )
            return true;
    }
    else if(player_objA)
    {
        if(objB->IsPet() && TO_PET(objB)->GetOwner()->DuelingWith == player_objA)
            return true;
        if(objB->IsSummon())
        {
            Object* summoner = TO_SUMMON(objB)->GetSummonOwner();
            if(summoner && summoner->IsPlayer())
            {
                if(TO_PLAYER(summoner)->DuelingWith == player_objA)
                    return true;
            }
        }
    }
    else if(player_objB)
    {
        if(objA->IsPet() && TO_PET(objA)->GetOwner()->DuelingWith == player_objB)
            return true;
        if(objA->IsSummon())
        {
            Object* summoner = TO_SUMMON(objA)->GetSummonOwner();
            if(summoner && summoner->IsPlayer())
            {
                if(TO_PLAYER(summoner)->DuelingWith == player_objB)
                    return true;
            }
        }
    }
    else if(player_objA == NULL && player_objB == NULL) // Ignore players, we have critters in sanctuaries
    {
        // Do not let units attack each other in sanctuary
        // We know they aren't dueling
        if(objA->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY) || objB->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
        {
            if(objA->m_factionTemplate && objB->m_factionTemplate && objA->m_factionTemplate != objB->m_factionTemplate)
            {
                if(objA->m_factionTemplate->FactionMask == FACTION_MASK_MONSTER && objB->m_factionTemplate->FactionMask != FACTION_MASK_MONSTER)
                    return true;
                if(objB->m_factionTemplate->FactionMask == FACTION_MASK_MONSTER && objA->m_factionTemplate->FactionMask != FACTION_MASK_MONSTER)
                    return true;
            }
            return false;
        }
    }

    if(objA->IsCreature())
    {
        if(objA->IsPet())
        {
            if(player_objB && !player_objB->IsPvPFlagged())
                return false;

            if(player_objB)
            {
                if(TO_PET(objA)->GetOwner())
                {
                    if(!TO_PET(objA)->GetOwner()->IsPvPFlagged())
                        return false;
                    // the target is PvP, its okay.
                }
                else
                    return false;
            }
        }
        else if(objA->IsSummon())
        {
            if(player_objB && !player_objB->IsPvPFlagged())
                return false;

            if(player_objB)
            {
                Object* summonownerA = TO_SUMMON(objA)->GetSummonOwner();
                if(summonownerA && summonownerA->IsPlayer())
                {
                    if(!TO_UNIT(summonownerA)->IsPvPFlagged())
                        return false;
                    // the target is PvP, its okay.
                }
                else
                    return false;
            }
        }
    }

    if( player_objA && player_objB )
    {
        if(objA->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY) || objB->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
            return false;

        if(player_objA->IsPvPFlagged() && !player_objB->IsPvPFlagged() && player_objA->DuelingWith != player_objB)
            return false;
        if(!player_objA->IsPvPFlagged() && !player_objB->IsPvPFlagged() && player_objA->DuelingWith != player_objB)
            return false;

        if(player_objA->IsFFAPvPFlagged() && player_objB->IsFFAPvPFlagged())
        {
            if( player_objA->GetGroup() && player_objA->GetGroup() == player_objB->GetGroup() )
                return false;

            if( player_objA == player_objB ) // Totems...
                return false;

            return true;        // can hurt each other in FFA pvp
        }

        //Handle BG's
        if( player_objA->m_bg != NULL)
        {
            //Handle Arenas
            if( player_objA->GetTeam() != player_objB->GetTeam() )
                return true;
        }

        // same faction can't kill each other.
        if(player_objA->m_faction == player_objB->m_faction)
            return false;

        return true; // Skip the rest of this, it's all faction shit.
    }

    return (GetFactionsInteractStatus(TO_UNIT(objA), TO_UNIT(objB)) == 1);
}

bool FactionSystem::isAttackable(Object* objA, Object* objB, bool CheckStealth)// A can attack B?
{
    return (intisAttackable(objA, objB, CheckStealth) == 1);
}

bool FactionSystem::isHostile(Object* objA, Object* objB)// B is hostile for A?
{
    return isAttackable(objA, objB, false);
}

Player* FactionSystem::GetPlayerFromObject(Object* obj)
{
    Player* player_obj = NULLPLR;

    if( obj->IsPlayer() )
    {
        player_obj =  TO_PLAYER( obj );
    }
    else if( obj->IsPet() )
    {
        Pet* pet_obj = TO_PET(obj);
        if( pet_obj )
            player_obj =  pet_obj->GetPetOwner();
    }
    else if( obj->IsUnit() )
    {   // If it's not a player nor a pet, it can still be a totem.
        if(obj->IsSummon())
            player_obj =  TO_PLAYER(TO_SUMMON(obj)->GetSummonOwner());
    }
    return player_obj;
}

bool FactionSystem::isCombatSupport(Object* objA, Object* objB)// B combat supports A?
{
    if( !objA || !objB )
        return false;

    // can't support corpses...
    if( objA->GetTypeId() == TYPEID_CORPSE || objB->GetTypeId() == TYPEID_CORPSE )
        return false;

    // We do need all factiondata for this
    if( objB->m_faction == NULL || objA->m_faction == NULL || objB->m_faction == NULL || objA->m_faction == NULL )
        return false;

    if(!objA->PhasedCanInteract(objB))
        return false;

    if(objB->IsCreature() && isTargetDummy(objB->GetEntry()))
        return false;

    bool combatSupport = false;
    uint32 fSupport = objB->m_factionTemplate->FriendlyMask;
    uint32 myFaction = objA->m_factionTemplate->FactionMask;
    if(fSupport & myFaction)
        combatSupport = true;

    // check friend/enemy list
    for(uint32 i = 0; i < 4; i++)
    {
        if(objB->m_factionTemplate->FriendlyFactions[i] && objB->m_factionTemplate->FriendlyFactions[i] == objA->m_factionTemplate->Faction)
        {
            combatSupport = true;
            break;
        }
        if(objB->m_factionTemplate->EnemyFactions[i] && objB->m_factionTemplate->EnemyFactions[i] == objA->m_factionTemplate->Faction)
        {
            combatSupport = false;
            break;
        }
    }
    return combatSupport;
}

bool FactionSystem::isAlliance(Object* objA)// A is alliance?
{
    if(!objA || objA->m_faction == NULL || objA->m_faction == NULL)
        return true;

    //Get stormwind faction frm dbc (11/72)
    FactionTemplateEntry * m_sw_factionTemplate = dbcFactionTemplate.LookupEntry(11);
    FactionEntry * m_sw_faction = dbcFaction.LookupEntry(72);

    if(m_sw_factionTemplate == objA->m_factionTemplate || m_sw_faction == objA->m_faction)
        return true;

    //Is StormWind hostile to ObjectA?
    uint32 faction = m_sw_factionTemplate->Faction;
    uint32 hostilemask = objA->m_factionTemplate->HostileMask;

    if(faction & hostilemask)
        return false;

    // check friend/enemy list
    for(uint32 i = 0; i < 4; i++)
    {
        if(objA->m_factionTemplate->EnemyFactions[i] == faction)
            return false;
    }

    //Is ObjectA hostile to StormWind?
    faction = objA->m_factionTemplate->Faction;
    hostilemask = m_sw_factionTemplate->HostileMask;

    if(faction & hostilemask)
        return false;

    // check friend/enemy list
    for(uint32 i = 0; i < 4; i++)
    {
        if(objA->m_factionTemplate->EnemyFactions[i] == faction)
            return false;
    }

    //We're not hostile towards SW, so we are allied
    return true;
}
