/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    //can't use items while dead.
    if(_player->getDeathState()==CORPSE)
        return;

    uint8 bagIndex, slot, castFlags;
    uint8 castCount;
    uint64 itemGUID;
    uint32 glyphIndex;
    uint32 spellId;
    recvPacket >> bagIndex >> slot >> castCount >> spellId >> itemGUID >> glyphIndex >> castFlags;

    Item* tmpItem = NULLITEM;
    tmpItem = _player->GetItemInterface()->GetInventoryItem(bagIndex,slot);

    if (!tmpItem)
        tmpItem = _player->GetItemInterface()->GetInventoryItem(slot);
    if (!tmpItem)
        return;

    ItemPrototype *itemProto = tmpItem->GetProto();
    if(!itemProto)
        return;

    if(sScriptMgr.CallScriptedItem(tmpItem,_player))
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

    uint32 x;
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

    if(tmpItem->HasEnchantedOnUseSpell(spellId))
        matching = true;

    if(!matching)
        return;

    // check for spell id
    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellId );

    if(!spellInfo)
    {
        sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_ON_STAND_UP)
    {
        if (_player->CombatStatus.IsInCombat() || _player->IsMounted())
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem,NULLITEM,INV_ERR_CANT_DO_IN_COMBAT);
            return;
        }

        if(_player->GetStandState()!=STANDSTATE_SIT)
            _player->SetStandState(STANDSTATE_SIT);
    }

    if(itemProto->HolidayId > 0)
    {
        uint32 mask = sWorld.ConverHolidayIdToMask(itemProto->HolidayId);
        if ((sWorld.GetCurrentHolidayMask() & mask) == 0)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem,NULLITEM,INV_ERR_OBJECT_IS_BUSY);
            return;
        }
    }

    if(itemProto->RequiredLevel > 0)
    {
        if(_player->getLevel() < (uint32)itemProto->RequiredLevel)
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem,NULLITEM,INV_ERR_ITEM_RANK_NOT_ENOUGH);
            return;
        }
    }

    if(itemProto->RequiredSkill > 0)
    {
        if(!_player->_HasSkillLine(itemProto->RequiredSkill))
        {
            _player->GetItemInterface()->BuildInventoryChangeError(tmpItem,NULLITEM,INV_ERR_ITEM_RANK_NOT_ENOUGH);
            return;
        }

        if(itemProto->RequiredSkillRank > 0)
        {
            if(_player->_GetSkillLineCurrent(itemProto->RequiredSkill, false) < (uint32)itemProto->RequiredSkillRank)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(tmpItem,NULLITEM,INV_ERR_ITEM_RANK_NOT_ENOUGH);
                return;
            }
        }
    }

    if( !_player->ignoreitemreq_cheat && (itemProto->AllowableClass && !(_player->getClassMask() & itemProto->AllowableClass) || itemProto->AllowableRace && !(_player->getRaceMask() & itemProto->AllowableRace) ))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(tmpItem,NULLITEM,INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM);
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
            if( !_player->GetSummon() || _player->GetSummon()->GetEntry() != (uint32)itemProto->ForcedPetId )
            {
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_SPELL_IN_PROGRESS, castCount, 0);
                return;
            }
        }
    }

    if(!sHookInterface.OnCastSpell(_player, spellInfo))
    {
        _player->SendCastResult(spellInfo->Id, SPELL_FAILED_UNKNOWN, castCount, 0);
        return;
    }

    Spell* spell = new Spell(_player, spellInfo, false, NULLAURA);
    spell->extra_cast_number= castCount;
    spell->m_glyphIndex = glyphIndex;
    spell->i_caster = tmpItem;
    if( spell->prepare(&targets) == SPELL_CANCAST_OK )
        _player->Cooldown_AddItem( itemProto, x );
}

bool IsException(Player* plr, uint32 spellid);

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();

    uint32 spellId;
    uint8  cn, castFlags;
    recvPacket >> cn >> spellId >> castFlags;

    if(!spellId)
    {
        sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        return;
    }
    // check for spell id
    SpellEntry *spellInfo = dbcSpell.LookupEntry(spellId);

    if(!spellInfo || !sHookInterface.OnCastSpell(_player, spellInfo))
    {
        SKIP_READ_PACKET(recvPacket);
        if(spellInfo == NULL)
            sLog.outDebug("WORLD: unknown spell id %i\n", spellId);
        return;
    }

    if(!_player->isAlive() && !(spellInfo->Attributes & ATTRIBUTES_CASTABLE_WHILE_DEAD))
    {
        SKIP_READ_PACKET(recvPacket);
        return;
    }

    if(_player->GetUInt32Value(UNIT_FIELD_CHARMEDBY))
    {
        SKIP_READ_PACKET(recvPacket);
        _player->SendCastResult(spellInfo->Id, SPELL_FAILED_CHARMED, cn, 0);
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

    if((!GetPlayer()->HasSpell(spellId) || spellInfo->Attributes & ATTRIBUTES_PASSIVE) && !IsException(_player, spellId))
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
        if((spellInfo->Flags3 & FLAGS3_ACTIVATE_AUTO_SHOT) /*spellInfo->Attributes == 327698*/) // auto shot..
        {
            //sLog.outString( "HandleSpellCast: Auto Shot-type spell cast (id %u, name %s)" , spellInfo->Id , spellInfo->Name );
            Item* weapon = GetPlayer()->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
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
        if( spellInfo->self_cast_only == true )
        {
            if( targets.m_unitTarget && targets.m_unitTarget != _player->GetGUID() )
            {
                // send the error message
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }

        if( targets.m_unitTarget && GetPlayer()->GetMapMgr() && spellInfo->c_is_flags & SPELL_FLAG_IS_DAMAGING )
        {
            Unit* pUnit = GetPlayer()->GetMapMgr()->GetUnit( targets.m_unitTarget );
            if( pUnit && pUnit != GetPlayer() && !sFactionSystem.isAttackable( GetPlayer(), pUnit, false ) && !pUnit->IsInRangeOppFactSet(GetPlayer()) && !pUnit->CombatStatus.DidDamageTo(GetPlayer()->GetGUID()))
            {
                //GetPlayer()->BroadcastMessage("Faction exploit detected. You will be disconnected in 5 seconds.");
                //GetPlayer()->Kick(5000);
                // Just cancel the cast
                _player->SendCastResult(spellInfo->Id, SPELL_FAILED_BAD_TARGETS, cn, 0);
                return;
            }
        }

        Spell* spell = new Spell(GetPlayer(), spellInfo, false, NULLAURA);
        spell->extra_cast_number = cn;
        spell->prepare(&targets);
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

    Object* caster = NULL;
    if (_player->m_CurrentCharm != NULL)
        caster = _player->m_CurrentCharm;
    else if (_player->m_Summon != NULL)
        caster = _player->m_Summon;
    else if (_player->GetVehicle() != NULL)
        caster = _player->GetVehicle();

    if (caster == NULL)
        return;

    uint64 guid;
    uint32 spellid;
    uint8 castnumber;
    uint8 castFlags;
    recvPacket >> guid >> castnumber >> spellid >> castFlags;

    SpellEntry* sp = dbcSpell.LookupEntry(spellid);
    SpellCastTargets targets;
    targets.read(recvPacket, caster->GetGUID(), castFlags);

    // Summoned Elemental's Freeze
    if (spellid == 33395)
    {
        if (!_player->m_Summon)
            return;
    }
    else if ((!_player->m_CurrentCharm || guid != _player->m_CurrentCharm->GetGUID()) && _player->GetVehicle() == NULL)
    {
        return;
    }

    Spell* pSpell = new Spell(caster, sp, false, NULLAURA);
    pSpell->extra_cast_number = castnumber;
    pSpell->prepare(&targets);
}

bool IsException(Player* plr, uint32 spellid)
{
    switch(spellid)
    {
    case 63644:
    case 63645:
        {
            if(plr->m_talentSpecsCount > 1)
                return true;
            else
                return false;
        }break;
    }
    return false;
}
