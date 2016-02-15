/***
 * Demonstrike Core
 */

#include <g3dlite\G3D.h>
#include "VMapLib.h"
#include "VMapDefinitions.h"

using G3D::Vector3;
using G3D::AABox;
using G3D::inf;
using std::pair;

#include <set>
#include <iomanip>

template<> struct BoundsTrait<VMAP::ModelSpawn*>
{
    static void getBounds(const VMAP::ModelSpawn* const &obj, G3D::AABox& out) { out = obj->getBounds(); }
};

namespace VMAP
{
    bool readChunk(FILE *rf, char *dest, const char *compare, G3D::uint32 len)
    {
        if (fread(dest, sizeof(char), len, rf) != len) return false;
        return memcmp(dest, compare, len) == 0;
    }

    Vector3 ModelPosition::transform(const Vector3& pIn) const
    {
        Vector3 out = pIn * iScale;
        out = iRotation * out;
        return(out);
    }

    //=================================================================

    TileAssembler::TileAssembler(const std::string& pSrcDirName, const std::string& pTileDestDirName, const std::string &pObjectDestDirName)
        : iTileDestDir(pTileDestDirName), iObjDestDir(pObjectDestDirName), iSrcDir(pSrcDirName), iFilterMethod(NULL), iCurrentUniqueNameId(0)
    {
        //init();
    }

    TileAssembler::~TileAssembler()
    {
        //delete iCoordModelMapping;
    }

    bool TileAssembler::convertWorld2()
    {
        printf("\n");
        bool success = readMapSpawns();
        if (!success)
            return false;

        char output_filename[1024]; // export Map data
        for (MapData::iterator map_iter = mapData.begin(); map_iter != mapData.end() && success; ++map_iter)
        {
            // build global map tree
            std::vector<ModelSpawn*> mapSpawns;
            UniqueEntryMap::iterator entry;
            printf("Calculating model bounds for map %03u...                   \r", map_iter->first);
            for (entry = map_iter->second->UniqueEntries.begin(); entry != map_iter->second->UniqueEntries.end(); ++entry)
            {
                // M2 models don't have a bound set in WDT/ADT placement data, i still think they're not used for LoS at all on retail
                if (entry->second.flags & MOD_M2)
                {
                    if (!calculateTransformedBound(entry->second))
                        break;
                }
                else if (entry->second.flags & MOD_WORLDSPAWN) // WMO maps and terrain maps use different origin, so we need to adapt :/
                {
                    /// @todo remove extractor hack and uncomment below line:
                    //entry->second.iPos += Vector3(533.33333f*32, 533.33333f*32, 0.f);
                    entry->second.iBound = entry->second.iBound + Vector3(533.33333f*32, 533.33333f*32, 0.f);
                }
                mapSpawns.push_back(&(entry->second));
                spawnedModelFiles.insert(entry->second.name);
            }

            printf("Creating map tree for map %03u...                   \r", map_iter->first);
            BIH pTree;
            pTree.build(mapSpawns, BoundsTrait<ModelSpawn*>::getBounds);

            // ===> possibly move this code to StaticMapTree class
            std::map<G3D::uint32, G3D::uint32> modelNodeIdx;
            for (G3D::uint32 i=0; i<mapSpawns.size(); ++i)
                modelNodeIdx.insert(pair<G3D::uint32, G3D::uint32>(mapSpawns[i]->ID, i));

            FILE *mapfile = NULL; // write map tree file
            sprintf(output_filename, "%s/%03u.vmtiletree", iTileDestDir.c_str(), map_iter->first);
            fopen_s(&mapfile, output_filename, "wb");
            if (mapfile == NULL)
            {
                success = false;
                printf("Cannot open %s\n", output_filename);
                break;
            }

            //general info
            if (success && fwrite(VMAP_MAGIC, 10, 1, mapfile) != 1) success = false;
            G3D::uint32 globalTileID = StaticMapTree::packTileID(65, 65);
            G3D::uint32 spawnSize = map_iter->second->TileEntries[globalTileID].size(); // only maps without terrain (tiles) have global WMO
            if (success && fwrite(&spawnSize, sizeof(uint32), 1, mapfile) != 1) success = false;
            // Nodes
            if (success && fwrite("NODE", 4, 1, mapfile) != 1) success = false;
            if (success) success = pTree.writeToFile(mapfile);
            // global map spawns (WDT), if any (most instances)
            if (success && fwrite("GOBJ", 4, 1, mapfile) != 1) success = false;
            for (std::set<G3D::uint32>::iterator glob = map_iter->second->TileEntries[globalTileID].begin(); glob != map_iter->second->TileEntries[globalTileID].end() && success; ++glob)
                success = ModelSpawn::writeToFile(mapfile, map_iter->second->UniqueEntries[*glob]);
            // <====

            // Map tile offsets
            uint32 offsets[64][64];
            memset(&offsets, 0, sizeof(uint32)*64*64);
            size_t offsetLoc = ftell(mapfile);
            fwrite(offsets, sizeof(uint32)*64*64, 1, mapfile);
            // <====

            // write map tile files, similar to ADT files, only with extra BSP tree node info
            for(uint8 x = 0; x < 64; x++)
            {
                for(uint8 y = 0; y < 64; y++)
                {
                    G3D::uint32 globalTileID = StaticMapTree::packTileID(x, y);
                    if(map_iter->second->TileEntries.find(globalTileID) == map_iter->second->TileEntries.end())
                        continue;

                    std::set<const ModelSpawn*> correctSpawns;
                    std::set<uint32> &spawns = map_iter->second->TileEntries[globalTileID];
                    for(std::set<uint32>::iterator itr = spawns.begin(); itr != spawns.end(); itr++)
                    {
                        const ModelSpawn &spawn = map_iter->second->UniqueEntries[*itr];
                        if (spawn.flags & MOD_WORLDSPAWN) // WDT spawn, saved as tile 65/65 currently...
                            continue;
                        correctSpawns.insert(&spawn);
                    }

                    if(uint32 count = correctSpawns.size())
                    {
                        offsets[x][y] = ftell(mapfile);
                        if (success && fwrite(&count, sizeof(G3D::uint32), 1, mapfile) != 1)
                            success = false;
                        for(std::set<const ModelSpawn*>::iterator itr = correctSpawns.begin(); itr != correctSpawns.end() && success; itr++)
                        {
                            const ModelSpawn *spawn = *itr;
                            success = success && ModelSpawn::writeToFile(mapfile, *spawn);
                            // MapTree nodes to update when loading tile:
                            std::map<G3D::uint32, G3D::uint32>::iterator nIdx = modelNodeIdx.find(spawn->ID);
                            if (success && fwrite(&nIdx->second, sizeof(G3D::uint32), 1, mapfile) != 1)
                                success = false;
                        }
                    }
                }
            }
            fseek(mapfile, offsetLoc, SEEK_SET);
            fwrite(offsets, sizeof(uint32)*64*64, 1, mapfile);
            fclose(mapfile);
            //break; //test, extract only first map; TODO: remvoe this line
        }
        printf("Map tree generation finished\r\n");

        // add an object models, listed in temp_gameobject_models file
        exportGameobjectModels();

        // export objects
        printf("\nConverting Model Files \n");
        uint32 index = 0;
        for (std::set<std::string>::iterator mfile = spawnedModelFiles.begin(); mfile != spawnedModelFiles.end(); ++mfile)
        {
            printf("Converting model %05u of %05u\r", index++, spawnedModelFiles.size());
            if (!convertRawFile(*mfile))
            {
                OUT_ERROR("error converting %s", (*mfile).c_str());
                success = false;
                continue;
            }
        }
        printf("Finished converting model files\r\n");

        //cleanup:
        for (MapData::iterator map_iter = mapData.begin(); map_iter != mapData.end(); ++map_iter)
            delete map_iter->second;
        return success;
    }

    bool TileAssembler::readMapSpawns()
    {
        std::string fname = iSrcDir + "/dir_bin";
        FILE *dirf = fopen(fname.c_str(), "rb");
        if (!dirf)
        {
            OUT_DETAIL("Could not read dir_bin file!");
            return false;
        }

        OUT_DETAIL("Read coordinate mapping...");
        G3D::uint32 mapID, tileX, tileY, check=0;
        G3D::Vector3 v1, v2;
        ModelSpawn spawn;
        while (!feof(dirf))
        {
            check = 0;
            // read mapID, tileX, tileY, Flags, adtID, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
            check += (G3D::uint32)fread(&mapID, sizeof(G3D::uint32), 1, dirf);
            if (check == 0) // EoF...
                break;
            check += (G3D::uint32)fread(&tileX, sizeof(G3D::uint32), 1, dirf);
            check += (G3D::uint32)fread(&tileY, sizeof(G3D::uint32), 1, dirf);
            if (!ModelSpawn::readFromFile(dirf, spawn))
                break;

            MapSpawns *current;
            MapData::iterator map_iter = mapData.find(mapID);
            if (map_iter == mapData.end())
            {
                OUT_DETAIL("spawning Map %d", mapID);
                mapData[mapID] = current = new MapSpawns();
            } else current = (*map_iter).second;
            current->UniqueEntries.insert(std::make_pair(spawn.ID, spawn));
            current->TileEntries[StaticMapTree::packTileID(tileX, tileY)].insert(spawn.ID);
        }
        bool success = (ferror(dirf) == 0);
        fclose(dirf);
        return success;
    }

    bool TileAssembler::calculateTransformedBound(ModelSpawn &spawn)
    {
        std::string modelFilename(iSrcDir);
        modelFilename.push_back('/');
        modelFilename.append(spawn.name);

        ModelPosition modelPosition;
        modelPosition.iDir = spawn.iRot;
        modelPosition.iScale = spawn.iScale;
        modelPosition.init();

        WorldModel_Raw raw_model;
        if (!raw_model.Read(modelFilename.c_str()))
            return false;

        G3D::uint32 groups = raw_model.groupsArray.size();
        if (groups != 1)
            OUT_DETAIL("Warning: '%s' does not seem to be a M2 model!", modelFilename.c_str());

        AABox modelBound;
        bool boundEmpty=true;

        for (G3D::uint32 g=0; g<groups; ++g) // should be only one for M2 files...
        {
            std::vector<Vector3>& vertices = raw_model.groupsArray[g].vertexArray;

            if (vertices.empty())
            {
                OUT_DETAIL("error: model %s has no geometry!", spawn.name);
                continue;
            }

            G3D::uint32 nvectors = vertices.size();
            for (G3D::uint32 i = 0; i < nvectors; ++i)
            {
                G3D::Vector3 v = modelPosition.transform(vertices[i]);

                if (boundEmpty)
                    modelBound = G3D::AABox(v, v), boundEmpty=false;
                else
                    modelBound.merge(v);
            }
        }
        spawn.iBound = modelBound + spawn.iPos;
        spawn.flags |= MOD_HAS_BOUND;
        return true;
    }

    struct WMOLiquidHeader
    {
        int xverts, yverts, xtiles, ytiles;
        float pos_x;
        float pos_y;
        float pos_z;
        short type;
    };
    //=================================================================
    bool TileAssembler::convertRawFile(const std::string& pModelFilename)
    {
        bool success = true;
        std::string filename = iSrcDir;
        if (filename.length() >0)
            filename.push_back('/');
        filename.append(pModelFilename);

        WorldModel_Raw raw_model;
        if (!raw_model.Read(filename.c_str()))
            return false;

        // write WorldModel
        WorldModel model;
        model.setRootWmoID(raw_model.RootWMOID);
        if (!raw_model.groupsArray.empty())
        {
            std::vector<GroupModel> groupsArray;

            G3D::uint32 groups = raw_model.groupsArray.size();
            for (G3D::uint32 g = 0; g < groups; ++g)
            {
                GroupModel_Raw& raw_group = raw_model.groupsArray[g];
                groupsArray.push_back(GroupModel(raw_group.mogpflags, raw_group.GroupWMOID, raw_group.bounds ));
                groupsArray.back().setMeshData(raw_group.vertexArray, raw_group.triangles);
                groupsArray.back().setLiquidData(raw_group.liquid);
            }

            model.setGroupModels(groupsArray);
        }

        success = model.writeFile(iObjDestDir + "/" + pModelFilename + ".vmo");
        return success;
    }

    void TileAssembler::exportGameobjectModels()
    {
        FILE* model_list = fopen((iSrcDir + "/" + "temp_gameobject_models").c_str(), "rb");
        if (!model_list)
            return;

        FILE* model_list_copy = fopen((iObjDestDir + "/" + GAMEOBJECT_MODELS).c_str(), "wb");
        if (!model_list_copy)
        {
            fclose(model_list);
            return;
        }

        G3D::uint32 name_length, displayId;
        char buff[500];
        while (!feof(model_list))
        {
            if (fread(&displayId, sizeof(G3D::uint32), 1, model_list) != 1
                || fread(&name_length, sizeof(G3D::uint32), 1, model_list) != 1
                || name_length >= sizeof(buff)
                || fread(&buff, sizeof(char), name_length, model_list) != name_length)
            {
                OUT_DETAIL("\nFile 'temp_gameobject_models' seems to be corrupted");
                break;
            }

            std::string model_name(buff, name_length);

            WorldModel_Raw raw_model;
            if ( !raw_model.Read((iSrcDir + "/" + model_name).c_str()) )
                continue;

            spawnedModelFiles.insert(model_name);
            AABox bounds;
            bool boundEmpty = true;
            for (G3D::uint32 g = 0; g < raw_model.groupsArray.size(); ++g)
            {
                std::vector<Vector3>& vertices = raw_model.groupsArray[g].vertexArray;

                G3D::uint32 nvectors = vertices.size();
                for (G3D::uint32 i = 0; i < nvectors; ++i)
                {
                    Vector3& v = vertices[i];
                    if (boundEmpty)
                        bounds = AABox(v, v), boundEmpty = false;
                    else
                        bounds.merge(v);
                }
            }

            fwrite(&displayId, sizeof(G3D::uint32), 1, model_list_copy);
            fwrite(&name_length, sizeof(G3D::uint32), 1, model_list_copy);
            fwrite(&buff, sizeof(char), name_length, model_list_copy);
            fwrite(&bounds.low(), sizeof(G3D::Vector3), 1, model_list_copy);
            fwrite(&bounds.high(), sizeof(G3D::Vector3), 1, model_list_copy);
        }

        fclose(model_list);
        fclose(model_list_copy);
    }
        // temporary use defines to simplify read/check code (close file and return at fail)
        #define READ_OR_RETURN(V, S) if (fread((V), (S), 1, rf) != 1) { \
                                        fclose(rf); OUT_DETAIL("readfail, op = %i", readOperation); return(false); }
        #define READ_OR_RETURN_WITH_DELETE(V, S) if (fread((V), (S), 1, rf) != 1) { \
                                        fclose(rf); OUT_DETAIL("readfail, op = %i", readOperation); delete[] V; return(false); };
        #define CMP_OR_RETURN(V, S)  if (strcmp((V), (S)) != 0)        { \
                                        fclose(rf); OUT_DETAIL("cmpfail, %s!=%s", V, S);return(false); }

    bool GroupModel_Raw::Read(FILE* rf)
    {
        char blockId[5];
        blockId[4] = 0;
        int blocksize;
        int readOperation = 0;

        READ_OR_RETURN(&mogpflags, sizeof(G3D::uint32));
        READ_OR_RETURN(&GroupWMOID, sizeof(G3D::uint32));


        G3D::Vector3 vec1, vec2;
        READ_OR_RETURN(&vec1, sizeof(G3D::Vector3));

        READ_OR_RETURN(&vec2, sizeof(G3D::Vector3));
        bounds.set(vec1, vec2);

        READ_OR_RETURN(&liquidflags, sizeof(G3D::uint32));

        // will this ever be used? what is it good for anyway??
        G3D::uint32 branches;
        READ_OR_RETURN(&blockId, 4);
        CMP_OR_RETURN(blockId, "GRP ");
        READ_OR_RETURN(&blocksize, sizeof(int));
        READ_OR_RETURN(&branches, sizeof(G3D::uint32));
        for (G3D::uint32 b=0; b<branches; ++b)
        {
            G3D::uint32 indexes;
            // indexes for each branch (not used jet)
            READ_OR_RETURN(&indexes, sizeof(G3D::uint32));
        }

        // ---- indexes
        READ_OR_RETURN(&blockId, 4);
        CMP_OR_RETURN(blockId, "INDX");
        READ_OR_RETURN(&blocksize, sizeof(int));
        G3D::uint32 nindexes;
        READ_OR_RETURN(&nindexes, sizeof(G3D::uint32));
        if (nindexes >0)
        {
            G3D::uint16 *indexarray = new G3D::uint16[nindexes];
            READ_OR_RETURN_WITH_DELETE(indexarray, nindexes*sizeof(G3D::uint16));
            triangles.reserve(nindexes / 3);
            for (G3D::uint32 i=0; i<nindexes; i+=3)
                triangles.push_back(MeshTriangle(indexarray[i], indexarray[i+1], indexarray[i+2]));

            delete[] indexarray;
        }

        // ---- vectors
        READ_OR_RETURN(&blockId, 4);
        CMP_OR_RETURN(blockId, "VERT");
        READ_OR_RETURN(&blocksize, sizeof(int));
        G3D::uint32 nvectors;
        READ_OR_RETURN(&nvectors, sizeof(G3D::uint32));

        if (nvectors >0)
        {
            float *vectorarray = new float[nvectors*3];
            READ_OR_RETURN_WITH_DELETE(vectorarray, nvectors*sizeof(float)*3);
            for (G3D::uint32 i=0; i<nvectors; ++i)
                vertexArray.push_back( Vector3(vectorarray + 3*i) );

            delete[] vectorarray;
        }
        // ----- liquid
        liquid = 0;
        if (liquidflags& 1)
        {
            WMOLiquidHeader hlq;
            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "LIQU");
            READ_OR_RETURN(&blocksize, sizeof(int));
            READ_OR_RETURN(&hlq, sizeof(WMOLiquidHeader));
            liquid = new WmoLiquid(hlq.xtiles, hlq.ytiles, Vector3(hlq.pos_x, hlq.pos_y, hlq.pos_z), hlq.type);
            G3D::uint32 size = hlq.xverts*hlq.yverts;
            READ_OR_RETURN(liquid->GetHeightStorage(), size*sizeof(float));
            size = hlq.xtiles*hlq.ytiles;
            READ_OR_RETURN(liquid->GetFlagsStorage(), size);
        }

        return true;
    }


    GroupModel_Raw::~GroupModel_Raw()
    {
        delete liquid;
    }

    bool WorldModel_Raw::Read(const char * path)
    {
        FILE* rf = fopen(path, "rb");
        if (!rf)
        {
            OUT_DETAIL("ERROR: Can't open raw model file: %s", path);
            return false;
        }

        char ident[8];
        int readOperation = 0;

        READ_OR_RETURN(&ident, 8);
        CMP_OR_RETURN(ident, RAW_VMAP_MAGIC);

        // we have to read one int. This is needed during the export and we have to skip it here
        G3D::uint32 tempNVectors;
        READ_OR_RETURN(&tempNVectors, sizeof(G3D::uint32));

        G3D::uint32 groups;
        READ_OR_RETURN(&groups, sizeof(G3D::uint32));
        READ_OR_RETURN(&RootWMOID, sizeof(G3D::uint32));

        groupsArray.resize(groups);
        bool succeed = true;
        for (G3D::uint32 g = 0; g < groups && succeed; ++g)
            succeed = groupsArray[g].Read(rf);

        fclose(rf);
        return succeed;
    }

    // drop of temporary use defines
    #undef READ_OR_RETURN
    #undef CMP_OR_RETURN
}
