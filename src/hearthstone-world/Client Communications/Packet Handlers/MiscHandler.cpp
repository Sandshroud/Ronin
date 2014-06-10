/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleRepopRequestOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Recvd CMSG_REPOP_REQUEST Message" );
    uint8 popcheck;
    recv_data >> popcheck;
    // Todo: Death checks and whatnot.

    if(_player->m_CurrentTransporter)
        _player->m_CurrentTransporter->RemovePlayer(_player);

    if(_player->GetVehicle())
        _player->GetVehicle()->RemovePassenger(_player);

      GetPlayer()->RepopRequestedPlayer();
}

void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recv_data )
{
    if(!_player->IsInWorld() || !_player->GetLootGUID()) 
        return;
    uint32 itemid = 0;
    uint32 amt = 1;
    uint8 lootSlot = 0;
    uint8 error = 0;
    SlotResult slotresult;

    Item* add;

    if(_player->isCasting())
        _player->InterruptCurrentSpell();

    GameObject* pGO = NULLGOB;
    Object* pLootObj;

    // handle item loot
    uint64 guid = _player->GetLootGUID();
    if( GUID_HIPART(guid) == HIGHGUID_TYPE_ITEM )
        pLootObj = _player->GetItemInterface()->GetItemByGUID(guid);
    else
        pLootObj = _player->GetMapMgr()->_GetObject(guid);

    if( pLootObj == NULL )
        return;

    if( pLootObj->GetTypeId() == TYPEID_GAMEOBJECT )
        pGO = TO_GAMEOBJECT(pLootObj);

    recv_data >> lootSlot;
    if (lootSlot >= pLootObj->m_loot.items.size())
        return;

    if( pLootObj->m_loot.items.at(lootSlot).roll != NULL )
        return;

    uint32 ffatype = pLootObj->m_loot.items.at(lootSlot).ffa_loot;
    if (ffatype == 0 || ffatype == 1)
    {
        //make sure this player can still loot it in case of ffa_loot
        if(pLootObj->m_loot.items.at(lootSlot).has_looted.size())
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_ALREADY_LOOTED);
            return;
        }
    }
    else if(ffatype == 2)
    {
        //make sure this player can still loot it in case of ffa_loot
        LooterSet::iterator itr = pLootObj->m_loot.items.at(lootSlot).has_looted.find(_player->GetLowGUID());

        if(itr != pLootObj->m_loot.items.at(lootSlot).has_looted.end())
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM,INV_ERR_ALREADY_LOOTED);
            return;
        }
    }

    itemid = pLootObj->m_loot.items.at(lootSlot).item.itemproto->ItemId;
    ItemPrototype* it = pLootObj->m_loot.items.at(lootSlot).item.itemproto;
    if((error = _player->GetItemInterface()->CanReceiveItem(it, 1, NULL)))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, error);
        return;
    }

    if(it->Class == ITEM_CLASS_QUEST)
    {
        QuestAssociationList *tempList = sQuestMgr.GetQuestAssociationListForItemId( itemid );
        if(tempList != NULL)
        {
            uint32 countNeeded = 0;
            for(QuestAssociationList::iterator itr = tempList->begin(); itr != tempList->end(); itr++)
            {
                for(uint8 p = 0; p < 6; p++)
                    if((*itr)->qst->required_item[p] == itemid)
                        countNeeded += (*itr)->qst->required_itemcount[p];
            }

            if(countNeeded)
            {
                countNeeded -= _player->GetItemInterface()->GetItemCount(itemid);
                if(amt > countNeeded)
                    amt = countNeeded;
            }
        }
    }

    if(amt == 0)
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_CANT_CARRY_MORE_OF_THIS);
        return;
    }

    if(pGO)
        CALL_GO_SCRIPT_EVENT(pGO, OnLootTaken)(_player, it);
    else if(pLootObj->IsCreature())
        CALL_SCRIPT_EVENT(pLootObj, OnLootTaken)(_player, it);

    sHookInterface.OnLoot(_player, pLootObj, 0, itemid);

    add = GetPlayer()->GetItemInterface()->FindItemLessMax(itemid, amt, false);
    if (!add)
    {
        slotresult = GetPlayer()->GetItemInterface()->FindFreeInventorySlot(it);
        if(!slotresult.Result)
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
            return;
        }

        sLog.Debug("HandleAutostoreItem","AutoLootItem %u",itemid);
        Item* item = objmgr.CreateItem( itemid, GetPlayer());

        item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, amt);
        if(pLootObj->m_loot.items.at(lootSlot).iRandomProperty!=NULL)
        {
            item->SetRandomProperty(pLootObj->m_loot.items.at(lootSlot).iRandomProperty->ID);
            item->ApplyRandomProperties(false);
        }
        else if(pLootObj->m_loot.items.at(lootSlot).iRandomSuffix != NULL)
        {
            item->SetRandomSuffix(pLootObj->m_loot.items.at(lootSlot).iRandomSuffix->id);
            item->ApplyRandomProperties(false);
        }

        if( GetPlayer()->GetItemInterface()->SafeAddItem(item,slotresult.ContainerSlot, slotresult.Slot) )
        {
            sQuestMgr.OnPlayerItemPickup(GetPlayer(), item, amt);
            SendItemPushResult(item,false,true,true,true,slotresult.ContainerSlot,slotresult.Slot,1);
        }
        else
        {
            item->DeleteMe();
            item = NULLITEM;
        }
    }
    else
    {
        add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + amt);
        add->m_isDirty = true;

        sQuestMgr.OnPlayerItemPickup(GetPlayer(), add, amt);
        SendItemPushResult(add, false, true, true, false, _player->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()), 0xFFFFFFFF, amt);
    }

    // in case of ffa_loot update only the player who recives it.
    if (!pLootObj->m_loot.items.at(lootSlot).ffa_loot)
    {
        pLootObj->m_loot.items.at(lootSlot).has_looted.insert(_player->GetLowGUID());

        // this gets sent to all looters
        WorldPacket data(SMSG_LOOT_REMOVED, 1);
        data << lootSlot;
        Player* plr;
        for(LooterSet::iterator itr = pLootObj->m_loot.looters.begin(); itr != pLootObj->m_loot.looters.end(); itr++)
        {
            plr = _player->GetMapMgr()->GetPlayer((*itr));
            if( plr != NULL )
                plr->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        pLootObj->m_loot.items.at(lootSlot).has_looted.insert(_player->GetLowGUID());
        WorldPacket data(SMSG_LOOT_REMOVED, 1);
        data << lootSlot;
        _player->GetSession()->SendPacket(&data);
    }

    /* any left yet? (for fishing bobbers) */
    if(pGO && pGO->GetEntry() == GO_FISHING_BOBBER)
    {
        for(vector<__LootItem>::iterator itr = pLootObj->m_loot.items.begin(); itr != pLootObj->m_loot.items.end(); itr++)
        {
            if(!itr->has_looted.size())
                return;
        }

        pGO->ExpireAndDelete();
    }
}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & recv_data )
{
    // sanity checks
    CHECK_INWORLD_RETURN();

    // lookup the object we will be looting
    Object* pLootObj = NULL;
    if( GUID_HIPART(_player->GetLootGUID()) == HIGHGUID_TYPE_ITEM )
        pLootObj = _player->GetItemInterface()->GetItemByGUID(_player->GetLootGUID());
    else
        pLootObj = _player->GetMapMgr()->_GetObject(_player->GetLootGUID());

    Player* plr;
    if( pLootObj == NULL )
        return;

    // is there any left? :o
    if( pLootObj->m_loot.gold == 0 )
        return;

    uint32 money = pLootObj->m_loot.gold;
    for(LooterSet::iterator itr = pLootObj->m_loot.looters.begin(); itr != pLootObj->m_loot.looters.end(); itr++)
    {
        if((plr = _player->GetMapMgr()->GetPlayer(*itr)))
            plr->GetSession()->OutPacket(SMSG_LOOT_CLEAR_MONEY);
    }

    WorldPacket pkt(SMSG_LOOT_MONEY_NOTIFY, 100);
    if(!_player->InGroup())
    {
        if((_player->GetUInt32Value(PLAYER_FIELD_COINAGE) + money) >= PLAYER_MAX_GOLD)
            return;

        pkt << money;
        SendPacket(&pkt);
        _player->ModUnsigned32Value( PLAYER_FIELD_COINAGE , money);
        pLootObj->m_loot.gold = 0;
    }
    else
    {
        Group* party = _player->GetGroup();
        pLootObj->m_loot.gold = 0;

        vector<Player*  > targets;
        targets.reserve(party->MemberCount());

        SubGroup * sgrp;
        GroupMembersSet::iterator itr;
        party->getLock().Acquire();
        for(uint32 i = 0; i < party->GetSubGroupCount(); i++)
        {
            sgrp = party->GetSubGroup(i);
            for(itr = sgrp->GetGroupMembersBegin(); itr != sgrp->GetGroupMembersEnd(); itr++)
            {
                if((*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->GetZoneId() == _player->GetZoneId() && _player->GetInstanceID() == (*itr)->m_loggedInPlayer->GetInstanceID())
                    targets.push_back((*itr)->m_loggedInPlayer);
            }
        }
        party->getLock().Release();

        if(!targets.size())
            return;

        uint32 share = money / uint32(targets.size());
        pkt << share;

        for(vector<Player*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
        {
            if(((*itr)->GetUInt32Value(PLAYER_FIELD_COINAGE) + share) >= PLAYER_MAX_GOLD)
                continue;

            (*itr)->ModUnsigned32Value(PLAYER_FIELD_COINAGE, share);
            (*itr)->GetSession()->SendPacket(&pkt);
        }
    }
}

void WorldSession::HandleLootOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;

    if(_player->GetMapMgr()->GetCreature(guid) && _player->GetMapMgr()->GetCreature(guid)->IsVehicle())
        return;

    if(_player->isCasting())
        _player->InterruptCurrentSpell();

    if(_player->InGroup() && !_player->m_bg)
    {
        Group * party = _player->GetGroup();
        if(party)
        {
            if(party->GetMethod() == PARTY_LOOT_MASTER)
            {
                WorldPacket data(SMSG_LOOT_MASTER_LIST, 324);
                uint32 real_count = 0;
                SubGroup *s;
                GroupMembersSet::iterator itr;
                party->Lock();
                data << (uint8)party->MemberCount();
                for(uint32 i = 0; i < party->GetSubGroupCount(); i++)
                {
                    s = party->GetSubGroup(i);
                    for(itr = s->GetGroupMembersBegin(); itr != s->GetGroupMembersEnd(); itr++)
                    {
                        if((*itr)->m_loggedInPlayer && _player->GetZoneId() == (*itr)->m_loggedInPlayer->GetZoneId())
                        {
                            data << (*itr)->m_loggedInPlayer->GetGUID();
                            ++real_count;
                        }
                    }
                }
                party->Unlock();
                data.put<uint32>(0, real_count);
                party->SendPacketToAll(&data);
            }
        }
    }

    _player->SendLoot(guid, _player->GetMapId(), LOOT_CORPSE);
}


void WorldSession::HandleLootReleaseOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    WorldPacket data(SMSG_LOOT_RELEASE_RESPONSE, 9);
    data << guid << uint8( 1 );
    SendPacket( &data );

    _player->SetLootGUID(0);
    _player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
    _player->m_currentLoot = 0;

    // special case
    if( GUID_HIPART( guid ) == HIGHGUID_TYPE_GAMEOBJECT )
    {
        GameObject* pGO = _player->GetMapMgr()->GetGameObject( GUID_LOPART(guid) );
        if( pGO == NULL )
            return;

        pGO->m_loot.looters.erase(_player->GetLowGUID());
        switch( pGO->GetType())
        {
        case GAMEOBJECT_TYPE_FISHINGNODE:
            {
                if(pGO->IsInWorld())
                    pGO->RemoveFromWorld(true);

                pGO->Destruct();
                pGO = NULLGOB;
            }break;
        case GAMEOBJECT_TYPE_CHEST:
            {
                pGO->m_loot.looters.erase( _player->GetLowGUID() );
                //check for locktypes

                Lock* pLock = dbcLock.LookupEntry( pGO->GetInfo()->GetLockID() );
                if( pLock )
                {
                    for( uint32 i=0; i < 8; i++ )
                    {
                        if( pLock->locktype[i] )
                        {
                            if( pLock->locktype[i] == 2 ) //locktype;
                            {
                                //herbalism and mining;
                                if( pLock->lockmisc[i] == LOCKTYPE_MINING || pLock->lockmisc[i] == LOCKTYPE_HERBALISM )
                                {
                                    //we still have loot inside.
                                    if( pGO->m_loot.HasItems(_player) )
                                    {
                                        pGO->SetState(1);
                                        return;
                                    }

                                    if( pGO->CanMine() )
                                    {
                                        pGO->ClearLoot();
                                        pGO->UseMine();
                                        return;
                                    }
                                    else
                                    {
                                        pGO->CalcMineRemaining( true );
                                        pGO->Despawn( 0, pGO->GetInfo()->RespawnTimer);
                                        return;
                                    }
                                }
                                else //other type of locks that i dont bother to split atm ;P
                                {
                                    if(pGO->m_loot.HasItems(_player))
                                    {
                                        pGO->SetState(1);
                                        return;
                                    }

                                    pGO->CalcMineRemaining(true);

                                    //Don't interfere with scripted GO's
                                    if(!sEventMgr.HasEvent(pGO, EVENT_GMSCRIPT_EVENT))
                                        pGO->Despawn(0, pGO->GetInfo()->RespawnTimer);
                                    return;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if( pGO->m_loot.HasItems(_player) )
                    {
                        pGO->SetState(1);
                        return;
                    }

                    pGO->Despawn(0, pGO->GetInfo()->RespawnTimer);
                }
            }
        }
        return;
    }
    else if( GUID_HIPART(guid) == HIGHGUID_TYPE_ITEM )
    {
        // if we have no items left, destroy the item.
        Item* pItem = _player->GetItemInterface()->GetItemByGUID(guid);
        if( pItem != NULL )
        {
            if( !pItem->m_loot.HasItems(_player) )
                _player->GetItemInterface()->SafeFullRemoveItemByGuid(guid);
        }

        return;
    }
    else if( GUID_HIPART(guid) == HIGHGUID_TYPE_CREATURE )
    {
        Unit* pLootTarget = _player->GetMapMgr()->GetUnit(guid);
        if( pLootTarget != NULL )
        {
            pLootTarget->m_loot.looters.erase(_player->GetLowGUID());
            if( !pLootTarget->m_loot.HasLoot(_player) )
            {
                TO_CREATURE(pLootTarget)->UpdateLootAnimation(_player);

                // skinning
                if(!pLootTarget->IsPet() && !TO_CREATURE(pLootTarget)->IsSummon()
                    && lootmgr.IsSkinnable( pLootTarget->GetEntry()) && !TO_CREATURE(pLootTarget)->Skinned)
                {
                    pLootTarget->BuildFieldUpdatePacket( _player, UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );
                }
            }
        }
    }
    else if( GUID_HIPART(guid) == HIGHGUID_TYPE_CORPSE )
    {
        Corpse* pCorpse = objmgr.GetCorpse((uint32)guid);
        if( pCorpse != NULL && !pCorpse->m_loot.HasLoot(_player) )
            pCorpse->Despawn();
    }
}

void WorldSession::HandleWhoOpcode( WorldPacket & recv_data )
{
    uint32 min_level;
    uint32 max_level;
    uint32 class_mask;
    uint32 race_mask;
    uint32 zone_count;
    uint32 * zones = 0;
    uint32 name_count;
    string * names = 0;
    string chatname;
    string unkstr;
    bool cname;
    uint32 i;

    if( ((uint32)UNIXTIME - m_lastWhoTime) < 10 && !GetPlayer()->bGMTagOn )
        return;

    m_lastWhoTime = (uint32)UNIXTIME;
    recv_data >> min_level >> max_level;
    recv_data >> chatname >> unkstr >> race_mask >> class_mask;
    recv_data >> zone_count;

    if(zone_count > 0 && zone_count < 10)
    {
        zones = new uint32[zone_count];

        for(i = 0; i < zone_count; i++)
            recv_data >> zones[i];
    }
    else
    {
        zone_count = 0;
    }

    recv_data >> name_count;
    if(name_count > 0 && name_count < 10)
    {
        names = new string[name_count];

        for(i = 0; i < name_count; i++)
            recv_data >> names[i];
    }
    else
    {
        name_count = 0;
    }

    if(chatname.length() > 0)
        cname = true;
    else
        cname = false;

    sLog.Debug( "WORLD"," Recvd CMSG_WHO Message with %u zones and %u names", zone_count, name_count );

    bool gm = false;
    uint32 team = _player->GetTeam();
    if(HasGMPermissions())
        gm = true;

    uint32 sent_count = 0;
    uint32 total_count = 0;

    PlayerStorageMap::const_iterator itr,iend;
    Player* plr;
    uint32 lvl;
    bool add;
    WorldPacket data;
    data.SetOpcode(SMSG_WHO);
    data << uint64(0);

    objmgr._playerslock.AcquireReadLock();
    iend=objmgr._players.end();
    itr=objmgr._players.begin();
    while(itr !=iend && sent_count < 50)
    {
        plr = itr->second;
        ++itr;

        if(!plr->GetSession() || !plr->IsInWorld())
            continue;

        if(!HasGMPermissions())
            if(plr->GetSession()->HasGMPermissions())
                continue;

        // Team check
        if(!sWorld.cross_faction_world)
            if(!gm && plr->GetTeam() != team && !plr->GetSession()->HasGMPermissions())
                continue;

        ++total_count;

        // Add by default, if we don't have any checks
        add = true;

        // Chat name
        if(cname && chatname != *plr->GetNameString())
            continue;

        // Level check
        lvl = plr->m_uint32Values[UNIT_FIELD_LEVEL];
        if(min_level && max_level)
        {
            // skip players outside of level range
            if(lvl < min_level || lvl > max_level)
                continue;
        }

        // Zone id compare
        if(zone_count)
        {
            // people that fail the zone check don't get added
            add = false;
            for(i = 0; i < zone_count; i++)
            {
                if(zones[i] == plr->GetZoneId())
                {
                    add = true;
                    break;
                }
            }
        }

        if(!(class_mask & plr->getClassMask()) || !(race_mask & plr->getRaceMask()))
            add = false;

        // skip players that fail zone check
        if(!add)
            continue;

        // name check
        if(name_count)
        {
            // people that fail name check don't get added
            add = false;
            for(i = 0; i < name_count; i++)
            {
                if(!strnicmp(names[i].c_str(), plr->GetName(), names[i].length()))
                {
                    add = true;
                    break;
                }
            }
        }

        if(!add)
            continue;

        // if we're here, it means we've passed all testing
        // so add the names :)
        data << plr->GetName();
        uint32 GuildId = plr->m_playerInfo->GuildId;
        if(GuildId)
        {
            GuildInfo* gInfo = guildmgr.GetGuildInfo(GuildId);
            if(gInfo != NULL)
                data << gInfo->m_guildName.c_str();
            else
                data << uint8(0);
        }
        else
            data << uint8(0);

        data << plr->m_uint32Values[UNIT_FIELD_LEVEL];
        data << uint32(plr->getClass());
        data << uint32(plr->getRace());
        data << plr->getGender();
        data << uint32(plr->GetZoneId());
        ++sent_count;
    }
    objmgr._playerslock.ReleaseReadLock();
    data.wpos(0);
    data << sent_count;
    data << sent_count;

    SendPacket(&data);

    // free up used memory
    if(zones)
        delete [] zones;
    if(names)
        delete [] names;
}

void WorldSession::HandleLogoutRequestOpcode( WorldPacket & recv_data )
{
    Player* pPlayer = GetPlayer();
    sLog.Debug( "WORLD"," Recvd CMSG_LOGOUT_REQUEST Message" );

    if(pPlayer)
    {
        WorldPacket data(SMSG_LOGOUT_RESPONSE, 5);
        sHookInterface.OnLogoutRequest(pPlayer);
        if(pPlayer->m_isResting ||    // We are resting so log out instantly
            pPlayer->GetTaxiState() ||  // or we are on a taxi
            HasGMPermissions())        // or we are a gm
        {
            data << uint8(0);
            data << uint32(0x1000000);
            SendPacket( &data );
            SetLogoutTimer(1);
            return;
        }

        if(pPlayer->DuelingWith != NULL || pPlayer->CombatStatus.IsInCombat())
        {
            //can't quit still dueling or attacking
            data << uint8(0x1); //Logout accepted
            data << uint32(0);
            SendPacket( &data );
            return;
        }

        data << uint8(0); //Logout accepted
        data << uint32(0);
        SendPacket( &data );

        //stop player from moving
        pPlayer->SetMovement(MOVE_ROOT,1);

        // Set the "player locked" flag, to prevent movement
        pPlayer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

        //make player sit
        pPlayer->SetStandState(STANDSTATE_SIT);
        SetLogoutTimer(PLAYER_LOGOUT_DELAY);
    }
}

void WorldSession::HandlePlayerLogoutOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Recvd CMSG_PLAYER_LOGOUT Message" );
    if(!HasGMPermissions())
    {
        // send "You do not have permission to use this"
        SendNotification(NOTIFICATION_MESSAGE_NO_PERMISSION);
    } 
    else
        LogoutPlayer(true);
}

void WorldSession::HandleLogoutCancelOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Recvd CMSG_LOGOUT_CANCEL Message" );

    Player* pPlayer = GetPlayer();
    if(!pPlayer || !_logoutTime)
        return;

    //Cancel logout Timer
    SetLogoutTimer(0);

    //tell client about cancel
    OutPacket(SMSG_LOGOUT_CANCEL_ACK);

    //unroot player
    pPlayer->SetMovement(MOVE_UNROOT,5);

    // Remove the "player locked" flag, to allow movement
    pPlayer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    //make player stand
    pPlayer->SetStandState(STANDSTATE_STAND);

    sLog.Debug( "WORLD"," sent SMSG_LOGOUT_CANCEL_ACK Message" );
}

void WorldSession::HandleZoneUpdateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    uint32 newZone;
    recv_data >> newZone;

    _player->_EventExploration();

    //clear buyback
    _player->GetItemInterface()->EmptyBuyBack();
}

void WorldSession::HandleSetTargetOpcode( WorldPacket & recv_data )
{
    // obselete?
}

void WorldSession::HandleSetSelectionOpcode( WorldPacket & recv_data )
{
    uint64 guid;
    recv_data >> guid;

    _player->SetUInt64Value(UNIT_FIELD_TARGET, guid);
    _player->SetSelection(guid);

    if(_player->m_comboPoints)
        _player->UpdateComboPoints();
}

void WorldSession::HandleStandStateChangeOpcode( WorldPacket & recv_data )
{
    uint32 animstate;
    recv_data >> animstate;

    _player->SetStandState(int8(animstate));
}

void WorldSession::HandleBugOpcode( WorldPacket & recv_data )
{
    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recv_data >> suggestion >> contentlen >> content >> typelen >> type;

    if( suggestion == 0 )
        sLog.Debug( "WORLD"," Received CMSG_BUG [Bug Report]" );
    else
        sLog.Debug( "WORLD"," Received CMSG_BUG [Suggestion]" );

    sLog.outDebug( type.c_str( ) );
    sLog.outDebug( content.c_str( ) );
}

void WorldSession::HandleCorpseReclaimOpcode(WorldPacket& recv_data)
{
    if(_player->isAlive())
        return;

    sLog.outDebug("WORLD: Received CMSG_RECLAIM_CORPSE");

    uint64 guid;
    recv_data >> guid;

    Corpse* pCorpse = objmgr.GetCorpse( guid );

    if( pCorpse == NULL )
    {
        QueryResult* result = CharacterDatabase.Query("SELECT * FROM corpses WHERE guid = %u AND mapId = %u",
            guid, _player->GetMapId());
        if(result == NULL)
            return;
        delete result;
        pCorpse = _player->CreateCorpse();
    }

    // Check that we're reviving from a corpse, and that corpse is associated with us.
    if( pCorpse->GetUInt32Value( CORPSE_FIELD_OWNER ) != _player->GetLowGUID() && pCorpse->GetUInt32Value( CORPSE_FIELD_FLAGS ) == 5 )
    {
        WorldPacket data( SMSG_RESURRECT_FAILED, 4 );
        data << uint32(1); // this is a real guess!
        SendPacket(&data);
        return;
    }

    // Check we are actually in range of our corpse
    if ( pCorpse->GetDistance2dSq( _player ) > CORPSE_MINIMUM_RECLAIM_RADIUS_SQ || GetPlayer()->PreventRes)
    {
        WorldPacket data( SMSG_RESURRECT_FAILED, 4 );
        data << uint32(1);
        SendPacket(&data);
        return;
    }

    // Check death clock before resurrect they must wait for release to complete
    if( pCorpse->GetDeathClock() + (_player->ReclaimCount*15) > time( NULL ) )
    {
        WorldPacket data( SMSG_RESURRECT_FAILED, 4 );
        data << uint32(1);
        SendPacket(&data);
        return;
    }

    GetPlayer()->ResurrectPlayer();
    GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, GetPlayer()->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2 );
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    sLog.outDebug("WORLD: Received CMSG_RESURRECT_RESPONSE");

    if(GetPlayer()->isAlive() || GetPlayer()->PreventRes)
        return;

    uint64 guid;
    uint8 status;
    recv_data >> guid;
    recv_data >> status;

    // check
    if( guid != 0 && _player->resurrector != (uint32)guid )
    {
        // error
        return;
    }

    // need to check guid
    Unit* pl = _player->GetMapMgr()->GetUnit(guid);
    if(!pl || status != 1)
    {
        _player->m_resurrectHealth = 0;
        _player->m_resurrectMana = 0;
        _player->resurrector = 0;
        return;
    }

    // reset resurrector
    _player->resurrector = 0;
    _player->SetMovement(MOVE_UNROOT, 1);
    _player->ResurrectPlayer(pl);
    _player->m_resurrectHealth = 0;
    _player->m_resurrectMana = 0;

}

void WorldSession::HandleUpdateAccountData(WorldPacket& recv_data)
{
    uint32 uiID;
    recv_data >> uiID;
    if(uiID > 8)
    {
        SKIP_READ_PACKET(recv_data);
        sLog.outString("WARNING: Accountdata > 8 (%d) was requested to be updated by %s of account %d!", uiID, GetPlayer()->GetName(), GetAccountId());
        return;
    }

    uint32 _time, uiDecompressedSize;
    recv_data >> _time >> uiDecompressedSize;

    // client wants to 'erase' current entries
    if(uiDecompressedSize == 0 || uiDecompressedSize >= 0xFFFF)
    {
        SKIP_READ_PACKET(recv_data);
        SetAccountData(uiID, NULL, false,0);

        WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4+4);
        data << uint32(uiID) << uint32(0);
        SendPacket(&data);
        return;
    }

    uLongf uid = uiDecompressedSize;
    size_t ReceivedPackedSize = recv_data.size() - 12;
    char* acctdata = new char[uiDecompressedSize+1];
    memset(acctdata, 0, uiDecompressedSize+1);
    if(uiDecompressedSize > ReceivedPackedSize) // if packed is compressed
    {
        int32 ZlibResult = uncompress((uint8*)acctdata, &uid, recv_data.contents() + 12, (uLong)ReceivedPackedSize);
        switch (ZlibResult)
        {
        case Z_OK:                //0 no error decompression is OK
            {
                SetAccountData(uiID, acctdata, uiDecompressedSize, _time);
                sLog.outDebug("WORLD: Successfully decompressed account data %d for %s, and updated storage array.", uiID, GetPlayer() ? GetPlayer()->GetName() : GetAccountNameS());
            }break;

        case Z_ERRNO:               //-1
        case Z_STREAM_ERROR:        //-2
        case Z_DATA_ERROR:          //-3
        case Z_MEM_ERROR:           //-4
        case Z_BUF_ERROR:           //-5
        case Z_VERSION_ERROR:       //-6
            {
                delete [] acctdata;
                sLog.outString("WORLD WARNING: Decompression of account data %d for %s FAILED.", uiID, GetPlayer() ? GetPlayer()->GetName() : GetAccountNameS());
            }break;

        default:
            delete [] acctdata;
            sLog.outString("WORLD WARNING: Decompression gave a unknown error: %x, of account data %d for %s FAILED.", ZlibResult, uiID, GetPlayer() ? GetPlayer()->GetName() : GetAccountNameS());
            break;
        }
    }
    else
    {
        memcpy(acctdata, recv_data.contents() + 12, uiDecompressedSize);
        SetAccountData(uiID, acctdata, uiDecompressedSize, _time);
    }SKIP_READ_PACKET(recv_data); // Spam cleanup for packet size checker... Because who cares about this dataz

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4+4);
    data << uint32(uiID) << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleRequestAccountData(WorldPacket& recv_data)
{
    uint32 id;
    recv_data >> id;
    if(id > 8)
    {
        sLog.outString("WARNING: Accountdata > 8 (%d) was requested by %s of account %d!", id, GetPlayer() ? GetPlayer()->GetName() : "UNKNOWN", GetAccountId());
        return;
    }

    AccountDataEntry* res = GetAccountData(id);
    if(res == NULL || res->data == NULL)
        return;

    uLongf destSize = compressBound(res->sz);
    ByteBuffer bbuff;
    bbuff.resize(destSize);
    if(res->sz < 500)
        bbuff.append(res->data);
    else if(res->sz && compress((uint8*)bbuff.contents(), &destSize, (uint8*)res->data, res->sz) != Z_OK)
    {
        sLog.outDebug("Error while compressing ACCOUNT_DATA");
        SKIP_READ_PACKET(recv_data);
        return;
    }

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA, 18+bbuff.size());
    data << uint64(_player ? _player->GetGUID() : 0);
    data << uint32(id);
    data << uint32(res->timeStamp);
    data << uint32(res->sz);
    data.append(bbuff);
    SendPacket(&data);
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recv_data)
{
    sLog.Debug( "WORLD"," Received CMSG_SET_ACTION_BUTTON" );
    uint8 button, type;
    uint32 PackedAction, action;
    recv_data >> button >> PackedAction;

    type = ACTION_BUTTON_TYPE(PackedAction);
    action = ACTION_BUTTON_ACTION(PackedAction);

    sLog.outDebug("BUTTON: %u ACTION: %u TYPE: %u", button, action, type );
    if(PackedAction == 0)
    {
        sLog.outDebug( "MISC: Remove action from button %u", button );
        GetPlayer()->setAction(button, 0, 0);
    }
    else
    {
        switch(type)
        {
        case ACTION_BUTTON_SPELL:
            {
                sLog.outDebug( "MISC: Added Spell %u into button %u", action, button );
                GetPlayer()->setAction(button,action,type);
            }break;
        case ACTION_BUTTON_EQSET:
            {
                sLog.outDebug( "MISC: Added EquipmentSet %u into button %u", action, button );
                GetPlayer()->setAction(button,action,type);
            }break;
        case ACTION_BUTTON_MACRO:
        case ACTION_BUTTON_CMACRO:
            {
                sLog.outDebug( "MISC: Added Macro %u into button %u", action, button );
                GetPlayer()->setAction(button,action,type);
            }break;
        case ACTION_BUTTON_ITEM:
            {
                sLog.outDebug( "MISC: Added Item %u into button %u", action, button );
                GetPlayer()->setAction(button,action,type);
            }break;
        default:
            {
                sLog.outDebug( "Unknown Action Type %u for button %u", action, button );
            }break;
        }
    }
}

void WorldSession::HandleSetWatchedFactionIndexOpcode(WorldPacket &recvPacket)
{
    uint32 factionid;
    recvPacket >> factionid;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, factionid);
}

void WorldSession::HandleTogglePVPOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    std::string error;
    uint32 areaId = _player->GetAreaId();
    if(sWorld.FunServerMall != -1 && areaId == (uint32)sWorld.FunServerMall)
    {
        if(AreaTableEntry *at = dbcAreaTable.LookupEntry(areaId))
        {
            error.append("You cannot flag for PvP while in the area: ");
            error.append(at->name);
            error.append(".");
        }
        else
            error.append("You cannot do that here.");
        return;
    }
    else if(_player->HasAreaFlag(OBJECT_AREA_FLAG_INSANCTUARY))
        error.append("You cannot do that here.");

    if(!error.length())
        _player->PvPToggle(); // Crow: Should be a delayed pvp flag
    else
        sChatHandler.ColorSystemMessage(this, MSG_COLOR_WHITE, error.c_str());
}

void WorldSession::HandleGameObjectUse(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;
    sLog.outDebug("WORLD: CMSG_GAMEOBJ_USE: [GUID %d]", guid);

    GameObject* obj = _player->GetMapMgr()->GetGameObject(GUID_LOPART(guid));
    if (!obj)
        return;
    obj->Use(_player);
}

void WorldSession::HandleTutorialFlag( WorldPacket & recv_data )
{
    uint32 iFlag;
    recv_data >> iFlag;

    uint32 wInt = (iFlag / 32);
    uint32 rInt = (iFlag % 32);
    if(wInt >= 7)
    {
        Disconnect();
        return;
    }

    m_tutorials[wInt] |= (1 << rInt);
    sLog.Debug("WorldSession","Received Tutorial Flag Set {%u}.", iFlag);
}

void WorldSession::HandleTutorialClear( WorldPacket & recv_data )
{
    for ( uint32 iI = 0; iI < 8; iI++)
        m_tutorials[iI] = 0xFFFFFFFF;
}

void WorldSession::HandleTutorialReset( WorldPacket & recv_data )
{
    for ( uint32 iI = 0; iI < 8; iI++)
        m_tutorials[iI] = 0x00;
}

void WorldSession::HandleSetSheathedOpcode( WorldPacket & recv_data )
{
    uint32 active;
    recv_data >> active;
    _player->SetByte(UNIT_FIELD_BYTES_2,0,(uint8)active);
}

void WorldSession::HandlePlayedTimeOpcode( WorldPacket & recv_data )
{
    uint8 send;
    recv_data >> send;

    uint32 playedt = (uint32)UNIXTIME - _player->m_playedtime[2];
    if(playedt)
    {
        _player->m_playedtime[0] += playedt;
        _player->m_playedtime[1] += playedt;
        _player->m_playedtime[2] += playedt;
    }

    WorldPacket data(SMSG_PLAYED_TIME, 9);
    data << (uint32)_player->m_playedtime[1];
    data << (uint32)_player->m_playedtime[0];
    data << uint8(send);
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode( WorldPacket & recv_data )
{
    CHECK_PACKET_SIZE( recv_data, 8 );
    CHECK_INWORLD_RETURN();

    uint64 guid;
    recv_data >> guid;

    Player* player = _player->GetMapMgr()->GetPlayer( (uint32)guid );
    if( player == NULL )
        return;

    WorldPacket data(SMSG_INSPECT_TALENT, 1000);
    data << player->GetNewGUID();
    player->BuildPlayerTalentsInfo(&data);

    // build items inspect part. could be sent separately as SMSG_INSPECT
    uint32 slotUsedMask = 0;
    uint16 enchantmentMask = 0;
    size_t maskPos = data.wpos();
    data << uint32(slotUsedMask);   // will be replaced later
    for(uint32 slot = 0; slot < EQUIPMENT_SLOT_END; slot++)
    {
        Item* item = player->GetItemInterface()->GetInventoryItem(slot);
        if( item )
        {
            slotUsedMask |= (1 << slot);

            data << uint32(item->GetEntry());

            size_t maskPosEnch = data.wpos();
            enchantmentMask = 0;
            data << uint16(enchantmentMask);

            for(uint32 ench = 0; ench < 12; ench++)
            {
                uint16 enchId = (uint16) item->GetUInt32Value( ITEM_FIELD_ENCHANTMENT_1_1 + (ench * 3));
                if( enchId )
                {
                    enchantmentMask |= (1 << ench);
                    data << uint16(enchId);
                }
            }
            *(uint16*)&data.contents()[maskPosEnch] = enchantmentMask;

            data << uint16(0);  // unk

            uint64 creatorguid = item->GetUInt64Value(ITEM_FIELD_CREATOR);
            if(creatorguid)
                data << WoWGuid(creatorguid);
            else
                data << uint8(0);

            data << uint32(0);  // unk
        }
    }
    *(uint32*)&data.contents()[maskPos] = slotUsedMask;
    SendPacket( &data );
}

void WorldSession::HandleSetActionBarTogglesOpcode(WorldPacket &recvPacket)
{
    CHECK_INWORLD_RETURN();
    uint8 cActionBarId;
    recvPacket >> cActionBarId;

    sLog.Debug("WorldSession","Received CMSG_SET_ACTIONBAR_TOGGLES for actionbar id %d.", cActionBarId);

    _player->SetByte(PLAYER_FIELD_BYTES, 2, cActionBarId);
}

void WorldSession::HandleSelfResurrectOpcode(WorldPacket& recv_data)
{
    uint32 self_res_spell = _player->GetUInt32Value(PLAYER_SELF_RES_SPELL);
    if(self_res_spell)
    {
        SpellEntry * sp = dbcSpell.LookupEntry(self_res_spell);
        if(sp != NULL)
        {
            Spell* s = new Spell(_player, sp, false, NULLAURA);
            SpellCastTargets tgt;
            tgt.m_unitTarget=_player->GetGUID();
            s->prepare(&tgt);
        }
    }
}

void WorldSession::HandleRandomRollOpcode(WorldPacket& recv_data)
{
    uint32 min, max;
    recv_data >> min >> max;

    sLog.outDebug("WORLD: Received MSG_RANDOM_ROLL: %u-%u", min, max);

    WorldPacket data(20);
    data.SetOpcode(MSG_RANDOM_ROLL);
    data << min << max;

    if(max < min)
        return;

    uint32 roll;

    // generate number
    roll = RandomUInt(max - min) + min + 1;

    // append to packet, and guid
    data << roll << _player->GetGUID();

    // send to set
    if(_player->InGroup())
        _player->GetGroup()->SendPacketToAll(&data);
    else
        GetPlayer()->SendMessageToSet(&data, true, true);
}

void WorldSession::HandleLootMasterGiveOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    uint32 itemid = 0;
    uint32 amt = 1;
    uint8 error = 0;
    SlotResult slotresult;

    Creature* pCreature = NULLCREATURE;
    Loot *pLoot = NULL;
    /* struct:
    {CLIENT} Packet: (0x02A3) CMSG_LOOT_MASTER_GIVE PacketSize = 17
    |------------------------------------------------|----------------|
    |00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|
    |------------------------------------------------|----------------|
    |39 23 05 00 81 02 27 F0 01 7B FC 02 00 00 00 00 |9#....'..{......|
    |00                                           |.               |
    -------------------------------------------------------------------

        uint64 creatureguid
        uint8  slotid
        uint64 target_playerguid

    */
    uint64 creatureguid, target_playerguid;
    uint8 slotid;
    recv_data >> creatureguid >> slotid >> target_playerguid;

    if(_player->GetGroup() == NULL || _player->GetGroup()->GetLooter() != _player->m_playerInfo)
        return;

    Player* player = _player->GetMapMgr()->GetPlayer((uint32)target_playerguid);
    if(!player)
        return;

    // cheaterz!
    if(_player->GetLootGUID() != creatureguid)
        return;

    //now its time to give the loot to the target player
    if(GUID_HIPART(GetPlayer()->GetLootGUID()) == HIGHGUID_TYPE_CREATURE)
    {
        pCreature = _player->GetMapMgr()->GetCreature(GUID_LOPART(creatureguid));
        if (!pCreature)return;
        pLoot=&pCreature->m_loot;
    }

    if(!pLoot)
        return;

    if (slotid >= pLoot->items.size())
    {
        sLog.outDebug("AutoLootItem: Player %s might be using a hack! (slot %d, size %d)",
                        GetPlayer()->GetName(), slotid, pLoot->items.size());
        return;
    }

    uint32 ffatype = pLoot->items.at(slotid).ffa_loot;
    if (ffatype != 2)
    {
        if (pLoot->items.at(slotid).has_looted.size())
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM,INV_ERR_ALREADY_LOOTED);
            return;
        }
    }
    else
    {
        //make sure this player can still loot it in case of ffa_loot
        LooterSet::iterator itr = pLoot->items.at(slotid).has_looted.find(player->GetLowGUID());

        if (itr != pLoot->items.at(slotid).has_looted.end())
        {
            GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM,INV_ERR_ALREADY_LOOTED);
            return;
        }
    }

    itemid = pLoot->items.at(slotid).item.itemproto->ItemId;
    ItemPrototype* it = pLoot->items.at(slotid).item.itemproto;

    if((error = player->GetItemInterface()->CanReceiveItem(it, 1, 0)))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, error);
        return;
    }

    if(pCreature)
        CALL_SCRIPT_EVENT(pCreature, OnLootTaken)(player, it);

    slotresult = player->GetItemInterface()->FindFreeInventorySlot(it);
    if(!slotresult.Result)
    {
        GetPlayer()->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_INVENTORY_FULL);
        return;
    }

    Item* item = objmgr.CreateItem( itemid, player);

    item->SetUInt32Value(ITEM_FIELD_STACK_COUNT,amt);
    if(pLoot->items.at(slotid).iRandomProperty!=NULL)
    {
        item->SetRandomProperty(pLoot->items.at(slotid).iRandomProperty->ID);
        item->ApplyRandomProperties(false);
    }
    else if(pLoot->items.at(slotid).iRandomSuffix != NULL)
    {
        item->SetRandomSuffix(pLoot->items.at(slotid).iRandomSuffix->id);
        item->ApplyRandomProperties(false);
    }

    if( player->GetItemInterface()->SafeAddItem(item,slotresult.ContainerSlot, slotresult.Slot) )
    {
        player->GetSession()->SendItemPushResult(item,false,true,true,true,slotresult.ContainerSlot,slotresult.Slot,1);
        sQuestMgr.OnPlayerItemPickup(player, item, amt);
    }
    else
    {
        item->DeleteMe();
        item = NULLITEM;
    }

    pLoot->items.at(slotid).has_looted.insert(player->GetLowGUID());

    // this gets sent to all looters
    if(ffatype == 0)
    {
        // this gets sent to all looters
        WorldPacket data(1);
        data.SetOpcode(SMSG_LOOT_REMOVED);
        data << slotid;
        Player* plr;
        for(LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); itr++)
        {
            if((plr = _player->GetMapMgr()->GetPlayer(*itr)))
                plr->GetSession()->SendPacket(&data);
        }
    }
}

void WorldSession::HandleLootRollOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    /* struct:

    {CLIENT} Packet: (0x02A0) CMSG_LOOT_ROLL PacketSize = 13
    |------------------------------------------------|----------------|
    |00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|
    |------------------------------------------------|----------------|
    |11 4D 0B 00 BD 06 01 F0 00 00 00 00 02       |.M...........   |
    -------------------------------------------------------------------

    uint64 creatureguid
    uint21 slotid
    uint8  choice

    */
    uint64 creatureguid;
    uint32 slotid;
    uint8 choice;
    recv_data >> creatureguid >> slotid >> choice;

    LootRoll* li = NULLROLL;

    uint32 guidtype = GUID_HIPART(creatureguid);
    if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
    {
        GameObject* pGO = _player->GetMapMgr()->GetGameObject(GUID_LOPART(creatureguid));
        if (!pGO)
            return;
        if (slotid >= pGO->m_loot.items.size() || pGO->m_loot.items.size() == 0)
            return;
        if (pGO->GetInfo() && pGO->GetInfo()->Type == GAMEOBJECT_TYPE_CHEST)
            li = pGO->m_loot.items[slotid].roll;
    }
    else if (guidtype == HIGHGUID_TYPE_CREATURE)
    {
        // Creatures
        Creature* pCreature = _player->GetMapMgr()->GetCreature(GUID_LOPART(creatureguid));
        if (!pCreature)
            return;

        if (slotid >= pCreature->m_loot.items.size() || pCreature->m_loot.items.size() == 0)
            return;

        li = pCreature->m_loot.items[slotid].roll;
    }
    else
        return;

    if(!li)
        return;

    li->PlayerRolled(_player->getPlayerInfo(), choice);
}

void WorldSession::HandleOpenItemOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 2);
    int8 slot, containerslot;
    recv_data >> containerslot >> slot;

    Item* pItem = _player->GetItemInterface()->GetInventoryItem(containerslot, slot);
    if(!pItem)
        return;

    // gift wrapping handler
    if(pItem->GetUInt32Value(ITEM_FIELD_GIFTCREATOR) && pItem->wrapped_item_id)
    {
        ItemPrototype * it = ItemPrototypeStorage.LookupEntry(pItem->wrapped_item_id);
        if(it == NULL)
            return;

        pItem->SetUInt32Value(ITEM_FIELD_GIFTCREATOR,0);
        pItem->SetUInt32Value(OBJECT_FIELD_ENTRY,pItem->wrapped_item_id);
        pItem->wrapped_item_id = 0;
        pItem->SetProto(it);
        pItem->Bind(ITEM_BIND_ON_PICKUP);
        if(it->MaxDurability)
        {
            pItem->SetUInt32Value(ITEM_FIELD_DURABILITY,it->MaxDurability);
            pItem->SetUInt32Value(ITEM_FIELD_MAXDURABILITY,it->MaxDurability);
        }

        pItem->m_isDirty=true;
        pItem->SaveToDB(containerslot,slot, false, NULL);
        return;
    }

    Lock *lock = dbcLock.LookupEntry( pItem->GetProto()->LockId );

    uint32 removeLockItems[8] = {0,0,0,0,0,0,0,0};

    if(lock) // locked item
    {
        for(int i=0; i<8;++i)
        {
            if(lock->locktype[i] == 1 && lock->lockmisc[i] > 0)
            {
                int8 slot = _player->GetItemInterface()->GetInventorySlotById(lock->lockmisc[i]);
                if(slot != ITEM_NO_SLOT_AVAILABLE && slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END)
                {
                    removeLockItems[i] = lock->lockmisc[i];
                }
                else
                {
                    _player->GetItemInterface()->BuildInventoryChangeError(pItem,NULLITEM,INV_ERR_ITEM_LOCKED);
                    return;
                }
            }
            else if(lock->locktype[i] == 2 && pItem->locked)
            {
                _player->GetItemInterface()->BuildInventoryChangeError(pItem,NULLITEM,INV_ERR_ITEM_LOCKED);
                return;
            }
        }
        for(int j=0; j<8;++j)
        {
            if(removeLockItems[j])
                _player->GetItemInterface()->RemoveItemAmt(removeLockItems[j],1);
        }
    }

    // fill loot
    _player->SetLootGUID(pItem->GetGUID());
    if( !pItem->m_looted )
    {
        // delete item from database, so we can't cheat
        pItem->DeleteFromDB();
        lootmgr.FillItemLoot(&pItem->m_loot, pItem->GetEntry(), _player->GetTeam());

        /*if(pItem->GetProto()->Lootgold > 0) // Gold can be looted from items. http://www.wowhead.com/?item=45724
            pItem->m_loot.gold = pItem->GetProto()->Lootgold;*/

        if(pItem->m_loot.gold)
            pItem->m_loot.gold = int32(float(pItem->m_loot.gold) * sWorld.getRate(RATE_MONEY));

        pItem->m_looted = true;
    }

    _player->SendLoot(pItem->GetGUID(), _player->GetMapId(), LOOT_CORPSE);
}

void WorldSession::HandleCompleteCinematic(WorldPacket& recv_data)
{
    // when a Cinematic is started the player is going to sit down, when its finished its standing up.
    _player->SetStandState(STANDSTATE_STAND);
};

void WorldSession::HandleResetInstanceOpcode(WorldPacket& recv_data)
{
    sInstanceMgr.ResetSavedInstances(_player);
}

void WorldSession::HandleToggleCloakOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    //////////////////////////
    //  PLAYER_FLAGS                                       = 3104 / 0x00C20 / 0000000000000000000110000100000
    //                                                                                           ^
    // This bit, on = toggled OFF, off = toggled ON.. :S

    //uint32 SetBit = 0 | (1 << 11);

    if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_NOCLOAK))
        _player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_NOCLOAK);
    else
        _player->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_NOCLOAK);
}

void WorldSession::HandleToggleHelmOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN();
    //////////////////////////
    //  PLAYER_FLAGS                                       = 3104 / 0x00C20 / 0000000000000000000110000100000
    //                                                                                            ^
    // This bit, on = toggled OFF, off = toggled ON.. :S

    //uint32 SetBit = 0 | (1 << 10);

    if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_NOHELM))
        _player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_NOHELM);
    else
        _player->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_NOHELM);
}

void WorldSession::HandleDungeonDifficultyOpcode(WorldPacket& recv_data)
{
    uint32 data;
    recv_data >> data;

    if(_player->GetGroup() && _player->IsGroupLeader())
    {
        WorldPacket pData;
        pData.Initialize(MSG_SET_DUNGEON_DIFFICULTY);
        pData << data;

        _player->iInstanceType = data;
        sInstanceMgr.ResetSavedInstances(_player);

        Group * m_Group = _player->GetGroup();

        m_Group->SetDifficulty(data);
        m_Group->Lock();
        for(uint32 i = 0; i < m_Group->GetSubGroupCount(); i++)
        {
            for(GroupMembersSet::iterator itr = m_Group->GetSubGroup(i)->GetGroupMembersBegin(); itr != m_Group->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
            {
                if((*itr)->m_loggedInPlayer)
                {
                    (*itr)->m_loggedInPlayer->iInstanceType = data;
                    (*itr)->m_loggedInPlayer->GetSession()->SendPacket(&pData);
                }
            }
        }
        m_Group->Unlock();
    }
    else if(!_player->GetGroup())
    {
        _player->iInstanceType = data;
        sInstanceMgr.ResetSavedInstances(_player);
    }
}

void WorldSession::HandleRaidDifficultyOpcode(WorldPacket& recv_data)
{
    uint32 data;
    recv_data >> data;

    if(_player->GetGroup() && _player->IsGroupLeader())
    {
        WorldPacket pData;
        pData.Initialize(MSG_SET_RAID_DIFFICULTY);
        pData << data;

        _player->iRaidType = data;
        Group * m_Group = _player->GetGroup();

        m_Group->SetRaidDifficulty(data);
        m_Group->Lock();
        for(uint32 i = 0; i < m_Group->GetSubGroupCount(); i++)
        {
            for(GroupMembersSet::iterator itr = m_Group->GetSubGroup(i)->GetGroupMembersBegin(); itr != m_Group->GetSubGroup(i)->GetGroupMembersEnd(); itr++)
            {
                if((*itr)->m_loggedInPlayer)
                {
                    (*itr)->m_loggedInPlayer->iRaidType = data;
                    (*itr)->m_loggedInPlayer->GetSession()->SendPacket(&pData);
                }
            }
        }
        m_Group->Unlock();
    }
    else if(!_player->GetGroup())
    {
        _player->iRaidType = data;
    }
}

void WorldSession::HandleSummonResponseOpcode(WorldPacket & recv_data)
{
    uint64 summonguid;
    bool agree;
    recv_data >> summonguid;
    recv_data >> agree;

    //Do we have a summoner?
    if(!_player->m_summoner)
    {
        SendNotification("Summoner guid has changed or does not exist.");
        return;
    }

    // Summoner changed?
    if(_player->m_summoner->GetGUID() != summonguid)
    {
        SendNotification("Summoner guid has changed or does not exist.");
        return;
    }

    //not during combat
    if(_player->CombatStatus.IsInCombat())
        return;

    //Map checks.
    MapInfo * inf = WorldMapInfoStorage.LookupEntry(_player->m_summonMapId);
    if(!inf)
        return;

    //are we summoning from witin the same instance?
    if( _player->m_summonInstanceId != _player->GetInstanceID() )
    {
        // if not, are we allowed on the summoners map?
        uint8 pReason = CheckTeleportPrerequisites(NULL, this, _player, inf->mapid);
        if( pReason )
        {
            SendNotification(NOTIFICATION_MESSAGE_NO_PERMISSION);
            return;
        }
    }
    if(agree)
    {
        if(!_player->SafeTeleport(_player->m_summonMapId, _player->m_summonInstanceId, _player->m_summonPos))
            SendNotification(NOTIFICATION_MESSAGE_FAILURE);

        _player->m_summoner = NULLOBJ;
        _player->m_summonInstanceId = _player->m_summonMapId = 0;
        return;
    }
    else
    {
        // Null-out the summoner
        _player->m_summoner = NULLOBJ;
        _player->m_summonInstanceId = _player->m_summonMapId = 0;
        return;
    }
}

void WorldSession::HandleDismountOpcode(WorldPacket& recv_data)
{
    sLog.Debug( "WORLD"," Received CMSG_DISMOUNT"  );

    if( !_player->IsInWorld() || _player->GetTaxiState())
        return;

    if( _player->IsMounted() )
        TO_UNIT(_player)->Dismount();
}

void WorldSession::HandleSetAutoLootPassOpcode(WorldPacket & recv_data)
{
    uint32 on;
    recv_data >> on;

    if( _player->IsInWorld() )
        _player->BroadcastMessage("Auto loot passing is now %s.", on ? "on" : "off");

    _player->m_passOnLoot = (on!=0) ? true : false;
}

void WorldSession::HandleRemoveGlyph(WorldPacket & recv_data)
{
    uint32 glyphSlot;
    recv_data >> glyphSlot;
    _player->UnapplyGlyph(glyphSlot);
    if(glyphSlot < GLYPHS_COUNT)
        _player->m_specs[_player->m_talentActiveSpec].glyphs[glyphSlot] = 0;
}

void WorldSession::HandleWorldStateUITimerUpdate(WorldPacket& recv_data)
{
    WorldPacket data(SMSG_WORLD_STATE_UI_TIMER_UPDATE, 4);
    data << (uint32)UNIXTIME;
    SendPacket(&data);
}

void WorldSession::HandleReadyForAccountDataTimes(WorldPacket& recv_data)
{
    sLog.Debug( "WORLD","Received CMSG_READY_FOR_ACCOUNT_DATA_TIMES" );
    SendAccountDataTimes(0x15);
}

void WorldSession::HandleFarsightOpcode(WorldPacket& recv_data)
{
    uint8 type;
    recv_data >> type;

    // TODO

    GetPlayer()->UpdateVisibility();
}

void WorldSession::HandleGameobjReportUseOpCode( WorldPacket& recv_data )
{
    if(!_player->IsInWorld()) // Teleporting? :O
    {
        SKIP_READ_PACKET(recv_data);
        return;
    }

    uint64 guid;
    recv_data >> guid;
    GameObject* gameobj = _player->GetMapMgr()->GetGameObject(GUID_LOPART(guid));
    if(gameobj != NULLGOB && gameobj->GetInfo())
    {
        // Gossip Script
        GossipScript* goscript = NULL;
        if((goscript = sScriptMgr.GetRegisteredGossipScript(GTYPEID_GAMEOBJECT, gameobj->GetEntry(), false)))
            goscript->GossipHello(gameobj, _player, true);

        if(gameobj->CanActivate())
            sQuestMgr.OnGameObjectActivate(_player, gameobj);
    }
}

void WorldSession::HandleTalentWipeConfirmOpcode( WorldPacket& recv_data )
{
    uint64 guid;
    recv_data >> guid;
    CHECK_INWORLD_RETURN();

    uint32 playerGold = _player->GetUInt32Value( PLAYER_FIELD_COINAGE );
    uint32 price = _player->CalcTalentResetCost(_player->GetTalentResetTimes());

    if( playerGold < price )
        return;

    _player->SetUInt32Value( PLAYER_FIELD_COINAGE, playerGold - price );

    _player->GetAchievementInterface()->HandleAchievementCriteriaTalentResetCostTotal( price );
    _player->GetAchievementInterface()->HandleAchievementCriteriaTalentResetCount();

    _player->CastSpell(_player, 14867, true);   // Spell: "Untalent Visual Effect"
    _player->CastSpell(_player, 46331, true);   // Spell: "Trainer: Untrain Talents"

    WorldPacket data( MSG_TALENT_WIPE_CONFIRM, 12); // You don't have any talent.
    data << uint64(0);
    data << uint32(0);
    SendPacket( &data );
    return;
}

void WorldSession::HandleCalendarGetCalendar(WorldPacket& recv_data)
{
    time_t cur_time = time(NULL);

    WorldPacket data(SMSG_CALENDAR_SEND_CALENDAR, 4+4*0+4+4*0+4+4);

    // TODO: calendar invite event output
    // TODO: calendar event output
    data << uint32(0);          //invite node count
    data << uint32(0);          //event count
    data << uint32(0);          //wtf??
    data << uint32(secsToTimeBitFields(cur_time));          // current time

    uint32 count = 0;
    size_t p_count = data.wpos();
    data << uint32(count);  // Calender shit.
    data.put<uint32>(p_count,count);
    data << uint32(1135753200); // Unix Time for when calendars started?
    data << uint32(0);          //  unk counter 4
    data << uint32(0);          // unk counter 5
    SendPacket(&data);
}

void WorldSession::HandleCalendarGetEvent(WorldPacket& recv_data)
{

}

void WorldSession::HandleCalendarGuildFilter(WorldPacket& recv_data)
{

}

void WorldSession::HandleCalendarArenaTeam(WorldPacket& recv_data)
{

}

void WorldSession::HandleCalendarAddEvent(WorldPacket& recv_data)
{
    SKIP_READ_PACKET(recv_data);    // set to end to avoid warnings spam

    std::string unk1, unk2;
    recv_data >> unk1;
    recv_data >> unk2;

    uint8   unk3, unk4;
    uint32  unk5, unk6, unk7, unk8, unk9, count = 0;
    recv_data >> unk3;
    recv_data >> unk4;
    recv_data >> unk5;
    recv_data >> unk6;
    recv_data >> unk7;
    recv_data >> unk8;
    recv_data >> unk9;
    if(!((unk9 >> 6) & 1))
    {
        recv_data >> count;
        if (count)
        {
            uint8 unk12,unk13;
            WoWGuid guid;
            for (uint i = 0; i < count; i++)
            {
                recv_data >> guid;
                recv_data >> unk12;
                recv_data >> unk13;
            }
        }
    }
}

void WorldSession::HandleCalendarUpdateEvent(WorldPacket& recv_data)
{
    SKIP_READ_PACKET(recv_data);

    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> std::string
    //recv_data >> std::string
    //recv_data >> uint8
    //recv_data >> uint8
    //recv_data >> uint32
    //recv_data >> uint32
    //recv_data >> uint32
    //recv_data >> uint32
    //recv_data >> uint32
}

void WorldSession::HandleCalendarRemoveEvent(WorldPacket& recv_data)
{
    SKIP_READ_PACKET(recv_data);

    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint32

}

void WorldSession::HandleCalendarCopyEvent(WorldPacket& recv_data)
{
    SKIP_READ_PACKET(recv_data);

    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint32

}

void WorldSession::HandleCalendarEventInvite(WorldPacket& recv_data)
{
    recv_data.hexlike();
    SKIP_READ_PACKET(recv_data);

    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> std::string
    //recv_data >> uint8
    //recv_data >> uint8

}

void WorldSession::HandleCalendarEventRsvp(WorldPacket& recv_data)
{
    recv_data.hexlike();
    SKIP_READ_PACKET(recv_data);

    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint32

}

void WorldSession::HandleCalendarEventRemoveInvite(WorldPacket& recv_data)
{
    recv_data.hexlike();
    SKIP_READ_PACKET(recv_data);

    //recv_data >> wowguid(guid);
    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint64
}

void WorldSession::HandleCalendarEventStatus(WorldPacket& recv_data)
{
    recv_data.hexlike();
    SKIP_READ_PACKET(recv_data);

    //recv_data >> wowguid(guid);
    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint32
}

void WorldSession::HandleCalendarEventModeratorStatus(WorldPacket& recv_data)
{
    recv_data.hexlike();
    SKIP_READ_PACKET(recv_data);

    //recv_data >> wowguid(guid);
    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint32
}

void WorldSession::HandleCalendarComplain(WorldPacket& recv_data)
{
    recv_data.hexlike();
    SKIP_READ_PACKET(recv_data);

    //recv_data >> uint64
    //recv_data >> uint64
    //recv_data >> uint64
}

void WorldSession::HandleCalendarGetNumPending(WorldPacket & /*recv_data*/)
{
    WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
    data << uint32(0);      // 0 - no pending invites, 1 - some pending invites
    SendPacket(&data);
}

void WorldSession::HandleMeetingStoneInfo(WorldPacket& )
{
    sLog.Debug("WORLD"," Received CMSG_MEETINGSTONE_INFO");
    //Used for LFR/LFG updates
}

void WorldSession::HandleHearthandResurrect(WorldPacket &recv_data)
{
    CHECK_INWORLD_RETURN();
    _player->ResurrectPlayer();
    _player->SafeTeleport(_player->GetBindMapId(),0,_player->GetBindPositionX(),_player->GetBindPositionY(),_player->GetBindPositionZ(),_player->GetOrientation());
}

uint8 WorldSession::CheckTeleportPrerequisites(AreaTrigger * pAreaTrigger, WorldSession * pSession, Player* pPlayer, uint32 mapid)
{
    MapInfo* pMapInfo = LimitedMapInfoStorage.LookupEntry(mapid);
    MapEntry* map = dbcMap.LookupEntry(mapid);

    //is this map enabled?
    if( pMapInfo == NULL || !pMapInfo->HasFlag(WMI_INSTANCE_ENABLED))
        return AREA_TRIGGER_FAILURE_UNAVAILABLE;

    //Do we need TBC expansion?
    if(!pSession->HasFlag(ACCOUNT_FLAG_XPACK_01) && pMapInfo->HasFlag(WMI_INSTANCE_XPACK_01))
        return AREA_TRIGGER_FAILURE_NO_BC;

    //Do we need WOTLK expansion?
    if(!pSession->HasFlag(ACCOUNT_FLAG_XPACK_02) && pMapInfo->HasFlag(WMI_INSTANCE_XPACK_02))
        return AREA_TRIGGER_FAILURE_NO_WOTLK;

    //Are we trying to enter a non-heroic instance in heroic mode?
    if(pMapInfo->type != INSTANCE_MULTIMODE && pMapInfo->type != INSTANCE_NULL)
        if((map->IsRaid() ? (pPlayer->iRaidType >= MODE_10PLAYER_HEROIC) : (pPlayer->iInstanceType >= MODE_5PLAYER_HEROIC)))
            return AREA_TRIGGER_FAILURE_NO_HEROIC;

    // These can be overridden by cheats/GM
    if(!pPlayer->triggerpass_cheat)
    {
        //Do we meet the areatrigger level requirements?
        if( pAreaTrigger != NULL && pAreaTrigger->required_level && pPlayer->getLevel() < pAreaTrigger->required_level)
            return AREA_TRIGGER_FAILURE_LEVEL;

        //Do we meet the map level requirements?
        if( pPlayer->getLevel() < pMapInfo->minlevel )
            return AREA_TRIGGER_FAILURE_LEVEL;

        //Do we need any quests?
        if( pMapInfo->required_quest && !( pPlayer->HasFinishedDailyQuest(pMapInfo->required_quest) || pPlayer->HasFinishedDailyQuest(pMapInfo->required_quest)))
            return AREA_TRIGGER_FAILURE_NO_ATTUNE_Q;

        //Do we need certain items?
        if( pMapInfo->required_item && !pPlayer->GetItemInterface()->GetItemCount(pMapInfo->required_item, true))
            return AREA_TRIGGER_FAILURE_NO_ATTUNE_I;

        //Do we need to be in a group?
        if((map->IsRaid() || pMapInfo->type == INSTANCE_MULTIMODE ) && !pPlayer->GetGroup())
            return AREA_TRIGGER_FAILURE_NO_GROUP;

        //Does the group have to be a raid group?
        if( map->IsRaid() && pPlayer->GetGroup()->GetGroupType() != GROUP_TYPE_RAID )
            return AREA_TRIGGER_FAILURE_NO_RAID;

        // Need http://www.wowhead.com/?spell=46591 to enter Magisters Terrace
        if( mapid == 585 && pPlayer->iInstanceType >= MODE_5PLAYER_HEROIC && !pPlayer->HasSpell(46591)) // Heroic Countenance
            return AREA_TRIGGER_FAILURE_NO_HEROIC;

        //Are we trying to enter a saved raid/heroic instance?
        if(map->IsRaid())
        {
            //Raid queue, did we reach our max amt of players?
            if( pPlayer->m_playerInfo && pMapInfo->playerlimit >= 5 && (int32)((pMapInfo->playerlimit - 5)/5) < pPlayer->m_playerInfo->subGroup)
                return AREA_TRIGGER_FAILURE_IN_QUEUE;

            //All Heroic instances are automatically unlocked when reaching lvl 80, no keys needed here.
            if( pPlayer->getLevel() < 80)
            {
                //otherwise we still need to be lvl 65 for heroic.
                if( pPlayer->iRaidType && pPlayer->getLevel() < uint32(pMapInfo->HasFlag(WMI_INSTANCE_XPACK_02) ? 80 : 70))
                    return AREA_TRIGGER_FAILURE_LEVEL_HEROIC;

                //and we might need a key too.
                bool reqkey = (pMapInfo->heroic_key[0]||pMapInfo->heroic_key[1])?true:false;
                bool haskey = (pPlayer->GetItemInterface()->GetItemCount(pMapInfo->heroic_key[0], false) || pPlayer->GetItemInterface()->GetItemCount(pMapInfo->heroic_key[1], false))?true:false;
                if(reqkey && !haskey)
                    return AREA_TRIGGER_FAILURE_NO_KEY;
            }
        }
    }

    if(!sHookInterface.OnCheckTeleportPrerequisites(pPlayer, mapid))
        return AREA_TRIGGER_FAILURE_UNAVAILABLE;

    // Nothing more to check, should be ok
    return AREA_TRIGGER_FAILURE_OK;
}

void WorldSession::SendGossipForObject(Object* pObject)
{
    list<QuestRelation *>::iterator it;
    std::set<uint32> ql;

    switch(pObject->GetTypeId())
    {
    case TYPEID_GAMEOBJECT:
        {
            GameObject* Go = TO_GAMEOBJECT(pObject);
            GossipScript* Script = sScriptMgr.GetRegisteredGossipScript(GTYPEID_GAMEOBJECT, Go->GetEntry());
            if(!Script)
                return;

            Script->GossipHello(Go, _player, true);
        }break;
    case TYPEID_ITEM:
        {
            Item* pItem = TO_ITEM(pObject);
            GossipScript* Script = sScriptMgr.GetRegisteredGossipScript(GTYPEID_ITEM, pItem->GetEntry());
            if(!Script)
                return;

            Script->GossipHello(pItem, _player, true);
        }break;
    case TYPEID_UNIT:
        {
            Creature* TalkingWith = TO_CREATURE(pObject);
            if(!TalkingWith)
                return;

            //stop when talked to for 3 min
            if(TalkingWith->GetAIInterface())
                TalkingWith->GetAIInterface()->StopMovement(180000);

            // unstealth meh
            if( _player->InStealth() )
                _player->m_AuraInterface.RemoveAllAurasOfType( SPELL_AURA_MOD_STEALTH );

            // reputation
            _player->Reputation_OnTalk(TalkingWith->m_faction);

            sLog.Debug( "WORLD"," Received CMSG_GOSSIP_HELLO from %u", TalkingWith->GetLowGUID());

            GossipScript* Script = sScriptMgr.GetRegisteredGossipScript(GTYPEID_CTR, TalkingWith->GetEntry());
            if(!Script)
                return;

            if (TalkingWith->isQuestGiver() && TalkingWith->HasQuests())
            {
                WorldPacket data(SMSG_GOSSIP_MESSAGE, 100);
                Script->GossipHello(TalkingWith, _player, false);
                if(!_player->CurrentGossipMenu)
                    return;

                _player->CurrentGossipMenu->BuildPacket(data);
                uint32 count = 0;
                size_t pos = data.wpos();
                data << uint32(count);
                for (it = TalkingWith->QuestsBegin(); it != TalkingWith->QuestsEnd(); it++)
                {
                    uint32 status = sQuestMgr.CalcQuestStatus(GetPlayer(), *it);
                    if (status >= QMGR_QUEST_CHAT)
                    {
                        if((*it)->qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
                            status = 8;

                        if (!ql.count((*it)->qst->id) )
                        {
                            ql.insert((*it)->qst->id);
                            count++;

                            uint32 icon;
                            uint32 questid = (*it)->qst->id;
                            switch(status)
                            {
                            case QMGR_QUEST_FINISHED:
                                icon = 4;
                                break;
                            case QMGR_QUEST_CHAT:
                                {
                                    if((*it)->qst->qst_is_repeatable)
                                        icon = 7;
                                    else
                                        icon = 8;
                                }break;
                            default:
                                icon = status;
                                break;
                            }

                            data << uint32( questid );
                            data << uint32( icon );
                            data << uint32( (*it)->qst->qst_max_level );
                            data << uint32( (*it)->qst->qst_flags );
                            data << uint8( (*it)->qst->qst_is_repeatable ? 1 : 0 );
                            data << (*it)->qst->qst_title;
                        }
                    }
                }
                data.put<uint32>(pos, count);
                SendPacket(&data);
                sLog.Debug( "WORLD"," Sent SMSG_GOSSIP_MESSAGE" );
            }
            else
            {
                Script->GossipHello(TalkingWith, _player, true);
            }
        }break;
    }
}
