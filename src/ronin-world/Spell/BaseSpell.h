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

class SpellCastTargets
{
public:
    void read ( WorldPacket & data, uint64 caster );
    void write ( WorldPacket & data);

    SpellCastTargets() : m_castFlags(0), m_targetIndex(0), m_targetMask(0), m_src(0.f, 0.f, 0.f), m_dest(0.f, 0.f, 0.f), m_dest_transGuid(0), m_src_transGuid(0),
        m_unitTarget(0), m_itemTarget(0), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(uint8 castFlags, uint32 targetIndex, uint32 TargetMask, uint64 unitTarget, uint64 itemTarget, float srcX, float srcY, float srcZ, float destX, float destY, float destZ)
        : m_castFlags(castFlags), m_targetIndex(targetIndex), m_targetMask(TargetMask), m_src(srcX, srcY, srcZ), m_dest(destX, destY, destZ), m_dest_transGuid(0), m_src_transGuid(0),
        m_unitTarget(unitTarget), m_itemTarget(itemTarget), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(uint64 unitTarget) : m_castFlags(0), m_targetIndex(0), m_targetMask(0x2), m_src(0.f, 0.f, 0.f), m_dest(0.f, 0.f, 0.f), m_dest_transGuid(0), m_src_transGuid(0),
        m_unitTarget(unitTarget), m_itemTarget(0), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(Unit *caster, float destX, float destY, float destZ) : m_castFlags(0), m_targetIndex(0), m_targetMask(0x20|0x40), m_src(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ()),
        m_dest(destX, destY, destZ), m_dest_transGuid(0), m_src_transGuid(0), m_unitTarget(0), m_itemTarget(0), missilespeed(0), missilepitch(0), traveltime(0) { }

    SpellCastTargets(WorldPacket & data, uint64 caster) : m_castFlags(0), m_targetIndex(0), m_targetMask(0), m_src(0.f, 0.f, 0.f), m_dest(0.f, 0.f, 0.f),
        missilespeed(0), missilepitch(0), traveltime(0) { m_unitTarget = m_itemTarget = m_dest_transGuid = m_src_transGuid = 0; read(data, caster); }

    SpellCastTargets& operator=(const SpellCastTargets &target)
    {
        m_castFlags = target.m_castFlags;
        m_targetIndex = target.m_targetIndex;
        m_targetMask = target.m_targetMask;
        m_unitTarget = target.m_unitTarget;
        m_itemTarget = target.m_itemTarget;

        m_src = target.m_src;
        m_dest = target.m_dest;

        traveltime = target.traveltime;
        missilespeed = target.missilespeed;
        missilepitch = target.missilepitch;
        m_strTarget = target.m_strTarget;

        m_dest_transGuid = target.m_dest_transGuid;
        m_src_transGuid = target.m_src_transGuid;
        return *this;
    }

    uint8 m_castFlags;
    uint32 m_targetIndex, m_targetMask;
    WoWGuid m_unitTarget, m_itemTarget;

    WoWGuid m_src_transGuid, m_dest_transGuid;
    LocationVector m_src, m_dest;
    float traveltime, missilespeed, missilepitch;
    std::string m_strTarget;

    bool hasDestination() { return m_dest.x && m_dest.y; }
};

struct SpellTarget
{
    SpellTarget(WoWGuid guid) : Guid(guid)
    {
        HitResult = EffectMask = ReflectResult = 0;
        moddedAmount[0] = moddedAmount[1] = moddedAmount[2] = false;
        accumAmount = effectAmount[0] = effectAmount[1] = effectAmount[2] = 0;
        resistMod = 0.f;

        AuraAddResult = AURA_APPL_NOT_RUN;
        aura = NULL;
    }

    WoWGuid Guid;
    uint8 HitResult;
    uint8 EffectMask;
    uint8 ReflectResult;

    float resistMod;
    bool moddedAmount[3];
    int32 accumAmount, effectAmount[3];

    uint8 AuraAddResult;
    Aura *aura;
};

typedef Loki::AssocVector<WoWGuid, SpellTarget*> SpellTargetStorage;
typedef std::vector<std::pair<WoWGuid, uint8>> SpellMissesStorage;
typedef std::set<WoWGuid> SpellDelayTargets;

// Spell instance
class BaseSpell
{
public:
    BaseSpell(Unit* caster, SpellEntry *info, uint8 castNumber, WoWGuid itemGuid);
    ~BaseSpell();

    void _Prepare();
    virtual void Destruct();

    RONIN_INLINE Unit *GetCaster() { return _unitCaster; }
    RONIN_INLINE SpellEntry *GetSpellProto() { return m_spellInfo; }
    RONIN_INLINE uint8 GetCastNumber() { return m_castNumber; }

    // Packet writing functions
    void writeSpellGoTargets( WorldPacket * data );
    void writeSpellCastFlagData(WorldPacket *data, uint32 cast_flags);

    // Send Packet functions
    bool IsNeedSendToClient();
    void SendSpellStart();
    void SendSpellGo();
    void SendSpellMisses(SpellTarget *forced = NULL);
    void SendProjectileUpdate();

    void SendCastResult(uint8 result);
    void SendInterrupted(uint8 result);
    void SendChannelStart(int32 duration);
    void SendChannelUpdate(uint32 time);

    static void SendHealSpellOnPlayer(WorldObject* caster, WorldObject* target, uint32 dmg, bool critical, uint32 overheal, uint32 spellid);
    static void SendHealManaSpellOnPlayer(WorldObject* caster, WorldObject* target, uint32 dmg, uint32 powertype, uint32 spellid);
    void SendResurrectRequest(Player* target);

    RONIN_INLINE int32 GetDuration()
    {
        if (b_durSet == false)
        {
            b_durSet = true;
            if(_unitCaster && (m_duration = m_spellInfo->CalculateSpellDuration(_unitCaster->getLevel(), 0)) != -1 && m_spellInfo->SpellGroupType)
            {
                _unitCaster->SM_FIValue(SMT_DURATION, (int32*)&m_duration, m_spellInfo->SpellGroupType);
                _unitCaster->SM_PIValue(SMT_DURATION, (int32*)&m_duration, m_spellInfo->SpellGroupType);
            }
        }
        return m_duration;
    }

    RONIN_INLINE void SetInternalRadius(uint32 i)
    {
        b_radSet[i] = true;
        m_radius[0][0][i] = m_spellInfo->radiusHostile[0][i], m_radius[0][1][i] = m_spellInfo->radiusFriend[0][i];
        m_radius[1][0][i] = m_spellInfo->radiusHostile[1][i], m_radius[1][1][i] = m_spellInfo->radiusFriend[1][i];
        if (m_spellInfo->SpellGroupType && _unitCaster)
        {
            _unitCaster->SM_FFValue(SMT_RADIUS, &m_radius[0][0][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS, &m_radius[0][0][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_FFValue(SMT_RADIUS, &m_radius[0][1][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS, &m_radius[0][1][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_FFValue(SMT_RADIUS, &m_radius[1][0][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS, &m_radius[1][0][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_FFValue(SMT_RADIUS, &m_radius[1][1][i], m_spellInfo->SpellGroupType);
            _unitCaster->SM_PFValue(SMT_RADIUS, &m_radius[1][1][i], m_spellInfo->SpellGroupType);
        }
    }

    RONIN_INLINE float GetRadius(uint32 i)
    {
        if (b_radSet[i] == false)
            SetInternalRadius(i);
        return m_radius[0][0][i];
    }

    RONIN_INLINE float GetMaxRadius(uint32 i)
    {
        if (b_radSet[i] == false)
            SetInternalRadius(i);
        return m_radius[1][0][i];
    }

    RONIN_INLINE float GetFriendlyRadius(uint32 i)
    {
        if (b_radSet[i] == false)
            SetInternalRadius(i);
        return m_radius[0][1][i];
    }

    RONIN_INLINE float GetMaxFriendlyRadius(uint32 i)
    {
        if (b_radSet[i] == false)
            SetInternalRadius(i);
        return m_radius[1][1][i];
    }

    bool Reflect(Unit* refunit);

protected:
    SpellTarget *GetSpellTarget(WoWGuid guid)
    {
        SpellTargetStorage::iterator itr;
        if((itr = m_fullTargetMap.find(guid)) == m_fullTargetMap.end())
            return NULL;
        return itr->second;
    }

protected:
    Unit *_unitCaster;
    WoWGuid m_casterGuid;

    SpellEntry *m_spellInfo;
    WoWGuid m_itemCaster;
    uint8 m_castNumber;

    int32  m_duration;
    float  m_radius[2][2][3];
    bool   b_radSet[3], b_durSet;
    bool   m_AreaAura;

    uint32 m_castTime, m_timer, m_delayedTimer;
    uint32 m_spellState;

    float m_missilePitch, m_missileSpeed;
    uint32 m_missileTravelTime;
    bool m_isDelayedAOEMissile;

    Aura* m_triggeredByAura;
    bool m_triggeredSpell;
    BaseSpell* m_reflectedParent;

    bool m_isCasting;

protected: // Spell targetting
    SpellTargetStorage m_fullTargetMap, m_effectTargetMaps[3];
    SpellDelayTargets m_delayTargets;
    SpellMissesStorage m_spellMisses;
    SpellCastTargets m_targets;
};
