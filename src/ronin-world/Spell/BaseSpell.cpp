/***
 * Demonstrike Core
 */

#include "StdAfx.h"

BaseSpell::BaseSpell(WorldObject* caster, SpellEntry *info) : m_caster(caster), m_spellInfo(info)
{

}

BaseSpell::~BaseSpell()
{
    m_caster = NULL;
    m_spellInfo = NULL;
}
