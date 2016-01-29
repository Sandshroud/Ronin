/***
 * Demonstrike Core
 */

#pragma once

class CreaturePath;

struct CreaturePathStorage
{
    std::map<uint32, CreaturePath*> m_creaturePaths;
};

class SERVER_DECL PathMgr : public Singleton<PathMgr>
{
public:
    PathMgr();
    ~PathMgr();

    void Initialize();
    void LoadFromDB();

    void Update(uint32 mapId, uint32 diff);

private:
    std::map<uint32, CreaturePathStorage*> m_creaturePaths;
};

class SERVER_DECL CreaturePath
{
    friend class PathMgr;

public:
    CreaturePath();
    ~CreaturePath();

    bool Load(Field *fields);

    void Update(Creature *ctr);
    void UpdateLoc(LocationVector *vector);

    // Disable path processing
    void DisablePath() { m_pathEnabled = false; }

private:
    void _UpdateTimer(uint32 diff);

    uint32 m_timer, m_pathLength;

    bool m_pathEnabled;


    std::vector<Position*> m_pathPositions;
};

#define sPathMgr PathMgr::getSingleton()
