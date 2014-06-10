/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL ChannelMgr :  public Singleton < ChannelMgr >
{
public:
    ChannelMgr();
    ~ChannelMgr();

    Channel *GetCreateDBCChannel(const char *name, Player* p, uint32 type_id);
    Channel *GetCreateChannel(const char *name, Player* p);
    Channel *GetChannel(const char *name, Player* p, bool requiresIn = false);
    Channel *GetChannel(const char * name, uint32 team);
    Channel *GetChannel(uint32 id);
    void RemoveChannel(Channel * chn);
    bool seperatechannels;

    void BroadcastToDBCChannels(uint32 dbc_id, Player* plr, const char * message);

private:
    //team 0: aliance, team 1 horde
    typedef std::map<std::string,Channel *> ChannelList;
    typedef std::multimap<uint32, std::pair<std::string, Channel*> > DBCChannelMap;
    typedef std::pair<DBCChannelMap::iterator, DBCChannelMap::iterator> DBCChannelBounds;
    typedef HM_NAMESPACE::hash_map<uint32, Channel*> ChannelMap;
    ChannelMap m_idToChannel;
    uint32 m_idHigh, m_dbcidHigh;
    ChannelList Channels[2];
    DBCChannelMap DBCChannels[2];
    Mutex lock;
};

#define channelmgr ChannelMgr::getSingleton()
