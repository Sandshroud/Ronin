/***
 * Demonstrike Core
 */

#pragma once

/*
 * World State Manager Class
 * See doc/World State Manager.txt for implementation details
 */

// forward declaration for mapmgr
class MapMgr;

// some defines
#define FACTION_MASK_ALL -1
#define ZONE_MASK_ALL -1

class SERVER_DECL WorldStateManager
{
    struct WorldState
    {
        //uint32 StateId;       // index in map
        int32 FactionMask;
        int32 ZoneMask;
        uint32 Value;
    };

    typedef map<uint32, WorldState> WorldStateMap;

    // storing world state variables
    WorldStateMap m_states;

    // mapmgr we are working with.
    MapMgr* m_mapMgr;

    // synchronization object
    // shouldn't REALLY be needed, but we're paranoid..
    //Mutex m_lock;

    // public methods
public:

    // constructor, not much to do though, except set mapmgr reference
    WorldStateManager(MapMgr* mgr)
    {
        m_mapMgr = mgr;
    }

    // bhoom! all cleaned up by C++ automatically
    ~WorldStateManager() {}

    // initializes a world state.
    // use only at map creation.
    // it will do error checking and re-initialize existing elements, however it is a bad practice!
    // creating a world state will not send it to the player! this is because all world state creation should be done
    // at instance creation, not in the middle of it running!
    void CreateWorldState(uint32 uWorldStateId, uint32 uInitialValue, int32 iFactionMask = FACTION_MASK_ALL, int32 iZoneMask = ZONE_MASK_ALL);

    // updates a world state (sets and stores value, updates in clients on map)
    // NOTE: If the world state is not created/initialized, it WILL NOT BE UPDATED/SENT to the client!!!!
    void UpdateWorldState(uint32 uWorldStateId, uint32 uValue);

    // sends the current world states to a new player on the map.
    // this should also be called upon changing zone.
    void SendWorldStates(Player* pPlayer);

    // clears world states for a player leaving the map.
    void ClearWorldStates(Player* pPlayer);

    // loads a setting from the database.
    static int32 GetPersistantSetting(uint32 keyVal, int32 defaultReturn);
    static void SetPersistantSetting(uint32 keyVal, int32 Value);
};

struct WorldStateTemplate
{
    uint32 uField;
    int32 iFactionMask;
    int32 iZoneMask;
    int32 uValue;
};

class WorldStateTemplateManager : public Singleton<WorldStateTemplateManager>
{
    typedef list<WorldStateTemplate> WorldStateTemplateList;
    WorldStateTemplateList m_templatesForMaps[NUM_MAPS];
    WorldStateTemplateList m_general;
public:
    // loads predefined fields from database
    void LoadFromDB(int32 mapid = -1);

    // applys a map template to a new instance
    void ApplyMapTemplate(MapMgr* pmgr);
};

#define sWorldStateTemplateManager WorldStateTemplateManager::getSingleton()
