/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#ifdef M_PI
#undef M_PI
#endif

#define M_PI 3.14159265358979323846f

Creature::Creature(CreatureData *data, uint64 guid) : Unit(guid)
{
    if(_creatureData = data)
        SetEntry(_creatureData->Entry);

    m_spawn = NULL;
    m_quests = NULL;
    IP_shield = NULL;
    m_SellItems = NULL;
    _myScriptClass = NULL;
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
    original_emotestate = 0;
    m_custom_waypoint_map = 0;
    BaseAttackType = SCHOOL_NORMAL;
    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = -1;

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
        m_escorter = NULL;

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
        if (_creatureData->Rank == ELITE_WORLDBOSS || _creatureData->Flags & CREATURE_FLAGS1_BOSS)
            sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_BOSSCORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else if ( _creatureData->Rank == ELITE_RAREELITE || _creatureData->Rank == ELITE_RARE)
            sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_RARECORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, TIME_CREATURE_REMOVE_CORPSE, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        m_corpseEvent = false;
    }
}

int32 Creature::GetBaseAttackTime(uint8 weaponType)
{
    switch(weaponType)
    {
    case 0: return _creatureData->AttackTime;
    case 1: return 0;
    case 2: return _creatureData->RangedAttackTime;
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

        if((_creatureData->Boss && GetMapMgr()->GetMapInfo() && GetMapMgr()->GetdbcMap()->IsRaid()) || m_noRespawn)
            RemoveFromWorld(false, true);
        else
        {
            if(_creatureData->RespawnTime)
                RemoveFromWorld(true, false);
            else RemoveFromWorld(false, true);
        }

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

    SetUInt32Value(UNIT_FIELD_FLAGS, original_flags);
    SetUInt32Value(UNIT_NPC_EMOTESTATE, original_emotestate);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, original_MountedDisplayID);
    Skinned = false;
    m_taggingGroup = m_taggingPlayer = 0;
    m_lootMethod = -1;

    /* creature death state */
    if(m_spawn && m_spawn->death_state == 1)
    {
        uint32 newhealth = GetUInt32Value(UNIT_FIELD_HEALTH) / 100;
        if(!newhealth)
            newhealth = 1;
        SetUInt32Value(UNIT_FIELD_HEALTH, 1);
        m_limbostate = true;
        bInvincible = true;
        SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DEAD);
    }

    SetDeathState(ALIVE);
    GetAIInterface()->StopMovement(0); // after respawn monster can move
    m_PickPocketed = false;
    PushToWorld(m);
}

void Creature::Create(uint32 mapid, float x, float y, float z, float ang)
{
    WorldObject::_Create( mapid, x, y, z, ang );
    m_spawnLocation.ChangeCoords(x, y, z, ang);
}

///////////
/// Looting
uint32 Creature::GetRequiredLootSkill()
{
    if(GetCreatureData()->Flags & CREATURE_FLAGS1_HERBLOOT)
        return SKILL_HERBALISM;     // herbalism
    else if(GetCreatureData()->Flags & CREATURE_FLAGS1_MININGLOOT)
        return SKILL_MINING;        // mining
    else if(GetCreatureData()->Flags & CREATURE_FLAGS1_ENGINEERLOOT)
        return SKILL_ENGINERING;
    else return SKILL_SKINNING;      // skinning
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
    if( _creatureData->Money == -1)
    {
        GetLoot()->gold = 0;
        return;
    }
    else if(_creatureData->Money == 0)
    {
        CreatureData *info = GetCreatureData();
        if (info && info->Type != BEAST)
        {
            if(GetUInt32Value(UNIT_FIELD_MAXHEALTH) <= 1667)
                GetLoot()->gold = uint32((info->Rank+1)*getLevel()*((rand()%5) + 1)); //generate copper
            else GetLoot()->gold = uint32((info->Rank+1)*getLevel()*((rand()%5) + 1)*(GetUInt32Value(UNIT_FIELD_MAXHEALTH)*0.0006)); //generate copper
        } else GetLoot()->gold = 0; // Beasts don't drop money
    } else GetLoot()->gold = uint32(_creatureData->Money);
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
    m_spawn->factionid = GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE);
    m_spawn->flags = uint32(original_flags);
    m_spawn->emote_state = uint32(original_emotestate);
    m_spawn->death_state = getDeathState();
    m_spawn->stand_state = GetStandState();
    m_spawn->vehicle = IsVehicle();
    m_spawn->CanMove = GetCanMove();
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
        << uint32(m_spawn->MountedDisplayID) << ", "
        << int32(1) << ", "
        << uint32(m_spawn->vehicle ? 1 : 0) << ", "
        << uint32(m_spawn->CanMove) << ", "
        << int32(m_spawn->vendormask) << " )";

    WorldDatabase.Execute(ss.str().c_str());
    if(newSpawn && IsInWorld())
        GetMapMgr()->AddSpawn(GetMapMgr()->GetPosX(m_spawn->x), GetMapMgr()->GetPosY(m_spawn->y), m_spawn);
}

void Creature::LoadScript()
{
    _myScriptClass = sScriptMgr.CreateAIScriptClassForEntry(castPtr<Creature>(this));
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

void Creature::AddToWorld()
{
    if(!CanAddToWorld())
        return;

    WorldObject::AddToWorld();
}

void Creature::AddToWorld(MapMgr* pMapMgr)
{
    if(!CanAddToWorld())
        return;

    WorldObject::AddToWorld(pMapMgr);
}

bool Creature::CanAddToWorld()
{
    if(_creatureData == NULL)
        return false;

    // force set faction
    if(GetFaction() == NULL)
    {
        _setFaction();
        if(GetFaction() == NULL)
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
        if(addrespawnevent && _creatureData->RespawnTime > 0)
            delay = _creatureData->RespawnTime;
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

    SetFaction(_creatureData->Faction, false);
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, _creatureData->Faction);
    _setFaction();

    GetAIInterface()->Init(this, AITYPE_AGRO, MOVEMENTTYPE_NONE);

    // Update InRangeSet
    UpdateOppFactionSet();
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
        if (PctRegenModifier)
            amt += (amt * PctRegenModifier) / 100;
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
        float amt = (getLevel()+10)*PctPowerRegenModifier[POWER_TYPE_MANA]/10;
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

void Creature::FormationLinkUp(uint32 SqlId)
{
    if(!m_mapMgr)       // shouldnt happen
        return;

    Creature* creature = NULL;
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
    if(!IsInWorld()) // shouldnt happen
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

bool Creature::Load(CreatureSpawn *spawn, uint32 mode)
{
    //Use proto displayid (random + gender generator), unless there is an id  specified in spawn->displayid
    uint8 gender; uint32 model;
    if(!_creatureData->GenerateModelId(gender, model))
        return false;

    m_spawn = spawn;
    _extraInfo = CreatureInfoExtraStorage.LookupEntry(GetEntry());

    uint32 level = _creatureData->MinLevel;
    if(_creatureData->MaxLevel > _creatureData->MinLevel)
        level += RandomUInt(_creatureData->MaxLevel-_creatureData->MinLevel);

    original_flags = spawn->flags;
    original_emotestate = spawn->emote_state;
    original_MountedDisplayID = spawn->MountedDisplayID;

    //Set fields
    for(uint32 i = 0; i < 7; i++)
        SetUInt32Value(UNIT_FIELD_RESISTANCES+i,_creatureData->Resistances[i]);

    uint32 baseHP = _creatureData->MinHealth;
    if(_creatureData->MaxHealth > _creatureData->MinHealth)
        baseHP += RandomUInt(_creatureData->MaxHealth - _creatureData->MinHealth);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, baseHP);
    SetUInt32Value(UNIT_FIELD_HEALTH, baseHP);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, baseHP);

    if(spawn->Bytes)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0, spawn->Bytes->bytes);
        SetUInt32Value(UNIT_FIELD_BYTES_1, spawn->Bytes->bytes1);
        SetUInt32Value(UNIT_FIELD_BYTES_2, spawn->Bytes->bytes2);
    }

    SetByte(UNIT_FIELD_BYTES_0, 0, RACE_HUMAN);
    SetByte(UNIT_FIELD_BYTES_0, 1, WARRIOR);
    if(_creatureData->MaxPower) SetByte(UNIT_FIELD_BYTES_0, 1, MAGE);
    SetByte(UNIT_FIELD_BYTES_0, 2, gender);

    SetUInt32Value(UNIT_FIELD_DISPLAYID, model);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, model);

    EventModelChange();

    float realscale = (_creatureData->Scale > 0.0f ? _creatureData->Scale : GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( model )));
    sLog.Debug("Creatures","NPC %u (model %u) got scale %f", GetEntry(), model, realscale);
    SetFloatValue(OBJECT_FIELD_SCALE_X, realscale);

    setLevel(level);
    SetUInt32Value(UNIT_NPC_EMOTESTATE, original_emotestate);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID,original_MountedDisplayID);
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME,_creatureData->AttackTime);
    SetFloatValue(UNIT_FIELD_MINDAMAGE, _creatureData->MinDamage);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, _creatureData->MaxDamage);
    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME,_creatureData->RangedAttackTime);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,_creatureData->RangedMinDamage);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,_creatureData->RangedMaxDamage);

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, _creatureData->Item1);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, _creatureData->Item2);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, _creatureData->Item3);

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, spawn->factionid);
    SetUInt32Value(UNIT_FIELD_FLAGS, spawn->flags);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, _creatureData->BoundingRadius);
    SetFloatValue(UNIT_FIELD_COMBATREACH, _creatureData->CombatReach);

    // set position
    SetPosition( spawn->x, spawn->y, spawn->z, spawn->o);
    m_spawnLocation.ChangeCoords(spawn->x, spawn->y, spawn->z, spawn->o);
    m_aiInterface->SetWaypointMap(objmgr.GetWayPointMap(spawn->id));

    //use proto faction if spawn faction is unspecified
    m_factionTemplate = dbcFactionTemplate.LookupEntry(spawn->factionid ? spawn->factionid : _creatureData->Faction);
    if(m_factionTemplate)
    {
        // not a neutral creature
        FactionEntry *faction = m_factionTemplate->GetFaction();
        if(faction && (!(faction->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0)))
            GetAIInterface()->m_canCallForHelp = true;
    } else sLog.Warning("Creature", "Creature is missing a valid faction template for entry %u.", spawn->entry);

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

    BaseAttackType=_creatureData->AttackType;
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);   // better set this one

////////////AI

    // kek
    for(std::list<AI_Spell*>::iterator itr = _creatureData->spells.begin(); itr != _creatureData->spells.end(); itr++)
        if((*itr)->difficulty_mask == -1 || (*itr)->difficulty_mask & (1 << mode))
            m_aiInterface->addSpellToList(*itr);

    m_aiInterface->m_canRangedAttack = false;
    m_aiInterface->m_canCallForHelp = false;
    m_aiInterface->m_CallForHelpHealth = 0.0f;
    m_aiInterface->m_canFlee = false;
    m_aiInterface->m_FleeHealth = 0.0f;
    m_aiInterface->m_FleeDuration = 0;
    m_aiInterface->sendflee_message = false;
    m_aiInterface->flee_message = "";
    if(_extraInfo != NULL)
    {
        m_aiInterface->m_canRangedAttack = _extraInfo->m_canRangedAttack;
        m_aiInterface->m_canCallForHelp = _extraInfo->m_canCallForHelp;
        m_aiInterface->m_CallForHelpHealth = _extraInfo->m_callForHelpHP;
        m_aiInterface->m_canFlee = _extraInfo->m_canFlee;
        m_aiInterface->m_FleeHealth = _extraInfo->m_fleeHealth;
        m_aiInterface->m_FleeDuration = _extraInfo->m_fleeDuration;
        m_aiInterface->sendflee_message = _extraInfo->sendflee_message;
        m_aiInterface->flee_message = _extraInfo->flee_message;
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
    myFamily = dbcCreatureFamily.LookupEntry(_creatureData->Family);


// PLACE FOR DIRTY FIX BASTARDS
    // HACK! set call for help on civ health @ 100%
    if(_creatureData->Civilian >= 1)
        m_aiInterface->m_CallForHelpHealth = 100;

    //HACK!
    if(GetUInt32Value(UNIT_FIELD_DISPLAYID) == 17743 || GetUInt32Value(UNIT_FIELD_DISPLAYID) == 20242 || 
        GetUInt32Value(UNIT_FIELD_DISPLAYID) == 15435 || _creatureData->Family == UNIT_TYPE_MISC)
        m_useAI = false;

    switch(_creatureData->Powertype)
    {
    case POWER_TYPE_MANA:
        {
            SetPowerType(POWER_TYPE_MANA);
            SetPower(POWER_TYPE_MANA, _creatureData->MaxPower);
            SetMaxPower(POWER_TYPE_MANA,_creatureData->MaxPower);
            SetUInt32Value(UNIT_FIELD_BASE_MANA, _creatureData->MaxPower);
        }break;
    case POWER_TYPE_RAGE:
        {
            SetPowerType(POWER_TYPE_RAGE);
            SetPower(POWER_TYPE_RAGE, _creatureData->MaxPower*10);
            SetMaxPower(POWER_TYPE_RAGE,_creatureData->MaxPower*10);
        }break;
    case POWER_TYPE_FOCUS:
        {
            SetPowerType(POWER_TYPE_FOCUS);
            SetPower(POWER_TYPE_FOCUS, _creatureData->MaxPower);
            SetMaxPower(POWER_TYPE_FOCUS,_creatureData->MaxPower);
        }break;
    case POWER_TYPE_ENERGY:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetPower(POWER_TYPE_ENERGY, _creatureData->MaxPower);
            SetMaxPower(POWER_TYPE_ENERGY,_creatureData->MaxPower);
        }break;
    case POWER_TYPE_RUNE:
        {
            SetPowerType(POWER_TYPE_RUNE);
            SetPower(POWER_TYPE_RUNE, _creatureData->MaxPower*10);
            SetMaxPower(POWER_TYPE_RUNE,_creatureData->MaxPower*10);
        }break;
    case POWER_TYPE_RUNIC:
        {
            SetPowerType(POWER_TYPE_RUNIC);
            SetPower(POWER_TYPE_RUNIC, _creatureData->MaxPower*10);
            SetMaxPower(POWER_TYPE_RUNIC,_creatureData->MaxPower*10);
        }break;
    default:
        {
            sLog.outError("Creature %u has an unhandled powertype.", GetEntry());
        }break;
    }

    has_combat_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_ENTER_COMBAT);
    has_waypoint_text = objmgr.HasMonsterSay(GetEntry(), MONSTER_SAY_EVENT_RANDOM_WAYPOINT);
    m_aiInterface->m_isGuard = isGuard(GetEntry());

    //CanMove (overrules AI)
    if(!GetCanMove())
        GetMovementInterface()->setRooted(true);

    /* creature death state */
    if(spawn->death_state == 1)
    {
        uint32 newhealth = GetUInt32Value(UNIT_FIELD_HEALTH) / 100;
        if(!newhealth)
            newhealth = 1;
        SetUInt32Value(UNIT_FIELD_HEALTH, 1);
        m_limbostate = true;
        bInvincible = true;
        SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DEAD);
    }

    m_invisFlag = _creatureData->Invisibility_type;
    if( spawn->stand_state )
        SetStandState( (uint8)spawn->stand_state );

    uint32 tmpitemid;
    if(tmpitemid = _creatureData->Item1)
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = sItemMgr.LookupEntry(tmpitemid);
            }
            else if(getLevel() > 10)
                setcanperry(true); // Who cares what else it is, set parry = true.
        }
    }

    if(tmpitemid = _creatureData->Item2)
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(!b_has_shield && DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = sItemMgr.LookupEntry(tmpitemid);
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

bool Creature::Load(uint32 mode, float x, float y, float z, float o)
{
    uint8 gender; uint32 model;
    if(!_creatureData->GenerateModelId(gender, model))
        return false;

    _extraInfo = CreatureInfoExtraStorage.LookupEntry(GetEntry());

    uint32 level = _creatureData->MinLevel;
    if(_creatureData->MaxLevel > _creatureData->MinLevel)
        level += RandomUInt(_creatureData->MaxLevel-_creatureData->MinLevel);

    for(uint32 i = 0; i < 7; i++)
        SetUInt32Value(UNIT_FIELD_RESISTANCES+i,_creatureData->Resistances[i]);

    uint32 baseHP = _creatureData->MinHealth;
    if(_creatureData->MaxHealth > _creatureData->MinHealth)
        baseHP += RandomUInt(_creatureData->MaxHealth - _creatureData->MinHealth);
    SetUInt32Value(UNIT_FIELD_HEALTH, baseHP);
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, baseHP);
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, baseHP);

    SetByte(UNIT_FIELD_BYTES_0, 0, RACE_HUMAN);
    SetByte(UNIT_FIELD_BYTES_0, 1, WARRIOR);
    if(_creatureData->MaxPower) SetByte(UNIT_FIELD_BYTES_0, 1, MAGE);
    SetByte(UNIT_FIELD_BYTES_0, 2, gender);

    float dbcscale = GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( model ));
    float realscale = (_creatureData->Scale > 0.0f ? _creatureData->Scale : dbcscale);
    SetFloatValue(OBJECT_FIELD_SCALE_X, realscale);
    sLog.Debug("Creature","NPC %u (model %u) got scale %f, found in DBC %f, found in Proto: %f", _creatureData->Entry, model, realscale, dbcscale, _creatureData->Scale);

    SetUInt32Value(UNIT_FIELD_DISPLAYID,model);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID,model);

    EventModelChange();

    setLevel(level);

    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME,_creatureData->AttackTime);
    SetFloatValue(UNIT_FIELD_MINDAMAGE, _creatureData->MinDamage);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, _creatureData->MaxDamage);

    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME,_creatureData->RangedAttackTime);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE,_creatureData->RangedMinDamage);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE,_creatureData->RangedMaxDamage);

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, _creatureData->Item1);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, _creatureData->Item2);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, _creatureData->Item3);

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, _creatureData->Faction);
    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, _creatureData->BoundingRadius);
    SetFloatValue(UNIT_FIELD_COMBATREACH, _creatureData->CombatReach);
    original_emotestate = 0;

    // set position
    SetPosition( x, y, z, o);
    m_spawnLocation.ChangeCoords(x, y, z, o);
    m_factionTemplate = dbcFactionTemplate.LookupEntry(_creatureData->Faction);
    if(m_factionTemplate)
    {
        FactionEntry *faction = m_factionTemplate->GetFaction();
        // not a neutral creature
        if(faction && (!(faction->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0)))
            GetAIInterface()->m_canCallForHelp = true;
    }

    //SETUP NPC FLAGS
    SetUInt32Value(UNIT_NPC_FLAGS,_creatureData->NPCFLags);

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

    BaseAttackType=_creatureData->AttackType;

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);   // better set this one

    ////////////AI

    // kek
    for(std::list<AI_Spell*>::iterator itr = _creatureData->spells.begin(); itr != _creatureData->spells.end(); itr++)
        if((*itr)->difficulty_mask == -1 || (*itr)->difficulty_mask & (1 << mode))
            m_aiInterface->addSpellToList(*itr);

    m_aiInterface->m_canRangedAttack = false;
    m_aiInterface->m_canCallForHelp = false;
    m_aiInterface->m_CallForHelpHealth = 0.0f;
    m_aiInterface->m_canFlee = false;
    m_aiInterface->m_FleeHealth = 0.0f;
    m_aiInterface->m_FleeDuration = 0;
    if(_extraInfo != NULL)
    {
        m_aiInterface->m_canRangedAttack = _extraInfo->m_canRangedAttack;
        m_aiInterface->m_canCallForHelp = _extraInfo->m_canCallForHelp;
        m_aiInterface->m_CallForHelpHealth = _extraInfo->m_callForHelpHP;
        m_aiInterface->m_canFlee = _extraInfo->m_canFlee;
        m_aiInterface->m_FleeHealth = _extraInfo->m_fleeHealth;
        m_aiInterface->m_FleeDuration = _extraInfo->m_fleeDuration;
    }

    //these fields are always 0 in db
    GetAIInterface()->setMoveType(0);
    GetAIInterface()->setMoveRunFlag(0);

    // load formation data
    m_aiInterface->SetFormationSQLId(0);
    m_aiInterface->SetFormationFollowAngle(0.0f);
    m_aiInterface->SetFormationFollowDistance(0.0f);

    //////////////AI
    myFamily = dbcCreatureFamily.LookupEntry(_creatureData->Family);

    // PLACE FOR DIRTY FIX BASTARDS
    // HACK! set call for help on civ health @ 100%
    if(m_aiInterface->m_CallForHelpHealth == 0.0f && _creatureData->Civilian >= 1)
        m_aiInterface->m_CallForHelpHealth = 100;

    //HACK!
    if(GetUInt32Value(UNIT_FIELD_DISPLAYID) == 17743 || GetUInt32Value(UNIT_FIELD_DISPLAYID) == 20242 ||
        GetUInt32Value(UNIT_FIELD_DISPLAYID) == 15435 || _creatureData->Family == UNIT_TYPE_MISC)
        m_useAI = false;

    switch(_creatureData->Powertype)
    {
    case POWER_TYPE_MANA:
        {
            SetPowerType(POWER_TYPE_MANA);
            SetPower(POWER_TYPE_MANA,_creatureData->MaxPower);
            SetMaxPower(POWER_TYPE_MANA,_creatureData->MaxPower);
            SetUInt32Value(UNIT_FIELD_BASE_MANA, _creatureData->MaxPower);
        }break;
    case POWER_TYPE_RAGE:
        {
            SetPowerType(POWER_TYPE_RAGE);
            SetPower(POWER_TYPE_RAGE,_creatureData->MaxPower*10);
            SetMaxPower(POWER_TYPE_RAGE,_creatureData->MaxPower*10);
        }break;
    case POWER_TYPE_FOCUS:
        {
            SetPowerType(POWER_TYPE_FOCUS);
            SetPower(POWER_TYPE_FOCUS,_creatureData->MaxPower);
            SetMaxPower(POWER_TYPE_FOCUS,_creatureData->MaxPower);
        }break;
    case POWER_TYPE_ENERGY:
        {
            SetPowerType(POWER_TYPE_ENERGY);
            SetPower(POWER_TYPE_ENERGY,_creatureData->MaxPower);
            SetMaxPower(POWER_TYPE_ENERGY,_creatureData->MaxPower);
        }break;
    case POWER_TYPE_RUNE:
        {
            SetPowerType(POWER_TYPE_RUNE);
            SetPower(POWER_TYPE_RUNE,_creatureData->MaxPower*10);
            SetMaxPower(POWER_TYPE_RUNE,_creatureData->MaxPower*10);
        }break;
    case POWER_TYPE_RUNIC:
        {
            SetPowerType(POWER_TYPE_RUNIC);
            SetPower(POWER_TYPE_RUNIC,_creatureData->MaxPower*10);
            SetMaxPower(POWER_TYPE_RUNIC,_creatureData->MaxPower*10);
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
        GetMovementInterface()->setRooted(true);

    m_invisFlag = _creatureData->Invisibility_type;

    uint32 tmpitemid;
    if(tmpitemid = _creatureData->Item1)
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = sItemMgr.LookupEntry(tmpitemid);
            }
            else if(getLevel() > 10)
                setcanperry(true); // Who cares what else it is, set parry = true.
        }
    }

    if(!b_has_shield && (tmpitemid = _creatureData->Item2))
    {
        ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid);
        if(DBCItem)
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
            {
                b_has_shield = true;
                IP_shield = sItemMgr.LookupEntry(tmpitemid);
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
    for(std::set<uint32>::iterator itr = _creatureData->Auras.begin(); itr != _creatureData->Auras.end(); itr++)
        if(SpellEntry *sp = dbcSpell.LookupEntry((*itr)))
            CastSpell(this, sp, true);

    LoadScript();
    Unit::OnPushToWorld();

    if(_myScriptClass)
        _myScriptClass->OnLoad();

    if(m_spawn)
    {
        if(m_aiInterface->GetFormationSQLId())
        {
            // add event
            sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::FormationLinkUp, m_aiInterface->GetFormationSQLId(), EVENT_CREATURE_FORMATION_LINKUP, 1000, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            haslinkupevent = true;
        }

        if(m_spawn->ChannelData)
        {
            if(m_spawn->ChannelData->channel_target_creature)
                sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::ChannelLinkUpCreature, m_spawn->ChannelData->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);  // only 5 attempts

            if(m_spawn->ChannelData->channel_target_go)
                sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::ChannelLinkUpGO, m_spawn->ChannelData->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);  // only 5 attempts
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
                sEventMgr.AddEvent(castPtr<Creature>(this), &Creature::UpdateItemAmount, itr->second.itemid, EVENT_ITEM_UPDATE, VENDOR_ITEMS_UPDATE_TIME, 1,0);
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

void Creature::TriggerScriptEvent(int ref)
{
    if( _myScriptClass )
        _myScriptClass->StringFunctionCall(ref);
}

void Creature::DestroyCustomWaypointMap()
{
    if(m_custom_waypoint_map)
    {
        m_aiInterface->SetWaypointMap(0);
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
    SetUInt32Value(UNIT_NPC_EMOTESTATE, m_spawn ? m_spawn->emote_state : 0);
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
