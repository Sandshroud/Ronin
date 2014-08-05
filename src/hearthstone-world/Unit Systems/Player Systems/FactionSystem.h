/***
 * Demonstrike Core
 */

#pragma once

enum FactionMasks
{
    FACTION_MASK_NONE       = 0,
    FACTION_MASK_PLAYER     = 1,
    FACTION_MASK_ALLIANCE   = 2,
    FACTION_MASK_HORDE      = 4,
    FACTION_MASK_MONSTER    = 8
};

enum FactionInteractionStatus
{
    FI_STATUS_NONE      = 0, // Internal error, or not attackable
    FI_STATUS_FRIENDLY  = 1, // Not attackable, unless set in faction list
    FI_STATUS_NEUTRAL   = 2, // Attackable, but will not attack unless attacked
    FI_STATUS_HOSTILE   = 3  // Always attackable/attacks
};

class SERVER_DECL FactionSystem : public Singleton<FactionSystem>
{
public:
    FactionSystem();
    ~FactionSystem();

    void LoadFactionInteractionData();

    // System checks
    bool CanEitherUnitAttack(Unit* objA, Unit* objB, bool CheckStealth = true);
    bool AC_GetAttackableStatus(Player* plr, Unit *target);

    bool isHostile(Object* objA, Object* objB);
    bool isAttackable(Object* objA, Object* objB, bool CheckStealth = true);
    bool isCombatSupport(Object* objA, Object* objB); // B combat supports A?;
    bool isAlliance(Object* objA); // A is alliance?

//private:
    bool IsInteractionLocked(Object *obj);
    FactionInteractionStatus GetFactionsInteractStatus(Object *objA, Object *objB);

    FactionInteractionStatus GetPlayerAttackStatus(Player *plrA, Player *plrB);
    FactionInteractionStatus GetUnitAreaInteractionStatus(Unit *unitA, Unit *unitB);
    FactionInteractionStatus GetAttackableStatus(Object* objA, Object* objB, bool CheckStealth);

    Player* GetPlayerFromObject(Object* obj);

    HEARTHSTONE_INLINE bool isFriendly(Object *objA, Object *objB) { return !isHostile(objA, objB); }
    HEARTHSTONE_INLINE bool isSameFaction(Object* objA, Object* objB)
    {
        // shouldn't be necessary but still
        if( objA->GetFactionTemplate() == NULL || objB->GetFactionTemplate() == NULL )
            return false;

        return (objB->GetFaction() == objA->GetFaction());
    }

};

#define sFactionSystem FactionSystem::getSingleton()
