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

    RONIN_INLINE SpellEntry *GetSpellProto() { return m_spellInfo; }

    void SendCastResult(uint8 result);
    void SendInterrupted(uint8 result);
    void SendChannelStart(int32 duration);
    void SendChannelUpdate(uint32 time);

protected:
    WorldObject *m_caster;
    SpellEntry *m_spellInfo;
    uint8 m_castNumber;
};
