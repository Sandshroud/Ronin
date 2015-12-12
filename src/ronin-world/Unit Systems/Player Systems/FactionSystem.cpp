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
    return (GetAttackableStatus(plr, target, false) == FI_STATUS_NONE);
}

FactionInteractionStatus FactionSystem::GetFactionsInteractStatus(WorldObject *objA, WorldObject *objB)
{
    FactionTemplateEntry *factionTemplateA = objA->GetFactionTemplate(), *factionTemplateB = objB->GetFactionTemplate();
    FactionEntry *factionA = objA->GetFaction(), *factionB = objB->GetFaction();
    if(factionTemplateA == NULL || factionTemplateB == NULL || factionA == NULL || factionB == NULL)
        return FI_STATUS_NONE;
    if(factionTemplateA == factionTemplateB || factionA == factionB)
        return FI_STATUS_FRIENDLY; // Same faction, we can skip the rest of the checks
    if(factionA->ID == 35 || factionB->ID == 35)
        return FI_STATUS_FRIENDLY; // 35 is forced friendly to all

    // Check friendly masks
    if(factionTemplateA->FriendlyMask & factionTemplateB->FactionMask)
        return FI_STATUS_FRIENDLY;
    if(factionTemplateB->FriendlyMask & factionTemplateA->FactionMask)
        return FI_STATUS_FRIENDLY;

    // Check hostile masks
    if(factionTemplateA->HostileMask & factionTemplateB->FactionMask)
        return FI_STATUS_HOSTILE;
    if(factionTemplateB->HostileMask & factionTemplateA->FactionMask)
        return FI_STATUS_HOSTILE;

    // Reputation System Checks
    if(objA->IsPlayer() && !objB->IsPlayer())
    {
        if(factionB->RepListId >= 0)
        {
            if(castPtr<Player>(objA)->IsHostileBasedOnReputation(factionB))
                return FI_STATUS_HOSTILE;
            if(castPtr<Player>(objA)->IsAtWar(factionB->ID))
                return FI_STATUS_NEUTRAL;
        }
    }
    else if(objB->IsPlayer() && !objA->IsPlayer())
    {
        if(factionB->RepListId >= 0)
        {
            if(castPtr<Player>(objB)->IsHostileBasedOnReputation(factionB))
                return FI_STATUS_HOSTILE;
            if(castPtr<Player>(objB)->IsAtWar(factionB->ID))
                return FI_STATUS_NEUTRAL;
        }
    }

    // check friend/enemy list
    for(uint8 i = 0; i < 4; i++)
    {
        if(factionTemplateA->EnemyFactions[i] && factionTemplateA->EnemyFactions[i] == factionTemplateB->Faction)
            return FI_STATUS_HOSTILE;
        if(factionTemplateB->EnemyFactions[i] && factionTemplateB->EnemyFactions[i] == factionTemplateA->Faction)
            return FI_STATUS_HOSTILE;
        if(factionTemplateB->FriendlyFactions[i] && factionTemplateB->FriendlyFactions[i] == factionTemplateA->Faction)
            return FI_STATUS_FRIENDLY;
        if(factionTemplateB->FriendlyFactions[i] && factionTemplateB->FriendlyFactions[i] == factionTemplateA->Faction)
            return FI_STATUS_FRIENDLY;
    }

    return FI_STATUS_NEUTRAL;
}

FactionInteractionStatus FactionSystem::GetUnitAreaInteractionStatus(Unit *unitA, Unit *unitB)
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
            return FI_STATUS_FRIENDLY;
    }
    return FI_STATUS_HOSTILE;
}

FactionInteractionStatus FactionSystem::GetPlayerAttackStatus(Player *plrA, Player *plrB)
{
    if(plrA->DuelingWith == plrB && plrA->GetDuelState() == DUEL_STATE_STARTED)
        return FI_STATUS_HOSTILE;
    if(plrB->IsFFAPvPFlagged())
        return (plrA->IsFFAPvPFlagged() ? FI_STATUS_HOSTILE : FI_STATUS_NEUTRAL);
    if(plrA->GetGroup() != plrB->GetGroup())
    {
        if(plrA->GetTeam() != plrB->GetTeam())
        {
            if(plrA->IsPvPFlagged() && plrB->IsPvPFlagged())
                return FI_STATUS_HOSTILE;
            return FI_STATUS_NEUTRAL;
        }
    }
    return FI_STATUS_FRIENDLY;
}

/// Where we check if we object A can attack object B. This is used in many feature's
/// Including the spell class, the player class, and the AI interface class.
FactionInteractionStatus FactionSystem::GetAttackableStatus(WorldObject* objA, WorldObject* objB, bool CheckStealth)// A can attack B?
{
    // can't attack self.. this causes problems with buffs if we don't have it :p
    if( !objA || !objB || objA == objB )
        return FI_STATUS_NONE;
    if( !objA->IsInWorld() || !objB->IsInWorld() )
        return FI_STATUS_NONE;
    // can't attack corpses either...
    if( objA->GetTypeId() == TYPEID_CORPSE || objB->GetTypeId() == TYPEID_CORPSE )
        return FI_STATUS_NONE;
    // Dead people can't attack anything.
    if( (objA->IsUnit() && !castPtr<Unit>(objA)->isAlive()) || (objB->IsUnit() && !castPtr<Unit>(objB)->isAlive()) )
        return FI_STATUS_NONE;
    // Checks for untouchable, unattackable
    if( objA->IsUnit() && (objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) ||
        objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) || objA->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
        return FI_STATUS_NONE;
    if( objB->IsUnit() && (objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9) ||
        objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI) || objB->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
        return FI_STATUS_NONE;
    if(!objA->PhasedCanInteract(objB))
        return FI_STATUS_NONE;

    // we cannot attack sheathed units. Maybe checked in other places too ?
    // !! warning, this presumes that objA is attacking ObjB
    if( CheckStealth && objB->IsUnit() /*&& castPtr<Unit>(objB)->InStealth()*/ )
        if(objA->CalcDistance(objB) > 5.0f)
            return FI_STATUS_FRIENDLY;
    // Get players (or owners of pets/totems)
    Player* player_objA = GetPlayerFromObject(objA);
    Player* player_objB = GetPlayerFromObject(objB);

    // Always kill critters
    if(!player_objB && objB->IsCreature() && castPtr<Creature>(objB)->GetCreatureType() == CRITTER)
        if(player_objA)
            return FI_STATUS_HOSTILE;
    // Disable GM attacking.
    if(player_objA && player_objB && player_objA->bGMTagOn)
        return FI_STATUS_NONE;
    // Don't allow players to attack GMs
    if(player_objA && player_objB && player_objB->bGMTagOn)
        return FI_STATUS_NONE;
    // Disable GMs attacking creatures.
    if(player_objA && !player_objB && player_objA->bGMTagOn)
        return FI_STATUS_NONE;
    // Creatures cannot attack a GM with tag on.
    if(!player_objA && player_objB && player_objB->bGMTagOn)
        return FI_STATUS_NONE;
    if(objA->IsCreature() && isTargetDummy(objA->GetEntry()))
        return FI_STATUS_FRIENDLY; // Bwahahaha

    if( player_objA && player_objB )
    {
        if(player_objA->DuelingWith == player_objB && player_objA->GetDuelState() == DUEL_STATE_STARTED )
            return FI_STATUS_HOSTILE;
    }
    else if(player_objA)
    {
        if(objB->IsPet() && castPtr<Pet>(objB)->GetOwner()->DuelingWith == player_objA)
            return FI_STATUS_HOSTILE;
        if(objB->IsSummon())
        {
            WorldObject* summoner = castPtr<Summon>(objB)->GetSummonOwner();
            if(summoner && summoner->IsPlayer())
            {
                if(castPtr<Player>(summoner)->DuelingWith == player_objA)
                    return FI_STATUS_HOSTILE;
            }
        }
    }
    else if(player_objB)
    {
        if(objA->IsPet() && castPtr<Pet>(objA)->GetOwner()->DuelingWith == player_objB)
            return FI_STATUS_HOSTILE;
        if(objA->IsSummon())
        {
            WorldObject* summoner = castPtr<Summon>(objA)->GetSummonOwner();
            if(summoner && summoner->IsPlayer())
            {
                if(castPtr<Player>(summoner)->DuelingWith == player_objB)
                    return FI_STATUS_HOSTILE;
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
        else if(objA->GetFactionTemplate() == NULL || objB->GetFactionTemplate() == NULL || objA->GetFactionTemplate() == objB->GetFactionTemplate())
            allowedCombat = false;
        else if(objA->GetFactionTemplate()->ID == 35 || objB->GetFactionTemplate()->ID == 35)
            allowedCombat = false;
        if(!allowedCombat)
            return FI_STATUS_FRIENDLY;
    }

    if(objA->IsCreature())
    {
        if(objA->IsPet())
        {
            if(player_objB && !player_objB->IsPvPFlagged())
                return FI_STATUS_FRIENDLY;

            if(player_objB)
            {
                if(castPtr<Pet>(objA)->GetOwner())
                {
                    if(!castPtr<Pet>(objA)->GetOwner()->IsPvPFlagged())
                        return FI_STATUS_FRIENDLY;
                    // the target is PvP, its okay.
                } else return FI_STATUS_FRIENDLY;
            }
        }
        else if(objA->IsSummon())
        {
            if(player_objB && !player_objB->IsPvPFlagged())
                return FI_STATUS_FRIENDLY;

            if(player_objB)
            {
                WorldObject* summonownerA = castPtr<Summon>(objA)->GetSummonOwner();
                if(summonownerA && summonownerA->IsPlayer())
                {
                    if(!castPtr<Unit>(summonownerA)->IsPvPFlagged())
                        return FI_STATUS_FRIENDLY;
                    // the target is PvP, its okay.
                } else return FI_STATUS_FRIENDLY;
            }
        }
    }

    if( player_objA && player_objB )
    {
        if(player_objA->IsPvPFlagged() && !player_objB->IsPvPFlagged() && player_objA->DuelingWith != player_objB)
            return FI_STATUS_FRIENDLY;
        if(!player_objA->IsPvPFlagged() && !player_objB->IsPvPFlagged() && player_objA->DuelingWith != player_objB)
            return FI_STATUS_FRIENDLY;
        if(player_objA->IsFFAPvPFlagged() && player_objB->IsFFAPvPFlagged())
        {
            if( player_objA->GetGroup() && player_objA->GetGroup() == player_objB->GetGroup() )
                return FI_STATUS_FRIENDLY;

            if( player_objA == player_objB ) // Totems...
                return FI_STATUS_FRIENDLY;

            return FI_STATUS_HOSTILE;       // can hurt each other in FFA pvp
        }

        //Handle BG's
        if( player_objA->m_bg != NULL)
        {
            //Handle Arenas
            if( player_objA->GetTeam() != player_objB->GetTeam() )
                return FI_STATUS_HOSTILE;
        }

        // same faction can't kill each other.
        if(player_objA->GetFaction() == player_objB->GetFaction())
            return FI_STATUS_FRIENDLY;

        return FI_STATUS_HOSTILE; // Skip the rest of this, it's all faction shit.
    }

    return GetFactionsInteractStatus(castPtr<Unit>(objA), castPtr<Unit>(objB));
}

bool FactionSystem::IsInteractionLocked(WorldObject *obj)
{
    if(obj == NULL || !obj->IsInWorld())
        return true;
    if(obj->GetTypeId() == TYPEID_CORPSE)
        return true;
    if(obj->GetFaction() == NULL)
        return true;

    if(obj->IsUnit())
    {
        Unit *uObj = castPtr<Unit>(obj);
        if(!uObj->isAlive())
            return true;
        else if(uObj->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9)
            || uObj->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI)
            || uObj->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            return true;
        else if(uObj->IsPlayer())
        {
            if(castPtr<Player>(uObj)->bGMTagOn)
                return true;
        }
        else //if(obj->IsCreature()) // No need to double check
        {

        }
    }

    return false;
}

bool FactionSystem::CanEitherUnitAttack(Unit *unitA, Unit *unitB, bool CheckStealth)// A can attack B?
{
    // can't attack self.. this causes problems with buffs if we don't have it :p
    if(IsInteractionLocked(unitA) || IsInteractionLocked(unitB))
        return false;
    if(!unitA->PhasedCanInteract(unitB) || !unitB->PhasedCanInteract(unitA))
        return false;
    // Only check target for stealth
    /*if( CheckStealth && unitB->InStealth() && unitA->CalcDistance(unitB) > 5.0f)
        return false;*/
    if(GetUnitAreaInteractionStatus(unitA, unitB) < FI_STATUS_NEUTRAL)
        return false;

    if(unitA->IsPlayer() && unitB->IsPlayer())
    {
        if(GetPlayerAttackStatus(castPtr<Player>(unitA), castPtr<Player>(unitB)) >= FI_STATUS_NEUTRAL)
            return true;
    }
    else if(unitA->IsPlayer())
    {
        Player *plrA = castPtr<Player>(unitA);
        if(unitB->IsPet() && castPtr<Pet>(unitB)->GetOwner()->DuelingWith == plrA)
            return true;
        else if(unitB->IsSummon())
        {
            WorldObject *summoner = castPtr<Summon>(unitB)->GetSummonOwner();
            if(summoner && summoner->IsPlayer() && castPtr<Player>(summoner)->DuelingWith == plrA)
                return true;
        }
    }
    else if(unitB->IsPlayer())
    {
        Player *plrB = castPtr<Player>(unitB);
        if(unitA->IsPet() && castPtr<Pet>(unitA)->GetOwner()->DuelingWith == plrB)
            return true;
        else if(unitA->IsSummon())
        {
            WorldObject *summoner = castPtr<Summon>(unitA)->GetSummonOwner();
            if(summoner && summoner->IsPlayer() && castPtr<Player>(summoner)->DuelingWith == plrB)
                return true;
        }
    }
    else /// Creature hate crimes
    { } // Factions can handle this

    return (GetFactionsInteractStatus(unitA, unitB) >= FI_STATUS_NEUTRAL);
}

bool FactionSystem::isAttackable(WorldObject* objA, WorldObject* objB, bool CheckStealth)// A can attack B?
{
    return (GetAttackableStatus(objA, objB, CheckStealth) >= FI_STATUS_NEUTRAL);
}

bool FactionSystem::isHostile(WorldObject* objA, WorldObject* objB)// B is hostile for A?
{
    return (GetAttackableStatus(objA, objB, true) == FI_STATUS_HOSTILE);
}

Player* FactionSystem::GetPlayerFromObject(WorldObject* obj)
{
    Player* player_obj = NULL;

    if( obj->IsPlayer() )
    {
        player_obj =  castPtr<Player>( obj );
    }
    else if( obj->IsPet() )
    {
        Pet* pet_obj = castPtr<Pet>(obj);
        if( pet_obj )
            player_obj =  pet_obj->GetPetOwner();
    }
    else if( obj->IsUnit() )
    {   // If it's not a player nor a pet, it can still be a totem.
        if(obj->IsSummon())
            player_obj =  castPtr<Player>(castPtr<Summon>(obj)->GetSummonOwner());
    }
    return player_obj;
}

bool FactionSystem::isCombatSupport(WorldObject* objA, WorldObject* objB)// B combat supports A?
{
    if( IsInteractionLocked(objA) || IsInteractionLocked(objB) )
        return false;

    // can't support corpses...
    if( objA->GetTypeId() == TYPEID_CORPSE || objB->GetTypeId() == TYPEID_CORPSE )
        return false;

    // We do need all factiondata for this
    if( objB->GetFactionTemplate() == NULL || objA->GetFactionTemplate() == NULL || objB->GetFaction() == NULL || objA->GetFaction() == NULL )
        return false;

    if(!objA->PhasedCanInteract(objB))
        return false;

    if(objB->IsCreature() && isTargetDummy(objB->GetEntry()))
        return false;

    bool combatSupport = false;
    uint32 fSupport = objB->GetFactionTemplate()->FriendlyMask;
    uint32 myFaction = objA->GetFactionTemplate()->FactionMask;
    if(fSupport & myFaction)
        combatSupport = true;

    // check friend/enemy list
    for(uint32 i = 0; i < 4; i++)
    {
        if(objB->GetFactionTemplate()->FriendlyFactions[i] && objB->GetFactionTemplate()->FriendlyFactions[i] == objA->GetFactionTemplate()->Faction)
        {
            combatSupport = true;
            break;
        }
        if(objB->GetFactionTemplate()->EnemyFactions[i] && objB->GetFactionTemplate()->EnemyFactions[i] == objA->GetFactionTemplate()->Faction)
        {
            combatSupport = false;
            break;
        }
    }
    return combatSupport;
}

bool FactionSystem::isAlliance(WorldObject* objA)// A is alliance?
{
    if(!objA || objA->GetFactionTemplate() == NULL || objA->GetFaction() == NULL)
        return true;

    //Get stormwind faction frm dbc (11/72)
    FactionTemplateEntry * m_sw_factionTemplate = dbcFactionTemplate.LookupEntry(11);
    FactionEntry * m_sw_faction = dbcFaction.LookupEntry(72);

    if(m_sw_factionTemplate == objA->GetFactionTemplate() || m_sw_faction == objA->GetFaction())
        return true;

    //Is StormWind hostile to ObjectA?
    if(objA->GetFactionTemplate()->FactionMask & m_sw_factionTemplate->HostileMask)
        return false;

    //Is ObjectA hostile to StormWind?
    if(objA->GetFactionTemplate()->HostileMask & m_sw_factionTemplate->FactionMask)
        return false;

    // check friend/enemy list
    for(uint8 i = 0; i < 4; i++)
    {
        if(objA->GetFactionTemplate()->EnemyFactions[i] == m_sw_faction->ID)
            return false;
        if(m_sw_factionTemplate->EnemyFactions[i] == objA->GetFactionID())
            return false;
    }

    //We're not hostile towards SW, so we are allied
    return true;
}
