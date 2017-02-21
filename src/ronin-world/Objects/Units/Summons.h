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

class Summon;

enum SummonType
{
    SUMMON_TYPE_NONE = 0,
    SUMMON_TYPE_COMPANION,
    SUMMON_TYPE_GUARDIAN,
    SUMMON_TYPE_POSSESSED,
    SUMMON_TYPE_TOTEM,
    SUMMON_TYPE_WILD,
    SUMMON_TYPE_CRITTER
};

class SERVER_DECL SummonHandler
{
public:
    SummonHandler() { };
    ~SummonHandler() { m_summon = NULL; };

    void Initialize(Summon* n_summon);

    virtual void Destruct() { };
    virtual void OnPushToWorld() { };
    virtual void OnRemoveInRangeObject(WorldObject* object) { };
    virtual uint8 GetSummonType() { return SUMMON_TYPE_NONE; };
    virtual void Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot) { };

protected:
    Summon* m_summon;
};

class SERVER_DECL CompanionSummon : public SummonHandler
{
public:
    CompanionSummon() { };
    ~CompanionSummon() { };

    uint8 GetSummonType() { return SUMMON_TYPE_COMPANION; };
    void Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL GuardianSummon : public SummonHandler
{
public:
    GuardianSummon() { };
    ~GuardianSummon() { };

    uint8 GetSummonType() { return SUMMON_TYPE_GUARDIAN; };
    void Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL PossessedSummon : public SummonHandler
{
public:
    PossessedSummon() { };
    ~PossessedSummon() { };

    uint8 GetSummonType() { return SUMMON_TYPE_POSSESSED; };
    void Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL TotemSummon : public SummonHandler
{
public:
    TotemSummon() { };
    ~TotemSummon() { };

    void SetupSpells();
    void OnPushToWorld() { SetupSpells(); };
    uint8 GetSummonType() { return SUMMON_TYPE_TOTEM; };
    void Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL WildSummon : public SummonHandler
{
public:
    WildSummon() { };
    ~WildSummon() { };

    uint8 GetSummonType() { return SUMMON_TYPE_WILD; };
    void Load(Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
};

class SERVER_DECL Summon : public Creature
{
public:
    Summon(CreatureData* data, uint64 guid);
    ~Summon();

    virtual bool IsTotem() { return GetSummonType() == SUMMON_TYPE_TOTEM; }
    virtual bool IsSummon() { return true; }

    void Init();
    void Destruct();
    void OnPushToWorld();
    void CreateAs(SummonHandler* NewHandle);
    void Load(Unit* m_owner, LocationVector & position, uint32 spellid, int32 summonslot);

    int32 GetSummonSlot() { return summonslot; };
    Unit* GetSummonOwner() { return s_Owner; };
    // Returns the internal summon type or none
    uint8 GetSummonType() { return m_Internal ? m_Internal->GetSummonType() : SUMMON_TYPE_NONE; };

private:
    SummonHandler* m_Internal;

    int32 summonslot;       // Summon slot of the creature in the owner's summonhandler, -1 means no slot
    Unit* s_Owner;          // Summoner of the creature
};
