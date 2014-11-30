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
const char * gAreaTriggerFormat                         = "ucuusffffuu";
const char * gCreatureVehicleData                      = "ubuuuuuuuuubbubbubbubbubbubbubbubb";
const char * gCreatureInfoExtra                         = "uuuhubbfbfuisbb";
const char * gFishingFormat                             = "uuu";
const char * gGameObjectNameFormat                      = "uuusssiuuuuuuuuuuuuuuuuuuuuuuuuu";
const char * gGraveyardFormat                           = "uffffuuuux";
const char * gItemPageFormat                            = "usu";
const char * gNpcTextFormat                             = "ussssssssssssssssfuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuufuuuuuuu";
const char * gTeleportCoordFormat                       = "uxuffff";
const char * gWorldMapInfoFormat                        = "usbuuuufffiuuuuuuuu";
const char * gRandomItemCreationFormat                  = "uuuu";
const char * gRandomCardCreationFormat                  = "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";
const char * gScrollCreationFormat                      = "uu";
const char * gZoneGuardsFormat                          = "uuu";

/** SQLStorage symbols
 */
SERVER_DECL SQLStorage<AchievementReward, HashMapStorageContainer<AchievementReward> >          AchievementRewardStorage;
SERVER_DECL SQLStorage<AreaTrigger, HashMapStorageContainer<AreaTrigger> >                      AreaTriggerStorage;
SERVER_DECL SQLStorage<CreatureVehicleData, HashMapStorageContainer<CreatureVehicleData> >      CreatureVehicleDataStorage;
SERVER_DECL SQLStorage<CreatureInfoExtra, HashMapStorageContainer<CreatureInfoExtra> >          CreatureInfoExtraStorage;
SERVER_DECL SQLStorage<FishingZoneEntry, HashMapStorageContainer<FishingZoneEntry> >            FishingZoneStorage;
SERVER_DECL SQLStorage<GameObjectInfo, HashMapStorageContainer<GameObjectInfo> >                GameObjectNameStorage;
SERVER_DECL SQLStorage<GraveyardTeleport, HashMapStorageContainer<GraveyardTeleport> >          GraveyardStorage;
SERVER_DECL SQLStorage<ItemPage, HashMapStorageContainer<ItemPage> >                            ItemPageStorage;
SERVER_DECL SQLStorage<GossipText, HashMapStorageContainer<GossipText> >                        NpcTextStorage;
SERVER_DECL SQLStorage<TeleportCoords, HashMapStorageContainer<TeleportCoords> >                TeleportCoordStorage;
SERVER_DECL SQLStorage<MapInfo, ArrayStorageContainer<MapInfo> >                                WorldMapInfoStorage;
SERVER_DECL SQLStorage<MapInfo, ArrayStorageContainer<MapInfo> >                                LimitedMapInfoStorage;
SERVER_DECL SQLStorage<ZoneGuardEntry, HashMapStorageContainer<ZoneGuardEntry> >                ZoneGuardStorage;
SERVER_DECL SQLStorage<RandomItemCreation, HashMapStorageContainer<RandomItemCreation> >        RandomItemCreationStorage;
SERVER_DECL SQLStorage<RandomCardCreation, HashMapStorageContainer<RandomCardCreation> >        RandomCardCreationStorage;
SERVER_DECL SQLStorage<ScrollCreation, HashMapStorageContainer<ScrollCreation> >                ScrollCreationStorage;

SERVER_DECL std::set<std::string> ExtraMapCreatureTables;
SERVER_DECL std::set<std::string> ExtraMapGameObjectTables;

void ObjectMgr::LoadProfessionDiscoveries()
{
    QueryResult * result = WorldDatabase.Query("SELECT * from professiondiscoveries");
    if ( result != NULL )
    {
        do
        {
            Field *f = result->Fetch();
            ProfessionDiscovery * pf = new ProfessionDiscovery;
            pf->SpellId = f[0].GetUInt32();
            pf->SpellToDiscover = f[1].GetUInt32();
            pf->SkillValue = f[2].GetUInt32();
            pf->Chance = f[3].GetFloat();
            ProfessionDiscoveryTable.insert( pf );
        }
        while( result->NextRow() );
    }
    delete result;
}

void ObjectMgr::LoadExtraItemStuff()
{
    std::map<uint32,uint32> foodItems;
    QueryResult * result = WorldDatabase.Query("SELECT * FROM itempetfood ORDER BY entry");
    if(result)
    {
        do
        {
            Field *f = result->Fetch();
            foodItems.insert( std::make_pair( f[0].GetUInt32(), f[1].GetUInt32() ) );
        }while(result->NextRow());
        delete result;
    }

    for(ConstructDBCStorageIterator(ItemSetEntry) itr = dbcItemSet.begin(); itr != dbcItemSet.end(); ++itr)
    {
        ItemSetEntry *itemset = (*itr);
        for(uint8 i = 0; i < 10; i++)
        {
            if(itemset->itemid[i])
            {
                ItemsInSets.insert(std::make_pair(itemset->itemid[i], itemset->id));
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
                                IP = ItemPrototypeStorage.LookupEntry(*itr2);
                                size_t spot = 0;
                                std::string name = IP->Name1, Prefix = "";
                                if((spot = name.find(" ")) != std::string::npos)
                                    Prefix = name.substr(0, spot);
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
                        IP = ItemPrototypeStorage.LookupEntry(*itr2);
                        size_t spot = 0;
                        std::string name = IP->Name1, Prefix = "";
                        if((spot = name.find(" ")) != std::string::npos)
                            Prefix = name.substr(0, spot);
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
                        }
                        else
                            rankByPrefix.insert(std::make_pair(Prefix, rank));

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

    foodItems.clear();
}

#define make_task(storage, itype, storagetype, tablename, format) tl.AddTask( new Task( \
    new CallbackP2< /**/SQLStorage< itype, storagetype< itype > >/**/, /**/const char */**/, /**/const char */**/> \
    (&storage, &SQLStorage< itype, storagetype< itype > >::Load, tablename, format) ) )

#define make_task2(storage, itype, storagetype, tablename, format) tl.AddTask( new Task( \
    new CallbackP2< /**/SQLStorage< itype, storagetype< itype > >/**/, /**/const char */**/, /**/const char */**/> \
    (&storage, &SQLStorage< itype, storagetype< itype > >::LoadWithLoadColumn, tablename, format) ) )

void Storage_FillTaskList(TaskList & tl)
{
    make_task(CreatureInfoExtraStorage, CreatureInfoExtra, HashMapStorageContainer, "creature_info", gCreatureInfoExtra);
    make_task(GameObjectNameStorage, GameObjectInfo, HashMapStorageContainer, "gameobject_names", gGameObjectNameFormat);
    make_task(CreatureVehicleDataStorage, CreatureVehicleData, HashMapStorageContainer, "creature_proto_vehicle", gCreatureVehicleData);
    make_task(AreaTriggerStorage, AreaTrigger, HashMapStorageContainer, "areatriggers", gAreaTriggerFormat);
    make_task(ItemPageStorage, ItemPage, HashMapStorageContainer, "itempages", gItemPageFormat);
    make_task(GraveyardStorage, GraveyardTeleport, HashMapStorageContainer, "graveyards", gGraveyardFormat);
    make_task(TeleportCoordStorage, TeleportCoords, HashMapStorageContainer, "teleport_coords", gTeleportCoordFormat);
    make_task(FishingZoneStorage, FishingZoneEntry, HashMapStorageContainer, "fishing", gFishingFormat);
    make_task(NpcTextStorage, GossipText, HashMapStorageContainer, "npc_text", gNpcTextFormat);
    make_task(WorldMapInfoStorage, MapInfo, ArrayStorageContainer, "worldmap_info", gWorldMapInfoFormat);
    make_task2(LimitedMapInfoStorage, MapInfo, ArrayStorageContainer, "worldmap_info", gWorldMapInfoFormat);
    make_task(ZoneGuardStorage, ZoneGuardEntry, HashMapStorageContainer, "zoneguards", gZoneGuardsFormat);
    make_task(AchievementRewardStorage, AchievementReward, HashMapStorageContainer, "achievement_rewards", gAchievementRewardFormat);
    make_task(RandomItemCreationStorage, RandomItemCreation, HashMapStorageContainer, "randomitemcreation", gRandomItemCreationFormat);
    make_task(RandomCardCreationStorage, RandomCardCreation, HashMapStorageContainer, "randomcardcreation", gRandomCardCreationFormat);
    make_task(ScrollCreationStorage, ScrollCreation, HashMapStorageContainer, "scrollcreation", gScrollCreationFormat);
}

void Storage_Cleanup()
{
    GameObjectNameStorage.Cleanup();
    CreatureVehicleDataStorage.Cleanup();
    CreatureInfoExtraStorage.Cleanup();

    StorageContainerIterator<AreaTrigger> * ATitr = AreaTriggerStorage.MakeIterator();
    AreaTrigger * a;
    while(!ATitr->AtEnd())
    {
        a = ATitr->Get();

        if (a->Name)
        {
            free(a->Name);
            a->Name = NULL;
        }

        if(!ATitr->Inc())
            break;
    }
    ATitr->Destruct();

    AreaTriggerStorage.Cleanup();

    StorageContainerIterator<ItemPage> * IPitr = ItemPageStorage.MakeIterator();
    ItemPage * i;
    while(!IPitr->AtEnd())
    {
        i = IPitr->Get();

        if (i->text)
        {
            free(i->text);
            i->text = NULL;
        }

        if(!IPitr->Inc())
            break;
    }
    IPitr->Destruct();

    ItemPageStorage.Cleanup();
    RandomItemCreationStorage.Cleanup();
    RandomCardCreationStorage.Cleanup();
    ScrollCreationStorage.Cleanup();
    GraveyardStorage.Cleanup();
    TeleportCoordStorage.Cleanup();
    FishingZoneStorage.Cleanup();
    NpcTextStorage.Cleanup();

    StorageContainerIterator<MapInfo> * MIitr = WorldMapInfoStorage.MakeIterator();
    MapInfo * m;
    while(!MIitr->AtEnd())
    {
        m = MIitr->Get();

        if (m->name)
        {
            free(m->name);
            m->name = NULL;
        }

        if(!MIitr->Inc())
            break;
    }

    MIitr = LimitedMapInfoStorage.MakeIterator();
    while(!MIitr->AtEnd())
    {
        m = MIitr->Get();

        if (m->name)
        {
            free(m->name);
            m->name = NULL;
        }

        if(!MIitr->Inc())
            break;
    }
    MIitr->Destruct();

    WorldMapInfoStorage.Cleanup();
    LimitedMapInfoStorage.Cleanup();
    ZoneGuardStorage.Cleanup();
}

std::vector<std::pair<std::string,std::string> > additionalTables;

bool LoadAdditionalTable(const char * TableName, const char * SecondName)
{
    if(!stricmp(TableName, "creature_spawns"))
    {
        ExtraMapCreatureTables.insert(std::string(SecondName));
        return false;
    }
    else if(!stricmp(TableName, "gameobject_spawns"))
    {
        ExtraMapGameObjectTables.insert(std::string(SecondName));
        return false;
    }
    else if(!stricmp(TableName, "gameobject_names"))    // GO Names
        GameObjectNameStorage.LoadAdditionalData(SecondName, gGameObjectNameFormat);
    else if(!stricmp(TableName, "areatriggers"))        // Areatriggers
        AreaTriggerStorage.LoadAdditionalData(SecondName, gAreaTriggerFormat);
    else if(!stricmp(TableName, "itempages"))           // Item Pages
        ItemPageStorage.LoadAdditionalData(SecondName, gItemPageFormat);
    else if(!stricmp(TableName, "npc_text"))            // NPC Text Storage
        NpcTextStorage.LoadAdditionalData(SecondName, gNpcTextFormat);
    else if(!stricmp(TableName, "fishing"))             // Fishing Zones
        FishingZoneStorage.LoadAdditionalData(SecondName, gFishingFormat);
    else if(!stricmp(TableName, "teleport_coords"))     // Teleport coords
        TeleportCoordStorage.LoadAdditionalData(SecondName, gTeleportCoordFormat);
    else if(!stricmp(TableName, "graveyards"))          // Graveyards
        GraveyardStorage.LoadAdditionalData(SecondName, gGraveyardFormat);
    else if(!stricmp(TableName, "worldmap_info"))       // WorldMapInfo
    {
        LimitedMapInfoStorage.LoadAdditionalData(SecondName, gWorldMapInfoFormat);
        WorldMapInfoStorage.LoadAdditionalData(SecondName, gWorldMapInfoFormat);
    }
    else if(!stricmp(TableName, "zoneguards"))
        ZoneGuardStorage.LoadAdditionalData(SecondName, gZoneGuardsFormat);
    else
        return false;

    return true;
}

bool Storage_ReloadTable(const char * TableName)
{
    // bur: mah god this is ugly :P
    if(!stricmp(TableName, "creature_proto_vehicle"))  // Creature Vehicle Proto
        CreatureVehicleDataStorage.Reload();
    else if(!stricmp(TableName, "gameobject_names"))    // GO Names
        GameObjectNameStorage.Reload();
    else if(!stricmp(TableName, "areatriggers"))        // Areatriggers
        AreaTriggerStorage.Reload();
    else if(!stricmp(TableName, "itempages"))           // Item Pages
        ItemPageStorage.Reload();
    else if(!stricmp(TableName, "quests"))              // Quests
        sQuestMgr.LoadQuests();
    else if(!stricmp(TableName, "npc_text"))            // NPC Text Storage
        NpcTextStorage.Reload();
    else if(!stricmp(TableName, "fishing"))             // Fishing Zones
        FishingZoneStorage.Reload();
    else if(!stricmp(TableName, "teleport_coords"))     // Teleport coords
        TeleportCoordStorage.Reload();
    else if(!stricmp(TableName, "graveyards"))          // Graveyards
        GraveyardStorage.Reload();
    else if(!stricmp(TableName, "worldmap_info"))       // WorldMapInfo
    {
        LimitedMapInfoStorage.Reload();
        WorldMapInfoStorage.Reload();
    }
    else if(!stricmp(TableName, "zoneguards"))
        ZoneGuardStorage.Reload();
    else if(!stricmp(TableName, "spell_disable"))
        objmgr.ReloadDisabledSpells();
    else if(!stricmp(TableName, "spellfixes"))
        objmgr.LoadSpellFixes();
    else if(!stricmp(TableName, "vendors"))
        objmgr.ReloadVendors();
    else if(!stricmp(TableName, "command_overrides"))   // Command Overrides
    {
        sComTableStore.Dealloc();
        sComTableStore.Init();
        sComTableStore.Load();
    }
    else
        return false;

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
    ExtraMapCreatureTables.insert(std::string("creature_spawns"));
    ExtraMapGameObjectTables.insert(std::string("gameobject_spawns"));

    std::string strData = mainIni->ReadString("Startup", "LoadAdditionalTables", "");
    if(strData.empty())
        return;

    std::vector<std::string> strs = RONIN_UTIL::StrSplit(strData, ",");
    if(strs.empty())
        return;

    for(std::vector<std::string>::iterator itr = strs.begin(); itr != strs.end(); itr++)
    {
        char s1[200], s2[200];
        if(sscanf((*itr).c_str(), "%s %s", s1, s2) != 2)
            continue;

        if(LoadAdditionalTable(s2, s1))
        {
            std::pair<std::string,std::string> tmppair;
            tmppair.first = std::string(s1);
            tmppair.second = std::string(s2);
            additionalTables.push_back(tmppair);
        }
    }
}
