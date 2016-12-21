/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
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
    typedef std::map<uint32, Channel*> ChannelMap;
    ChannelMap m_idToChannel;
    uint32 m_idHigh, m_dbcidHigh;
    ChannelList Channels[2];
    DBCChannelMap DBCChannels[2];
    Mutex lock;
};

#define channelmgr ChannelMgr::getSingleton()
