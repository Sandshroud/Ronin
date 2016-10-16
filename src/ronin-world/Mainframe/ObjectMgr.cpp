/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( ObjectMgr );

ObjectMgr::ObjectMgr()
{
    m_hiItemGuid = 0;
    m_hiGroupId = 1;
    m_mailid = 0;
    m_hiPlayerGuid = 0;
    m_hiCorpseGuid = 0;
    m_equipmentSetGuid = 0;
}

ObjectMgr::~ObjectMgr()
{
    sLog.Notice("ObjectMgr", "Deleting Corpses...");
    CorpseCollectorUnload();

    sLog.Notice("ObjectMgr", "Deleting Itemsets...");
    for(ItemSetContentMap::iterator i = mItemSets.begin(); i != mItemSets.end(); i++)
        delete i->second;
    mItemSets.clear();

    sLog.Notice("ObjectMgr", "Deleting PlayerCreateInfo...");
    for( PlayerCreateInfoMap::iterator i = mPlayerCreateInfo.begin( ); i != mPlayerCreateInfo.end( ); ++ i )
        delete i->second;
    mPlayerCreateInfo.clear( );

    sLog.Notice("ObjectMgr", "Deleting Vendors...");
    for( VendorMap::iterator i = mVendors.begin( ); i != mVendors.end( ); ++ i )
        i->second.clear();
    mVendors.clear();

    sLog.Notice("ObjectMgr", "Deleting Spell Override...");
    for(OverrideIdMap::iterator i = mOverrideIdMap.begin(); i != mOverrideIdMap.end(); i++)
        delete i->second;

    sLog.Notice("ObjectMgr", "Deleting Groups...");
    for(GroupMap::iterator itr = m_groups.begin(); itr != m_groups.end();)
    {
        Group* pGroup = itr->second;
        ++itr;

        if( pGroup != NULL )
        {
            for( uint32 i = 0; i < pGroup->GetSubGroupCount(); i++ )
                if(SubGroup* pSubGroup = pGroup->GetSubGroup( i ))
                    pSubGroup->Disband();
            delete pGroup;
        }
    }

    sLog.Notice("ObjectMgr", "Deleting Player Information...");
    for(std::map<WoWGuid, PlayerInfo*>::iterator itr = m_playersinfo.begin(); itr != m_playersinfo.end(); itr++)
    {
        itr->second->m_Group = NULL;
        delete itr->second;
    }

    sLog.Notice("ObjectMgr", "Deleting Achievement Cache...");
    for(AchievementCriteriaMap::iterator itr = m_achievementCriteriaMap.begin(); itr != m_achievementCriteriaMap.end(); itr++)
        delete (itr->second);

    while(mTransports.size())
    {
        delete mTransports.begin()->second;
        mTransports.erase(mTransports.begin());
    }
}

//
// Groups
//

Group * ObjectMgr::GetGroupByLeader(Player* pPlayer)
{
    GroupMap::iterator itr;
    Group * ret= NULL;
    m_groupLock.AcquireReadLock();
    for(itr = m_groups.begin(); itr != m_groups.end(); itr++)
    {
        if(itr->second->GetLeader()==pPlayer->getPlayerInfo())
        {
            ret = itr->second;
            break;
        }
    }

    m_groupLock.ReleaseReadLock();
    return ret;
}

Group * ObjectMgr::GetGroupById(uint32 id)
{
    GroupMap::iterator itr;
    Group * ret= NULL;
    m_groupLock.AcquireReadLock();
    itr = m_groups.find(id);
    if(itr!=m_groups.end())
        ret=itr->second;

    m_groupLock.ReleaseReadLock();
    return ret;
}

//
// Player names
//
void ObjectMgr::DeletePlayerInfo( uint32 guid )
{
    PlayerInfo * pl;
    std::map<WoWGuid,PlayerInfo*>::iterator i;
    PlayerNameStringIndexMap::iterator i2;
    playernamelock.AcquireWriteLock();
    i=m_playersinfo.find(guid);
    if(i==m_playersinfo.end())
    {
        playernamelock.ReleaseWriteLock();
        return;
    }

    pl=i->second;
    if(pl->m_Group)
    {
        pl->m_Group->RemovePlayer(pl);
        pl->m_Group = NULL;
    }

    std::string pnam = RONIN_UTIL::TOLOWER_RETURN(pl->charName);
    i2 = m_playersInfoByName.find(pnam);
    if( i2 != m_playersInfoByName.end() && i2->second == pl )
        m_playersInfoByName.erase( i2 );

    delete i->second;
    m_playersinfo.erase(i);

    playernamelock.ReleaseWriteLock();
}

PlayerInfo *ObjectMgr::GetPlayerInfo( WoWGuid guid )
{
    std::map<WoWGuid,PlayerInfo*>::iterator i;
    PlayerInfo * rv;
    playernamelock.AcquireReadLock();
    i=m_playersinfo.find(guid);
    if(i!=m_playersinfo.end())
        rv = i->second;
    else rv = NULL;
    playernamelock.ReleaseReadLock();
    return rv;
}

void ObjectMgr::AddPlayerInfo(PlayerInfo *pn)
{
    playernamelock.AcquireWriteLock();
    m_playersinfo[pn->charGuid] =  pn;
    std::string pnam = RONIN_UTIL::TOLOWER_RETURN(pn->charName);
    m_playersInfoByName[pnam] = pn;
    playernamelock.ReleaseWriteLock();
}

PlayerInfo * ObjectMgr::LoadPlayerInfo(WoWGuid guid)
{
    QueryResult *result = CharacterDatabase.Query("SELECT guid,name,race,class,team,appearance,appearance2,appearance3,level,mapId,instanceId,positionX,positionY,positionZ,orientation,zoneId,lastSaveTime FROM character_data WHERE guid = '%u' LIMIT 1", guid.getLow());
    if(result == NULL)
        return NULL;

    Field *fields = result->Fetch();
    PlayerInfo *pn = new PlayerInfo(fields[0].GetUInt64());
    pn->charName = fields[1].GetString();
    pn->charRace = fields[2].GetUInt8();
    pn->charClass = fields[3].GetUInt8();
    pn->charTeam = fields[4].GetUInt8();
    pn->charAppearance = fields[5].GetUInt32();
    pn->charAppearance2 = fields[6].GetUInt32();
    pn->charAppearance3 = fields[7].GetUInt32();
    pn->lastLevel = fields[8].GetUInt32();
    pn->lastMapID = fields[9].GetUInt32();
    pn->lastInstanceID = fields[10].GetUInt32();
    pn->lastPositionX = fields[11].GetFloat();
    pn->lastPositionY = fields[12].GetFloat();
    pn->lastPositionZ = fields[13].GetFloat();
    pn->lastOrientation = fields[14].GetFloat();
    pn->lastZone = fields[15].GetUInt32();
    pn->lastOnline = fields[16].GetUInt64();
    delete result;

    if(CharRaceEntry * race = dbcCharRace.LookupEntry(pn->charRace))
        pn->charTeam = race->TeamId;

    if( GetPlayerInfoByName(pn->charName.c_str()) != NULL )
    {
        // gotta rename him
        char temp[300];
        snprintf(temp, 300, "%s__%X__", pn->charName.c_str(), pn->charGuid.getLow());
        sLog.Notice("ObjectMgr", "Renaming duplicate player %s to %s. (%u)", pn->charName.c_str(), temp, pn->charGuid.getLow());
        CharacterDatabase.WaitExecute("UPDATE character_data SET name = '%s', forced_rename_pending = 1 WHERE guid = %u", CharacterDatabase.EscapeString(temp).c_str(), pn->charGuid.getLow());
        pn->charName = temp;
    }

    if(GuildMember *member = guildmgr.GetGuildMember(pn->charGuid))
    {
        pn->GuildId = member->guildId;
        pn->GuildRank = member->pRank->iId;
    }

    std::string lpn = RONIN_UTIL::TOLOWER_RETURN(pn->charName);

    playernamelock.AcquireWriteLock();
    // Store playerinfo by name
    m_playersInfoByName[lpn] = pn;
    // Store playerinfo by guid
    m_playersinfo[pn->charGuid] = pn;
    playernamelock.ReleaseWriteLock();
    return pn;
}

void ObjectMgr::UpdatePlayerData(WoWGuid guid, QueryResult *result)
{
    Field *fields = result->Fetch();
    if(m_playersinfo.find(guid) == m_playersinfo.end())
        return;
    playernamelock.AcquireWriteLock();
    if(PlayerInfo *info = m_playersinfo.at(guid))
    {
        std::string chrName = info->charName;
        info->charName = fields[0].GetString();
        info->charRace = fields[1].GetUInt8();
        info->charClass = fields[2].GetUInt8();
        info->charTeam = fields[3].GetUInt8();
        info->charAppearance = fields[4].GetUInt32();
        info->charAppearance2 = fields[5].GetUInt32();
        info->charAppearance3 = fields[6].GetUInt32();
        info->lastLevel = fields[7].GetUInt32();
        info->lastMapID = fields[8].GetUInt32();
        info->lastInstanceID = fields[9].GetUInt32();
        info->lastPositionX = fields[10].GetFloat();
        info->lastPositionY = fields[11].GetFloat();
        info->lastPositionZ = fields[12].GetFloat();
        info->lastOrientation = fields[13].GetFloat();
        info->lastZone = fields[14].GetUInt32();
        info->lastOnline = fields[15].GetUInt64();
        if(strcmp(info->charName.c_str(), chrName.c_str()))
        {
            std::string lpn = RONIN_UTIL::TOLOWER_RETURN(chrName);
            m_playersInfoByName.erase(lpn);
            lpn = RONIN_UTIL::TOLOWER_RETURN(info->charName);
            m_playersInfoByName[lpn] = info;
        }
    }

    playernamelock.ReleaseWriteLock();
}

void ObjectMgr::RenamePlayerInfo(PlayerInfo * pn, const char * oldname, const char * newname)
{
    playernamelock.AcquireWriteLock();
    std::string oldn = RONIN_UTIL::TOLOWER_RETURN(oldname);
    PlayerNameStringIndexMap::iterator itr = m_playersInfoByName.find( oldn );
    if( itr != m_playersInfoByName.end() && itr->second == pn )
    {
        std::string newn = RONIN_UTIL::TOLOWER_RETURN(newname);
        m_playersInfoByName.erase( itr );
        m_playersInfoByName[newn] = pn;
    }

    playernamelock.ReleaseWriteLock();
}

void ObjectMgr::LoadSpellSkills()
{
    for(uint32 i = 0; i < dbcSkillLineSpell.GetNumRows(); i++)
    {
        if (SkillLineAbilityEntry *sp = dbcSkillLineSpell.LookupRow(i))
        {
            mSpellSkills[sp->spell] = sp;
        }
    }
    sLog.Notice("ObjectMgr", "%u spell skills loaded.", mSpellSkills.size());
}

SkillLineAbilityEntry* ObjectMgr::GetSpellSkill(uint32 id)
{
    return mSpellSkills[id];
}

void ObjectMgr::LoadPlayersInfo()
{
    if(QueryResult *result = CharacterDatabase.Query("SELECT guid,name,race,class,team,appearance,appearance2,appearance3,level,mapId,instanceId,positionX,positionY,positionZ,orientation,zoneId,lastSaveTime FROM character_data"))
    {
        uint32 period = (result->GetRowCount() / 20) + 1, c = 0;

        do
        {
            Field *fields = result->Fetch();
            PlayerInfo *pn = new PlayerInfo(fields[0].GetUInt64());
            pn->charName = fields[1].GetString();
            pn->charRace = fields[2].GetUInt8();
            pn->charClass = fields[3].GetUInt8();
            pn->charTeam = fields[4].GetUInt8();
            pn->charAppearance = fields[5].GetUInt32();
            pn->charAppearance2 = fields[6].GetUInt32();
            pn->charAppearance3 = fields[7].GetUInt32();
            pn->lastLevel = fields[8].GetUInt32();
            pn->lastMapID = fields[9].GetUInt32();
            pn->lastInstanceID = fields[10].GetUInt32();
            pn->lastPositionX = fields[11].GetFloat();
            pn->lastPositionY = fields[12].GetFloat();
            pn->lastPositionZ = fields[13].GetFloat();
            pn->lastOrientation = fields[14].GetFloat();
            pn->lastZone = fields[15].GetUInt32();
            pn->lastOnline = fields[16].GetUInt64();

            if( GetPlayerInfoByName(pn->charName.c_str()) != NULL )
            {
                // gotta rename him
                char temp[300];
                snprintf(temp, 300, "%s__%X__", pn->charName.c_str(), pn->charGuid.getLow());
                sLog.Notice("ObjectMgr", "Renaming duplicate player %s to %s. (%u)", pn->charName.c_str(), temp, pn->charGuid.getLow());
                CharacterDatabase.WaitExecute("UPDATE character_data SET name = '%s', forced_rename_pending = 1 WHERE guid = %u", CharacterDatabase.EscapeString(temp).c_str(), pn->charGuid.getLow());
                pn->charName = temp;
            }

            if(GuildMember *member = guildmgr.GetGuildMember(pn->charGuid))
            {
                pn->GuildId = member->guildId;
                pn->GuildRank = member->pRank->iId;
            }

            std::string lpn = RONIN_UTIL::TOLOWER_RETURN(pn->charName);
            // Store playerinfo by name
            m_playersInfoByName[lpn] = pn;

            //this is startup -> no need in lock -> don't use addplayerinfo
            m_playersinfo[pn->charGuid] = pn;

            if( !((++c) % period) )
                sLog.Notice("PlayerInfo", "%u/%u characters loaded", c, result->GetRowCount());
        } while( result->NextRow() );

        delete result;
    }
    sLog.Notice("ObjectMgr", "%u players loaded.", m_playersinfo.size());
}

PlayerInfo* ObjectMgr::GetPlayerInfoByName(const char * name)
{
    std::string lpn = RONIN_UTIL::TOLOWER_RETURN(name);
    PlayerNameStringIndexMap::iterator i;
    PlayerInfo *rv = NULL;
    playernamelock.AcquireReadLock();

    i = m_playersInfoByName.find(lpn);
    if( i != m_playersInfoByName.end() )
        rv = i->second;

    playernamelock.ReleaseReadLock();
    return rv;
}

void ObjectMgr::LoadPlayerCreateInfo()
{
    QueryResult *result = WorldDatabase.Query( "SELECT race, class, map, zone, position_x, position_y, position_z, orientation FROM player_createinfo" );
    if( result == NULL )
    {
        sLog.Error("PlayerCreateInfo", "Incorrect structure in player_createinfo");
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        CharRaceEntry *raceEntry = dbcCharRace.LookupEntry(fields[0].GetUInt8());
        CharClassEntry *classEntry = dbcCharClass.LookupEntry(fields[1].GetUInt8());
        if(raceEntry == NULL || classEntry == NULL)
            continue;
        uint16 index = uint16(classEntry->classId)|(uint16(raceEntry->RaceId)<<8);
        if(mPlayerCreateInfo.find(index) != mPlayerCreateInfo.end())
            continue;

        PlayerCreateInfo *pPlayerCreateInfo = new PlayerCreateInfo();
        pPlayerCreateInfo->race = raceEntry->RaceId;
        pPlayerCreateInfo->class_ = classEntry->classId;
        pPlayerCreateInfo->factiontemplate = raceEntry->FactionId;
        pPlayerCreateInfo->displayId[0] = raceEntry->maleModel;
        pPlayerCreateInfo->displayId[1] = raceEntry->femaleModel;
        pPlayerCreateInfo->requiredExpansion = raceEntry->requiredExpansion&0x7FFF;
        pPlayerCreateInfo->requiredExpansion |= (classEntry->requiredExpansion&0x7FFF)<<16;
        pPlayerCreateInfo->mapId = fields[2].GetUInt32();
        pPlayerCreateInfo->zoneId = fields[3].GetUInt32();
        pPlayerCreateInfo->positionX = fields[4].GetFloat();
        pPlayerCreateInfo->positionY = fields[5].GetFloat();
        pPlayerCreateInfo->positionZ = fields[6].GetFloat();
        pPlayerCreateInfo->Orientation = fields[7].GetFloat();
        mPlayerCreateInfo.insert(std::make_pair(index, pPlayerCreateInfo));
    } while( result->NextRow() );
    delete result;

    if(result = WorldDatabase.Query("SELECT * FROM player_createinfo_spells"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint8 race = fields[0].GetUInt8(), _class = fields[1].GetUInt8();
            if(PlayerCreateInfo *pPlayerCreateInfo = GetPlayerCreateInfo(race, _class))
                pPlayerCreateInfo->spell_list.insert(fields[2].GetUInt32());
        } while( result->NextRow() );
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT * FROM player_createinfo_items"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint8 race = fields[0].GetUInt8(), _class = fields[1].GetUInt8();
            PlayerCreateInfo *pPlayerCreateInfo = GetPlayerCreateInfo(race, _class);
            if(pPlayerCreateInfo == NULL)
                continue;

            CreateInfo_ItemStruct itm;
            itm.protoid = fields[2].GetUInt32();
            itm.amount = fields[3].GetUInt32();
            pPlayerCreateInfo->items.push_back(itm);
        } while( result->NextRow() );
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT * FROM player_createinfo_bars"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint8 race = fields[0].GetUInt8(), _class = fields[1].GetUInt8();
            PlayerCreateInfo *pPlayerCreateInfo = GetPlayerCreateInfo(race, _class);
            if(pPlayerCreateInfo == NULL)
                continue;

            CreateInfo_ActionBarStruct bar;
            bar.button = fields[2].GetUInt8();
            bar.action = fields[3].GetUInt32();
            bar.type = fields[4].GetUInt8();
            pPlayerCreateInfo->bars.push_back(bar);
        } while( result->NextRow() );
        delete result;
    }

    sLog.Notice("ObjectMgr", "%u player create infos loaded.", mPlayerCreateInfo.size());
}

Corpse* ObjectMgr::LoadCorpse(uint32 guid)
{
    QueryResult *result = CharacterDatabase.Query("SELECT * FROM Corpses WHERE guid =%u ", guid );
    if( result == NULL )
        return NULL;

    Corpse* pCorpse;
    do
    {
        Field *fields = result->Fetch();
        pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE, fields[0].GetUInt32());
        pCorpse->Create( fields[1].GetUInt32(), fields[9].GetUInt32(), fields[4].GetFloat(), fields[5].GetFloat(), fields[6].GetFloat(), fields[7].GetFloat());
        pCorpse->Init();
        pCorpse->SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, fields[2].GetUInt32()); 
        pCorpse->SetUInt32Value(CORPSE_FIELD_FLAGS, fields[3].GetUInt32());
        pCorpse->SetZoneId(fields[8].GetUInt32());
        pCorpse->SetInstanceID(fields[10].GetUInt32());
        if(pCorpse->GetUInt32Value(CORPSE_FIELD_DISPLAY_ID) == 0)
        {
            RemoveCorpse(pCorpse);
            pCorpse->Destruct();
            pCorpse = NULL;
            continue;
        }

        pCorpse->SetLoadedFromDB(true);
        pCorpse->SetInstanceID(fields[8].GetUInt32());
        sWorldMgr.PushToWorldQueue(pCorpse);
    } while( result->NextRow() );

    delete result;
    return pCorpse;
}


//------------------------------------------------------
// Live corpse retreival.
// comments: I use the same tricky method to start from the last corpse instead of the first
//------------------------------------------------------
Corpse* ObjectMgr::GetCorpseByOwner(uint32 ownerguid)
{
    Corpse* rv = NULL;
    _corpseslock.Acquire();
    for (CorpseMap::const_iterator itr = m_corpses.begin();itr != m_corpses.end();)
    {
        if(itr->second->GetUInt32Value(CORPSE_FIELD_OWNER) == ownerguid)
        {
            rv = itr->second;
            break;
        }
        ++itr;
    }
    _corpseslock.Release();

    return rv;
}

void ObjectMgr::DelinkPlayerCorpses(Player* pOwner)
{
    //dupe protection agaisnt crashs
    Corpse* c = GetCorpseByOwner(pOwner->GetLowGUID());
    if(c == NULL)
        return;
    CorpseAddEventDespawn(c);
}

void ObjectMgr::SetHighestGuids()
{
    QueryResult *result;
    if( result = CharacterDatabase.Query( "SELECT MAX(guid) FROM character_data" ) )
    {
        m_hiPlayerGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    if( result = CharacterDatabase.Query("SELECT MAX(itemguid) FROM item_data") )
    {
        m_hiItemGuid = (uint32)result->Fetch()[0].GetUInt32();
        delete result;
    }

    if( result = CharacterDatabase.Query( "SELECT MAX(guid) FROM corpses" ) )
    {
        m_hiCorpseGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT MAX(id) FROM creature_spawns"))
    {
        m_hiCreatureSpawnId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT MAX(id) FROM gameobject_spawns"))
    {
        m_hiGameObjectSpawnId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    if(result = CharacterDatabase.Query("SELECT MAX(group_id) FROM groups"))
    {
        m_hiGroupId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    if(result = CharacterDatabase.Query("SELECT MAX(setguid) FROM equipmentsets"))
    {
        m_equipmentSetGuid = result->Fetch()[0].GetUInt64();
        delete result;
    }

    sTracker.GetGUIDCount();

    sLog.Notice("ObjectMgr", "HighGuid(CORPSE) = %u", m_hiCorpseGuid);
    sLog.Notice("ObjectMgr", "HighGuid(PLAYER) = %u", m_hiPlayerGuid);
    sLog.Notice("ObjectMgr", "HighGuid(GAMEOBJ) = %u", m_hiGameObjectSpawnId);
    sLog.Notice("ObjectMgr", "HighGuid(UNIT) = %u", m_hiCreatureSpawnId);
    sLog.Notice("ObjectMgr", "HighGuid(ITEM) = %u", m_hiItemGuid);
    sLog.Notice("ObjectMgr", "HighGuid(GROUP) = %u", m_hiGroupId);
    sLog.Notice("ObjectMgr", "HighGuid(EQSETS) = %u", m_equipmentSetGuid);
}

void ObjectMgr::ListGuidAmounts()
{
    QueryResult *result;
    uint32 amount[7] = { 0, 0, 0, 0, 0, 0, 0 };
    std::string name[7] = {"Characters", "Player Items", "Corpses", "Groups", "GM Tickets", "Creatures", "Gameobjects"};

    if(result = CharacterDatabase.Query("SELECT guid FROM character_data"))
    {
        amount[0] = result->GetRowCount();
        delete result;
    }

    if(result = CharacterDatabase.Query("SELECT itemguid FROM item_data"))
    {
        amount[1] = result->GetRowCount();
        delete result;
    }

    if(result = CharacterDatabase.Query( "SELECT guid FROM corpses" ))
    {
        amount[2] = result->GetRowCount();
        delete result;
    }

    if(result = CharacterDatabase.Query("SELECT group_id FROM groups"))
    {
        amount[3] = result->GetRowCount();
        delete result;
    }

    if(result = CharacterDatabase.Query("SELECT guid FROM gm_tickets"))
    {
        amount[4] = result->GetRowCount();
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT id FROM creature_spawns"))
    {
        amount[5] = result->GetRowCount();
        delete result;
    }

    if(result = WorldDatabase.Query("SELECT id FROM gameobject_spawns"))
    {
        amount[6] = result->GetRowCount();
        delete result;
    }

    for(int i = 0; i < 7; i++)
        sLog.Notice("ObjectMgr", "Load Amount(%s) = %u", name[i].c_str(), amount[i] ? amount[i] : 0);
}

uint32 ObjectMgr::GenerateMailID()
{
    return m_mailid++;
}

uint64 ObjectMgr::GenerateEquipmentSetGuid()
{
    return m_equipmentSetGuid++;
}

uint32 ObjectMgr::GenerateItemGuid()
{
    m_guidGenMutex.Acquire();
    uint32 ret = ++m_hiItemGuid;
    m_guidGenMutex.Release();
    return ret;
}

uint32 ObjectMgr::GeneratePlayerGuid()
{
    m_playerguidlock.Acquire();
    uint32 ret = ++m_hiPlayerGuid;
    m_playerguidlock.Release();
    return ret;
}

Player* ObjectMgr::GetPlayer(const char* name, bool caseSensitive)
{
    Player * rv = NULL;
    PlayerStorageMap::const_iterator itr;
    _playerslock.AcquireReadLock();

    if(!caseSensitive)
    {
        std::string strName = name;
        RONIN_UTIL::TOLOWER(strName);
        for (itr = _players.begin(); itr != _players.end();)
        {
            if(!stricmp(itr->second->GetNameString()->c_str(), strName.c_str()))
            {
                rv = itr->second;
                break;
            }
            ++itr;
        }
    }
    else
    {
        for (itr = _players.begin(); itr != _players.end();)
        {
            if(!strcmp(itr->second->GetName(), name))
            {
                rv = itr->second;
                break;
            }
            ++itr;
        }
    }

    _playerslock.ReleaseReadLock();

    return rv;
}

Player* ObjectMgr::GetPlayer(WoWGuid guid)
{
    Player * rv = NULL;

    _playerslock.AcquireReadLock();
    PlayerStorageMap::const_iterator itr = _players.find(guid);
    rv = (itr != _players.end()) ? itr->second : NULL;
    _playerslock.ReleaseReadLock();

    return rv;
}

PlayerCreateInfo* ObjectMgr::GetPlayerCreateInfo(uint8 race, uint8 class_) const
{
    uint16 index = uint16(class_)|(uint16(race)<<8);
    if(mPlayerCreateInfo.find(index) != mPlayerCreateInfo.end())
        return mPlayerCreateInfo.at(index);
    return NULL;
}

void ObjectMgr::LoadVendors()
{
    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM creature_vendor "))
    {
        if( result->GetFieldCount() < 7 )
        {
            sLog.Notice("ObjectMgr", "Invalid format in creature_vendor (%u/7) columns, not enough data to proceed.\n", result->GetFieldCount() );
            delete result;
            return;
        }
        else if( result->GetFieldCount() > 7)
            sLog.Notice("ObjectMgr", "Invalid format in creature_vendor (%u/7) columns, loading anyway because we have enough data\n", result->GetFieldCount() );

        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            ItemPrototype *proto = sItemMgr.LookupEntry(fields[1].GetUInt32());
            if(proto == NULL || proto->SellPrice > proto->BuyPrice)
                continue;

            uint32 extendedCost = fields[4].GetUInt32();
            ItemExtendedCostEntry *extendCostEntry = NULL;
            if(extendedCost > 0 && (extendCostEntry = dbcItemExtendedCost.LookupEntry(extendedCost)) == NULL)
            {
                sLog.Warning("ObjectMgr","Item %u at vendor %u has extended cost %u which is invalid. Skipping.", proto->ItemId, entry, extendedCost);
                continue;
            }

            std::vector<VendorItem> &vendorItems = mVendors[entry];
            VendorItem itm;
            itm.itemid              = fields[1].GetUInt32();
            itm.max_amount          = fields[2].GetUInt32();
            itm.incrtime            = fields[3].GetUInt32();
            itm.vendormask          = fields[5].GetUInt32();
            itm.IsDependent         = fields[6].GetBool();
            itm.extendedCost        = extendedCost;
            vendorItems.push_back(itm);
        }while( result->NextRow() );
        delete result;
    }
    sLog.Notice("ObjectMgr", "%u vendors loaded.", mVendors.size());
}

void ObjectMgr::ReloadVendors()
{
    // Clean out vectors then clean out our map
    for( VendorMap::iterator i = mVendors.begin( ); i != mVendors.end( ); ++ i )
        i->second.clear();
    mVendors.clear();

    LoadVendors();
}

void ObjectMgr::ProcessTitles()
{
    for(uint32 i = 0; i < dbcCharTitle.GetNumRows(); i++)
    {
        if(CharTitleEntry *entry = dbcCharTitle.LookupRow(i))
        {
            if(RONIN_UTIL::FindXinYString("%s", entry->titleFormat))
            {
                std::string title = entry->titleFormat;
                size_t pos = title.find("%s"), cutoff = 2;
                if(pos < title.size()/2)
                {
                    if(title.at(pos+cutoff) == ' ')
                        cutoff++;
                    else if(title.at(pos+cutoff+1) == ' ')
                        cutoff += 2;
                    entry->titleName = title.substr(pos+cutoff, title.length());
                } else entry->titleName = title.substr(0, pos-1);
            } else entry->titleName = entry->titleFormat;
            printf("");
        }
    }
    sLog.Notice("ObjectMgr", "Processed %u character titles", dbcCharTitle.GetNumRows());
}

void ObjectMgr::ProcessCreatureFamilies()
{
    for(uint32 i = 0; i < dbcCreatureFamily.GetNumRows(); i++)
    {
        if(CreatureFamilyEntry *entry = dbcCreatureFamily.LookupRow(i))
        {
            std::string name = entry->name;
            m_creatureFamilyNameToID.insert(std::make_pair(name, entry->ID));
        }
    }
    sLog.Notice("ObjectMgr", "Processed %u creature families", dbcCreatureFamily.GetNumRows());
}

bool ObjectMgr::GetCreatureFamilyIDFromName(const char *name, uint32 &out)
{
    if(m_creatureFamilyNameToID.find(name) != m_creatureFamilyNameToID.end())
    {
        out = m_creatureFamilyNameToID.at(name);
        return true;
    }
    return false;
}

void ObjectMgr::LoadTrainers()
{
    mTrainerData.clear();

    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM creature_trainerdata "))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();

            TrainerData &data = mTrainerData[entry];
            data.category = fields[1].GetUInt8();
            data.subCategory = fields[2].GetUInt8();
            data.reqSkill = fields[3].GetUInt32();
            data.reqSkillValue = fields[4].GetUInt32();
            data.trainerTitle = fields[5].GetString();
        }while( result->NextRow() );
        delete result;
    }
    sLog.Notice("ObjectMgr", "%u trainers loaded.", mTrainerData.size());

    for(CreatureDataManager::iterator itr = sCreatureDataMgr.begin(); itr != sCreatureDataMgr.end(); ++itr)
    {
        CreatureData *data = (*itr)->second;
        if((data->NPCFLags & (UNIT_NPC_FLAG_TRAINER|UNIT_NPC_FLAG_TRAINER_PROF)) == 0)
            continue;

        for(uint8 i = WARRIOR; i < CLASS_MAX; i++)
        {
            if(i == 10)
                continue;
            std::string className = classNames[i-1];
            className.append(" Trainer");
            if(strcmp(className.c_str(), data->subName.c_str()))
                continue;

            TrainerData &tdata = mTrainerData[data->entry];
            tdata.category = 0x01;
            tdata.subCategory = i;
            tdata.reqSkill = tdata.reqSkillValue = 0;
            tdata.trainerTitle = className.c_str();
            break;
        }
    }

    uint32 count = 0;
    if(QueryResult *result = WorldDatabase.Query("SELECT * FROM trainer_spells "))
    {
        do
        {
            Field* fields = result->Fetch();
            uint8 category = fields[0].GetUInt8();
            uint8 subCategory = fields[1].GetUInt8();
            std::pair<uint8, uint8> catPair = std::make_pair(category, subCategory);

            SpellEntry *sp;
            if((sp = dbcSpell.LookupEntry(fields[2].GetUInt32())) == NULL)
                continue;
            if(sp->NameHash == SPELL_HASH_MASTERY)
                continue;
            ObjectMgr::TrainerSpellMap *map = &mTrainerSpellStorage[catPair];
            if(map->find(sp->Id) != map->end())
                continue;

            TrainerSpell tSpell;
            tSpell.entry = sp;
            tSpell.spellCost = fields[3].GetUInt32();
            tSpell.requiredLevel = fields[4].GetUInt32();
            tSpell.reqSkill = fields[5].GetUInt32();
            tSpell.reqSkillValue = fields[6].GetUInt32();
            map->insert(std::make_pair(sp->Id, tSpell));
            count++;
        }while( result->NextRow() );
        delete result;
    }
    sLog.Notice("ObjectMgr", "%u trainer spells loaded.", count);

    // Ensure we have hardcoded mastery spells stored for level 80
    for(uint8 i = WARRIOR; i < CLASS_MAX; i++)
    {
        uint32 spellId;
        if((spellId = classMasterySpells[i-1]) == 0)
            continue;
        SpellEntry *sp;
        if((sp = dbcSpell.LookupEntry(spellId)) == NULL)
            continue;

        std::pair<uint8, uint8> catPair = std::make_pair(0x01, i);
        ObjectMgr::TrainerSpellMap *map = &mTrainerSpellStorage[catPair];
        if(map->find(spellId) != map->end())
            continue;

        TrainerSpell tSpell;
        tSpell.entry = sp;
        tSpell.spellCost = 222000;
        tSpell.requiredLevel = sp->spellLevelBaseLevel;
        tSpell.reqSkill = tSpell.reqSkillValue = 0;
        map->insert(std::make_pair(sp->Id, tSpell));
    }
}

void ObjectMgr::FillVendorList(uint32 entry, uint32 vendorMask, std::vector<AvailableCreatureItem> &toFill)
{
    if(mVendors.find(entry) == mVendors.end())
        return;

    toFill.clear();
    std::vector<VendorItem> itemList = mVendors.at(entry);
    toFill.resize(itemList.size());

    for(uint16 i = 0; i < itemList.size(); i++)
    {
        AvailableCreatureItem &vendorItem = toFill[i];
        vendorItem.proto = sItemMgr.LookupEntry(itemList[i].itemid);
        vendorItem.availableAmount = vendorItem.max_amount = itemList[i].max_amount;
        vendorItem.incrtime = itemList[i].incrtime;
        vendorItem.extended_cost = dbcItemExtendedCost.LookupEntry(itemList[i].extendedCost);
        vendorItem.IsDependent = itemList[i].IsDependent;
        vendorItem.refreshTime = 0;
    }
}

Item* ObjectMgr::CreateItem(uint32 entry,Player* owner, uint32 count)
{
    ItemPrototype * proto = sItemMgr.LookupEntry(entry);
    if(proto == NULL)
        return NULL;

    Item *ret = NULL;
    uint32 counter = GenerateItemGuid();
    if(proto->InventoryType == INVTYPE_BAG)
        ret = new Container(proto, counter);
    else ret = new Item(proto, counter);
    ret->Init();
    ret->SetCount(count);
    ret->SetOwner(owner);
    return ret;
}

TrainerData *ObjectMgr::GetTrainerData(uint32 entry)
{
    if(mTrainerData.find(entry) != mTrainerData.end())
        return &mTrainerData.at(entry);
    return NULL;
}

ObjectMgr::TrainerSpellMap *ObjectMgr::GetTrainerSpells(uint8 category, uint8 subCategory)
{
    std::pair<uint8, uint8> catPair = std::make_pair(category, subCategory);
    if(mTrainerSpellStorage.find(catPair) != mTrainerSpellStorage.end())
        return &mTrainerSpellStorage.at(catPair);
    return NULL;
}

Item* ObjectMgr::LoadItem(uint64 guid)
{
    Item* pReturn = NULL;
    if(QueryResult * result = CharacterDatabase.Query("SELECT * FROM item_instance WHERE guid = %u", GUID_LOPART(guid)))
    {
        if(ItemPrototype * pProto = sItemMgr.LookupEntry(result->Fetch()[2].GetUInt32()))
        {
            if(pProto->InventoryType == INVTYPE_BAG)
                pReturn = new Container(pProto, GUID_LOPART(guid));
            else pReturn = new Item(pProto, GUID_LOPART(guid));
            pReturn->LoadFromDB(result->Fetch());
        }
        delete result;
    }

    return pReturn;
}

void ObjectMgr::LoadCorpses(MapInstance* mgr)
{
    QueryResult *result = CharacterDatabase.Query("SELECT * FROM corpses WHERE mapId = %u", mgr->GetMapId());
    if(result)
    {
        Corpse* pCorpse = NULL;
        do
        {
            Field *fields = result->Fetch();
            uint32 instanceid = fields[10].GetUInt32();
            if(instanceid && instanceid != mgr->GetInstanceID())
                continue;
            pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE, fields[0].GetUInt32());
            pCorpse->Init();
            pCorpse->SetInstanceID(instanceid);
            pCorpse->Create( fields[1].GetUInt64(), fields[9].GetUInt32(), fields[4].GetFloat(), fields[5].GetFloat(), fields[6].GetFloat(), fields[7].GetFloat());
            pCorpse->SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, fields[2].GetUInt32()); 
            pCorpse->SetUInt32Value(CORPSE_FIELD_FLAGS, fields[3].GetUInt32());
            pCorpse->SetZoneId(fields[8].GetUInt32());
            if(pCorpse->GetUInt32Value(CORPSE_FIELD_DISPLAY_ID) == 0)
            {
                RemoveCorpse(pCorpse);
                pCorpse->Destruct();
                pCorpse = NULL;
                continue;
            }

            pCorpse->PushToWorld(mgr);
        } while( result->NextRow() );

        delete result;
    }
}

std::list<ItemPrototype*>* ObjectMgr::GetListForItemSet(uint32 setid)
{
    return mItemSets[setid];
}

void ObjectMgr::CorpseAddEventDespawn(Corpse* pCorpse)
{
    if(!pCorpse->IsInWorld())
    {
        RemoveCorpse(pCorpse);
        pCorpse->Destruct();
        return;
    }
}

void ObjectMgr::DespawnCorpse(uint64 Guid)
{
    Corpse * pCorpse = objmgr.GetCorpse((uint32)Guid);
    if(pCorpse == NULL) // Already Deleted
    {
        RemoveCorpse((uint32)Guid); // Just to make sure.
        return;
    }

    pCorpse->Despawn();
    RemoveCorpse(pCorpse);
}

void ObjectMgr::CorpseCollectorUnload(bool saveOnly)
{
    CorpseMap::const_iterator itr;
    _corpseslock.Acquire();
    // Save our corpses first!
    for (itr = m_corpses.begin(); itr != m_corpses.end(); itr++)
        itr->second->SaveToDB();
    if(saveOnly)
    {
        _corpseslock.Release();
        return;
    }

    for (itr = m_corpses.begin(); itr != m_corpses.end();)
    {
        Corpse* c = itr->second;
        ++itr;
        if(c != NULL)
        {
            if(c->IsInWorld())
                c->RemoveFromWorld();
            c->Destruct();
            c = NULL;
        }
    }
    m_corpses.clear();
    _corpseslock.Release();
}

PlayerInfo* ObjectMgr::CreatePlayer()
{
    m_playerguidlock.Acquire();
    uint32 guid = ++m_hiPlayerGuid;
    m_playerguidlock.Release();

    return new PlayerInfo(guid);
}

void ObjectMgr::AddPlayer(Player* p)//add it to global storage
{
    _playerslock.AcquireWriteLock();
    _players[p->GetLowGUID()] = p;
    _playerslock.ReleaseWriteLock();
}

void ObjectMgr::RemovePlayer(Player* p)
{
    _playerslock.AcquireWriteLock();
    _players.erase(p->GetLowGUID());
    _playerslock.ReleaseWriteLock();
}

Corpse* ObjectMgr::CreateCorpse()
{
    m_corpseguidlock.Acquire();
    uint32 guid = ++m_hiCorpseGuid;
    m_corpseguidlock.Release();

    Corpse* pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE, guid);
    pCorpse->Init();
    return pCorpse;
}

void ObjectMgr::AddCorpse(Corpse* p)//add it to global storage
{
    _corpseslock.Acquire();
    m_corpses[p->GetLowGUID()]=p;
    _corpseslock.Release();
}

void ObjectMgr::RemoveCorpse(Corpse* p)
{
    _corpseslock.Acquire();
    m_corpses.erase(p->GetLowGUID());
    _corpseslock.Release();
}

void ObjectMgr::RemoveCorpse(uint32 corpseguid)
{
    _corpseslock.Acquire();
    m_corpses.erase(corpseguid);
    _corpseslock.Release();
}

Corpse* ObjectMgr::GetCorpse(uint32 corpseguid)
{
    Corpse* rv;
    _corpseslock.Acquire();
    CorpseMap::const_iterator itr = m_corpses.find(corpseguid);
    rv = (itr != m_corpses.end()) ? itr->second : NULL;
    _corpseslock.Release();
    return rv;
}

Transporter* ObjectMgr::GetTransporter(uint32 guid)
{
    Transporter* rv;
    _TransportLock.Acquire();
    std::map<uint32, Transporter* >::const_iterator itr = mTransports.find(guid);
    rv = (itr != mTransports.end()) ? itr->second : NULL;
    _TransportLock.Release();
    return rv;
}

void ObjectMgr::AddTransport(Transporter* pTransporter)
{
    _TransportLock.Acquire();
    mTransports[pTransporter->GetLowGUID()]=pTransporter;
    _TransportLock.Release();
}

Transporter* ObjectMgr::GetTransporterByEntry(uint32 entry)
{
    Transporter* rv = NULL;
    _TransportLock.Acquire();
    std::map<uint32, Transporter* >::iterator itr = mTransports.begin();
    for(; itr != mTransports.end(); itr++)
    {
        if(itr->second->GetEntry() == entry)
        {
            rv = itr->second;
            break;
        }
    }
    _TransportLock.Release();
    return rv;
}

void ObjectMgr::HashWMOAreaTables()
{
    for(uint32 i = 0; i < dbcWMOAreaTable.GetNumRows(); i++)
    {
        WMOAreaTableEntry *entry = dbcWMOAreaTable.LookupRow(i);
        std::pair<uint32, std::pair<uint32, uint32> > WMOID(entry->groupId, std::make_pair(entry->rootId, entry->adtId));
        WMOAreaTables.insert(std::make_pair(WMOID, entry));
    }
}

WMOAreaTableEntry* ObjectMgr::GetWMOAreaTable(int32 adtid, int32 rootid, int32 groupid)
{
    std::pair<uint32, std::pair<uint32, uint32> > WMOIDs(groupid, std::make_pair(rootid, adtid));
    if(WMOAreaTables.find(WMOIDs) != WMOAreaTables.end())
        return WMOAreaTables.at(WMOIDs);
    return NULL;
}

void ObjectMgr::LoadGroups()
{
    QueryResult * result = CharacterDatabase.Query("SELECT * FROM groups");
    if(result)
    {
        do
        {
            Group * g = new Group(false);
            g->LoadFromDB(result->Fetch());
        } while(result->NextRow());
        delete result;
    }

    sLog.Notice("ObjectMgr", "%u groups loaded.", m_groups.size());
}

void ObjectMgr::ResetDailies()
{
    _playerslock.AcquireReadLock();
    PlayerStorageMap::iterator itr = _players.begin();
    for(; itr != _players.end(); itr++)
    {
        Player* pPlayer = itr->second;
        uint8 eflags = 0;
    }
    _playerslock.ReleaseReadLock();
}

QueryResult* ObjectMgr::SQLCheckExists(const char* tablename, const char* columnname, uint64 columnvalue)
{
    if(!tablename || !columnname || !columnvalue)
        return NULL;

    QueryResult* result = WorldDatabase.Query("SELECT %s FROM %s WHERE %s = '%u' LIMIT 1", columnname, tablename, columnname, columnvalue);
    return result;
}
