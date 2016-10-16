/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    //can't use items while dead.
    if(_player->isDead())
        return;

    uint8 bagIndex, slot, castCount;
    uint64 itemGUID;
    uint32 spellId;
    recvPacket >> bagIndex >> slot >> castCount >> spellId >> itemGUID;

    Item* tmpItem = _player->GetInventory()->GetInventoryItem(bagIndex, slot);
    if (tmpItem == NULL && (tmpItem = _player->GetInventory()->GetInventoryItem(slot)) == NULL)
        return;

    ItemPrototype *itemProto = tmpItem->GetProto();
    if(itemProto == NULL)
        return;

    tmpItem->Bind(ITEM_BIND_ON_USE);
    if(itemProto->QuestId)
    {
        // Item Starter
        Quest *qst = sQuestMgr.GetQuestPointer(itemProto->QuestId);
        if(!qst)
            return;

        if( sQuestMgr.PlayerMeetsReqs(_player, qst, false) != QMGR_QUEST_AVAILABLE || qst->qst_min_level > _player->getLevel() )
            return;

        WorldPacket data;
        sQuestMgr.BuildQuestDetails(&data, qst, tmpItem, 0, _player);
        SendPacket(&data);
    }

    SpellCastTargets targets(recvPacket, _player->GetGUID());
    targets.m_itemTarget = itemGUID;
    uint8 x;
    bool matching = false;
    for(x = 0; x < 5; x++)
    {
        if(itemProto->Spells[x].Trigger == USE)
        {
            if(itemProto->Spells[x].Id == spellId)
            {
                matching = true;
                break;
            }
        }
    }

    if(matching == false)
        return;

    // check for spell id
    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellId );
    if(spellInfo == NULL)
    {
        sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
    {
        if (_player->IsInCombat() || _player->IsMounted())
        {
            _player->GetInventory()->BuildInventoryChangeError(tmpItem,NULL,INV_ERR_NOT_IN_COMBAT);
            return;
        }

        if(_player->GetStandState()!=STANDSTATE_SIT)
            _player->SetStandState(STANDSTATE_SIT);
    }

    if(itemProto->RequiredLevel > 0)
    {
        if(_player->getLevel() < (uint32)itemProto->RequiredLevel)
        {
            _player->GetInventory()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_CANT_EQUIP_RANK);
            return;
        }
    }

    if(itemProto->RequiredSkill > 0)
    {
        if(!_player->_HasSkillLine(itemProto->RequiredSkill))
        {
            _player->GetInventory()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_CANT_EQUIP_RANK);
            return;
        }

        if(itemProto->RequiredSkillRank > 0)
        {
            if(_player->_GetSkillLineCurrent(itemProto->RequiredSkill, false) < (uint32)itemProto->RequiredSkillRank)
            {
                _player->GetInventory()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_CANT_EQUIP_RANK);
                return;
            }
        }
    }

    if(itemProto->AllowableClass && !(_player->getClassMask() & itemProto->AllowableClass) || itemProto->AllowableRace && !(_player->getRaceMask() & itemProto->AllowableRace))
    {
        _player->GetInventory()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_CANT_EQUIP_EVER);
        return;
    }

    if( !_player->Cooldown_CanCast( itemProto, x ) )
    {
        _player->SendCastResult(spellInfo->Id, SPELL_FAILED_NOT_READY, castCount, 0);
        return;
    }

    if(_player->m_currentSpell)
    {
        _player->SendCastResult(spellInfo->Id, SPELL_FAILED_SPELL_IN_PROGRESS, castCount, 0);
        return;
    }

    if( itemProto->ForcedPetId >= 0 )
    {
        if( itemProto->ForcedPetId == 0 )
        {
            if( _player->GetGUID() != targets.m_unitTarget )
            {
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, castCount, 0);
                return;
            }
        }
        else
        {
            _player->SendCastResult(spellInfo->Id, SPELL_FAILED_SPELL_IN_PROGRESS, castCount, 0);
            return;
        }
    }

    if(Spell* spell = new Spell(_player, spellInfo, castCount))
        if( spell->prepare(&targets, false) == SPELL_CANCAST_OK )
            _player->Cooldown_AddItem( itemProto, x );
}

bool IsException(Player* plr, uint32 spellid);

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint8 cn; uint32 spellId;
    recvPacket >> cn >> spellId;
    SpellCastTargets targets(recvPacket, _player->GetGUID());

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
        return;
    }

    if((!_player->isAlive() && !spellInfo->isCastableWhileDead()) || _player->m_CurrentCharm)
        return;

    if(_player->GetUInt32Value(UNIT_FIELD_CHARMEDBY))
    {
        _player->SendCastResult(spellInfo->Id, SPELL_FAILED_CHARMED, cn, 0);
        return;
    }

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
            /*Item* weapon = GetPlayer()->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
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
                if(!targets.m_unitTarget)
                {
                    sLog.outString( "Cancelling auto-shot cast because targets.m_unitTarget is null!" );
                    return;
                }

                _player->m_AutoShotSpell = dbcSpell.LookupEntry(spellid);
                _player->m_AutoShotDuration = _player->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);
                //This will fix fast clicks
                if(_player->m_AutoShotAttackTimer < 500)
                    _player->m_AutoShotAttackTimer = 500;
                _player->m_onAutoShot = true;
            }*/

            return;
        }

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

        if( targets.m_unitTarget && GetPlayer()->GetMapInstance() && spellInfo->isSpellDamagingEffect() )
        {
            Unit* pUnit = GetPlayer()->GetMapInstance()->GetUnit( targets.m_unitTarget );
            if( pUnit && pUnit != GetPlayer() && !sFactionSystem.isAttackable( GetPlayer(), pUnit, false ))
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
    else if(SpellEntry* info = dbcSpell.LookupEntry(spellId))
        _player->m_AuraInterface.RemoveAllPosAurasByNameHash(info->NameHash);
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

}

void WorldSession::HandleCharmForceCastSpell(WorldPacket & recvPacket)
{
    CHECK_INWORLD_RETURN();

    WorldObject* caster = NULL;
    if (_player->m_CurrentCharm != NULL)
        caster = _player->m_CurrentCharm;
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
        return;
    else if (!_player->m_CurrentCharm || guid != _player->m_CurrentCharm->GetGUID())
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
            if(plr->GetTalentInterface()->GetSpecCount() > 1)
                return true;
        }break;
    }
    return false;
}
