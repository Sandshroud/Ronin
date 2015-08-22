/***
 * Demonstrike Core
 */

#pragma once

// Spell instance
class BaseSpell
{
public:
    BaseSpell(WorldObject* caster, SpellEntry *info);
    ~BaseSpell();

    RONIN_INLINE SpellEntry *GetSpellProto() { return m_spellInfo; }

protected:
    WorldObject *m_caster;
    SpellEntry *m_spellInfo;
};
