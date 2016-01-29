/***
 * Demonstrike Core
 */

#include "Stdafx.h"

createFileSingleton(PathMgr);

PathMgr::PathMgr() : Singleton<PathMgr>()
{

}

PathMgr::~PathMgr()
{

}

void PathMgr::Initialize()
{

}

void PathMgr::LoadFromDB()
{

}

void PathMgr::Update(uint32 mapId, uint32 diff)
{
    if(m_creaturePaths.find(mapId) == m_creaturePaths.end())
        return;
    CreaturePathStorage *storage = m_creaturePaths.at(mapId);
    for(auto itr = storage->m_creaturePaths.begin(); itr != storage->m_creaturePaths.end(); itr++)
        itr->second->_UpdateTimer(diff);
}

CreaturePath::CreaturePath() : m_pathEnabled(false), m_timer(0), m_pathLength(0)
{

}

CreaturePath::~CreaturePath()
{

}

bool CreaturePath::Load(Field *fields)
{

    return true;
}

void CreaturePath::Update(Creature *ctr)
{

}

void CreaturePath::UpdateLoc(LocationVector *vector)
{
    //vector->ChangeCoords(0.f, 0.f, 0.f);

}

void CreaturePath::_UpdateTimer(uint32 diff)
{   // Only update and reprocess path
    if(m_pathEnabled == false || m_pathLength == 0)
        return;

    if((m_timer += diff) >= m_pathLength)
        m_timer %= m_pathLength;

}