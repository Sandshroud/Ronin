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
        mapName = _mapInfo->mapName;
    else mapName = "Unknown";
}

Map::~Map()
{
    sLog.Notice("Map", "~Map %u", _mapId);
    delete _terrain;

    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        for(CreatureSpawnList::iterator i = itr->second.CreatureSpawns.begin(), i2; i != itr->second.CreatureSpawns.end(); i++)
            delete (*i);
        itr->second.CreatureSpawns.clear();
        for(GOSpawnList::iterator i = itr->second.GOSpawns.begin(); i != itr->second.GOSpawns.end(); i++)
            delete (*i);
        itr->second.GOSpawns.clear();
    }
    m_spawns.clear();

    // collision
    if (Collision)
        sVMapInterface.DeactivateMap(_mapId);
}

static bool ctr_first_table_warning = true;
bool CheckResultLengthCreatures(QueryResult * res)
{
    uint32 fieldcount = res->GetFieldCount();
    if( fieldcount != 9 )
    {
        if( ctr_first_table_warning )
        {
            ctr_first_table_warning = false;
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, format("Your creature_spawns table has the wrong column count(%u,%u).", fieldcount, 22).c_str(), "This table has skipped loading in order to avoid crashing.", "Please correct this, if you do not no spawns will show.", NULL);
        }

        return false;
    }
    return true;
}

static bool go_first_table_warning = true;
bool CheckResultLengthGameObject(QueryResult * res)
{
    uint32 fieldcount = res->GetFieldCount();
    if( fieldcount != 11 )
    {
        if( go_first_table_warning )
        {
            go_first_table_warning = false;
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, format("Your gameobject_spawns table has the wrong column count(%u,%u).", fieldcount, 11).c_str(), "This table has skipped loading in order to avoid crashing.", "Please correct this, if you do not no spawns will show.", NULL);
        }

        return false;
    }
    return true;
}

void Map::LoadSpawns(bool reload /* = false */)
{
    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        for(CreatureSpawnList::iterator i = itr->second.CreatureSpawns.begin(), i2; i != itr->second.CreatureSpawns.end(); i++)
            delete (*i);
        itr->second.CreatureSpawns.clear();
        for(GOSpawnList::iterator i = itr->second.GOSpawns.begin(); i != itr->second.GOSpawns.end(); i++)
            delete (*i);
        itr->second.GOSpawns.clear();
    }
    m_spawns.clear();

    if(QueryResult *result = WorldDatabase.Query("SELECT id, entry, position_x, position_y, position_z, orientation, modelId, vendorMask FROM creature_spawns WHERE Map = %u", _mapId))
    {
        do
        {
            Field * fields = result->Fetch();
            CreatureSpawn *cspawn = new CreatureSpawn();
            cspawn->id = fields[0].GetUInt32();
            cspawn->entry = fields[1].GetUInt32();
            cspawn->x = fields[2].GetFloat();
            cspawn->y = fields[3].GetFloat();
            cspawn->z = fields[4].GetFloat();
            cspawn->o = NormAngle(fields[5].GetFloat());
            cspawn->modelId = fields[6].GetUInt32();
            cspawn->vendormask = fields[7].GetUInt32();

            uint32 cellx = CellHandler<MapMgr>::GetPosX(cspawn->x), celly = CellHandler<MapMgr>::GetPosY(cspawn->y);
            GetSpawnsListAndCreate(cellx, celly)->CreatureSpawns.push_back(cspawn);
        }while(result->NextRow());
        delete result;
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT id, entry, position_x, position_y, position_z, orientation, state, flags, faction, scale FROM gameobject_spawns WHERE map = %u", _mapId))
    {
        do
        {
            Field * fields = result->Fetch();
            GOSpawn *gspawn = new GOSpawn();
            gspawn->id = fields[0].GetUInt32();
            gspawn->entry = fields[1].GetUInt32();
            gspawn->x = fields[2].GetFloat();
            gspawn->y = fields[3].GetFloat();
            gspawn->z = fields[4].GetFloat();
            gspawn->o = NormAngle(fields[5].GetFloat());
            gspawn->state = fields[6].GetUInt32();
            gspawn->flags = fields[7].GetUInt32();
            gspawn->faction = fields[8].GetUInt32();
            gspawn->scale = std::min<float>(255.f, fields[9].GetFloat());

            uint32 cellx = CellHandler<MapMgr>::GetPosX(gspawn->x), celly = CellHandler<MapMgr>::GetPosY(gspawn->y);
            GetSpawnsListAndCreate(cellx, celly)->GOSpawns.push_back(gspawn);
        }while(result->NextRow());
        delete result;
    }
}

