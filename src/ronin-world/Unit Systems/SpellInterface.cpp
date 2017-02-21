
#include "StdAfx.h"

SpellInterface::SpellInterface(Unit *unit) : m_Unit(unit)
{
    m_currentSpell = NULL;
    m_lastSpell = NULL;
}

SpellInterface::~SpellInterface()
{

}

void SpellInterface::Update(uint32 msTime, uint32 uiDiff)
{
    if(m_currentSpell != NULL)
        m_currentSpell->Update(uiDiff);
}

void SpellInterface::Cleanup()
{
    CleanupCurrentSpell();
}

void SpellInterface::OnRemoveFromWorld()
{
    CleanupCurrentSpell();
}

void SpellInterface::CleanupCurrentSpell()
{
    if(m_currentSpell != NULL)
        m_currentSpell->cancel();
    m_currentSpell = NULL;
}

bool SpellInterface::CleanupSpecificSpell(SpellEntry *sp)
{
    if(m_currentSpell && m_currentSpell->GetSpellProto() == sp)
    {
        m_currentSpell->cancel();
        m_currentSpell = NULL;
        return true;
    }
    return false;
}

SpellEntry *SpellInterface::GetCurrentSpellProto()
{
    if(m_currentSpell)
        return m_currentSpell->GetSpellProto();
    return NULL;
}

void SpellInterface::ProcessSpell(Spell *castingSpell)
{
    m_currentSpell = castingSpell;
    m_lastSpell = castingSpell->GetSpellProto();
}

void SpellInterface::FinishSpell(Spell *finishedSpell)
{
    if(m_currentSpell == finishedSpell)
        m_currentSpell = NULL;
}

void SpellInterface::InterruptCast(Spell *interruptSpell, uint32 interruptTime)
{
    if(m_currentSpell == NULL || m_currentSpell == interruptSpell)
        return;
    if(m_currentSpell->getState() >= SPELL_STATE_FINISHED)
        return;
    if(interruptTime && m_currentSpell->GetSpellProto()->School)
        PreventSchoolCast(m_currentSpell->GetSpellProto()->School, interruptTime);
    CleanupCurrentSpell();
}

void SpellInterface::LaunchSpell(SpellEntry *info, Unit *target, uint8 castNumber)
{
    SpellCastTargets targets(target->GetGUID());
    if(Spell *spell = new Spell(m_Unit, info, castNumber))
        spell->prepare(&targets, false);
}

void SpellInterface::TriggerSpell(SpellEntry *info, Unit *target, uint8 castNumber)
{
    SpellCastTargets targets(target->GetGUID());
    if(Spell *spell = new Spell(m_Unit, info, castNumber))
        spell->prepare(&targets, true);
}

void SpellInterface::PushbackCast(uint32 school)
{
    if(m_currentSpell)
        m_currentSpell->AddTime(school);
}

void SpellInterface::PreventSchoolCast(uint32 school, uint32 duration)
{

}

bool SpellInterface::checkCast(SpellEntry *sp, uint8 &errorOut)
{
    return true;
}
