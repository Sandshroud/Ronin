/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#define MAX_POSITIVE_AURAS 48
#define MAX_NEGATIVE_AURAS 32
#define MAX_PASSIVE_AURAS 80
#define MAX_AURAS MAX_POSITIVE_AURAS+MAX_NEGATIVE_AURAS
#define TOTAL_AURAS MAX_AURAS+MAX_PASSIVE_AURAS

class Unit;
class Modifier;

struct AuraCheckResponse
{
    uint32 Error;
    uint32 Misc;
};

enum AURA_CHECK_RESULT
{
    AURA_CHECK_RESULT_NONE                = 1,
    AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT = 2,
    AURA_CHECK_RESULT_LOWER_BUFF_PRESENT  = 3,
};

enum AuraStatus : uint16
{
    AURA_STATUS_NONE = 0,

    AURA_STATUS_SPELL_IMPARING_MASK = 0xCF
};

class SERVER_DECL AuraInterface
{
public:
    AuraInterface(Unit *unit);
    ~AuraInterface();

    void Destruct();

    void Update(uint32 diff);
    void SavePlayerAuras(std::stringstream *ss);

    void OnChangeLevel(uint32 newlevel);
    uint8 GetFreeSlot(bool ispositive);
    void OnAuraRemove(Aura* aura, uint8 aura_slot);

    bool IsDazed();
    bool IsPoisoned();
    void UpdateDuelAuras();
    void SendAuraData();
    void BuildOutOfRangeAuraUpdate(WorldPacket *data);
    bool BuildAuraUpdateAllPacket(WorldPacket* data);
    void SpellStealAuras(Unit* caster, int32 MaxSteals);
    void UpdateShapeShiftAuras(uint32 oldSS, uint32 newSS);
    void AttemptDispel(Unit* caster, int32 Mechanic, bool hostile);
    void MassDispel(Unit* caster, uint32 index, SpellEntry* Dispelling, uint32 MaxDispel, uint8 start, uint8 end);

    bool OverrideSimilarAuras(WorldObject *caster, Aura *aura);
    bool UpdateAuraModifier(uint32 spellId, WoWGuid casterGuid, uint8 indexEff, Modifier *mod);
    void UpdateAuraModsWithModType(uint32 modType);

    /*****************************
    ************ Info ************
    ******************************/
    uint32 GetSpellIdFromAuraSlot(uint32 slot);
    AuraCheckResponse AuraCheck(SpellEntry *info, WoWGuid casterGuid);
    uint32 GetAuraSpellIDWithNameHash(uint32 name_hash);

    bool HasAura(uint32 spellid);
    bool HasAuraVisual(uint32 visualid);//not spell id!!!
    bool HasActiveAura(uint32 spelllid);
    bool HasNegativeAura(uint32 spell_id); //just to reduce search range in some cases
    bool HasAuraWithMechanic(uint32 mechanic);
    bool HasActiveAura(uint32 spelllid,WoWGuid guid);
    bool HasPosAuraWithMechanic(uint32 mechanic);
    bool HasNegAuraWithMechanic(uint32 mechanic);
    bool HasNegativeAuraWithNameHash(uint32 name_hash); //just to reduce search range in some cases
    bool HasCombatStatusAffectingAuras(WoWGuid checkGuid);
    bool HasAurasOfNameHashWithCaster(uint32 namehash, WoWGuid casterguid);

    // Mounts
    void OnDismount();

    bool HasMountAura();
    bool GetMountedAura(uint32 &auraId);

    bool HasFlightAura();
    void RemoveFlightAuras();

    /*****************************
    ************ Add *************
    ******************************/
    void AddAura(Aura* aur, uint8 slot = 0xFF);
//  uint32 AddAuraVisual(uint32 SpellId, uint32 count, bool positive);

    /*****************************
    ********** Removal ***********
    ******************************/

    //! Remove aura from unit
    void RemoveAura(Aura* aur);
    void RemoveAuraBySlot(uint8 Slot);
    bool RemoveAuras(uint32 * SpellIds);
    void RemoveAuraNoReturn(uint32 spellId);
    bool RemovePositiveAura(uint32 spellId);
    bool RemoveNegativeAura(uint32 spellId);
    bool RemoveAuraByNameHash(uint32 namehash);//required to remove weaker instances of a spell
    bool RemoveAuraPosByNameHash(uint32 namehash);//required to remove weaker instances of a spell
    bool RemoveAuraNegByNameHash(uint32 namehash);//required to remove weaker instances of a spell
    void RemoveAuraBySlotOrRemoveStack(uint8 Slot);
    bool RemoveAura(uint32 spellId, WoWGuid guid = 0);
    void EventRemoveAura(uint32 SpellId) { RemoveAura(SpellId); }


    //! Remove all auras
    void RemoveAllAuras();
    void RemoveAllExpiringAuras();
    void RemoveAllNegativeAuras();
    void RemoveAllNonPassiveAuras();
    void RemoveAllAurasExpiringWithPet();
    void RemoveAllAreaAuras(WoWGuid skipguid);
    bool RemoveAllAurasFromGUID(WoWGuid guid); //remove if they come from the same caster.
    void RemoveAllAurasOfType(uint32 auratype);//ex:to remove morph spells
    bool RemoveAllPosAurasFromGUID(WoWGuid guid); //remove if they come from the same caster.
    bool RemoveAllNegAurasFromGUID(WoWGuid guid); //remove if they come from the same caster.
    void RemoveAllAurasByInterruptFlag(uint32 flag);
    void RemoveAllAurasWithAuraName(uint32 auraName);
    void RemoveAllAurasWithSpEffect(uint32 EffectId);
    bool RemoveAllPosAurasByNameHash(uint32 namehash);//required to remove weaker instances of a spell
    bool RemoveAllNegAurasByNameHash(uint32 namehash);//required to remove weaker instances of a spell
    bool RemoveAllAuras(uint32 spellId, WoWGuid guid = 0); //remove stacked auras but only if they come from the same caster. Shaman purge If GUID = 0 then removes all auras with this spellid
    void RemoveAllAurasWithDispelType(uint32 DispelType);
    void RemoveAllAurasWithAttributes(uint8 index, uint32 attributeFlag);
    bool RemoveAllAurasByNameHash(uint32 namehash, bool passive);//required to remove weaker instances of a spell
    void RemoveAllAurasByInterruptFlagButSkip(uint32 flag, uint32 skip);
    void RemoveAllAurasOfSchool(uint32 School, bool Positive, bool Immune);
    uint32 GetAuraCountWithFamilyNameAndSkillLine(uint32 spellFamily, uint32 SkillLine);
    bool RemoveAllAurasByMechanic( uint32 MechanicType, int32 MaxDispel = -1, bool HostileOnly = true ); // Removes all (de)buffs on unit of a specific mechanic type.

    Aura* FindAuraBySlot(uint8 auraSlot);
    Aura* FindAura(uint32 spellId, WoWGuid guid = 0);
    Aura* FindPositiveAuraByNameHash(uint32 namehash);
    Aura* FindNegativeAuraByNameHash(uint32 namehash);
    Aura* FindActiveAura(uint32 spellId, WoWGuid guid = 0);
    Aura* FindActiveAuraWithNameHash(uint32 namehash, WoWGuid guid = 0);

    void EventDeathAuraRemoval();
//  bool HasVisibleAura(uint32 spellid);

    uint32 GetAuraStatus() { return AURA_STATUS_NONE; }

private:
    Unit* m_Unit;
    Aura *m_auras[TOTAL_AURAS];
    uint8 m_maxPosAuraSlot, m_maxNegAuraSlot, m_maxPassiveAuraSlot;
    std::map<uint8, uint8> m_buffIndexAuraSlots;
    RWMutex m_auraLock;

    /*******************
    **** Modifiers
    ********/
    void _RecalculateModAmountByType(Modifier *mod);

public:
    void UpdateModifier(uint8 auraSlot, uint8 index, Modifier *mod, bool apply);
    static uint16 createModifierIndex(uint8 index1, uint8 index2) { return ((uint16(index2)<<8) | uint16(index1)); }

    class ModifierHolder
    {
    public:
        ModifierHolder(uint32 slot,SpellEntry* info) : auraSlot(slot),spellInfo(info) {for(uint8 i=0;i<3;i++)mod[i]=NULL;};

        uint32 auraSlot;
        SpellEntry *spellInfo;
        Modifier *mod[3];
    };

    struct ModifierType
    {
        explicit ModifierType(uint32 type) : auraType(type), auraSlot(0), auraIndex(0) {}
        ModifierType(uint32 slot, uint32 type, uint32 index) : auraType(type), auraSlot(slot), auraIndex(index) {}

        bool operator<(const ModifierType& val) const { return auraType < val.auraType; }
        bool operator==(const ModifierType& val) const { return auraType == val.auraType && auraSlot == val.auraSlot && auraIndex == val.auraIndex; }
        uint32 auraType, auraSlot, auraIndex;
    };

    typedef Loki::AssocVector<uint16, Modifier*> modifierMap;
    typedef Loki::AssocVector<uint32, modifierMap > modifierTypeMap;

    bool HasAurasWithModType(uint32 modType)
    {
        bool ret = false;
        m_auraLock.LowAcquire();
        if(!m_modifiersByModType.empty() && m_modifiersByModType.find(modType) == m_modifiersByModType.end())
            ret = !m_modifiersByModType[modType].empty();
        m_auraLock.LowRelease();
        return ret;
    }

    /// !DEPRECATED NOT THREAD SAFE
    modifierMap *GetModMapByModType(uint32 modType) { return NULL; }

    // Used for traversing mod map, operator()(Modifier *mod) for operation
    class ModCallback { public: virtual void operator()(Modifier *mod) = 0; virtual void postTraverse(uint32 modType) {}; };

    // Push our callback on all objects inside our modMap
    void TraverseModMap(uint32 modType, ModCallback *callBack);

    // Modifier calculations
    void SM_FIValue( uint32 modifier, int32* v, uint32* group );
    void SM_FFValue( uint32 modifier, float* v, uint32* group );
    void SM_PIValue( uint32 modifier, int32* v, uint32* group );
    void SM_PFValue( uint32 modifier, float* v, uint32* group );

private:
    // Ordered by aura slot
    Loki::AssocVector<uint8, ModifierHolder*> m_modifierHolders;
    // Storage is <ModType, <Index, Modifier> >
    modifierTypeMap m_modifiersByModType;
    // Storage is <SpellGroup, <ModType, <Index, Modifier> > >
    Loki::AssocVector<uint16, Loki::AssocVector<uint8, int32>> m_spellGroupModifiers;
    // Storage is <<SpellId, effIndex>, Modifier>
    Loki::AssocVector<std::pair<uint32, uint8>, int32> m_calcModCache;

    static uint32 get32BitOffsetAndGroup(uint32 value, uint8 &group);
    void UpdateSpellGroupModifiers(bool apply, Modifier *mod, bool silent);
};
