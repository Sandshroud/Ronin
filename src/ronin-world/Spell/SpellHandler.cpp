/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{

}

bool IsException(Player* plr, uint32 spellid);

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint8 cn;
    uint32 spellId;
    recvPacket >> cn >> spellId;
    if(spellId == 0)
    {
        sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    // check for spell id
    SpellEntry *spellInfo = NULL;
    if((spellInfo = dbcSpell.LookupEntry(spellId)) == NULL)
    {
        sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        SKIP_READ_PACKET(recvPacket);
        return;
    }

    if(!_player->isAlive() && !spellInfo->isCastableWhileDead())
    {
        SKIP_READ_PACKET(recvPacket);
        return;
    }

    if(_player->GetUInt32Value(UNIT_FIELD_CHARMEDBY))
    {
        _player->SendCastResult(spellInfo->Id, SPELL_FAILED_CHARMED, cn, 0);
        SKIP_READ_PACKET(recvPacket);
        return;
    }
    if(_player->m_CurrentCharm)
    {
        SKIP_READ_PACKET(recvPacket);
        return;
    }

    sLog.Debug("WORLD","Received cast_spell packet, spellId - %i (%s), data length = %i", spellId, spellInfo->Name, recvPacket.size());

    // Cheat Detection only if player and not from an item
    // this could fuck up things but meh it's needed ALOT of the newbs are using WPE now
    // WPE allows them to mod the outgoing packet and basicly choose what ever spell they want :(
    if((!GetPlayer()->HasSpell(spellId) || spellInfo->isPassiveSpell()) && !IsException(_player, spellId))
    {
        // Some spells the player doesn't actually know, but are given to him by his current shapeshift.
        // These spells should be allowed to be cast.
        uint8 shapeshift = GetPlayer()->GetShapeShift();
        SpellShapeshiftFormEntry * ssf = dbcSpellShapeshiftForm.LookupEntry(shapeshift);
        if(!ssf) return;

        bool ok = false;
        for(uint8 i = 0; i < 8; i++)
            if( ssf->spells[i] == spellId)
                ok = true;

        if( !ok )
            return;
    }

    if (GetPlayer()->GetOnMeleeSpell() != spellId)
    {
        //autoshot 75
        if((spellInfo->isAutoRepeatSpell()) /*spellInfo->Attributes == 327698*/) // auto shot..
        {
            //sLog.outString( "HandleSpellCast: Auto Shot-type spell cast (id %u, name %s)" , spellInfo->Id , spellInfo->Name );
            Item* weapon = GetPlayer()->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
            if(!weapon)
                return;
            uint32 spellid;
            switch(weapon->GetProto()->SubClass)
            {
            case 2:          // bows
            case 3:          // guns
            case 18:         // crossbow
                spellid = SPELL_RANGED_GENERAL;
                break;
            case 16:            // thrown
                spellid = SPELL_RANGED_THROW;
                break;
            case 19:            // wands
                spellid = SPELL_RANGED_WAND;
                break;
            default:
                spellid = 0;
                break;
            }

            if(!spellid)
                spellid = spellInfo->Id;

            if(!_player->m_onAutoShot)
            {
                _player->m_AutoShotTarget = _player->GetSelection();
                uint32 duration = _player->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
                SpellCastTargets targets(recvPacket, GetPlayer()->GetGUID());
                if(!targets.m_unitTarget)
                {
                    sLog.outString( "Cancelling auto-shot cast because targets.m_unitTarget is null!" );
                    return;
                }
                SpellEntry *sp = dbcSpell.LookupEntry(spellid);

                _player->m_AutoShotSpell = sp;
                _player->m_AutoShotDuration = duration;
                //This will fix fast clicks
                if(_player->m_AutoShotAttackTimer < 500)
                    _player->m_AutoShotAttackTimer = 500;
                _player->m_onAutoShot = true;
            }

            return;
        }

        SpellCastTargets targets(recvPacket, GetPlayer()->GetGUID());

        if(_player->m_currentSpell)
        {
            if( _player->m_currentSpell->getState() == SPELL_STATE_CASTING )
            {
                // cancel the existing channel spell, cast this one
                _player->m_currentSpell->cancel();
            }
            else
            {
                // send the error message
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_SPELL_IN_PROGRESS, cn, 0);
                return;
            }
        }

        // some anticheat stuff
        if( spellInfo->isSpellSelfCastOnly() )
        {
            if( targets.m_unitTarget && targets.m_unitTarget != _player->GetGUID() )
            {
                // send the error message
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }

        if( targets.m_unitTarget && GetPlayer()->GetMapMgr() && spellInfo->isSpellDamagingEffect() )
        {
            Unit* pUnit = GetPlayer()->GetMapMgr()->GetUnit( targets.m_unitTarget );
            if( pUnit && pUnit != GetPlayer() && !sFactionSystem.isAttackable( GetPlayer(), pUnit, false ) && !pUnit->CombatStatus.DidDamageTo(GetPlayer()->GetGUID()))
            {
                //GetPlayer()->BroadcastMessage("Faction exploit detected. You will be disconnected in 5 seconds.");
                //GetPlayer()->Kick(5000);
                // Just cancel the cast
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }

        if(Spell* spell = new Spell(GetPlayer(), spellInfo, cn))
            spell->prepare(&targets, false);
    }
}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    if(GetPlayer()->m_currentSpell)
        GetPlayer()->m_currentSpell->cancel();
}

void WorldSession::HandleCancelAuraOpcode( WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    if(_player->m_currentSpell && _player->m_currentSpell->GetSpellProto()->Id == spellId)
        _player->m_currentSpell->cancel();
    else
    {
        SpellEntry* info = dbcSpell.LookupEntry(spellId);
        _player->m_AuraInterface.RemoveAllPosAurasByNameHash(info->NameHash);
        sLog.Debug("Aura","Removing aura with names %s", info->Name);
    }
}

void WorldSession::HandleCancelChannellingOpcode( WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    if(_player->m_currentSpell != NULL)
    {
        if(_player->m_currentSpell->GetSpellProto()->Id != spellId)
            sLog.Debug("Spell","Player cancelled spell that was not being channeled: %u", spellId);

        _player->m_currentSpell->cancel();
    }
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data)
{
    if(_player)
        _player->m_onAutoShot = false;
}

void WorldSession::HandleCharmForceCastSpell(WorldPacket & recvPacket)
{
    CHECK_INWORLD_RETURN();

    WorldObject* caster = NULL;
    if (_player->m_CurrentCharm != NULL)
        caster = _player->m_CurrentCharm;
    else if (_player->m_Summon != NULL)
        caster = _player->m_Summon;
    else if (_player->GetVehicle() != NULL)
        caster = _player->GetVehicle();
    if (caster == NULL)
        return;

    WoWGuid guid;
    uint32 spellid;
    uint8 castnumber;
    uint8 castFlags;
    recvPacket >> guid >> castnumber >> spellid;

    SpellEntry* sp = dbcSpell.LookupEntry(spellid);
    SpellCastTargets targets(recvPacket, caster->GetGUID());

    // Summoned Elemental's Freeze
    if (spellid == 33395)
    {
        if (!_player->m_Summon)
            return;
    }
    else if ((!_player->m_CurrentCharm || guid != _player->m_CurrentCharm->GetGUID()) && _player->GetVehicle() == NULL)
        return;

    if(Spell* pSpell = new Spell(caster, sp, castnumber))
        pSpell->prepare(&targets, false);
}

bool IsException(Player* plr, uint32 spellid)
{
    switch(spellid)
    {
    case 63644:
    case 63645:
        {
            if(plr->m_talentInterface.GetSpecCount() > 1)
                return true;
        }break;
    }
    return false;
}
