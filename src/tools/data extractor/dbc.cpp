/*
 * Thetruecrow
 * Citric
 * CactusEMU
 */

#include "dbc.h"
#include "misc.h"
#include "MPQ.h"
#include "DBCFile.h"

#include <set>
#include <map>

void ExtractDBCFiles(int locale, bool basicLocale)
{
    printf("Extracting dbc files...\n");

    std::set<std::pair<int, std::string> > dbcfiles;

    int foundCount = 0;

    for(int i = 0; i < PATCH_REV_COUNT + 1; i++)
    {
        TMPQArchive * archive = (TMPQArchive *)localeMPQ[i];
        TFileEntry * pFileEntry = archive->pFileTable;
        TFileEntry * pFileTableEnd = archive->pFileTable + archive->dwFileTableSize;

        // Parse the entire block table
        while(pFileEntry < pFileTableEnd)
        {
            // Only take existing files
            if( pFileEntry->dwFlags & MPQ_FILE_EXISTS &&
                (pFileEntry->dwFlags & MPQ_FILE_PATCH_FILE) == 0 &&
                (pFileEntry->dwFlags & MPQ_FILE_DELETE_MARKER) == 0 &&
                pFileEntry->szFileName != NULL)
            {
                std::string name = pFileEntry->szFileName;
                if (i != 0)
                {
                    if(name.find(langs[locale]) == 0)
                        name = name.substr(strlen(langs[locale]) + 1);
                    else
                    {
                        pFileEntry++;
                        continue;
                    }
                }

                if(name.rfind(".dbc") == name.length() - strlen(".dbc"))
                {
                    std::string new_name;
                    new_name.append(name.data(), name.length()-strlen(".dbc"));
                    new_name.append(".db2");

                    //Verify if this dbc isn't in the list yet. StormLibs return some extra dbcs :P
                    // Also check to make sure we haven't already been converted to the new db2 structure
                    if(i != 0)
                    {
                        bool alreadyExist = false;
                        for(std::set<std::pair<int, std::string> >::iterator itr = dbcfiles.begin(); itr != dbcfiles.end(); itr++)
                        {
                            if(itr->second == name || itr->second == new_name)
                            {
                                alreadyExist = true;
                                break;
                            }
                        }
                        if(alreadyExist)
                        {
                            pFileEntry++;
                            continue;
                        }
                    }
                    dbcfiles.insert(std::pair<int, std::string>(i, name));
                    foundCount++;

                }
                else if(name.rfind(".db2") == name.length() - strlen(".db2"))
                {
                    //Verify if this dbc isn't in the list yet. StormLibs return some extra dbcs :P
                    if(i != 0)
                    {
                        bool alreadyExist = false;
                        for(std::set<std::pair<int, std::string> >::iterator itr = dbcfiles.begin(); itr != dbcfiles.end(); itr++)
                        {
                            if(itr->second == name)
                            {
                                alreadyExist = true;
                                break;
                            }
                        }
                        if(alreadyExist)
                        {
                            pFileEntry++;
                            continue;
                        }
                    }
                    dbcfiles.insert(std::pair<int, std::string>(i, name));
                    std::string old_name;
                    old_name.append(name.data(), name.length()-strlen(".db2"));
                    old_name.append(".dbc");

                    if(dbcfiles.find(std::make_pair(i, old_name)) != dbcfiles.end())
                        dbcfiles.erase(std::make_pair(i, old_name));
                    else foundCount++;
                }
            }

            // Move to the next file entry
            pFileEntry++;
        }
    }
    printf("Found %i dbc files\n", foundCount);

    std::string path = "./dbc/";
    if(!basicLocale)
    {
        path += langs[locale];
        path += "/";
    }
    CreateDir(path);

    // extract DBCs
    int count = 0;
    for (std::set<std::pair<int, std::string> >::iterator iter = dbcfiles.begin(); iter != dbcfiles.end(); ++iter)
    {
        std::string filename = path;
        filename += (iter->second.c_str() + strlen("DBFilesClient\\"));

        if(ExtractFileToHardDrive(localeMPQ[iter->first], iter->second.c_str(), filename.c_str()) == ERROR_SUCCESS)
            ++count;
        else
        {
            assert(false);
        }
    }
    printf("Extracted %u DBC files\n\n", count);
    CreateCustomDBCFiles();
}

uint32 ReadMapDBC()
{
    printf("Read Map.dbc file... ");
    DBCFile dbc("DBFilesClient\\Map.dbc");

    if(!dbc.open())
    {
        printf("Fatal error: Invalid Map.dbc file format!\n");
        exit(1);
    }

    size_t map_count = dbc.getRecordCount();
    map_ids = new map_id[map_count];
    for(uint32 x = 0; x < map_count; ++x)
    {
        map_ids[x].id = dbc.getRecord(x).getUInt(0);
        strcpy_s(map_ids[x].name, dbc.getRecord(x).getString(1));
    }
    printf("Done! (%u maps loaded)\n", map_count);
    return map_count;
}

void ReadLiquidTypeTableDBC()
{
    printf("Read LiquidType.dbc file...");
    DBCFile dbc("DBFilesClient\\LiquidType.dbc");
    if(!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t LiqType_count = dbc.getRecordCount();
    size_t LiqType_maxid = dbc.getMaxId();
    LiqType = new uint16[LiqType_maxid + 1];
    memset(LiqType, 0xff, (LiqType_maxid + 1) * sizeof(uint16));

    for(uint32 x = 0; x < LiqType_count; ++x)
        LiqType[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);

    printf("Done! (%u LiqTypes loaded)\n", LiqType_count);
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

void OpenRequiredModelMPQs()
{
    char filename[512];

    sprintf_s(filename,"%s/Data/art.MPQ", input_path);
    printf("Loading %s\n", filename);
    if(!SFileOpenArchive(filename, 0, MPQ_OPEN_READ_ONLY, &ArtMPQ))
    {
        printf("%i\n", GetLastError());
        assert(false && "\nLoad of MPQ failed");
    }

    for(int j = 0; j < 3; j++)
    {
        sprintf_s(filename, "%s/Data/expansion%u.MPQ", input_path, j+1);
        printf("Patching %s into ART\n", filename);
        if(!SFileOpenPatchArchive(ArtMPQ, filename, "", MPQ_OPEN_READ_ONLY))
        {
            printf("%i\n", GetLastError());
            assert(false && "\nLoad of MPQ failed");
        }
    }

    for(int i = 0; i < PATCH_REV_COUNT; ++i)
    {
        char ext[7] = "";
        sprintf_s(ext, "-%i", patchRev[i]);

        sprintf_s(filename,"%s/Data/wow-update%s.MPQ", input_path, ext);
        printf("    -%i\n", patchRev[i]);
        if(!SFileOpenPatchArchive(ArtMPQ, filename, "base", MPQ_OPEN_READ_ONLY))
        {
            printf("%i\n", GetLastError());
            assert(false && "Load of MPQ patch failed");
        }
    }
    printf("\nArt MPQ patched!\n");
}

void CreateCustomDBCFiles()
{
    printf("Creating required custom dbc files...\n");
    OpenRequiredModelMPQs();

    std::map<uint32, DisplayBounding*> m_DisplayMap;
    std::map<std::string, ModelCache> modelCache;
    std::map<uint32, DBCFile::Record> modelInfoEntries;

    //map.dbc
    DBCFile displayInfo("DBFilesClient\\CreatureDisplayInfo.dbc"), modelInfo("DBFilesClient\\CreatureModelData.dbc");
    if(!displayInfo.open() || !modelInfo.open())
        return;

    std::string updateStr;
    std::string path = "./dbc/";
    std::string filename = path + std::string("CreatureBoundInformation.dbc");
    FILE* newDBC = fopen(filename.c_str(), "wb");
    if(newDBC != NULL)
    {
        char header[4];
        unsigned int na = 0, nb = 8, es = sizeof(DisplayBounding), ss = 0;
        header[0] = 'W'; header[1] = 'D';
        header[2] = 'B'; header[3] = 'C';
        fwrite(header, 4, 1, newDBC);

        for (DBCFile::Iterator itr = modelInfo.begin(); itr != modelInfo.end(); ++itr)
        {
            unsigned int entry = itr->getInt(0);
            modelInfoEntries.insert(std::make_pair(entry, *itr));
        }

        for (DBCFile::Iterator itr = displayInfo.begin(); itr != displayInfo.end(); ++itr)
        {
            unsigned int displayid = itr->getInt(0);
            unsigned int modelentry = itr->getInt(1);
            float modelscale = itr->getFloat(4);

            std::map<uint32, DBCFile::Record>::iterator  modelitr = modelInfoEntries.find(modelentry);
            if (modelitr == modelInfoEntries.end())
            {
#ifdef _DEBUG
                printf("Cannot find model entry for display %u (entry %u)\n", displayid, modelentry);
#endif
                continue;
            }
            updateStr.append(".");
            printf("Building%s %10s\r", updateStr.c_str(), "");
            if(!(na%10)) updateStr.clear();

            DisplayBounding* BoundingInfo = new DisplayBounding();
            DBCFile::Record modelrec = modelitr->second;

            const char* modelname = modelrec.getString(2);

            std::string strmodelname(modelname);

            replace(strmodelname, ".mdx", ".m2", 0);
            replace(strmodelname, ".MDX", ".m2", 0);

            M2Header* header;
            M2Attachment* attachments;
            M2Bone* bones;
            uint16* bonelookups;

            std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelname);
            if (cacheitr == modelCache.end())
            {
                MPQFile modelf(strmodelname.c_str(), ArtMPQ);
                if (modelf.isEof())
                {
                    delete BoundingInfo;
#ifdef _DEBUG
                    printf("Error: cannot open %s\n", strmodelname.c_str());
#endif
                    continue;
                }

                header = (M2Header*)malloc(sizeof(M2Header));
                modelf.read(header, sizeof(M2Header));

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
                modelCache.insert(std::make_pair(modelname, cacheentry));
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

    printf("Cleaning up bound data...\n");
    DisplayBounding* buff = NULL;
    for(std::map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(), itr2; itr != m_DisplayMap.end();)
    {
        itr2 = itr++;
        buff = itr2->second;
        m_DisplayMap.erase(itr2);
        delete buff;
        buff = NULL;
    }
    printf("Done!\n\n");
}
