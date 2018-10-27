/***
* Demonstrike Core
*/

#include "StdAfx.h"

/*
 * Start translocate code between silvermoon and undercity
 */
bool TranslocateToTirisfalGlades(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 0;
    X = 1806.014f;
    Y = 333.556f;
    Z = 70.398f;
    O = 4.705f;
    return true;
}

bool TranslocateToSilvermoonCity(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 530;
    X = 10021.391f;
    Y = -7014.54f;
    Z = 49.718f;
    O = 4.031f;
    return true;
}
/*
 * End translocate code between silvermoon and undercity
 */

/*
 * Start translocation trigger for Firewing Point Spire
 * Two spells trigger the same spell, each altering the teleportation target
 */
bool TranslocateToDuskwitherSpireTrigger(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount)
{
    static SpellEntry *translocate = dbcSpell.LookupEntry(26572);
    if(translocate == NULL) // We need our translocate
        return false;

    if(!target->IsPlayer() || castPtr<Player>(target)->GetTeam() != TEAM_HORDE)
        return false;

    castPtr<Player>(target)->GetSpellInterface()->LaunchSpellFromSpell(translocate, NULL, spell->GetSpellProto());
    return true;
}

bool TranslocateToDuskwitherSpire(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    if(spell->GetSpellParent() == NULL)
        return false;

    switch(spell->GetSpellParent()->Id)
    {
    case 34448:
        {
            mapId = 530;
            X = 9334.39f;
            Y = -7812.97f;
            Z = 136.57f;
            O = 5.2f;
        }break;
    case 34452:
        {
            mapId = 530;
            X = 9333.29f;
            Y = -7878.79f;
            Z = 74.91f;
            O = 2.173f;
        }break;
    }
    return true;
}

/*
 * Start translocation trigger for falcon watch
 * Two spells trigger the same spell, each altering the teleportation target
 */
bool TranslocateToFalconWatchTrigger(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount)
{
    static SpellEntry *translocate = dbcSpell.LookupEntry(30141);
    if(translocate == NULL) // We need our translocate
        return false;

    if(!target->IsPlayer() || castPtr<Player>(target)->GetTeam() != TEAM_HORDE)
        return false;

    castPtr<Player>(target)->GetSpellInterface()->LaunchSpellFromSpell(translocate, NULL, spell->GetSpellProto());
    return true;
}

bool TranslocateToFalconWatch(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    if(spell->GetSpellParent() == NULL)
        return false;

    switch(spell->GetSpellParent()->Id)
    {
    case 25650:
        {
            mapId = 530;
            X = -594.42f;
            Y = 4080.17f;
            Z = 93.83f;
            O = 2.1f;
        }break;
    case 25652:
        {
            mapId = 530;
            X = -589.83f;
            Y = 4079.29f;
            Z = 143.26f;
            O = 4.4f;
        }break;
    }
    return true;
}

/*
 * Start translocation trigger for Firewing Point Spire
 * Two different spells, one handler, triggers 2 spells
 */
bool TranslocateToFirewingPointSpireTrigger(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount)
{
    static SpellEntry *translocate = NULL;
    switch(spell->GetSpellProto()->Id)
    {
    case 25140: translocate = dbcSpell.LookupEntry(32568); break;
    case 29129: translocate = dbcSpell.LookupEntry(32569); break;
    }

    if(translocate == NULL) // We need our translocate
        return false;

    if(!target->IsPlayer() || castPtr<Player>(target)->GetTeam() != TEAM_HORDE)
        return false;

    castPtr<Player>(target)->GetSpellInterface()->LaunchSpellFromSpell(translocate, NULL, spell->GetSpellProto());
    return true;
}

bool TranslocateToFirewingPointSpireTop(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 530;
    X = -2260.26f;
    Y = 3114.48f;
    Z = 136.35f;
    O = 4.57f;
    return true;
}

bool TranslocateToFirewingPointSpireBottom(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 530;
    X = -2307.35f;
    Y = 3123.92f;
    Z = 13.69f;
    O = 2.15f;
    return true;
}

/*
 * Start translocation trigger for Firewing Point
 * Two different spells, one handler, triggers 2 spells
 */
bool TranslocateToFirewingPointTrigger(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount)
{
    static SpellEntry *translocate = NULL;
    switch(spell->GetSpellProto()->Id)
    {
    case 29128: translocate = dbcSpell.LookupEntry(32571); break;
    case 25143: translocate = dbcSpell.LookupEntry(32572); break;
    }

    if(translocate == NULL) // We need our translocate
        return false;

    if(!target->IsPlayer() || castPtr<Player>(target)->GetTeam() != TEAM_HORDE)
        return false;

    castPtr<Player>(target)->GetSpellInterface()->LaunchSpellFromSpell(translocate, NULL, spell->GetSpellProto());
    return true;
}

bool TranslocateToFirewingPointTop(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 530;
    X = -2260.27f;
    Y = 3211.04f;
    Z = 14.1116f;
    O = 3.7f;
    return true;
}

bool TranslocateToFirewingPointBottom(SpellEffectClass *spell, uint32 effIndex, WorldObject *target, int32 amount, uint32 &mapId, float &X, float &Y, float &Z, float &O)
{
    mapId = 530;
    X = -2259.74f;
    Y = 3215.03f;
    Z = -4.05f;
    O = 3.14f;
    return true;
}

void SpellManager::_RegisterTranslocateData()
{
    // Gameobjects have these spells, but the data doesn't exist so create them here
    _CreateScriptedEffectSpell(34448);
    _CreateScriptedEffectSpell(34452);

    _RegisterTeleportData(25649, SP_EFF_INDEX_0, &TranslocateToTirisfalGlades);
    _RegisterTeleportData(26566, SP_EFF_INDEX_0, &TranslocateToSilvermoonCity);
    _RegisterScriptedEffect(34448, SP_EFF_INDEX_0, &TranslocateToDuskwitherSpireTrigger);
    _RegisterScriptedEffect(34452, SP_EFF_INDEX_0, &TranslocateToDuskwitherSpireTrigger);
    _RegisterTeleportData(26572, SP_EFF_INDEX_0, &TranslocateToDuskwitherSpire);
    _RegisterScriptedEffect(25650, SP_EFF_INDEX_0, &TranslocateToFalconWatchTrigger);
    _RegisterScriptedEffect(25652, SP_EFF_INDEX_0, &TranslocateToFalconWatchTrigger);
    _RegisterTeleportData(30141, SP_EFF_INDEX_0, &TranslocateToFalconWatch);
    _RegisterScriptedEffect(25140, SP_EFF_INDEX_0, &TranslocateToFirewingPointSpireTrigger);
    _RegisterScriptedEffect(29129, SP_EFF_INDEX_0, &TranslocateToFirewingPointSpireTrigger);
    _RegisterTeleportData(32568, SP_EFF_INDEX_0, &TranslocateToFirewingPointSpireTop);
    _RegisterTeleportData(32569, SP_EFF_INDEX_0, &TranslocateToFirewingPointSpireBottom);
    _RegisterScriptedEffect(29128, SP_EFF_INDEX_0, &TranslocateToFirewingPointTrigger);
    _RegisterScriptedEffect(25143, SP_EFF_INDEX_0, &TranslocateToFirewingPointTrigger);
    _RegisterTeleportData(32571, SP_EFF_INDEX_0, &TranslocateToFirewingPointTop);
    _RegisterTeleportData(32572, SP_EFF_INDEX_0, &TranslocateToFirewingPointBottom);
}
