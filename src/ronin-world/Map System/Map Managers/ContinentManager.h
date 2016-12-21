/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
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

class SERVER_DECL ContinentManager : public ThreadContext
{
    friend class MapInstance;
public:
    ContinentManager(MapEntry *mapEntry, Map *map);
    ~ContinentManager();

    bool Initialize();

    bool run();

    Map *GetMapData() { return m_mapData; }
    MapInstance *GetContinent() { return m_continent; }

private:
    uint32 m_mapId;

    // Map data for terrain, datacells etc
    Map *m_mapData;

    // Map Entry data pointer
    MapEntry *m_mapEntry;

    // Our continent instance
    MapInstance *m_continent;
};
