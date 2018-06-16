/*
 * Sandshroud Project Ronin
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

createFileSingleton(CNavMeshInterface);

void CNavMeshInterface::Init()
{
    if(sWorld.PathFinding == false)
        return;

    allocator = NULL;
    if( (hModule = LoadLibrary("./Modules/Pathfinding.dll")) && ((allocator = (mmap_manager_construction)GetProcAddress(hModule, "mmap_manager_construction")) == NULL) )
        FreeLibrary(hModule);
    else if(allocator)
        sLog.Success("NavMeshInterface", "Pathfinding linked successfully");
}

void CNavMeshInterface::DeInit()
{
    for(auto it : m_maps)
        delete it.second;
    FreeLibrary(hModule);
    allocator = NULL;
    m_maps.clear();
}

MMapManagerExt* CNavMeshInterface::GetOrCreateMMapManager(uint32 mapid)
{
    mapLock.Acquire();
    MMapManagerExt* ret = NULL;
    if(sWorld.PathFinding && allocator != NULL)
    {
        if(m_maps.find(mapid) == m_maps.end())
        {
            if(ret = allocator(sWorld.MNavPath.c_str(), mapid))
                m_maps.insert(std::make_pair(mapid, ret));
        } else ret = m_maps.at(mapid);
    }
    mapLock.Release();
    return ret;
}

bool CNavMeshInterface::IsNavmeshLoaded(uint32 mapid, uint32 x, uint32 y)
{
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
        return mmap->IsNavmeshLoaded(x, y);
    return false;
}

bool CNavMeshInterface::IsNavmeshLoadedAtPosition(uint32 mapid, float x, float y)
{
    if(!AreCoordinatesValid(x, y))
        return false;
    return IsNavmeshLoaded(mapid, (GetPosX(x)/CellsPerTile), (GetPosY(y)/CellsPerTile));
}

bool CNavMeshInterface::LoadNavMesh(uint32 mapid, uint32 x, uint32 y)
{
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
        return mmap->LoadNavMesh(x, y);
    return false;
}

void CNavMeshInterface::UnloadNavMesh(uint32 mapid, uint32 x, uint32 y)
{
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
        mmap->UnloadNavMesh(x, y);
}

bool CNavMeshInterface::BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, Position& out)
{
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
        return mmap->getNextPositionOnPathToLocation(startx, starty, startz, endx, endy, endz, out);
    return false;
}

Position CNavMeshInterface::BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool best)
{
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
    {
        if(best)
            return mmap->getBestPositionOnPathToLocation(startx, starty, startz, endx, endy, endz);
        return mmap->getNextPositionOnPathToLocation(startx, starty, startz, endx, endy, endz);
    }
    return Position(endx, endy, endz);
}

PositionMapContainer* CNavMeshInterface::BuildFullPath(Unit* m_Unit, uint32 mapid, float startx, float starty, float startz, float &endx, float &endy, float &endz, bool straight)
{
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
        return mmap->BuildFullPath(m_Unit->canFly(), startx, starty, startz, endx, endy, endz, straight);
    return NULL;
}

float CNavMeshInterface::GetWalkingHeight(uint32 mapid, float x, float y, float z, float z2)
{
    Position Step(0.f, 0.f, 0.f);
    float height = MMAP_UNAVAILABLE;
    if(MMapManagerExt* mmap = GetOrCreateMMapManager(mapid))
        if(mmap->GetWalkingHeightInternal(x, y, z, z2, Step))
            height = Step.z;
    return height;
}
