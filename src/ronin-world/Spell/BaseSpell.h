/***
 * Demonstrike Core
 */

#pragma once

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
        m_radius[0][i] = ::GetDBCRadius(dbcSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
        m_radius[1][i] = ::GetDBCFriendlyRadius(dbcSpellRadius.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));
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

    uint32  m_spellState;
};
