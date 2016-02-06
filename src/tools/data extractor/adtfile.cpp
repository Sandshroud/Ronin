/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include "mpqfile.h"
#include "system.h"
#include "adtfile.h"

#include <algorithm>
#include <cstdio>

#ifdef WIN32
#define snprintf _snprintf
#endif

char const* GetPlainName(char const* FileName)
{
    const char * szTemp;

    if((szTemp = strrchr(FileName, '\\')) != NULL)
        FileName = szTemp + 1;
    return FileName;
}

char* GetPlainName(char* FileName)
{
    char * szTemp;

    if((szTemp = strrchr(FileName, '\\')) != NULL)
        FileName = szTemp + 1;
    return FileName;
}

void FixNameCase(char* name, size_t len)
{
    char* ptr = name + len - 1;

    //extension in lowercase
    for (; *ptr != '.'; --ptr)
        *ptr |= 0x20;

    for (; ptr >= name; --ptr)
    {
        if (ptr > name && *ptr >= 'A' && *ptr <= 'Z' && isalpha(*(ptr - 1)))
            *ptr |= 0x20;
        else if ((ptr == name || !isalpha(*(ptr - 1))) && *ptr >= 'a' && *ptr <= 'z')
            *ptr &= ~0x20;
    }
}

void FixNameSpaces(char* name, size_t len)
{
    for (size_t i=0; i<len-3; i++)
    {
        if(name[i] == ' ')
            name[i] = '_';
    }
}

char* GetExtension(char* FileName)
{
    if (char* szTemp = strrchr(FileName, '.'))
        return szTemp;
    return NULL;
}

ADTFile::ADTFile(HANDLE mpqarchive, char* filename) : ADT(mpqarchive, filename, false)
{
    Adtfilename.append(filename);
}

/*
for(uint8 x = 0; x < 8; x++)
{
    for(uint8 y = 0; y < 8; y++)
    {
        float centerX = header.zpos+(chunkStep*x)+halfStep, centerY = header.xpos+(chunkStep*y)+halfStep;
        // Calculate triangle Top
        triangle tTop;
        // X Axis - Different X axis
        tTop.corners[0].x = centerX - halfStep;
        tTop.corners[1].x = centerX + halfStep;
        tTop.corners[2].x = centerX;
        // Y Axis - Same Y axis
        tTop.corners[0].y = centerY - halfStep;
        tTop.corners[1].y = centerY - halfStep;
        tTop.corners[2].y = centerY;
        // Height
        tTop.corners[0].z = header.ypos+V9[x][y]; // Top left
        tTop.corners[1].z = header.ypos+V9[x+1][y]; // Top right
        tTop.corners[2].z = header.ypos+V8[x][y]; // Center
        // Push our triangle to storage
        triangles.push_back(tTop);

        // Calculate triangle Left
        triangle tLeft;
        // X Axis - Same X Axis
        tLeft.corners[0].x = centerX - halfStep;
        tLeft.corners[1].x = centerX - halfStep;
        tLeft.corners[2].x = centerX;
        // Y Axis - Different Y Axis
        tLeft.corners[0].y = centerY - halfStep;
        tLeft.corners[1].y = centerY + halfStep;
        tLeft.corners[2].y = centerY;
        // Height
        tLeft.corners[0].z = header.ypos+V9[x][y]; // Top left
        tLeft.corners[1].z = header.ypos+V9[x][y+1]; // Bottom left
        tLeft.corners[2].z = header.ypos+V8[x][y]; // Center
        // Push our triangle to storage
        triangles.push_back(tLeft);

        // Calculate triangle Right
        triangle tRight;
        // X Axis - Same X Axis
        tRight.corners[0].x = centerX + halfStep;
        tRight.corners[1].x = centerX + halfStep;
        tRight.corners[2].x = centerX;
        // Y Axis - Differeny Y Axis
        tRight.corners[0].y = centerY - halfStep;
        tRight.corners[1].y = centerY + halfStep;
        tRight.corners[2].y = centerY;
        // Height
        tRight.corners[0].z = header.ypos+V9[x+1][y]; // Top right
        tRight.corners[1].z = header.ypos+V9[x+1][y+1]; // bottom right
        tRight.corners[2].z = header.ypos+V8[x][y]; // Center
        // Push our triangle to storage
        triangles.push_back(tRight);

        // Calculate triangle Bottom
        triangle tBottom;
        // X Axis - Different X Axis
        tBottom.corners[0].x = centerX - halfStep;
        tBottom.corners[1].x = centerX + halfStep;
        tBottom.corners[2].x = centerX;
        // Y Axis - Same Y Axis
        tBottom.corners[0].y = centerY + halfStep;
        tBottom.corners[1].y = centerY + halfStep;
        tBottom.corners[2].y = centerY;
        // Height
        tBottom.corners[0].z = header.ypos+V9[x][y+1]; // Bottom left
        tBottom.corners[1].z = header.ypos+V9[x+1][y+1]; // Bottom right
        tBottom.corners[2].z = header.ypos+V8[x][y]; // Center
        // Push our triangle to storage
        triangles.push_back(tBottom);
    }
}
*/
bool ADTFile::parseCHNK(uint32 map_num, uint32 tileX, uint32 tileY, FILE *output)
{
    if(ADT.isEof ())
        return false;

    std::string dirname = std::string(szWorkDirWmo) + "/dir_bin";
    FILE *dirfile;
    fopen_s(&dirfile, dirname.c_str(), "ab");
    if(!dirfile)
    {
        printf("Can't open dirfile!'%s'\n", dirname.c_str());
        return false;
    }

    static float tileSize = 533.33333f, chunkSize = tileSize/16.f, chunkStep = chunkSize/9.f, halfStep = chunkStep/2.f;
    std::vector<triangle> triangles;

    bool res = false;
    uint32 offsets[16][16], mh2oOffset = 0;
    uint32 size = 0, adtVersion = 0, chunkCount = 0;
    while (!ADT.isEof())
    {
        char fourcc[5];
        ADT.read(&fourcc,4);
        ADT.read(&size, 4);
        flipcc(fourcc);
        fourcc[4] = 0;

        size_t currPos = ADT.getPos(), nextpos = currPos + size;
        if(size)
        {
            if (!strcmp(fourcc,"MVER"))
                ADT.read(&adtVersion, sizeof(uint32));
            else if (!strcmp(fourcc,"MHDR"))
            { }// MHDR is after version header, contaiins offset data for parsing the ADT file
            else if (!strcmp(fourcc,"MCNK"))
            {
                MapChunkHeader header;
                ADT.read(&header, sizeof(MapChunkHeader));
                if(header.offsMCVT)
                    offsets[header.ix][header.iy] = currPos+header.offsMCVT;
                else offsets[header.ix][header.iy] = 0;
            }
            else if (!strcmp(fourcc,"MFBO"))
            {   // Flight box
                // These are height planes for the top of the world and the bottom of the world
                unsigned short minimum[3][3], maximum[3][3];
                ADT.read(&maximum, sizeof(short)*3*3);
                ADT.read(&minimum, sizeof(short)*3*3);
                // Camera doesn't go below the minimum
                minimum, maximum;
            }
            else if (!strcmp(fourcc,"MH2O"))
                mh2oOffset = currPos;
        }

        //======================
        ADT.seek(nextpos);
    }

    for(uint8 x = 0; x < 16; x++)
    {
        for(uint8 y = 0; y < 16; y++)
        {
            if(offsets[x][y])
            {
                res = true;
                ADT.seek(offsets[x][y]);

            }
        }
    }

    ADT.close();
    fclose(dirfile);
    return res;
}

void ADTFile::parseWMO(uint32 map_num, uint32 tileX, uint32 tileY)
{
    if(ADT.isEof ())
        return;

    std::string dirname = std::string(szWorkDirWmo) + "/dir_bin";
    FILE *dirfile;
    fopen_s(&dirfile, dirname.c_str(), "ab");
    if(!dirfile)
    {
        printf("Can't open dirfile!'%s'\n", dirname.c_str());
        return;
    }

    uint32 size;
    while (!ADT.isEof())
    {
        char fourcc[5];
        ADT.read(&fourcc,4);
        ADT.read(&size, 4);
        flipcc(fourcc);
        fourcc[4] = 0;

        size_t nextpos = ADT.getPos() + size;

        if(size)
        {
            if (!strcmp(fourcc,"MMDX"))
            {
                char* buf = new char[size];
                ADT.read(buf, size);
                char* p = buf;
                int t = 0;
                while (p < buf + size)
                {
                    std::string path(p);

                    char* s = GetPlainName(p);
                    FixNameCase(s, strlen(s));
                    FixNameSpaces(s, strlen(s));

                    ModelInstanceNameMap.insert(std::make_pair(t++, new string(s)));
                    ExtractSingleModel(path);
                    p += strlen(p) + 1;
                }
                delete[] buf;
            }
            else if (!strcmp(fourcc,"MWMO"))
            {
                char* buf = new char[size];
                ADT.read(buf, size);
                char* p = buf;
                int q = 0;
                while (p < buf + size)
                {
                    char* s = GetPlainName(p);
                    FixNameCase(s, strlen(s));
                    FixNameSpaces(s, strlen(s));

                    WMOInstanceNameMap.insert(std::make_pair(q++, new string(s)));
                    p += strlen(p) + 1;
                }
                delete[] buf;
            }
            //======================
            else if (!strcmp(fourcc,"MDDF"))
            {
                nMDX = (int)size / 36;
                for (int i=0; i<nMDX; ++i)
                {
                    uint32 id;
                    ADT.read(&id, 4);
                    ModelInstance inst(ADT, ModelInstanceNameMap[id]->c_str(), map_num, tileX, tileY, dirfile);
                }
                for(std::map<uint32, std::string*>::iterator itr = ModelInstanceNameMap.begin(); itr != ModelInstanceNameMap.end(); itr++)
                    delete itr->second;
                ModelInstanceNameMap.clear();
            }
            else if (!strcmp(fourcc,"MODF"))
            {
                nWMO = (int)size / 64;
                for (int i=0; i<nWMO; ++i)
                {
                    uint32 id;
                    ADT.read(&id, 4);
                    WMOInstance inst(ADT, WMOInstanceNameMap[id]->c_str(), map_num, tileX, tileY, dirfile);
                }
                for(std::map<uint32, std::string*>::iterator itr = WMOInstanceNameMap.begin(); itr != WMOInstanceNameMap.end(); itr++)
                    delete itr->second;
                WMOInstanceNameMap.clear();
            }
        }

        //======================
        ADT.seek(nextpos);
    }

    ADT.close();
    fclose(dirfile);
}

ADTFile::~ADTFile()
{
    ADT.close();
}
