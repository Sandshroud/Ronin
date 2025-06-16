/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2014-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

//From Extractor
#include "mpqfile.h"
#include "adtfile.h"
#include "wdtfile.h"
#include "dbcfile.h"
#include "wmo.h"

#include "system.h"

Mutex fileLock;
time_t UNIXTIME;
tm g_localTime;

//------------------------------------------------------------------------------
// Defines

#define MPQ_BLOCK_SIZE 0x1000

//-----------------------------------------------------------------------------

HANDLE WorldMpq = NULL, LocaleMpq = NULL;

uint32 CONF_TargetBuild = 15595;              // 4.3.4.15595

uint32 max_thread_count = 8;

uint32 map_compression_type = MAP_COMPRESSED_CHUNKS;

// List MPQ for extract maps from
char const* CONF_mpq_list[]=
{
    "world.MPQ",
    "art.MPQ",
    "world2.MPQ",
    "expansion1.MPQ",
    "expansion2.MPQ",
    "expansion3.MPQ",
    NULL
};

uint32 const Builds[] = {13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480, 14545, 15005, 15050, 15211, 15354, 15595, 0};
#define LAST_DBC_IN_DATA_BUILD 13623    // after this build mpqs with dbc are back to locale folder
#define LOCALES_COUNT 12

char const* Locales[LOCALES_COUNT] =
{
    "enGB", "enUS",
    "deDE", "esES",
    "frFR", "koKR",
    "zhCN", "zhTW",
    "enCN", "enTW",
    "esMX", "ruRU"
};

TCHAR const* LocalesT[LOCALES_COUNT] =
{
    _T("enGB"), _T("enUS"),
    _T("deDE"), _T("esES"),
    _T("frFR"), _T("koKR"),
    _T("zhCN"), _T("zhTW"),
    _T("enCN"), _T("enTW"),
    _T("esMX"), _T("ruRU"),
};

typedef struct
{
    char name[64];
    unsigned int id;
}map_id;

map_id * map_ids;

uint16 *LiqType = 0;
uint8 *liqSound = 0;
int8 *liquidMaterial = 0;

uint32 map_count;
char output_path[128]=".";
char input_path[1024]=".";
bool preciseVectorData = false;

// Constants

const char* szRawMAPMagic = "HMAP434_2";
const char* szRawVMAPMagic = "VMAP050";
const char* szWorkDirWmo = "./Tiles/Buildings";
const char* szWorkDirDbc = "./DBC";
const char* szWorkDirTiles = "./Tiles";
const char* szWorkDirObject = "./Tiles/obj";
const char* szWorkDirFailed = "./Failed";

bool LoadLocaleMPQFile(int locale)
{
    TCHAR buff[512];
    memset(buff, 0, sizeof(buff));
    _stprintf(buff, _T("%s%s/locale-%s.MPQ"), input_path, LocalesT[locale], LocalesT[locale]);
    if (!SFileOpenArchive(buff, 0, MPQ_OPEN_READ_ONLY, &LocaleMpq))
    {
        if (GetLastError() != ERROR_PATH_NOT_FOUND)
        {
            _tprintf(_T("Loading %s locale MPQs\n"), LocalesT[locale]);
            _tprintf(_T("Cannot open archive %s\n"), buff);
        }
        return false;
    }

    _tprintf(_T("Loading %s locale MPQs\n"), LocalesT[locale]);
    char const* prefix = NULL;
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        memset(buff, 0, sizeof(buff));
        prefix = Locales[locale];
        _stprintf(buff, _T("%swow-update-%u.MPQ"), input_path, Builds[i]);

        if (!SFileOpenPatchArchive(LocaleMpq, buff, prefix, 0) && GetLastError() != ERROR_FILE_NOT_FOUND)
            _tprintf(_T("Cannot open patch archive %s\n"), buff);
    }

    printf("\n");
    return true;
}

bool LoadCommonMPQFiles(uint32 build)
{
    printf("Loading common MPQ files\n");

    TCHAR filename[512];
    _stprintf(filename, _T("%sworld.MPQ"), input_path);
    if (!SFileOpenArchive(filename, 0, MPQ_OPEN_READ_ONLY, &WorldMpq))
    {
        if (GetLastError() == ERROR_PATH_NOT_FOUND)
            _tprintf(_T("Not found %s\n"), filename);
        else _tprintf(_T("Cannot open archive %s\n"), filename);
        return false;
    }

    for(uint8 i = 1; CONF_mpq_list[i]; i++)
    {
        if (build < 15211 && !strcmp("world2.MPQ", CONF_mpq_list[i]))   // 4.3.2 and higher MPQ
            continue;

        _stprintf(filename, _T("%s%s"), input_path, CONF_mpq_list[i]);
        if (!SFileOpenPatchArchive(WorldMpq, filename, "", MPQ_OPEN_READ_ONLY))
        {
            if (GetLastError() == ERROR_PATH_NOT_FOUND)
                _tprintf(_T("Not found %s\n"), filename);
            else _tprintf(_T("Cannot open archive %s\n"), filename);
            return false;
        }
        _tprintf(_T("Patched world with %s\n"), filename);
    }

    char const* prefix = NULL;
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        memset(filename, 0, sizeof(filename));
        prefix = Builds[i] > LAST_DBC_IN_DATA_BUILD ? "" : "base";
        if (prefix == "base")
            _stprintf(filename, _T("%swow-update-%u.MPQ"), input_path, Builds[i]);
        else _stprintf(filename, _T("%swow-update-base-%u.MPQ"), input_path, Builds[i]);
        if(!SFileOpenPatchArchive(WorldMpq, filename, prefix, MPQ_OPEN_READ_ONLY))
            continue;
        _tprintf(_T("Loaded %s\n"), filename);
    }

    printf("\n");
    return true;
}

bool FileExists(const char* file)
{
    FILE* n = NULL;
    int res = fopen_s(&n, file, "rb");
    switch(res)
    {
    case 0:break;
    case ENOENT:return false;
    default:
        printf("Error opening file %s\n", file);
        return false;
    }
    fclose(n);
    return true;
}

bool WMOExists(const char* file)
{
    FILE* n = NULL;
    Guard fileGuard(fileLock);
    int res = fopen_s(&n, file, "rb");
    switch(res)
    {
    case 0:break;
    case ENOENT:return false;
    default:
        printf("Error opening file %s\n", file);
        return false;
    }
    fclose(n);
    return true;
}

void strToLower(char* str)
{
    while(*str)
    {
        *str=tolower(*str);
        ++str;
    }
}

void replace(std::string &str, const char* find, const char* rep, uint32 limit)
{
    uint32 i=0;
    std::string::size_type pos=0;
    while((pos = str.find(find, pos)) != std::string::npos)
    {
        str.erase(pos, strlen(find));
        str.insert(pos, rep);
        pos += strlen(rep);

        ++i;
        if (limit != 0 && i == limit)
            break;
    }
}

bool CreateCustomDBCFiles();

bool ExtractDBCFiles()
{
    bool success = true;

    char fileType[4], searchType[10], buff[250], fileBuff[4000], *fileTypes[2] = { "dbc", "db2" };
    size_t foundCount = 0;
    SFILE_FIND_DATA data;
    for(int i = 0; i < 2; i++)
    {
        sprintf(fileType, "%s", fileTypes[i]);
        sprintf(searchType, "*.%s", fileType);

        printf("\nExtracting %s files...\n", fileType);
        if (HANDLE find = SFileFindFirstFile(LocaleMpq, searchType, &data, NULL))
        {
            do
            {
                HANDLE DBCFile;
                std::string str = data.cFileName, str2 = data.szPlainName;
                if (!SFileOpenFileEx(LocaleMpq, str.c_str(), SFILE_OPEN_FROM_MPQ, &DBCFile))
                {
                    printf("Error opening %s file %s\n", fileType, str.c_str());
                    success = false;
                    break;
                }

                // Skip files reporting 0 file size
                if(SFileGetFileSize(DBCFile, NULL) == 0)
                    continue;
                SFileCloseFile(DBCFile);

                sprintf(buff, "%s\\%s", szWorkDirDbc, str2.c_str());
                if(!SFileExtractFile(LocaleMpq, str.c_str(), buff, SFILE_OPEN_FROM_MPQ))
                {
                    printf("Error writing %s file %s\n", fileType, str.c_str());
                    success = false;
                    break;
                } else foundCount++;
            } while (success && SFileFindNextFile(find, &data));
            SFileFindClose(find);
        }
        if(success == false)
            break;
        printf("Found %zu %s files\n", foundCount, fileType);
        foundCount = 0;
    }

    return success && CreateCustomDBCFiles();
}

// copied from contrib/extractor/System.cpp
bool ReadLiquidDataDBC()
{
    printf("Read LiquidType.dbc file...");

    DBCFile dbc(LocaleMpq, "DBFilesClient\\LiquidType.dbc");
    if(!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        return false;
    }

    size_t LiqType_count = dbc.getRecordCount();
    size_t LiqType_maxid = dbc.getRecord(LiqType_count - 1).getUInt(0);
    LiqType = new uint16[LiqType_maxid + 1];
    memset(LiqType, 0xff, (LiqType_maxid + 1) * sizeof(uint16));
    liqSound = new uint8[LiqType_maxid + 1];
    memset(liqSound, 0xff, (LiqType_maxid + 1) * sizeof(uint8));

    for(uint32 x = 0; x < LiqType_count; ++x)
    {
        LiqType[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);
        liqSound[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(14);
    }

    printf("Done! (%u LiqTypes loaded)\n\n", (unsigned int)LiqType_count);

    DBCFile lmdbc(LocaleMpq, "DBFilesClient\\LiquidMaterial.dbc");
    if(!lmdbc.open())
    {
        printf("Fatal error: Invalid LiquidMaterial.dbc file format!\n");
        return false;
    }

    size_t liqMatCount = lmdbc.getRecordCount();
    size_t liqMat_maxid = lmdbc.getRecord(liqMatCount - 1).getUInt(0);
    liquidMaterial = new int8[liqMat_maxid + 1];
    memset(liquidMaterial, 0x7f, (liqMat_maxid + 1) * sizeof(int8));

    for(uint32 x = 0; x < liqMatCount; ++x)
    {
        int8 materialType = lmdbc.getRecord(x).getUInt(1);
        liquidMaterial[lmdbc.getRecord(x).getUInt(0)] = materialType;
    }

    printf("Done! (%u LiqMaterials loaded)\n\n", (unsigned int)liqMatCount);
    return true;
}

bool skipModel(std::string fullName)
{
    // Generic usually points to PVP objects or more doodads
    if(fullName.find("\\generic\\") != std::string::npos)
        return false;
    if(fullName.find("\\Generic\\") != std::string::npos)
        return false;
    if(fullName.find("\\GENERIC\\") != std::string::npos)
        return false;
    // Extract goober entities
    if(fullName.find("\\goober\\") != std::string::npos)
        return false;
    if(fullName.find("\\Goober\\") != std::string::npos)
        return false;
    if(fullName.find("\\GOOBER\\") != std::string::npos)
        return false;
    // Doodads need to be extracted
    if(fullName.find("doodad") != std::string::npos)
        return false;
    if(fullName.find("Doodad") != std::string::npos)
        return false;
    if(fullName.find("DOODAD") != std::string::npos)
        return false;
    return true;
}

bool ExtractBuildings()
{
    SFILE_FIND_DATA data;
    if (HANDLE find = SFileFindFirstFile(WorldMpq, "*.wmo,*.m2", &data, NULL))
    {
        int success = 1;
        printf("Processing and extracting buildings...\r");
        do
        {
            std::string str = data.cFileName, str2 = data.szPlainName;
            if(str2.find(".m2") != std::string::npos || str2.find(".M2") != std::string::npos)
            {
                if(skipModel(str) == false) // Only extract doodads
                    ExtractSingleModel(str);
            } else success = ExtractSingleWmo(WorldMpq, str);

            if(success && success != 2)
            {
                str2.resize(50, ' ');
                printf("Finished Processing %s\r", str2.c_str());
            }
        } while (success && SFileFindNextFile(find, &data));
        SFileFindClose(find);

        if(success)
        {
            printf("Extract buildings complete (No fatal errors)\r\n\n");
            return true;
        }
    }
    return false;
}

int ExtractSingleWmo(HANDLE mpqArchive, std::string& fname)
{
    // Copy files from archive
    char szLocalFile[1024];
    std::string plain_name = GetPlainName(fname.c_str());
    sprintf(szLocalFile, "%s\\%s", szWorkDirWmo, plain_name.c_str());
    FixNameCase(szLocalFile,strlen(szLocalFile));
    if (WMOExists(szLocalFile))
        return 2;

    std::string name_tail; // Cut off non root wmo(_xxx.wmo are subsections)
    if(plain_name.length() > 8 && (name_tail = plain_name.substr(plain_name.size()-8)).length() && name_tail[0] == '_' && isdigit(name_tail[1]) && isdigit(name_tail[2]) && isdigit(name_tail[3]))
        return 2;

    bool file_ok = true;
    WMORoot froot(fname);
    if(!froot.open(mpqArchive))
    {
        printf("Couldn't open RootWmo!!!\n");
        return 1;
    }

    Guard fileGuard(fileLock);
    FILE *output = fopen(szLocalFile,"wb");
    if(!output)
    {
        printf("couldn't open %s for writing!\n", szLocalFile);
        return 0;
    }

    froot.ConvertToVMAPRootWmo(output);
    int Wmo_nVertices = 0;
    //printf("root has %d groups\n", froot->nGroups);
    if (froot.nGroups !=0)
    {
        for (uint32 i = 0; i < froot.nGroups; ++i)
        {
            char temp[1024];
            strcpy(temp, fname.c_str());
            temp[fname.length()-4] = 0;
            char groupFileName[1024];
            sprintf(groupFileName, "%s_%03u.wmo", temp, i);
            //printf("Trying to open groupfile %s\n",groupFileName);

            std::string s = groupFileName;
            WMOGroup fgroup(s);
            if(!fgroup.open(mpqArchive))
            {
                printf("Could not open all Group file for: %s\n", plain_name.c_str());
                file_ok = false;
                break;
            }

            Wmo_nVertices += fgroup.ConvertToVMAPGroupWmo(output, &froot, preciseVectorData);
        }
    }

    fseek(output, 8, SEEK_SET); // store the correct no of vertices
    fwrite(&Wmo_nVertices,sizeof(int),1,output);
    fclose(output);

    // Delete the extracted file in the case of an error
    if (!file_ok)
        remove(szLocalFile);
    return 1;
}

void CleanupVMapData(bool &success);
void writeVMapData(FILE *mapFile, uint32 mapId, VMAP::ModelSpawnMap*, VMAP::TiledModelSpawnMap*);

class ThreadLoader
{
public:
    void ParseMapInfo(MapCreationInfo* mapInfo)
    {
        for (uint8 x = 0; x < 64; ++x)
        {
            for (uint8 y = 0; y < 64; ++y)
            {
                if(mapInfo->chnkFiles[x][y] = mapInfo->WDT->GetADTMap(WorldMpq, x, y))
                    mapInfo->chnkFiles[x][y]->parseCHNK(mapInfo, x, y);

                if(mapInfo->WDTFiles[x][y] = mapInfo->WDT->GetObjMap(WorldMpq, x, y))
                    mapInfo->WDTFiles[x][y]->parseWMO(mapInfo->mapId, mapInfo, x, y);
            }
        }

        FILE *mapFile = NULL;
        fopen_s(&mapFile, mapInfo->fileName, "w+b");
        if(mapFile == NULL)
        {
            delete mapInfo->WDT;
            delete mapInfo;
            return;
        }

        fwrite(szRawMAPMagic, 10, 1, mapFile);
        size_t offPos = ftell(mapFile);
        uint32 offsets[64][64], vmapOffs = 0;
        memset(offsets, 0, sizeof(uint32)*64*64);
        fwrite(offsets, sizeof(uint32)*64*64, 1, mapFile);
        fwrite(&vmapOffs, sizeof(uint32), 1, mapFile);
        fwrite(&map_compression_type, sizeof(uint32), 1, mapFile);

        for (uint8 x = 0; x < 64; ++x)
        {
            for (uint8 y = 0; y < 64; ++y)
            {
                if(mapInfo->chnkFiles[x][y])
                {
                    size_t pos = ftell(mapFile);
                    mapInfo->chnkFiles[x][y]->WriteCHNK(mapFile);
                    delete mapInfo->chnkFiles[x][y];
                    mapInfo->chnkFiles[x][y] = NULL;
                    offsets[x][y] = pos;
                }
            }
        }

        // End of height map data
        vmapOffs = ftell(mapFile);
        fseek(mapFile, offPos, SEEK_SET);
        fwrite(offsets, sizeof(uint32)*64*64, 1, mapFile);
        fwrite(&vmapOffs, sizeof(uint32), 1, mapFile);
        fseek(mapFile, vmapOffs, SEEK_SET);
        writeVMapData(mapFile, mapInfo->mapId, &mapInfo->modelSpawns, &mapInfo->tileModelSpawnSets);
        fclose(mapFile);

        for (uint8 x = 0, index = 0; x < 64; ++x)
            for (uint8 y = 0; y < 64; ++y)
                delete mapInfo->WDTFiles[x][y];
        delete mapInfo->WDT;
        delete mapInfo;
    }
};

//#define LIMIT_MAP_GENERATION 1

void ParsMapFiles()
{
    ThreadLoader *loader = new ThreadLoader();
    ThreadTaskList threadList(max_thread_count);
    uint32 index = 0, threadCount = threadList.spawn("MapManager", "Tasklist spawned", true);

    uint32 currentMapId;
#ifndef LIMIT_MAP_GENERATION
    std::vector<MapCreationInfo*> allocationSet;
#endif
    for (unsigned int i=0; i<map_count; ++i)
    {
        MapCreationInfo* mapInfo = new MapCreationInfo();

        mapInfo->WDT = NULL;
        mapInfo->mapId = map_ids[i].id;
        sprintf_s(mapInfo->id, 10,"%03u",map_ids[i].id);
        sprintf_s(mapInfo->fn, 512,"World\\Maps\\%s\\%s.wdt", map_ids[i].name, map_ids[i].name);
        sprintf_s(mapInfo->dirName, 255, "%s/dir_bin_%s", szWorkDirWmo, mapInfo->id);
        sprintf_s(mapInfo->fileName, 100, "%s\\%s.tiletree", szWorkDirTiles, mapInfo->id);
        if(FileExists(mapInfo->fileName))
        {
            delete mapInfo;
            continue;
        }

        for (uint8 x = 0, index = 0; x < 64; ++x)
        {
            for (uint8 y = 0; y < 64; ++y)
            {
                mapInfo->chnkFiles[x][y] = NULL;
                mapInfo->WDTFiles[x][y] = NULL;
            }
        }

        mapInfo->modelSpawns.clear();
        mapInfo->tileModelSpawnSets.clear();

        mapInfo->WDT = new WDTFile(WorldMpq, mapInfo, mapInfo->fn, map_ids[i].name);
        if(!mapInfo->WDT->init(mapInfo->id, map_ids[i].id))
        {
            delete mapInfo->WDT;
            delete mapInfo;
            continue;
        }

#ifdef LIMIT_MAP_GENERATION
        if(index++ == threadCount)
        {
            printf("%u Maps currently processing, waiting to rebuild list\r", index-1);
            threadList.wait(UNIXTIME, g_localTime);
            printf("%u Maps finished processing, rebuilding process list\n", index-1);
            index = 0;
        }

        printf("Adding Map %03u/%03u (%u)%s to processing list\n", i, map_count, map_ids[i].id, map_ids[i].name);
        threadList.AddTask(new CallbackP1<ThreadLoader, MapCreationInfo*>(loader, &ThreadLoader::ParseMapInfo, mapInfo));
#else
        printf("Adding Map %03u/%03u (%u)%s to processing list\n", i, map_count, map_ids[i].id, map_ids[i].name);
        allocationSet.push_back(mapInfo);
        index++;
#endif
    }

#ifndef LIMIT_MAP_GENERATION
    while(allocationSet.size())
    {
        MapCreationInfo* mapInfo = *allocationSet.begin();
        allocationSet.erase(allocationSet.begin());
        threadList.AddTask(new CallbackP1<ThreadLoader, MapCreationInfo*>(loader, &ThreadLoader::ParseMapInfo, mapInfo));
    }
#endif

    printf("%u Maps currently processing, waiting to write\r", index);
    threadList.wait(UNIXTIME, g_localTime);
    printf("%u Maps finished processing, writing to map file\n", index);
    threadList.kill();
    threadList.waitForThreadsToExit();

    delete loader;
}

void getGamePath()
{
#ifdef _WIN32
    strcpy(input_path,"Data\\");
#else
    strcpy(input_path,"Data/");
#endif
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Main
//
// The program must be run with two command line arguments
//
// Arg1 - The source MPQ name (for testing reading and file find)
// Arg2 - Listfile name
//

void OnCrash(bool Terminate)
{

}

int main(int argc, char ** argv)
{
    bool success = true, hasBuildings = false, hasDBCs = false;
    getGamePath();

    printf("Beginning work ....\n\n");
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // Create the working directory
    if(mkdir(szWorkDirTiles))
        success = (errno == EEXIST);
    if(success && mkdir(szWorkDirDbc))
        hasDBCs = success = (errno == EEXIST);
    if(success && mkdir(szWorkDirWmo))
        hasBuildings = success = (errno == EEXIST);
    if(success && mkdir(szWorkDirObject))
        success = (errno == EEXIST);

    if(!LoadCommonMPQFiles(CONF_TargetBuild))
    {
        getchar();
        return 0;
    }

    for (uint8 i = 0; i < LOCALES_COUNT; ++i)
    {
        //Open MPQs
        if (!LoadLocaleMPQFile(i))
        {
            if (GetLastError() != ERROR_PATH_NOT_FOUND)
                printf("Unable to load %s locale archives!\n", Locales[i]);
            continue;
        }

        printf("Detected and using locale: %s\n", Locales[i]);
        break;
    }

    if(success && !hasDBCs)
        if(!(success = ExtractDBCFiles()))
            rename(szWorkDirDbc, szWorkDirFailed);

    if(success && !(success = ReadLiquidDataDBC()))
    {
        printf("FAILED TO PARSE LIQUID DBCs");
        getchar();
        return 0;
    }

    // extract data
    if (success && !hasBuildings)
        if(!(success = ExtractBuildings()))
            rename(szWorkDirWmo, szWorkDirFailed);

    if(success)
    {
        //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
        //map.dbc
        DBCFile dbc(LocaleMpq, "DBFilesClient\\Map.dbc");
        if (success = dbc.open())
        {
            map_count = dbc.getRecordCount();
            map_ids = new map_id[map_count];
            for (unsigned int x = 0; x < map_count; ++x)
            {
                map_ids[x].id = dbc.getRecord(x).getUInt(0);
                strcpy(map_ids[x].name, dbc.getRecord(x).getString(1));
            }

            ParsMapFiles();
            delete [] map_ids;
            delete [] LiqType;

            // Extract models, listed in GameObjectDisplayInfo.dbc
            ExtractGameobjectModels();
        } else printf("FAILED TO OPEN MAP DBC");
    }

    SFileCloseArchive(WorldMpq);
    SFileCloseArchive(LocaleMpq);
    // Clean up vmap data
    CleanupVMapData(success);

    printf("\n");
    if (!success)
    {
        printf("ERROR: Work NOT complete.\n   Precise vector data=%d.\nPress enter to close.\n", preciseVectorData);
        getchar();
        return 0;
    }

    printf("Work complete. No errors. Press enter to close.\n");
    getchar();
    return 0;
}

bool CreateCustomDBCFiles()
{
    printf("\nCreating required custom dbc files...\n");

    std::map<uint32, DisplayBounding*> m_DisplayMap;
    std::map<std::string, ModelCache> modelCache;
    std::map<uint32, std::string> modelInfoEntries;

    //map.dbc
    DBCFile displayInfo(LocaleMpq, "DBFilesClient\\CreatureDisplayInfo.dbc"), modelInfo(LocaleMpq, "DBFilesClient\\CreatureModelData.dbc");
    if(!displayInfo.open() || !modelInfo.open())
    {
        printf("Failed to open display and model DBC files\n");
        return false;
    }

    std::string updateStr, filename = szWorkDirDbc + std::string("/CreatureBoundInformation.dbc");
    FILE* newDBC = NULL;
    fopen_s(&newDBC, filename.c_str(), "w+");
    if(newDBC != NULL)
    {
        char header[4];
        unsigned int na = 0, nb = 8, es = sizeof(DisplayBounding), ss = 0;
        header[0] = 'W'; header[1] = 'D';
        header[2] = 'B'; header[3] = 'C';
        fwrite(header, 4, 1, newDBC);

        for (DBCFile::Iterator itr = modelInfo.begin(); itr != modelInfo.end(); ++itr)
        {
            std::string modelName = itr->getString(2);
            if(modelName.empty() || modelName.size() < 4)
                continue;

            modelInfoEntries.insert(std::make_pair(itr->getInt(0), modelName));
        }

        for (DBCFile::Iterator itr = displayInfo.begin(); itr != displayInfo.end(); ++itr)
        {
            unsigned int displayid = itr->getInt(0);
            unsigned int modelentry = itr->getInt(1);
            float modelscale = itr->getFloat(4);

            std::map<uint32, std::string>::iterator  modelitr = modelInfoEntries.find(modelentry);
            if (modelitr == modelInfoEntries.end())
                continue;

            if(!(na%10))
            {
                updateStr.append(".");
                printf("Building%s %10s\r", updateStr.c_str(), "");
                if(!(na%100)) updateStr.clear();
            }

            DisplayBounding* BoundingInfo = new DisplayBounding();

            std::string strmodelname(modelitr->second);
            replace(strmodelname, ".mdx", ".m2", 0);
            replace(strmodelname, ".MDX", ".m2", 0);

            ModelHeader* header;
            M2Attachment* attachments;
            M2Bone* bones;
            uint16* bonelookups;

            std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelitr->second.c_str());
            if (cacheitr == modelCache.end())
            {
                MPQFile modelf(WorldMpq, strmodelname.c_str());
                if (modelf.isEof())
                {
                    delete BoundingInfo;
                    continue;
                }

                header = (ModelHeader*)malloc(sizeof(ModelHeader));
                modelf.read(header, sizeof(ModelHeader));

                attachments = (M2Attachment*)malloc(header->nAttachments * sizeof(M2Attachment));
                modelf.seek(header->ofsAttachments);
                modelf.read(attachments, header->nAttachments * sizeof(M2Attachment));

                bonelookups = (uint16*)malloc(header->nBoneLookupTable * sizeof(uint16));
                modelf.seek(header->ofsBoneLookupTable);
                modelf.read(bonelookups, header->nBoneLookupTable * sizeof(uint16));

                bones = (M2Bone*)malloc(header->nBones * sizeof(M2Bone));
                modelf.seek(header->ofsBones);
                modelf.read(bones, header->nBones * sizeof(M2Bone));

                ModelCache cacheentry;
                cacheentry.attachments = attachments;
                cacheentry.bones = bones;
                cacheentry.bonelookups = bonelookups;
                cacheentry.header = header;
                modelCache.insert(std::make_pair(modelitr->second.c_str(), cacheentry));
            }
            else
            {
                header = cacheitr->second.header;
                bones = cacheitr->second.bones;
                bonelookups = cacheitr->second.bonelookups;
                attachments = cacheitr->second.attachments;
            }

#ifdef GET_BONE_DATA
            // try and get the bone
            for (uint32 i = 0; i < header->nAttachments; ++i)
            {
                if (attachments[i].bone > header->nBoneLookupTable)
                {
#ifdef _DEBUG
                    printf("Attachment %u requests bonelookup %u (too large, bonelookup table is only %u entries)\n", i, attachments[i].bone, header->nBoneLookupTable);
#endif
                    continue;
                }

                uint16 boneindex = bonelookups[attachments[i].bone];
                if (boneindex > header->nBones)
                {
#ifdef _DEBUG
                    printf("Attachment %u requests bone %u (too large, bone table is only %u entries)\n", i, boneindex, header->nBones);
#endif
                    continue;
                }
                M2Bone & bone = bones[boneindex];
                //printf("Attachment %u (bone pivot %f %f %f offset %f %f %f)\n", attachments[i].id, bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2], attachments[i].pos[0],  attachments[i].pos[1],  attachments[i].pos[2]);

                float realpos[3];
                realpos[0] = (/*bone.pivotpoint[0] +*/ attachments[i].pos[0]) * modelscale;
                realpos[1] = (/*bone.pivotpoint[1] +*/ attachments[i].pos[1]) * modelscale;
                realpos[2] = (/*bone.pivotpoint[2] +*/ attachments[i].pos[2]) * modelscale;

                //fix coord system
//                  float tmp = realpos[2];
//                  realpos[2] = realpos[1];
//                  realpos[1] = -tmp;
                //fprintf(fo, "insert into `display_attachment_points` VALUES (%u, %u, %f, %f, %f);\n", displayid, attachments[i].id, attachments[i].pos[0], attachments[i].pos[1], attachments[i].pos[2]);
                //printf("Attachmnent %u point %f %f %f pivot %f %f %f\n", attachments[i].id, realpos[0], realpos[1], realpos[2], bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2]);
            }
#endif

            BoundingInfo->Entry = displayid;
            BoundingInfo->Low[0] = ((floor(1000000*(header->boundingbox1[0] * modelscale)))/1000000);
            BoundingInfo->Low[1] = ((floor(1000000*(header->boundingbox1[1] * modelscale)))/1000000);
            BoundingInfo->Low[2] = ((floor(1000000*(header->boundingbox1[2] * modelscale)))/1000000);
            BoundingInfo->High[0] = ((floor(1000000*(header->boundingbox2[0] * modelscale)))/1000000);
            BoundingInfo->High[1] = ((floor(1000000*(header->boundingbox2[1] * modelscale)))/1000000);
            BoundingInfo->High[2] = ((floor(1000000*(header->boundingbox2[2] * modelscale)))/1000000);
            BoundingInfo->BoundRadius = ((floor(1000000*(header->boundingradius * modelscale)))/1000000);
            m_DisplayMap.insert(std::make_pair(displayid, BoundingInfo));
            na++;
        }

        printf("%u Creature Bound Information entries created.\n", na);
        fwrite(&na, 4, 1, newDBC);
        fwrite(&nb, 4, 1, newDBC);
        fwrite(&es, 4, 1, newDBC);
        fwrite(&ss, 4, 1, newDBC);
        for(std::map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(); itr != m_DisplayMap.end(); itr++)
            fwrite(((uint8*)(itr->second)), es, 1, newDBC);

        fclose(newDBC);
        newDBC = NULL;
    }

    printf("Cleaning up bound data...\r");
    DisplayBounding* buff = NULL;
    for(std::map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(), itr2; itr != m_DisplayMap.end();)
    {
        itr2 = itr++;
        buff = itr2->second;
        m_DisplayMap.erase(itr2);
        delete buff;
        buff = NULL;
    }
    printf("Bound data cleaned.             \n\n");
    return true;
}
