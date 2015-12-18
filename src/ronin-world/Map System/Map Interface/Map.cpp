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
                for(CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(), i2; i != sp->CreatureSpawns.end(); i++)
                    delete (*i);

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
    if( fieldcount != 9 )
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
    for(SpawnsMap::iterator itr = m_spawns.begin(); itr != m_spawns.end(); itr++)
    {
        CellSpawnsMap *spm = itr->second;
        itr->second = NULL;
        if(spm)
        {
            for(CellSpawnsMap::iterator itr2 = spm->begin(); itr2 != spm->end(); itr2++)
            {
                CellSpawns *sp = itr2->second;
                itr2->second = NULL;
                if(sp)
                {
                    for(CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(); i != sp->CreatureSpawns.end(); i++)
                        delete (*i);

                    for(GOSpawnList::iterator it = sp->GOSpawns.begin(); it!=sp->GOSpawns.end(); it++)
                        delete (*it);

                    delete sp;
                }
            }

            spm->clear();
            delete spm;
        }
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

