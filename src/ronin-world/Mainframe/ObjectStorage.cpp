/***
 * Demonstrike Core
 */

#include "StdAfx.h"

/** Table formats converted to strings
b = bool
c = uint8
h = uint16
u = uint32
i = int32
f = float
s = std::string
x = skip
*/
const char * gAchievementRewardFormat                   = "uuuubuss";
const char * gCreatureVehicleData                       = "ubuuuuuuuuubbubbubbubbubbubbubbubb";
const char * gCreatureInfoExtra                         = "uuuhubbfbfuisbb";
const char * gGameObjectNameFormat                      = "uuusssfuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuiu";
const char * gGraveyardFormat                           = "uffffuuuux";
const char * gItemPageFormat                            = "usu";
const char * gNpcTextFormat                             = "ussssssssssssssssfuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuu";
const char * gTeleportCoordFormat                       = "uxuffff";
const char * gWorldMapInfoFormat                        = "usXuuuufffiuuuuuuuu";

/** SQLStorage symbols
 */
SERVER_DECL SQLStorage<CreatureVehicleData, HashMapStorageContainer<CreatureVehicleData> >      CreatureVehicleDataStorage;
SERVER_DECL SQLStorage<CreatureInfoExtra, HashMapStorageContainer<CreatureInfoExtra> >          CreatureInfoExtraStorage;
SERVER_DECL SQLStorage<GameObjectInfo, HashMapStorageContainer<GameObjectInfo> >                GameObjectNameStorage;
SERVER_DECL SQLStorage<GraveyardTeleport, HashMapStorageContainer<GraveyardTeleport> >          GraveyardStorage;
SERVER_DECL SQLStorage<ItemPage, HashMapStorageContainer<ItemPage> >                            ItemPageStorage;
SERVER_DECL SQLStorage<GossipText, HashMapStorageContainer<GossipText> >                        NpcTextStorage;
SERVER_DECL SQLStorage<TeleportCoords, HashMapStorageContainer<TeleportCoords> >                TeleportCoordStorage;

SERVER_DECL std::set<std::string> ExtraMapCreatureTables;
SERVER_DECL std::set<std::string> ExtraMapGameObjectTables;

void ObjectMgr::LoadExtraItemStuff()
{
    for(uint32 i = 0; i < dbcItemSet.GetNumRows(); i++)
    {
        ItemSetEntry *itemset = dbcItemSet.LookupRow(i);
        for(uint8 i = 0; i < 9; i++)
        {
            if(itemset->setItems[i])
            {
                ItemsInSets.insert(std::make_pair(itemset->setItems[i], itemset->id));
            }
        }
    }

    if(mItemSets.size())
    {
        std::map<uint32, int32> ItemSetsHighest;
        std::map<uint32, std::map<uint32, std::set<uint32> > > RankByLevelInSet;
        for(ItemSetContentMap::iterator itr = mItemSets.begin(); itr != mItemSets.end(); itr++)
        {
            for(std::list<ItemPrototype*>::iterator itr2 = itr->second->begin(); itr2 != itr->second->end(); itr2++)
            {
                if(ItemSetsHighest.find(itr->first) == ItemSetsHighest.end() || (*itr2)->ItemLevel > ItemSetsHighest[itr->first])
                    ItemSetsHighest[itr->first] = (*itr2)->ItemLevel;

                if(RankByLevelInSet.find(itr->first) == RankByLevelInSet.end())
                {
                    std::set<uint32> pie;
                    pie.insert((*itr2)->ItemId);
                    std::map<uint32, std::set<uint32> > mapset;
                    mapset.insert(std::make_pair((*itr2)->ItemLevel, pie));

                    RankByLevelInSet.insert(std::make_pair(itr->first, mapset));
                }
                else
                {
                    if(RankByLevelInSet[itr->first].find((*itr2)->ItemLevel) == RankByLevelInSet[itr->first].end())
                    {
                        std::set<uint32> pie;
                        pie.insert((*itr2)->ItemId);
                        RankByLevelInSet[itr->first].insert(std::make_pair((*itr2)->ItemLevel, pie));
                    } else RankByLevelInSet[itr->first][(*itr2)->ItemLevel].insert((*itr2)->ItemId);
                }
            }
        }

        for(std::map<uint32, std::map<uint32, std::set<uint32> > >::iterator itr = RankByLevelInSet.begin(); itr != RankByLevelInSet.end(); itr++)
        {
            uint32 rank = 1;
            uint32 lastRank = 0;
            uint32 oldRanking = 0;
            std::string lastPrefix = "";
            std::set<uint32> UsedRanks;
            ItemPrototype *IP = NULL;
            std::map<std::string, uint32> rankByPrefix;
            for(int32 i = 0; i <= ItemSetsHighest[itr->first]; i++)
            {
                if(oldRanking > 0)
                {
                    rank = oldRanking;
                    oldRanking = 0;
                }

                if(itr->second.find(i) != itr->second.end())
                {
                    if(lastRank)
                    {
                        if(i-lastRank < 13)
                        {
                            if(lastPrefix.length())
                            {
                                std::set<uint32>::iterator itr2 = itr->second[i].begin();
                                IP = sItemMgr.LookupEntry(*itr2);
                                size_t spot = 0;
                                std::string Prefix = "";
                                if((spot = IP->Name.find(" ")) != std::string::npos)
                                    Prefix = IP->Name.substr(0, spot);
                                if(strcmp(lastPrefix.c_str(), Prefix.c_str()))
                                {
                                    while(UsedRanks.find(rank) != UsedRanks.end())
                                        rank++;
                                }
                                IP = NULL;
                            }
                        }
                        else
                        {
                            while(UsedRanks.find(rank) != UsedRanks.end())
                                rank++;
                        }
                    }

                    for(std::set<uint32>::iterator itr2 = itr->second[i].begin(); itr2 != itr->second[i].end(); itr2++)
                    {
                        IP = sItemMgr.LookupEntry(*itr2);
                        size_t spot = 0;
                        std::string Prefix = "";
                        if((spot = IP->Name.find(" ")) != std::string::npos)
                            Prefix = IP->Name.substr(0, spot);
                        if(strcmp(lastPrefix.c_str(), Prefix.c_str()))
                        {
                            while(UsedRanks.find(rank) != UsedRanks.end())
                                rank++;
                        }

                        lastPrefix = Prefix;
                        if(rankByPrefix.find(Prefix) != rankByPrefix.end())
                        {
                            if(!oldRanking)
                                oldRanking = rank;
                            rank = rankByPrefix.at(Prefix);
                        } else rankByPrefix.insert(std::make_pair(Prefix, rank));

                        IP->ItemSetRank = rank;
                        if(UsedRanks.find(rank) == UsedRanks.end())
                            UsedRanks.insert(rank);
                        IP = NULL;
                    }
                    lastRank = i;
                }
                rankByPrefix.clear();
            }
        }
    }
}

#define make_task(storage, itype, storagetype, tablename, format) tl.AddTask( new Task( \
    new CallbackP2< /**/SQLStorage< itype, storagetype< itype > >/**/, /**/ std::string/**/, /**/std::string/**/> \
    (&storage, &SQLStorage< itype, storagetype< itype > >::Load, tablename, format) ) )

void Storage_FillTaskList(TaskList & tl)
{
    make_task(CreatureInfoExtraStorage, CreatureInfoExtra, HashMapStorageContainer, "creature_info", gCreatureInfoExtra);
    make_task(GameObjectNameStorage, GameObjectInfo, HashMapStorageContainer, "gameobject_names", gGameObjectNameFormat);
    make_task(CreatureVehicleDataStorage, CreatureVehicleData, HashMapStorageContainer, "creature_proto_vehicle", gCreatureVehicleData);
    make_task(ItemPageStorage, ItemPage, HashMapStorageContainer, "item_pages", gItemPageFormat);
    make_task(GraveyardStorage, GraveyardTeleport, HashMapStorageContainer, "graveyards", gGraveyardFormat);
    make_task(TeleportCoordStorage, TeleportCoords, HashMapStorageContainer, "teleport_coords", gTeleportCoordFormat);
    make_task(NpcTextStorage, GossipText, HashMapStorageContainer, "npc_text", gNpcTextFormat);
}

void Storage_Cleanup()
{
    GameObjectNameStorage.Cleanup();
    CreatureVehicleDataStorage.Cleanup();
    CreatureInfoExtraStorage.Cleanup();
    ItemPageStorage.Cleanup();
    GraveyardStorage.Cleanup();
    TeleportCoordStorage.Cleanup();
    NpcTextStorage.Cleanup();
}

std::vector<std::pair<std::string,std::string> > additionalTables;

bool LoadAdditionalTable(const char * TableName, const char * SecondName)
{
    return true;
}

bool Storage_ReloadTable(const char * TableName)
{
    // bur: mah god this is ugly :P
    if(!stricmp(TableName, "creature_proto_vehicle"))  // Creature Vehicle Proto
        CreatureVehicleDataStorage.Reload();
    else if(!stricmp(TableName, "gameobject_names"))    // GO Names
        GameObjectNameStorage.Reload();
    else if(!stricmp(TableName, "item_pages"))           // Item Pages
        ItemPageStorage.Reload();
    else if(!stricmp(TableName, "quests"))              // Quests
        sQuestMgr.LoadQuests();
    else if(!stricmp(TableName, "npc_text"))            // NPC Text Storage
        NpcTextStorage.Reload();
    else if(!stricmp(TableName, "teleport_coords"))     // Teleport coords
        TeleportCoordStorage.Reload();
    else if(!stricmp(TableName, "graveyards"))          // Graveyards
        GraveyardStorage.Reload();
    else if(!stricmp(TableName, "vendors"))
        objmgr.ReloadVendors();
    else if(!stricmp(TableName, "command_overrides"))   // Command Overrides
    {
        sComTableStore.Dealloc();
        sComTableStore.Init();
    } else return false;

    uint32 len = (uint32)strlen(TableName);
    uint32 len2;
    for(std::vector<std::pair<std::string,std::string> >::iterator itr = additionalTables.begin(); itr != additionalTables.end(); itr++)
    {
        len2=(uint32)itr->second.length();
        if(!strnicmp(TableName, itr->second.c_str(), std::min(len,len2)))
            LoadAdditionalTable(TableName, itr->first.c_str());
    }
    return true;
}

void Storage_LoadAdditionalTables()
{

}
