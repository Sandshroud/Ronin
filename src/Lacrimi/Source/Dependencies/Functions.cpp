/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

Creature* CreateAndLoadCreature(MapMgr* mgr, uint32 entry, uint32 instancemode, float x, float y, float z, float o, int32 phase, bool push)
{
    if(mgr == NULL)
        return NULL;

    if(entry == 0)
        return NULL;

    if(x == 0.0f && y == 0.0f)
        return NULL;

    if(sCreatureDataMgr.GetCreatureData(entry) == NULL)
        return NULL;

    if(instancemode > 3)
        instancemode = 0;

    Creature* ctr = mgr->CreateCreature(entry);
    if(ctr == NULL)
        return NULL;

    ctr->Load(instancemode, x, y, z, o);
    if(push == true)
        ctr->PushToWorld(mgr);
    return ctr;
}

GameObject* CreateAndLoadGameObject( MapMgr* mgr, uint32 entry, float x, float y, float z, float o, int32 phase, bool push)
{
    if(mgr == NULL)
        return NULL;

    if(entry == 0)
        return NULL;

    if(x == 0.0f && y == 0.0f)
        return NULL;

    GameObject* GO = mgr->CreateGameObject(entry);
    GO->SetInstanceID(mgr->GetInstanceID());
    if(!GO->CreateFromProto(entry, mgr->GetMapId(), x, y, z, o))
        return NULL;
    if(push) GO->PushToWorld(mgr);
    return GO;
}

void WaypointCreate(Creature *pCreature , float x, float y, float z, float o, uint32 waittime, uint32 flags, uint32 modelid)
{
    if(pCreature == NULL)
        return;

    if(!pCreature->m_custom_waypoint_map)
    pCreature->m_custom_waypoint_map = new WayPointMap;

    if(!modelid)
        modelid = pCreature->GetUInt32Value(UNIT_FIELD_DISPLAYID);

    WayPoint * wp = new WayPoint;
    wp->id = uint32(pCreature->m_custom_waypoint_map->size() ? pCreature->m_custom_waypoint_map->size() + 1 : 1);
    wp->x = x;
    wp->y = y;
    wp->z = z;
    wp->orientation = o;
    wp->flags = flags;
    wp->forwardInfo = NULL;
    wp->backwardInfo = NULL;
    wp->waittime = waittime;

    if(wp->id <= 0)
        return; //not valid id

    if(pCreature->m_custom_waypoint_map->size() <= wp->id)
        pCreature->m_custom_waypoint_map->resize(wp->id+1);

    if((*pCreature->m_custom_waypoint_map)[wp->id] == NULL)
        (*pCreature->m_custom_waypoint_map)[wp->id] = wp;

}

void EnableWaypoints(Creature *creat)
{
    if(!creat)
        return;
    if(!creat->m_custom_waypoint_map)
        return;

    creat->GetAIInterface()->SetWaypointMap(creat->m_custom_waypoint_map);
}

void DeleteWaypoints(Creature *creat)
{
    if(creat == NULL)
        return;

    if(creat->m_custom_waypoint_map == NULL)
        return;

    WayPointMap::iterator i = creat->m_custom_waypoint_map->begin();

    for(; i != creat->m_custom_waypoint_map->end(); ++i)
    {
        if((*i) != NULL)
            delete (*i);
    }

    creat->m_custom_waypoint_map->clear();
}

void DeleteSpawned(Creature *creat)
{
    if(creat == NULL)
        return;

    creat->Despawn(0, 0);
    creat->SafeDelete();
}

void GameobjectDelete(GameObject *pC, uint32 duration)
{
    if(pC == NULL)
        return;

    pC->ExpireAndDelete(duration);
}

void MoveToPlayer(Player *plr, Creature *creat)
{
    if(plr == NULL)
        return;

    if(creat == NULL)
        return;

    creat->GetAIInterface()->MoveTo(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ());
}

void CreateCustomWaypointMap(Creature *creat)
{
    if(creat == NULL)
        return;

    creat->DestroyCustomWaypointMap();

    creat->m_custom_waypoint_map = new WayPointMap;
    creat->GetAIInterface()->SetWaypointMap(creat->m_custom_waypoint_map);
}

bool AddItem( uint32 pEntry, Player *pPlayer, uint32 pCount)
{
    if ( pPlayer == NULL|| pEntry == 0 || pCount == 0 )
        return false;

    Item *ItemStack = pPlayer->GetInventory()->FindItemLessMax( pEntry, pCount, false );
    if ( ItemStack == NULL)
    {
        ItemPrototype* ItemProto = sItemMgr.LookupEntry( pEntry );
        if ( ItemProto == NULL )
            return false;

        SlotResult Result = pPlayer->GetItemInterface()->FindFreeInventorySlot( ItemProto );
        if ( !Result.Result )
        {
            pPlayer->GetItemInterface()->BuildInventoryChangeError( NULL, NULL, INV_ERR_INVENTORY_FULL );
            return false;
        }
        else
        {
            Item *NewItem = objmgr.CreateItem( pEntry, pPlayer );
            if ( NewItem == NULL )
                return false;

            if( ItemProto->MaxCount > 0 && (uint32)ItemProto->MaxCount < pCount )
                pCount = ItemProto->MaxCount;

            NewItem->SetUInt32Value( ITEM_FIELD_STACK_COUNT, pCount );
            if ( pPlayer->GetItemInterface()->SafeAddItem( NewItem, Result.ContainerSlot, Result.Slot ) == ADD_ITEM_RESULT_ERROR )
            {
                NewItem->Destruct();
                NewItem = NULL;
                return false;
            }

            return true;
        }
    }
    ItemStack->SetCount( ItemStack->GetUInt32Value( ITEM_FIELD_STACK_COUNT ) + pCount );
    return true;
}

void EventCreatureDelete(Creature *creat, uint32 time)  // Creature and time in ms
{
    sEventMgr.AddEvent(creat, &Creature::SafeDelete, EVENT_CREATURE_SAFE_DELETE, time, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void EventCastSpell(Unit *caster, Unit *target, uint32 spellid, uint32 time)
{
    sEventMgr.AddEvent(castPtr<Unit>(caster), &Unit::EventCastSpell, castPtr<Unit>(target), dbcSpell.LookupEntry(spellid), EVENT_UNK, time, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void EventPlaySound(Creature *creat, uint32 id, uint32 time)
{
    sEventMgr.AddEvent(castPtr<WorldObject>(creat), &WorldObject::PlaySoundToSet, id, EVENT_UNK, time, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void EventCreatureSay(Creature *creat, std::string say, uint32 time)
{
    sEventMgr.AddEvent(castPtr<Unit>(creat), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_SAY, (uint32)LANG_UNIVERSAL, say.c_str(), EVENT_UNIT_CHAT_MSG, time, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void EventCreatureYell(Creature *creat, std::string say, uint32 time)
{
    sEventMgr.AddEvent(castPtr<Unit>(creat), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_YELL, (uint32)LANG_UNIVERSAL, say.c_str(), EVENT_UNIT_CHAT_MSG, time, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

Creature *getNpcQuester(Player *plr, uint32 npcid)
{
    if( plr == NULL || !plr->IsInWorld() )
        return NULL;
    return castPtr<Creature>(plr->GetMapMgr()->GetObjectClosestToCoords(npcid, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 99999.0f, TYPEID_UNIT));
}

GameObject *getGOQuester(Player *plr, uint32 goid)
{
    if( plr == NULL || !plr->IsInWorld() )
        return NULL;
    return castPtr<GameObject>(plr->GetMapMgr()->GetObjectClosestToCoords(goid, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 99999.0f, TYPEID_GAMEOBJECT));
}

QuestLogEntry* GetQuest( Player* pPlayer, uint32 pQuestId )
{
    return pPlayer->GetQuestLogForEntry(pQuestId);
}

void KillMobForQuest( Player* pPlayer, QuestLogEntry* pQuest, uint32 pRequiredMobCount )
{
    if ( pPlayer == NULL )
        return;
        
    uint32 i = pRequiredMobCount;
    if ( !pQuest || pQuest->GetQuest() == NULL )
        return;

    if ( pQuest->GetObjectiveCount( i ) < pQuest->GetQuest()->required_mobcount[i] )
    {
        pQuest->SetObjectiveCount( i, pQuest->GetObjectiveCount( i ) + 1 );
        pQuest->SendUpdateAddKill( i );
        pQuest->UpdatePlayerFields();
    }
}

void KillMobForQuest( Player* pPlayer, uint32 pQuestId, uint32 pRequiredMobCount )
{
    if ( pPlayer == NULL )
        return;
        
    uint32 i = pRequiredMobCount;
    QuestLogEntry* pQuest = GetQuest( pPlayer, pQuestId );
    if ( !pQuest || pQuest->GetQuest() == NULL )
        return;

    if ( pQuest->GetObjectiveCount( i ) < pQuest->GetQuest()->required_mobcount[i] )
    {
        pQuest->SetObjectiveCount( i, pQuest->GetObjectiveCount( i ) + 1 );
        pQuest->SendUpdateAddKill( i );
        pQuest->UpdatePlayerFields();
    }
}
