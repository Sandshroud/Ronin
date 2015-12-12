/***
 * Demonstrike Core
 */

// Class Map
// Holder for all instances of each mapmgr, handles transferring
// players between, and template holding.

#include "StdAfx.h"

Map::Map(uint32 mapid, MapInfo * inf)
{
    _mapInfo = inf;
    _mapId = mapid;
    // Collision = inf->Collision;
    Collision = sWorld.Collision;

    //new stuff Load Spawns
    LoadSpawns();

    bool mapCollision = false;
    // collision
    if (Collision)
        mapCollision = sVMapInterface.ActivateMap(_mapId);

    // Setup terrain
    _terrain = new TerrainMgr(sWorld.MapPath, _mapId, !(inf->type == INSTANCE_NULL), mapCollision);
    _terrain->LoadTerrainHeader();

    // get our name
    me = dbcMap.LookupEntry(_mapId);
    if(_mapInfo)
        name = _mapInfo->name;
    else
        name = "Unknown";
}

Map::~Map()
{
    sLog.Notice("Map", "~Map %u", _mapId);
    delete _terrain;

    CellSpawns *sp = NULL;
    CreatureSpawn *cs = NULL;
    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        if(itr->second != NULL)
        {
            for(CellSpawnsMap::iterator itr2 = itr->second->begin(); itr2 != itr->second->end(); itr2++)
            {
                sp = itr2->second;
                itr2->second = NULL;
                for(CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(), i2; i != sp->CreatureSpawns.end();)
                {
                    cs = (*i);
                    i++;

                    if(cs->ChannelData != NULL)
                    {
                        delete cs->ChannelData;
                        cs->ChannelData = NULL;
                    }

                    delete cs;
                    cs = NULL;
                }

                for(GOSpawnList::iterator it = sp->GOSpawns.begin(); it!=sp->GOSpawns.end(); it++)
                    delete (*it);

                delete sp;
                sp = NULL;
            }

            itr->second->clear();
            delete itr->second;
            itr->second = NULL;
        }
    }
    m_spawns.clear();

    // collision
    if (Collision)
        sVMapInterface.DeactivateMap(_mapId);
}

static bool first_table_warning = true;
bool CheckResultLengthCreatures(QueryResult * res)
{
    uint32 fieldcount = res->GetFieldCount();
    if( fieldcount != 19 )
    {
        if( first_table_warning )
        {
            first_table_warning = false;
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, format("Your creature_spawns table has the wrong column count(%u,%u).", fieldcount, 22).c_str(),
                "This table has skipped loading in order to avoid crashing.", "Please correct this, if you do not no spawns will show.", NULL);
        }

        return false;
    }
    return true;
}

static bool first_table_warningg = true;
bool CheckResultLengthGameObject(QueryResult * res)
{
    uint32 fieldcount = res->GetFieldCount();
    if( fieldcount != 11 )
    {
        if( first_table_warningg )
        {
            first_table_warningg = false;
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, format("Your gameobject_spawns table has the wrong column count(%u,%u).", fieldcount, 11).c_str(),
                "This table has skipped loading in order to avoid crashing.", "Please correct this, if you do not no spawns will show.", NULL);
        }

        return false;
    }
    return true;
}

void Map::LoadSpawns(bool reload /* = false */)
{
    CreatureSpawnCount = 0;
    QueryResult* result;
    std::set<std::string>::iterator tableiterator;
    if(reload)//perform cleanup
    {
        CellSpawns *sp = NULL;
        CreatureSpawn *cs = NULL;
        for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
        {
            if(itr->second != NULL)
            {
                for(CellSpawnsMap::iterator itr2 = itr->second->begin(); itr2 != itr->second->end(); itr2++)
                {
                    sp = itr2->second;
                    itr2->second = NULL;
                    for(CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(); i != sp->CreatureSpawns.end();)
                    {
                        cs = (*i);
                        i++;

                        if(cs->ChannelData != NULL)
                        {
                            delete cs->ChannelData;
                            cs->ChannelData = NULL;
                        }

                        delete cs;
                    }

                    for(GOSpawnList::iterator it = sp->GOSpawns.begin(); it!=sp->GOSpawns.end(); it++)
                        delete (*it);

                    delete sp;
                }

                itr->second->clear();
                delete itr->second;
                itr->second = NULL;
            }
        }
        m_spawns.clear();
    }

    for(tableiterator = ExtraMapCreatureTables.begin(); tableiterator != ExtraMapCreatureTables.end(); ++tableiterator)
    {
        if(result = WorldDatabase.Query("SELECT * FROM %s WHERE Map = %u", (*tableiterator).c_str(), _mapId))
        {
            if(CheckResultLengthCreatures( result) )
            {
                do
                {
                    Field * fields = result->Fetch();
                    CreatureSpawn *cspawn = new CreatureSpawn();
                    cspawn->id = fields[0].GetUInt32();
                    cspawn->entry = fields[1].GetUInt32();
                    cspawn->x = fields[3].GetFloat();
                    cspawn->y = fields[4].GetFloat();
                    cspawn->z = fields[5].GetFloat();
                    cspawn->o = NormAngle(fields[6].GetFloat());
                    cspawn->flags = fields[8].GetUInt32();
                    cspawn->factionid = fields[7].GetUInt32();
                    cspawn->emote_state = fields[9].GetUInt32();
                    cspawn->death_state = fields[10].GetUInt16();
                    cspawn->stand_state = fields[11].GetUInt8();
                    cspawn->MountedDisplayID = fields[15].GetUInt32();
                    cspawn->CanMove = fields[17].GetUInt32();
                    cspawn->vendormask = fields[18].GetUInt32();
                    cspawn->modelId = 0;
                    cspawn->modelGender = 0;

                    cspawn->ChannelData = new SpawnChannelData(fields[12].GetUInt32(), fields[13].GetUInt32(), fields[14].GetUInt32());
                    if(cspawn->ChannelData->channel_spell == 0 &&
                        cspawn->ChannelData->channel_target_go == 0 &&
                        cspawn->ChannelData->channel_target_creature == 0)
                    {
                        delete cspawn->ChannelData;
                        cspawn->ChannelData = NULL;
                    }

                    uint32 cellx = CellHandler<MapMgr>::GetPosX(cspawn->x);
                    uint32 celly = CellHandler<MapMgr>::GetPosY(cspawn->y);
                    GetSpawnsListAndCreate(cellx, celly)->CreatureSpawns.push_back(cspawn);
                    ++CreatureSpawnCount;
                }while(result->NextRow());
            }

            delete result;
        }
    }

    for(tableiterator = ExtraMapGameObjectTables.begin(); tableiterator != ExtraMapGameObjectTables.end(); ++tableiterator)
    {
        result = WorldDatabase.Query("SELECT * FROM %s WHERE map = %u", (*tableiterator).c_str(), _mapId);
        if(result)
        {
            if( CheckResultLengthGameObject(result) )
            {
                do
                {
                    Field * fields = result->Fetch();
                    GOSpawn *gspawn = new GOSpawn();
                    gspawn->id = fields[0].GetUInt32();
                    gspawn->entry = fields[1].GetUInt32();
                    gspawn->x = fields[3].GetFloat();
                    gspawn->y = fields[4].GetFloat();
                    gspawn->z = fields[5].GetFloat();
                    gspawn->o = NormAngle(fields[6].GetFloat());
                    gspawn->state = fields[7].GetUInt32();
                    gspawn->flags = fields[8].GetUInt32();
                    gspawn->faction = fields[9].GetUInt32();
                    gspawn->scale = std::min<float>(255.f, fields[10].GetFloat());
                    uint32 cellx = CellHandler<MapMgr>::GetPosX(gspawn->x);
                    uint32 celly = CellHandler<MapMgr>::GetPosY(gspawn->y);
                    GetSpawnsListAndCreate(cellx, celly)->GOSpawns.push_back(gspawn);
                }while(result->NextRow());
            }
            delete result;
        }
    }
}

