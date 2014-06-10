/***
 * Demonstrike Core
 */

#include "StdAfx.h"

SERVER_DECL CNavMeshInterface NavMeshInterface;

void CNavMeshInterface::Init()
{
    sLog.Notice("NavMeshInterface", "Init");
    memset( MMaps, 0, sizeof(MMapManager*)*NUM_MAPS );
}

void CNavMeshInterface::DeInit()
{
    for(uint32 i = 0; i < NUM_MAPS; i++)
        delete MMaps[i];
}

MMapManager* CNavMeshInterface::GetOrCreateMMapManager(uint32 mapid)
{
    if(MMaps[mapid] != NULL)
        return MMaps[mapid];

    return (MMaps[mapid] = new MMapManager(mapid));
}

bool CNavMeshInterface::IsNavmeshLoaded(uint32 mapid, uint32 x, uint32 y)
{
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    return mmap->IsNavmeshLoaded(x, y);
}

bool CNavMeshInterface::LoadNavMesh(uint32 mapid, uint32 x, uint32 y)
{
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    return mmap->LoadNavMesh(x, y);
}

void CNavMeshInterface::UnloadNavMesh(uint32 mapid, uint32 x, uint32 y)
{
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    mmap->UnloadNavMesh(x, y);
}

bool CNavMeshInterface::BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, LocationVector& out)
{
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    return mmap->getNextPositionOnPathToLocation(startx, starty, startz, endx, endy, endz, out);
}

LocationVector CNavMeshInterface::BuildPath(uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool best)
{
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    if(best)
        return mmap->getBestPositionOnPathToLocation(startx, starty, startz, endx, endy, endz);
    return mmap->getNextPositionOnPathToLocation(startx, starty, startz, endx, endy, endz);
}

LocationVectorMapContainer* CNavMeshInterface::BuildFullPath(Unit* m_Unit, uint32 mapid, float startx, float starty, float startz, float endx, float endy, float endz, bool straight)
{
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    return mmap->BuildFullPath(m_Unit, startx, starty, startz, endx, endy, endz, straight);
}

float CNavMeshInterface::GetWalkingHeight(uint32 mapid, float x, float y, float z, float z2)
{
    LocationVector Step;
    float height = MMAP_UNAVAILABLE;
    MMapManager* mmap = GetOrCreateMMapManager(mapid);
    if(mmap->GetWalkingHeightInternal(x, y, z, z2, Step))
        height = Step.z;
    return height;
}

MMapManager::MMapManager(uint32 mapid)
{
    lastTileRef = 0;
    m_navMesh = NULL;
    ManagerMapId = mapid;

    // load and init dtNavMesh - read parameters from file
    uint32 pathLen = uint32(sWorld.MMapPath.length() + strlen("/000.mmap")+1);
    char *fileName = new char[pathLen];
    snprintf(fileName, pathLen, (sWorld.MMapPath+"/%03i.mmap").c_str(), ManagerMapId);

    FILE* file = fopen(fileName, "rb");
    if (!file)
    {
        sLog.Debug("NavMeshInterface", "Could not load mmap %03i", mapid);
        delete [] fileName;
        return;
    }

    dtNavMeshParams params;
    fread(&params, sizeof(dtNavMeshParams), 1, file);
    fclose(file);

    dtNavMesh* mesh = dtAllocNavMesh();
    ASSERT(mesh);
    if(dtStatusFailed(mesh->init(&params)))
    {
        dtFreeNavMesh(mesh);
        sLog.Debug("NavMeshInterface", "Failed to initialize dtNavMesh for mmap %03u from file %s", mapid, fileName);
        delete [] fileName;
        return;
    }

    m_navMeshQuery = dtAllocNavMeshQuery();
    if(dtStatusFailed(m_navMeshQuery->init(mesh, 1024)))
    {
        dtFreeNavMesh(mesh);
        sLog.Debug("NavMeshInterface", "Failed to initialize dtNavMeshQuery for mmap %03u from file %s", mapid, fileName);
        delete [] fileName;
        return;
    }

    delete [] fileName;

    sLog.Debug("NavMeshInterface", "Loaded %03i.mmap", mapid);

    // store inside our map list
    m_navMesh = mesh;
}

MMapManager::~MMapManager()
{
    for(uint32 x = 0; x < 64; x++)
        for(uint32 y = 0; y < 64; y++)
            UnloadNavMesh(x, y);
    dtFreeNavMesh(m_navMesh);
    dtFreeNavMeshQuery(m_navMeshQuery);
}

float MMapManager::calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
    float dx = Position2X-Position1X;
    float dy = Position2Y-Position1Y;
    double angle = 0.0f;

    // Calculate angle
    if (dx == 0.0)
    {
        if (dy == 0.0)
            angle = 0.0;
        else if (dy > 0.0)
            angle = M_PI * 0.5 /* / 2 */;
        else
            angle = M_PI * 3.0 * 0.5/* / 2 */;
    }
    else if (dy == 0.0)
    {
        if (dx > 0.0)
            angle = 0.0;
        else
            angle = M_PI;
    }
    else
    {
        if (dx < 0.0)
            angle = atanf(dy/dx) + M_PI;
        else if (dy < 0.0)
            angle = atanf(dy/dx) + (2*M_PI);
        else
            angle = atanf(dy/dx);
    }

    // Convert to degrees
    angle = angle * float(180 / M_PI);

    // Return
    return float(angle);
}

bool MMapManager::LoadNavMesh(uint32 x, uint32 y)
{
    if(m_navMesh == NULL)
        return false;

    uint32 PackedTileID = packTileID(x, y);
    dtTileRef reference = 0;

    ReferenceMap::iterator itr = TileReferences.find(PackedTileID);
    if(itr == TileReferences.end())
    {
        // load this tile :: mmaps/MMMXXYY.mmtile
        uint32 pathLen = uint32(sWorld.MMapPath.length() + strlen("/0000000.mmtile")+1);
        char *fileName = new char[pathLen];
        snprintf(fileName, pathLen, (sWorld.MMapPath+"/%03i%02i%02i.mmtile").c_str(), ManagerMapId, x, y);
        FILE *file = fopen(fileName, "rb");
        if (!file)
        {
            sLog.Debug("NavMeshInterface", "Could not open mmtile file '%s'", fileName);
            delete [] fileName;
            return false;
        }
        delete [] fileName;

        // read header
        MmapTileHeader fileHeader;
        fread(&fileHeader, sizeof(MmapTileHeader), 1, file);
        if (fileHeader.mmapMagic != MMAP_MAGIC)
        {
            sLog.Error("NavMeshInterface", "Bad header in mmap %03u%02i%02i.mmtile", ManagerMapId, x, y);
            fclose(file);
            return false;
        }

        if (fileHeader.mmapVersion != MMAP_VERSION)
        {
            sLog.Error("NavMeshInterface", "%03u%02i%02i.mmtile was built with generator v%i, expected v%i", ManagerMapId, x, y, fileHeader.mmapVersion, MMAP_VERSION);
            fclose(file);
            return false;
        }

        unsigned char* data = (unsigned char*)malloc(fileHeader.size);
        ASSERT(data);

        size_t result = fread(data, fileHeader.size, 1, file);
        if(!result)
        {
            sLog.Error("NavMeshInterface", "Bad header or data in mmap %03u%02u%02u.mmtile", ManagerMapId, x, y);
            fclose(file);
            return false;
        }
        fclose(file);

        dtStatus dtresult;
        dtMeshHeader* header = (dtMeshHeader*)data;

        // memory allocated for data is now managed by detour, and will be deallocated when the tile is removed
        dtresult = m_navMesh->addTile(data, fileHeader.size, DT_TILE_FREE_DATA, 0, &reference);
        if(dtStatusInProgress(dtresult)) // We already have it loaded, oops.
            free(data);
        else if(dtStatusFailed(dtresult))
        {
            free(data);
            sLog.Debug("NavMeshInterface", "Could not load %03u%02u%02u.mmtile into navmesh", ManagerMapId, x, y);
            return false;
        }
        else
        {
            sLog.Debug("NavMeshInterface", "Loaded mmtile %03u[%I64ld] into %03u[%02u,%02u]", ManagerMapId, reference, ManagerMapId, x, y);
            TileReferences.insert(make_pair(PackedTileID, new TileReferenceC(reference)));
        }
    }
    else reference = itr->second->ID;

    TileLoadCount[reference]++;
    return true;
}

void MMapManager::UnloadNavMesh(uint32 x, uint32 y)
{
    if(m_navMesh == NULL)
        return;

    uint32 PackedTileID = packTileID(x, y);
    ReferenceMap::iterator itr = TileReferences.find(PackedTileID);
    if(itr == TileReferences.end())
        return; // We aren't loaded, so why continue?

    dtTileRef reference = itr->second->ID;
    if(TileLoadCount[reference] == 1)
    {
        dtStatus status = m_navMesh->removeTile(reference, NULL, NULL);
        if(dtStatusFailed(status))
        {
            sLog.Debug("NavMeshInterface", "Failed to unload mmtile %03u[%I64ld] from %03u[%02u,%02u]", ManagerMapId, reference, ManagerMapId, x, y);
            return;
        }
        delete itr->second;
        TileReferences.erase(itr);
        sLog.Debug("NavMeshInterface", "Unloaded mmtile %03u[%I64ld] from %03u[%02u,%02u]", ManagerMapId, reference, ManagerMapId, x, y);
    }

    TileLoadCount[reference]--;
}

bool MMapManager::IsNavmeshLoaded(uint32 x, uint32 y)
{
    uint32 PackedTileID = packTileID(x, y);
    ReferenceMap::iterator itr = TileReferences.find(PackedTileID);
    if(itr == TileReferences.end())
        return false;
    return true;
}

LocationVector MMapManager::getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz)
{
    if(m_navMesh == NULL)
        return LocationVector(endx, endy, endz);

    //convert to nav coords.
    float startPos[3] = { starty, startz, startx };
    float endPos[3] = { endy, endz, endx };
    float mPolyPickingExtents[3] = { 2.00f, 4.00f, 2.00f };
    float closestPoint[3] = {0.0f, 0.0f, 0.0f};
    LocationVector pos;
    pos.x = endx;
    pos.y = endy;
    pos.z = endz;
    dtStatus result;
    dtQueryFilter* mPathFilter = new dtQueryFilter();
    if(mPathFilter)
    {
        dtPolyRef mStartRef;
        result = m_navMeshQuery->findNearestPoly(startPos, mPolyPickingExtents, mPathFilter, &mStartRef, closestPoint);
        if(dtStatusFailed(result) || !mStartRef)
        {
            delete mPathFilter;
            mPathFilter = NULL;
            return pos;
        }

        dtPolyRef mEndRef;
        result = m_navMeshQuery->findNearestPoly(endPos, mPolyPickingExtents, mPathFilter, &mEndRef, closestPoint);
        if(dtStatusFailed(result) || !mEndRef)
        {
            delete mPathFilter;
            mPathFilter = NULL;
            return pos;
        }

        if (mStartRef != 0 && mEndRef != 0)
        {
            int mNumPathResults;
            dtPolyRef mPathResults[50];
            result = m_navMeshQuery->findPath(mStartRef, mEndRef,startPos, endPos, mPathFilter, mPathResults, &mNumPathResults, 50);
            if(dtStatusFailed(result) || mNumPathResults <= 0)
            {
                delete mPathFilter;
                mPathFilter = NULL;
                return pos;
            }

            int mNumPathPoints;
            float actualpath[3*20];
            dtPolyRef polyrefs = 0;
            result = m_navMeshQuery->findStraightPath(startPos, endPos, mPathResults, mNumPathResults, actualpath, NULL, &polyrefs, &mNumPathPoints, 20);
            if (dtStatusFailed(result) /*|| mNumPathPoints < 3*/)
            {
                delete mPathFilter;
                mPathFilter = NULL;
                return pos;
            }

            pos.y = actualpath[3]; //0 3 6
            pos.z = actualpath[4]; //1 4 7
            pos.x = actualpath[5]; //2 5 8
            delete mPathFilter;
            mPathFilter = NULL;
            return pos;
        }
    }
    return pos;
}

bool MMapManager::getNextPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz, LocationVector& out)
{
    if(m_navMesh == NULL)
        return false;

    dtStatus result;
    //convert to nav coords.
    float startPos[3] = { starty, startz, startx };
    float endPos[3] = { endy, endz, endx };
    float mPolyPickingExtents[3] = { 2.00f, 4.00f, 2.00f };
    float closestPoint[3] = {0.0f, 0.0f, 0.0f};
    dtQueryFilter* mPathFilter = new dtQueryFilter();
    if(mPathFilter)
    {
        dtPolyRef mStartRef;
        result = m_navMeshQuery->findNearestPoly(startPos, mPolyPickingExtents, mPathFilter, &mStartRef, closestPoint);
        if(dtStatusFailed(result) || !mStartRef)
        {
            delete mPathFilter;
            mPathFilter = NULL;
            return false;
        }

        dtPolyRef mEndRef;
        result = m_navMeshQuery->findNearestPoly(endPos, mPolyPickingExtents, mPathFilter, &mEndRef, closestPoint);
        if(dtStatusFailed(result) || !mEndRef)
        {
            delete mPathFilter;
            mPathFilter = NULL;
            return false;
        }

        if (mStartRef != 0 && mEndRef != 0)
        {
            int mNumPathResults;
            dtPolyRef mPathResults[50];
            result = m_navMeshQuery->findPath(mStartRef, mEndRef,startPos, endPos, mPathFilter, mPathResults, &mNumPathResults, 50);
            if(dtStatusFailed(result) || mNumPathResults <= 0)
            {
                delete mPathFilter;
                mPathFilter = NULL;
                return false;
            }

            int mNumPathPoints;
            float actualpath[3*20];
            dtPolyRef polyrefs = 0;
            result = m_navMeshQuery->findStraightPath(startPos, endPos, mPathResults, mNumPathResults, actualpath, NULL, &polyrefs, &mNumPathPoints, 20);
            if (dtStatusFailed(result) /*|| mNumPathPoints < 3*/)
            {
                delete mPathFilter;
                mPathFilter = NULL;
                return false;
            }

            out.y = actualpath[3]; //0 3 6
            out.z = actualpath[4]; //1 4 7
            out.x = actualpath[5]; //2 5 8
            delete mPathFilter;
            mPathFilter = NULL;
            return true;
        }
    }
    return false;
}

LocationVector MMapManager::getBestPositionOnPathToLocation(float startx, float starty, float startz, float endx, float endy, float endz)
{
    if(startx == endx && starty == endy && startz == endz)
        return LocationVector(endx, endy, endz); // Fixed below but save us some time here.

    LocationVector pos(startx, starty, startz);
    LocationVector nextpos(startx, starty, startz);
    LocationVector returnpos(endx, endy, endz);
    pos = getNextPositionOnPathToLocation(startx, starty, startz, endx, endy, endz);
    float line = calcAngle(startx, starty, pos.x, pos.y);
    while(1)
    {
        if(nextpos.x == returnpos.x || nextpos.y == returnpos.y)
            break; // This WILL fix NAN issues

        nextpos = getNextPositionOnPathToLocation(pos.x, pos.y, pos.z, endx, endy, endz);
        float angle = calcAngle( startx, starty, nextpos.x, nextpos.y );
        if(angle != line)
        {   // We have to turn, so stop our line here.
            returnpos = pos;
            break;
        }

        if(pos.x == nextpos.x || pos.y == nextpos.y)
        {
            returnpos = pos;
            break;
        }
        pos = nextpos;
    }
    return returnpos;
}

dtPolyRef MMapManager::GetPathPolyByPosition(dtPolyRef const* polyPath, uint32 polyPathSize, float const* point, float* distance) const
{
    if (!polyPath || !polyPathSize)
        return 0;

    dtPolyRef nearestPoly = 0;
    float minDist2d = FLT_MAX;
    float minDist3d = 0.0f;

    for (uint32 i = 0; i < polyPathSize; ++i)
    {
        float closestPoint[3];
        if (dtStatusFailed(m_navMeshQuery->closestPointOnPoly(polyPath[i], point, closestPoint)))
            continue;

        float d = dtVdist2DSqr(point, closestPoint);
        if (d < minDist2d)
        {
            minDist2d = d;
            nearestPoly = polyPath[i];
            minDist3d = dtVdistSqr(point, closestPoint);
        }

        if (minDist2d < 1.0f) // shortcut out - close enough for us
            break;
    }

    if (distance)
        *distance = dtSqrt(minDist3d);

    return (minDist2d < 3.0f) ? nearestPoly : 0;
}

dtPolyRef MMapManager::GetPolyByLocation(dtQueryFilter* m_filter, dtPolyRef const* polyPath, uint32 polyPathSize, float const* point, float* distance) const
{
    // first we check the current path
    // if the current path doesn't contain the current poly,
    // we need to use the expensive navMesh.findNearestPoly
    dtPolyRef polyRef = GetPathPolyByPosition(polyPath, polyPathSize, point, distance);
    if (polyRef != 0)
        return polyRef;

    // we don't have it in our old path
    // try to get it by findNearestPoly()
    // first try with low search box
    float extents[3] = {3.0f, 5.0f, 3.0f};    // bounds of poly search area
    float closestPoint[3] = {0.0f, 0.0f, 0.0f};
    if (dtStatusSucceed(m_navMeshQuery->findNearestPoly(point, extents, m_filter, &polyRef, closestPoint)) && polyRef != 0)
    {
        *distance = dtVdist(closestPoint, point);
        return polyRef;
    }

    // still nothing ..
    // try with bigger search box
    // Note that the extent should not overlap more than 128 polygons in the navmesh (see dtNavMeshQuery::findNearestPoly)
    extents[1] = 50.0f;
    if (dtStatusSucceed(m_navMeshQuery->findNearestPoly(point, extents, m_filter, &polyRef, closestPoint)) && polyRef != 0)
    {
        *distance = dtVdist(closestPoint, point);
        return polyRef;
    }

    return 0;
}

uint32 MMapManager::fixupCorridor(dtPolyRef* path, uint32 npath, uint32 maxPath, dtPolyRef* visited, uint32 nvisited)
{
    int32 furthestPath = -1;
    int32 furthestVisited = -1;

    // Find furthest common polygon.
    for (int32 i = npath-1; i >= 0; --i)
    {
        bool found = false;
        for (int32 j = nvisited-1; j >= 0; --j)
        {
            if (path[i] == visited[j])
            {
                furthestPath = i;
                furthestVisited = j;
                found = true;
            }
        }
        if (found)
            break;
    }

    // If no intersection found just return current path.
    if (furthestPath == -1 || furthestVisited == -1)
        return npath;

    // Concatenate paths.

    // Adjust beginning of the buffer to include the visited.
    uint32 req = nvisited - furthestVisited;
    uint32 orig = uint32(furthestPath+1) < npath ? furthestPath+1 : npath;
    uint32 size = npath-orig > 0 ? npath-orig : 0;
    if (req+size > maxPath)
        size = maxPath-req;

    if (size)
        memmove(path+req, path+orig, size*sizeof(dtPolyRef));

    // Store visited
    for (uint32 i = 0; i < req; ++i)
        path[i] = visited[(nvisited-1)-i];

    return req+size;
}

static const uint32 MAX_STEER_POINTS = 3;

bool MMapManager::getSteerTarget(float* startPos, float* endPos, float minTargetDist, dtPolyRef* path, uint32 pathSize, float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef)
{
    // Find steer target.
    float steerPath[MAX_STEER_POINTS*3];
    unsigned char steerPathFlags[MAX_STEER_POINTS];
    dtPolyRef steerPathPolys[MAX_STEER_POINTS];
    uint32 nsteerPath = 0;
    dtStatus dtResult = m_navMeshQuery->findStraightPath(startPos, endPos, path, pathSize, steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
    if (!nsteerPath || dtStatusFailed(dtResult))
        return false;

    // Find vertex far enough to steer to.
    uint32 ns = 0;
    while (ns < nsteerPath)
    {
        // Stop at Off-Mesh link or when point is further than slop away.
        if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
            !inRangeYZX(&steerPath[ns*3], startPos, minTargetDist, 1000.0f))
            break;
        ns++;
    }
    // Failed to find good point to steer to.
    if (ns >= nsteerPath)
        return false;

    dtcopy(steerPos, &steerPath[ns*3]);
    steerPos[1] = startPos[1];  // keep Z value
    steerPosFlag = steerPathFlags[ns];
    steerPosRef = steerPathPolys[ns];

    return true;
}

dtStatus MMapManager::findSmoothPath(dtQueryFilter* m_filter, float* startPos, float* endPos, dtPolyRef* polyPath, uint32 polyPathSize, float* smoothPath, int* smoothPathSize, const uint32 maxSmoothPathSize)
{
    ASSERT(polyPathSize <= 74);
    *smoothPathSize = 0;
    uint32 nsmoothPath = 0;

    dtPolyRef polys[74];
    memcpy(polys, polyPath, sizeof(dtPolyRef)*polyPathSize);
    uint32 npolys = polyPathSize;

    float iterPos[3], targetPos[3];
    if (dtStatusFailed(m_navMeshQuery->closestPointOnPolyBoundary(polys[0], startPos, iterPos)))
        return DT_FAILURE;
    if (dtStatusFailed(m_navMeshQuery->closestPointOnPolyBoundary(polys[npolys-1], endPos, targetPos)))
        return DT_FAILURE;

    dtVcopy(&smoothPath[nsmoothPath*3], iterPos);
    nsmoothPath++;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && nsmoothPath < maxSmoothPathSize)
    {
        // Find location to steer towards.
        float steerPos[3];
        unsigned char steerPosFlag;
        dtPolyRef steerPosRef = 0;

        if (!getSteerTarget(iterPos, targetPos, 0.3f, polys, npolys, steerPos, steerPosFlag, steerPosRef))
            break;

        bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END);
        bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION);

        // Find movement delta.
        float delta[3];
        dtVsub(delta, steerPos, iterPos);
        float len = dtSqrt(dtVdot(delta, delta));
        // If the steer target is end of path or off-mesh link, do not move past the location.
        if ((endOfPath || offMeshConnection) && len < 4.0f)
            len = 1.0f;
        else
            len = 4.0f / len;

        float moveTgt[3];
        dtVmad(moveTgt, iterPos, delta, len);

        // Move
        float result[3];
        const static uint32 MAX_VISIT_POLY = 16;
        dtPolyRef visited[MAX_VISIT_POLY];

        uint32 nvisited = 0;
        m_navMeshQuery->moveAlongSurface(polys[0], iterPos, moveTgt, m_filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY);
        npolys = fixupCorridor(polys, npolys, 74, visited, nvisited);

        m_navMeshQuery->getPolyHeight(polys[0], result, &result[1]);
        result[1] += 0.5f;
        dtVcopy(iterPos, result);

        // Handle end of path and off-mesh links when close enough.
        if (endOfPath && inRangeYZX(iterPos, steerPos, 0.3f, 1.0f))
        {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (nsmoothPath < maxSmoothPathSize)
            {
                dtVcopy(&smoothPath[nsmoothPath*3], iterPos);
                nsmoothPath++;
            }
            break;
        }
        else if (offMeshConnection && inRangeYZX(iterPos, steerPos, 0.3f, 1.0f))
        {
            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = 0;
            dtPolyRef polyRef = polys[0];
            uint32 npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
                prevRef = polyRef;
                polyRef = polys[npos];
                npos++;
            }

            for (uint32 i = npos; i < npolys; ++i)
                polys[i-npos] = polys[i];

            npolys -= npos;

            // Handle the connection.
            float startPos[3], endPos[3];
            if (dtStatusSucceed(m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos)))
            {
                if (nsmoothPath < maxSmoothPathSize)
                {
                    dtVcopy(&smoothPath[nsmoothPath*3], startPos);
                    nsmoothPath++;
                }
                // Move position at the other side of the off-mesh link.
                dtVcopy(iterPos, endPos);
                m_navMeshQuery->getPolyHeight(polys[0], iterPos, &iterPos[1]);
                iterPos[1] += 0.5f;
            }
        }

        // Store results.
        if (nsmoothPath < maxSmoothPathSize)
        {
            dtVcopy(&smoothPath[nsmoothPath*3], iterPos);
            nsmoothPath++;
        }
    }

    *smoothPathSize = nsmoothPath;

    // this is most likely a loop
    return nsmoothPath < 74 ? DT_SUCCESS : DT_FAILURE;
}

LocationVectorMapContainer* MMapManager::BuildFullPath(Unit* m_Unit, float startx, float starty, float startz, float endx, float endy, float endz, bool straight)
{
    if(m_navMesh == NULL)
        return NULL;

    dtQueryFilter* mPathFilter = new dtQueryFilter();
    unsigned short includeFlags = 0x01;

    if (m_Unit->GetTypeId() == TYPEID_UNIT)
    {
        // creatures don't take environmental damage
        Creature* creature = (Creature*)m_Unit;
        if (creature->GetCanMove() & LIMIT_WATER)
            includeFlags |= (0x08/*Water*/ | 0x02/*Magma*/ | 0x04/*Slime*/);
    }
    else if (m_Unit->GetTypeId() == TYPEID_PLAYER)
    {
        // perfect support not possible, just stay 'safe'
        includeFlags |= 0x08;
    }
    mPathFilter->setIncludeFlags(includeFlags);

    uint32 m_polyLength = 0, m_maxLength = 74;
    dtPolyRef m_pathPolyRefs[74]; // array of detour polygon references

    float distToStartPoly, distToEndPoly;
    float endPoint[3] = { endy, endz, endx }, startPoint[3] = { starty, startz, startx };
    dtPolyRef mStartRef = GetPolyByLocation(mPathFilter, m_pathPolyRefs, m_maxLength, startPoint, &distToStartPoly);
    dtPolyRef mEndRef = GetPolyByLocation(mPathFilter, m_pathPolyRefs, m_maxLength, endPoint, &distToEndPoly);
    if(mStartRef == 0 || mEndRef == 0 || mStartRef == mEndRef)
    {
        delete mPathFilter;
        mPathFilter = NULL;
        return NULL;
    }

    dtStatus dtResult = m_navMeshQuery->findPath(
            mStartRef,          // start polygon
            mEndRef,            // end polygon
            startPoint,         // start position
            endPoint,           // end position
            mPathFilter,        // polygon search filter
            m_pathPolyRefs,     // [out] path
            (int*)&m_polyLength,
            m_maxLength);       // max number of polygons in output path
    if(dtStatusFailed(dtResult) || !m_polyLength)
    {
        delete mPathFilter;
        mPathFilter = NULL;
        return NULL;
    }

    bool farFromPoly = (distToStartPoly > 7.0f || distToEndPoly > 7.0f);

    float pathPoints[74*3];
    uint32 pointCount = 0;
    if (straight)
    {
        dtResult = m_navMeshQuery->findStraightPath(
                startPoint,         // start position
                endPoint,           // end position
                m_pathPolyRefs,     // current path
                m_polyLength,       // lenth of current path
                pathPoints,         // [out] path corner points
                NULL,               // [out] flags
                NULL,               // [out] shortened path
                (int*)&pointCount,
                m_maxLength);               // maximum number of points/polygons to use
    }
    else
    {
        dtResult = findSmoothPath(mPathFilter,
                startPoint,         // start position
                endPoint,           // end position
                m_pathPolyRefs,     // current path
                m_polyLength,       // length of current path
                pathPoints,         // [out] path corner points
                (int*)&pointCount,
                m_maxLength);               // maximum number of points
    }

    if (pointCount < 2 || dtStatusFailed(dtResult))
    {
        // only happens if pass bad data to findStraightPath or navmesh is broken
        // single point paths can be generated here
        // TODO : check the exact cases
        delete mPathFilter;
        mPathFilter = NULL;
        return NULL;
    }

    LocationVectorMapContainer* map = new LocationVectorMapContainer();
    map->InternalMap = new LocationVectorMap();
    map->TotalMoveTime = 0;
    float x = startx, y = starty, z = startz;
    for (uint32 i = 0; i < pointCount-1; ++i)
    {
        LocationVector pos(pathPoints[i*3+2], pathPoints[i*3], pathPoints[i*3+1]);
        float distance = m_Unit->CalcDistance(x, y, z, pos.x, pos.y, pos.z);
        map->TotalMoveTime += float2int32(m_Unit->GetAIInterface()->GetMovementTime(distance));
        map->InternalMap->insert(make_pair(map->TotalMoveTime, pos));
        x = pos.x, y = pos.y, z = pos.z;
    }
    map->StartTime = getMSTime();
    return map;
}

bool MMapManager::GetWalkingHeightInternal(float positionx, float positiony, float positionz, float endz, LocationVector& out)
{
    if(m_navMesh == NULL)
        return false;

    dtStatus result;
    //convert to nav coords.
    float startPos[3] = { positionx, positiony, positionz };
    float endPos[3] = { positionx, positiony, endz };
    float mPolyPickingExtents[3] = { 2.00f, 2.00f, 4.00f };
    float closestPoint[3] = {0.0f, 0.0f, 0.0f};
    dtQueryFilter* mPathFilter = new dtQueryFilter();
    if(mPathFilter)
    {
        dtPolyRef mStartRef;
        result = m_navMeshQuery->findNearestPoly(startPos, mPolyPickingExtents, mPathFilter, &mStartRef, closestPoint);
        if(dtStatusFailed(result) || !mStartRef)
        {
            delete mPathFilter;
            mPathFilter = NULL;
            return false;
        }

        dtPolyRef mEndRef;
        result = m_navMeshQuery->findNearestPoly(endPos, mPolyPickingExtents, mPathFilter, &mEndRef, closestPoint);
        if(dtStatusFailed(result) || !mEndRef)
        {
            delete mPathFilter;
            mPathFilter = NULL;
            return false;
        }

        if (mStartRef != 0 && mEndRef != 0)
        {
            int mNumPathResults;
            dtPolyRef mPathResults[50];
            result = m_navMeshQuery->findPath(mStartRef, mEndRef,startPos, endPos, mPathFilter, mPathResults, &mNumPathResults, 50);
            if(dtStatusFailed(result) || mNumPathResults <= 0)
            {
                delete mPathFilter;
                mPathFilter = NULL;
                return false;
            }

            int mNumPathPoints;
            float actualpath[3*2];
            dtPolyRef polyrefs = 0;
            result = m_navMeshQuery->findStraightPath(startPos, endPos, mPathResults, mNumPathResults, actualpath, NULL, &polyrefs, &mNumPathPoints, 2);
            if (dtStatusFailed(result))
            {
                delete mPathFilter;
                mPathFilter = NULL;
                return false;
            }

            if(mNumPathPoints < 3)
            {
                out.y = positiony;
                out.z = positionz;
                out.x = positionx;
                delete mPathFilter;
                mPathFilter = NULL;
                return true;
            }

            out.y = actualpath[3];
            out.z = actualpath[4];
            out.x = actualpath[5];
            delete mPathFilter;
            mPathFilter = NULL;
            return true;
        }
    }
    return false;
}
