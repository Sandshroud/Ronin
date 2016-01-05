/***
 * Demonstrike Core
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
