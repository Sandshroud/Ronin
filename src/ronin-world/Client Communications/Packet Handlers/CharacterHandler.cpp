/***
 * Demonstrike Core
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
                printf("renaming character %s, %u\n", pName, guid.getLow());
                if( Player* pPlayer = objmgr.GetPlayer(guid) )
                {
                    pPlayer->GetSession()->SystemMessage("Your character has had a force rename set, you will be prompted to rename your character at next login in conformance with server rules.");
                }

                CharacterDatabase.WaitExecute("UPDATE character_data SET forced_rename_pending = 1 WHERE guid = %u", guid);
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

void WorldSession::CharacterEnumProc(QueryResult * result)
{
    uint32 num = 0;
    m_asyncQuery = false;

    //Erm, reset it here in case player deleted his DK.
    m_hasDeathKnight = false;
    m_characterMapIds.clear();

    ByteBuffer bitBuff, byteBuff;
    bitBuff.WriteBits(0, 23);
    bitBuff.WriteBit(1);

    // parse m_characters and build a mighty packet of
    // characters to send to the client.
    if( result )
    {
        struct
        {
            void clear() { displayid = 0; invtype = 0; enchantment = 0; }
            uint8 invtype;
            uint32 displayid;
            uint32 enchantment;
        } items[19];

        num = result->GetRowCount();
        bitBuff.reserve(num * 3);
        byteBuff.reserve(num * 381);
        bitBuff.WriteBits(num, 17);

        do
        {
            for(uint8 i = 0; i < 19; i++)
                items[i].clear();

            Field *fields = result->Fetch();
            std::string name = fields[7].GetString();
            WoWGuid charGuid = fields[0].GetUInt64();
            WoWGuid guildGuid = fields[14].GetUInt32() ? MAKE_NEW_GUID(fields[14].GetUInt32(), 0, HIGHGUID_TYPE_GUILD) : 0;
            uint8 _race = fields[2].GetUInt8(), _class = fields[3].GetUInt8(), _level = fields[1].GetUInt8();
            uint32 flags = 0, _bytes1 = fields[5].GetUInt32(), _bytes2 = fields[6].GetUInt32();
            uint32 mapId = fields[11].GetUInt32(), zoneId = fields[12].GetUInt32();
            float x = fields[8].GetFloat(), y = fields[9].GetFloat(), z = fields[10].GetFloat();
            uint8 hairStyle = ((_bytes1>>16)&0xFF), hairColor = ((_bytes1>>24)&0xFF), facialHair = (_bytes2&0xFF), face = ((_bytes1>>8)&0xFF), skin = (_bytes1&0xFF);
            m_characterMapIds.insert(std::make_pair(charGuid, mapId));

            uint32 customizationFlag = 0;
            if (false) customizationFlag = 0x01;
            else if (false) customizationFlag = 0x10000;
            else if (false) customizationFlag = 0x100000;

            if( _level > m_highestLevel )
                m_highestLevel = _level;
            if( _class == DEATHKNIGHT )
                m_hasDeathKnight = true;
            uint32 player_flags = 0;
            if(flags & PLAYER_FLAG_NOHELM)
                player_flags |= 0x400;
            if(flags & PLAYER_FLAG_NOCLOAK)
                player_flags |= 0x800;
            if(fields[13].GetUInt32() != 0)
                player_flags |= 0x2000;
            /*if(fields[14].GetUInt32() != 0)
                player_flags |= 0x4000;
            uint64 banned = fields[13].GetUInt64();
            if(banned && (banned < 10 || banned > UNIXTIME))
                player_flags |= 0x1000000;*/

            QueryResult *res = NULL;
            uint32 petFamily = 0, petLevel = 0, petDisplay = 0;
            if(res = CharacterDatabase.Query("SELECT entry, level FROM pet_data WHERE ownerguid='%u' AND active = 1", charGuid.getLow()))
            {
                if(CreatureData *petData = sCreatureDataMgr.GetCreatureData(res->Fetch()[0].GetUInt32()))
                {
                    petFamily = petData->family;
                    petDisplay = petData->displayInfo[0];
                }
                petLevel = res->Fetch()[1].GetUInt32();
                delete res;
            }

            if(res = CharacterDatabase.Query("SELECT character_inventory.container, character_inventory.slot, item_data.itementry, item_enchantments.enchantid FROM character_inventory JOIN item_data ON character_inventory.itemguid = item_data.itemguid LEFT JOIN item_enchantments ON character_inventory.itemguid = item_enchantments.itemguid AND item_enchantments.enchantslot = 0 WHERE guid=%u AND container = -1 AND slot < 19", charGuid.getLow()))
            {
                do
                {
                    Field *fields = res->Fetch();
                    int8 containerslot = fields[0].GetInt8(), slot = fields[1].GetInt8();
                    if(ItemPrototype *proto = sItemMgr.LookupEntry(fields[2].GetUInt32()))
                    {
                        // slot0 = head, slot14 = cloak
                        items[slot].invtype = proto->InventoryType;
                        items[slot].displayid = proto->DisplayInfoID;
                        if( SpellItemEnchantEntry *enc = dbcSpellItemEnchant.LookupEntry( fields[3].GetUInt32() ) )
                            items[slot].enchantment = enc->visualAura;
                    }
                } while(res->NextRow());
                delete res;
            }

            // Packet content flags
            bool firstLogin = false;
            bitBuff.WriteBitString(4, charGuid[3], guildGuid[1], guildGuid[7], guildGuid[2]);
            bitBuff.WriteBits(uint32(name.length()), 7);
            bitBuff.WriteBitString(4, charGuid[4], charGuid[7], guildGuid[3], charGuid[5]);
            bitBuff.WriteBitString(4, guildGuid[6], charGuid[1], guildGuid[5], guildGuid[4]);
            bitBuff.WriteBitString(5, firstLogin, charGuid[0], charGuid[2], charGuid[6], guildGuid[0]);

            byteBuff << uint8(_class);
            for( uint8 i = 0; i < EQUIPMENT_SLOT_END; i++ )
                byteBuff << items[i].invtype << items[i].displayid << uint32(items[i].enchantment);
            for( uint8 i = 0; i < 4; i++)
                byteBuff << uint8(0) << uint32(0) << uint32(0);

            byteBuff << uint32(petFamily);                  // Pet family
            byteBuff.WriteByteSeq(guildGuid[2]);
            byteBuff << uint8(0);                           // List order
            byteBuff << uint8(hairStyle);                   // Hair style
            byteBuff.WriteByteSeq(guildGuid[3]);
            byteBuff << uint32(petDisplay);                 // Pet DisplayID
            byteBuff << uint32(player_flags);               // Character flags
            byteBuff << uint8(hairColor);                   // Hair color
            byteBuff.WriteByteSeq(charGuid[4]);
            byteBuff << uint32(mapId);                      // Map Id
            byteBuff.WriteByteSeq(guildGuid[5]);
            byteBuff << float(z);                           // Z
            byteBuff.WriteByteSeq(guildGuid[6]);
            byteBuff << uint32(petLevel);                   // Pet level
            byteBuff.WriteByteSeq(charGuid[3]);
            byteBuff << float(y);                           // Y
            byteBuff << uint32(customizationFlag);          // Character customization flags
            byteBuff << uint8(facialHair);                  // Facial hair
            byteBuff.WriteByteSeq(charGuid[7]);
            byteBuff << uint8(fields[4].GetUInt8());        // Gender
            byteBuff.append(name.c_str(), name.length());   // Name
            byteBuff << uint8(face);                        // Face
            byteBuff.WriteByteSeq(charGuid[0]);
            byteBuff.WriteByteSeq(charGuid[2]);
            byteBuff.WriteByteSeq(guildGuid[1]);
            byteBuff.WriteByteSeq(guildGuid[7]);
            byteBuff << float(x);                           // X
            byteBuff << uint8(skin);                        // Skin
            byteBuff << uint8(_race);                       // Race
            byteBuff << uint8(_level);                      // Level
            byteBuff.WriteByteSeq(charGuid[6]);
            byteBuff.WriteByteSeq(guildGuid[4]);
            byteBuff.WriteByteSeq(guildGuid[0]);
            byteBuff.WriteByteSeq(charGuid[5]);
            byteBuff.WriteByteSeq(charGuid[1]);
            byteBuff << uint32(zoneId);                     // Zone id
        } while( result->NextRow() );
    } else bitBuff.WriteBits(0, 17);
    bitBuff.FlushBits();

    WorldPacket data(SMSG_CHARACTER_ENUM, 200);
    data.append(bitBuff.contents(), bitBuff.size());
    data.append(byteBuff.contents(), byteBuff.size());
    SendPacket( &data );
}

void WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
    if( m_asyncQuery )      // should be enough
        return;

    AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP1<World, uint32>(World::getSingletonPtr(), &World::CharacterEnumProc, GetAccountId()) );
    q->AddQuery("SELECT guid, level, race, class, gender, bytes, bytes2, name, positionX, positionY, positionZ, mapId, zoneId, deathstate, guild_members.guildid FROM character_data LEFT JOIN guild_members ON character_data.guid = guild_members.playerid WHERE acct=%u ORDER BY guid ASC LIMIT 10", GetAccountId());
    m_asyncQuery = true;
    CharacterDatabase.QueueAsyncQuery(q);
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
    std::string name;
    uint8 race, class_;

    recv_data >> name >> race >> class_;
    recv_data.rpos(0);

    WorldPacket data(SMSG_CHARACTER_CREATE, 1);
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
    // loading characters

    //checking number of chars is useless since client will not allow to create more than 10 chars
    //as the 'create' button will not appear (unless we want to decrease maximum number of characters)
    Player* pNewChar = objmgr.CreatePlayer();
    pNewChar->SetSession(this);
    if(!pNewChar->Create( recv_data ))
    {
        // failed.
        pNewChar->ok_to_remove = true;
        pNewChar->Destruct();
        pNewChar = NULL;
        return;
    }

    pNewChar->UnSetBanned();
    pNewChar->addSpell(22027);    // Remove Insignia

    pNewChar->SaveToDB(true);

    PlayerInfo *pn = new PlayerInfo(pNewChar->GetGUID());
    pn->accountId = GetAccountId();
    pn->charName = pNewChar->GetName();
    pn->charClass = pNewChar->getClass();
    pn->charRace = pNewChar->getRace();
    pn->charGender = pNewChar->getGender();
    pn->charTeam = pNewChar->GetTeam();
    pn->lastLevel = pNewChar->getLevel();
    pn->lastZone = pNewChar->GetZoneId();
    pn->lastOnline = UNIXTIME;
    objmgr.AddPlayerInfo(pn);

    pNewChar->ok_to_remove = true;
    pNewChar->Destruct();
    pNewChar = NULL;

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
        QueryResult * result = CharacterDatabase.Query("SELECT name FROM character_data WHERE guid = %u AND acct = %u", guid, _accountId);
        if(result == NULL)
            return CHAR_DELETE_FAILED;
        std::string name = result->Fetch()[0].GetString();
        delete result;

        GuildMember* gMember = inf->GuildId ? guildmgr.GetGuildMember(inf->charGuid) : NULL;
        if(gMember != NULL && gMember->pRank->iId == 0)
            return CHAR_DELETE_FAILED_GUILD_LEADER;

        for(uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
            if( inf->arenaTeam[i] != NULL && inf->arenaTeam[i]->m_leader == guid )
                return CHAR_DELETE_FAILED_ARENA_CAPTAIN;

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

        for(uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
            if( ArenaTeam *arTeam = inf->arenaTeam[i] )
                arTeam->RemoveMember(inf);

        sWorld.LogPlayer(this, "deleted character %s (GUID: %u)", name.c_str(), guid.getLow());
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

    QueryResult * result = CharacterDatabase.Query("SELECT forced_rename_pending FROM character_data WHERE guid = %u AND acct = %u", guid.getLow(), _accountId);
    if(result == NULL)
        return;
    delete result;

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

    QueryResult * result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
    if(result2)
    {
        if(result2->Fetch()[0].GetUInt32() > 0)
        {
            // That name is banned!
            data << uint8(CHAR_NAME_PROFANE);
            data << guid << name;
            SendPacket(&data);
            delete result2;
            return;
        }
        delete result2;
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

    // If we're here, the name is okay.
    CharacterDatabase.Query("UPDATE character_data SET name = \'%s\',  forced_rename_pending  = 0 WHERE guid = %u AND acct = %u", name.c_str(), guid.getLow(), _accountId);

    data << uint8(0) << guid << name;
    SendPacket(&data);
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WorldSession"," Recvd Player Logon Message" );

    WoWGuid guid;
    guid[2] = recv_data.ReadBit();
    guid[3] = recv_data.ReadBit();
    guid[0] = recv_data.ReadBit();
    guid[6] = recv_data.ReadBit();
    guid[4] = recv_data.ReadBit();
    guid[5] = recv_data.ReadBit();
    guid[1] = recv_data.ReadBit();
    guid[7] = recv_data.ReadBit();
    recv_data.ReadByteSeq(guid[2]);
    recv_data.ReadByteSeq(guid[7]);
    recv_data.ReadByteSeq(guid[0]);
    recv_data.ReadByteSeq(guid[3]);
    recv_data.ReadByteSeq(guid[5]);
    recv_data.ReadByteSeq(guid[6]);
    recv_data.ReadByteSeq(guid[1]);
    recv_data.ReadByteSeq(guid[4]);

    //Better validate this Guid before we create an invalid _player.
    uint8 response = CHAR_LOGIN_NO_CHARACTER;

    //already active?
    if(m_characterMapIds.find(guid) == m_characterMapIds.end())
        response = CHAR_LOGIN_NO_CHARACTER;
    else if(WorldMapInfoStorage.LookupEntry(m_characterMapIds.at(guid)) == NULL)
        response = CHAR_LOGIN_NO_WORLD;
    else if(objmgr.GetPlayer(guid) != NULL || m_loggingInPlayer || _player)
        response = CHAR_LOGIN_DUPLICATE_CHARACTER;
    else if( PlayerInfo * plrInfo = objmgr.GetPlayerInfo(guid) )
        response = CHAR_LOGIN_SUCCESS;

    if(response != CHAR_LOGIN_SUCCESS)
    {
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &response);
        return;
    }

    //We have a valid Guid so let's create the player and login
    Player* plr = new Player(guid);
    plr->Init();
    plr->SetSession(this);
    m_bIsWLevelSet = false;

    sLog.Debug("WorldSession", "Async loading player %u", guid.getLow());
    m_loggingInPlayer = plr;
    plr->LoadFromDB();
}

void WorldSession::FullLogin(Player* plr)
{
    sLog.Debug("WorldSession", "Fully loading player %u", plr->GetLowGUID());
    SetPlayer(plr);
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
    data << uint32(0) << uint32(0x01) << uint32(0);
    SendPacket(&data);

    // Anti max level hack.
    if(sWorld.LevelCap_Custom_All && (plr->getLevel() > sWorld.LevelCap_Custom_All))
        plr->SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.LevelCap_Custom_All);

    // Enable certain GM abilities on login.
    if(HasGMPermissions())
    {
        plr->bGMTagOn = true;
        plr->m_isGmInvisible = true;
        plr->m_invisible = true;
        plr->bInvincible = true;
        if(CanUseCommand('z'))
        {
            plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);
            plr->triggerpass_cheat = true; // Enable for admins automatically.
        } else plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
    }

    for(uint32 z = 0; z < NUM_ARENA_TEAM_TYPES; ++z)
    {
        if(plr->m_playerInfo->arenaTeam[z] != NULL)
        {
            plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z*6), plr->m_playerInfo->arenaTeam[z]->m_id);
            if(plr->m_playerInfo->arenaTeam[z]->m_leader == plr->GetGUID())
                plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z*6) + 1, 0);
            else plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z*6) + 1, 1);
        }
    }

    data.Initialize(SMSG_LEARNED_DANCE_MOVES, 8);
    data << uint32(0) << uint32(0);
    SendPacket(&data);

    // Set TIME OF LOGIN
    CharacterDatabase.Execute("UPDATE character_data SET online = 1 WHERE guid = %u" , plr->GetLowGUID());

    bool enter_world = true;

    // Find our transporter and add us if we're on one.
    if(!plr->GetTransportGuid().empty())
    {
        WoWGuid transGuid = plr->GetTransportGuid();
        Transporter* pTrans = objmgr.GetTransporter(transGuid.getLow());
        if(pTrans)
        {
            if(plr->isDead())
                plr->RemoteRevive();

            float c_tposx, c_tposy, c_tposz, c_tposo;
            plr->GetMovementInterface()->GetTransportPosition(c_tposx, c_tposy, c_tposz, c_tposo);
            c_tposx += pTrans->GetPositionX();
            c_tposy += pTrans->GetPositionY();
            c_tposz += pTrans->GetPositionZ();

            if(plr->GetMapId() != pTrans->GetMapId())   // loaded wrong map
            {
                plr->SetMapId(pTrans->GetMapId());

                WorldPacket dataw(SMSG_NEW_WORLD, 20);
                dataw << c_tposx << c_tposo << c_tposy;
                dataw << pTrans->GetMapId() << c_tposz;
                SendPacket(&dataw);

                // shit is sent in worldport ack.
                enter_world = false;
            }

            plr->SetPosition(c_tposx, c_tposy, c_tposz, c_tposo);
            plr->m_CurrentTransporter = pTrans;
            pTrans->AddPlayer(plr);
        }
    }

    sLog.Debug( "WorldSession","Player %s logged in.", plr->GetName());

    if(plr->GetTeam() == 1)
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
    if( plr->m_isResting)       // We are in a resting zone, turn on Zzz
        plr->ApplyPlayerRestState(true);

    //Check if there is a time difference between lastlogoff and now
    if( plr->m_timeLogoff > 0 && plr->GetUInt32Value(UNIT_FIELD_LEVEL) < plr->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))   // if timelogoff = 0 then it's the first login
    {   //Calculate rest bonus
        if( uint32 timediff = uint32(UNIXTIME) - plr->m_timeLogoff )
            plr->AddCalculatedRestXP(timediff);
    }

    if(Group *group = plr->GetGroup())
        group->Update();

    if(enter_world && !plr->IsInWorld())
        sWorldMgr.PushToWorldQueue(plr);

    sTracker.CheckPlayerForTracker(plr, true);

    objmgr.AddPlayer(plr);

    m_loggingInPlayer = NULL;
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

    QueryResult* result = CharacterDatabase.Query("SELECT bytes2 FROM character_data WHERE guid = '%u'", guid.getLow());
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
    CharacterDatabase.Execute("UPDATE character_data SET gender = '%u', bytes = '%u', bytes2 = '%u', customizable = '0' WHERE guid = '%u'", gender, skin | (face << 8) | (hairStyle << 16) | (hairColor << 24), player_bytes2, guid.getLow());
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
