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

bool ChatHandler::HandleRenameAllCharacter(const char * args, WorldSession * m_session)
{
    WoWGuid guid;
    uint32 uCount = 0;
    uint32 ts = getMSTime();
    QueryResult * result = CharacterDatabase.Query("SELECT guid, name FROM character_data");
    if( result )
    {
        do
        {
            guid = result->Fetch()[0].GetUInt64();
            const char * pName = result->Fetch()[1].GetString();
            size_t szLen = strlen(pName);

            if( !sWorld.VerifyName(pName, szLen) )
            {
                sLog.printf("renaming character %s, %u\n", pName, guid.getLow());
                if( Player* pPlayer = objmgr.GetPlayer(guid) )
                {
                    pPlayer->GetSession()->SystemMessage("Your character has had a force rename set, you will be prompted to rename your character at next login in conformance with server rules.");
                }

                CharacterDatabase.WaitExecute("UPDATE character_data SET customizeFlags = customizeFlags|0x01 WHERE guid = %u", guid);
                ++uCount;
            }

        } while (result->NextRow());
        delete result;
    }

    SystemMessage(m_session, "Procedure completed in %u ms. %u character(s) forced to rename.", getMSTime() - ts, uCount);
    return true;
}

void CapitalizeString(std::string& arg)
{
    if(arg.length() == 0) return;
    arg[0] = toupper(arg[0]);
    for(uint32 x = 1; x < arg.size(); ++x)
        arg[x] = tolower(arg[x]);
}

void WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
    if(m_asyncQuery)
        return;

    m_asyncQuery = true;
    AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP1<World, uint32>(World::getSingletonPtr(), &World::CharEnumDisplayData, GetAccountId()) );
    uint8 index = 0;
    uint32 count = 0, num = std::min<uint32>(MAXIMUM_CHAR_PER_ENUM, m_charData.size());
    for(auto itr = m_charData.begin(); itr != m_charData.end() && count < num; itr++, count++)
    {
        q->AddQuery("SELECT banTimeExpiration FROM banned_characters WHERE guid='%u'", itr->second->charGuid.getLow());
        q->AddQuery("SELECT entry, level FROM pet_data WHERE ownerguid='%u' AND active = 1", itr->second->charGuid.getLow());
        q->AddQuery("SELECT character_inventory.container, character_inventory.slot, item_data.itementry, item_enchantments.enchantid FROM character_inventory JOIN item_data ON character_inventory.itemguid = item_data.itemguid LEFT JOIN item_enchantments ON character_inventory.itemguid = item_enchantments.itemguid AND item_enchantments.enchantslot = 0 WHERE guid=%u AND container = -1 AND slot < 19", itr->second->charGuid.getLow());
        q->AddQuery("SELECT character_inventory.container, character_inventory.slot, item_enchantments.enchantid FROM character_inventory JOIN item_data ON character_inventory.itemguid = item_data.itemguid LEFT JOIN item_enchantments ON character_inventory.itemguid = item_enchantments.itemguid AND item_enchantments.enchantslot = 9 WHERE guid=%u AND container = -1 AND slot < 19", itr->second->charGuid.getLow());
        q->AddQuery("SELECT name, race, class, team, appearance, appearance2, appearance3, customizeFlags, deathState, level, mapId, instanceId, positionX, positionY, positionZ, orientation, zoneId, lastSaveTime FROM character_data WHERE guid = '%u' AND lastSaveTime != '%llu'", itr->second->charGuid.getLow(), itr->second->lastOnline);
    }
    CharacterDatabase.QueueAsyncQuery(q);
}

void WorldSession::CharEnumDisplayData(QueryResultVector& results)
{
    //Erm, reset it here in case player deleted his DK.
    m_hasDeathKnight = false;
    m_asyncQuery = false;

    ByteBuffer bitBuff, byteBuff;
    bitBuff.WriteBits(0, 23);
    bitBuff.WriteBit(1);

    // parse m_characters and build a mighty packet of
    // characters to send to the client.
    if( !m_charData.empty() )
    {
        struct
        {
            void clear() { displayid = 0; invtype = 0; enchantment = 0; }
            uint8 invtype;
            uint32 displayid;
            uint32 enchantment;
        } items[19];

        ItemPrototype *proto;
        uint32 count = 0, num = std::min<uint32>(MAXIMUM_CHAR_PER_ENUM, m_charData.size());
        bitBuff.reserve(num * 3);
        byteBuff.reserve(num * 381);
        bitBuff.WriteBits(num, 17);

        for(auto itr = m_charData.begin(); itr != m_charData.end() && count < num; itr++, count++)
        {
            for(uint8 i = 0; i < 19; i++)
                items[i].clear();

            if(results[count*5 + 4].result && !objmgr.GetPlayer(itr->second->charGuid))
                objmgr.UpdatePlayerData(itr->second->charGuid, results[count*5 + 4].result);

            uint32 flags = 0;
            PlayerInfo *info = itr->second;
            WoWGuid guildGuid(MAKE_NEW_GUID(info->GuildId, 0, HIGHGUID_TYPE_GUILD));
            uint8 hairStyle = ((info->charAppearance>>16)&0xFF), hairColor = ((info->charAppearance>>24)&0xFF), facialHair = (info->charAppearance2&0xFF), face = ((info->charAppearance>>8)&0xFF), skin = (info->charAppearance&0xFF), gender = info->charAppearance3&0xFF;

            uint32 player_flags = 0, customizationFlag = 0;
            if (info->charCustomizeFlags & 0x08)
                customizationFlag = 0x00100000; // Race change
            else if (info->charCustomizeFlags & 0x04)
                customizationFlag = 0x00010000; // Faction change
            else if (info->charCustomizeFlags & 0x02)
                customizationFlag = 0x00000001; // Recustomize

            if(info->charCustomizeFlags & 0x01)
                player_flags |= 0x4000;

            if( info->lastLevel > m_highestLevel )
                m_highestLevel = info->lastLevel;
            if( info->charClass == DEATHKNIGHT )
                m_hasDeathKnight = true;
            if(flags & PLAYER_FLAG_NOHELM)
                player_flags |= 0x400;
            if(flags & PLAYER_FLAG_NOCLOAK)
                player_flags |= 0x800;
            if(info->lastDeathState != 0)
                player_flags |= 0x2000;
            // Check to see if character is marked for unavailability
            if(info->lastLevel > m_maxLevel || !objmgr.CheckPlayerCreateInfo(info->charRace, info->charClass))
                player_flags |= 0x01000000;

            else if(QueryResult *res = results[count*5].result)
            {   // Expire time is infinite when 0
                uint64 expireTime = res->Fetch()[0].GetUInt64();
                if(expireTime == 0 || expireTime < UNIXTIME)
                {
                    player_flags |= 0x01000000;
                    m_bannedCharacters.insert(info->charGuid);
                } else m_bannedCharacters.erase(info->charGuid);
            } else m_bannedCharacters.erase(info->charGuid);

            uint32 petFamily = 0, petLevel = 0, petDisplay = 0;
            if(QueryResult *res = results[count*5 + 1].result)
            {
                if(CreatureData *petData = sCreatureDataMgr.GetCreatureData(res->Fetch()[0].GetUInt32()))
                {
                    petFamily = petData->family;
                    petDisplay = petData->displayInfo[0];
                }
                petLevel = res->Fetch()[1].GetUInt32();
            }

            // Parse queried item data
            if(QueryResult *res = results[count*5 + 2].result)
            {
                do
                {
                    Field *fields = res->Fetch();
                    int8 containerslot = fields[0].GetInt8(), slot = fields[1].GetInt8();
                    if(proto = sItemMgr.LookupEntry(fields[2].GetUInt32()))
                    {
                        // slot0 = head, slot14 = cloak
                        items[slot].invtype = proto->InventoryType;
                        items[slot].displayid = proto->DisplayInfoID;
                        if( SpellItemEnchantEntry *enc = dbcSpellItemEnchant.LookupEntry( fields[3].GetUInt32() ) )
                            items[slot].enchantment = enc->visualAura;
                    }
                } while(res->NextRow());
            }

            // Parse queried transmog data
            if(QueryResult *res = results[count*5 + 3].result)
            {
                do
                {
                    Field *fields = res->Fetch();
                    int8 containerslot = fields[0].GetInt8(), slot = fields[1].GetInt8();
                    if(items[slot].displayid && (proto = sItemMgr.LookupEntry(fields[2].GetUInt32())))
                        items[slot].displayid = proto->DisplayInfoID;
                } while(res->NextRow());
            }

            // Packet content flags
            bool firstLogin = false;
            bitBuff.WriteBitString(4, info->charGuid[3], guildGuid[1], guildGuid[7], guildGuid[2]);
            bitBuff.WriteBits(uint32(info->charName.length()), 7);
            bitBuff.WriteBitString(4, info->charGuid[4], info->charGuid[7], guildGuid[3], info->charGuid[5]);
            bitBuff.WriteBitString(4, guildGuid[6], info->charGuid[1], guildGuid[5], guildGuid[4]);
            bitBuff.WriteBitString(5, firstLogin, info->charGuid[0], info->charGuid[2], info->charGuid[6], guildGuid[0]);

            byteBuff << uint8(info->charClass);
            for( uint8 i = 0; i < EQUIPMENT_SLOT_END; i++ )
                byteBuff << items[i].invtype << items[i].displayid << uint32(items[i].enchantment);
            for( uint8 i = 0; i < 4; i++)
                byteBuff << uint8(0) << uint32(0) << uint32(0);

            byteBuff << uint32(petFamily);                  // Pet family
            byteBuff.WriteByteSeq(guildGuid[2]);
            byteBuff << uint8(itr->first);                  // List order
            byteBuff << uint8(hairStyle);                   // Hair style
            byteBuff.WriteByteSeq(guildGuid[3]);
            byteBuff << uint32(petDisplay);                 // Pet DisplayID
            byteBuff << uint32(player_flags);               // Character flags
            byteBuff << uint8(hairColor);                   // Hair color
            byteBuff.WriteByteSeq(info->charGuid[4]);
            byteBuff << uint32(info->lastMapID);            // Map Id
            byteBuff.WriteByteSeq(guildGuid[5]);
            byteBuff << float(info->lastPositionZ);         // Z
            byteBuff.WriteByteSeq(guildGuid[6]);
            byteBuff << uint32(petLevel);                   // Pet level
            byteBuff.WriteByteSeq(info->charGuid[3]);
            byteBuff << float(info->lastPositionY);         // Y
            byteBuff << uint32(customizationFlag);          // Character customization flags
            byteBuff << uint8(facialHair);                  // Facial hair
            byteBuff.WriteByteSeq(info->charGuid[7]);
            byteBuff << uint8(gender);                      // Gender
            byteBuff.append(info->charName.c_str(), info->charName.length()); // Name
            byteBuff << uint8(face);                        // Face
            byteBuff.WriteByteSeq(info->charGuid[0]);
            byteBuff.WriteByteSeq(info->charGuid[2]);
            byteBuff.WriteByteSeq(guildGuid[1]);
            byteBuff.WriteByteSeq(guildGuid[7]);
            byteBuff << float(info->lastPositionX);         // X
            byteBuff << uint8(skin);                        // Skin
            byteBuff << uint8(info->charRace);              // Race
            byteBuff << uint8(info->lastLevel);             // Level
            byteBuff.WriteByteSeq(info->charGuid[6]);
            byteBuff.WriteByteSeq(guildGuid[4]);
            byteBuff.WriteByteSeq(guildGuid[0]);
            byteBuff.WriteByteSeq(info->charGuid[5]);
            byteBuff.WriteByteSeq(info->charGuid[1]);
            byteBuff << uint32(info->lastZone);             // Zone id
        } // loop finish for character data
    } else bitBuff.WriteBits(0, 17);
    bitBuff.FlushBits();

    WorldPacket data(SMSG_CHARACTER_ENUM, 200);
    data.append(bitBuff.contents(), bitBuff.size());
    data.append(byteBuff.contents(), byteBuff.size());
    SendPacket( &data );
}

void WorldSession::HandleCharReorderOpcode( WorldPacket & recv_data )
{
    uint8 count = recv_data.ReadBits(10);
    // Limit count to 10, or character size
    if(count > m_charData.size() || count > MAXIMUM_CHAR_PER_ENUM)
    {
        SKIP_READ_PACKET(recv_data);
        return;
    }

    std::vector<WoWGuid> guidSet;
    for(uint8 i = 0; i < count; i++)
    {
        WoWGuid guid;
        recv_data.ReadGuidBitString(8, guid, 1, 4, 5, 3, 0, 7, 6, 2);
        guidSet.push_back(guid);
    }

    charDataLock.Acquire();
    // Get the amount of characters in our pool
    uint8 maxCount = m_charData.size();
    std::map<WoWGuid, PlayerInfo*> charData;
    for(auto itr = m_charData.begin(); itr != m_charData.end(); itr++)
        charData.insert(std::make_pair(itr->second->charGuid, itr->second));
    m_charData.clear();

    uint8 maxSlot = 0;
    std::stringstream ss;
    for(uint8 i = 0; i < count; i++)
    {
        recv_data.ReadGuidByteString(6, guidSet[i], 6, 5, 1, 4, 0, 3);
        uint8 slot = recv_data.read<uint8>()/10;
        recv_data.ReadGuidByteString(2, guidSet[i], 2, 7);
        maxSlot = std::max<uint8>(maxSlot, slot);

        PlayerInfo *info = NULL;
        if(charData.find(guidSet[i]) == charData.end())
            continue;
        info = charData.at(guidSet[i]);
        charData.erase(guidSet[i]);

        if(!ss.str().empty())
            ss << ", ";
        ss << "('" << GetAccountId() << "', '" << guidSet[i].getLow() << "', '" << uint32(slot) << "')";
        m_charData.insert(std::make_pair(slot, info));
    }

    // Clean up any extra data by appending it
    while(!charData.empty())
    {
        WoWGuid guid = charData.begin()->first;
        PlayerInfo *info = charData.begin()->second;
        charData.erase(charData.begin());

        if(!ss.str().empty())
            ss << ", ";
        uint32 slot = ++maxSlot;
        ss << "('" << GetAccountId() << "', '" << guid.getLow() << "', '" << slot << "')";
        m_charData.insert(std::make_pair(slot, info));
    }

    charDataLock.Release();
    CharacterDatabase.Execute("REPLACE INTO account_characters VALUES %s;", ss.str().c_str());
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
    std::string name;
    uint8 race, class_;

    recv_data >> name >> race >> class_;

    WorldPacket data(SMSG_CHARACTER_CREATE, 1);
    if(m_charData.size() >= 10)
    {
        //checking number of chars is useless since client will not allow to create more than 10 chars
        //as the 'create' button will not appear (unless we want to decrease maximum number of characters)
        data << uint8(CHAR_CREATE_ACCOUNT_LIMIT);
        SendPacket(&data);
        return;
    }

    if(!sWorld.VerifyName(name.c_str(), name.length()))
    {
        data << uint8(CHAR_CREATE_NAME_IN_USE);
        SendPacket(&data);
        return;
    }

    //reserved for console whisper
    if(name == "Console" ||  name == "console")
    {
        data << uint8(CHAR_CREATE_NAME_IN_USE);
        SendPacket(&data);
        return;
    }

    if(objmgr.GetPlayerInfoByName(name.c_str()) != 0)
    {
        data << uint8(CHAR_CREATE_NAME_IN_USE);
        SendPacket(&data);
        return;
    }

    if( class_ == DEATHKNIGHT )
    {
        if(!HasFlag(ACCOUNT_FLAG_XPACK_02))
        {
            data << uint8(CHAR_CREATE_EXPANSION);
            SendPacket(&data);
            return;
        }
        else if(!CanCreateDeathKnight() && !HasGMPermissions())
        {
            if(sWorld.m_deathKnightReqLevel > m_highestLevel)
                data << uint8(CHAR_CREATE_LEVEL_REQUIREMENT);
            else data << uint8(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
            SendPacket(&data);
            return;
        }
    }

    if(QueryResult * result = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str()))
    {
        if(result->Fetch()[0].GetUInt32() > 0)
        {
            // That name is banned!
            data << uint8(CHAR_NAME_PROFANE);
            SendPacket(&data);
            delete result;
            return;
        }
        delete result;
    }

    PlayerInfo *pn = objmgr.CreatePlayer();
    pn->charName = name.c_str();
    // correct capitalization
    CapitalizeString(pn->charName);
    pn->charRace = race;
    pn->charClass = class_;
    pn->charAppearance3 = recv_data.read<uint8>();
    pn->charAppearance = recv_data.read<uint32>();
    pn->charAppearance2 = recv_data.read<uint8>();
    uint8 outfitId = recv_data.read<uint8>();

    // Construct player pointer and check initialization
    Player* pNewChar = new Player(pn, this);
    if(!pNewChar->Initialize())
    {
        WorldPacket data(SMSG_CHARACTER_CREATE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);

        pNewChar->Destruct();
        return;
    }

    // Set base data and create in DB
    pNewChar->CreateInDatabase();

    // Store new player info
    objmgr.AddPlayerInfo(pn);

    // Clean up our pointer here
    pNewChar->Destruct();

    uint8 newIndex = m_charData.size()+1; // Add new character data
    for(auto itr = m_charData.begin(); itr != m_charData.end(); itr++)
        newIndex = std::max<uint8>(newIndex, itr->first+1);

    m_charData.insert(std::make_pair(newIndex, pn));
    CharacterDatabase.Execute("INSERT INTO account_characters VALUES('%u', '%u', '%u');", GetAccountId(), pn->charGuid.getLow(), newIndex);

    // CHAR_CREATE_SUCCESS
    data << uint8(CHAR_CREATE_SUCCESS);
    SendPacket(&data);

    sLogonCommHandler.UpdateAccountCount(GetAccountId(), 1);
}

void WorldSession::HandleCharDeleteOpcode( WorldPacket & recv_data )
{
    WoWGuid guid;
    recv_data >> guid;

    uint8 fail = CHAR_DELETE_SUCCESS;
    if(objmgr.GetPlayer(guid) != NULL)
        fail = CHAR_DELETE_FAILED; // "Char deletion failed"
    else if(!HasCharacterData(guid))
        fail = CHAR_DELETE_FAILED;
    else fail = DeleteCharacter(guid);

    OutPacket(SMSG_CHARACTER_DELETE, 1, &fail);
    if(fail == CHAR_DELETE_SUCCESS)
        sLogonCommHandler.UpdateAccountCount(GetAccountId(), -1);
}

uint8 WorldSession::DeleteCharacter(WoWGuid guid)
{
    PlayerInfo * inf = objmgr.GetPlayerInfo(guid);
    if( inf != NULL && inf->m_loggedInPlayer == NULL )
    {
        GuildMember* gMember = inf->GuildId ? guildmgr.GetGuildMember(inf->charGuid) : NULL;
        if(gMember != NULL && gMember->pRank->iId == 0)
            return CHAR_DELETE_FAILED_GUILD_LEADER;

        /*for(uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
            if( inf->arenaTeam[i] != NULL && inf->arenaTeam[i]->m_leader == guid )
                return CHAR_DELETE_FAILED_ARENA_CAPTAIN;*/

        guildmgr.RemoveMember(NULL, inf);
        for(uint8 i = 0; i < NUM_CHARTER_TYPES; i++)
        {
            if( inf->charterId[i] != 0 )
            {
                if(Charter *pCharter = guildmgr.GetCharter(inf->charterId[i], (CharterTypes)i))
                {
                    if( pCharter->LeaderGuid == inf->charGuid.getLow() )
                        pCharter->Destroy();
                    else pCharter->RemoveSignature(inf->charGuid.getLow());
                }
            }
        }

        charDataLock.Acquire();
        for(auto itr = m_charData.begin(); itr != m_charData.end(); itr++)
        {
            if(itr->second->charGuid == guid)
            {
                m_charData.erase(itr);
                break;
            }
        }
        charDataLock.Release();
        /*for(uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
            if( ArenaTeam *arTeam = inf->arenaTeam[i] )
                arTeam->RemoveMember(inf);*/

        sWorld.LogPlayer(this, "deleted character %s (GUID: %u)", inf->charName.c_str(), guid.getLow());
        Player::DeleteFromDB(guid);

        /* remove player info */
        objmgr.DeletePlayerInfo(guid.getLow());
        return CHAR_DELETE_SUCCESS;
    }

    return CHAR_DELETE_FAILED;
}

void WorldSession::HandleCharRenameOpcode(WorldPacket & recv_data)
{
    WorldPacket data(SMSG_CHARACTER_RENAME, recv_data.size() + 1);

    WoWGuid guid;
    std::string name;
    recv_data >> guid >> name;

    PlayerInfo * pi = objmgr.GetPlayerInfo(guid);
    if(pi == NULL)
        return;
    if(!(pi->charCustomizeFlags & 0x01))
        return;

    // Check name for rule violation.
    const char * szName=name.c_str();
    for(uint32 x = 0; x < strlen(szName); x++)
    {
        if((int)szName[x] < 65 || ((int)szName[x] > 90 && (int)szName[x] < 97) || (int)szName[x] > 122)
        {
            if((int)szName[x] < 65)
            {
                data << uint8(CHAR_NAME_TOO_SHORT); // Name is too short.
            }
            else if((int)szName[x] > 122) // Name is too long.
            {
                data << uint8(CHAR_NAME_TOO_LONG);
            }
            else
            {
                data << uint8(CHAR_NAME_FAILURE); // No clue.
            }
            data << guid << name;
            SendPacket(&data);
            return;
        }
    }

    if(QueryResult *result = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str()))
    {
        if(result->Fetch()[0].GetUInt32() > 0)
        {
            // That name is banned!
            data << uint8(CHAR_NAME_PROFANE);
            data << guid << name;
            SendPacket(&data);
            delete result;
            return;
        }
        delete result;
    }

    // Check if name is in use.
    if(objmgr.GetPlayerInfoByName(name.c_str()) != 0)
    {
        data << uint8(CHAR_NAME_FAILURE);
        data << guid << name;
        SendPacket(&data);
        return;
    }

    // correct capitalization
    CapitalizeString(name);
    objmgr.RenamePlayerInfo(pi, pi->charName.c_str(), name.c_str());

    sWorld.LogPlayer(this, "a rename was pending. Renamed character %s (GUID: %u) to %s.", pi->charName.c_str(), pi->charGuid.getLow(), name.c_str());
    pi->charName = name;
    pi->charCustomizeFlags &= ~0x01;

    // If we're here, the name is okay.
    CharacterDatabase.Query("UPDATE character_data SET name = '%s', customizeFlags = '%u' WHERE guid = '%u'", CharacterDatabase.EscapeString(name).c_str(), pi->charCustomizeFlags, guid.getLow());

    data << uint8(0) << guid << name;
    SendPacket(&data);
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WorldSession"," Recvd Player Logon Message" );

    WoWGuid guid;
    recv_data.ReadGuidBitString(8, guid, 2, 3, 0, 6, 4, 5, 1, 7);
    recv_data.ReadGuidByteString(8, guid, 2, 7, 0, 3, 5, 6, 1, 4);

    PlayerInfo *pInfo = NULL;
    // Check to see if we have data for this character
    for(auto itr = m_charData.begin(); itr != m_charData.end(); itr++)
    {
        if(itr->second->charGuid == guid)
        {
            pInfo = itr->second;
            break;
        }
    }

    uint8 response = CHAR_LOGIN_SUCCESS;
    //already active?
    if (m_loggingInPlayer || _player || sWorld.HasPendingWorldPush(this))
        response = CHAR_LOGIN_IN_PROGRESS;
    //Better validate this Guid before we create an invalid _player.
    else if(pInfo == NULL || !objmgr.CheckPlayerCreateInfo(pInfo->charRace, pInfo->charClass))
        response = CHAR_LOGIN_NO_CHARACTER;
    else if(pInfo->lastLevel > m_maxLevel)
        response = CHAR_LOGIN_DISABLED;
    else if((pInfo->charCustomizeFlags & 0x01) || m_bannedCharacters.find(guid) != m_bannedCharacters.end())
        response = CHAR_LOGIN_TEMPORARY_GM_LOCK;
    else if(objmgr.GetPlayer(guid) != NULL)
        response = CHAR_LOGIN_DUPLICATE_CHARACTER;
    else if (uint8 vError = sWorldMgr.ValidateMapId(pInfo->lastMapID))
    {
        if (vError == 2)
        {
            sWorld.QueueWorldPush(this, guid, pInfo->lastMapID);
            return;
        }
        response = CHAR_LOGIN_NO_WORLD;
    }

    if(response != CHAR_LOGIN_SUCCESS)
    {
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &response);
        return;
    }

    PlayerLoginProc(pInfo);
}

void WorldSession::PlayerLoginProc(PlayerInfo *info)
{
    //We have a valid Guid so let's create the player and login
    Player *plr = new Player(info, this);
    if(!plr->Initialize())
    {
        delete plr;
        return;
    }

    sLog.Debug("WorldSession", "Async loading player %u", info->charGuid.getLow());
    (m_loggingInPlayer = plr)->LoadFromDB();
}

void WorldSession::FullLogin(Player* plr)
{
    sLog.Debug("WorldSession", "Fully loading player %u", plr->GetLowGUID());
    SetPlayer(plr);
    m_loggingInPlayer = NULL;

    m_MoverWoWGuid = plr->GetGUID();

    /* world preload */
    WorldPacket data(SMSG_LOGIN_VERIFY_WORLD, 20);
    data << plr->GetMapId();
    data << plr->GetPositionX();
    data << plr->GetPositionY();
    data << plr->GetPositionZ();
    data << plr->GetOrientation();
    SendPacket(&data);

    SendAccountDataTimes(0xEA);

    data.Initialize(SMSG_FEATURE_SYSTEM_STATUS, 2);
    data << uint8(2) << uint32(1) << uint32(1) << uint32(2) << uint32(0);
    data.WriteBitString(6, 1, 1, 0, 1, 0, 0);
    if (1) // bit4
        data << uint32(1) << uint32(0) << uint32(10) << uint32(60);
    if (0) // bit5
        data << uint32(0) << uint32(0) << uint32(0);
    SendPacket(&data);

    data.Initialize(MSG_SET_DUNGEON_DIFFICULTY, 12);
    data << plr->iInstanceType << uint32(0x01) << uint32(plr->GetGroup() ? 1 : 0);
    SendPacket(&data);

    // Anti max level hack.
    if(sWorld.LevelCap_Custom_All && (plr->getLevel() > sWorld.LevelCap_Custom_All))
        plr->SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.LevelCap_Custom_All);

    // Enable certain GM abilities on login.
    if(HasGMPermissions())
    {
        if(CanUseCommand('z'))
        {
            plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);
        } else plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
    }

    data.Initialize(SMSG_LEARNED_DANCE_MOVES, 8);
    data << uint32(0) << uint32(0);
    SendPacket(&data);

    bool enter_world = true;

    // Find our transporter and add us if we're on one.
    if(!plr->GetTransportGuid().empty() && plr->GetTransportGuid().getHigh() == HIGHGUID_TYPE_MO_TRANSPORT)
    {
        if(sTransportMgr.CheckTransportPosition(plr->GetTransportGuid(), plr->GetMapId()))
        {
            sTransportMgr.UpdateTransportData(plr);

            // shit is sent in worldport ack.
            enter_world = false;
        }
    }

    sLog.Debug( "WorldSession","Player %s logged in.", plr->GetName());

    if(plr->GetTeam() == TEAM_HORDE)
        sWorld.HordePlayers++;
    else sWorld.AlliancePlayers++;

    if(sWorld.SendMovieOnJoin && false)//plr->m_FirstLogin && !HasGMPermissions())
        plr->SendCinematic(plr->myRace->CinematicId);

    sLog.Debug( "WorldSession","Created new player for existing players (%s)", plr->GetName() );

    // send friend list (for ignores)
    plr->Social_SendFriendList(7);

    // send to gms
    if(HasGMPermissions())
        sWorld.SendMessageToGMs(this, "%s%s %s (%s) is now online.|r", MSG_COLOR_GOLD, CanUseCommand('z') ? "Admin" : "GameMaster", plr->GetName(), GetAccountNameS(), GetPermissions());

    //Set current RestState
    if( plr->m_restData.isResting )       // We are in a resting zone, turn on Zzz
        plr->ApplyPlayerRestState(true);

    //Check if there is a time difference between lastlogoff and now
    if( plr->m_timeLogoff > 0 && plr->GetUInt32Value(UNIT_FIELD_LEVEL) < plr->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))   // if timelogoff = 0 then it's the first login
    {   //Calculate rest bonus
        if( uint32 timediff = uint32(UNIXTIME) - plr->m_timeLogoff )
            plr->AddCalculatedRestXP(timediff);
    }

    if(Group *group = plr->GetGroup())
        group->Update();

    if(enter_world && !plr->IsInWorld() && !sWorldMgr.PushToWorldQueue(plr) && !plr->EjectFromInstance())
        plr->TeleportToHomebind(); // If all else fails, send is back to homebind.

    sTracker.CheckPlayerForTracker(plr, true);

    objmgr.AddPlayer(plr);
}

bool ChatHandler::HandleRenameCommand(const char * args, WorldSession * m_session)
{
    // prevent buffer overflow
    if(strlen(args) > 100)
        return false;

    char name1[100], name2[100];
    if(sscanf(args, "%s %s", name1, name2) != 2)
        return false;

    if(!sWorld.VerifyName(name2, strlen(name2)))
    {
        RedSystemMessage(m_session, "That name is invalid or contains invalid characters.");
        return true;
    }

    std::string new_name = name2;
    PlayerInfo * pi = objmgr.GetPlayerInfoByName(name1);
    if(pi == 0)
    {
        RedSystemMessage(m_session, "Player not found with this name.");
        return true;
    }

    if( objmgr.GetPlayerInfoByName(new_name.c_str()) != NULL )
    {
        RedSystemMessage(m_session, "Player found with this name in use already.");
        return true;
    }

    objmgr.RenamePlayerInfo(pi, pi->charName.c_str(), new_name.c_str());
    pi->charName = new_name;

    // look in world for him
    if(Player* plr = objmgr.GetPlayer(pi->charGuid))
    {
        plr->SetName(new_name);
        BlueSystemMessageToPlr(plr, "%s changed your name to '%s'.", m_session->GetPlayer()->GetName(), new_name.c_str());
        plr->SaveToDB(false);
    } else CharacterDatabase.WaitExecute("UPDATE character_data SET name = '%s' WHERE guid = %u", CharacterDatabase.EscapeString(new_name).c_str(), pi->charGuid.getLow());

    GreenSystemMessage(m_session, "Changed name of '%s' to '%s'.", (char*)name1, (char*)name2);
    sWorld.LogGM(m_session, "renamed character %s (GUID: %u) to %s", (char*)name1, pi->charGuid.getLow(), (char*)name2);
    sWorld.LogPlayer(m_session, "GM renamed character %s (GUID: %u) to %s", (char*)name1, pi->charGuid.getLow(), ((char*)name2));
    return true;
}

void WorldSession::HandleAlterAppearance(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: CMSG_ALTER_APPEARANCE");

    uint32 hair, colour, facialhair;
    recv_data >> hair >> colour >> facialhair;

    BarberShopStyleEntry * Hair = dbcBarberShopStyle.LookupEntry(hair);
    BarberShopStyleEntry * facialHair = dbcBarberShopStyle.LookupEntry(facialhair);
    if(!facialHair || !Hair)
        return;

    uint8 newHair = Hair->hair_id;
    uint8 newFacialHair = facialHair->hair_id;
    uint32 level = _player->getLevel();
    float cost = 0;
    uint8 oldHair = _player->GetByte(PLAYER_BYTES, 2);
    uint8 oldColour = _player->GetByte(PLAYER_BYTES, 3);
    uint8 oldFacialHair = _player->GetByte(PLAYER_BYTES_2, 0);

    if(oldHair == newHair && oldColour == (uint8)colour && oldFacialHair == newFacialHair)
        return;

    if(level >= 100)
        level = 100;

    gtFloat *cutcosts = dbcBarberShopPrices.LookupEntry(level - 1);
    if(!cutcosts)
        return;

    WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);

    if(oldHair != newHair)
        cost += cutcosts->val;

    if((oldColour != colour) && (oldHair == newHair))
        cost += cutcosts->val * 0.5f;

    if(oldFacialHair != newFacialHair)
        cost += cutcosts->val * 0.75f;

    if(_player->GetUInt32Value(PLAYER_FIELD_COINAGE) < cost)
    {
        data << uint32(1); // not enough money
        SendPacket(&data);
        return;
    }

    data << uint32(0); // ok
    SendPacket(&data);
    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)cost);
    _player->SetByte(PLAYER_BYTES, 2, newHair);
    _player->SetByte(PLAYER_BYTES, 3, (uint8)colour);
    _player->SetByte(PLAYER_BYTES_2, 0, newFacialHair);
    _player->SetStandState(0);
}

void WorldSession::HandleCharCustomizeOpcode(WorldPacket & recv_data)
{
    WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, recv_data.size() + 1);
    WoWGuid guid;
    std::string name;
    recv_data >> guid >> name;

    uint8 gender, skin, hairColor, hairStyle, facialHair, face;
    recv_data >> gender >> skin >> hairColor >> hairStyle >> facialHair >> face;

    PlayerInfo* pi = objmgr.GetPlayerInfo(guid);
    if( pi == NULL )
        return;

    QueryResult* result = CharacterDatabase.Query("SELECT appearance2 FROM character_data WHERE guid = '%u'", guid.getLow());
    if(!result)
        return;

    if(name != pi->charName)
    {
        // Check name for rule violation.
        const char * szName = name.c_str();
        for(uint32 x = 0; x < strlen(szName); ++x)
        {
            if(int(szName[x]) || (int(szName[x]) > 90 && int(szName[x]) < 97) || int(szName[x]) > 122)
            {
                data << uint8(0x32);
                data << guid << name;
                SendPacket(&data);
                return;
            }
        }

        QueryResult * result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
        if(result2)
        {
            if(result2->Fetch()[0].GetUInt32() > 0)
            {
                // That name is banned!
                data << uint8(0x31);
                data << guid << name;
                SendPacket(&data);
                return;
            }
            delete result2;
        }

        // Check if name is in use.
        if(objmgr.GetPlayerInfoByName(name.c_str()) != 0)
        {
            data << uint8(0x32);
            data << guid << name;
            SendPacket(&data);
            return;
        }

        // correct capitalization
        CapitalizeString(name);
        objmgr.RenamePlayerInfo(pi, pi->charName.c_str(), name.c_str());
        pi->charName = name;

        CharacterDatabase.Execute("UPDATE character_data SET name = '%s' WHERE guid = '%u'", CharacterDatabase.EscapeString(name).c_str(), guid.getLow());
    }

    Field* fields = result->Fetch();
    uint32 player_bytes2 = fields[0].GetUInt32();
    player_bytes2 &= ~0xFF;
    player_bytes2 |= facialHair;
    CharacterDatabase.Execute("UPDATE character_data SET appearance3 = '%u', appearance = '%u', appearance2 = '%u', customizeFlags = '0', lastSaveTime = '%llu' WHERE guid = '%u'", gender, skin | (face << 8) | (hairStyle << 16) | (hairColor << 24), player_bytes2, uint64(UNIXTIME), guid.getLow());
    delete result;

    //WorldPacket data(SMSG_CHAR_CUSTOMIZE, recv_data.size() + 1);
    data << uint8(0);
    data << guid;
    data << name;
    data << uint8(gender);
    data << uint8(skin);
    data << uint8(face);
    data << uint8(hairStyle);
    data << uint8(hairColor);
    data << uint8(facialHair);
    SendPacket(&data);
}

void WorldSession::HandleRandomizeCharNameOpcode(WorldPacket &recvData)
{
    uint8 gender, race;
    recvData >> race >> gender;

    WorldPacket data(SMSG_RANDOMIZE_CHAR_NAME, 10);
    data.WriteBit(0); // unk
    if (Player::IsValidRace(race) && Player::IsValidGender(gender))
    {
        std::string name = "";//sWorld.GetRandomCharacterName(race, gender);
        data.WriteBits(name.size(), 7);
        data.WriteString(name);
    } else data.WriteBits(0, 7);

    SendPacket(&data);
}

void WorldSession::HandleEquipmentSetSave(WorldPacket &recv_data)
{

}

void WorldSession::HandleEquipmentSetDelete(WorldPacket &recv_data)
{

}

void WorldSession::HandleEquipmentSetUse(WorldPacket &recv_data)
{

}
