/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( ObjectMgr );

ObjectMgr::ObjectMgr()
{
    m_hiPetGuid = 0;
    m_hiContainerGuid = 0;
    m_hiItemGuid = 0;
    m_hiGroupId = 1;
    m_mailid = 0;
    m_hiPlayerGuid = 0;
    m_hiCorpseGuid = 0;
    m_hiArenaTeamId = 0;
    m_ticketid = 1;
    m_equipmentSetGuid = 0;
    mQuestPOIMap.clear();
}

ObjectMgr::~ObjectMgr()
{
    sLog.Notice("ObjectMgr", "Deleting Corpses...");
    CorpseCollectorUnload();

    sLog.Notice("ObjectMgr", "Deleting Itemsets...");
    for(ItemSetContentMap::iterator i = mItemSets.begin(); i != mItemSets.end(); i++)
    {
        delete i->second;
    }
    mItemSets.clear();

    sLog.Notice("ObjectMgr", "Deleting PlayerCreateInfo...");
    for( PlayerCreateInfoMap::iterator i = mPlayerCreateInfo.begin( ); i != mPlayerCreateInfo.end( ); ++ i ) {
        delete i->second;
    }
    mPlayerCreateInfo.clear( );

    sLog.Notice("ObjectMgr", "Deleting Vendors...");
    for( VendorMap::iterator i = mVendors.begin( ); i != mVendors.end( ); ++ i )
    {
        delete i->second;
    }

    sLog.Notice("ObjectMgr", "Deleting Spell Override...");
    for(OverrideIdMap::iterator i = mOverrideIdMap.begin(); i != mOverrideIdMap.end(); i++)
    {
        delete i->second;
    }

    sLog.Notice("ObjectMgr", "Deleting Trainers...");
    for( TrainerMap::iterator i = mTrainers.begin( ); i != mTrainers.end( ); ++ i) {
        Trainer * t = i->second;
        if(t->UIMessage)
            delete [] t->UIMessage;
        delete t;
    }

    sLog.Notice("ObjectMgr", "Deleting Level Information...");
    for( LevelInfoMap::iterator i = mLevelInfo.begin(); i != mLevelInfo.end(); i++)
    {
        LevelMap * l = i->second;
        for(LevelMap::iterator i2 = l->begin(); i2 != l->end(); i2++)
        {
            delete i2->second;
        }
        l->clear();
        delete l;
    }

    sLog.Notice("ObjectMgr", "Deleting Waypoint Cache...");
    for(HM_NAMESPACE::hash_map<uint32, WayPointMap*>::iterator i = m_waypoints.begin(); i != m_waypoints.end(); i++)
    {
        for(WayPointMap::iterator i3 = i->second->begin(); i3 != i->second->end(); i3++)
            if((*i3))
                delete (*i3);

        delete i->second;
    }

    sLog.Notice("ObjectMgr", "Deleting NPC Say Texts...");
    for(uint32 i = 0 ; i < NUM_MONSTER_SAY_EVENTS ; i++)
    {
        NpcMonsterSay * p;
        for(MonsterSayMap::iterator itr = mMonsterSays[i].begin(); itr != mMonsterSays[i].end(); itr++)
        {
            p = itr->second;
            for(uint32 j = 0; j < p->TextCount; ++j)
                free((char*)p->Texts[j]);
            delete [] p->Texts;
            free((char*)p->MonsterName);
            delete p;
        }

        mMonsterSays[i].clear();
    }

    sLog.Notice("ObjectMgr", "Deleting Reputation Tables...");
    for(ReputationModMap::iterator itr = m_reputation_creature.begin(); itr != m_reputation_creature.end(); itr++)
    {
        ReputationModifier * mod = itr->second;
        mod->mods.clear();
        delete mod;
    }
    for(ReputationModMap::iterator itr = m_reputation_faction.begin(); itr != m_reputation_faction.end(); itr++)
    {
        ReputationModifier * mod = itr->second;
        mod->mods.clear();
        delete mod;
    }

    for(HM_NAMESPACE::hash_map<uint32,InstanceReputationModifier*>::iterator itr = m_reputation_instance.begin(); itr != m_reputation_instance.end(); itr++)
    {
        InstanceReputationModifier * mod = itr->second;
        mod->mods.clear();
        delete mod;
    }

    sLog.Notice("ObjectMgr", "Deleting Groups...");
    for(GroupMap::iterator itr = m_groups.begin(); itr != m_groups.end();)
    {
        Group* pGroup = itr->second;
        ++itr;

        if( pGroup != NULL )
        {
            for( uint32 i = 0; i < pGroup->GetSubGroupCount(); i++ )
            {
                SubGroup* pSubGroup = pGroup->GetSubGroup( i );
                if( pSubGroup != NULL )
                {
                    pSubGroup->Disband();
                }
            }
            delete pGroup;
        }
    }

    sLog.Notice("ObjectMgr", "Deleting Player Information...");
    for(HM_NAMESPACE::hash_map<uint32, PlayerInfo*>::iterator itr = m_playersinfo.begin(); itr != m_playersinfo.end(); itr++)
    {
        itr->second->m_Group = NULL;
        free(itr->second->name);
        delete itr->second;
    }

    sLog.Notice("ObjectMgr", "Deleting GM Tickets...");
    for(GmTicketList::iterator itr = GM_TicketList.begin(); itr != GM_TicketList.end(); itr++)
        delete (*itr);

    sLog.Notice("ObjectMgr", "Deleting Arena Teams...");
    for(HM_NAMESPACE::hash_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeams.begin(); itr != m_arenaTeams.end(); itr++)
        delete itr->second;

    sLog.Notice("ObjectMgr", "Deleting Profession Discoveries...");
    std::set<ProfessionDiscovery*>::iterator itr = ProfessionDiscoveryTable.begin();
    for ( ; itr != ProfessionDiscoveryTable.end(); itr++ )
        delete (*itr);

    sLog.Notice("ObjectMgr", "Deleting Achievement Cache...");
    for(AchievementCriteriaMap::iterator itr = m_achievementCriteriaMap.begin(); itr != m_achievementCriteriaMap.end(); itr++)
        delete (itr->second);

    sLog.Notice("ObjectMgr", "Deleting Pet Levelup Spells...");
    for(PetLevelupSpellMap::iterator itr = mPetLevelupSpellMap.begin(); itr != mPetLevelupSpellMap.end(); itr++)
    {
        itr->second.clear();
    }
    mPetLevelupSpellMap.clear();
    mQuestPOIMap.clear();
}

void ObjectMgr::LoadAchievements()
{
    for(uint32 i = 0; i < dbcAchievement.GetNumRows(); i++)
    {
        AchievementEntry * ae = dbcAchievement.LookupRow(i);
        if(ae)
            ae->AssociatedCriteriaCount = 0;
    }

    for(uint32 i = 0; i < dbcAchievementCriteria.GetNumRows(); i++)
    {
        AchievementCriteriaEntry * ace = dbcAchievementCriteria.LookupRow( i );
        if( ace )
        {
            AchievementCriteriaMap::iterator itr = m_achievementCriteriaMap.find( ace->requiredType );
            if( itr == m_achievementCriteriaMap.end() )
            {
                // We need to make a new set, and insert this.
                AchievementCriteriaSet * acs = new AchievementCriteriaSet;
                acs->insert( ace );
                m_achievementCriteriaMap.insert( make_pair( ace->requiredType, acs ) );
            }
            else
            {
                // Push into old set
                AchievementCriteriaSet * acs = itr->second;
                acs->insert( ace );
            }

            AchievementEntry * ae = dbcAchievement.LookupEntry( ace->referredAchievement );
            if( ae )
            {
                if(ae->AssociatedCriteriaCount >= 32)
                    continue;
                ae->AssociatedCriteria[ ae->AssociatedCriteriaCount ] = ace->ID;
                ae->AssociatedCriteriaCount++;
            }
        }
    }
    sLog.Notice("AchievementMgr", "Loaded %u achievements", dbcAchievementCriteria.GetNumRows());
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
        if(itr->second->GetLeader()==pPlayer->m_playerInfo)
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
    HM_NAMESPACE::hash_map<uint32,PlayerInfo*>::iterator i;
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

    string pnam = string(pl->name);
    HEARTHSTONE_TOLOWER(pnam);
    i2 = m_playersInfoByName.find(pnam);
    if( i2 != m_playersInfoByName.end() && i2->second == pl )
        m_playersInfoByName.erase( i2 );

    free(pl->name);
    delete i->second;
    m_playersinfo.erase(i);

    playernamelock.ReleaseWriteLock();
}

PlayerInfo *ObjectMgr::GetPlayerInfo( uint32 guid )
{
    HM_NAMESPACE::hash_map<uint32,PlayerInfo*>::iterator i;
    PlayerInfo * rv;
    playernamelock.AcquireReadLock();
    i=m_playersinfo.find(guid);
    if(i!=m_playersinfo.end())
        rv = i->second;
    else
        rv = NULL;
    playernamelock.ReleaseReadLock();
    return rv;
}

void ObjectMgr::AddPlayerInfo(PlayerInfo *pn)
{
    playernamelock.AcquireWriteLock();
    m_playersinfo[pn->guid] =  pn ;
    string pnam = string(pn->name);
    HEARTHSTONE_TOLOWER(pnam);
    m_playersInfoByName[pnam] = pn;
    playernamelock.ReleaseWriteLock();
}

void ObjectMgr::RenamePlayerInfo(PlayerInfo * pn, const char * oldname, const char * newname)
{
    playernamelock.AcquireWriteLock();
    string oldn = string(oldname);
    HEARTHSTONE_TOLOWER(oldn);

    PlayerNameStringIndexMap::iterator itr = m_playersInfoByName.find( oldn );
    if( itr != m_playersInfoByName.end() && itr->second == pn )
    {
        string newn = string(newname);
        HEARTHSTONE_TOLOWER(newn);
        m_playersInfoByName.erase( itr );
        m_playersInfoByName[newn] = pn;
    }

    playernamelock.ReleaseWriteLock();
}

void ObjectMgr::LoadSpellSkills()
{
    uint32 i;
//  int total = sSkillStore.GetNumRows();

    for(i = 0; i < dbcSkillLineSpell.GetNumRows(); i++)
    {
        SkillLineSpell *sp = dbcSkillLineSpell.LookupRow(i);
        if (sp)
        {
            mSpellSkills[sp->spell] = sp;
        }
    }
    sLog.Notice("ObjectMgr", "%u spell skills loaded.", mSpellSkills.size());
}

SkillLineSpell* ObjectMgr::GetSpellSkill(uint32 id)
{
    return mSpellSkills[id];
}

void ObjectMgr::LoadPlayersInfo()
{
    PlayerInfo * pn;
    QueryResult *result = CharacterDatabase.Query("SELECT guid,name,race,class,level,gender,zoneId,timestamp,acct,instance_id,mapId,positionX,positionY,positionZ,orientation FROM characters");
    uint32 period, c;
    if(result)
    {
        period = (result->GetRowCount() / 20) + 1;
        c = 0;

        do
        {
            Field *fields = result->Fetch();
            pn = new PlayerInfo;
            memset(pn, 0, sizeof(PlayerInfo));
            pn->guid = fields[0].GetUInt32();
            pn->name = strdup(fields[1].GetString());
            pn->race = fields[2].GetUInt8();
            pn->_class = fields[3].GetUInt8();
            pn->lastLevel = fields[4].GetUInt32();
            pn->gender = fields[5].GetUInt8();
            pn->lastZone=fields[6].GetUInt32();
            pn->lastOnline=fields[7].GetUInt32();
            pn->acct = fields[8].GetUInt32();
            pn->curInstanceID = fields[9].GetUInt32();
            pn->lastmapid = fields[10].GetUInt32();
            pn->lastpositionx = fields[11].GetFloat();
            pn->lastpositiony = fields[12].GetFloat();
            pn->lastpositionz = fields[13].GetFloat();
            pn->lastorientation = fields[14].GetFloat();
            CharRaceEntry * race = dbcCharRace.LookupEntry(pn->race);
            pn->team = race->TeamId;

            if( GetPlayerInfoByName(pn->name) != NULL )
            {
                // gotta rename him
                char temp[300];
                snprintf(temp, 300, "%s__%X__", pn->name, pn->guid);
                sLog.Notice("ObjectMgr", "Renaming duplicate player %s to %s. (%u)", pn->name,temp,pn->guid);
                CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s', forced_rename_pending = 1 WHERE guid = %u",
                    CharacterDatabase.EscapeString(string(temp)).c_str(), pn->guid);

                free(pn->name);
                pn->name = strdup(temp);
            }

            string lpn=string(pn->name);
            HEARTHSTONE_TOLOWER(lpn);
            m_playersInfoByName[lpn] = pn;

            //this is startup -> no need in lock -> don't use addplayerinfo
            m_playersinfo[(uint32)pn->guid] = pn;

            if( !((++c) % period) )
                sLog.Notice("PlayerInfo", "Done %u/%u, %u%% complete.", c, result->GetRowCount(), float2int32( (float(c) / float(result->GetRowCount()))*100.0f ));
        } while( result->NextRow() );

        delete result;
    }
    sLog.Notice("ObjectMgr", "%u players loaded.", m_playersinfo.size());
}

PlayerInfo* ObjectMgr::GetPlayerInfoByName(const char * name)
{
    string lpn = string(name);
    HEARTHSTONE_TOLOWER(lpn);
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
    QueryResult *result = WorldDatabase.Query( "SELECT * FROM playercreateinfo" );

    if( result == NULL )
    {
        sLog.Error("MySQL","Query failed: SELECT * FROM playercreateinfo");
        return;
    }

    if( result->GetFieldCount() < 25 )
    {
        sLog.Error("PlayerCreateInfo", "Incorrect number of columns in playercreateinfo found %u, should be 25. check for sql updates", result->GetFieldCount());
        delete result;
        return;
    }

    PlayerCreateInfo *pPlayerCreateInfo;
    int fieldcount = 0;

    do
    {
        Field *fields = result->Fetch();
        fieldcount = 0;

        pPlayerCreateInfo = new PlayerCreateInfo;
        pPlayerCreateInfo->index = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->race = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->factiontemplate = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->class_ = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->mapId = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->zoneId = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->positionX = fields[fieldcount++].GetFloat();
        pPlayerCreateInfo->positionY = fields[fieldcount++].GetFloat();
        pPlayerCreateInfo->positionZ = fields[fieldcount++].GetFloat();
        pPlayerCreateInfo->Orientation = fields[fieldcount++].GetFloat();
        pPlayerCreateInfo->displayId = fields[fieldcount++].GetUInt16();
        pPlayerCreateInfo->strength = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->ability = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->stamina = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->intellect = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->spirit = fields[fieldcount++].GetUInt8();
        pPlayerCreateInfo->health = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->mana = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->rage = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->focus = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->energy = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->runic = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->attackpower = fields[fieldcount++].GetUInt32();
        pPlayerCreateInfo->mindmg = fields[fieldcount++].GetFloat();
        pPlayerCreateInfo->maxdmg = fields[fieldcount++].GetFloat();

        QueryResult *sk_sql = WorldDatabase.Query("SELECT * FROM playercreateinfo_skills WHERE indexid = %u", pPlayerCreateInfo->index);
        if(sk_sql)
        {
            do
            {
                Field *fields = sk_sql->Fetch();
                CreateInfo_SkillStruct tsk;
                tsk.skillid = fields[1].GetUInt32();
                tsk.currentval = fields[2].GetUInt32();
                tsk.maxval = fields[3].GetUInt32();
                pPlayerCreateInfo->skills.push_back(tsk);
            } while(sk_sql->NextRow());
            delete sk_sql;
        }

        QueryResult *sp_sql = WorldDatabase.Query("SELECT * FROM playercreateinfo_spells WHERE indexid = %u", pPlayerCreateInfo->index);
        if(sp_sql)
        {
            do
            {
                pPlayerCreateInfo->spell_list.insert(sp_sql->Fetch()[1].GetUInt32());
            } while(sp_sql->NextRow());
            delete sp_sql;
        }

        QueryResult *items_sql = WorldDatabase.Query("SELECT * FROM playercreateinfo_items WHERE indexid = %u", pPlayerCreateInfo->index);
        if(items_sql)
        {
            do
            {
                Field *fields = items_sql->Fetch();
                CreateInfo_ItemStruct itm;
                itm.protoid = fields[1].GetUInt32();
                itm.slot = fields[2].GetUInt8();
                itm.amount = fields[3].GetUInt32();
                pPlayerCreateInfo->items.push_back(itm);
            } while(items_sql->NextRow());
            delete items_sql;
        }

        QueryResult *bars_sql = WorldDatabase.Query("SELECT * FROM playercreateinfo_bars WHERE class = %u",pPlayerCreateInfo->class_ );
        if(bars_sql)
        {
            do
            {
                Field *fields = bars_sql->Fetch();
                CreateInfo_ActionBarStruct bar;
                bar.button = fields[2].GetUInt8();
                bar.action = fields[3].GetUInt32();
                bar.type = fields[4].GetUInt8();
                pPlayerCreateInfo->actionbars.push_back(bar);
            } while(bars_sql->NextRow());
            delete bars_sql;
        }

        mPlayerCreateInfo[pPlayerCreateInfo->index] = pPlayerCreateInfo;
    } while( result->NextRow() );
    delete result;

    sLog.Notice("ObjectMgr", "%u player create infos loaded.", mPlayerCreateInfo.size());
    GenerateLevelUpInfo();
}

Corpse* ObjectMgr::LoadCorpse(uint32 guid)
{
    QueryResult *result = CharacterDatabase.Query("SELECT * FROM Corpses WHERE guid =%u ", guid );
    if( result == NULL )
        return NULLCORPSE;

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
        pCorpse->LoadValues( fields[11].GetString());
        if(pCorpse->GetUInt32Value(CORPSE_FIELD_DISPLAY_ID) == 0)
        {
            RemoveCorpse(pCorpse);
            pCorpse->Destruct();
            pCorpse = NULL;
            continue;
        }

        pCorpse->SetLoadedFromDB(true);
        pCorpse->SetInstanceID(fields[8].GetUInt32());
        pCorpse->AddToWorld();
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
    CorpseMap::const_iterator itr;
    Corpse* rv = NULLCORPSE;
    _corpseslock.Acquire();
    for (itr = m_corpses.begin();itr != m_corpses.end();)
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
    sEventMgr.AddEvent(c, &Corpse::Delink, EVENT_CORPSE_SPAWN_BONES, 1, 1, 0);
    CorpseAddEventDespawn(c);
}

void ObjectMgr::LoadGMTickets()
{
    QueryResult *result = CharacterDatabase.Query( "SELECT guid, playerGuid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment FROM gm_tickets WHERE deleted = 0" );

    if(result == NULL)
        return;

    GM_Ticket *ticket;
    do
    {
        Field *fields = result->Fetch();

        ticket = new GM_Ticket;
        ticket->guid = fields[0].GetUInt64();
        ticket->playerGuid = fields[1].GetUInt64();
        ticket->name = fields[2].GetString();
        ticket->level = fields[3].GetUInt32();
        ticket->map = fields[4].GetUInt32();
        ticket->posX = fields[5].GetFloat();
        ticket->posY = fields[6].GetFloat();
        ticket->posZ = fields[7].GetFloat();
        ticket->message = fields[8].GetString();
        ticket->timestamp = fields[9].GetUInt32();
        ticket->deleted = false;

        ticket->assignedToPlayer = fields[11].GetUInt64();
        ticket->comment = fields[12].GetString();

        AddGMTicket(ticket, true);

    } while( result->NextRow() );

    sLog.Notice("ObjectMgr", "%u open GM Tickets loaded.", result->GetRowCount());
    delete result;
}

void ObjectMgr::SaveGMTicket(GM_Ticket* ticket, QueryBuffer * buf)
{
    std::stringstream ss;
    ss << "REPLACE INTO gm_tickets (`guid`, `playerGuid`, `name`, `level`, `map`, `posX`, `posY`, `posZ`, `message`, `timestamp`, `deleted`, `assignedto`, `comment`) VALUES(";
    ss << ticket->guid << ", ";
    ss << ticket->playerGuid << ", '";
    ss << CharacterDatabase.EscapeString(ticket->name) << "', ";
    ss << ticket->level << ", ";
    ss << ticket->map << ", ";
    ss << ticket->posX << ", ";
    ss << ticket->posY << ", ";
    ss << ticket->posZ << ", '";
    ss << CharacterDatabase.EscapeString(ticket->message) << "', ";
    ss << ticket->timestamp << ", ";

    if( ticket->deleted  )
        ss << uint32( 1 );
    else
        ss << uint32( 0 );
    ss << ",";

    ss << ticket->assignedToPlayer << ", '";
    ss << CharacterDatabase.EscapeString(ticket->comment) << "');";

    if(buf == NULL)
        CharacterDatabase.ExecuteNA(ss.str( ).c_str( ));
    else
        buf->AddQueryStr(ss.str());
}

void ObjectMgr::LoadQuestPOI()
{
    uint32 count = 0;
    uint32 pointcount = 0;
    map< uint32, map<uint32, vector<QuestPOIPoint*> > > PoiMap;
    PoiMap.clear();
    QueryResult *points = WorldDatabase.Query("SELECT * FROM quest_poi_points ORDER BY questId, id, internalIndex ASC");
    if (points)
    {
        do
        {
            Field *pointFields = points->Fetch();
            uint32 questId = pointFields[0].GetUInt32();
            uint32 poiId = pointFields[1].GetUInt32();
            int32 x = pointFields[3].GetInt32();
            int32 y = pointFields[4].GetInt32();
            QuestPOIPoint* point = new QuestPOIPoint(x, y);
            PoiMap[questId][poiId].push_back(point);
            pointcount++;
        }while (points->NextRow());
        delete points;
    }

    QueryResult *result = WorldDatabase.Query("SELECT * FROM quest_poi ORDER BY questId, id ASC");
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 questId          = fields[0].GetUInt32();
            uint32 id               = fields[1].GetUInt32();
            int32 objIndex          = fields[2].GetInt32();
            uint32 mapId            = fields[3].GetUInt32();
            uint32 WorldMapAreaId   = fields[4].GetUInt32();
            uint32 FloorId          = fields[5].GetUInt32();
            uint32 unk3             = fields[6].GetUInt32();
            uint32 unk4             = fields[7].GetUInt32();
            QuestPOI POI(id, objIndex, mapId, WorldMapAreaId, FloorId, unk3, unk4);
            if(PoiMap[questId][id].size())
                for(vector<QuestPOIPoint*>::iterator itr = PoiMap[questId][id].begin(); itr != PoiMap[questId][id].end(); itr++)
                    POI.points.push_back(*(*itr));
            mQuestPOIMap[questId].push_back(POI);
            count++;
        } while (result->NextRow());
        delete result;
    }
    PoiMap.clear();
    sLog.Notice("ObjectMgr", "%u quest POI definitions, %u POI points", count, pointcount);
}

void ObjectMgr::LoadRecallPoints()
{
    QueryResult *result = WorldDatabase.Query( "SELECT * FROM recall");
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            RecallLocation *loc = new RecallLocation();
            loc->lowercase_name = loc->RealName = std::string(fields[0].GetString());
            HEARTHSTONE_TOLOWER(loc->lowercase_name);
            loc->mapId = fields[1].GetUInt32();
            loc->x = fields[2].GetFloat();
            loc->y = fields[3].GetFloat();
            loc->z = fields[4].GetFloat();
            loc->orient = fields[5].GetFloat();
            m_recallLocations.insert(loc);
        }while (result->NextRow());
        delete result;
    }

    sLog.Notice("ObjectMgr", "Loaded %u recall points", m_recallLocations.size());
}

void ObjectMgr::SetHighestGuids()
{
    QueryResult *result;

    result = CharacterDatabase.Query( "SELECT MAX(guid) FROM characters" );
    if( result )
    {
        m_hiPlayerGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM playeritems");
    if( result )
    {
        m_hiItemGuid = (uint32)result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query( "SELECT MAX(guid) FROM corpses" );
    if( result )
    {
        m_hiCorpseGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(id) FROM creature_spawns");
    if(result)
    {
        m_hiCreatureSpawnId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(id) FROM gameobject_spawns");
    if(result)
    {
        m_hiGameObjectSpawnId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(group_id) FROM groups");
    if(result)
    {
        m_hiGroupId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM gm_tickets");
    if(result)
    {
        m_ticketid = result->Fetch()[0].GetUInt64() + 1;
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(setguid) FROM equipmentsets");
    if(result)
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
    sLog.Notice("ObjectMgr", "HighGuid(CONTAINER) = %u", m_hiContainerGuid);
    sLog.Notice("ObjectMgr", "HighGuid(GROUP) = %u", m_hiGroupId);
    sLog.Notice("ObjectMgr", "HighGuid(TICKET) = %u", m_ticketid - 1);
    sLog.Notice("ObjectMgr", "HighGuid(EQSETS) = %u", m_equipmentSetGuid);
}

void ObjectMgr::ListGuidAmounts()
{
    QueryResult *result;
    uint32 amount[8];
    std::string name[8] = {"Characters", "Player Items", "Corpses", "Groups", "GM Tickets", "Creatures", "Gameobjects", "Vehicles"};

    result = CharacterDatabase.Query("SELECT guid FROM characters");
    if(result)
    {
        amount[0] = result->GetRowCount();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT guid FROM playeritems");
    if(result)
    {
        amount[1] = result->GetRowCount();
        delete result;
    }

    result = CharacterDatabase.Query( "SELECT guid FROM corpses" );
    if(result)
    {
        amount[2] = result->GetRowCount();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT group_id FROM groups");
    if(result)
    {
        amount[3] = result->GetRowCount();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT guid FROM gm_tickets");
    if(result)
    {
        amount[4] = result->GetRowCount();
        delete result;
    }

    result = WorldDatabase.Query("SELECT id FROM creature_spawns");
    if(result)
    {
        amount[5] = result->GetRowCount();
        delete result;
    }

    result = WorldDatabase.Query("SELECT id FROM gameobject_spawns");
    if(result)
    {
        amount[6] = result->GetRowCount();
        delete result;
    }

    result = WorldDatabase.Query("SELECT id FROM creature_spawns WHERE vehicle > '0'");
    if(result)
    {
        amount[7] = result->GetRowCount();
        delete result;
    }

    for(int i = 0; i < 8; i++)
        sLog.Notice("ObjectMgr", "Load Amount(%s) = %u", name[i].c_str(), amount[i] ? amount[i] : 0);
}

uint64 ObjectMgr::GenerateTicketID()
{
    return m_ticketid++;
}

uint32 ObjectMgr::GenerateMailID()
{
    return m_mailid++;
}

uint64 ObjectMgr::GenerateEquipmentSetGuid()
{
    return m_equipmentSetGuid++;
}

uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{
    ASSERT(guidhigh == HIGHGUID_TYPE_ITEM || guidhigh == HIGHGUID_TYPE_CONTAINER || guidhigh == HIGHGUID_TYPE_PLAYER);

    uint32 ret;
    if(guidhigh == HIGHGUID_TYPE_ITEM)
    {
        m_guidGenMutex.Acquire();
        ret = ++m_hiItemGuid;
        m_guidGenMutex.Release();
    }else if(guidhigh==HIGHGUID_TYPE_PLAYER)
    {
        m_playerguidlock.Acquire();
        ret = ++m_hiPlayerGuid;
        m_playerguidlock.Release();
    }else{
        m_guidGenMutex.Acquire();
        ret = ++m_hiContainerGuid;
        m_guidGenMutex.Release();
    }
    return ret;
}

void ObjectMgr::ProcessGameobjectQuests()
{
    QueryResult *result = WorldDatabase.Query("SELECT * FROM npc_gossip_textid");
    if(result)
    {
        uint32 entry, text;
        do
        {
            entry = result->Fetch()[0].GetUInt32();
            text  = result->Fetch()[1].GetUInt32();

            mNpcToGossipText[entry] = text;

        } while(result->NextRow());
        delete result;
    }
    sLog.Notice("ObjectMgr", "%u NPC Gossip TextIds loaded.", mNpcToGossipText.size());
}

Player* ObjectMgr::GetPlayer(const char* name, bool caseSensitive)
{
    Player * rv = NULLPLR;
    PlayerStorageMap::const_iterator itr;
    _playerslock.AcquireReadLock();

    if(!caseSensitive)
    {
        std::string strName = name;
        HEARTHSTONE_TOLOWER(strName);
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

Player* ObjectMgr::GetPlayer(uint32 guid)
{
    Player * rv = NULLPLR;

    _playerslock.AcquireReadLock();
    PlayerStorageMap::const_iterator itr = _players.find(guid);
    rv = (itr != _players.end()) ? itr->second : NULLPLR;
    _playerslock.ReleaseReadLock();

    return rv;
}

PlayerCreateInfo* ObjectMgr::GetPlayerCreateInfo(uint8 race, uint8 class_) const
{
    PlayerCreateInfoMap::const_iterator itr;
    for (itr = mPlayerCreateInfo.begin(); itr != mPlayerCreateInfo.end(); itr++)
    {
        if( (itr->second->race == race) && (itr->second->class_ == class_) )
            return itr->second;
    }
    return NULL;
}

void ObjectMgr::AddGMTicket(GM_Ticket *ticket, bool startup)
{
    ASSERT( ticket );
    GM_TicketList.push_back(ticket);

    // save
    if(!startup)
        SaveGMTicket(ticket, NULL);
}

void ObjectMgr::UpdateGMTicket(GM_Ticket *ticket)
{
    SaveGMTicket(ticket, NULL);
}

void ObjectMgr::DeleteGMTicketPermanently(uint64 ticketGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->guid == ticketGuid)
            i = GM_TicketList.erase(i);
        else
            ++i;
    }

    // kill from db
    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE guid=%u", ticketGuid);
}

void ObjectMgr::DeleteAllRemovedGMTickets()
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->deleted)
            i = GM_TicketList.erase(i);
        else
            ++i;
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE deleted=1");
}

void ObjectMgr::RemoveGMTicketByPlayer(uint64 playerGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            SaveGMTicket((*i), NULL);
        }
        ++i;
    }
}

void ObjectMgr::RemoveGMTicket(uint64 ticketGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            SaveGMTicket((*i), NULL);
        }
        ++i;
    }
}

GM_Ticket* ObjectMgr::GetGMTicketByPlayer(uint64 playerGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            return (*i);
        }
        ++i;
    }
    return NULL;
}

GM_Ticket* ObjectMgr::GetGMTicket(uint64 ticketGuid)
{
    for(GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if((*i)->guid == ticketGuid)
        {
            return (*i);
        }
        ++i;
    }
    return NULL;
}

void ObjectMgr::LoadVendors()
{
    HM_NAMESPACE::hash_map<uint32, std::map<uint32, CreatureItem>*>::const_iterator itr;
    std::map<uint32, CreatureItem> *items;
    CreatureItem itm;

    QueryResult *result = WorldDatabase.Query("SELECT * FROM vendors ORDER BY listindex");
    if( result != NULL )
    {
        if( result->GetFieldCount() < 9 )
        {
            sLog.Notice("ObjectMgr", "Invalid format in vendors (%u/7) columns, not enough data to proceed.\n", result->GetFieldCount() );
            delete result;
            return;
        }
        else if( result->GetFieldCount() > 9 )
            sLog.Notice("ObjectMgr", "Invalid format in vendors (%u/7) columns, loading anyway because we have enough data\n", result->GetFieldCount() );

        do
        {
            Field* fields = result->Fetch();

            itr = mVendors.find(fields[0].GetUInt32());
            if( itr == mVendors.end() )
            {
                items = new std::map<uint32, CreatureItem>;
                mVendors[fields[0].GetUInt32()] = items;
            }
            else
                items = itr->second;

            itm.itemid              = fields[2].GetUInt32();
            itm.amount              = fields[3].GetUInt32();
            itm.available_amount    = fields[4].GetUInt32();
            itm.max_amount          = fields[4].GetUInt32();
            itm.incrtime            = fields[5].GetUInt32();
            itm.extended_cost       = NULL;
            itm.IsDependent         = fields[7].GetBool();
            itm.vendormask          = fields[8].GetUInt32();

            uint32 ec = fields[6].GetUInt32();
            if( ec != 0 )
            {
                itm.extended_cost = dbcItemExtendedCost.LookupEntry(ec);
                if( itm.extended_cost == NULL )
                {
                    if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                        WorldDatabase.Execute("UPDATE vendors set extendedcost = '0' where item = '%u' AND entry = '%u'", itm.itemid, fields[0].GetUInt32());
                    sLog.Warning("ObjectMgr","Item %u at vendor %u has extended cost %u which is invalid. Skipping.", itm.itemid, fields[0].GetUInt32(), ec);
                    continue;
                }
            }

            uint32 slot = 1;
            if(items->size())
                slot = items->rbegin()->first+1;
            items->insert(make_pair(slot, itm) );
        }while( result->NextRow() );
        delete result;
    }
    sLog.Notice("ObjectMgr", "%u vendors loaded.", mVendors.size());
}

void ObjectMgr::ReloadVendors()
{
    mVendors.clear();
    LoadVendors();
}

std::map<uint32, CreatureItem>* ObjectMgr::GetVendorList(uint32 entry)
{
    return mVendors[entry];
}

void ObjectMgr::LoadTotemSpells()
{
    std::stringstream query;
    QueryResult *result = WorldDatabase.Query( "SELECT * FROM totemspells" );

    if(result != NULL)
    {
        SpellEntry * sp;
        do
        {
            Field *fields = result->Fetch();
            if(!(fields[1].GetUInt32()))
                continue;

            sp = dbcSpell.LookupEntry(fields[1].GetUInt32());
            if(!sp)
                continue;

            m_totemSpells.insert( TotemSpellMap::value_type( fields[0].GetUInt32(), sp ));
        } while( result->NextRow() );
        delete result;
    }

    sLog.Notice("ObjectMgr", "%u totem spells loaded.", m_totemSpells.size());
}

SpellEntry* ObjectMgr::GetTotemSpell(uint32 spellId)
{
    return m_totemSpells[spellId];
}

void ObjectMgr::LoadAIThreatToSpellId()
{
    QueryResult *result = WorldDatabase.Query( "SELECT * FROM ai_threattospellid" );

    if(result == NULL)
        return;

    uint32 spellid;
    SpellEntry * sp;

    do
    {
        Field *fields = result->Fetch();
        spellid = fields[0].GetUInt32();
        sp = dbcSpell.LookupEntry( spellid );
        if( sp != NULL )
            sp->ThreatForSpell = fields[1].GetUInt32();
        else
        {
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
            {
                WorldDatabase.Execute( "DELETE FROM ai_threattospellid where spell = '%u'", spellid);
            }
            sLog.Warning("AIThreatSpell", "Cannot apply to spell %u; spell is nonexistant.", spellid);
        }
        spellid = 0;

    } while( result->NextRow() );

    delete result;
}

void ObjectMgr::LoadSpellFixes()
{
    SpellEntry* sp;
    QueryResult * result = WorldDatabase.Query("SELECT * FROM spellfixes");
    int i, j;

    if(result)
    {
        if( result->GetFieldCount() != 13 )
        {
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, "Incorrect column count at spellfixes, skipping, please fix it.", "", NULL);
            delete result;
            return;
        }

        sLog.Notice("ObjectMgr", "%u spell fixes from database...", result->GetRowCount());

        do
        {
            Field * f = result->Fetch();
            uint32 sf_spellId = f[0].GetUInt32();
            uint32 sf_procFlags = f[1].GetUInt32();
            uint32 sf_procChance = f[2].GetUInt32();
            uint32 sf_procCharges = f[3].GetUInt32();
            uint32 sf_effectClassMask[3][3];

            sf_effectClassMask[0][0] = f[4].GetUInt32();
            sf_effectClassMask[0][1] = f[5].GetUInt32();
            sf_effectClassMask[0][2] = f[6].GetUInt32();
            sf_effectClassMask[1][0] = f[7].GetUInt32();
            sf_effectClassMask[1][1] = f[8].GetUInt32();
            sf_effectClassMask[1][2] = f[9].GetUInt32();
            sf_effectClassMask[2][0] = f[10].GetUInt32();
            sf_effectClassMask[2][1] = f[11].GetUInt32();
            sf_effectClassMask[2][2] = f[12].GetUInt32();

            if( sf_spellId )
            {
                sp = dbcSpell.LookupEntry( sf_spellId );
                if( sp != NULL )
                {
                    if( sf_procFlags )
                        sp->procFlags = sf_procFlags;

                    if( sf_procChance )
                        sp->procChance = sf_procChance;

                    if ( sf_procCharges )
                        sp->procCharges = sf_procCharges;

                    for(i = 0; i < 3; i++)
                        for(j = 0; j < 3; ++j)
                            if(sf_effectClassMask[i][j])
                                sp->EffectSpellClassMask[i][j] = sf_effectClassMask[i][j];
                }
                else
                {
                    WorldDatabase.Execute("DELETE FROM SpellFixes where spellid = '%u'", sf_spellId);
                }
            }
        }while(result->NextRow());
        delete result;
    }
}

Item* ObjectMgr::CreateItem(uint32 entry,Player* owner)
{
    ItemPrototype * proto = ItemPrototypeStorage.LookupEntry(entry);
    if(!proto)
        return NULLITEM;

    if(proto->InventoryType == INVTYPE_BAG)
    {
        Container* pContainer(new Container(HIGHGUID_TYPE_CONTAINER,GenerateLowGuid(HIGHGUID_TYPE_CONTAINER)));
        pContainer->Create( entry, owner);
        pContainer->SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
        return pContainer;
    }
    else
    {
        Item* pItem(new Item(HIGHGUID_TYPE_ITEM,GenerateLowGuid(HIGHGUID_TYPE_ITEM)));
        pItem->Create(entry, owner);
        pItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
        return pItem;
    }
}

Item* ObjectMgr::LoadItem(uint64 guid)
{
    QueryResult * result = CharacterDatabase.Query("SELECT * FROM playeritems WHERE guid = %u", GUID_LOPART(guid));
    Item* pReturn = NULLITEM;

    if(result)
    {
        ItemPrototype * pProto = ItemPrototypeStorage.LookupEntry(result->Fetch()[2].GetUInt32());
        if(!pProto)
            return NULLITEM;

        if(pProto->InventoryType == INVTYPE_BAG)
        {
            Container* pContainer(new Container(HIGHGUID_TYPE_CONTAINER,(uint32)guid));
            pContainer->LoadFromDB(result->Fetch());
            pReturn = pContainer;
        }
        else
        {
            Item* pItem(new Item(HIGHGUID_TYPE_ITEM,(uint32)guid));
            pItem->LoadFromDB(result->Fetch(), NULLPLR, false);
            pReturn = pItem;
        }
        delete result;
    }

    return pReturn;
}

void ObjectMgr::LoadCorpses(MapMgr* mgr)
{
    QueryResult *result = CharacterDatabase.Query("SELECT * FROM corpses WHERE mapId = %u", mgr->GetMapId());
    if(result)
    {
        Corpse* pCorpse = NULLCORPSE;
        do
        {
            Field *fields = result->Fetch();
            uint32 instanceid = fields[10].GetUInt32();
            if(instanceid && mgr->GetMapInfo()->type != 0)
                if(instanceid != mgr->GetInstanceID())
                    continue;
            pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE, fields[0].GetUInt32());
            pCorpse->Init();
            pCorpse->SetInstanceID(instanceid);
            pCorpse->Create( fields[1].GetUInt64(), fields[9].GetUInt32(), fields[4].GetFloat(), fields[5].GetFloat(), fields[6].GetFloat(), fields[7].GetFloat());
            pCorpse->SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, fields[2].GetUInt32()); 
            pCorpse->SetUInt32Value(CORPSE_FIELD_FLAGS, fields[3].GetUInt32());
            pCorpse->SetZoneId(fields[8].GetUInt32());
            pCorpse->LoadValues( fields[11].GetString());
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
        pCorpse = NULLCORPSE;
    }
    else
        sEventMgr.AddEvent(pCorpse->GetMapMgr(), &MapMgr::EventCorpseDespawn, pCorpse->GetGUID(), EVENT_CORPSE_DESPAWN, 600000, 1,0);
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
        if(c != NULLCORPSE)
        {
            if(c->IsInWorld())
                c->RemoveFromWorld(false);
            c->Destruct();
            c = NULLCORPSE;
        }
    }
    m_corpses.clear();
    _corpseslock.Release();
}

GossipMenu::GossipMenu(uint64 Creature_Guid, uint32 Text_Id) : TextId(Text_Id), CreatureGuid(Creature_Guid)
{

}

void GossipMenu::AddItem(uint8 Icon, const char* Text, int32 Id, bool Coded, uint32 BoxMoney, const char* BoxMessage)
{
    GossipMenuItem GossipItem;
    GossipItem.Id = (uint32)Menu.size();
    if(Id > 0)
        GossipItem.IntId = Id;
    else
        GossipItem.IntId = GossipItem.Id;

    GossipItem.Icon = Icon;
    GossipItem.Coded = Coded;
    GossipItem.BoxMoney = BoxMoney;
    GossipItem.Text = Text;
    GossipItem.BoxMessage = BoxMessage;

    Menu.push_back(GossipItem);
}

void GossipMenu::AddItem(GossipMenuItem* GossipItem)
{
    Menu.push_back(*GossipItem);
}

void GossipMenu::BuildPacket(WorldPacket& Packet)
{
    Packet << uint64(CreatureGuid);
    Packet << uint32(0);
    Packet << uint32(TextId);
    Packet << uint32(Menu.size());

    for(std::vector<GossipMenuItem>::iterator iter = Menu.begin();
        iter != Menu.end(); iter++)
    {
        Packet << iter->Id;
        Packet << iter->Icon;
        Packet << iter->Coded;      // makes pop up box password
        Packet << iter->BoxMoney;   // money required to open menu, 2.0.3
        Packet << iter->Text;       // text for gossip item
        Packet << iter->BoxMessage; // accept text (related to money) pop up box, 2.0.3
    }
}

void GossipMenu::SendTo(Player* Plr)
{
    WorldPacket data(SMSG_GOSSIP_MESSAGE, Menu.size() * 50 + 12);
    BuildPacket(data);
    data << uint32(0);  // 0 quests obviously
    Plr->GetSession()->SendPacket(&data);
}

void ObjectMgr::CreateGossipMenuForPlayer(GossipMenu** Location, uint64 Guid, uint32 TextID, Player* Plr)
{
    GossipMenu *Menu = new GossipMenu(Guid, TextID);
    ASSERT(Menu);

    if(Plr->CurrentGossipMenu != NULL)
        delete Plr->CurrentGossipMenu;

    Plr->CurrentGossipMenu = Menu;
    *Location = Menu;
}

GossipMenuItem GossipMenu::GetItem(uint32 Id)
{
    if(Id >= Menu.size())
    {
        GossipMenuItem k;
        k.IntId = 1;
        return k;
    } else {
        return Menu[Id];
    }
}

uint32 ObjectMgr::GetGossipTextForNpc(uint32 ID)
{
    return mNpcToGossipText[ID];
}

void ObjectMgr::LoadTrainers()
{
    LoadDisabledSpells();
    QueryResult* result = WorldDatabase.Query("SELECT * FROM trainer_defs");
    if(result == NULL)
        return;

    QueryResult* result2 = WorldDatabase.Query("SELECT * FROM trainer_spells");
    if(result2 == NULL)
        return;

    if(result2->GetFieldCount() != 10)
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, "Trainers table format is invalid. Please update your database.");
        delete result;
        return;
    }

    Field* fields2;
    uint32 entry = NULL;
    uint32 CastSpellID = NULL;
    uint32 LearnSpellID = NULL;
    hash_map< uint32, uint32 > TSCounterMap;
    hash_map< uint32, hash_map< uint32, TrainerSpell > > TSMap;
    do
    {
        bool abrt = false;
        TrainerSpell ts;
        fields2 = result2->Fetch();

        entry = fields2[0].GetUInt32();
        CastSpellID = fields2[1].GetUInt32();
        LearnSpellID = fields2[2].GetUInt32();

        ts.pCastSpell = NULL;
        ts.pLearnSpell = NULL;
        ts.pCastRealSpell = NULL;

        if( CastSpellID != 0 )
        {
            ts.pCastSpell = dbcSpell.LookupEntry( CastSpellID );
            if( ts.pCastSpell )
            {
                for( int k = 0; k < 3; ++k )
                {
                    if( ts.pCastSpell->Effect[k] == SPELL_EFFECT_LEARN_SPELL )
                    {
                        ts.pCastRealSpell = dbcSpell.LookupEntry(ts.pCastSpell->EffectTriggerSpell[k]);
                        if( ts.pCastRealSpell == NULL )
                        {
                            sLog.Warning("Trainers", "Trainer %u contains cast spell %u that is non-teaching", entry, CastSpellID);
                            abrt = true;
                        }break;
                    }
                }
            }

            if( abrt )
                continue;
        }

        if( LearnSpellID != 0 )
            ts.pLearnSpell = dbcSpell.LookupEntry( LearnSpellID );

        if( ts.pCastSpell == NULL && ts.pLearnSpell == NULL )
        {
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
            {
                if(ts.pCastSpell == NULL)
                    WorldDatabase.Execute("DELETE FROM trainer_spells where entry='%u' AND learn_spell='%u'",entry, LearnSpellID);
                else
                    WorldDatabase.Execute("DELETE FROM trainer_spells where entry='%u' AND cast_spell='%u'",entry, CastSpellID);
            }
            sLog.Warning("ObjectMgr", "Trainer %u skipped invalid spell (%u/%u).", entry, CastSpellID, LearnSpellID);
            continue; //omg a bad spell !
        }

        if( ts.pCastSpell && !ts.pCastRealSpell )
            continue;

        ts.Cost = fields2[3].GetUInt32();
        ts.RequiredSpell = fields2[4].GetUInt32();
        ts.RequiredSkillLine = fields2[5].GetUInt32();
        ts.RequiredSkillLineValue = fields2[6].GetUInt32();
        ts.RequiredLevel = fields2[7].GetUInt32();
        ts.DeleteSpell = fields2[8].GetUInt32();
        ts.IsProfession = (fields2[9].GetUInt32() != 0) ? true : false;
        TSMap[entry][TSCounterMap[entry]++] = ts;
    }while(result2->NextRow());
    delete result2;

    Trainer* tr = NULL;
    GossipText* text;
    const char* temp;
    size_t len;

    do
    {
        Field * fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        CreatureInfo* trainer_info = CreatureNameStorage.LookupEntry(entry);
        if(!trainer_info)
        {
            sLog.Warning("Trainers", "NPC id for Trainer %u does not exist, skipping.", entry);
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
            {
                WorldDatabase.Execute("DELETE FROM trainer_defs where entry='%u'",entry);
                WorldDatabase.Execute("DELETE FROM trainer_spells where entry='%u'",entry);
            }
            continue;
        }

        tr = new Trainer;
        tr->RequiredSkill = fields[1].GetUInt32();
        tr->RequiredSkillLine = fields[2].GetUInt32();
        tr->RequiredClass = fields[3].GetUInt32();
        tr->TrainerType = fields[4].GetUInt32();

        temp = fields[5].GetString();
        len = strlen(temp);
        if(!len)
        {
            temp = "What can I teach you $N?";
            len = strlen(temp);
        }
        tr->UIMessage = new char[len+1];
        strncpy(tr->UIMessage, temp, len);
        tr->UIMessage[len] = 0;

        uint32 tmptxtid[2];
        for( uint32 i = 0; i < 2; i++)
        {
            tmptxtid[i] = fields[6+i].GetUInt32();
            if( tmptxtid[i] )
            {
                text = NpcTextStorage.LookupEntry(tmptxtid[i]);
                if(text == 0)
                {
                    if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                    {
                        std::string columnname = (i == 0 ? "can_train_gossip_textid" : "cannot_train_gossip_textid");
                        WorldDatabase.Execute("UPDATE trainer_defs SET %s = '0' where entry = '%u'", columnname.c_str(), entry);
                    }
                    sLog.Warning("Trainers", "Trainer %u contains an invalid npc_gossip_id %d.", entry, tmptxtid[i] );
                    tmptxtid[i] = 0;
                }
            }
        }
        tr->Can_Train_Gossip_TextId = tmptxtid[0];
        tr->Cannot_Train_GossipTextId = tmptxtid[1];

        // Start Spell Insertion.
        if(TSMap[entry].size())
        {
            tr->SpellCount = (uint32)TSMap[entry].size();
            hash_map< uint32, TrainerSpell >::iterator itr;
            for(itr = TSMap[entry].begin(); itr != TSMap[entry].end(); itr++)
                tr->Spells.push_back(itr->second);
        }
        // End Spell Insertion.

        //and now we insert it to our lookup table
        if(!tr->SpellCount)
        {
            if(tr->UIMessage)
                delete [] tr->UIMessage;
            delete tr;
            continue;
        }

        mTrainers.insert( TrainerMap::value_type( entry, tr ) );
    } while(result->NextRow());
    delete result;

    TSMap.clear();
    TSCounterMap.clear();
    sLog.Notice("ObjectMgr", "%u trainers loaded.", mTrainers.size());
}

Trainer* ObjectMgr::GetTrainer(uint32 Entry)
{
    TrainerMap::iterator iter = mTrainers.find(Entry);
    if(iter == mTrainers.end())
        return NULL;

    return iter->second;
}

uint32 GetBaseHPForLevel(uint32 level, uint32 Class);
uint32 GetBaseManaForLevel(uint32 level, uint32 Class);

void ObjectMgr::GenerateLevelUpInfo()
{
    // Generate levelup information for each class.
    PlayerCreateInfo * PCI;
    LevelInfo* lvl0 = NULL;
    LevelInfo* lvl = NULL;
    for(uint32 Class = WARRIOR; Class <= DRUID; ++Class)
    {
        // These are empty.
        if(Class == 10)
            continue;

        // Search for a playercreateinfo.
        for(uint32 Race = RACE_HUMAN; Race <= RACE_DRAENEI; Race++ )
        {
            PCI = GetPlayerCreateInfo(Race, Class);
            if(PCI == NULL)
                continue;   // Class not valid for this race.

            // Generate each level's information
            lvl0 = new LevelInfo();
            lvl0->HP = PCI->health;
            lvl0->Mana = PCI->mana;
            lvl0->Stat[0] = PCI->strength;
            lvl0->Stat[1] = PCI->ability;
            lvl0->Stat[2] = PCI->stamina;
            lvl0->Stat[3] = PCI->intellect;
            lvl0->Stat[4] = PCI->spirit;
            lvl0->XPToNextLevel = 400;
            lvl0->BaseHP = PCI->health;
            lvl0->BaseMana = PCI->mana;
            LevelMap * lMap = new LevelMap;

            // Insert into map
            lMap->insert( LevelMap::value_type( 1, lvl0 ) );

            for(uint32 Level = 2; Level <= sWorld.GetMaxLevelStatCalc(); Level++)
            {
                lvl = new LevelInfo;
                memset(lvl, 0, sizeof(LevelInfo));

                // Calculate Stats
                for(uint32 s = 0; s < 5; ++s)
                {
                    lvl->Stat[s] = CalcStatForLevel( Level, Race, Class, s );
                    switch(s)
                    {
                    case STAT_STRENGTH:     lvl->Stat[s] += PCI->strength; break;
                    case STAT_INTELLECT:    lvl->Stat[s] += PCI->intellect; break;
                    case STAT_STAMINA:      lvl->Stat[s] += PCI->stamina; break;
                    case STAT_SPIRIT:       lvl->Stat[s] += PCI->spirit; break;
                    case STAT_AGILITY:      lvl->Stat[s] += PCI->ability; break;
                    }
                }

                uint32 BaseHP = 0;
                uint32 BaseMana = 0;
                if(Level <= MAX_PREDEFINED_NEXTLEVELXP)
                {
                    // Description: We're calculating the Base Mana and HP that we get per level. These are based off of
                    // the total value at level 80 and are probably incorrect at lower levels.
                    // At a future date, we should attempt to correct them for those levels by decreasing the amount at
                    // lower levels.
                    // The first attempt at doing so is below.
                    switch(Class)
                    {
                    case PRIEST:
                        {
                            BaseHP = uint32(87 * Level);
                            BaseMana = uint32(48.2875f * Level);
                        }break;
                    case WARRIOR:
                        {
                            BaseHP = uint32(101.5125f * Level);
                            BaseMana = 0;
                        }break;
                    case DEATHKNIGHT:
                        {
                            BaseHP = uint32(101.5125f * Level);
                            BaseMana = 0;
                        }break;
                    case HUNTER:
                        {
                            BaseHP = uint32(91.55f * Level);
                            BaseMana = uint32(63.075f * Level);
                        }break;
                    case ROGUE:
                        {
                            BaseHP = uint32(95.05f * Level);
                            BaseMana = 0;
                        }break;
                    case SHAMAN:
                        {
                            BaseHP = uint32(81.0625f * Level);
                            BaseMana = uint32(54.95f * Level);
                        }break;
                    case DRUID:
                        {
                            BaseHP = uint32(92.7125f * Level);
                            BaseMana = uint32(43.7f * Level);
                        }break;
                    case PALADIN:
                        {
                            BaseHP = uint32(86.675f * Level);
                            BaseMana = uint32(54.925f * Level);
                        }break;
                    case MAGE:
                        {
                            BaseHP = uint32(87.0375f * Level);
                            BaseMana = uint32(40.85f * Level);
                        }break;
                    case WARLOCK:
                        {
                            BaseHP = uint32(89.55f * Level);
                            BaseMana = uint32(48.2f * Level);
                        }break;
                    }

                    float perlevmod = 4.0f + (Level / 16.0f);
                    BaseMana = uint32(BaseMana / (perlevmod - (Level / 10.0f)));
                    BaseHP = uint32(BaseHP / (perlevmod - (Level / 10.0f)));
                }
                else
                {
                    BaseHP = GetBaseHPForLevel(Level, Class);
                    BaseMana = GetBaseManaForLevel(Level, Class);
                }

                // Apply HP/Mana
                uint32 HP = BaseHP;
                if( lvl->Stat[STAT_STAMINA] > 20 )
                {
                    HP += 20;
                    HP += ( (lvl->Stat[STAT_STAMINA]-20) * 10);
                }
                else
                    HP += lvl->Stat[STAT_STAMINA];
                uint32 Mana = BaseMana;
                if(BaseMana)
                {
                    if( lvl->Stat[STAT_INTELLECT] > 20 )
                    {
                        Mana += 20;
                        Mana += ((lvl->Stat[STAT_INTELLECT]-20) * 10);
                    }
                    else
                        Mana += lvl->Stat[STAT_INTELLECT];
                }

                lvl->HP = HP;
                lvl->Mana = Mana;
                lvl->BaseHP = BaseHP;
                lvl->BaseMana = BaseMana;

                // Calculate next level XP
                uint32 nextLvlXP = 0;

                // This is a fixed table taken from 3.3.5 wow. This can't get more blizzlike with the "if" cases ;)
                if( ( Level ) < MAX_PREDEFINED_NEXTLEVELXP )
                    nextLvlXP = NextLevelXp[(Level)];
                else // Crow: (80-81: 1687591) (81-82: 1704273) (82-83: 1720846) (83-84: 1737310) (84-85: 1753665)
                    nextLvlXP = uint32(Level*(21149.3671f-((Level-(MAX_PREDEFINED_NEXTLEVELXP-1))*54.4791f)));

                lvl->XPToNextLevel = nextLvlXP;

                // Apply to map.
                lMap->insert( LevelMap::value_type( Level, lvl ) );
            }

            // Insert back into the main map.
            mLevelInfo.insert(make_pair(make_pair(Race, Class), lMap));
        }
    }
    sLog.Notice("ObjectMgr", "%u level up information generated.", mLevelInfo.size());
}

LevelInfo* ObjectMgr::GetLevelInfo(uint32 Race, uint32 Class, uint32 Level)
{
    // Let's check that our level is valid first.
    if( Level > MAXIMUM_ATTAINABLE_LEVEL ) // too far.
        Level = MAXIMUM_ATTAINABLE_LEVEL;

    // Iterate levelinfo map until we find the right class+race.
    LevelInfoMap::iterator itr = mLevelInfo.find(make_pair(Race, Class));
    if(itr != mLevelInfo.end())
    {
        // Pull the level information from the second map.
        LevelMap::iterator it2 = itr->second->find( Level );
        if(it2 != itr->second->end())
            return it2->second;
        return itr->second->at((uint)itr->second->size());
    }

    return NULL;
}

uint32 ObjectMgr::GetPetSpellCooldown(uint32 SpellId)
{
    SpellEntry* sp = dbcSpell.LookupEntry( SpellId );
    if(sp)
    {
        uint32 pscd = ( sp->CategoryRecoveryTime == 0 ? sp->RecoveryTime : sp->CategoryRecoveryTime) +  (sp->StartRecoveryCategory == 0 ? sp->StartRecoveryTime : sp->StartRecoveryCategory);
        return pscd > PET_SPELL_SPAM_COOLDOWN ? pscd : PET_SPELL_SPAM_COOLDOWN;
    }
    sLog.Error("ObjectMgr","GetPetSpellCooldown tried to add a non existing spell %d",SpellId);
    return 600000;//
}

void ObjectMgr::LoadSpellOverride()
{
    QueryResult *result = WorldDatabase.Query( "SELECT DISTINCT overrideId FROM spelloverride" );

    if(result != NULL)
    {
        SpellEntry * sp;
        uint32 spellid;
        std::list<SpellEntry*>* list;
        QueryResult *resultIn;

        do
        {
            Field *fields = result->Fetch();
            resultIn = WorldDatabase.Query("SELECT spellId FROM spelloverride WHERE overrideId = %u", fields[0].GetUInt32());
            list = new std::list<SpellEntry*>;
            if(resultIn)
            {
                do
                {
                    Field *fieldsIn = resultIn->Fetch();
                    spellid = fieldsIn[0].GetUInt32();
                    sp = dbcSpell.LookupEntry(spellid);
                    if(!spellid || !sp)
                        continue;

                    list->push_back(sp);

                }while(resultIn->NextRow());
                delete resultIn;
            }

            if(list->size() == 0)
                delete list;
            else
                mOverrideIdMap.insert( OverrideIdMap::value_type( fields[0].GetUInt32(), list ));

        } while( result->NextRow() );
        delete result;
    }

    sLog.Notice("ObjectMgr", "%u spell overrides loaded.", mOverrideIdMap.size());
}

void ObjectMgr::SetVendorList(uint32 Entry, std::map<uint32, CreatureItem>* list_)
{
    mVendors[Entry] = list_;
}

struct SpawnCoords
{
    uint32 spawnid;
    float x, y, z, o;
};

void ObjectMgr::LoadCreatureWaypoints()
{
    Field *fields;
    std::map<uint32, SpawnCoords*> SpawnCoordMap;
    QueryResult *result = WorldDatabase.Query("SELECT id, position_x, position_y, position_z, orientation FROM creature_spawns ORDER BY id");
    if(result == NULL)
        return;
    SpawnCoords* sp = NULL;
    do
    {
        fields = result->Fetch();
        sp = new SpawnCoords();
        sp->spawnid = fields[0].GetUInt32();
        sp->x = fields[1].GetFloat();
        sp->y = fields[2].GetFloat();
        sp->z = fields[3].GetFloat();
        sp->o = fields[4].GetFloat();
        SpawnCoordMap.insert(make_pair(sp->spawnid, sp));
        fields = NULL;
        sp = NULL;
    }while( result->NextRow() );
    delete result;
    result = NULL;


    uint16 count = 0;
    map<uint32, uint16> WaypointCount;
    HM_NAMESPACE::hash_map<uint32, WayPointMap*>::const_iterator i;
    uint32 lastspawnid = 0, spawnid = 0, waypointcounter = 0, start = getMSTime();
    result = WorldDatabase.Query("SELECT spawnid, COUNT(waypointid) FROM creature_waypoints GROUP BY spawnid ORDER BY spawnid, waypointid");
    if(result == NULL)
    {
        for(std::map<uint32, SpawnCoords*>::iterator itr = SpawnCoordMap.begin(); itr != SpawnCoordMap.end(); itr++)
            delete itr->second;
        SpawnCoordMap.clear();
        return;
    }

    do
    {
        fields = result->Fetch();
        WaypointCount.insert(make_pair(fields[0].GetUInt32(), fields[1].GetUInt16()));
        fields = NULL;
    }while( result->NextRow() );
    delete result;
    result = NULL;

    result = WorldDatabase.Query("SELECT * FROM creature_waypoints ORDER BY spawnid, waypointid");
    if(result == NULL)
    {
        for(std::map<uint32, SpawnCoords*>::iterator itr = SpawnCoordMap.begin(); itr != SpawnCoordMap.end(); itr++)
            delete itr->second;
        SpawnCoordMap.clear();
        return;
    }

    set<uint32> skip;
    WayPoint* wp = NULL;
    WayPointMap* m = NULL;
    do
    {
        fields = result->Fetch();

        wp = new WayPoint();
        spawnid = fields[0].GetUInt32();
        if(SpawnCoordMap.find(spawnid) == SpawnCoordMap.end())
            continue;

        if(skip.find(spawnid) != skip.end())
        {
            delete wp;
            wp = NULL;
            continue;
        }

        wp->id = fields[1].GetUInt32();
        if(wp->id < 0)
        {
            sLog.Error("ObjectMgr", "Waypoints cannot start below 0, waypoint skipped for %u\n", spawnid);
            continue;
        }

        wp->x = fields[2].GetFloat();
        wp->y = fields[3].GetFloat();
        wp->z = fields[4].GetFloat();
        wp->orientation = fields[5].GetFloat();
        wp->waittime = fields[6].GetUInt32();
        wp->flags = fields[7].GetUInt32();
        wp->forwardInfo = new ConditionalData(fields[8].GetBool(), fields[9].GetUInt32(), fields[12].GetUInt32(), fields[14].GetUInt32(), fields[16].GetUInt32(), fields[18].GetString());
        if(wp->forwardInfo->EmoteID == 0
            && wp->forwardInfo->SkinID == 0
            && wp->forwardInfo->StandState == 0
            && wp->forwardInfo->SpellToCast == 0
            && wp->forwardInfo->SayText.length() == 0)
        {
            delete wp->forwardInfo;
            wp->forwardInfo = NULL;
        }

        wp->backwardInfo = new ConditionalData(fields[10].GetBool(), fields[11].GetUInt32(), fields[13].GetUInt32(), fields[15].GetUInt32(), fields[17].GetUInt32(), fields[19].GetString());
        if(wp->backwardInfo->EmoteID == 0
            && wp->backwardInfo->SkinID == 0
            && wp->backwardInfo->StandState == 0
            && wp->backwardInfo->SpellToCast == 0
            && wp->backwardInfo->SayText.length() == 0)
        {
            delete wp->backwardInfo;
            wp->backwardInfo = NULL;
        }

        wp->count = WaypointCount.at(spawnid);

        i = m_waypoints.find(spawnid);
        if(i == m_waypoints.end())
        {
            m_waypoints[spawnid] = new WayPointMap();
            WayPoint* wp2 = new WayPoint();
            wp2->id = 0;
            wp2->x = SpawnCoordMap[spawnid]->x;
            wp2->y = SpawnCoordMap[spawnid]->y;
            wp2->z = SpawnCoordMap[spawnid]->z;
            wp2->orientation = SpawnCoordMap[spawnid]->o;
            wp2->waittime = 0;
            wp2->flags = wp->flags;
            wp2->count = WaypointCount.at(spawnid);
            m_waypoints[spawnid]->push_back(wp2);
            m_waypoints[spawnid]->push_back(wp);
        }
        else
            i->second->push_back(wp);

        wp = NULL;
        fields = NULL;
        waypointcounter++;
    }while( result->NextRow() );
    WaypointCount.clear();
    delete result;

    for(std::map<uint32, SpawnCoords*>::iterator itr = SpawnCoordMap.begin(); itr != SpawnCoordMap.end(); itr++)
        delete itr->second;
    SpawnCoordMap.clear();

    sLog.Notice("ObjectMgr", "%u waypoints cached in %ums.", waypointcounter, getMSTime()-start);
}

WayPointMap*ObjectMgr::GetWayPointMap(uint32 spawnid)
{
    HM_NAMESPACE::hash_map<uint32,WayPointMap*>::const_iterator i;
    i = m_waypoints.find(spawnid);
    if(i != m_waypoints.end())
        return i->second;
    else return NULL;
}

Pet* ObjectMgr::CreatePet()
{
    uint32 guid;
    m_petlock.Acquire();
    guid = ++m_hiPetGuid;
    m_petlock.Release();

    uint64 fullguid = ((uint64)HIGHGUID_TYPE_PET << 32) | ((uint64)guid << 24) | guid;
    Pet* pet(new Pet(fullguid));
    pet->Init();
    return pet;
}

Player* ObjectMgr::CreatePlayer()
{
    uint32 guid;
    m_playerguidlock.Acquire();
    guid = ++m_hiPlayerGuid;
    m_playerguidlock.Release();
    Player* p(new Player(guid));
    p->Init();
    return p;
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
    uint32 guid;
    m_corpseguidlock.Acquire();
    guid = ++m_hiCorpseGuid;
    m_corpseguidlock.Release();
    Corpse* pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE,guid);
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
    rv = (itr != m_corpses.end()) ? itr->second : NULLCORPSE;
    _corpseslock.Release();
    return rv;
}

Transporter* ObjectMgr::GetTransporter(uint32 guid)
{
    Transporter* rv;
    _TransportLock.Acquire();
    HM_NAMESPACE::hash_map<uint32, Transporter* >::const_iterator itr = mTransports.find(guid);
    rv = (itr != mTransports.end()) ? itr->second : NULLTRANSPORT;
    _TransportLock.Release();
    return rv;
}

void ObjectMgr::AddTransport(Transporter* pTransporter)
{
    _TransportLock.Acquire();
    mTransports[pTransporter->GetUIdFromGUID()]=pTransporter;
    _TransportLock.Release();
}

Transporter* ObjectMgr::GetTransporterByEntry(uint32 entry)
{
    Transporter* rv = NULLTRANSPORT;
    _TransportLock.Acquire();
    HM_NAMESPACE::hash_map<uint32, Transporter* >::iterator itr = mTransports.begin();
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

void ObjectMgr::LoadReputationModifierTable(const char * tablename, ReputationModMap * dmap)
{
    QueryResult * result = WorldDatabase.Query("SELECT * FROM %s", tablename);
    ReputationModMap::iterator itr;
    ReputationModifier * modifier;
    ReputationMod mod;

    if(result)
    {
        do
        {
            mod.faction[0] = result->Fetch()[1].GetUInt32();
            mod.faction[1] = result->Fetch()[2].GetUInt32();
            mod.value = result->Fetch()[3].GetInt32();
            mod.replimit = result->Fetch()[4].GetUInt32();

            itr = dmap->find(result->Fetch()[0].GetUInt32());
            if(itr == dmap->end())
            {
                modifier = new ReputationModifier;
                modifier->entry = result->Fetch()[0].GetUInt32();
                modifier->mods.push_back(mod);
                dmap->insert( ReputationModMap::value_type( result->Fetch()[0].GetUInt32(), modifier ) );
            }
            else
            {
                itr->second->mods.push_back(mod);
            }
        } while(result->NextRow());
        delete result;
    }
    sLog.Notice("ObjectMgr", "%u reputation modifiers on %s.", dmap->size(), tablename);
}

void ObjectMgr::LoadReputationModifiers()
{
    LoadReputationModifierTable("reputation_creature_onkill", &m_reputation_creature);
    LoadReputationModifierTable("reputation_faction_onkill", &m_reputation_faction);
    LoadInstanceReputationModifiers();
}

ReputationModifier * ObjectMgr::GetReputationModifier(uint32 entry_id, uint32 faction_id)
{
    // first, try fetching from the creature table (by faction is a fallback)
    ReputationModMap::iterator itr = m_reputation_creature.find(entry_id);
    if(itr != m_reputation_creature.end())
        return itr->second;

    // fetch from the faction table
    itr = m_reputation_faction.find(faction_id);
    if(itr != m_reputation_faction.end())
        return itr->second;

    // no data. fallback to default -5 value.
    return 0;
}

void ObjectMgr::LoadMonsterSay()
{
    QueryResult * result = WorldDatabase.Query("SELECT * FROM npc_monstersay");
    if(result == NULL)
        return;

    uint32 Entry, Event;
    Field * fields = result->Fetch();
    do
    {
        Entry = fields[0].GetUInt32();
        Event = fields[1].GetUInt32();

        if(Event >= NUM_MONSTER_SAY_EVENTS)
            continue;

        if( mMonsterSays[Event].find( Entry ) != mMonsterSays[Event].end() )
        {
            sLog.outError("Duplicate monstersay event %u for entry %u, skipping", Event, Entry  );
            continue;
        }

        NpcMonsterSay * ms = new NpcMonsterSay;
        ms->Chance = fields[2].GetFloat();
        ms->Language = fields[3].GetUInt32();
        ms->Type = fields[4].GetUInt32();
        ms->MonsterName = fields[5].GetString() ? strdup(fields[5].GetString()) : strdup("None");

        char * texts[5];
        char * text;
        uint32 textcount = 0;

        for(uint32 i = 0; i < 5; i++)
        {
            text = (char*)fields[6+i].GetString();
            if(!text) continue;
            if(strlen(fields[6+i].GetString()) < 5)
                continue;

            texts[textcount] = strdup(fields[6+i].GetString());

            // check for ;
            if(texts[textcount][strlen(texts[textcount])-1] == ';')
                texts[textcount][strlen(texts[textcount])-1] = 0;

            ++textcount;
        }

        if(!textcount)
        {
            free(((char*)ms->MonsterName));
            delete ms;
            continue;
        }

        ms->Texts = new const char*[textcount];
        memcpy(ms->Texts, texts, sizeof(char*) * textcount);
        ms->TextCount = textcount;

        mMonsterSays[Event].insert( make_pair( Entry, ms ) );

    } while(result->NextRow());
    sLog.Notice("ObjectMgr", "%u monster say events loaded.", result->GetRowCount());
    delete result;
}

void ObjectMgr::HandleMonsterSayEvent(Creature* pCreature, MONSTER_SAY_EVENTS Event)
{
    MonsterSayMap::iterator itr = mMonsterSays[Event].find(pCreature->GetEntry());
    if(itr == mMonsterSays[Event].end())
        return;

    NpcMonsterSay * ms = itr->second;
    if(Rand(ms->Chance))
    {
        // chance successful.
        int choice = (ms->TextCount == 1) ? 0 : RandomUInt(ms->TextCount - 1);
        const char * text = ms->Texts[choice];
        pCreature->SendChatMessage(ms->Type, ms->Language, text);
    }
}

bool ObjectMgr::HasMonsterSay(uint32 Entry, MONSTER_SAY_EVENTS Event)
{
    MonsterSayMap::iterator itr = mMonsterSays[Event].find(Entry);
    if(itr == mMonsterSays[Event].end())
        return false;

    return true;
}

void ObjectMgr::LoadInstanceReputationModifiers()
{
    QueryResult * result = WorldDatabase.Query("SELECT * FROM reputation_instance_onkill");
    if(result == NULL)
        return;

    do
    {
        Field * fields = result->Fetch();
        InstanceReputationMod mod;
        mod.mapid = fields[0].GetUInt32();
        mod.mob_rep_reward = fields[1].GetInt32();
        mod.mob_rep_reward_heroic = fields[2].GetInt32();
        mod.mob_rep_limit = fields[3].GetUInt32();
        mod.mob_rep_limit_heroic = fields[4].GetUInt32();
        mod.boss_rep_reward = fields[5].GetInt32();
        mod.boss_rep_reward_heroic = fields[6].GetInt32();
        mod.boss_rep_limit = fields[7].GetUInt32();
        mod.boss_rep_limit_heroic = fields[8].GetUInt32();
        mod.faction[0] = fields[9].GetUInt32();
        mod.faction[1] = fields[10].GetUInt32();
        HM_NAMESPACE::hash_map<uint32, InstanceReputationModifier*>::iterator itr = m_reputation_instance.find(mod.mapid);
        if(itr == m_reputation_instance.end())
        {
            InstanceReputationModifier * m = new InstanceReputationModifier;
            m->mapid = mod.mapid;
            m->mods.push_back(mod);
            m_reputation_instance.insert( make_pair( m->mapid, m ) );
        }
        else
            itr->second->mods.push_back(mod);

    } while(result->NextRow());
    sLog.Notice("ObjectMgr", "%u instance reputation modifiers loaded.", m_reputation_instance.size());
    delete result;
}

bool ObjectMgr::HandleInstanceReputationModifiers(Player* pPlayer, Unit* pVictim)
{
    uint32 team = pPlayer->GetTeam();
    MapEntry* map = dbcMap.LookupEntry(pPlayer->GetMapId());
    bool is_boss, is_heroic;
    if(pVictim->GetTypeId() != TYPEID_UNIT)
        return false;

    HM_NAMESPACE::hash_map<uint32, InstanceReputationModifier*>::iterator itr = m_reputation_instance.find(pVictim->GetMapId());
    if(itr == m_reputation_instance.end())
        return false;

    is_boss = (TO_CREATURE( pVictim )->proto && TO_CREATURE( pVictim )->proto->boss) ? true : false;

    if(map->IsRaid()) // We are good here I guess.
        is_heroic = (pPlayer->IsInWorld() && pPlayer->iRaidType >= MODE_10PLAYER_HEROIC && pPlayer->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL) ? true : false;
    else
        is_heroic = (pPlayer->IsInWorld() && pPlayer->iInstanceType == MODE_5PLAYER_HEROIC && pPlayer->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL) ? true : false;

    // Apply the bonuses as normal.
    int32 replimit;
    int32 value;

    for(vector<InstanceReputationMod>::iterator i = itr->second->mods.begin(); i !=  itr->second->mods.end(); i++)
    {
        if(!(*i).faction[team])
            continue;

        if(is_boss)
        {
            value = is_heroic ? i->boss_rep_reward_heroic : i->boss_rep_reward;
            replimit = is_heroic ? i->boss_rep_limit_heroic : i->boss_rep_limit;
        }
        else
        {
            value =  is_heroic ? i->mob_rep_reward_heroic : i->mob_rep_reward;
            replimit = is_heroic ? i->mob_rep_limit_heroic : i->mob_rep_limit;
        }

        if(!value || (replimit && pPlayer->GetStanding(i->faction[team]) >= replimit))
            continue;

        //value *= sWorld.getRate(RATE_KILLREPUTATION);
        value = float2int32(float(value) * sWorld.getRate(RATE_KILLREPUTATION));
        pPlayer->ModStanding(i->faction[team], value);
    }

    return true;
}

void ObjectMgr::LoadDisabledSpells()
{
    QueryResult * result = WorldDatabase.Query("SELECT * FROM spell_disable");
    if(result)
    {
        do
        {
            m_disabled_spells.insert( result->Fetch()[0].GetUInt32() );
        } while(result->NextRow());
        delete result;
    }

    sLog.Notice("ObjectMgr", "%u disabled spells.", m_disabled_spells.size());
}

void ObjectMgr::ReloadDisabledSpells()
{
    m_disabled_spells.clear();
    LoadDisabledSpells();
}

void ObjectMgr::HashWMOAreaTables()
{
    for(ConstructDBCStorageIterator(WMOAreaTableEntry) itr = dbcWMOAreaTable.begin(); itr != dbcWMOAreaTable.end(); ++itr)
    {
        std::pair<uint32, std::pair<uint32, uint32> > WMOID((*itr)->groupId, std::make_pair((*itr)->rootId, (*itr)->adtId));
        WMOAreaTables.insert(std::make_pair(WMOID, *itr));
    }
}

WMOAreaTableEntry* ObjectMgr::GetWMOAreaTable(int32 adtid, int32 rootid, int32 groupid)
{
    std::pair<uint32, std::pair<uint32, uint32> > WMOIDs(groupid, std::make_pair(rootid, adtid));
    if(WMOAreaTables.find(WMOIDs) != WMOAreaTables.end())
        return WMOAreaTables.at(WMOIDs);
    return NULL;
}

RecallLocation *ObjectMgr::GetRecallLocByName(std::string name)
{   // Should never happen
    if(name.length() == 0)
        return NULL;

    _recallLock.Acquire();
    RecallLocation *result = NULL;
    std::string low_name = HEARTHSTONE_TOLOWER_RETURN(name);
    for(RecallSet::iterator itr = m_recallLocations.begin(); itr != m_recallLocations.end(); itr++)
    {
        if((low_name.length() == (*itr)->lowercase_name.length()) &&
            !strcmp(low_name.c_str(), (*itr)->lowercase_name.c_str()))
        {
            _recallLock.Release();
            return *itr; // 100% direct match
        }

        if((*itr)->lowercase_name.find(low_name) != std::wstring::npos)
            result = *itr;
    }
    _recallLock.Release();
    return result;
}

bool ObjectMgr::AddRecallLocation(std::string name, uint32 mapId, float x, float y, float z, float o)
{
    _recallLock.Acquire();
    std::string low_name = HEARTHSTONE_TOLOWER_RETURN(name);
    for(RecallSet::iterator itr = m_recallLocations.begin(); itr != m_recallLocations.end(); itr++)
    {
        if((low_name.length() == (*itr)->lowercase_name.length()) &&
            !strcmp(low_name.c_str(), (*itr)->lowercase_name.c_str()))
        {
            _recallLock.Release();
            return false; // We can't add a location that has an existing name used
        }
    }

    RecallLocation *loc = new RecallLocation();
    loc->RealName = name;
    loc->lowercase_name = low_name;
    loc->mapId = mapId;
    loc->x = x;
    loc->y = y;
    loc->z = z;
    loc->orient = o;
    m_recallLocations.insert(loc);
    WorldDatabase.Execute("REPLACE INTO recall VALUES('%s', '%u', '%f', '%f', '%f', '%f')", WorldDatabase.EscapeString(loc->RealName).c_str(), loc->mapId, loc->x, loc->y, loc->z, loc->orient);
    _recallLock.Release();
    return true;
}

bool ObjectMgr::FillRecallNames(std::string match, std::set<RecallLocation*> &output)
{   // Should never happen
    if(match.length() == 0)
        return NULL;

    uint32 count = 0;
    _recallLock.Acquire();
    RecallLocation *result = NULL;
    std::string low_match = HEARTHSTONE_TOLOWER_RETURN(match);
    for(RecallSet::iterator itr = m_recallLocations.begin(); itr != m_recallLocations.end(); itr++)
        if((*itr)->lowercase_name.find(low_match) != std::wstring::npos)
            output.insert(*itr), count++;
    _recallLock.Release();
    return count > 0;
}

bool ObjectMgr::DeleteRecallLocation(std::string name)
{   // Should never happen
    if(!name.length())
        return false;

    _recallLock.Acquire();
    RecallLocation *result = NULL;
    std::string low_name = HEARTHSTONE_TOLOWER_RETURN(name);
    for(RecallSet::iterator itr = m_recallLocations.begin(); itr != m_recallLocations.end(); itr++)
    {
        if((low_name.length() == (*itr)->lowercase_name.length()) &&
            !strcmp(low_name.c_str(), (*itr)->lowercase_name.c_str()))
        {
            result = *itr;// 100% direct match
            break;
        }
    }
    if(result == NULL)
    {
        _recallLock.Release();
        return false;
    }

    m_recallLocations.erase(result);
    _recallLock.Release();
    WorldDatabase.Execute("DELETE FROM recall WHERE name = '%s'", result->RealName.c_str());
    delete result;
    return true;
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

void ObjectMgr::LoadArenaTeams()
{
    QueryResult * result = CharacterDatabase.Query("SELECT * FROM arenateams");
    if( result != NULL )
    {
        do
        {
            ArenaTeam * team = new ArenaTeam(result->Fetch());
            AddArenaTeam(team);
            if(team->m_id > m_hiArenaTeamId)
                m_hiArenaTeamId=team->m_id;

        } while(result->NextRow());
        delete result;
    }

    /* update the ranking */
    UpdateArenaTeamRankings();
}

ArenaTeam * ObjectMgr::GetArenaTeamById(uint32 id)
{
    HM_NAMESPACE::hash_map<uint32, ArenaTeam*>::iterator itr;
    m_arenaTeamLock.Acquire();
    itr = m_arenaTeams.find(id);
    m_arenaTeamLock.Release();
    return (itr == m_arenaTeams.end()) ? NULL : itr->second;
}

ArenaTeam * ObjectMgr::GetArenaTeamByName(string & name, uint32 Type)
{
    m_arenaTeamLock.Acquire();
    for(HM_NAMESPACE::hash_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeams.begin(); itr != m_arenaTeams.end(); itr++)
    {
        if(!strnicmp(itr->second->m_name.c_str(), name.c_str(), name.size()))
        {
            m_arenaTeamLock.Release();
            return itr->second;
        }
    }
    m_arenaTeamLock.Release();
    return NULL;
}

void ObjectMgr::RemoveArenaTeam(ArenaTeam * team)
{
    m_arenaTeamLock.Acquire();
    m_arenaTeams.erase(team->m_id);
    m_arenaTeamMap[team->m_type].erase(team->m_id);
    m_arenaTeamLock.Release();
}

void ObjectMgr::AddArenaTeam(ArenaTeam * team)
{
    m_arenaTeamLock.Acquire();
    m_arenaTeams[team->m_id] = team;
    m_arenaTeamMap[team->m_type].insert(make_pair(team->m_id,team));
    m_arenaTeamLock.Release();
}

class ArenaSorter
{
public:
    bool operator()(ArenaTeam* const & a,ArenaTeam* const & b)
    {
        return (a->m_stat_rating > b->m_stat_rating);
    }
        bool operator()(ArenaTeam*& a, ArenaTeam*& b)
        {
                return (a->m_stat_rating > b->m_stat_rating);
        }
};

void ObjectMgr::UpdateArenaTeamRankings()
{
    m_arenaTeamLock.Acquire();
    for(uint32 i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
    {
        vector<ArenaTeam*> ranking;

        for(HM_NAMESPACE::hash_map<uint32,ArenaTeam*>::iterator itr = m_arenaTeamMap[i].begin(); itr != m_arenaTeamMap[i].end(); itr++)
            ranking.push_back(itr->second);

        std::sort(ranking.begin(), ranking.end(), ArenaSorter());
        uint32 rank = 1;
        for(vector<ArenaTeam*>::iterator itr = ranking.begin(); itr != ranking.end(); itr++)
        {
            if((*itr)->m_stat_ranking != rank)
            {
                (*itr)->m_stat_ranking = rank;
                (*itr)->SaveToDB();
            }
            ++rank;
        }
    }
    m_arenaTeamLock.Release();
}

void ObjectMgr::UpdateArenaTeamWeekly()
{   // reset weekly matches count for all teams and all members
    m_arenaTeamLock.Acquire();
    for(uint32 i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
    {
        for(HM_NAMESPACE::hash_map<uint32,ArenaTeam*>::iterator itr = m_arenaTeamMap[i].begin(); itr != m_arenaTeamMap[i].end(); itr++)
        {
            ArenaTeam *team = itr->second;
            if(team)
            {
                team->m_stat_gamesplayedweek = 0;
                team->m_stat_gameswonweek = 0;
                for(uint32 j = 0; j < team->m_memberCount; ++j)
                {
                    team->m_members[j].Played_ThisWeek = 0;
                    team->m_members[j].Won_ThisWeek = 0;
                }
                team->SaveToDB();
            }
        }
    }
    m_arenaTeamLock.Release();
}

void ObjectMgr::ResetDailies()
{
    _playerslock.AcquireReadLock();
    PlayerStorageMap::iterator itr = _players.begin();
    for(; itr != _players.end(); itr++)
    {
        Player* pPlayer = itr->second;
        uint8 eflags = 0;
        if( pPlayer->IsInWorld() )
            eflags = EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT;
        sEventMgr.AddEvent(pPlayer, &Player::ResetDailyQuests, EVENT_PLAYER_UPDATE, 100, 0, eflags);
    }
    _playerslock.ReleaseReadLock();
}

void ObjectMgr::LoadPetLevelupSpellMap()
{
    CreatureFamilyEntry * creatureFamily;
    SpellEntry * sp;
    SkillLineSpell * sk;
    uint32 count = 0;

    for (uint32 i = 0; i < dbcCreatureFamily.GetNumRows(); i++)
    {
        //Valid hunter pet family?
        creatureFamily = dbcCreatureFamily.LookupEntry(i);
        if(!creatureFamily || creatureFamily->pettalenttype < 0)
            continue;

        for(uint32 j = 0; j < dbcSkillLineSpell.GetNumRows(); ++j)
        {
            //Valid skill line?
            sk = dbcSkillLineSpell.LookupEntry(j);
            if(!sk || sk->raceMask != 0 || sk->classMask != 0)
                continue;

            //Vaild pet-family spell?
            sp = dbcSpell.LookupEntry(sk->spell);
            if(!sp || sp->SpellFamilyName != SPELLFAMILY_HUNTER)
                continue;

            switch(sk->skilline)
            {
            case SKILL_PET_IMP:
                {
                    mPetLevelupSpellMap[FAMILY_FAKE_IMP][sp->spellLevel] = sk->spell;
                }break;
            case SKILL_PET_VOIDWALKER:
                {
                    mPetLevelupSpellMap[FAMILY_FAKE_VOIDWALKER][sp->spellLevel] = sk->spell;
                }break;
            case SKILL_PET_SUCCUBUS:
                {
                    mPetLevelupSpellMap[FAMILY_FAKE_SUCCUBUS][sp->spellLevel] = sk->spell;
                }break;
            case SKILL_PET_FELHUNTER:
                {
                    mPetLevelupSpellMap[FAMILY_FAKE_FELHUNTER][sp->spellLevel] = sk->spell;
                }break;
            case SKILL_PET_FELGUARD:
                {
                    mPetLevelupSpellMap[FAMILY_FAKE_FELGUARD][sp->spellLevel] = sk->spell;
                }break;
            case SPELL_HASH_GROWL:
                {
                    mPetLevelupSpellMap[creatureFamily->ID][sp->spellLevel] = sk->spell;
                }break;
            case SPELL_HASH_COWER:
                {
                    mPetLevelupSpellMap[creatureFamily->ID][sp->spellLevel] = sk->spell;
                }break;
            }

            switch(creatureFamily->ID)
            {
            case FAMILY_BAT:
                {
                    if(sk->skilline !=  SKILL_PET_BAT)
                        continue;
                }break;
            case FAMILY_BEAR:
                {
                    if(sk->skilline !=  SKILL_PET_BEAR)
                        continue;
                }break;
            case FAMILY_BIRD_OF_PREY:
                {
                    if(sk->skilline !=  SKILL_PET_OWL)
                        continue;
                }break;
            case FAMILY_BOAR:
                {
                    if(sk->skilline != SKILL_PET_BOAR)
                        continue;
                }break;
            case FAMILY_CARRION_BIRD:
                {
                    if(sk->skilline !=  SKILL_PET_CARRION_BIRD)
                        continue;
                }break;
            case FAMILY_CAT:
                {
                    if(sk->skilline !=  SKILL_PET_CAT)
                        continue;
                }break;
            case FAMILY_CHIMAERA:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_CHIMAERA)
                        continue;
                }break;
            case FAMILY_CORE_HOUND:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_CORE_HOUND)
                        continue;
                }break;
            case FAMILY_CRAB:
                {
                    if(sk->skilline !=  SKILL_PET_CRAB)
                        continue;
                }break;
            case FAMILY_CROCILISK:
                {
                    if(sk->skilline !=  SKILL_PET_CROCILISK)
                        continue;
                }break;
            case FAMILY_DEVILSAUR:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_DEVILSAUR)
                        continue;
                }break;
            case FAMILY_DRAGONHAWK:
                {
                    if(sk->skilline !=  SKILL_PET_DRAGONHAWK)
                        continue;
                }break;
            case FAMILY_GORILLA:
                {
                    if(sk->skilline !=  SKILL_PET_GORILLA)
                        continue;
                }break;
            case FAMILY_HYENA:
                {
                    if(sk->skilline !=  SKILL_PET_HYENA)
                        continue;
                }break;
            case FAMILY_MOTH:
                {
                    if(sk->skilline !=  SKILL_PET_MOTH)
                        continue;
                }break;
            case FAMILY_NETHER_RAY:
                {
                    if(sk->skilline !=  SKILL_PET_NETHER_RAY)
                        continue;
                }break;
            case FAMILY_RAPTOR:
                {
                    if(sk->skilline !=  SKILL_PET_RAPTOR)
                        continue;
                }break;
            case FAMILY_RAVAGER:
                {
                    if(sk->skilline !=  SKILL_PET_RAVAGER)
                        continue;
                }break;
            case FAMILY_RHINO:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_RHINO)
                        continue;
                }break;
            case FAMILY_SCORPID:
                {
                    if(sk->skilline !=  SKILL_PET_SCORPID)
                        continue;
                }break;
            case FAMILY_SERPENT:
                {
                    if(sk->skilline !=  SKILL_PET_SERPENT)
                        continue;
                }break;
            case FAMILY_SILITHID:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_SILITHID)
                        continue;
                }break;
            case FAMILY_SPIDER:
                {
                    if(sk->skilline !=  SKILL_PET_SPIDER)
                        continue;
                }break;
            case FAMILY_SPIRIT_BEAST:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_SPIRIT_BEAST)
                        continue;
                }break;
            case FAMILY_SPOREBAT:
                {
                    if(sk->skilline !=  SKILL_PET_SPOREBAT)
                        continue;
                }break;
            case FAMILY_TALLSTRIDER:
                {
                    if(sk->skilline !=  SKILL_PET_TALLSTRIDER)
                        continue;
                }break;
            case FAMILY_TURTLE:
                {
                    if(sk->skilline !=  SKILL_PET_TURTLE)
                        continue;
                }break;
            case FAMILY_WARP_STALKER:
                {
                    if(sk->skilline !=  SKILL_PET_WARP_STALKER)
                        continue;
                }break;
            case FAMILY_WASP:
                {
                    if(sk->skilline !=  SKILL_PET_WASP)
                        continue;
                }break;
            case FAMILY_WIND_SERPENT:
                {
                    if(sk->skilline !=  SKILL_PET_WIND_SERPENT)
                        continue;
                }break;
            case FAMILY_WOLF:
                {
                    if(sk->skilline !=  SKILL_PET_WOLF)
                        continue;
                }break;
            case FAMILY_WORM:
                {
                    if(sk->skilline !=  SKILL_PET_EXOTIC_WORM)
                        continue;
                }break;
            default:
                {
                    sLog.Error("ObjectMgr", "Unhandled creature family %u", creatureFamily->ID);
                }break;
            }
            mPetLevelupSpellMap[creatureFamily->ID][sp->spellLevel] = sk->spell;
            ++count;
        }
    }
    sLog.Notice("ObjectMgr", "%u Pet LevelUp Spells loaded.",   count);
}

PetLevelupSpellSet const* ObjectMgr::GetPetLevelupSpellList(uint32 petFamily)   const
{
    PetLevelupSpellMap::const_iterator itr = mPetLevelupSpellMap.find(petFamily);
    if(itr != mPetLevelupSpellMap.end())
        return &itr->second;
    return NULL;
}

QueryResult* ObjectMgr::SQLCheckExists(const char* tablename, const char* columnname, uint64 columnvalue)
{
    if(!tablename || !columnname || !columnvalue)
        return NULL;

    QueryResult* result = WorldDatabase.Query("SELECT %s FROM %s WHERE %s = '%u' LIMIT 1", columnname, tablename, columnname, columnvalue);
    return result;
}

/* Crow: This is for my server, but feel free to change it if you want.
This is an attempt to maintain a blizzlike health gain after the blizz cap. */
uint32 GetBaseHPForLevel(uint32 level, uint32 Class)
{
    uint32 coeff = 0;
    switch(Class)
    {
    case PRIEST:
        {
            coeff = 4.35f;
        }break;
    case WARRIOR:
    case DEATHKNIGHT:
        {
            coeff = 5.075625f;
        }break;
    case HUNTER:
        {
            coeff = 4.5775f;
        }break;
    case ROGUE:
        {
            coeff = 4.7525f;
        }break;
    case SHAMAN:
        {
            coeff = 4.053125f;
        }break;
    case DRUID:
        {
            coeff = 4.635625f;
        }break;
    case PALADIN:
        {
            coeff = 4.33375f;
        }break;
    case MAGE:
        {
            coeff = 4.351875f;
        }break;
    case WARLOCK:
        {
            coeff = 4.4775f;
        }break;
    }

    return ((200+((level-(MAX_PREDEFINED_NEXTLEVELXP))*coeff))*level)/2;
}

uint32 GetBaseManaForLevel(uint32 level, uint32 Class)
{
    uint32 coeff = 0;
    switch(Class)
    {
    case WARRIOR:
    case ROGUE:
    case DEATHKNIGHT:
        {
            return 0;
        }break;
    case PRIEST:
        {
            coeff = 2.4432f;
        }break;
    case HUNTER:
        {
            coeff = 3.15375f;
        }break;
    case SHAMAN:
        {
            coeff = 2.7475f;
        }break;
    case DRUID:
        {
            coeff = 2.185f;
        }break;
    case PALADIN:
        {
            coeff = 2.74625f;
        }break;
    case MAGE:
        {
            coeff = 2.0425f;
        }break;
    case WARLOCK:
        {
            coeff = 2.41f;
        }break;
    }

    return ((100+((level-(MAX_PREDEFINED_NEXTLEVELXP))*coeff))*level)/2;
}
