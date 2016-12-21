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

extern SERVER_DECL SQLStorage<GameObjectInfo, HashMapStorageContainer<GameObjectInfo> >             GameObjectNameStorage;
extern SERVER_DECL SQLStorage<CreatureVehicleData, HashMapStorageContainer<CreatureVehicleData> >   CreatureVehicleDataStorage;
extern SERVER_DECL SQLStorage<CreatureInfoExtra, HashMapStorageContainer<CreatureInfoExtra> >       CreatureInfoExtraStorage;
extern SERVER_DECL SQLStorage<ItemPage, HashMapStorageContainer<ItemPage> >                         ItemPageStorage;
extern SERVER_DECL SQLStorage<GossipText, HashMapStorageContainer<GossipText> >                     NpcTextStorage;
extern SERVER_DECL SQLStorage<GraveyardTeleport, HashMapStorageContainer<GraveyardTeleport> >       GraveyardStorage;
extern SERVER_DECL SQLStorage<TeleportCoords, HashMapStorageContainer<TeleportCoords> >             TeleportCoordStorage;

void Storage_FillTaskList(TaskList & tl);
void Storage_Cleanup();
bool Storage_ReloadTable(const char * TableName);
void Storage_LoadAdditionalTables();

extern SERVER_DECL std::set<std::string> ExtraMapCreatureTables;
extern SERVER_DECL std::set<std::string> ExtraMapGameObjectTables;
