/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#ifdef M_PI
#undef M_PI
#endif

#define M_PI 3.14159265358979323846f

Creature::Creature(CreatureData *data, uint64 guid) : Unit(guid), _creatureData(data)
{
    SetEntry(data->entry);

    m_spawn = NULL;
    m_quests = NULL;
    IP_shield = NULL;
    m_SellItems = NULL;
    mTrainer = NULL;
    auctionHouse = NULL;
    m_escorter = NULL;
    m_respawnCell = NULL;
    myFamily = NULL;

    mTaxiNode = 0;
    m_H_regenTimer = 0;
    m_P_regenTimer = 0;
    m_p_DelayTimer = 0;
    m_enslaveCount = 0;
    m_enslaveSpell = 0;

    m_TaxiNode = 0;

    m_custom_waypoint_map = 0;
    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = -1;

    Skinned = false; // 0x02
    b_has_shield = false; // 0x04
    m_noDeleteAfterDespawn = false; // 0x08
    m_noRespawn = false; // 0x10
    m_isGuard = false; // 0x20
    m_canRegenerateHP = true; // 0x40
    m_limbostate = false; // 0x80
    m_corpseEvent=false; // 0x100
    m_PickPocketed = false; // 0x200
    haslinkupevent = false; // 0x400
    has_waypoint_text = has_combat_text = false; // 0x800  // 0x1000
}

Creature::~Creature()
{

}

void Creature::Init()
{
    Unit::Init();
    if(uint32 vehicleKitId = _creatureData->vehicleEntry)
        InitVehicleKit(vehicleKitId);
}

void Creature::Destruct()
{
    sEventMgr.RemoveEvents(this);

    if(m_escorter)
        m_escorter = NULL;

    if(m_custom_waypoint_map != 0)
    {
        for(WayPointMap::iterator itr = m_custom_waypoint_map->begin(); itr != m_custom_waypoint_map->end(); itr++)
            delete (*itr);
        delete m_custom_waypoint_map;
    }
    if(m_respawnCell != NULL)
        m_respawnCell->_respawnObjects.erase(this);
    Unit::Destruct();
}

void Creature::Update( uint32 p_time )
{
    Unit::Update( p_time );
    if(IsTotem() && isDead())
    {
        RemoveFromWorld(false, true);
        return;
    }

    if(m_corpseEvent)
    {
        sEventMgr.RemoveEvents(this);
        if (_creatureData->rank == ELITE_WORLDBOSS || _creatureData->flags & CREATURE_FLAGS1_BOSS)
            sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_BOSSCORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else if ( _creatureData->rank == ELITE_RAREELITE || _creatureData->rank == ELITE_RARE)
            sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_RARECORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_CORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        m_corpseEvent = false;
    }
}

int32 Creature::GetBaseAttackTime(uint8 weaponType)
{
    switch(weaponType)
    {
    case 0: return _creatureData->attackTime;
    case 1: return 0;
    case 2: return _creatureData->rangedAttackTime;
    }
    return 2000;
}

void Creature::SafeDelete()
{
    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::DeleteMe, EVENT_CREATURE_SAFE_DELETE, 1000, 1, 0);
}

void Creature::DeleteMe()
{
    if(IsInWorld())
        RemoveFromWorld(false, true);

    Destruct();
}

void Creature::OnRemoveCorpse()
{
    // time to respawn!
    if (IsInWorld() && (int32)m_mapMgr->GetInstanceID() == m_instanceId)
    {

        sLog.Debug("Creature","OnRemoveCorpse Removing corpse of "I64FMT"...", GetGUID());

        if(((_extraInfo && _extraInfo->isBoss) && GetMapMgr()->GetMapInfo() && GetMapMgr()->GetdbcMap()->IsRaid()) || m_noRespawn)
            RemoveFromWorld(false, true);
        else if(_creatureData->respawnTime)
            RemoveFromWorld(true, false);
        else RemoveFromWorld(false, true);

        SetDeathState(DEAD);
        SetPosition(GetSpawnX(), GetSpawnY(), GetSpawnZ(), GetSpawnO());
    }
}

void Creature::OnRespawn( MapMgr* m)
{
    sLog.outDebug("Respawning "I64FMT"...", GetGUID());
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
    SetUInt32Value(UNIT_NPC_FLAGS, _creatureData->NPCFLags);

    Skinned = false;
    m_taggingGroup = m_taggingPlayer = 0;
    m_lootMethod = -1;

    SetDeathState(ALIVE);
    GetAIInterface()->StopMovement(0); // after respawn monster can move
    m_PickPocketed = false;
    PushToWorld(m);
}

///////////
/// Looting
uint32 Creature::GetRequiredLootSkill()
{
    if(GetCreatureData()->flags & CREATURE_FLAGS1_HERBLOOT)
        return SKILL_HERBALISM;     // herbalism
    else if(GetCreatureData()->flags & CREATURE_FLAGS1_MININGLOOT)
        return SKILL_MINING;        // mining
    else if(GetCreatureData()->flags & CREATURE_FLAGS1_ENGINEERLOOT)
        return SKILL_ENGINERING;
    return SKILL_SKINNING;      // skinning
}

void Creature::GenerateLoot()
{
    if(IsPet())
    {
        GetLoot()->gold = 0;
        return;
    }

    uint8 team = 0;
    uint8 difficulty = (m_mapMgr ? (m_mapMgr->iInstanceMode) : 0);
    if(CombatStatus.GetKiller() && CombatStatus.GetKiller()->IsPlayer())
        team = castPtr<Player>(CombatStatus.GetKiller())->GetTeam();

    lootmgr.FillCreatureLoot(GetLoot(), GetEntry(), difficulty, team);

    // -1 , no gold; 0 calculated according level; >0 coppercoins
    if( _creatureData->money == -1)
    {
        GetLoot()->gold = 0;
        return;
    }
    else if(_creatureData->money == 0)
    {
        CreatureData *info = GetCreatureData();
        if (info && info->type != BEAST)
        {
            if(GetUInt32Value(UNIT_FIELD_MAXHEALTH) <= 1667)
                GetLoot()->gold = uint32((info->rank+1)*getLevel()*((rand()%5) + 1)); //generate copper
            else GetLoot()->gold = uint32((info->rank+1)*getLevel()*((rand()%5) + 1)*(GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.0006)); //generate copper
        } else GetLoot()->gold = 0; // Beasts don't drop money
    } else GetLoot()->gold = uint32(_creatureData->money);
    if(GetLoot()->gold)
        GetLoot()->gold = float2int32(floor(float(GetLoot()->gold) * sWorld.getRate(RATE_MONEY)));
}

void Creature::SaveToDB(bool saveposition /*= false*/)
{
    if(IsPet() || IsSummon()) //Just in case.
        return;

    bool newSpawn = !IsSpawn();
    if(newSpawn)
        (m_spawn = new CreatureSpawn())->id = objmgr.GenerateCreatureSpawnID();
    else if(m_spawn->id == 0)
        m_spawn->id = objmgr.GenerateCreatureSpawnID();

    m_spawn->entry = GetEntry();
    m_spawn->x = (!saveposition && (m_spawn != NULL)) ? m_spawn->x : m_position.x;
    m_spawn->y = (!saveposition && (m_spawn != NULL)) ? m_spawn->y : m_position.y;
    m_spawn->z = (!saveposition && (m_spawn != NULL)) ? m_spawn->z : m_position.z;
    m_spawn->o = (!saveposition && (m_spawn != NULL)) ? m_spawn->o : m_position.o;
    m_spawn->modelId = GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID);
    m_spawn->vendormask = newSpawn ? 0x01 : GetVendorMask();

    std::stringstream ss;
    ss << "REPLACE INTO creature_spawns VALUES("
        << uint32(m_spawn->id) << ","
        << uint32(m_spawn->entry) << ","
        << uint32(GetMapId()) << ","
        << float(m_spawn->x) << ","
        << float(m_spawn->y) << ","
        << float(m_spawn->z) << ","
        << float(m_spawn->o) << ","
        << uint32(m_spawn->modelId) << ","
        << int32(m_spawn->vendormask) << " )";

    WorldDatabase.Execute(ss.str().c_str());
    if(newSpawn && IsInWorld())
        GetMapMgr()->AddSpawn(GetMapMgr()->GetPosX(m_spawn->x), GetMapMgr()->GetPosY(m_spawn->y), m_spawn);
}

void Creature::DeleteFromDB()
{
    if( m_spawn == NULL )
        return;

    WorldDatabase.Execute("DELETE FROM creature_spawns WHERE id=%u",  GetSQL_id());
    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE spawnid=%u",  GetSQL_id());
}

/////////////
/// Quests

void Creature::AddQuest(QuestRelation *Q)
{
    m_quests->push_back(Q);
}

void Creature::DeleteQuest(QuestRelation *Q)
{
    std::list<QuestRelation *>::iterator it;
    for ( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        if((*it) != NULL)
        {
            if (((*it)->type == Q->type) && ((*it)->qst == Q->qst ))
                delete (*it);
        }
    }

    m_quests->clear();
}

Quest* Creature::FindQuest(uint32 quest_id, uint8 quest_relation)
{
    std::list<QuestRelation *>::iterator it;
    for (it = m_quests->begin(); it != m_quests->end(); it++)
    {
        QuestRelation *ptr = (*it);

        if ((ptr->qst->id == quest_id) && (ptr->type & quest_relation))
        {
            return ptr->qst;
        }
    }
    return NULL;
}

uint16 Creature::GetQuestRelation(uint32 quest_id)
{
    uint16 quest_relation = 0;
    std::list<QuestRelation *>::iterator it;

    for (it = m_quests->begin(); it != m_quests->end(); it++)
    {
        if ((*it)->qst->id == quest_id)
        {
            quest_relation |= (*it)->type;
        }
    }
    return quest_relation;
}

uint32 Creature::NumOfQuests()
{
    return (uint32)m_quests->size();
}

void Creature::_LoadQuests()
{
    sQuestMgr.LoadNPCQuests(castPtr<Creature>(this));
}

void Creature::SetDeathState(DeathState s)
{
    Unit::SetDeathState(s);
    if(s == JUST_DIED)
    {
        GetAIInterface()->SetUnitToFollow(NULL);

        //despawn all summons we created
        SummonExpireAll(true);

        if(m_enslaveSpell)
            RemoveEnslave();

        if(m_currentSpell)
            m_currentSpell->cancel();

        m_deathState = CORPSE;
        m_corpseEvent = true;
        if ( lootmgr.IsSkinnable(GetEntry()))
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    }
}

bool Creature::CanAddToWorld()
{
    if(_creatureData == NULL)
        return false;

    // force set faction
    if(GetFaction() == NULL)
        return false;

    return true;
}

void Creature::OnPushToWorld()
{
    for(std::set<uint32>::iterator itr = _creatureData->Auras.begin(); itr != _creatureData->Auras.end(); itr++)
        if(SpellEntry *sp = dbcSpell.LookupEntry((*itr)))
            CastSpell(this, sp, true);

    Unit::OnPushToWorld();

    if(m_spawn)
    {
        if(m_aiInterface.GetFormationSQLId())
        {
            // add event
            sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::FormationLinkUp, m_aiInterface.GetFormationSQLId(), EVENT_CREATURE_FORMATION_LINKUP, 1000, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            haslinkupevent = true;
        }
    }

    m_aiInterface.m_is_in_instance = (m_mapMgr->GetMapInfo()->type!=INSTANCE_NULL) ? true : false;
    if (HasItems())
    {
        for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
        {
            if (itr->second.max_amount == 0)
                itr->second.available_amount=0;
            else if (itr->second.available_amount < itr->second.max_amount)
                sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::UpdateItemAmount, itr->second.itemid, EVENT_ITEM_UPDATE, VENDOR_ITEMS_UPDATE_TIME, 1,0);
        }
    }

    if(GetAreaFlags() & OBJECT_AREA_FLAG_INSANCTUARY)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
}

void Creature::RemoveFromWorld(bool addrespawnevent, bool free_guid)
{
    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = 1;

    m_AuraInterface.RemoveAllAuras();

    if(IsPet())
    {
        if(IsInWorld())
            Unit::RemoveFromWorld(true);

        SafeDelete();
    }
    else
    {
        uint32 delay = 0;
        if(addrespawnevent && _creatureData->respawnTime > 0)
            delay = _creatureData->respawnTime;
        Despawn(0, delay);
    }
}

void Creature::EnslaveExpire()
{
    m_enslaveCount++;
    Player* caster = objmgr.GetPlayer(GetUInt32Value(UNIT_FIELD_CHARMEDBY));
    if(caster)
    {
        caster->SetUInt64Value(UNIT_FIELD_CHARM, 0);
        caster->SetUInt64Value(UNIT_FIELD_SUMMON, 0);
        WorldPacket data(8);
        data.Initialize(SMSG_PET_SPELLS);
        data << uint64(0);
        caster->GetSession()->SendPacket(&data);
    }

    SetUInt64Value(UNIT_FIELD_CHARMEDBY, 0);
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, 0);

    SetFaction(_creatureData->faction, false);
    GetAIInterface()->Init(this, AITYPE_AGRO, MOVEMENTTYPE_NONE);
}

bool Creature::RemoveEnslave()
{
    return RemoveAura(m_enslaveSpell);
}

void Creature::AddInRangeObject(WorldObject* pObj)
{
    Unit::AddInRangeObject(pObj);
}

void Creature::OnRemoveInRangeObject(WorldObject* pObj)
{
    if(m_escorter == pObj)
    {
        // we lost our escorter, return to the spawn.
        m_aiInterface.StopMovement(10000);
        DestroyCustomWaypointMap();
        Despawn(1000, 1000);
    }

    Unit::OnRemoveInRangeObject(pObj);
}

void Creature::ClearInRangeSet()
{
    Unit::ClearInRangeSet();
}

void Creature::RegenerateHealth(bool isinterrupted)
{
    if(m_limbostate || !m_canRegenerateHP || isinterrupted)
        return;

    uint32 cur = GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 mh = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(cur >= mh)
        return;
    if(GetAIInterface()->getAIState() == STATE_EVADE)
    {
        //though creatures have their stats we use some wierd formula for amt
        float amt = getLevel()*2.0f;
        if(amt <= 1.0f)//this fixes regen like 0.98
            cur++;
        else cur += (uint32)amt;

        SetUInt32Value(UNIT_FIELD_HEALTH, (cur >= mh) ? mh : cur);
        return;
    }

    cur += float2int32(ceil(float(mh)/3.f));
    if(cur >= mh || (mh-cur < mh/8)) cur = mh;
    SetUInt32Value(UNIT_FIELD_HEALTH, cur);
}

void Creature::RegenerateMana(bool isinterrupted)
{
    if (m_interruptRegen)
        return;

    uint32 cur = GetPower(POWER_TYPE_MANA);
    uint32 mm = GetMaxPower(POWER_TYPE_MANA);
    if(cur >= mm)
        return;

    if(isinterrupted || GetAIInterface()->getAIState() == STATE_EVADE)
    {
        float amt = (getLevel()+10);
        if(amt <= 1.0)//this fixes regen like 0.98
            cur++;
        else cur += (uint32)amt;
        SetPower(POWER_TYPE_MANA, (cur>=mm)?mm:cur);
        return;
    }

    cur += float2int32(ceil(float(mm)/10.f));
    if(cur >= mm || (mm-cur < mm/16)) cur = mm;
    SetPower(POWER_TYPE_MANA, cur);
}

void Creature::AddVendorItem(uint32 itemid, uint32 amount, uint32 vendormask, uint32 ec)
{
    CreatureItem ci;
    memset(&ci, 0, sizeof(CreatureItem));
    ci.amount = amount;
    ci.itemid = itemid;
    ci.IsDependent = true;
    ci.vendormask = vendormask;
    ci.extended_cost = NULL;
    if(ec)
        ci.extended_cost = dbcItemExtendedCost.LookupEntry(ec);

    if(!m_SellItems)
    {
        m_SellItems = new std::map<uint32, CreatureItem>;
        objmgr.SetVendorList(GetEntry(), m_SellItems);
    }

    uint32 slot = 1;
    if(m_SellItems->size())
        slot = m_SellItems->rbegin()->first+1;
    m_SellItems->insert(std::make_pair(slot, ci));
}

void Creature::ModAvItemAmount(uint32 itemid, uint32 value)
{
    for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
    {
        if(itr->second.itemid == itemid)
        {
            if(itr->second.available_amount)
            {
                if(value > itr->second.available_amount)    // shouldnt happen
                {
                    itr->second.available_amount=0;
                    return;
                }
                else
                    itr->second.available_amount -= value;

                if(!event_HasEvent(EVENT_ITEM_UPDATE))
                    sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::UpdateItemAmount, itr->second.itemid, EVENT_ITEM_UPDATE, itr->second.incrtime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            return;
        }
    }
}

void Creature::UpdateItemAmount(uint32 itemid)
{
    for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
    {
        if(itr->second.itemid == itemid)
        {
            if (itr->second.max_amount==0)      // shouldnt happen
                itr->second.available_amount=0;
            else
            {
                itr->second.available_amount = itr->second.max_amount;
            }
            return;
        }
    }
}

bool Creature::isBoss()
{
    return false;
}

bool Creature::isCivilian()
{
    return false;
}

void Creature::FormationLinkUp(uint32 SqlId)
{
    if(!m_mapMgr)       // shouldnt happen
        return;

    if( Creature *creature = m_mapMgr->GetSqlIdCreature(SqlId) )
    {
        m_aiInterface.SetFormationLinkTarget(creature);
        haslinkupevent = false;
        event_RemoveEvents(EVENT_CREATURE_FORMATION_LINKUP);
    }
}

void Creature::Load(uint32 mapId, float x, float y, float z, float o, uint32 mode, CreatureSpawn *spawn)
{
    // Set our spawn pointer
    m_spawn = spawn;

    // Set worldobject Create data
    _Create(mapId, x, y, z, o);
    // Set our extra data pointer
    _extraInfo = CreatureInfoExtraStorage.LookupEntry(GetEntry());

    uint32 model = 0, gender=0;
    _creatureData->VerifyModelInfo(model, gender);

    uint32 level = _creatureData->minLevel;
    if(_creatureData->maxLevel > _creatureData->minLevel)
        level += RandomUInt(_creatureData->maxLevel-_creatureData->minLevel);

    //Set fields
    for(uint8 i = 0; i < 7; i++)
        SetUInt32Value(UNIT_FIELD_RESISTANCES+i, _creatureData->resistances[i]);

    uint8 race = RACE_HUMAN;
    if(CreatureDisplayInfoEntry *displayEntry = dbcCreatureDisplayInfo.LookupEntry(model))
        if(CreatureDisplayInfoExtraEntry *extraInfo = dbcCreatureDisplayInfoExtra.LookupEntry(displayEntry->ExtraDisplayInfoEntry))
            race = extraInfo->Race;

    SetByte(UNIT_FIELD_BYTES_0, 0, race);
    SetByte(UNIT_FIELD_BYTES_0, 1, _creatureData->Class);
    SetByte(UNIT_FIELD_BYTES_0, 2, gender);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, model);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, model);
    EventModelChange();

    setLevel(level);
    SetFloatValue(OBJECT_FIELD_SCALE_X, _creatureData->scale);
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, _creatureData->attackTime);
    SetFloatValue(UNIT_FIELD_MINDAMAGE, _creatureData->minDamage);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, _creatureData->maxDamage);
    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, _creatureData->rangedAttackTime);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, _creatureData->rangedMinDamage);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, _creatureData->rangedMaxDamage);

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, _creatureData->inventoryItem[0]);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, _creatureData->inventoryItem[1]);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, _creatureData->inventoryItem[2]);

    SetFactionTemplate(_creatureData->faction);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, _creatureData->boundingRadius * _creatureData->scale);
    SetFloatValue(UNIT_FIELD_COMBATREACH, _creatureData->combatReach * _creatureData->scale);

    UpdateFieldValues();

    // Max out health
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));

    // set position
    if(WayPointMap *WPMap = (m_spawn ? objmgr.GetWayPointMap(m_spawn->id) : NULL))
        m_aiInterface.SetWaypointMap(WPMap);

    //use proto faction if spawn faction is unspecified
    if(m_factionTemplate = dbcFactionTemplate.LookupEntry(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE)))
    {
        // not a neutral creature
        FactionEntry *faction = m_factionTemplate->GetFaction();
        if(faction && (!(faction->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0)))
            GetAIInterface()->m_canCallForHelp = true;
    } else sLog.Warning("Creature", "Creature is missing a valid faction template for entry %u.", GetEntry());

    //SETUP NPC FLAGS
    SetUInt32Value(UNIT_NPC_FLAGS, _creatureData->NPCFLags);

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ) )
        m_SellItems = objmgr.GetVendorList(GetEntry());

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ) )
        _LoadQuests();

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR) )
        m_TaxiNode = sTaxiMgr.GetNearestTaxiNode( m_position.x, m_position.y, m_position.z, GetMapId() );

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER) || HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER_PROF))
        mTrainer = objmgr.GetTrainer(GetEntry());

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ) )
        auctionHouse = sAuctionMgr.GetAuctionHouse(GetEntry());

    ////////////AI
    m_aiInterface.InitalizeExtraInfo(_creatureData, _extraInfo, mode);

    // load formation data
    Formation* form = m_spawn ? sFormationMgr.GetFormation(m_spawn->id) : NULL;
    m_aiInterface.SetFormationSQLId(form ? form->fol : 0);
    m_aiInterface.SetFormationFollowDistance(form ? form->dist : 0.f);
    m_aiInterface.SetFormationFollowAngle(form ? form->ang : 0.f);

    //////////////AI
    myFamily = dbcCreatureFamily.LookupEntry(_creatureData->family);

    has_combat_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_ENTER_COMBAT);
    has_waypoint_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_RANDOM_WAYPOINT);
    m_aiInterface.m_isGuard = isGuard(GetEntry());

    //CanMove (overrules AI)
    if(!GetCanMove())
        GetMovementInterface()->setRooted(true);

    m_invisFlag = _creatureData->invisType;

    if(uint32 tmpitemid = _creatureData->inventoryItem[0])
    {
        if(ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid))
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
                b_has_shield = (IP_shield = sItemMgr.LookupEntry(tmpitemid)) != NULL;
        }
    }

    if(uint32 tmpitemid = _creatureData->inventoryItem[1])
    {
        if(ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid))
        {
            if(!b_has_shield && DBCItem->InventoryType == INVTYPE_SHIELD)
                b_has_shield = (IP_shield = sItemMgr.LookupEntry(tmpitemid)) != NULL;
        }
    }
}

void Creature::Despawn(uint32 delay, uint32 respawntime)
{
    if(delay)
    {
        sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::Despawn, (uint32)0, respawntime, EVENT_CREATURE_RESPAWN, delay, 1,0);
        return;
    }

    GetLoot()->items.clear();

    if(!IsInWorld())
        return;

    //Better make sure all summoned totems/summons/GO's created by this creature spawned removed.
    SummonExpireAll(true);

    if(respawntime)
    {
        /* get the cell with our SPAWN location. if we've moved cell this might break :P */
        MapCell * pCell = m_mapMgr->GetCellByCoords(m_spawnLocation.x, m_spawnLocation.y);
        if(!pCell)
            pCell = m_mapCell;

        ASSERT(pCell);
        pCell->_respawnObjects.insert(this);
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnCreature, castPtr<Creature>(this), pCell, EVENT_CREATURE_RESPAWN, respawntime, 1, 0);
        Unit::RemoveFromWorld(false);
        SetPosition( m_spawnLocation);
        m_respawnCell = pCell;
    } else Unit::RemoveFromWorld(true);
}

void Creature::DestroyCustomWaypointMap()
{
    if(m_custom_waypoint_map)
    {
        m_aiInterface.SetWaypointMap(0);
        for(WayPointMap::iterator itr = m_custom_waypoint_map->begin(); itr != m_custom_waypoint_map->end(); itr++)
            delete (*itr);
        delete m_custom_waypoint_map;
        m_custom_waypoint_map = NULL;
    }
}

void Creature::RemoveLimboState(Unit* healer)
{
    if(!m_limbostate)
        return;

    m_limbostate = false;
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    bInvincible = false;
}

// Generates 3 random waypoints around the NPC
void Creature::SetGuardWaypoints()
{
    if(!GetMapMgr())
        return;

    if(!GetCreatureData())
        return;

    GetAIInterface()->setMoveType(1);
    for(int i = 1; i <= 4; i++)
    {
        float ang = rand()/100.0f;
        float ran = (rand()%(100))/10.0f;
        while(ran < 1)
            ran = (rand()%(100))/10.0f;

        WayPoint * wp = NULL;
        wp = new WayPoint;
        wp->id = i;
        wp->flags = 0;
        wp->waittime = 800;  /* these guards are antsy :P */
        wp->x = GetSpawnX()+ran*sin(ang);
        wp->y = GetSpawnY()+ran*cos(ang);

        if(canFly())
            wp->z = GetSpawnZ();
        else
            wp->z = GetCHeightForPosition(canSwim() ? false : true); // Check water heights

        wp->orientation = GetSpawnO();
        wp->forwardInfo = NULL;
        wp->backwardInfo = NULL;
        GetAIInterface()->addWayPoint(wp);
    }
}
