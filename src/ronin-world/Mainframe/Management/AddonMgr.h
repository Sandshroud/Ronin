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

struct AddonEntry
{
    std::string name;
    uint32 crc;
    bool banned;
    bool isNew;
    bool showinlist;
};

class AddonMgr :  public Singleton < AddonMgr >
{
public:
    AddonMgr();
    ~AddonMgr();

    void LoadFromDB();
    void SaveToDB();

    void SendAddonInfoPacket(WorldPacket *source, WorldSession *m_session);

private:
    std::map<std::string, AddonEntry*> KnownAddons;

    bool IsAddonBanned(uint32 crc, std::string name = "");
    bool IsAddonBanned(std::string name, uint32 crc = 0);
    bool ShouldShowInList(std::string name, uint32 crc);
};

#define sAddonMgr AddonMgr::getSingleton()
