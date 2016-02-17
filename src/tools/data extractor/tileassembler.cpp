
#include <string>
#include <iostream>

#include <g3dlite/G3D.h>
#include <vmaplib/VMapLib.h>

extern const char* szWorkDirWmo;
extern const char* szWorkDirTiles;
extern const char* szWorkDirObject;

//=======================================================

static VMAP::TileAssembler *ta = NULL;

void writeVMapData(FILE *mapFile, uint32 mapId, VMAP::ModelSpawnMap* spawnMap, VMAP::TiledModelSpawnMap* tileSpawnMap)
{
    if(ta == NULL)
        ta = new VMAP::TileAssembler(szWorkDirWmo, szWorkDirTiles, szWorkDirObject);

    if(!ta->convertWorld3(mapFile, mapId, spawnMap, tileSpawnMap))
        printf("Failed converting vmap data for %u\n", mapId);
}

void CleanupVMapData(bool &success)
{
    if(ta == NULL)
        return;

    if(success && !ta->convertWorldObjects())
        success = false;
    delete ta;
}