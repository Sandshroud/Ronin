/***
 * Demonstrike Core
 */

// Class Map
// Holder for all instances of each mapmgr, handles transferring
// players between, and template holding.

#include "StdAfx.h"

Map::Map(MapEntry *map, char *name) : _mapId(map->MapID), mapName(name), _terrain(NULL), _entry(map)
{

}

Map::~Map()
{
    if(_terrain) delete _terrain, _terrain = NULL;

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
}

void Map::Initialize()
{
    //new stuff Load Spawns
    LoadSpawns();

    _terrain = new TerrainMgr(sWorld.MapPath, _mapId);

    // Initialize the terrain header
    _terrain->LoadTerrainHeader();

    // Load all the terrain from this map
    if(sWorld.ServerPreloading >= 1)
        _terrain->LoadAllTerrain();
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
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, format("Your creature_spawns table has the wrong column count(%u,%u).", fieldcount, 9).c_str(), "This table has skipped loading in order to avoid crashing.", "Please correct this, if you do not no spawns will show.", NULL);
        }

        return false;
    }
    return true;
}

static bool go_first_table_warning = true;
bool CheckResultLengthGameObject(QueryResult * res)
{
    uint32 fieldcount = res->GetFieldCount();
    if( fieldcount != 14 )
    {
        if( go_first_table_warning )
        {
            go_first_table_warning = false;
            sLog.LargeErrorMessage(LARGERRORMESSAGE_WARNING, format("Your gameobject_spawns table has the wrong column count(%u,%u).", fieldcount, 14).c_str(), "This table has skipped loading in order to avoid crashing.", "Please correct this, if you do not no spawns will show.", NULL);
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
            cspawn->eventId = 0;

            uint32 cellx = CellHandler<MapInstance>::GetPosX(cspawn->x), celly = CellHandler<MapInstance>::GetPosY(cspawn->y);
            GetSpawnsListAndCreate(cellx, celly)->CreatureSpawns.push_back(cspawn);
        }while(result->NextRow());
        delete result;
    }

    if(QueryResult *result = WorldDatabase.Query("SELECT id, entry, position_x, position_y, position_z, rotation0, rotation1, rotation2, rotation3, state, flags, faction, scale, eventId FROM gameobject_spawns WHERE map = %u", _mapId))
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
            gspawn->r0 = fields[5].GetFloat();
            gspawn->r1 = fields[6].GetFloat();
            gspawn->r2 = fields[7].GetFloat();
            gspawn->r3 = fields[8].GetFloat();
            gspawn->state = fields[9].GetUInt32();
            gspawn->flags = fields[10].GetUInt32();
            gspawn->faction = fields[11].GetUInt32();
            gspawn->scale = std::min<float>(255.f, fields[12].GetFloat());
            gspawn->eventId = fields[13].GetUInt32();

            uint32 cellx = CellHandler<MapInstance>::GetPosX(gspawn->x), celly = CellHandler<MapInstance>::GetPosY(gspawn->y);
            GetSpawnsListAndCreate(cellx, celly)->GOSpawns.push_back(gspawn);
        }while(result->NextRow());
        delete result;
    }
}

