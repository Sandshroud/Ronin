/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    SpellCastTargets targets(recvPacket, _player->GetGUID());

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

        if( sQuestMgr.PlayerMeetsReqs(_player, qst, false, false) != QMGR_QUEST_AVAILABLE || qst->qst_min_level > _player->getLevel() )
            return;

        WorldPacket data;
        sQuestMgr.BuildQuestDetails(&data, qst, tmpItem, 0, _player);
        SendPacket(&data);
    }

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
        if (_player->IsInCombat())
        {
            _player->GetInventory()->BuildInventoryChangeError(tmpItem,NULL,INV_ERR_NOT_IN_COMBAT);
            return;
        }

        if(_player->m_AuraInterface.HasMountAura())
        {
            _player->GetInventory()->BuildInventoryChangeError(tmpItem,NULL,INV_ERR_SHAPESHIFT_FORM_CANNOT_EQUIP);
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
        if(!_player->HasSkillLine(itemProto->RequiredSkill))
        {
            _player->GetInventory()->BuildInventoryChangeError(tmpItem, NULL, INV_ERR_CANT_EQUIP_RANK);
            return;
        }

        if(itemProto->RequiredSkillRank > 0)
        {
            if(_player->getSkillLineVal(itemProto->RequiredSkill, false) < (uint32)itemProto->RequiredSkillRank)
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

    if(_player->isCasting())
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

    if(Spell* spell = new Spell(_player, spellInfo, castCount, tmpItem->GetGUID()))
        if( spell->prepare(&targets, false) == SPELL_CANCAST_OK && !spell->HasCastTime())
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
    if(((!GetPlayer()->HasSpell(spellId)) || spellInfo->isPassiveSpell()) && !IsException(_player, spellId))
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

    _player->GetSpellInterface()->LaunchSpell(spellInfo, cn, targets);
}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    _player->GetSpellInterface()->CleanupCurrentSpell();
}

void WorldSession::HandleCancelAuraOpcode( WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    SpellEntry* info = dbcSpell.LookupEntry(spellId);
    if(info == NULL)
        return;
    if(!_player->GetSpellInterface()->CleanupSpecificSpell(info))
        _player->m_AuraInterface.RemoveAllPosAurasByNameHash(info->NameHash);
}

void WorldSession::HandleCancelChannellingOpcode( WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;
    SpellEntry* info = dbcSpell.LookupEntry(spellId);
    if(info == NULL)
        return;
    _player->GetSpellInterface()->CleanupSpecificSpell(info);
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data)
{

}

class SpellCategoryCooldownModCallback : public AuraInterface::ModCallback
{
public:
    virtual void operator()(Modifier *mod)
    {
        switch(mod->m_type)
        {
        case SPELL_AURA_MOD_SPELL_CATEGORY_COOLDOWN:
            (*categoryMap)[mod->m_miscValue[0]] += mod->m_amount;
            break;
        }
    }

    void Init(std::map<uint32, int32> *mapToFill) { categoryMap = mapToFill; }

    std::map<uint32, int32> *categoryMap;
};

void WorldSession::HandleRequestSpellCategoryCooldownOpcode(WorldPacket &recv_data)
{
    CHECK_INWORLD_RETURN();

    std::map<uint32, int32> categoryModifiers;
    _player->FillMapWithSpellCategories(&categoryModifiers);
    SpellCategoryCooldownModCallback spellCategoryCooldownCallback;
    spellCategoryCooldownCallback.Init(&categoryModifiers);
    _player->m_AuraInterface.TraverseModMap(SPELL_AURA_MOD_SPELL_CATEGORY_COOLDOWN, &spellCategoryCooldownCallback);

    WorldPacket data(SMSG_SPELL_CATEGORY_COOLDOWN, 20);
    data.WriteBits(categoryModifiers.size(), 23);
    data.FlushBits();
    for(auto itr = categoryModifiers.begin(); itr != categoryModifiers.end(); itr++)
        data << uint32(itr->first) << int32(-itr->second);
    SendPacket(&data);
}

void WorldSession::HandleCharmForceCastSpell(WorldPacket & recvPacket)
{
    CHECK_INWORLD_RETURN();

    Unit* caster = NULL;
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
