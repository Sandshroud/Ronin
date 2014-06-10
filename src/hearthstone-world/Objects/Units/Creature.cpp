/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#ifdef M_PI
#undef M_PI
#endif

#define M_PI       3.14159265358979323846f

Creature::Creature(uint64 guid)
{
#ifdef SHAREDPTR_DEBUGMODE
    printf("Creature::Creature()\n");
#endif
    m_valuesCount = UNIT_END;
    m_objectTypeId = TYPEID_UNIT;
    m_uint32Values = _fields;
    memset(m_uint32Values, 0,(UNIT_END)*sizeof(uint32));
    m_updateMask.SetCount(UNIT_END);
    SetUInt32Value( OBJECT_FIELD_TYPE, TYPEMASK_UNIT|TYPEMASK_OBJECT);
    SetUInt64Value( OBJECT_FIELD_GUID, guid);
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, 1);
    SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, 1);

    m_wowGuid.Init(GetGUID());

    m_quests = NULL;
    proto = NULL;
    IP_shield = NULL;
    LoadedProto = NULL;
    creature_info = NULL;
    m_SellItems = NULL;
    _myScriptClass = NULL;
    mTrainer = NULL;
    m_spawn = NULL;
    auctionHouse = NULL;
    m_escorter = NULLPLR;
    m_respawnCell = NULL;
    myFamily = NULL;

    mTaxiNode = 0;
    m_H_regenTimer = 0;
    m_P_regenTimer = 0;
    m_p_DelayTimer = 0;
    m_enslaveCount = 0;
    m_enslaveSpell = 0;

    for(uint32 x = 0; x < 7; x++)
    {
        FlatResistanceMod[x] = 0;
        BaseResistanceModPct[x] = 0;
        ResistanceModPct[x] = 0;
    }
    for(uint32 x = 0; x < 5; x++)
    {
        StatModPct[x] = 0;
        FlatStatMod[x] = 0;
        TotalStatModPct[x] = 0;
    }

    m_TaxiNode = 0;
    original_emotestate = 0;
    m_custom_waypoint_map = 0;
    m_walkSpeed = 2.5f;
    m_runSpeed = MONSTER_NORMAL_RUN_SPEED;
    m_base_runSpeed = m_runSpeed;
    m_base_walkSpeed = m_walkSpeed;
    BaseAttackType = SCHOOL_NORMAL;
    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = -1;
    spawnid = 0;

    m_useAI = true; // 0x01
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
}

void Creature::Destruct()
{
    sEventMgr.RemoveEvents(this);

    if(m_escorter)
        m_escorter = NULLPLR;

    if(_myScriptClass != 0)
        _myScriptClass->Destroy();

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
        if(proto == NULL)
            sEventMgr.AddEvent(TO_CREATURE(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else if (creature_info->Rank == ELITE_WORLDBOSS || creature_info->Flags1 & CREATURE_FLAGS1_BOSS)
            sEventMgr.AddEvent(TO_CREATURE(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_BOSSCORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else if ( creature_info->Rank == ELITE_RAREELITE || creature_info->Rank == ELITE_RARE)
            sEventMgr.AddEvent(TO_CREATURE(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_RARECORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else
            sEventMgr.AddEvent(TO_CREATURE(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_CORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        m_corpseEvent = false;
    }
}

void Creature::SafeDelete()
{
    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(TO_CREATURE(this), &Creature::DeleteMe, EVENT_CREATURE_SAFE_DELETE, 1000, 1, 0);
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

        if((GetMapMgr()->GetMapInfo() && GetMapMgr()->GetdbcMap()->IsRaid() && proto && proto->boss) || m_noRespawn)
        {
            RemoveFromWorld(false, true);
        }
        else
        {
            if(proto && proto->RespawnTime)
                RemoveFromWorld(true, false);
            else
                RemoveFromWorld(false, true);
        }

        setDeathState(DEAD);
        SetPosition(m_spawnLocation);
    }
}

void Creature::OnRespawn( MapMgr* m)
{
    sLog.outDebug("Respawning "I64FMT"...", GetGUID());
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0); // not tagging shiat
    if(proto)
        SetUInt32Value(UNIT_NPC_FLAGS, proto->NPCFLags);

    SetUInt32Value(UNIT_FIELD_FLAGS, original_flags);
    SetUInt32Value(UNIT_NPC_EMOTESTATE, original_emotestate);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, original_MountedDisplayID);
    Skinned = false;
    m_taggingGroup = m_taggingPlayer = 0;
    m_lootMethod = -1;

    /* creature death state */
    if(m_spawn && m_spawn->death_state == 1)
    {
        uint32 newhealth = m_uint32Values[UNIT_FIELD_HEALTH] / 100;
        if(!newhealth)
            newhealth = 1;
        SetUInt32Value(UNIT_FIELD_HEALTH, 1);
        m_limbostate = true;
        bInvincible = true;
        SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DEAD);
    }

    setDeathState(ALIVE);
    GetAIInterface()->StopMovement(0); // after respawn monster can move
    m_PickPocketed = false;
    PushToWorld(m);
}

void Creature::Create (const char* name, uint32 mapid, float x, float y, float z, float ang)
{
    Object::_Create( mapid, x, y, z, ang );
}

void Creature::CreateWayPoint (uint32 WayPointID, uint32 mapid, float x, float y, float z, float ang)
{
    Object::_Create( mapid, x, y, z, ang);
}

///////////
/// Looting
uint32 Creature::GetRequiredLootSkill()
{
    if(GetCreatureInfo()->TypeFlags & CREATURE_FLAGS1_HERBLOOT)
        return SKILL_HERBALISM;     // herbalism
    else if(GetCreatureInfo()->TypeFlags & CREATURE_FLAGS1_MININGLOOT)
        return SKILL_MINING;        // mining
    else if(GetCreatureInfo()->Flags1 & CREATURE_FLAGS1_ENGINEERLOOT)
        return SKILL_ENGINERING;
    else
        return SKILL_SKINNING;      // skinning
};

void Creature::GenerateLoot()
{
    if(IsPet() || proto == NULL)
    {
        m_loot.gold = 0;
        return;
    }

    uint8 team = 0;
    uint8 difficulty = (m_mapMgr ? (m_mapMgr->iInstanceMode) : 0);
    if(CombatStatus.GetKiller() && CombatStatus.GetKiller()->IsPlayer())
        team = TO_PLAYER(CombatStatus.GetKiller())->GetTeam();

    lootmgr.FillCreatureLoot(&m_loot, GetEntry(), difficulty, team);

    // -1 , no gold; 0 calculated according level; >0 coppercoins
    if( proto->money == -1)
    {
        m_loot.gold = 0;
        return;
    }

    if(proto->money == 0)
    {
        CreatureInfo *info = GetCreatureInfo();
        if (info && info->Type != BEAST)
        {
            if(m_uint32Values[UNIT_FIELD_MAXHEALTH] <= 1667)
                m_loot.gold = uint32((info->Rank+1)*getLevel()*((rand()%5) + 1)); //generate copper
            else
                m_loot.gold = uint32((info->Rank+1)*getLevel()*((rand()%5) + 1)*(GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.0006)); //generate copper
        }
        else //Beast don't drop money
            m_loot.gold = 0;
    }
    else
        m_loot.gold = uint32(proto->money);

    if(m_loot.gold)
        m_loot.gold = int32(float(m_loot.gold) * sWorld.getRate(RATE_MONEY));
}

void Creature::SaveToDB(bool saveposition /*= false*/)
{
    if(IsPet() || IsSummon()) //Just in case.
        return;

    bool newspawn = false;
    if(!spawnid)
        spawnid = objmgr.GenerateCreatureSpawnID();
    if(m_spawn == NULL)
    {
        newspawn = true;
        m_spawn = new CreatureSpawn();
    }

    m_spawn->id = spawnid;
    m_spawn->entry = GetEntry();
    m_spawn->x = (!saveposition && (m_spawn != NULL)) ? m_spawn->x : m_position.x;
    m_spawn->y = (!saveposition && (m_spawn != NULL)) ? m_spawn->y : m_position.y;
    m_spawn->z = (!saveposition && (m_spawn != NULL)) ? m_spawn->z : m_position.z;
    m_spawn->o = (!saveposition && (m_spawn != NULL)) ? m_spawn->o : m_position.o;
    m_spawn->factionid = m_uint32Values[UNIT_FIELD_FACTIONTEMPLATE];
    m_spawn->flags = uint32(original_flags);
    m_spawn->emote_state = uint32(original_emotestate);
    m_spawn->death_state = getDeathState();
    m_spawn->stand_state = GetStandState();
    m_spawn->phase = m_phaseMask;
    m_spawn->vehicle = IsVehicle();
    m_spawn->CanMove = GetCanMove();
    m_spawn->vendormask = VendorMask;

    std::stringstream ss;
    ss << "REPLACE INTO creature_spawns VALUES("
        << uint32(spawnid) << ","
        << uint32(m_spawn->entry) << ","
        << uint32(GetMapId()) << ","
        << float(m_spawn->x) << ","
        << float(m_spawn->y) << ","
        << float(m_spawn->z) << ","
        << float(m_spawn->o) << ","
        << uint32(m_spawn->factionid) << ","
        << uint32(m_spawn->flags) << ","
        << uint32(m_spawn->Bytes ? m_spawn->Bytes->bytes : 0) << ","
        << uint32(m_spawn->Bytes ? m_spawn->Bytes->bytes1 : 0) << ","
        << uint32(m_spawn->Bytes ? m_spawn->Bytes->bytes2 : 0) << ","
        << uint32(m_spawn->emote_state) << ","
        << uint32(m_spawn->death_state) << ", "
        << uint32(m_spawn->stand_state) << ", "
        << uint32(m_spawn->ChannelData ? m_spawn->ChannelData->channel_spell : 0) << ","
        << uint32(m_spawn->ChannelData ? m_spawn->ChannelData->channel_target_go : 0) << ","
        << uint32(m_spawn->ChannelData ? m_spawn->ChannelData->channel_target_creature : 0) << ","
        << uint32(m_spawn->MountedDisplay ? m_spawn->MountedDisplay->MountedDisplayID : 0) << ", "
        << int32(m_spawn->phase) << ", "
        << uint32(m_spawn->vehicle ? 1 : 0) << ", "
        << uint32(m_spawn->CanMove) << ", "
        << int32(m_spawn->vendormask) << " )";

    WorldDatabase.Execute(ss.str().c_str());
    if(newspawn && IsInWorld())
        GetMapMgr()->AddSpawn(GetMapMgr()->GetPosX(m_spawn->x), GetMapMgr()->GetPosY(m_spawn->y), m_spawn);
}

void Creature::LoadScript()
{
    _myScriptClass = sScriptMgr.CreateAIScriptClassForEntry(TO_CREATURE(this));
    if(_myScriptClass && _myScriptClass->LuaScript)
        sEventMgr.AddEvent(this, &Creature::UpdateAIScript, EVENT_AI_UPDATE, 50, 0, 0);
}

void Creature::UpdateAIScript()
{
    _myScriptClass->AIUpdate(NULL, 0);
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
    list<QuestRelation *>::iterator it;
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
    list<QuestRelation *>::iterator it;
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
    list<QuestRelation *>::iterator it;

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
    sQuestMgr.LoadNPCQuests(TO_CREATURE(this));
}

void Creature::CreatureSetDeathState(DeathState s)
{
    if(s == JUST_DIED)
    {
        GetAIInterface()->SetUnitToFollow(NULLUNIT);

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

void Creature::AddToWorld()
{
    if(creature_info == 0)
    {
        creature_info = CreatureNameStorage.LookupEntry(GetEntry());
        if(creature_info == 0)
            return;
    }

    // force set faction
    if(m_factionTemplate == NULL || m_faction == NULL)
    {
        _setFaction();
        if(m_factionTemplate == NULL || m_faction == NULL)
            return;
    }

    Object::AddToWorld();
}

void Creature::AddToWorld(MapMgr* pMapMgr)
{
    if(creature_info == NULL)
    {
        creature_info = CreatureNameStorage.LookupEntry(GetEntry());
        if(creature_info == NULL)
            return;
    }

    // force set faction
    if(m_factionTemplate == NULL || m_faction == NULL)
    {
        _setFaction();
        if(m_factionTemplate == NULL || m_faction == NULL)
            return;
    }

    Object::AddToWorld(pMapMgr);
}

bool Creature::CanAddToWorld()
{
    if(creature_info == NULL || proto == NULL)
        return false;

    // force set faction
    if(m_factionTemplate == NULL || m_faction == NULL)
    {
        _setFaction();
        if(m_factionTemplate == NULL || m_faction == NULL)
            return false;
    }

    return true;
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
        if(addrespawnevent && proto && proto->RespawnTime > 0)
            delay = proto->RespawnTime;
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
    SetIsPet(false);

    m_walkSpeed = m_base_walkSpeed;
    m_runSpeed = m_base_runSpeed;
    if(proto)
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, proto->Faction);
    else
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, 90);
    _setFaction();

    GetAIInterface()->Init(this, AITYPE_AGRO, MOVEMENTTYPE_NONE);

    // Update InRangeSet
    UpdateOppFactionSet();
}

bool Creature::RemoveEnslave()
{
    return RemoveAura(m_enslaveSpell);
}

void Creature::AddInRangeObject(Object* pObj)
{
    Unit::AddInRangeObject(pObj);
}

void Creature::OnRemoveInRangeObject(Object* pObj)
{
    if(m_escorter == pObj)
    {
        // we lost our escorter, return to the spawn.
        m_aiInterface->StopMovement(10000);
        DestroyCustomWaypointMap();
        Despawn(1000, 1000);
    }

    Unit::OnRemoveInRangeObject(pObj);
}

void Creature::ClearInRangeSet()
{
    Unit::ClearInRangeSet();
}

void Creature::CalcResistance(uint32 type)
{
    int32 res = (BaseResistance[type] * (100 + BaseResistanceModPct[type])) / 100;
    res += FlatResistanceMod[type];
    res += (res * ResistanceModPct[type]) / 100;
    if(type==0)res+=GetUInt32Value(UNIT_FIELD_STAT1)*2;//fix armor from agi
    SetUInt32Value(UNIT_FIELD_RESISTANCES + type, res > 0 ? res : 0);
}

void Creature::CalcStat(uint32 type)
{
    int32 res = (BaseStats[type]*(100+StatModPct[type]))/100;

    res += FlatStatMod[type];
    if(res < 0)
        res = 0;

    res += ((res*TotalStatModPct[type])/100);
    SetUInt32Value(UNIT_FIELD_STAT0+type, ((res > 0) ? res : 0));
}


void Creature::RegenerateHealth(bool isinterrupted)
{
    if(m_limbostate || !m_canRegenerateHP || isinterrupted)
        return;

    uint32 cur = GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 mh = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(cur >= mh)
        return;

    //though creatures have their stats we use some wierd formula for amt
    float amt = 0.0f;
    uint32 lvl = getLevel();

    amt = lvl*2.0f;
    if (PctRegenModifier)
        amt += (amt * PctRegenModifier) / 100;

    if(amt <= 1.0f)//this fixes regen like 0.98
        cur++;
    else
        cur += (uint32)amt;
    SetUInt32Value(UNIT_FIELD_HEALTH, (cur >= mh) ? mh : cur);
}

void Creature::RegenerateMana(bool isinterrupted)
{
    if (m_interruptRegen || isinterrupted)
        return;

    uint32 cur = GetUInt32Value(UNIT_FIELD_POWER1);
    uint32 mm = GetUInt32Value(UNIT_FIELD_MAXPOWER1);
    if(cur >= mm)
        return;

    float amt = (getLevel()+10)*PctPowerRegenModifier[POWER_TYPE_MANA]/10;
    if(amt <= 1.0)//this fixes regen like 0.98
        cur++;
    else
        cur += (uint32)amt;
    SetUInt32Value(UNIT_FIELD_POWER1,(cur>=mm)?mm:cur);
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
        m_SellItems = new map<uint32, CreatureItem>;
        objmgr.SetVendorList(GetEntry(), m_SellItems);
    }

    uint32 slot = 1;
    if(m_SellItems->size())
        slot = m_SellItems->rbegin()->first+1;
    m_SellItems->insert(make_pair(slot, ci));
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
                    sEventMgr.AddEvent(TO_CREATURE(this), &Creature::UpdateItemAmount, itr->second.itemid, EVENT_ITEM_UPDATE, itr->second.incrtime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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

void Creature::FormationLinkUp(uint32 SqlId)
{
    if(!m_mapMgr)       // shouldnt happen
        return;

    Creature* creature = NULLCREATURE;
    creature = m_mapMgr->GetSqlIdCreature(SqlId);
    if( creature != NULL )
    {
        m_aiInterface->SetFormationLinkTarget(creature);
        haslinkupevent = false;
        event_RemoveEvents(EVENT_CREATURE_FORMATION_LINKUP);
    }
}

void Creature::ChannelLinkUpGO(uint32 SqlId)
{
    if(!m_mapMgr)       // shouldnt happen
        return;

    GameObject* go = m_mapMgr->GetSqlIdGameObject(SqlId);
    if(go != NULL && m_spawn->ChannelData)
    {
        event_RemoveEvents(EVENT_CREATURE_CHANNEL_LINKUP);
        SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, go->GetGUID());
        SetUInt32Value(UNIT_CHANNEL_SPELL, m_spawn->ChannelData->channel_spell);
    }
}

void Creature::ChannelLinkUpCreature(uint32 SqlId)
{
    if(!m_mapMgr)       // shouldnt happen
        return;

    Creature* go = m_mapMgr->GetSqlIdCreature(SqlId);
    if(go != NULL && m_spawn->ChannelData)
    {
        event_RemoveEvents(EVENT_CREATURE_CHANNEL_LINKUP);
        SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, go->GetGUID());
        SetUInt32Value(UNIT_CHANNEL_SPELL, m_spawn->ChannelData->channel_spell);
    }
}

WayPoint * Creature::CreateWaypointStruct()
{
    return new WayPoint();
}

bool Creature::Load(CreatureSpawn *spawn, uint32 mode, MapInfo *info)
{
    if(m_loadedFromDB)
        return true;

    m_spawn = spawn;
    proto = CreatureProtoStorage.LookupEntry(spawn->entry);
    if(!proto)
        return false;

    creature_info = CreatureNameStorage.LookupEntry(spawn->entry);
    ExtraInfo = CreatureInfoExtraStorage.LookupEntry(proto->Id);
    if(!creature_info)
        return false;

    uint32 health = 0;
    uint32 power = 0;
    float mindmg = 0.0f;
    float maxdmg = 0.0f;
    uint32 level = 0;
    double decidingfactor[2] = { 0, 0 };

    spawnid = spawn->id;
    m_walkSpeed = m_base_walkSpeed = proto->walk_speed; //set speeds
    m_runSpeed = m_base_runSpeed = proto->run_speed; //set speeds
    m_flySpeed = proto->fly_speed;
    m_phaseMask = spawn->phase;
    VendorMask = spawn->vendormask;

    original_flags = spawn->flags;
    original_emotestate = spawn->emote_state;
    original_MountedDisplayID = spawn->MountedDisplay ? spawn->MountedDisplay->MountedDisplayID : 0;

    //Set fields
    SetUInt32Value(OBJECT_FIELD_ENTRY,proto->Id);

    // Heroic stats
    if(mode)
    {
        LoadedProto = proto->ModeProto[mode];
        if(LoadedProto != NULL)
        {
            decidingfactor[0] = LoadedProto->Maxlevel - LoadedProto->Minlevel;
            if(decidingfactor[0] == 0)
            {
                health = LoadedProto->Minhealth + RandomUInt(LoadedProto->Maxhealth - LoadedProto->Minhealth);
                mindmg = LoadedProto->Mindmg;
                maxdmg = LoadedProto->Maxdmg;
                level = LoadedProto->Minlevel;
            }
            else
            {
                decidingfactor[0] += 1;
                decidingfactor[1] = RandomUInt(decidingfactor[0]);
                if(decidingfactor[1] > 0)
                    decidingfactor[1]--;

                uint32 healthdiff = LoadedProto->Maxhealth - LoadedProto->Minhealth;
                healthdiff /= decidingfactor[0];
                health = LoadedProto->Minhealth + healthdiff*decidingfactor[1] + RandomUInt(healthdiff);

                uint32 damagediff = LoadedProto->Maxdmg-LoadedProto->Mindmg;
                damagediff /= decidingfactor[0];
                mindmg = LoadedProto->Mindmg + damagediff*decidingfactor[1];
                maxdmg = LoadedProto->Mindmg + damagediff*(decidingfactor[1]+1);
                level = LoadedProto->Minlevel + decidingfactor[1];
            }
            power = LoadedProto->Power;
            for(uint32 i = 0; i < 7; i++)
                SetUInt32Value(UNIT_FIELD_RESISTANCES + i, LoadedProto->Resistances[i]);
        }
        else
        {
            for(uint32 i = 0; i < 7; i++)
                SetUInt32Value(UNIT_FIELD_RESISTANCES+i,proto->Resistances[i]);

            float calcu = ((mode*2)/10);

            switch(mode) // TODO: find calculations
            {
            case 1: // 5 man heroic or 25 man.
                {
                    health = long2int32(double(proto->MinHealth + RandomUInt(proto->MaxHealth - proto->MinHealth)) * 1.5);
                    mindmg = proto->MinDamage * 1.2f;
                    maxdmg = proto->MaxDamage * 1.2f;
                    level = proto->MinLevel + RandomUInt(proto->MaxLevel - proto->MinLevel) + RandomUInt(10);
                    if(proto->MaxPower)
                        power = proto->MaxPower * 1.2;
                }break;

            default: // 10H or 25H
                {
                    uint64 newhealth = long2int32(double(proto->MinHealth + RandomUInt(proto->MaxHealth - proto->MinHealth)) * (1.5f + calcu));
                    if(newhealth > 4294967295) // This is the maximum uint32, its pretty much only for LK, and maybe some later cata content.
                        newhealth = 4294967295;

                    health = (uint32)newhealth;
                    mindmg = proto->MinDamage * (1.2f+calcu);
                    maxdmg = proto->MaxDamage * (1.2f+calcu);
                    level = proto->MinLevel + RandomUInt(proto->MaxLevel - proto->MinLevel) + 5 + RandomUInt(5);
                    if(proto->MaxPower)
                        power = proto->MaxPower * (1.1f+calcu);
                }break;
            }
        }
    }
    else
    {
        decidingfactor[0] = proto->MaxLevel - proto->MinLevel;
        if(decidingfactor[0] == 0)
        {
            health = proto->MinHealth + RandomUInt(proto->MaxHealth - proto->MinHealth);
            mindmg = proto->MinDamage;
            maxdmg = proto->MaxDamage;
            level = proto->MinLevel;
        }
        else
        {
            decidingfactor[0] += 1;
            decidingfactor[1] = RandomUInt(decidingfactor[0]);
            if(decidingfactor[1] > 0)
                decidingfactor[1]--;

            uint32 healthdiff = proto->MaxHealth - proto->MinHealth;
            healthdiff /= decidingfactor[0];
            health = proto->MinHealth + healthdiff*decidingfactor[1] + RandomUInt(healthdiff);

            uint32 damagediff = proto->MaxDamage-proto->MinDamage;
            damagediff /= decidingfactor[0];
            mindmg = proto->MinDamage + damagediff*decidingfactor[1];
            maxdmg = proto->MinDamage + damagediff*(decidingfactor[1]+1);
            level = proto->MinLevel + decidingfactor[1];

            if(proto->MaxPower)
            {
                uint32 powerdiff = proto->MaxPower - proto->MinPower;
                powerdiff /= decidingfactor[0];
                power = proto->MinPower + powerdiff*decidingfactor[1] + RandomUInt(powerdiff);
            }
        }
        for(uint32 i = 0; i < 7; i++)
            SetUInt32Value(UNIT_FIELD_RESISTANCES+i,proto->Resistances[i]);
    }

    for(uint32 i = 1; i < 7; i++)
        DamageDoneMod[i] = proto->Damage_Bonus[i-1];
    HealDoneModPos = proto->Healing_Bonus;

    SetUInt32Value(UNIT_FIELD_HEALTH, health);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, health);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, health);

    if(spawn->Bytes)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0, spawn->Bytes->bytes);
        SetUInt32Value(UNIT_FIELD_BYTES_1, spawn->Bytes->bytes1);
        SetUInt32Value(UNIT_FIELD_BYTES_2, spawn->Bytes->bytes2);
    }

    //Use proto displayid (random + gender generator), unless there is an id  specified in spawn->displayid
    uint32 model;
    uint32 gender = creature_info->GenerateModelId(&model);
    SetByte(UNIT_FIELD_BYTES_0, 2, gender);

    SetUInt32Value(UNIT_FIELD_DISPLAYID, model);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, model);

    EventModelChange();

    float realscale = (proto->Scale > 0.0f ? proto->Scale : GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( model )));
    sLog.Debug("Creatures","NPC %u (model %u) got scale %f", proto->Id, model, realscale);
    SetFloatValue(OBJECT_FIELD_SCALE_X, realscale);

    SetUInt32Value(UNIT_NPC_EMOTESTATE, original_emotestate);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,original_MountedDisplayID);
    SetUInt32Value(UNIT_FIELD_LEVEL, level);
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME,proto->AttackTime);
    SetFloatValue(UNIT_FIELD_MINDAMAGE, mindmg);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, maxdmg);
    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME,proto->RangedAttackTime);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,proto->RangedMinDamage);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,proto->RangedMaxDamage);

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, proto->Item1);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, proto->Item2);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, proto->Item3);

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, spawn->factionid);
    SetUInt32Value(UNIT_FIELD_FLAGS, spawn->flags);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, proto->BoundingRadius);
    SetFloatValue(UNIT_FIELD_COMBATREACH, proto->CombatReach);

    // set position
    SetPosition( spawn->x, spawn->y, spawn->z, spawn->o);
    m_spawnLocation.ChangeCoords(spawn->x, spawn->y, spawn->z, spawn->o);
    m_aiInterface->SetWaypointMap(objmgr.GetWayPointMap(spawn->id));

    //use proto faction if spawn faction is unspecified
    m_factionTemplate = dbcFactionTemplate.LookupEntry(spawn->factionid ? spawn->factionid : proto->Faction);
    if(m_factionTemplate)
    {
        // not a neutral creature
        m_faction = dbcFaction.LookupEntry(m_factionTemplate->Faction);
        if(!((!m_faction || m_faction->RepListId == -1) && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0))
            GetAIInterface()->m_canCallForHelp = true;
    }
    else
        sLog.Warning("Creature","Creature is missing a valid faction template for entry %u.", spawn->entry);


//SETUP NPC FLAGS
    SetUInt32Value(UNIT_NPC_FLAGS,proto->NPCFLags);

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

    //load resistances
    for(uint32 x=0;x<7;x++)
        BaseResistance[x]=GetUInt32Value(UNIT_FIELD_RESISTANCES+x);
    for(uint32 x=0;x<5;x++)
        BaseStats[x]=GetUInt32Value(UNIT_FIELD_STAT0+x);

    BaseDamage[0]=GetFloatValue(UNIT_FIELD_MINDAMAGE);
    BaseDamage[1]=GetFloatValue(UNIT_FIELD_MAXDAMAGE);
    BaseOffhandDamage[0]=GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
    BaseOffhandDamage[1]=GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
    BaseRangedDamage[0]=GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
    BaseRangedDamage[1]=GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
    BaseAttackType=proto->AttackType;

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);   // better set this one

////////////AI

    // kek
    for(list<AI_Spell*>::iterator itr = proto->spells.begin(); itr != proto->spells.end(); itr++)
        if((*itr)->difficulty_mask == -1 || (*itr)->difficulty_mask & (1 << mode))
            m_aiInterface->addSpellToList(*itr);

    if(ExtraInfo != NULL)
    {
        m_aiInterface->m_canRangedAttack = ExtraInfo->m_canRangedAttack;
        m_aiInterface->m_canCallForHelp = ExtraInfo->m_canCallForHelp;
        m_aiInterface->m_CallForHelpHealth = ExtraInfo->m_callForHelpHP;
        m_aiInterface->m_canFlee = ExtraInfo->m_canFlee;
        m_aiInterface->m_FleeHealth = ExtraInfo->m_fleeHealth;
        m_aiInterface->m_FleeDuration = ExtraInfo->m_fleeDuration;
        m_aiInterface->sendflee_message = ExtraInfo->sendflee_message;
        m_aiInterface->flee_message = ExtraInfo->flee_message;
    }
    else
    {
        m_aiInterface->m_canRangedAttack = false;
        m_aiInterface->m_canCallForHelp = false;
        m_aiInterface->m_CallForHelpHealth = 0.0f;
        m_aiInterface->m_canFlee = false;
        m_aiInterface->m_FleeHealth = 0.0f;
        m_aiInterface->m_FleeDuration = 0;
        m_aiInterface->sendflee_message = false;
        m_aiInterface->flee_message = "";
    }

    //these fields are always 0 in db
    GetAIInterface()->setMoveType(0);
    GetAIInterface()->setMoveRunFlag(0);

    // load formation data
    Formation* form = sFormationMgr.GetFormation(spawn->id);
    m_aiInterface->SetFormationSQLId( (form == NULL ? 0 : form->fol) );
    m_aiInterface->SetFormationFollowDistance( (form == NULL ? 0.0f : form->dist) );
    m_aiInterface->SetFormationFollowAngle( (form == NULL ? 0.0f : form->ang) );

//////////////AI
    myFamily = dbcCreatureFamily.LookupEntry(creature_info->Family);


// PLACE FOR DIRTY FIX BASTARDS
    // HACK! set call for help on civ health @ 100%
    if(creature_info->Civilian >= 1)
        m_aiInterface->m_CallForHelpHealth = 100;

    //HACK!
    if(m_uint32Values[UNIT_FIELD_DISPLAYID] == 17743 || m_uint32Values[UNIT_FIELD_DISPLAYID] == 20242 || 
        m_uint32Values[UNIT_FIELD_DISPLAYID] == 15435 || creature_info->Family == UNIT_TYPE_MISC)
        m_useAI = false;

    switch(proto->Powertype)
    {
    case POWER_TYPE_MANA:
        {
            SetPowerType(POWER_TYPE_MANA);
            SetUInt32Value(UNIT_FIELD_POWER1, power);
            SetMaxPower(POWER_TYPE_MANA,power);
            SetUInt32Value(UNIT_FIELD_BASE_MANA, power);
        }break;
    case POWER_TYPE_RAGE:
        {
            SetPowerType(POWER_TYPE_RAGE);
            SetPower(POWER_TYPE_RAGE, power*10);
            SetMaxPower(POWER_TYPE_RAGE,power*10);
        }break;
    case POWER_TYPE_FOCUS:
        {
            SetPowerType(POWER_TYPE_FOCUS);
            SetPower(POWER_TYPE_FOCUS, power);
            SetMaxPower(POWER_TYPE_FOCUS,power);
        }break;
    case POWER_TYPE_ENERGY:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetPower(POWER_TYPE_ENERGY, power);
            SetMaxPower(POWER_TYPE_ENERGY,power);
        }break;
    case POWER_TYPE_RUNE:
        {
            SetPowerType(POWER_TYPE_RUNE);
            SetPower(POWER_TYPE_RUNE, power*10);
            SetMaxPower(POWER_TYPE_RUNE,power*10);
        }break;
    case POWER_TYPE_RUNIC:
        {
            SetPowerType(POWER_TYPE_RUNIC);
            SetPower(POWER_TYPE_RUNIC, power*10);
            SetMaxPower(POWER_TYPE_RUNIC,power*10);
        }break;
        //Special vehicle power type cases.
    case POWER_TYPE_STEAM:
    case POWER_TYPE_HEAT:
    case POWER_TYPE_OOZ:
    case POWER_TYPE_BLOOD:
    case POWER_TYPE_WRATH:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetPower(POWER_TYPE_ENERGY, 100);
            SetMaxPower(POWER_TYPE_ENERGY,100);
        }break;
    case POWER_TYPE_PYRITE:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetMaxPower(POWER_TYPE_ENERGY,50);
            m_interruptRegen = true;
        }break;
    default:
        {
            sLog.outError("Creature %u has an unhandled powertype.", GetEntry());
        }break;
    }

    has_combat_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_ENTER_COMBAT);
    has_waypoint_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_RANDOM_WAYPOINT);
    m_aiInterface->m_isGuard = isGuard(GetEntry());

    m_aiInterface->getMoveFlags();
    //CanMove (overrules AI)
    if(!GetCanMove())
        Root();

    /* creature death state */
    if(spawn->death_state == 1)
    {
        uint32 newhealth = m_uint32Values[UNIT_FIELD_HEALTH] / 100;
        if(!newhealth)
            newhealth = 1;
        SetUInt32Value(UNIT_FIELD_HEALTH, 1);
        m_limbostate = true;
        bInvincible = true;
        SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DEAD);
    }

    m_invisFlag = proto->invisibility_type;
    if( spawn->stand_state )
        SetStandState( (uint8)spawn->stand_state );

    uint32 tmpitemid;
    if(tmpitemid = proto->Item1)
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = ItemPrototypeStorage.LookupEntry(tmpitemid);
            }
            else if(getLevel() > 10)
                setcanperry(true); // Who cares what else it is, set parry = true.
        }
    }

    if(tmpitemid = proto->Item2)
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(!b_has_shield && DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = ItemPrototypeStorage.LookupEntry(tmpitemid);
            }
            else if(getLevel() > 10)
            {
                if(DBCItem->InventoryType == INVTYPE_WEAPONOFFHAND
                    || DBCItem->InventoryType == INVTYPE_WEAPON)
                    setcanperry(true); // We can hold non weapons here, so we need to be careful and do checks.
            }
        }
    }

    m_loadedFromDB = true;
    return true;
}

bool Creature::Load(CreatureProto * proto_, uint32 mode, float x, float y, float z, float o)
{
    proto = proto_;

    creature_info = CreatureNameStorage.LookupEntry(proto->Id);
    ExtraInfo = CreatureInfoExtraStorage.LookupEntry(proto->Id);
    if(!creature_info)
        return false;

    m_walkSpeed = m_base_walkSpeed = proto->walk_speed; //set speeds
    m_runSpeed = m_base_runSpeed = proto->run_speed; //set speeds
    VendorMask = 1;

    //Set fields
    SetUInt32Value(OBJECT_FIELD_ENTRY,proto->Id);

    uint32 health = 0;
    uint32 power = 0;
    float mindmg = 0.0f;
    float maxdmg = 0.0f;
    uint32 level = 0;

    // Heroic stats
    if(mode)
    {
        LoadedProto = proto->ModeProto[mode];
        if(LoadedProto != NULL)
        {
            health = LoadedProto->Minhealth + RandomUInt(LoadedProto->Maxhealth - LoadedProto->Minhealth);
            power = LoadedProto->Power;
            mindmg = LoadedProto->Mindmg;
            maxdmg = LoadedProto->Maxdmg;
            level =  LoadedProto->Minlevel + (RandomUInt(LoadedProto->Maxlevel - LoadedProto->Minlevel));
            for(uint32 i = 0; i < 7; i++)
                SetUInt32Value(UNIT_FIELD_RESISTANCES + i, LoadedProto->Resistances[i]);
        }
        else
        {
            for(uint32 i = 0; i < 7; i++)
                SetUInt32Value(UNIT_FIELD_RESISTANCES+i,proto->Resistances[i]);

            float calcu = ((mode*2)/10);

            switch(mode) // TODO: find calculations
            {
            case 1: // 5 man heroic or 25 man.
                {
                    health = long2int32(double(proto->MinHealth + RandomUInt(proto->MaxHealth - proto->MinHealth)) * 1.5);
                    mindmg = proto->MinDamage * 1.2f;
                    maxdmg = proto->MaxDamage * 1.2f;
                    level = proto->MinLevel + RandomUInt(proto->MaxLevel - proto->MinLevel) + RandomUInt(10);
                    if(proto->MaxPower)
                        power = proto->MaxPower * 1.2;
                }break;

            default: // 10H or 25H
                {
                    uint64 newhealth = long2int32(double(proto->MinHealth + RandomUInt(proto->MaxHealth - proto->MinHealth)) * (1.5f + calcu));
                    if(newhealth > 4294967295) // This is the maximum uint32, its pretty much only for LK, and maybe some later cata content.
                        newhealth = 4294967295;

                    health = (uint32)newhealth;
                    mindmg = proto->MinDamage * (1.2f+calcu);
                    maxdmg = proto->MaxDamage * (1.2f+calcu);
                    level = proto->MinLevel + RandomUInt(proto->MaxLevel - proto->MinLevel) + RandomUInt(10);
                    if(proto->MaxPower)
                        power = proto->MaxPower * (1.1f+calcu);
                }break;
            }
        }
    }
    else
    {
        health = proto->MinHealth + RandomUInt(proto->MaxHealth - proto->MinHealth);
        if(proto->MaxPower > 0 && proto->MaxPower == proto->MinPower)
            power = proto->MaxPower;
        else if(proto->MaxPower > 0)
            power = proto->MinPower + RandomUInt(proto->MaxPower - proto->MinPower);
        mindmg = proto->MinDamage;
        maxdmg = proto->MaxDamage;
        level = proto->MinLevel + (RandomUInt(proto->MaxLevel - proto->MinLevel));
        for(uint32 i = 0; i < 7; i++)
            SetUInt32Value(UNIT_FIELD_RESISTANCES+i,proto->Resistances[i]);
    }

    for(uint32 i = 1; i < 7; i++)
        DamageDoneMod[i] = proto->Damage_Bonus[i-1];
    HealDoneModPos = proto->Healing_Bonus;

    SetUInt32Value(UNIT_FIELD_HEALTH, health);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, health);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, health);

    uint32 model = 0;
    uint32 gender = creature_info->GenerateModelId(&model);
    setGender(gender);

    float dbcscale = GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( model ));
    float realscale = (proto->Scale > 0.0f ? proto->Scale : dbcscale);
    SetFloatValue(OBJECT_FIELD_SCALE_X, realscale);
    sLog.Debug("Creature","NPC %u (model %u) got scale %f, found in DBC %f, found in Proto: %f", proto->Id, model, realscale, dbcscale, proto->Scale);

    SetUInt32Value(UNIT_FIELD_DISPLAYID,model);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID,model);

    EventModelChange();

    SetUInt32Value(UNIT_FIELD_LEVEL, level);

    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME,proto->AttackTime);
    SetFloatValue(UNIT_FIELD_MINDAMAGE, mindmg);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, maxdmg);

    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME,proto->RangedAttackTime);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,proto->RangedMinDamage);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,proto->RangedMaxDamage);

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, proto->Item1);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, proto->Item2);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, proto->Item3);

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, proto->Faction);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, proto->BoundingRadius);
    SetFloatValue(UNIT_FIELD_COMBATREACH, proto->CombatReach);
    original_emotestate = 0;

    // set position
    SetPosition( x, y, z, o);
    m_spawnLocation.ChangeCoords(x, y, z, o);
    m_factionTemplate = dbcFactionTemplate.LookupEntry(proto->Faction);
    if(m_factionTemplate)
    {
        m_faction = dbcFaction.LookupEntry(m_factionTemplate->Faction);
        // not a neutral creature
        if(!(m_faction->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0))
        {
            GetAIInterface()->m_canCallForHelp = true;
        }
    }

    //SETUP NPC FLAGS
    SetUInt32Value(UNIT_NPC_FLAGS,proto->NPCFLags);

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

    //load resistances
    for(uint32 x=0;x<7;x++)
        BaseResistance[x]=GetUInt32Value(UNIT_FIELD_RESISTANCES+x);
    for(uint32 x=0;x<5;x++)
        BaseStats[x]=GetUInt32Value(UNIT_FIELD_STAT0+x);

    BaseDamage[0]=GetFloatValue(UNIT_FIELD_MINDAMAGE);
    BaseDamage[1]=GetFloatValue(UNIT_FIELD_MAXDAMAGE);
    BaseOffhandDamage[0]=GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
    BaseOffhandDamage[1]=GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
    BaseRangedDamage[0]=GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
    BaseRangedDamage[1]=GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
    BaseAttackType=proto->AttackType;

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);   // better set this one

    ////////////AI

    // kek
    for(list<AI_Spell*>::iterator itr = proto->spells.begin(); itr != proto->spells.end(); itr++)
        if((*itr)->difficulty_mask == -1 || (*itr)->difficulty_mask & (1 << mode))
            m_aiInterface->addSpellToList(*itr);

    if(ExtraInfo != NULL)
    {
        m_aiInterface->m_canRangedAttack = ExtraInfo->m_canRangedAttack;
        m_aiInterface->m_canCallForHelp = ExtraInfo->m_canCallForHelp;
        m_aiInterface->m_CallForHelpHealth = ExtraInfo->m_callForHelpHP;
        m_aiInterface->m_canFlee = ExtraInfo->m_canFlee;
        m_aiInterface->m_FleeHealth = ExtraInfo->m_fleeHealth;
        m_aiInterface->m_FleeDuration = ExtraInfo->m_fleeDuration;
    }
    else
    {
        m_aiInterface->m_canRangedAttack = false;
        m_aiInterface->m_canCallForHelp = false;
        m_aiInterface->m_CallForHelpHealth = 0.0f;
        m_aiInterface->m_canFlee = false;
        m_aiInterface->m_FleeHealth = 0.0f;
        m_aiInterface->m_FleeDuration = 0;
    }

    //these fields are always 0 in db
    GetAIInterface()->setMoveType(0);
    GetAIInterface()->setMoveRunFlag(0);

    // load formation data
    m_aiInterface->SetFormationSQLId(0);
    m_aiInterface->SetFormationFollowAngle(0.0f);
    m_aiInterface->SetFormationFollowDistance(0.0f);

    //////////////AI

    myFamily = dbcCreatureFamily.LookupEntry(creature_info->Family);


    // PLACE FOR DIRTY FIX BASTARDS
    // HACK! set call for help on civ health @ 100%
    if(creature_info->Civilian >= 1)
        m_aiInterface->m_CallForHelpHealth = 100;

    //HACK!
    if(m_uint32Values[UNIT_FIELD_DISPLAYID] == 17743 || m_uint32Values[UNIT_FIELD_DISPLAYID] == 20242 ||
        m_uint32Values[UNIT_FIELD_DISPLAYID] == 15435 || creature_info->Family == UNIT_TYPE_MISC)
        m_useAI = false;

    switch(proto->Powertype)
    {
    case POWER_TYPE_MANA:
        {
            SetPowerType(POWER_TYPE_MANA);
            SetPower(POWER_TYPE_MANA,power);
            SetMaxPower(POWER_TYPE_MANA,power);
            SetUInt32Value(UNIT_FIELD_BASE_MANA, power);
        }break;
    case POWER_TYPE_RAGE:
        {
            SetPowerType(POWER_TYPE_RAGE);
            SetPower(POWER_TYPE_RAGE,power*10);
            SetMaxPower(POWER_TYPE_RAGE,power*10);
        }break;
    case POWER_TYPE_FOCUS:
        {
            SetPowerType(POWER_TYPE_FOCUS);
            SetPower(POWER_TYPE_FOCUS,power);
            SetMaxPower(POWER_TYPE_FOCUS,power);
        }break;
    case POWER_TYPE_ENERGY:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetPower(POWER_TYPE_ENERGY,power);
            SetMaxPower(POWER_TYPE_ENERGY,power);
        }break;
    case POWER_TYPE_RUNE:
        {
            SetPowerType(POWER_TYPE_RUNE);
            SetPower(POWER_TYPE_RUNE,power*10);
            SetMaxPower(POWER_TYPE_RUNE,power*10);
        }break;
    case POWER_TYPE_RUNIC:
        {
            SetPowerType(POWER_TYPE_RUNIC);
            SetPower(POWER_TYPE_RUNIC,power*10);
            SetMaxPower(POWER_TYPE_RUNIC,power*10);
        }break;
        //Special vehicle power type cases.
    case POWER_TYPE_STEAM:
    case POWER_TYPE_HEAT:
    case POWER_TYPE_OOZ:
    case POWER_TYPE_BLOOD:
    case POWER_TYPE_WRATH:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetPower(POWER_TYPE_ENERGY,100);
            SetMaxPower(POWER_TYPE_ENERGY,100);
        }break;
    case POWER_TYPE_PYRITE:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetMaxPower(POWER_TYPE_ENERGY,50);
            m_interruptRegen = true;
        }break;
    default:
        {
            sLog.outError("Creature %u has an unhandled powertype.", GetEntry());
        }break;
    }

    has_combat_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_ENTER_COMBAT);
    has_waypoint_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_RANDOM_WAYPOINT);
    m_aiInterface->m_isGuard = isGuard(GetEntry());

    m_aiInterface->getMoveFlags();
    //CanMove (overrules AI)
    if(!GetCanMove())
        Root();

    m_invisFlag = proto->invisibility_type;

    uint32 tmpitemid;
    if(tmpitemid = proto->Item1)
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = ItemPrototypeStorage.LookupEntry(tmpitemid);
            }
            else if(getLevel() > 10)
                setcanperry(true); // Who cares what else it is, set parry = true.
        }
    }

    if(!b_has_shield && (tmpitemid = proto->Item2))
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = ItemPrototypeStorage.LookupEntry(tmpitemid);
            }
            else if(getLevel() > 10)
            {
                if(DBCItem->InventoryType == INVTYPE_WEAPONOFFHAND
                    || DBCItem->InventoryType == INVTYPE_WEAPON)
                    setcanperry(true); // We can hold non weapons here, so we need to be careful and do checks.
            }
        }
    }

    return true;
}

void Creature::OnPushToWorld()
{
    if(proto)
    {
        uint8 mode = GetMapMgr()->iInstanceMode;
        if(mode && (proto->ModeProto[mode] != NULL))
        {
            set<uint32>::iterator itr = proto->ModeProto[mode]->start_auras.begin();
            SpellEntry * sp = NULL;
            for(; itr != proto->ModeProto[mode]->start_auras.end(); itr++)
            {
                sp = dbcSpell.LookupEntry((*itr));
                if(sp == NULL)
                    continue;
                CastSpell(this, sp, 0);
            }
        }
        else
        {
            set<uint32>::iterator itr = proto->start_auras.begin();
            SpellEntry * sp = NULL;
            for(; itr != proto->start_auras.end(); itr++)
            {
                sp = dbcSpell.LookupEntry((*itr));
                if(sp == NULL)
                    continue;
                CastSpell(this, sp, 0);
            }
        }
    }

    LoadScript();
    Unit::OnPushToWorld();

    if(_myScriptClass)
        _myScriptClass->OnLoad();

    if(m_spawn)
    {
        if(m_aiInterface->GetFormationSQLId())
        {
            // add event
            sEventMgr.AddEvent(TO_CREATURE(this), &Creature::FormationLinkUp, m_aiInterface->GetFormationSQLId(), EVENT_CREATURE_FORMATION_LINKUP, 1000, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            haslinkupevent = true;
        }

        if(m_spawn->ChannelData)
        {
            if(m_spawn->ChannelData->channel_target_creature)
                sEventMgr.AddEvent(TO_CREATURE(this), &Creature::ChannelLinkUpCreature, m_spawn->ChannelData->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);  // only 5 attempts

            if(m_spawn->ChannelData->channel_target_go)
                sEventMgr.AddEvent(TO_CREATURE(this), &Creature::ChannelLinkUpGO, m_spawn->ChannelData->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);  // only 5 attempts
        }
    }

    m_aiInterface->m_is_in_instance = (m_mapMgr->GetMapInfo()->type!=INSTANCE_NULL) ? true : false;
    if (HasItems())
    {
        for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
        {
            if (itr->second.max_amount == 0)
                itr->second.available_amount=0;
            else if (itr->second.available_amount < itr->second.max_amount)
                sEventMgr.AddEvent(TO_CREATURE(this), &Creature::UpdateItemAmount, itr->second.itemid, EVENT_ITEM_UPDATE, VENDOR_ITEMS_UPDATE_TIME, 1,0);
        }

    }
}

void Creature::Despawn(uint32 delay, uint32 respawntime)
{
    if(delay)
    {
        sEventMgr.AddEvent(TO_CREATURE(this), &Creature::Despawn, (uint32)0, respawntime, EVENT_CREATURE_RESPAWN, delay, 1,0);
        return;
    }

    m_loot.items.clear();

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
        pCell->_respawnObjects.insert(TO_OBJECT(this));
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnCreature, TO_CREATURE(this), pCell, EVENT_CREATURE_RESPAWN, respawntime, 1, 0);
        Unit::RemoveFromWorld(false);
        SetPosition( m_spawnLocation);
        m_respawnCell = pCell;
    }
    else
        Unit::RemoveFromWorld(true);
}

void Creature::TriggerScriptEvent(int ref)
{
    if( _myScriptClass )
        _myScriptClass->StringFunctionCall(ref);
}

void Creature::DestroyCustomWaypointMap()
{
    if(m_custom_waypoint_map)
    {
        for(WayPointMap::iterator itr = m_custom_waypoint_map->begin(); itr != m_custom_waypoint_map->end(); itr++)
        {
            delete (*itr);
        }
        delete m_custom_waypoint_map;
        m_custom_waypoint_map = 0;
        m_aiInterface->SetWaypointMap(0);
    }
}

void Creature::RemoveLimboState(Unit* healer)
{
    if(!m_limbostate)
        return;

    m_limbostate = false;
    SetUInt32Value(UNIT_NPC_EMOTESTATE, m_spawn ? m_spawn->emote_state : 0);
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    bInvincible = false;
}

// Generates 3 random waypoints around the NPC
void Creature::SetGuardWaypoints()
{
    if(!GetMapMgr())
        return;

    if(!GetCreatureInfo())
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
