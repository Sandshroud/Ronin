/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton(LootMgr);

struct loot_tb
{
    uint32 itemid;
    float chance;
};

bool Rand(float chance)
{
    float val = RandomFloat(10000.0f);
    float p = chance * 100.0f;
    return p >= val;
}

bool Rand(uint32 chance)
{
    int32 val = RandomUInt(10000);
    int32 p = int32(chance * 100);
    return p >= val;
}

bool Rand(int32 chance)
{
    int32 val = RandomUInt(10000);
    int32 p = chance * 100;
    return p >= val;
}

template <class T>  // works for anything that has the field 'chance' and is stored in plain array
const T& RandomChoice( const T* variant, int count )
{
    float totalChance = 0;
    for( int i = 0; i < count; i++)
        totalChance += variant[i].chance;

    float val = RandomFloat(totalChance);
    for( int i = 0; i < count; i++)
    {
        val -= variant[i].chance;
        if (val <= 0) return variant[i];
    }
    // should not come here, buf if it does, we should return something reasonable
    return variant[count-1];
}

template <class T>  // works for anything that has the field 'chance' and is stored in plain array
T* RandomChoiceVector( vector<pair<T*, float> > & variant )
{
    float totalChance = 0;
    float val;
    typename vector<pair<T*,float> >::iterator itr;

    if(variant.size() == 0)
        return NULL;

    for(itr = variant.begin(); itr != variant.end(); itr++)
        totalChance += itr->second;

    val = RandomFloat(totalChance);

    for(itr = variant.begin(); itr != variant.end(); itr++)
    {
        val -= itr->second;
        if (val <= 0) return itr->first;
    }
    // should not come here, buf if it does, we should return something reasonable
    return variant.begin()->first;
}

LootMgr::LootMgr()
{
    is_loading = false;
}

/*  Guidlines for loading loot:
    Crow: Heavier loot should be loaded first.
    If it has more than 3k, do a delayed load.
*/
void LootMgr::LoadLoot()
{
    //THIS MUST BE CALLED AFTER LOADING OF ITEMS
    is_loading = true;
    LoadLootProp();
    sLog.Debug("LootMgr","Loading loot...");
    LoadLootTables(FISHING_LOOT, &FishingLoot, false);
    is_loading = false;
}

void LootMgr::LoadDelayedLoot()
{
    is_loading = true;
    LoadLootTables(ITEM_LOOT, &ItemLoot, false);
    LoadLootTables(OBJECT_LOOT, &GOLoot, true);
    LoadLootTables(CREATURE_LOOT, &CreatureLoot, true);
    LoadLootTables(PICKPOCKETING_LOOT, &PickpocketingLoot, false);
    LoadLootTables(CREATURE_LOOT_GATHERING, &GatheringLoot, true);
    is_loading = false;
}

ItemRandomPropertiesEntry* LootMgr::GetRandomProperties(ItemPrototype * proto)
{
    map<uint32,RandomPropertyVector>::iterator itr;

    if(proto->RandomPropId==0)
        return NULL;

    itr = _randomprops.find(proto->RandomPropId);
    if(itr==_randomprops.end())
        return NULL;

    return RandomChoiceVector<ItemRandomPropertiesEntry>(itr->second);
}

ItemRandomSuffixEntry * LootMgr::GetRandomSuffix(ItemPrototype * proto)
{
    map<uint32,RandomSuffixVector>::iterator itr;

    if(proto->RandomSuffixId==0)
        return NULL;

    itr = _randomsuffix.find(proto->RandomSuffixId);
    if(itr==_randomsuffix.end())
        return NULL;

    return RandomChoiceVector<ItemRandomSuffixEntry>(itr->second);
}


void LootMgr::LoadLootProp()
{
    QueryResult * result = WorldDatabase.Query("SELECT * FROM item_randomprop_groups");
    uint32 id, eid;
    ItemRandomPropertiesEntry * rp;
    ItemRandomSuffixEntry * rs;
    float ch;

    if(result)
    {
        map<uint32, RandomPropertyVector>::iterator itr;
        do
        {
            id = result->Fetch()[0].GetUInt32();
            eid = result->Fetch()[1].GetUInt32();
            ch = result->Fetch()[2].GetFloat();

            rp = dbcItemRandomProperties.LookupEntry(eid);
            if(rp == NULL)
            {
                sLog.Error("LoadLootProp", "RandomProp group %u references non-existant randomprop %u.", id, eid);
                continue;
            }

            itr = _randomprops.find(id);
            if(itr == _randomprops.end())
            {
                RandomPropertyVector v;
                v.push_back(make_pair(rp, ch));
                _randomprops.insert(make_pair(id, v));
            }
            else
                itr->second.push_back(make_pair(rp,ch));

        } while(result->NextRow());
        delete result;
    }

    result = WorldDatabase.Query("SELECT * FROM item_randomsuffix_groups");
    if(result)
    {
        map<uint32, RandomSuffixVector>::iterator itr;
        do
        {
            id = result->Fetch()[0].GetUInt32();
            eid = result->Fetch()[1].GetUInt32();
            ch = result->Fetch()[2].GetFloat();

            rs = dbcItemRandomSuffix.LookupEntry(eid);
            if(rs == NULL)
            {
                sLog.Error("LoadLootProp", "RandomSuffix group %u references non-existant randomsuffix %u.", id, eid);
                continue;
            }

            itr = _randomsuffix.find(id);
            if(itr == _randomsuffix.end())
            {
                RandomSuffixVector v;
                v.push_back(make_pair(rs, ch));
                _randomsuffix.insert(make_pair(id, v));
            }
            else
                itr->second.push_back(make_pair(rs,ch));

        } while(result->NextRow());
        delete result;
    }
}

LootMgr::~LootMgr()
{
    sLog.Notice("LootMgr","Deleting Loot Tables...");
    for(LootStore::iterator iter=CreatureLoot.begin(); iter != CreatureLoot.end(); iter++)
        delete [] iter->second.items;

    for(LootStore::iterator iter=FishingLoot.begin(); iter != FishingLoot.end(); iter++)
        delete [] iter->second.items;

    for(LootStore::iterator iter=GatheringLoot.begin(); iter != GatheringLoot.end(); iter++)
        delete [] iter->second.items;

    for(LootStore::iterator iter=GOLoot.begin(); iter != GOLoot.end(); iter++)
        delete [] iter->second.items;

    for(LootStore::iterator iter=ItemLoot.begin(); iter != ItemLoot.end(); iter++)
        delete [] iter->second.items;

    for(LootStore::iterator iter=PickpocketingLoot.begin(); iter != PickpocketingLoot.end(); iter++)
        delete [] iter->second.items;
}

struct tempLootData
{
    uint32 itemid;
    float chance[4];
    uint32 mincount;
    uint32 maxcount;
    uint32 ffa_loot;
};

void LootMgr::LoadLootTables(const char * szTableName, LootStore * LootTable, bool MultiDifficulty)
{
    sLog.Debug("LootMgr","Attempting to load loot from table %s...", szTableName);
    vector< pair< uint32, vector< tempLootData > > > db_cache;
    vector< pair< uint32, vector< tempLootData > > >::iterator itr;
    db_cache.reserve(10000);
    LootStore::iterator tab;
    QueryResult *result = WorldDatabase.Query("SELECT * FROM %s ORDER BY entryid ASC",szTableName);
    if(!result)
    {
        sLog.Error("LootMgr", "Loading loot from table %s failed.", szTableName);
        return;
    }

    if(MultiDifficulty) // We have multiple difficulties.
    {
        if(result->GetFieldCount() != 9)
        {
            sLog.Error("LootMgr", "Incorrect structure in table %s(%u/9), loot loading has been cancled to prevent a crash.", szTableName, result->GetFieldCount());
            return;
        }
    }
    else if(result->GetFieldCount() != 6)
    {
        sLog.Error("LootMgr", "Incorrect structure in table %s(%u/6), loot loading has been cancled to prevent a crash.", szTableName, result->GetFieldCount());
        return;
    }

    uint32 pos = 0;
    uint32 entry_id = 0;
    uint32 last_entry = 0;
    uint32 total = (uint32) result->GetRowCount();

    tempLootData t;
    vector< tempLootData > ttab;
    Field *fields = NULL;
    do
    {
        fields = result->Fetch();
        entry_id = fields[0].GetUInt32();
        if(entry_id < last_entry)
        {
            sLog.Error("LootMgr", "WARNING: Out of order loot table being loaded.");
            delete result;
            return;
        }
        if(entry_id != last_entry)
        {
            if(last_entry != 0)
                db_cache.push_back( make_pair( last_entry, ttab) );
            ttab.clear();
        }

        if(MultiDifficulty) // We have multiple difficulties.
        {
            t.itemid = fields[1].GetUInt32();
            for(int8 i = 0; i < 4; i++)
                t.chance[i] = fields[2+i].GetFloat();
            t.mincount = fields[6].GetUInt32();
            t.maxcount = fields[7].GetUInt32();
            t.ffa_loot = fields[8].GetUInt32();
        }
        else // We have one chance, regardless of difficulty.
        {
            t.itemid = fields[1].GetUInt32();
            t.chance[0] = fields[2].GetFloat();
            t.mincount = fields[3].GetUInt32();
            t.maxcount = fields[4].GetUInt32();
            t.ffa_loot = fields[5].GetUInt32();
            for(int8 i = 1; i < 4; i++) // Other difficulties.
                t.chance[i] = 0.0f;
        }

        for(uint i = 0; i < 4; i++)
            if(t.chance[i] == -1.0f)
                t.chance[i] = RandomFloat(100);

        ttab.push_back( t );
        last_entry = entry_id;
    } while( result->NextRow() );

    total = uint32(db_cache.size());
    ItemPrototype* proto = NULL;
    StoreLootList *list = NULL;
    uint32 itemid;
    for( itr = db_cache.begin(); itr != db_cache.end(); itr++)
    {
        entry_id = (*itr).first;
        if(LootTable->find(entry_id) == LootTable->end())
        {
            StoreLootList &list = ((*LootTable)[entry_id]);
            list.count = (uint32)(*itr).second.size();
            list.items = new StoreLootItem[list.count];

            uint32 ind = 0;
            for(vector< tempLootData >::iterator itr2 = (*itr).second.begin(); itr2 != (*itr).second.end(); itr2++)
            {
                // Omit items that are not in db to prevent future bugs
                proto = ItemPrototypeStorage.LookupEntry((itemid = itr2->itemid));
                if(!proto)
                {
                    list.items[ind].item.itemproto = NULL;
                    if(mainIni->ReadBoolean("Server", "CleanDatabase", false))
                        WorldDatabase.Query("DELETE FROM %s where entryid ='%u' AND itemid = '%u'", szTableName, entry_id, itemid);
                    sLog.Warning("LootMgr", "Loot for %u contains non-existant item(%u). (%s)", entry_id, itemid, szTableName);
                }
                else
                {
                    list.items[ind].item.itemproto=proto;
                    list.items[ind].item.displayid=proto->DisplayInfoID;
                    for(int i = 0; i < 4; i++)
                        list.items[ind].chance[i] = itr2->chance[i];
                    list.items[ind].mincount = itr2->mincount;
                    list.items[ind].maxcount = itr2->maxcount;
                    list.items[ind].ffa_loot = itr2->ffa_loot;
                    if(proto->Class == ITEM_CLASS_QUEST)
                    {
                        ObjectQuestLoot *objQloot = NULL;
                        if(LootTable == &GOLoot)
                        {
                            if(_gameobjectquestloot[entry_id] == NULL)
                            {
                                _gameobjectquestloot[entry_id] = objQloot = new ObjectQuestLoot();
                                objQloot->index = 0; memset(&objQloot->QuestLoot, 0, sizeof(uint32)*6);
                            } else objQloot = _gameobjectquestloot.at(entry_id);
                        }
                        else if(LootTable == &CreatureLoot)
                        {
                            if(_creaturequestloot[entry_id] == NULL)
                            {
                                _creaturequestloot[entry_id] = objQloot = new ObjectQuestLoot();
                                objQloot->index = 0; memset(&objQloot->QuestLoot, 0, sizeof(uint32)*6);
                            } else objQloot = _creaturequestloot.at(entry_id);
                        }

                        if(objQloot && objQloot->index < 6)
                            objQloot->QuestLoot[objQloot->index++] = proto->ItemId;
                    }
                }
                ++ind;
            }
        }
    }

    sLog.Notice("LootMgr","%d loot templates loaded from %s", db_cache.size(), szTableName);
    delete result;
}

void LootMgr::PushLoot(StoreLootList *list,Loot * loot, uint8 difficulty, uint8 team, bool disenchant)
{
    uint32 i;
    uint32 count;
    float nrand = 0;
    float ncount = 0;
    assert(difficulty < 4);

    if (disenchant)
    {
        nrand = RandomUInt(10000) / 100.0f;
        ncount = 0;
    }

    for( uint32 x = 0; x < list->count; x++ )
    {
        if( list->items[x].item.itemproto )// this check is needed until loot DB is fixed
        {
            float chance = list->items[x].chance[difficulty];
            if(chance == 0.0f)
                continue;
            ItemPrototype *itemproto = list->items[x].item.itemproto;
            if(itemproto== NULL)
                continue;

            int lucky;
            if (disenchant)
            {
                lucky = nrand >= ncount && nrand <= (ncount+chance);
                ncount+= chance;
            }
            else
                lucky = Rand( chance * sWorld.getRate( RATE_DROP0 + itemproto->Quality ) );

            if( lucky )
            {
                if( list->items[x].mincount == list->items[x].maxcount )
                    count = list->items[x].maxcount;
                else
                    count = RandomUInt(list->items[x].maxcount - list->items[x].mincount) + list->items[x].mincount;

                for( i = 0; i < loot->items.size(); i++ )
                {
                    //itemid rand match a already placed item, if item is stackable and unique(stack), increment it, otherwise skips
                    if((loot->items[i].item.itemproto == list->items[x].item.itemproto) && itemproto->MaxCount > 0 && ((loot->items[i].StackSize + count) < (uint32)itemproto->MaxCount))
                    {
                        if(itemproto->Unique > 0 && ((loot->items[i].StackSize+count) < (uint32)itemproto->Unique))
                        {
                            loot->items[i].StackSize += count;
                            break;
                        }
                        else if (!itemproto->Unique)
                        {
                            loot->items[i].StackSize += count;
                            break;
                        }
                    }
                }

                if( i != loot->items.size() )
                    continue;

                __LootItem itm;
                itm.item =list->items[x].item;
                itm.StackSize = count;
                itm.roll = NULLROLL;
                itm.passed = false;
                itm.ffa_loot = list->items[x].ffa_loot;
                itm.has_looted.clear();

                if( itemproto->Quality > 1 && itemproto->ContainerSlots == 0 )
                {
                    itm.iRandomProperty=GetRandomProperties( itemproto );
                    itm.iRandomSuffix=GetRandomSuffix( itemproto );
                }
                else
                {
                    // save some calls :P
                    itm.iRandomProperty = NULL;
                    itm.iRandomSuffix = NULL;
                }

                loot->items.push_back(itm);
            }
        }
    }

    if( loot->items.size() > 16 )
    {
        std::vector<__LootItem>::iterator item_to_remove;
        std::vector<__LootItem>::iterator itr;
        int32 item_quality;
        bool quest_item;
        while( loot->items.size() > 16 )
        {
            item_to_remove = loot->items.begin();
            item_quality = 0;
            quest_item = false;
            for( itr = loot->items.begin(); itr != loot->items.end(); itr++ )
            {
                item_quality = (*itr).item.itemproto->Quality;
                quest_item = (*itr).item.itemproto->Class == ITEM_CLASS_QUEST;
                if( (*item_to_remove).item.itemproto->Quality > item_quality && !quest_item )
                {
                    item_to_remove = itr;
                }
            }
            loot->items.erase( item_to_remove );
        }
    }

}

void LootMgr::FillCreatureLoot(Loot * loot,uint32 loot_id, uint8 difficulty, uint8 team)
{
    loot->items.clear();
    loot->gold = 0;

    LootStore::iterator tab = CreatureLoot.find(loot_id);
    if( CreatureLoot.end() == tab)
        return;
    else
        PushLoot(&tab->second, loot, difficulty, team, false);
}

void LootMgr::FillGOLoot(Loot * loot,uint32 loot_id, uint8 difficulty, uint8 team)
{
    loot->items.clear ();
    loot->gold = 0;

    LootStore::iterator tab = GOLoot.find(loot_id);
    if( GOLoot.end() == tab)
        return;
    else
        PushLoot(&tab->second, loot, difficulty, team, false);
}

void LootMgr::FillFishingLoot(Loot * loot,uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;

    LootStore::iterator tab = FishingLoot.find(loot_id);
    if( FishingLoot.end() == tab)
        return;
    else
        PushLoot(&tab->second, loot, 0, 0, false);
}

void LootMgr::FillGatheringLoot(Loot * loot,uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;

    LootStore::iterator tab = GatheringLoot.find(loot_id);
    if(tab != GatheringLoot.end())
        PushLoot(&tab->second, loot, 0, 0, false);
}

void LootMgr::FillPickpocketingLoot(Loot * loot,uint32 loot_id)
{
    loot->items.clear();
    loot->gold = 0;

    LootStore::iterator tab = PickpocketingLoot.find(loot_id);
    if( PickpocketingLoot.end() == tab)
        return;
    else
        PushLoot(&tab->second, loot, 0, 0, false);
}

void LootMgr::FillItemLoot(Loot *loot, uint32 loot_id, uint8 team)
{
    loot->items.clear();
    loot->gold = 0;

    LootStore::iterator tab = ItemLoot.find(loot_id);
    if( ItemLoot.end()==tab)
        return;
    else
        PushLoot(&tab->second, loot, 0, team, false);
}

bool LootMgr::CanGODrop(uint32 LootId,uint32 itemid)
{
    LootStore::iterator tab =GOLoot.find(LootId);
    if( GOLoot.end() == tab)
        return false;
    StoreLootList *list=&(tab->second);
    for(uint32 x=0;x<list->count;x++)
    {
        if(list->items[x].item.itemproto->ItemId == itemid)
            return true;
    }
    return false;
}

//THIS should be cached
bool LootMgr::IsPickpocketable(uint32 creatureId)
{
    LootStore::iterator tab = PickpocketingLoot.find(creatureId);
    if( PickpocketingLoot.end() == tab)
        return false;
    else
        return true;
}

//THIS should be cached
bool LootMgr::IsSkinnable(uint32 creatureId)
{
    LootStore::iterator tab = GatheringLoot.find(creatureId);
    if( tab != GatheringLoot.end())
        return true;

    return false;
}

//THIS should be cached
bool LootMgr::IsFishable(uint32 zoneid)
{
    LootStore::iterator tab = FishingLoot.find(zoneid);
    return tab != FishingLoot.end();
}

void LootMgr::AddLoot(Loot* loot, uint32 itemid, uint32 mincount, uint32 maxcount, uint32 ffa_loot)
{
    uint32 i;
    uint32 count;
    ItemPrototype *itemproto = ItemPrototypeStorage.LookupEntry(itemid);

    if( itemproto )// this check is needed until loot DB is fixed
    {
        if( mincount == maxcount )
            count = maxcount;
        else
            count = RandomUInt(maxcount - mincount) + mincount;

        for( i = 0; i < loot->items.size(); i++ )
        {
            //itemid rand match a already placed item, if item is stackable and unique(stack), increment it, otherwise skips
            if((loot->items[i].item.itemproto == itemproto) && itemproto->MaxCount > 0 && ((loot->items[i].StackSize + count <= (uint32)itemproto->MaxCount)))
            {
                if(itemproto->Unique > 0 && ((loot->items[i].StackSize+count) < (uint32)itemproto->Unique))
                {
                    loot->items[i].StackSize += count;
                    break;
                }
                else if (!itemproto->Unique)
                {
                    loot->items[i].StackSize += count;
                    break;
                }
            }
        }

        if( i != loot->items.size() )
            return;

        _LootItem item;
        item.itemproto = itemproto;
        item.displayid = itemproto->DisplayInfoID;

        __LootItem itm;
        itm.item = item;
        itm.StackSize = count;
        itm.roll = NULLROLL;
        itm.passed = false;
        itm.ffa_loot = ffa_loot;
        itm.has_looted.clear();

        if( itemproto->Quality > 1 && itemproto->ContainerSlots == 0 )
        {
            itm.iRandomProperty=GetRandomProperties( itemproto );
            itm.iRandomSuffix=GetRandomSuffix( itemproto );
        }
        else
        {
            // save some calls :P
            itm.iRandomProperty = NULL;
            itm.iRandomSuffix = NULL;
        }

        loot->items.push_back(itm);
    }
}

LootRoll::LootRoll() : EventableObject()
{
}

void LootRoll::Init(uint32 timer, uint32 groupcount, uint64 guid, uint32 slotid, uint32 itemid, uint32 randomsuffixid, uint32 randompropertyid, MapMgr* mgr)
{
    _mgr = mgr;
    sEventMgr.AddEvent(this, &LootRoll::Finalize, EVENT_LOOT_ROLL_FINALIZE, 60000, 1,0);
    _groupcount = groupcount;
    _guid = guid;
    _slotid = slotid;
    _itemid = itemid;
    _randomsuffixid = randomsuffixid;
    _randompropertyid = randompropertyid;
    _remaining = groupcount;
}

LootRoll::~LootRoll()
{

}

void LootRoll::Finalize()
{
    if( !mLootLock.AttemptAcquire() ) // only one finalization, please. players on different maps can roll, too, so this is needed.
    {
        sEventMgr.RemoveEvents(this);
        return;
    }

    sEventMgr.RemoveEvents(this);

    // this we will have to finalize with groups types.. for now
    // we'll just assume need before greed. person with highest roll
    // in need gets the item.

    uint32 highest = 0;
    int8 hightype = -1;
    uint64 player = 0;

    WorldPacket data(34);

    for(std::map<uint32, uint32>::iterator itr = m_NeedRolls.begin(); itr != m_NeedRolls.end(); itr++)
    {
        if(itr->second > highest)
        {
            highest = itr->second;
            player = itr->first;
            hightype = NEED;
        }
    }

    if(!highest)
    {
        for(std::map<uint32, uint32>::iterator itr = m_GreedRolls.begin(); itr != m_GreedRolls.end(); itr++)
        {
            if(itr->second > highest)
            {
                highest = itr->second;
                player = itr->first;
                hightype = GREED;
            }
        }

        for(std::map<uint32, uint32>::iterator itr = m_DisenchantRolls.begin(); itr != m_DisenchantRolls.end(); itr++)
        {
            if(itr->second > highest)
            {
                highest = itr->second;
                player = itr->first;
                hightype = DISENCHANT;
            }
        }
    }


    Loot * pLoot = 0;
    uint32 guidtype = GUID_HIPART(_guid);
    if( guidtype == HIGHGUID_TYPE_CREATURE )
    {
        Creature* pc = _mgr->GetCreature(GUID_LOPART(_guid));
        if(pc) pLoot = &pc->m_loot;
    }
    else if( guidtype == HIGHGUID_TYPE_GAMEOBJECT )
    {
        GameObject* go = _mgr->GetGameObject(GUID_LOPART(_guid));
        if(go) pLoot = &go->m_loot;
    }

    if(!pLoot)
    {
        delete this;
        return;
    }

    if(_slotid >= pLoot->items.size())
    {
        delete this;
        return;
    }

    pLoot->items.at(_slotid).roll = NULLROLL;

    uint32 itemid = pLoot->items.at(_slotid).item.itemproto->ItemId;
    uint32 amt = pLoot->items.at(_slotid).StackSize;
    if(!amt)
    {
        delete this;
        return;
    }

    Player* _player = (player) ? _mgr->GetPlayer((uint32)player) : NULLPLR;
    if(!player || !_player)
    {
        /* all passed */
        data.Initialize(SMSG_LOOT_ALL_PASSED);
        data << _guid << _groupcount << _itemid << _randomsuffixid << _randompropertyid;
        set<uint32>::iterator pitr = m_passRolls.begin();
        while(_player == NULL && pitr != m_passRolls.end())
        {
            _player = _mgr->GetPlayer( (*(pitr)) );
            ++pitr;
        }

        if( _player != NULL )
        {
            if(_player->InGroup())
                _player->GetGroup()->SendPacketToAll(&data);
            else
                _player->GetSession()->SendPacket(&data);
        }

        /* item can now be looted by anyone :) */
        pLoot->items.at(_slotid).passed = true;
        delete this;
        return;
    }

    pLoot->items.at(_slotid).roll = 0;
    data.Initialize(SMSG_LOOT_ROLL_WON);
    data << _guid << _slotid << _itemid << _randomsuffixid << _randompropertyid;
    data << _player->GetGUID() << uint8(highest) << uint8(hightype);
    if(_player->InGroup())
        _player->GetGroup()->SendPacketToAll(&data);
    else
        _player->GetSession()->SendPacket(&data);

    if(hightype == DISENCHANT)
    {
        //generate Disenchantingloot
        Item * pItem = objmgr.CreateItem( itemid, _player);
        lootmgr.FillItemLoot(&pItem->m_loot, pItem->GetEntry(), _player->GetTeam());

        //add loot
        for(std::vector<__LootItem>::iterator iter=pItem->m_loot.items.begin();iter != pItem->m_loot.items.end();iter++)
        {
            itemid =iter->item.itemproto->ItemId;
            Item * Titem = objmgr.CreateItem( itemid, _player);
            if( Titem == NULLITEM )
                continue;
            if( !_player->GetItemInterface()->AddItemToFreeSlot(Titem) )
            {
                _player->GetSession()->SendNotification("No free slots were found in your inventory, item has been mailed.");
                sMailSystem.DeliverMessage(MAILTYPE_NORMAL, _player->GetGUID(), _player->GetGUID(), "Loot Roll", "Here is your reward.", 0, 0, itemid, 1, true);
            }
            Titem->DeleteMe();
            Titem = NULLITEM;
        }
        pItem->DeleteMe();
        pItem = NULLITEM;

        // Set a looter, doesn't matter who.
        pLoot->items.at(_slotid).has_looted.insert(_player->GetLowGUID());

        //Send "finish" packet
        data.Initialize(SMSG_LOOT_REMOVED);
        data << uint8(_slotid);
        Player* plr;
        for(LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); itr++)
        {
            if((plr = _player->GetMapMgr()->GetPlayer(*itr)))
                plr->GetSession()->SendPacket(&data);
        }

        delete this;    //end here and skip the rest
        return;
    }

    ItemPrototype* it = ItemPrototypeStorage.LookupEntry(itemid);

    int8 error;
    if((error = _player->GetItemInterface()->CanReceiveItem(it, 1, NULL)))
    {
        _player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, error);
        return;
    }

    Item* add = _player->GetItemInterface()->FindItemLessMax(itemid, amt, false);

    if (!add)
    {
        SlotResult slotresult = _player->GetItemInterface()->FindFreeInventorySlot(it);
        if(!slotresult.Result)
        {
            _player->GetSession()->SendNotification("No free slots were found in your inventory, item has been mailed.");
            sMailSystem.DeliverMessage(MAILTYPE_NORMAL, _player->GetGUID(), _player->GetGUID(), "Loot Roll", "Here is your reward.", 0, 0, it->ItemId, 1, true);
            data.Initialize(SMSG_LOOT_REMOVED);
            data << uint8(_slotid);
            Player* plr;
            for(LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); itr++)
            {
                if((plr = _player->GetMapMgr()->GetPlayer(*itr)))
                    plr->GetSession()->SendPacket(&data);
            }
            delete this;
            return;
        }

        sLog.Debug("HandleAutostoreItem","AutoLootItem %u",itemid);
        Item* item = objmgr.CreateItem( itemid, _player);

        item->SetUInt32Value(ITEM_FIELD_STACK_COUNT,amt);
        if(pLoot->items.at(_slotid).iRandomProperty!=NULL)
        {
            item->SetRandomProperty(pLoot->items.at(_slotid).iRandomProperty->ID);
            item->ApplyRandomProperties(false);
        }
        else if(pLoot->items.at(_slotid).iRandomSuffix != NULL)
        {
            item->SetRandomSuffix(pLoot->items.at(_slotid).iRandomSuffix->id);
            item->ApplyRandomProperties(false);
        }


        if( _player->GetItemInterface()->SafeAddItem(item,slotresult.ContainerSlot, slotresult.Slot) )
        {
            _player->GetSession()->SendItemPushResult(item,false,true,true,true,slotresult.ContainerSlot,slotresult.Slot,1);
            sQuestMgr.OnPlayerItemPickup(_player, item, amt);
        }
        else
        {
            item->DeleteMe();
            item = NULLITEM;
        }
    }
    else
    {
        add->SetCount(add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + amt);
        add->m_isDirty = true;
        sQuestMgr.OnPlayerItemPickup(_player, add, amt);
        _player->GetSession()->SendItemPushResult(add, false, true, true, false, _player->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()), 0xFFFFFFFF, 1);
    }

    // Set a looter, doesn't matter who.
    pLoot->items.at(_slotid).has_looted.insert(_player->GetLowGUID());

    // this gets sent to all looters
    data.Initialize(SMSG_LOOT_REMOVED);
    data << uint8(_slotid);
    Player* plr;
    for(LooterSet::iterator itr = pLoot->looters.begin(); itr != pLoot->looters.end(); itr++)
    {
        if((plr = _player->GetMapMgr()->GetPlayer(*itr)))
            plr->GetSession()->SendPacket(&data);
    }

    delete this;
}

void LootRoll::PlayerRolled(PlayerInfo* pInfo, uint8 choice)
{
    if(m_NeedRolls.find(pInfo->guid) != m_NeedRolls.end() || m_GreedRolls.find(pInfo->guid) != m_GreedRolls.end() || m_DisenchantRolls.find(pInfo->guid) != m_DisenchantRolls.end())
        return; // dont allow cheaters

    ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(_itemid);
    if(proto == NULL)
        return; // Shouldn't happen.

    mLootLock.Acquire();

    int roll = RandomUInt(99)+1;
    // create packet
    WorldPacket data(SMSG_LOOT_ROLL, 34);
    data << _guid << _slotid << uint64(pInfo->guid);
    data << _itemid << _randomsuffixid << _randompropertyid;

    if(!pInfo->m_loggedInPlayer)
        choice = PASS;
    bool acceptableroll = true;
    switch(choice)
    {
    case NEED:
        {
            if(pInfo->m_loggedInPlayer->CanNeedItem(proto))
            {
                m_NeedRolls.insert( std::make_pair(pInfo->guid, roll) );
                data << uint8(roll) << uint8(NEED);
            }
            else
            {
                m_passRolls.insert( pInfo->guid );
                data << uint8(128) << uint8(128);
            }
        }break;
    case GREED:
        {
            m_GreedRolls.insert( std::make_pair(pInfo->guid, roll) );
            data << uint8(roll) << uint8(GREED);
        }break;
    case DISENCHANT:
        {
            /*if(acceptableroll)
            {
                if(proto->DisenchantReqSkill < 0)
                    acceptableroll = false;
                else if(pInfo->m_Group)
                    acceptableroll = pInfo->m_Group->HasAcceptableDisenchanters(proto->DisenchantReqSkill);
                else
                    acceptableroll = (pInfo->m_loggedInPlayer->_HasSkillLine(333) && (pInfo->m_loggedInPlayer->_GetSkillLineCurrent(333) > uint32(proto->DisenchantReqSkill)));
            }*/

            if(acceptableroll)
            {
                m_DisenchantRolls.insert( std::make_pair(pInfo->guid, roll) );
                data << uint8(roll) << uint8(DISENCHANT);
            }
            else
            {
                m_passRolls.insert( pInfo->guid );
                data << uint8(128) << uint8(128);
            }
        }break;
    default: //pass
        {
            m_passRolls.insert( pInfo->guid );
            data << uint8(128) << uint8(128);
        }break;
    }

    data << uint8(0);

    if(pInfo->m_Group)
        pInfo->m_Group->SendPacketToAll(&data);
    else if(pInfo->m_loggedInPlayer)
        pInfo->m_loggedInPlayer->GetSession()->SendPacket(&data);

    // check for early completion
    if(!--_remaining)
    {
        mLootLock.Release(); // so we can call the other lock in a sec.
        // kill event early
        //sEventMgr.RemoveEvents(this);
        Finalize();
        return;
    }

    mLootLock.Release();
}

int32 LootRoll::event_GetInstanceID()
{
    return _mgr->GetInstanceID();
}

void LootMgr::FillObjectLootMap(map<uint32, vector<uint32> > *dest)
{
    sLog.Debug("LootMgr","Generating object loot map...");
    QueryResult *result = WorldDatabase.Query("SELECT entryid, itemid FROM objectloot");
    if( result != NULL )
    {
        do
        {
            uint32 itemid = result->Fetch()[1].GetUInt32();
            uint32 objid = result->Fetch()[0].GetUInt32();

            map<uint32, vector<uint32> >::iterator vtr = dest->find(itemid);
            if( vtr == dest->end() )
            {
                vector<uint32> tv;
                tv.push_back(objid);
                dest->insert(make_pair(itemid, tv));
            }
            else
                vtr->second.push_back(objid);
        } while (result->NextRow());
        delete result;
    }
}

bool Loot::HasLoot(Player* Looter)
{
    // check gold
    if( gold > 0 )
        return true;

    return HasItems(Looter);
}

bool Loot::HasItems(Player* Looter)
{
    // check items
    for(vector<__LootItem>::iterator itr = items.begin(); itr != items.end(); itr++)
    {
        // FFA CHECKS!
        if(itr->ffa_loot == 2)
        {
            // This is assuming that creatures with items like these are not minable/skinnable, so players can't fuck each other over.
            if(itr->has_looted.find(Looter->GetLowGUID()) != itr->has_looted.end())
                continue;
        }
        else if(itr->has_looted.size())
            continue;

        ItemPrototype * proto = itr->item.itemproto;
        if(proto == NULL)
            continue;
        //quest items check. type 4/5
        //quest items that dont start quests.
        if((proto->Bonding == ITEM_BIND_QUEST) && !(proto->QuestId) && !Looter->HasQuestForItem(proto->ItemId))
            continue;
        if((proto->Bonding == ITEM_BIND_QUEST2) && !(proto->QuestId) && !Looter->HasQuestForItem(proto->ItemId))
            continue;

        //quest items that start quests need special check to avoid drops all the time.
        if((proto->Bonding == ITEM_BIND_QUEST) && (proto->QuestId) && Looter->GetQuestLogForEntry(proto->QuestId))
            continue;
        if((proto->Bonding == ITEM_BIND_QUEST2) && (proto->QuestId) && Looter->GetQuestLogForEntry(proto->QuestId))
            continue;
        if((proto->Bonding == ITEM_BIND_QUEST) && (proto->QuestId) && Looter->HasFinishedQuest(proto->QuestId))
            continue;
        if((proto->Bonding == ITEM_BIND_QUEST2) && (proto->QuestId) && Looter->HasFinishedQuest(proto->QuestId))
            continue;

        //check for starting item quests that need questlines.
        if((proto->QuestId && proto->Bonding != ITEM_BIND_QUEST && proto->Bonding != ITEM_BIND_QUEST2))
        {
            bool HasRequiredQuests = true;
            Quest * pQuest = sQuestMgr.GetQuestPointer(proto->QuestId);
            if(pQuest == NULL)
                continue;

            //check if its a questline.
            for(uint32 i = 0; i < pQuest->count_requiredquests; i++)
            {
                if(pQuest->required_quests[i])
                {
                    if(Looter->HasFinishedQuest(pQuest->required_quests[i]) || !Looter->GetQuestLogForEntry(pQuest->required_quests[i]))
                    {
                        HasRequiredQuests = false;
                        break;
                    }
                }
            }

            if(!HasRequiredQuests)
                continue;
        }
        return true;
    }
    return false;
}

