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
s = string
x = skip
*/
const char * gAchievementRewardFormat                   = "uuuubuss";
const char * gAreaTriggerFormat                         = "ucuusffffuu";
const char * gCreatureNameFormat                        = "usssuuuuuuuuuuuffcc";
const char * gCreatureProtoFormat                       = "uuuuuucuufuuuffuffuuuuuuuuuuuffsuiufffuuiuuuuuuuuuu";
const char * gCreatureVehicleProto                      = "ubuuuuuuuuubbubbubbubbubbubbubbubb";
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
SERVER_DECL SQLStorage<CreatureInfo, HashMapStorageContainer<CreatureInfo> >                    CreatureNameStorage;
SERVER_DECL SQLStorage<CreatureProto, HashMapStorageContainer<CreatureProto> >                  CreatureProtoStorage;
SERVER_DECL SQLStorage<CreatureProtoVehicle, HashMapStorageContainer<CreatureProtoVehicle> >    CreatureProtoVehicleStorage;
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

SERVER_DECL set<string> ExtraMapCreatureTables;
SERVER_DECL set<string> ExtraMapGameObjectTables;

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

void ObjectMgr::LoadExtraGameObjectStuff()
{

}

void ObjectMgr::LoadExtraCreatureProtoStuff()
{
    CreatureProto * cn;
    CreatureInfo * ci;
    uint32 entry = 0;

    StorageContainerIterator<CreatureProto> * cpitr = CreatureProtoStorage.MakeIterator();
    while(!cpitr->AtEnd())
    {
        cn = cpitr->Get();

        if(cn->aura_string)
        {
            string auras = string(cn->aura_string);
            vector<string> aurs = StrSplit(auras, " ");
            for(vector<string>::iterator it = aurs.begin(); it != aurs.end(); it++)
            {
                uint32 id = atol((*it).c_str());
                if(id)
                    cn->start_auras.insert( id );
            }
        }

        if(!cn->MinHealth)
        {
            cn->MinHealth = 1;
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                WorldDatabase.Execute("UPDATE creature_proto SET minhealth = 1 WHERE entry = '%u'", cn->MinHealth, cn->Id);
        }

        if(!cn->MaxHealth)
        {
            cn->MaxHealth = 1;
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                WorldDatabase.Execute("UPDATE creature_proto SET maxhealth = 1 WHERE entry = '%u'", cn->MinHealth, cn->Id);
        }

        if(cn->MaxHealth < cn->MinHealth)
        {
            cn->MaxHealth = cn->MinHealth;
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                WorldDatabase.Execute("UPDATE creature_proto SET maxhealth = '%u' WHERE entry = '%u'", cn->MinHealth, cn->Id);
        }

        if(cn->MaxPower < cn->MinPower)
        {
            cn->MaxPower = cn->MinPower;
            if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                WorldDatabase.Execute("UPDATE creature_proto SET maxpower = '%u' WHERE entry = '%u'", cn->MinPower, cn->Id);
        }

        if (cn->AttackType > SCHOOL_ARCANE)
            cn->AttackType = SCHOOL_NORMAL;

        cn->ModeProto.clear();
        if(!cpitr->Inc())
            break;
    }
    cpitr->Destruct();

    StorageContainerIterator<CreatureInfo> * ciitr = CreatureNameStorage.MakeIterator();
    while(!ciitr->AtEnd())
    {
        ci = ciitr->Get();

        ci->lowercase_name = string(ci->Name);
        for(uint32 j = 0; j < ci->lowercase_name.length(); ++j)
            ci->lowercase_name[j] = tolower(ci->lowercase_name[j]); // Darvaleo 2008/08/15 - Copied lowercase conversion logic from ItemPrototype task

        if(!ciitr->Inc())
            break;
    }
    ciitr->Destruct();

    cn = NULL;

    QueryResult* result = WorldDatabase.Query( "SELECT * FROM ai_agents" );
    if( result != NULL )
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            cn = CreatureProtoStorage.LookupEntry(entry);
            if( cn == NULL )
            {
                sLog.Warning("AIAgent", "Agent skipped, NPC %u does not exist.", fields[0].GetUInt32());
                continue;
            }

            uint32 spellID = fields[1].GetUInt32();
            SpellEntry* spe = dbcSpell.LookupEntry(spellID);
            if( spe == NULL )
            {
                if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                    WorldDatabase.Execute("DELETE FROM ai_agents where entry = '%u' AND spell = '%u'", entry, spellID);
                sLog.Warning("AIAgent", "Agent skipped, NPC %u tried to add non-existing Spell %u.", entry, spellID);
                continue;
            }

            SkillLineSpell * _spell = objmgr.GetSpellSkill(spellID);
            if(_spell)
            {
                SkillLineEntry * _skill = dbcSkillLine.LookupEntry(_spell->skilline);
                if(_skill)
                {
                    if(_skill->type == SKILL_TYPE_PROFESSION)
                    {
                        sLog.outError("Creature %u has SpellID %u, which is a profession.", entry, spellID);
                        continue;
                    }
                }
            }

            AI_Spell *sp = new AI_Spell();
            const char* CharTargetType = fields[2].GetString();
            if(strcmp(CharTargetType, "ManaClass") == 0)
                sp->TargetType = TargetGen_ManaClass;
            else if(strcmp(CharTargetType, "RandomPlayerApplyAura") == 0)
                sp->TargetType = TargetGen_RandomPlayerApplyAura;
            else if(strcmp(CharTargetType, "RandomPlayerDestination") == 0)
                sp->TargetType = TargetGen_RandomPlayerDestination;
            else if(strcmp(CharTargetType, "RandomPlayer") == 0)
                sp->TargetType = TargetGen_RandomPlayer;
            else if(strcmp(CharTargetType, "RandomUnitApplyAura") == 0)
                sp->TargetType = TargetGen_RandomUnitApplyAura;
            else if(strcmp(CharTargetType, "RandomUnitDestination") == 0)
                sp->TargetType = TargetGen_RandomUnitDestination;
            else if(strcmp(CharTargetType, "RandomUnit") == 0)
                sp->TargetType = TargetGen_RandomUnit;
            else if(strcmp(CharTargetType, "Predefined") == 0)
                sp->TargetType = TargetGen_Predefined;
            else if(strcmp(CharTargetType, "SecondMostHated") == 0)
                sp->TargetType = TargetGen_SecondMostHated;
            else if(strcmp(CharTargetType, "SummonOwner") == 0)
                sp->TargetType = TargetGen_SummonOwner;
            else if(strcmp(CharTargetType, "Destination") == 0)
                sp->TargetType = TargetGen_Destination;
            else if(strcmp(CharTargetType, "Current") == 0)
                sp->TargetType = TargetGen_Current;
            else if(strcmp(CharTargetType, "Self") == 0)
                sp->TargetType = TargetGen_Self;

            const char* CharTargetFilter = fields[3].GetString();
            if(strcmp(CharTargetFilter, "ClosestFriendlyCorpse") == 0)
                sp->TargetFilter = TargetFilter_ClosestFriendlyCorpse;
            else if(strcmp(CharTargetFilter, "FriendlyCorpse") == 0)
                sp->TargetFilter = TargetFilter_FriendlyCorpse;
            else if(strcmp(CharTargetFilter, "WoundedFriendly") == 0)
                sp->TargetFilter = TargetFilter_WoundedFriendly;
            else if(strcmp(CharTargetFilter, "ClosestNotCurrent") == 0)
                sp->TargetFilter = TargetFilter_ClosestNotCurrent;
            else if(strcmp(CharTargetFilter, "ClosestFriendly") == 0)
                sp->TargetFilter = TargetFilter_ClosestFriendly;
            else if(strcmp(CharTargetFilter, "ManaClass") == 0)
                sp->TargetFilter = TargetFilter_ManaClass;
            else if(strcmp(CharTargetFilter, "IgnoreLineOfSight") == 0)
                sp->TargetFilter = TargetFilter_IgnoreLineOfSight;
            else if(strcmp(CharTargetFilter, "IgnoreSpecialStates") == 0)
                sp->TargetFilter = TargetFilter_IgnoreSpecialStates;
            else if(strcmp(CharTargetFilter, "InRangeOnly") == 0)
                sp->TargetFilter = TargetFilter_InRangeOnly;
            else if(strcmp(CharTargetFilter, "InMeleeRange") == 0)
                sp->TargetFilter = TargetFilter_InMeleeRange;
            else if(strcmp(CharTargetFilter, "Corpse") == 0)
                sp->TargetFilter = TargetFilter_Corpse;
            else if(strcmp(CharTargetFilter, "Aggroed") == 0)
                sp->TargetFilter = TargetFilter_Aggroed;
            else if(strcmp(CharTargetFilter, "SecondMostHated") == 0)
                sp->TargetFilter = TargetFilter_SecondMostHated;
            else if(strcmp(CharTargetFilter, "Wounded") == 0)
                sp->TargetFilter = TargetFilter_Wounded;
            else if(strcmp(CharTargetFilter, "NotCurrent") == 0)
                sp->TargetFilter = TargetFilter_NotCurrent;
            else if(strcmp(CharTargetFilter, "Friendly") == 0)
                sp->TargetFilter = TargetFilter_Friendly;
            else if(strcmp(CharTargetFilter, "Closest") == 0)
                sp->TargetFilter = TargetFilter_Closest;
            else if(strcmp(CharTargetFilter, "None") == 0)
                sp->TargetFilter = TargetFilter_None;

            sp->info = spe;
            sp->m_AI_Spell_disabled = false;
            sp->perctrigger = fields[4].GetFloat();
            sp->cooldown = fields[5].GetUInt32();
            sp->attackstoptimer = fields[6].GetUInt32();
            sp->casttime = fields[7].GetUInt32();
            sp->soundid = fields[8].GetUInt32();
            sp->ProcLimit = fields[9].GetUInt32();
            sp->ProcResetDelay = fields[10].GetUInt32();
            sp->mindist2cast = fields[11].GetUInt32();
            sp->maxdist2cast = fields[12].GetUInt32();
            sp->minHPPercentReq = fields[13].GetUInt32();
            sp->FloatData[0] = fields[14].GetFloat();
            sp->FloatData[1] = fields[15].GetFloat();
            sp->FloatData[2] = fields[16].GetFloat();
            sp->difficulty_mask = fields[17].GetInt32();
            cn->spells.push_back(sp);
            count++;
        }while( result->NextRow() );
        delete result;
        result = NULL;
        sLog.Notice("Storage", "%u AI agents loaded from the database.", count);
    }

    uint32 modecount = 0;
    bool loadmodes = false; // Crow: LOAD MOADS
    QueryResult * modechecks = WorldDatabase.Query( "SELECT * FROM creature_proto_mode");
    if(modechecks)
    {
        loadmodes = true;
        if(modechecks->GetFieldCount() != 18)
        {
            sLog.Error("ObjectStorage", "Incorrect field count(%u/18) for table creature_proto_mode, mode loading is disabled.", modechecks->GetFieldCount());
            loadmodes = false;
        }delete modechecks;
    }

    CreatureProto* proto;
    if(loadmodes)
    {
        // Load our mode proto.
        QueryResult * moderesult = WorldDatabase.Query( "SELECT * FROM creature_proto_mode");
        if(moderesult)
        {
            CreatureProtoMode* cpm = NULL;
            do
            {
                uint32 fieldcount = 0;
                Field *fields = moderesult->Fetch();
                entry = fields[fieldcount++].GetUInt32();
                proto = CreatureProtoStorage.LookupEntry(entry);
                if(proto == NULL)
                {
                    // Do something?
                    continue;
                }

                uint8 mode = fields[fieldcount++].GetUInt8();
                if(mode > 3 || mode < 1)
                {
                    sLog.Warning("ObjectStorage","Incorrect instance mode %u for creature %u, instance mode 3 max.", mode, entry);

                    if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                        WorldDatabase.Execute("DELETE FROM creature_proto_mode WHERE entry = %u AND mode = %u;", entry, mode);

                    continue;
                }

                cpm = new CreatureProtoMode();
                cpm->Minlevel = fields[fieldcount++].GetUInt32();
                cpm->Maxlevel = fields[fieldcount++].GetUInt32();
                cpm->Minhealth = fields[fieldcount++].GetUInt32();
                cpm->Maxhealth = fields[fieldcount++].GetUInt32();
                cpm->Mindmg = fields[fieldcount++].GetFloat();
                cpm->Maxdmg = fields[fieldcount++].GetFloat();
                cpm->Power = fields[fieldcount++].GetUInt32();
                for(uint8 i = 0; i < 7; i++)
                    cpm->Resistances[i] = fields[fieldcount++].GetUInt32();

                char* aura_string = (char*)fields[fieldcount++].GetString();
                if(aura_string)
                {
                    string auras = string(aura_string);
                    vector<string> aurs = StrSplit(auras, " ");
                    for(vector<string>::iterator it = aurs.begin(); it != aurs.end(); it++)
                    {
                        uint32 id = atol((*it).c_str());
                        if(id)
                            cpm->start_auras.insert( id );
                    }
                }

                cpm->auraimmune_flag = fields[fieldcount++].GetUInt32();

                // Begin cleanup changes.
                // Level cleanup.
                if(cpm->Maxlevel < cpm->Minlevel)
                    cpm->Maxlevel = cpm->Minlevel;

                // Health cleanup.
                if(cpm->Maxhealth < cpm->Minhealth)
                    cpm->Maxhealth = cpm->Minhealth;

                // Damage cleanup.
                if(cpm->Maxdmg < cpm->Mindmg)
                    cpm->Maxdmg = cpm->Mindmg;

                // End of cleanup changes.
                proto->ModeProto[mode] = cpm;
                modecount++;

            }while( moderesult->NextRow() );
            delete moderesult;
        }
    }

    // List what we've loaded.
    sLog.Notice("ObjectStorage","%u entries loaded from table creature_proto_mode", modecount);
}

void ObjectMgr::LoadExtraItemStuff()
{
    map<uint32,uint32> foodItems;
    QueryResult * result = WorldDatabase.Query("SELECT * FROM itempetfood ORDER BY entry");
    if(result)
    {
        do
        {
            Field *f = result->Fetch();
            foodItems.insert( make_pair( f[0].GetUInt32(), f[1].GetUInt32() ) );
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
                ItemsInSets.insert(make_pair(itemset->itemid[i], itemset->id));
            }
        }
    }

    if(mItemSets.size())
    {
        map<uint32, int32> ItemSetsHighest;
        map<uint32, map<uint32, set<uint32> > > RankByLevelInSet;
        for(ItemSetContentMap::iterator itr = mItemSets.begin(); itr != mItemSets.end(); itr++)
        {
            for(std::list<ItemPrototype*>::iterator itr2 = itr->second->begin(); itr2 != itr->second->end(); itr2++)
            {
                if(ItemSetsHighest.find(itr->first) == ItemSetsHighest.end() || (*itr2)->ItemLevel > ItemSetsHighest[itr->first])
                    ItemSetsHighest[itr->first] = (*itr2)->ItemLevel;

                if(RankByLevelInSet.find(itr->first) == RankByLevelInSet.end())
                {
                    set<uint32> pie;
                    pie.insert((*itr2)->ItemId);
                    map<uint32, set<uint32> > mapset;
                    mapset.insert(make_pair((*itr2)->ItemLevel, pie));

                    RankByLevelInSet.insert(make_pair(itr->first, mapset));
                }
                else
                {
                    if(RankByLevelInSet[itr->first].find((*itr2)->ItemLevel) == RankByLevelInSet[itr->first].end())
                    {
                        set<uint32> pie;
                        pie.insert((*itr2)->ItemId);
                        RankByLevelInSet[itr->first].insert(make_pair((*itr2)->ItemLevel, pie));
                    }
                    else
                        RankByLevelInSet[itr->first][(*itr2)->ItemLevel].insert((*itr2)->ItemId);
                }
            }
        }

        for(map<uint32, map<uint32, set<uint32> > >::iterator itr = RankByLevelInSet.begin(); itr != RankByLevelInSet.end(); itr++)
        {
            uint32 rank = 1;
            uint32 lastRank = 0;
            uint32 oldRanking = 0;
            string lastPrefix = "";
            set<uint32> UsedRanks;
            ItemPrototype *IP = NULL;
            map<string, uint32> rankByPrefix;
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
                                set<uint32>::iterator itr2 = itr->second[i].begin();
                                IP = ItemPrototypeStorage.LookupEntry(*itr2);
                                string name = IP->Name1;
                                size_t spot = 0;
                                string Prefix = "";
                                if((spot = name.find(" ")) != string::npos)
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

                    for(set<uint32>::iterator itr2 = itr->second[i].begin(); itr2 != itr->second[i].end(); itr2++)
                    {
                        IP = ItemPrototypeStorage.LookupEntry(*itr2);
                        string name = IP->Name1;
                        size_t spot = 0;
                        string Prefix = "";
                        if((spot = name.find(" ")) != string::npos)
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
                            rankByPrefix.insert(make_pair(Prefix, rank));

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
    make_task(CreatureNameStorage, CreatureInfo, HashMapStorageContainer, "creature_names", gCreatureNameFormat);
    make_task(GameObjectNameStorage, GameObjectInfo, HashMapStorageContainer, "gameobject_names", gGameObjectNameFormat);
    make_task(CreatureProtoStorage, CreatureProto, HashMapStorageContainer, "creature_proto", gCreatureProtoFormat);
    make_task(CreatureProtoVehicleStorage, CreatureProtoVehicle, HashMapStorageContainer, "creature_proto_vehicle", gCreatureVehicleProto);
    make_task(CreatureInfoExtraStorage, CreatureInfoExtra, HashMapStorageContainer, "creature_info", gCreatureInfoExtra);
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
    StorageContainerIterator<CreatureProto> * cpitr = CreatureProtoStorage.MakeIterator();
    CreatureProto * p;
    while(!cpitr->AtEnd())
    {
        p = cpitr->Get();

        if (p->aura_string)
        {
            free(p->aura_string);
            p->aura_string = NULL;
        }

        for(list<AI_Spell*>::iterator it = p->spells.begin(); it != p->spells.end(); it++)
            delete (*it);

        p->spells.clear();
        p->start_auras.clear();
        p->ModeProto.clear();

        if(!cpitr->Inc())
            break;
    }
    cpitr->Destruct();

    CreatureNameStorage.Cleanup();
    GameObjectNameStorage.Cleanup();
    CreatureProtoStorage.Cleanup();
    CreatureProtoVehicleStorage.Cleanup();
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

vector<pair<string,string> > additionalTables;

bool LoadAdditionalTable(const char * TableName, const char * SecondName)
{
    if(!stricmp(TableName, "creature_spawns"))
    {
        ExtraMapCreatureTables.insert(string(SecondName));
        return false;
    }
    else if(!stricmp(TableName, "gameobject_spawns"))
    {
        ExtraMapGameObjectTables.insert(string(SecondName));
        return false;
    }
    else if(!stricmp(TableName, "creature_proto"))      // Creature Proto
        CreatureProtoStorage.LoadAdditionalData(SecondName, gCreatureProtoFormat);
    else if(!stricmp(TableName, "creature_names"))      // Creature Names
        CreatureNameStorage.LoadAdditionalData(SecondName, gCreatureNameFormat);
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
    if(!stricmp(TableName, "creature_proto"))      // Creature Proto
        CreatureProtoStorage.Reload();
    else if(!stricmp(TableName, "creature_proto_vehicle"))  // Creature Vehicle Proto
        CreatureProtoVehicleStorage.Reload();
    else if(!stricmp(TableName, "creature_names"))      // Creature Names
        CreatureNameStorage.Reload();
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
    for(vector<pair<string,string> >::iterator itr = additionalTables.begin(); itr != additionalTables.end(); itr++)
    {
        len2=(uint32)itr->second.length();
        if(!strnicmp(TableName, itr->second.c_str(), min(len,len2)))
            LoadAdditionalTable(TableName, itr->first.c_str());
    }
    return true;
}

void Storage_LoadAdditionalTables()
{
    ExtraMapCreatureTables.insert(string("creature_spawns"));
    ExtraMapGameObjectTables.insert(string("gameobject_spawns"));

    string strData = mainIni->ReadString("Startup", "LoadAdditionalTables", "");
    if(strData.empty())
        return;

    vector<string> strs = StrSplit(strData, ",");
    if(strs.empty())
        return;

    for(vector<string>::iterator itr = strs.begin(); itr != strs.end(); itr++)
    {
        char s1[200];
        char s2[200];
        if(sscanf((*itr).c_str(), "%s %s", s1, s2) != 2)
            continue;

        if(LoadAdditionalTable(s2, s1))
        {
            pair<string,string> tmppair;
            tmppair.first = string(s1);
            tmppair.second = string(s2);
            additionalTables.push_back(tmppair);
        }
    }
}
