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
    m_trainerData = NULL;
    auctionHouse = NULL;
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
    m_zoneVisibleSpawn = false;

    //m_script = NULL;
}

Creature::~Creature()
{
    while(!m_combatSpells.empty())
    {
        CreatureSpell *spell = *m_combatSpells.begin();
        m_combatSpells.erase(m_combatSpells.begin());
        delete spell;
    }
}

void Creature::Init()
{
    Unit::Init();

    m_aiInterface.Init();

    for(std::vector<uint32>::iterator itr = _creatureData->combatSpells.begin(); itr != _creatureData->combatSpells.end(); itr++)
    {
        if(SpellEntry *spellEntry = dbcSpell.LookupEntry(*itr))
        {
            if(spellEntry->isSpellAuraApplicator() && !spellEntry->isNegativeSpell1())
                continue; // These aren't combat spells

            CreatureSpell *spell = new CreatureSpell();
            spell->spellEntry = spellEntry;
            // Soonest cast time is the least of our cooldowns
            spell->castTimer = std::min<uint32>(spellEntry->StartRecoveryTime, std::min<uint32>(spellEntry->CategoryRecoveryTime, spellEntry->RecoveryTime));
            // Cast cooldown is based on the highest of our cooldowns
            spell->cooldownTimer = std::max<uint32>(spellEntry->StartRecoveryTime, std::max<uint32>(spellEntry->CategoryRecoveryTime, spellEntry->RecoveryTime));
            // Aura applicator check
            if(spellEntry->isSpellAuraApplicator()) // If we're applying an aura, make sure that we don't recast within the duration time
                spell->cooldownTimer += (spellEntry->Duration[0] > 0 ? spellEntry->Duration[0] : spellEntry->Duration[1] > 0 ? spellEntry->Duration[1] : spellEntry->Duration[2] > 0 ? spellEntry->Duration[2] : 0);
            // Push back the cast timer a bit
            if(spell->castTimer == 0 || spell->castTimer == spell->cooldownTimer)
                spell->castTimer += spell->castTimer ? 2000 : 5000;
            if(spell->cooldownTimer < 5000)
                spell->cooldownTimer = 5000;
            m_combatSpells.push_back(spell);
        }
    }

    if(uint32 vehicleKitId = _creatureData->vehicleEntry)
        InitVehicleKit(vehicleKitId);
}

void Creature::Destruct()
{
    Unit::Destruct();
}

void Creature::Reactivate()
{
    if(m_deathState == DEAD)
    {
        // Reset after death specifics
        m_skinned = false;
        m_taggingGroup = m_taggingPlayer = 0;
        m_lootMethod = -1;
        m_pickPocketed = false;

        // Other interface hooks
        m_aiInterface.OnRespawn();

        m_movementInterface.OnRespawn();

        m_deathState = ALIVE;

        // Reload from proto and respawn at spawn loc
        Load(m_mapId, m_spawnLocation.x, m_spawnLocation.y, m_spawnLocation.z, m_spawnLocation.o, m_mapInstance->iInstanceMode, m_spawn, true);

        // Update position on map for inrange calls
        m_mapInstance->ChangeObjectLocation(this);
    }
}

void Creature::Despawn(uint32 respawntime)
{
    // Empty items
    GetLoot()->items.clear();

    //Better make sure all summoned totems/summons/GO's created by this creature spawned removed.
    SummonExpireAll(true);

    // Reset position to spawn loc
    SetPosition(m_spawnLocation);
    clearStateFlag(UF_CORPSE);
    m_inactiveFlags |= OBJECT_INACTIVE_FLAG_DESPAWNED;
    WorldObject::Deactivate(respawntime);
    m_despawnTimer = 0;
}

void Creature::Update(uint32 msTime, uint32 uiDiff)
{
    Unit::Update(msTime, uiDiff);

    if(hasStateFlag(UF_CORPSE))
    {
        if(m_despawnTimer > uiDiff)
            m_despawnTimer -= uiDiff;
        else Despawn(UF_CORPSE);
    }

    m_aiInterface.Update(msTime, uiDiff);

    // Combat handling, check if we have a script and if the script is not handling the combat update
    //if(m_script == NULL || !m_script->EventUpdateCombat(msTime, uiDiff))
        EventUpdateCombat(msTime, uiDiff);
}

void Creature::RemoveFromWorld()
{
    if(m_mapInstance && m_zoneId && m_zoneVisibleSpawn)
        m_mapInstance->RemoveZoneVisibleSpawn(m_zoneId, this);
    Unit::RemoveFromWorld();
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
    Unit::OnAuraModChanged(modType);
}

float Creature::GetAggroRange()
{
    float baseAggro = 20.f;
    // detect range auras
    baseAggro += m_aggroRangeMod;
    // Modifying based on rank
    baseAggro += ((float)_creatureData->rank)*1.25f;
    // Add half model size after modifiers
    baseAggro += GetModelHalfSize();
    return baseAggro;
}

void Creature::OnAddInRangeObject(WorldObject *pObj)
{
    if(pObj->IsUnit() && sFactionSystem.isHostile(this, pObj))
        m_inRangeHostiles.insert(castPtr<Unit>(pObj));
    Unit::OnAddInRangeObject(pObj);
}

void Creature::OnRemoveInRangeObject(WorldObject *pObj)
{
    if(pObj->GetGUID() == m_attackTarget)
        m_aiInterface.OnAttackStop();

    Unit::OnRemoveInRangeObject(pObj);
    if(pObj->IsUnit())
    {
        WorldObject::InRangeArray::iterator itr;
        m_inRangeHostiles.erase(castPtr<Unit>(pObj));
    }
}

void Creature::CheckTriggerRange(Unit *uObj, float distSq)
{

}

void Creature::UpdateInRangeObject(WorldObject *pObj)
{
    if(!pObj->IsUnit())
        return;

    Unit *uObj = castPtr<Unit>(pObj);
    bool isHostile = sFactionSystem.isHostile(this, pObj);
    if(isHostile == false)
        m_inRangeHostiles.erase(uObj);
    else if(m_inRangeHostiles.find(uObj) == m_inRangeHostiles.end())
        m_inRangeHostiles.insert(uObj);
}

void Creature::ClearInRangeObjects()
{
    m_inRangeHostiles.clear();
    Unit::ClearInRangeObjects();
}

void Creature::UpdateLootAnimation(Player* Looter)
{

}

void Creature::ClearTag()
{
    if( isAlive() )
    {
        ClearLoot();
        m_taggingGroup = m_taggingPlayer = 0;
        m_lootMethod = -1;

        // if we are alive, means that we left combat
        if( IsInWorld() )
            UpdateLootAnimation(NULL);
    }
    // dead, don't clear tag
}

void Creature::Tag(Player* plr)
{
    // Tagging
    if( m_taggingPlayer != 0 || isCritter() )
        return;

    m_taggingPlayer = plr->GetLowGUID();
    m_taggingGroup = plr->getPlayerInfo()->m_Group ? plr->getPlayerInfo()->m_Group->GetID() : 0;

    /* set loot method */
    if( Group *grp = plr->GetGroup() )
        m_lootMethod = grp->GetMethod();

    // update tag visual
    UpdateLootAnimation(plr);
}

void Creature::EventUpdateCombat(uint32 msTime, uint32 uiDiff)
{
    if(!hasStateFlag(UF_ATTACKING) || (isCasting() && GetCurrentSpell()->GetSpellProto()->isSpellAttackInterrupting()))
        return;

    // Handle our creature spell casting
    if(!m_AuraInterface.HasAurasWithModType(SPELL_AURA_MOD_SILENCE))
    {   // Check our individual spells to cast
        for(std::vector<CreatureSpell*>::iterator itr = m_combatSpells.begin(); itr != m_combatSpells.end(); itr++)
        {
            CreatureSpell *cSpell = (*itr);
            if(cSpell->castTimer > uiDiff)
            {
                cSpell->castTimer -= uiDiff;
                continue;
            }

            // Soft reset for timer
            cSpell->castTimer = 500;
            if(!sSpellMgr.CanCastCreatureCombatSpell(cSpell->spellEntry, this))
                continue;

            SpellCastTargets targets(m_attackTarget);
            if(!sSpellMgr.GenerateCreatureCombatSpellTargets(cSpell->spellEntry, this, &targets, m_attackTarget))
            {
                targets.m_targetMask |= TARGET_FLAG_UNIT;
                targets.m_unitTarget = m_attackTarget;
            }

            if(targets.m_targetMask & TARGET_FLAG_UNIT)
            {
                Unit *spellTarget = GetInRangeObject<Unit>(targets.m_unitTarget);
                if(spellTarget == NULL)
                    continue;
                if(cSpell->spellEntry->isSpellAuraApplicator() && spellTarget->HasAura(cSpell->spellEntry->Id))
                    continue;
            }

            cSpell->castTimer = cSpell->cooldownTimer;
            if(Spell *spell = new Spell(this, cSpell->spellEntry))
            {

                if(spell->prepare(&targets, false) == SPELL_CANCAST_OK)
                {
                    // Cancel auto attack if we have to cast
                    if(cSpell->spellEntry->CastingTimeIndex)
                        return;
                    break;
                }
            }
        }
    }

    // Update our auto attack states
    UpdateAutoAttackState();
}

void Creature::EventAttackStop()
{
    Unit::EventAttackStop();
    SetUInt64Value(UNIT_FIELD_TARGET, 0);
}

int32 Creature::GetBaseAttackTime(uint8 weaponType)
{
    if(uint32 weaponDisplay = GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+weaponType))
    {
        ItemPrototype *proto = sItemMgr.LookupEntry(weaponDisplay);
        if(proto == NULL || proto->Class != ITEM_CLASS_WEAPON)
            return 0;
        return proto->Delay;//weaponType == RANGED ? _creatureData->rangedAttackTime : _creatureData->meleeAttackTime;
    }
    return weaponType == MELEE ? 2000 : 0;
}

void Creature::SafeDelete()
{

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
    // Todo: update this function
    return;

    m_spawn->x = (!saveposition && (m_spawn != NULL)) ? m_spawn->x : m_position.x;
    m_spawn->y = (!saveposition && (m_spawn != NULL)) ? m_spawn->y : m_position.y;
    m_spawn->z = (!saveposition && (m_spawn != NULL)) ? m_spawn->z : m_position.z;
    m_spawn->o = (!saveposition && (m_spawn != NULL)) ? m_spawn->o : m_position.o;

    std::stringstream ss;
    ss << "REPLACE INTO creature_spawns VALUES("
        << uint32(m_objGuid.getLow()) << ","
        << uint32(m_objGuid.getEntry()) << ","
        << uint32(GetMapId()) << ","
        << float(m_spawn->x) << ","
        << float(m_spawn->y) << ","
        << float(m_spawn->z) << ","
        << float(m_spawn->o) << ","
        << uint32(m_spawn->modelId) << ","
        << int32(m_spawn->vendormask) << " )";
    WorldDatabase.Execute(ss.str().c_str());
}

void Creature::DeleteFromDB()
{
    if( m_spawn == NULL )
        return;

    return;
    WorldDatabase.Execute("DELETE FROM creature_spawns WHERE id=%u", GetLowGUID());
    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE spawnid=%u", GetLowGUID());
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
    Unit::SetDeathState(s == JUST_DIED ? DEAD : s);
    if(s == JUST_DIED)
    {
        m_aiInterface.OnDeath();

        //despawn all summons we created
        SummonExpireAll(true);

        if(m_enslaveSpell)
            RemoveEnslave();

        if(Spell *currentSpell = m_currentSpell)
            currentSpell->cancel();
        m_currentSpell = NULL;

        if (_creatureData->rank == ELITE_WORLDBOSS || _creatureData->flags & CREATURE_FLAGS1_BOSS)
            m_despawnTimer = TIME_CREATURE_REMOVE_BOSSCORPSE;
        else if ( _creatureData->rank == ELITE_RAREELITE || _creatureData->rank == ELITE_RARE)
            m_despawnTimer = TIME_CREATURE_REMOVE_RARECORPSE;
        else m_despawnTimer = TIME_CREATURE_REMOVE_CORPSE;

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

void Creature::BuildTrainerData(WorldPacket *data, Player *plr)
{
    *data << GetGUID();
    *data << uint32(m_trainerData ? m_trainerData->category : 0);
    *data << uint32(m_trainerData ? m_trainerData->subCategory : 0);
    if(m_trainerData)
    {
        size_t dataPos = data->wpos();
        uint32 count = 0;
        *data << uint32(count);
        if(ObjectMgr::TrainerSpellMap *trainerSpells = objmgr.GetTrainerSpells(m_trainerData->category, m_trainerData->subCategory))
        {
            for(ObjectMgr::TrainerSpellMap::iterator itr = trainerSpells->begin(); itr != trainerSpells->end(); itr++)
            {
                TrainerSpell spell = itr->second;
                // Creatures below 10 can only teach spells at or below their level
                if(getLevel() < 10 && spell.requiredLevel > getLevel())
                    continue;

                *data << uint32(spell.entry->Id);
                *data << uint8(plr->GetTrainerSpellStatus(&spell));
                *data << uint32(spell.spellCost);
                *data << uint8(spell.requiredLevel);
                *data << uint32(spell.reqSkill);
                *data << uint32(spell.reqSkillValue);
                // Chain spelling
                *data << uint32(0);
                *data << uint32(0);
                // Profession
                *data << uint32(0);
                *data << uint32(0);
                count++;
            }
        }
        data->put<uint32>(dataPos, count);
        *data << m_trainerData->trainerTitle;
    } else *data << uint32(0) << uint8(0);
}

bool Creature::CanTrainPlayer(Player *plr)
{
    if(m_trainerData == NULL)
        return false;
    if(m_trainerData->category == TRAINER_CATEGORY_TALENTS && m_trainerData->subCategory != plr->getClass())
        return false;
    if(uint16 skillLine = m_trainerData->reqSkill)
    {
        if(!plr->HasSkillLine(skillLine))
            return false;
        if(plr->getSkillLineVal(skillLine) < m_trainerData->reqSkillValue)
            return false;
    }
    return true;
}

void Creature::UpdateAreaInfo(MapInstance *instance)
{
    uint32 lastZone = m_zoneId;
    Unit::UpdateAreaInfo(instance);

    // Team based area code
    uint8 team = TEAM_NONE;
    if((team = GetTeam()) < TEAM_GUARD)
    {
        uint8 areaFlags = GetAreaFlags();
        if(areaFlags & OBJECT_AREA_FLAG_INSANCTUARY)
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    if(m_zoneVisibleSpawn)
    {
        if(lastZone)
            instance->RemoveZoneVisibleSpawn(lastZone, this);
        if(m_zoneId)
            instance->AddZoneVisibleSpawn(m_zoneId, this);
    }
}

void Creature::Respawn(bool addrespawnevent, bool free_guid)
{
    m_taggingPlayer = m_taggingGroup = 0;
    m_lootMethod = 1;

    m_AuraInterface.RemoveAllNonPassiveAuras();

    uint32 delay = 0;
    if(addrespawnevent && _creatureData->respawnTime > 0)
        delay = _creatureData->respawnTime;
    Despawn(delay);
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

void Creature::RegeneratePower(bool isinterrupted)
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

uint32 Creature::GetItemIdBySlot(uint16 slot)
{
    if(slot >= m_vendorItems.size())
        return 0xFFFFFFFF;
    return m_vendorItems[slot].proto->ItemId;
}

uint32 Creature::GetSlotByItemId(uint32 itemid)
{
    for(uint32 i = 0; i < m_vendorItems.size(); i++)
        if(m_vendorItems[i].proto->ItemId == itemid)
            return i;
    return 0xFFFFFFFF;
}

int32 Creature::GetAvailableAmount(uint16 slot, int32 defaultVal)
{
    if(slot >= m_vendorItems.size())
        return defaultVal;
    return m_vendorItems[slot].availableAmount;
}

AvailableCreatureItem *Creature::GetSellItemBySlot(uint16 slot)
{
    if(slot >= m_vendorItems.size())
        return NULL;
    return &m_vendorItems[slot];
}

void Creature::SendInventoryList(Player *plr)
{
    plr->ClearVendorIndex();
    uint32 counter = 0;
    ByteBuffer dataBuff;
    std::vector<bool> bitFlags;
    WorldPacket data(SMSG_LIST_INVENTORY, 10);
    if(HasItems())
    {
        uint32 slot = 0;
        for(std::vector<AvailableCreatureItem>::iterator itr = m_vendorItems.begin(); itr != m_vendorItems.end(); itr++, slot++)
        {
            AvailableCreatureItem *item = &(*itr);

            // Update code for available items
            int32 availableAmount = -1;
            if(int32 maxAmount = item->max_amount)
            {
                if(item->refreshTime && UNIXTIME > item->refreshTime)
                {
                    uint32 diff = uint32(item->refreshTime-UNIXTIME);
                    if(diff >= item->incrtime)
                    {
                        uint32 val = item->proto->BuyCount*float2int32(floor(diff/item->incrtime));
                        if(item->availableAmount+val >= item->max_amount)
                        {
                            item->availableAmount = item->max_amount;
                            item->refreshTime = 0;
                        }
                        else
                        {
                            item->availableAmount += val;
                            item->refreshTime += (diff-(val * item->incrtime));
                        }
                    }
                }
                availableAmount = item->availableAmount;
            }

            if(item->IsDependent)
            {
                if(item->proto->AllowableClass && !(plr->getClassMask() & item->proto->AllowableClass))
                    continue;
                if(item->proto->AllowableRace && !(plr->getRaceMask() & item->proto->AllowableRace))
                    continue;

                if(item->proto->Class == ITEM_CLASS_ARMOR && item->proto->SubClass >= ITEM_SUBCLASS_ARMOR_LIBRAM && item->proto->SubClass <= ITEM_SUBCLASS_ARMOR_SIGIL)
                    if(!(plr->GetArmorProficiency() & (uint32(1) << item->proto->SubClass)))
                        continue; // Do not show relics to classes that can't use them.
            }

            // If we can't buy this, then we can't buy anything above it either
            if(item->extended_cost && item->extended_cost->flags & 0x01)
                break; // Calculate if we've reached the correct guild rank to view these items

            dataBuff << (++counter);
            dataBuff << item->proto->Durability;
            ItemExtendedCostEntry *extended = NULL;
            if(extended = item->extended_cost)
            {
                dataBuff << uint32(extended->Id);
                bitFlags.push_back(false);
            } else bitFlags.push_back(true);
            bitFlags.push_back(true);

            dataBuff << item->proto->ItemId << uint32(1);
            dataBuff << uint32(sItemMgr.CalculateBuyPrice(item->proto->ItemId, 1, plr, this, extended));
            dataBuff << uint32(item->proto->DisplayInfoID) << availableAmount;
            dataBuff << uint32(item->proto->BuyCount);
            plr->AddVendorIndex(counter, slot);
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
    plr->PushPacket( &data );
    sLog.Debug( "WORLD"," Sent SMSG_LIST_INVENTORY" );
}

void Creature::AddVendorItem(uint32 itemid, uint32 vendormask, uint32 ec)
{
    uint16 slot = m_vendorItems.size();
    if(slot == 0xFFFF)
        return;

}

int32 Creature::ModAvItemAmount(uint32 slot, uint32 value)
{
    if(slot >= m_vendorItems.size())
        return 0;

    AvailableCreatureItem &item = m_vendorItems[slot];
    if(value >= item.availableAmount)
        item.availableAmount = 0;
    else item.availableAmount -= value;
    return item.availableAmount;
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

}

void Creature::Load(uint32 mapId, float x, float y, float z, float o, uint32 mode, CreatureSpawn *spawn, bool reload)
{
    // Set our spawn pointer
    m_spawn = spawn;

    // Set worldobject Create data
    _Create(mapId, x, y, z, o);

    // Event objects should be spawned inactive
    if(m_spawn && (m_spawn->eventId || m_spawn->conditionId))
    {
        if(m_spawn->eventId) m_inactiveFlags |= OBJECT_INACTIVE_FLAG_EVENTS;
        if(m_spawn->conditionId) m_inactiveFlags |= OBJECT_INACTIVE_FLAG_CONDITION;
        WorldObject::Deactivate(5000);
    }

    // Set our extra data pointer
    _extraInfo = CreatureInfoExtraStorage.LookupEntry(GetEntry());

    // Set our phase mask
    m_phaseMask = m_spawn ? m_spawn->phaseMask : m_phaseMask;

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
    {
        if(CreatureDisplayInfoExtraEntry *extraInfo = dbcCreatureDisplayInfoExtra.LookupEntry(displayEntry->ExtraDisplayInfoEntry))
            race = extraInfo->Race;

        if(displayEntry->sizeClass >= 4)
            m_zoneVisibleSpawn = true;
    }

    SetByte(UNIT_FIELD_BYTES_0, 0, race);
    SetByte(UNIT_FIELD_BYTES_0, 1, _creatureData->Class);
    SetByte(UNIT_FIELD_BYTES_0, 2, gender);
    SetUInt32Value(UNIT_FIELD_DISPLAYID, model);
    SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, model);

    setLevel(level);
    SetFloatValue(OBJECT_FIELD_SCALE_X, _creatureData->scale);
    SetUInt32Value(UNIT_FIELD_BASEATTACKTIME, _creatureData->meleeAttackTime);
    SetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, _creatureData->rangedAttackTime);

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, _creatureData->inventoryItem[0]);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+1, _creatureData->inventoryItem[1]);
    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID+2, _creatureData->inventoryItem[2]);

    SetFaction(_creatureData->faction, false);
    //SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, _creatureData->boundingRadius * _creatureData->scale);
    //SetFloatValue(UNIT_FIELD_COMBATREACH, _creatureData->combatReach * _creatureData->scale);

    UpdateFieldValues();

    // Max out health
    SetUInt32Value(UNIT_FIELD_HEALTH, GetUInt32Value(UNIT_FIELD_MAXHEALTH));

    // Set our unit Flags from creature data
    SetUInt32Value(UNIT_FIELD_FLAGS, _creatureData->flags);
    SetUInt32Value(UNIT_FIELD_FLAGS_2, _creatureData->flags2);

    // Reset aurastate
    SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    // Set our npc flags and apply any variable data
    SetUInt32Value(UNIT_NPC_FLAGS, _creatureData->NPCFLags);

    // Reset dynamic flags
    SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);

    if (!reload && HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR))
        objmgr.FillVendorList(GetEntry(), GetVendorMask(), m_vendorItems);

    if(!reload && HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER|UNIT_NPC_FLAG_TRAINER_PROF))
        m_trainerData = objmgr.GetTrainerData(GetEntry());

    if (!reload && HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TAXIVENDOR))
        sTaxiMgr.GetNearestTaxiNodes(mapId, x, y, z, m_taxiNode);

    if (!reload && HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
        _LoadQuests();

    if (!reload && HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER))
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

    // Push our waypoints through to AI
    m_aiInterface.SetWaypoints(m_spawn ? &m_spawn->m_waypointData : NULL);
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
        TaxiNodeEntry *taxiNode = dbcTaxiNode.LookupEntry(curloc);
        if(!plr->HasTaxiNode(curloc)) // Check for known nodes
        {
            plr->AddTaxiMask(curloc);

            plr->GetSession()->OutPacket(SMSG_NEW_TAXI_PATH);

            //Send packet
            data.Initialize(SMSG_TAXINODE_STATUS, 9);
            data << GetGUID() << uint8(1);
        }
        else if(!plr->HasNearbyTaxiNodes(curloc))
            data << uint32(2) << GetGUID();
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
    plr->PushPacket( &data );
}
