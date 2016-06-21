/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#ifdef M_PI
#undef M_PI
#endif

#define M_PI 3.14159265358979323846f

Creature::Creature(CreatureData *data, uint64 guid) : Unit(guid), _creatureData(data), m_aiInterface(this, m_movementInterface.GetPath())
{
    SetEntry(data->entry);

    m_spawn = NULL;
    m_quests = NULL;
    m_SellItems = NULL;
    auctionHouse = NULL;
    m_respawnCell = NULL;
    m_shieldProto = NULL;
    myFamily = NULL;

    m_taxiNode[0] = m_taxiNode[1] = 0;
    m_H_regenTimer = 0;
    m_P_regenTimer = 0;
    m_p_DelayTimer = 0;
    m_enslaveCount = 0;
    m_enslaveSpell = 0;

    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = -1;

    m_AreaUpdateTimer = 0;
    m_lastAreaPosition.ChangeCoords(0.0f, 0.0f, 0.0f);

    m_aggroRangeMod = 0.f;
    m_creaturePool = 0xFF;
    m_skinned = false; // 0x02
    b_has_shield = false; // 0x04
    m_noDeleteAfterDespawn = false; // 0x08
    m_noRespawn = false; // 0x10
    m_isGuard = false; // 0x20
    m_canRegenerateHP = true; // 0x40
    m_limbostate = false; // 0x80
    m_corpseEvent = false; // 0x100
    m_pickPocketed = false; // 0x200
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

    if(m_respawnCell != NULL)
        m_respawnCell->RemoveRespawn(this);
    Unit::Destruct();
}

void Creature::Reactivate()
{

}

void Creature::Update(uint32 msTime, uint32 uiDiff)
{
    m_AreaUpdateTimer += uiDiff;
    if(m_AreaUpdateTimer >= 5000)
    {
        if(m_lastAreaPosition.DistanceSq(GetPosition()) > sWorld.AreaUpdateDistance)
        {
            // Update our area id and position
            UpdateAreaInfo();
            m_lastAreaPosition = GetPosition();
        }
        m_AreaUpdateTimer = 0;
    }

    Unit::Update(msTime, uiDiff);

    m_aiInterface.Update(uiDiff);

    if(isDead())
    {
        if(IsTotem())
            Respawn(false, true);

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
    } else if(hasStateFlag(UF_ATTACKING))
        UpdateAutoAttackState();
}

void Creature::UpdateFieldValues()
{
    if(m_modQueuedModUpdates.find(100) != m_modQueuedModUpdates.end())
    {
        m_aggroRangeMod = 0.f;
        if(AuraInterface::modifierMap *modMap = m_AuraInterface.GetModMapByModType(SPELL_AURA_MOD_DETECT_RANGE))
            for(AuraInterface::modifierMap::iterator itr = modMap->begin(); itr != modMap->end(); itr++)
                m_aggroRangeMod += itr->second->m_amount;
    }
    Unit::UpdateFieldValues();
}

void Creature::OnAuraModChanged(uint32 modType)
{
    switch(modType)
    {
    case SPELL_AURA_MOD_DETECT_RANGE:
        m_modQueuedModUpdates[100].push_back(modType);
        break;
    }
}

float Creature::GetAggroRange()
{
    float baseAggro = 20.f;
    // detect range auras
    baseAggro += m_aggroRangeMod;
    // Modifying based on rank
    baseAggro += _creatureData->rank*1.25f;
    // Add half model size after modifiers
    baseAggro += GetModelHalfSize();
    return baseAggro;
}

void Creature::OnAddInRangeObject(WorldObject *pObj)
{
    if(pObj->IsUnit() && sFactionSystem.isHostile(this, pObj))
        m_inRangeHostiles.push_back(pObj->GetGUID());
    Unit::OnAddInRangeObject(pObj);
}

void Creature::OnRemoveInRangeObject(WorldObject *pObj)
{
    Unit::OnRemoveInRangeObject(pObj);
    if(pObj->IsUnit())
    {
        WorldObject::InRangeSet::iterator itr;
        if((itr = std::find(m_inRangeHostiles.begin(), m_inRangeHostiles.end(), pObj->GetGUID())) != m_inRangeHostiles.end())
            m_inRangeHostiles.erase(itr);
    }
}

void Creature::ClearInRangeSet()
{
    m_inRangeHostiles.clear();
    Unit::ClearInRangeSet();
}

void Creature::EventAttackStop()
{
    Unit::EventAttackStop();
    SetUInt64Value(UNIT_FIELD_TARGET, 0);
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
        RemoveFromWorld();

    Destruct();
}

void Creature::OnRemoveCorpse()
{
    // time to respawn!
    if (IsInWorld() && (int32)m_mapInstance->GetInstanceID() == m_instanceId)
    {

        sLog.Debug("Creature","OnRemoveCorpse Removing corpse of %llu...", GetGUID().raw());

        if(((_extraInfo && _extraInfo->isBoss) && GetMapInstance()->IsRaid()) || m_noRespawn)
            Respawn(false, true);
        else if(_creatureData->respawnTime)
            Respawn(true, false);
        else Respawn(false, true);

        SetDeathState(DEAD);
        SetPosition(GetSpawnX(), GetSpawnY(), GetSpawnZ(), GetSpawnO());
    }
}

void Creature::OnRespawn( MapInstance* m)
{
    sLog.outDebug("Respawning %llu...", GetGUID().raw());
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
    SetUInt32Value(UNIT_NPC_FLAGS, _creatureData->NPCFLags);

    m_skinned = false;
    m_taggingGroup = m_taggingPlayer = 0;
    m_lootMethod = -1;

    SetDeathState(ALIVE);
    m_pickPocketed = false;
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
    uint8 team = 0;
    uint8 difficulty = (m_mapInstance ? (m_mapInstance->iInstanceMode) : 0);
    if(!m_killer.empty() && m_killer.getHigh() == HIGHGUID_TYPE_PLAYER)
        if(Player *plr = GetInRangeObject<Player>(m_killer))
            team = plr->GetTeam();

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
        if (info && info->type != UT_BEAST)
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
    if(IsSummon()) //Just in case.
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
        GetMapInstance()->AddSpawn(GetMapInstance()->GetPosX(m_spawn->x), GetMapInstance()->GetPosY(m_spawn->y), m_spawn);
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
        m_aiInterface.OnDeath();

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
    Unit::OnPushToWorld();
    for(std::set<uint32>::iterator itr = _creatureData->Auras.begin(); itr != _creatureData->Auras.end(); itr++)
        if(SpellEntry *sp = dbcSpell.LookupEntry((*itr)))
            CastSpell(this, sp, true);
}

void Creature::UpdateAreaInfo(MapInstance *instance)
{
    Unit::UpdateAreaInfo(instance);
    if(GetTeam() < TEAM_GUARD && GetAreaFlags() & OBJECT_AREA_FLAG_INSANCTUARY)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
}

void Creature::Respawn(bool addrespawnevent, bool free_guid)
{
    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = 1;

    m_AuraInterface.RemoveAllAuras();

    uint32 delay = 0;
    if(addrespawnevent && _creatureData->respawnTime > 0)
        delay = _creatureData->respawnTime;
    Despawn(0, delay);
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
}

bool Creature::RemoveEnslave()
{
    return RemoveAura(m_enslaveSpell);
}

void Creature::RegenerateHealth(bool inCombat)
{
    if(m_limbostate || !m_canRegenerateHP || inCombat)
        return;

    uint32 cur = GetUInt32Value(UNIT_FIELD_HEALTH);
    uint32 mh = GetUInt32Value(UNIT_FIELD_MAXHEALTH);
    if(cur >= mh)
        return;

    if(hasStateFlag(UF_EVADING))
    {
        cur += getLevel()*2; //though creatures have their stats we use some wierd formula for amt
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

    if(isinterrupted || hasStateFlag(UF_EVADING))
    {
        cur += getLevel()+10;
        SetPower(POWER_TYPE_MANA, (cur>=mm)?mm:cur);
        return;
    }

    cur += float2int32(ceil(float(mm)/10.f));
    if(cur >= mm || (mm-cur < mm/16)) cur = mm;
    SetPower(POWER_TYPE_MANA, cur);
}

void Creature::SendInventoryList(Player *plr)
{
    uint32 counter = 0;
    ByteBuffer dataBuff;
    std::vector<bool> bitFlags;
    WorldPacket data(SMSG_LIST_INVENTORY, 10);
    if(HasItems())
    {
        for(std::map<uint32, CreatureItem>::iterator itr = GetSellItemBegin(); itr != GetSellItemEnd(); itr++)
        {
            if(counter >= 255)
            {
                sLog.Error("VendorListing", "Creature %u contains too many items, Displaying (150/%u) items.", GetEntry(), uint32(GetSellItemCount()));
                break;
            }

            if(ItemPrototype *curItem = sItemMgr.LookupEntry(itr->second.itemid))
            {
                if(!(itr->second.vendormask & GetVendorMask()))
                    continue;

                // Update code for available items
                int32 availableAmount = -1;
                if(int32 maxAmount = itr->second.max_amount)
                {
                    availableAmount = maxAmount;
                    if(m_limitedItems.find(itr->first) != m_limitedItems.end())
                    {
                        // Set available amount to the listed amount in mem
                        availableAmount = m_limitedItems.at(itr->first).second;

                        // Update item count based on incrementation time
                        if((UNIXTIME-m_limitedItems.at(itr->first).first) >= itr->second.incrtime)
                        {
                            uint32 diff = uint32((UNIXTIME - m_limitedItems.at(itr->first).first) / itr->second.incrtime);
                            m_limitedItems.at(itr->first).second += curItem->BuyCount*diff;
                            // For now we just update our available amount and check against our max amount
                            if((availableAmount = m_limitedItems.at(itr->first).second) >= maxAmount)
                                availableAmount = maxAmount; // If we're over, just set us to our max amount
                            m_limitedItems.at(itr->first).first = UNIXTIME;
                        }

                        // Check to see if we've reached max value, if so then erase
                        if(availableAmount == maxAmount)
                            m_limitedItems.erase(itr->first);
                    }
                }

                if(plr->ignoreitemreq_cheat == false)
                {
                    if(itr->second.IsDependent)
                    {
                        if(curItem->AllowableClass && !(plr->getClassMask() & curItem->AllowableClass))
                            continue;
                        if(curItem->AllowableRace && !(plr->getRaceMask() & curItem->AllowableRace))
                            continue;

                        if(curItem->Class == ITEM_CLASS_ARMOR && curItem->SubClass >= ITEM_SUBCLASS_ARMOR_LIBRAM && curItem->SubClass <= ITEM_SUBCLASS_ARMOR_SIGIL)
                            if(!(plr->GetArmorProficiency() & (uint32(1) << curItem->SubClass)))
                                continue; // Do not show relics to classes that can't use them.
                    }

                    if(itr->second.extended_cost)
                    {
                        if(itr->second.extended_cost->flags & 0x01)
                        {
                            // Calculate if we've reached the correct guild rank to view these items
                            continue; // TODO
                        }
                    } else if(curItem->SellPrice > curItem->BuyPrice)
                        continue;
                }

                dataBuff << (++counter);
                dataBuff << curItem->Durability;
                if(itr->second.extended_cost)
                {
                    dataBuff << uint32(itr->second.extended_cost->Id);
                    bitFlags.push_back(false);
                } else bitFlags.push_back(true);
                bitFlags.push_back(true);

                dataBuff << curItem->ItemId << uint32(1);
                dataBuff << uint32(sItemMgr.CalculateBuyPrice(curItem->ItemId, 1, plr, this, itr->second.extended_cost));
                dataBuff << uint32(curItem->DisplayInfoID) << availableAmount;
                dataBuff << uint32(curItem->BuyCount);
            }
        }
    }

    data.WriteGuidBitString(2, m_objGuid, 1, 0);
    data.WriteBits(counter, 21); // item count
    data.WriteGuidBitString(5, m_objGuid, 3, 6, 5, 2, 7);
    for (uint32 i = 0; i < bitFlags.size(); ++i)
        data.WriteBit(bitFlags[i]);
    data.WriteBit(m_objGuid[4]);
    data.FlushBits();
    data.append(dataBuff.contents(), dataBuff.size());

    data.WriteByteSeq(m_objGuid[5]);
    data.WriteByteSeq(m_objGuid[4]);
    data.WriteByteSeq(m_objGuid[1]);
    data.WriteByteSeq(m_objGuid[0]);
    data.WriteByteSeq(m_objGuid[6]);
    data << uint8(counter == 0);
    data.WriteByteSeq(m_objGuid[2]);
    data.WriteByteSeq(m_objGuid[3]);
    data.WriteByteSeq(m_objGuid[7]);
    plr->SendPacket( &data );
    sLog.Debug( "WORLD"," Sent SMSG_LIST_INVENTORY" );
}

void Creature::AddVendorItem(uint32 itemid, uint32 vendormask, uint32 ec)
{
    CreatureItem ci;
    memset(&ci, 0, sizeof(CreatureItem));
    ci.itemid = itemid;
    ci.IsDependent = true;
    ci.vendormask = vendormask;
    ci.extended_cost = ec ? dbcItemExtendedCost.LookupEntry(ec) : NULL;

    if(m_SellItems == NULL)
        m_SellItems = objmgr.AllocateVendorList(GetEntry());

    uint32 slot = 1;
    if(m_SellItems->size())
        slot = m_SellItems->rbegin()->first+1;
    m_SellItems->insert(std::make_pair(slot, ci));
}

int32 Creature::ModAvItemAmount(uint32 slot, uint32 value)
{
    std::map<uint32, CreatureItem>::iterator itr;
    if((itr = m_SellItems->find(slot)) == m_SellItems->end())
        return 0;

    if(m_limitedItems.find(slot) == m_limitedItems.end())
        m_limitedItems.insert(std::make_pair(slot, std::make_pair(UNIXTIME, (itr->second.max_amount>=value ? itr->second.max_amount-value : 0))));
    else if(m_limitedItems.at(slot).second <= value)
        m_limitedItems.at(slot).second = 0;
    else m_limitedItems.at(slot).second -= value;
    return m_limitedItems.at(slot).second;
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
    if(!m_mapInstance)       // shouldnt happen
        return;

    if( Creature *creature = m_mapInstance->GetSqlIdCreature(SqlId) )
    {
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

    // Event objects should be spawned inactive
    isActive = m_spawn->eventId ? false : true;

    // Set our extra data pointer
    _extraInfo = CreatureInfoExtraStorage.LookupEntry(GetEntry());

    // Grab our saved model spawn and check if it's valid from creature data
    uint32 model = m_spawn ? m_spawn->modelId : 0, gender=0;
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

    SetFaction(_creatureData->faction);
    //SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, _creatureData->boundingRadius * _creatureData->scale);
    //SetFloatValue(UNIT_FIELD_COMBATREACH, _creatureData->combatReach * _creatureData->scale);

    UpdateFieldValues();

    // Max out health
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));

    //use proto faction if spawn faction is unspecified
    if(m_factionTemplate = dbcFactionTemplate.LookupEntry(GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE)));/*
    {
        // not a neutral creature
        FactionEntry *faction = m_factionTemplate->GetFaction();
        if(faction && (!(faction->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0)))
            GetAIInterface()->m_canCallForHelp = true;
    } else sLog.Warning("Creature", "Creature is missing a valid faction template for entry %u.", GetEntry());*/

    //SETUP NPC FLAGS
    SetUInt32Value(UNIT_NPC_FLAGS, _creatureData->NPCFLags);

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ) )
        m_SellItems = objmgr.GetVendorList(GetEntry());

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR ) )
        sTaxiMgr.GetNearestTaxiNodes(mapId, x, y, z, m_taxiNode);

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ) )
        _LoadQuests();

    if ( HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ) )
        auctionHouse = sAuctionMgr.GetAuctionHouse(GetEntry());

    // load formation data
    Formation* form = NULL;//m_spawn ? sObjMgr.GetFormation(m_spawn->id) : NULL;

    //////////////AI
    myFamily = dbcCreatureFamily.LookupEntry(_creatureData->family);

    //CanMove (overrules AI)
    if(!GetCanMove())
        GetMovementInterface()->setRooted(true);

    m_invisFlag = _creatureData->invisType;

    if(uint32 tmpitemid = _creatureData->inventoryItem[0])
    {
        if(ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid))
        {
            if(DBCItem->InventoryType == INVTYPE_SHIELD)
                b_has_shield = (m_shieldProto = sItemMgr.LookupEntry(tmpitemid)) != NULL;
        }
    }

    if(uint32 tmpitemid = _creatureData->inventoryItem[1])
    {
        if(ItemDataEntry* DBCItem = db2Item.LookupEntry(tmpitemid))
        {
            if(!b_has_shield && DBCItem->InventoryType == INVTYPE_SHIELD)
                b_has_shield = (m_shieldProto = sItemMgr.LookupEntry(tmpitemid)) != NULL;
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
        MapCell * pCell = m_mapInstance->GetCellByCoords(m_spawnLocation.x, m_spawnLocation.y);
        if(!pCell)
            pCell = m_mapCell;

        ASSERT(pCell);
        pCell->AddRespawn(this);
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapInstance, &MapInstance::EventRespawnCreature, castPtr<Creature>(this), pCell, EVENT_CREATURE_RESPAWN, respawntime, 1, 0);
        Unit::RemoveFromWorld();
        SetPosition( m_spawnLocation);
        m_respawnCell = pCell;
    } else Unit::RemoveFromWorld();
}

void Creature::RemoveLimboState(Unit* healer)
{
    if(!m_limbostate)
        return;

    m_limbostate = false;
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));
}

void Creature::SendTaxiList(Player *plr)
{
    WorldPacket data(SMSG_SHOWTAXINODES, 48);
    if(uint32 curloc = GetTaxiNode(plr->GetTeam()))
    {
        if(!plr->HasTaxiNode(curloc)) // Check for known nodes
        {
            plr->AddTaxiMask(curloc);

            plr->GetSession()->OutPacket(SMSG_NEW_TAXI_PATH);

            //Send packet
            data.Initialize(SMSG_TAXINODE_STATUS, 9);
            data << GetGUID() << uint8(1);
        }
        else
        {
            data << uint32(1) << GetGUID() << uint32(curloc);
            if(UpdateMask *taxiMask = plr->GetTaximask())
            {   //Set Mask
                data << uint32(taxiMask->GetLength());
                data.append(taxiMask->GetMask(), taxiMask->GetLength());
            } else data << uint32(0);
        }
    } else data << uint32(1) << GetGUID() << uint64(0);
    plr->SendPacket( &data );
}
