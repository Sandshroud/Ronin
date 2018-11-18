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
    TransportStatus(uint32 transportEntry);
    ~TransportStatus();

    void Initialize(uint32 mapId);
    void Update();

    void QueueMapTransfer(uint32 oldMap, uint32 newMapId);

    uint32 getMapId() { Guard guard(dataLock); return _mapId; }

private:
    int32 _mapId, _pendingMapId;
    uint32 transportEntry;

    Mutex dataLock;
};

class TransportMgr : public Singleton<TransportMgr>
{
private:
    struct TransportData
    {
        int32 mapIds[2];
        TaxiPath *transportPath;
        std::map<size_t, uint32> pathPointTimes[2];
        std::map<uint32, uint32> taxiDelayTimers[2];

        uint32 timerStart[2], pathTravelTime[2];
        uint32 calculatedPathTimer;
        GameObjectInfo *transportTemplate;
    };

public:
    TransportMgr();
    ~TransportMgr();

    void LoadTransportData();
    void ProcessTransports(uint32 uiDiff);

    void PreloadMapInstance(uint32 msTime, MapInstance *instance, uint32 mapId);
    bool RegisterTransport(GameObject *gobj, uint32 mapId, GameObject::TransportTaxiData *dataOut);

    void ChangeTransportActiveMap(WoWGuid transport, uint32 oldMapId);
    bool CheckTransportPosition(WoWGuid transport, uint32 mapId);
    void UpdateTransportData(Player *plr);
    void ClearPlayerData(Player *plr);

protected:
    void _CreateTransportData(GameObjectInfo *info, TaxiPath *path);

    std::map<uint32, TransportData*> m_transportDataStorage;

    Mutex m_transportStatusLock;
    uint32 m_transportUpdateTimer;
    std::map<WoWGuid, TransportStatus*> m_transportStatusStorage;
};

#define sTransportMgr TransportMgr::getSingleton()
