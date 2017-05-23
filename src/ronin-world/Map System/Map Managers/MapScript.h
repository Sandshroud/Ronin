/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2017 Sandshroud <https://github.com/Sandshroud>
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

class MapInstance;

// Map script class, purely virtual
class SERVER_DECL MapScript
{
public:
    MapScript(MapInstance *instance) : _instance(instance) {}
    ~MapScript() { _instance = NULL; }

    // Update function called from inside map manager
    virtual void Update(uint32 msTime, uint32 uiDiff) = 0;

    // On push object to inworld status
    virtual void OnPushObject(WorldObject *obj) {}
    // On remove object from inworld status
    virtual void OnRemoveObject(WorldObject *obj) {}

private:
    MapInstance *_instance;

};

// Tie our map script to a map script allocator that we pass
// into instance managment script registers
class MapScriptAllocator
{
public:
    virtual MapScript *Allocate(MapInstance *instance) = 0;

    // Optional extra instance data table creation call
    virtual void CheckInstanceDataTables() {}

    virtual void LoadExtraInstanceData(uint32 instanceId) {}
};
