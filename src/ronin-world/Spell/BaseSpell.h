/***
 * Demonstrike Core
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
};

// Spell instance
class BaseSpell
{
public:
    BaseSpell(WorldObject* caster, SpellEntry *info, uint8 castNumber);
    ~BaseSpell();
    virtual void Destruct();

    RONIN_INLINE WorldObject *GetCaster() { return m_caster; }
    RONIN_INLINE SpellEntry *GetSpellProto() { return m_spellInfo; }
    RONIN_INLINE uint8 GetCastNumber() { return m_castNumber; }

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
            if((m_duration = m_spellInfo->CalculateSpellDuration(m_caster->getLevel(), 0)) != -1 && m_spellInfo->SpellGroupType && m_caster->IsUnit())
            {
                castPtr<Unit>(m_caster)->SM_FIValue(SMT_DURATION, (int32*)&m_duration, m_spellInfo->SpellGroupType);
                castPtr<Unit>(m_caster)->SM_PIValue(SMT_DURATION, (int32*)&m_duration, m_spellInfo->SpellGroupType);
            }
        }
        return m_duration;
    }

    RONIN_INLINE void SetInternalRadius(uint32 i)
    {
        b_radSet[i] = true;
        m_radius[0][i] = m_spellInfo->radiusHostile[i], m_radius[1][i] = m_spellInfo->radiusFriend[i];
        if (m_spellInfo->SpellGroupType && m_caster && m_caster->IsUnit())
        {
            castPtr<Unit>(m_caster)->SM_FFValue(SMT_RADIUS, &m_radius[0][i], m_spellInfo->SpellGroupType);
            castPtr<Unit>(m_caster)->SM_PFValue(SMT_RADIUS, &m_radius[0][i], m_spellInfo->SpellGroupType);
            castPtr<Unit>(m_caster)->SM_FFValue(SMT_RADIUS, &m_radius[1][i], m_spellInfo->SpellGroupType);
            castPtr<Unit>(m_caster)->SM_PFValue(SMT_RADIUS, &m_radius[1][i], m_spellInfo->SpellGroupType);
        }
    }

    RONIN_INLINE float GetRadius(uint32 i)
    {
        if (b_radSet[i] == false)
            SetInternalRadius(i);
        return m_radius[0][i];
    }

    RONIN_INLINE float GetFriendlyRadius(uint32 i)
    {
        if (b_radSet[i] == false)
            SetInternalRadius(i);
        return m_radius[1][i];
    }

protected:
    WorldObject *m_caster;
    SpellEntry *m_spellInfo;
    uint8 m_castNumber;

    int32  m_duration;
    float  m_radius[2][3];
    bool   b_radSet[3], b_durSet;
    bool   m_AreaAura;

    uint32  m_spellState;

    SpellCastTargets m_targets;
};
