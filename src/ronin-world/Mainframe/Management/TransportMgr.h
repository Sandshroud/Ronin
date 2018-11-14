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

class TransportStatus
{
public:
    TransportStatus();
    ~TransportStatus();

    void Update(uint32 msTime, uint32 uiDiff);

    uint32 getMapId() { return _mapId; }

private:
    uint32 _mapId;

};

class TransportMgr : public Singleton<TransportMgr>
{
private:
    struct TransportData
    {
        int32 mapIds[2];
        TaxiPath *transportPath;

        GameObjectInfo *transportTemplate;
    };

public:
    TransportMgr();
    ~TransportMgr();

    void LoadTransportData();
    void ProcessTransports(uint32 msTime);

    void PreloadMapInstance(uint32 msTime, MapInstance *instance, uint32 mapId);
    bool RegisterTransport(GameObject *gobj, uint32 mapId);
    void ProcessingPendingEvents(MapInstance *instance);

    bool CheckTransportPosition(WoWGuid transport, uint32 mapId);
    void UpdateTransportData(Player *plr);
    void ClearPlayerData(Player *plr);

protected:
    void _CreateTransportData(GameObjectInfo *info, TaxiPath *path);

    std::map<uint32, TransportData*> m_transportDataStorage;

    std::map<WoWGuid, TransportStatus*> m_transportStatusStorage;
};

#define sTransportMgr TransportMgr::getSingleton()
