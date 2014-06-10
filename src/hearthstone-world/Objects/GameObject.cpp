/***
 * Demonstrike Core
 */

#include "StdAfx.h"

GameObject::GameObject(uint64 guid)
{
    m_objectTypeId = TYPEID_GAMEOBJECT;
    m_valuesCount = GAMEOBJECT_END;
    m_uint32Values = _fields;
    memset(m_uint32Values, 0,(GAMEOBJECT_END)*sizeof(uint32));
    m_updateMask.SetCount(GAMEOBJECT_END);
    SetUInt32Value( OBJECT_FIELD_TYPE,TYPEMASK_GAMEOBJECT|TYPEMASK_OBJECT);
    SetUInt64Value( OBJECT_FIELD_GUID,guid);
    m_wowGuid.Init(GetGUID());
    SetFloatValue( OBJECT_FIELD_SCALE_X, 1);
    SetAnimProgress(100);
    counter = 0;
    bannerslot = bannerauraslot = -1;
    m_summonedGo = false;
    invisible = false;
    invisibilityFlag = INVIS_FLAG_NORMAL;
    spell = NULL;
    m_summoner = NULLUNIT;
    charges = -1;
    m_ritualmembers = NULL;
    m_rotation = 0;
    m_quests = NULL;
    pInfo = NULL;
    myScript = NULL;
    m_spawn = NULL;
    m_deleted = false;
    m_created = false;
    m_respawnCell = NULL;
    m_battleground = NULLBATTLEGROUND;
    initiated = false;
    m_loadedFromDB = false;
    memset(m_Go_Uint32Values, 0, sizeof(uint32)*GO_UINT32_MAX);
    m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] = 1;
    ChairListSlots.clear();
}

GameObject::~GameObject()
{

}

void GameObject::Init()
{
    Object::Init();
}

void GameObject::Destruct()
{
    if(m_ritualmembers)
        delete[] m_ritualmembers;

    uint32 guid = GetUInt32Value(OBJECT_FIELD_CREATED_BY);
    if(guid)
    {
        Player* plr = objmgr.GetPlayer(guid);
        if(plr && plr->GetSummonedObject() == TO_OBJECT(this) )
            plr->SetSummonedObject(NULLOBJ);

        if(plr == m_summoner)
            m_summoner = NULLOBJ;
    }

    if(m_respawnCell!=NULL)
        m_respawnCell->_respawnObjects.erase( TO_OBJECT(this) );

    if (m_summonedGo && m_summoner)
        for(int i = 0; i < 4; i++)
            if (m_summoner->m_ObjectSlots[i] == GetUIdFromGUID())
                m_summoner->m_ObjectSlots[i] = 0;

    if( m_battleground != NULL )
    {
        if( m_battleground->GetType() == BATTLEGROUND_ARATHI_BASIN )
        {
            if( bannerslot >= 0 && TO_ARATHIBASIN(m_battleground)->m_controlPoints[bannerslot] == TO_GAMEOBJECT(this) )
                TO_ARATHIBASIN(m_battleground)->m_controlPoints[bannerslot] = NULLGOB;

            if( bannerauraslot >= 0 && TO_ARATHIBASIN(m_battleground)->m_controlPointAuras[bannerauraslot] == TO_GAMEOBJECT(this) )
                TO_ARATHIBASIN(m_battleground)->m_controlPointAuras[bannerauraslot] = NULLGOB;
        }
        m_battleground = NULLBATTLEGROUND;
    }
    Object::Destruct();
}

bool GameObject::CreateFromProto(uint32 entry,uint32 mapid, const LocationVector vec)
{
    if(CreateFromProto(entry, mapid, vec.x, vec.y, vec.z, vec.o))
        return true;
    return false;
}

bool GameObject::CreateFromProto(uint32 entry,uint32 mapid, float x, float y, float z, float ang)
{
    pInfo = GameObjectNameStorage.LookupEntry(entry);
    if(!pInfo)
        return false;

    if(m_created) // Already created, just push back.
    {
        if(!initiated)
            InitAI();
        return true;
    }

    m_created = true;
    Object::_Create( mapid, x, y, z, ang );
    SetUInt32Value( OBJECT_FIELD_ENTRY, entry );

    UpdateRotation();

    SetState(1);
    SetDisplayId(pInfo->DisplayID );
    SetType(pInfo->Type);
    SetFlags(pInfo->DefaultFlags);
    InitAI();
    return true;
}

void GameObject::TrapSearchTarget()
{
    Update(200);
}

void GameObject::Update(uint32 p_time)
{
    if(m_event_Instanceid != m_instanceId)
    {
        event_Relocate();
        return;
    }

    if(!IsInWorld())
        return;

    if(m_deleted)
        return;

    if(spell != NULL && GetState() == 1 && GetType() != GAMEOBJECT_TYPE_AURA_GENERATOR)
    {
        if(checkrate > 1)
        {
            if(counter++%checkrate)
                return;
        }

        Object::InRangeSet::iterator itr,it2;
        Unit* pUnit;
        float dist;
        for( it2 = GetInRangeSetBegin(); it2 != GetInRangeSetEnd(); it2++)
        {
            itr = it2;
            dist = GetDistanceSq((*itr));
            if( (*itr) != m_summoner && (*itr)->IsUnit() && dist <= range)
            {
                pUnit = TO_UNIT(*itr);

                if(m_summonedGo)
                {
                    if(!m_summoner)
                    {
                        ExpireAndDelete();
                        return;
                    }
                    if(!sFactionSystem.isAttackable(m_summoner,pUnit))
                        continue;
                }

                Spell* sp = (new Spell(TO_OBJECT(this),spell,true,NULLAURA));
                SpellCastTargets tgt((*itr)->GetGUID());
                tgt.m_destX = GetPositionX();
                tgt.m_destY = GetPositionY();
                tgt.m_destZ = GetPositionZ();
                sp->prepare(&tgt);
                if(pInfo->Type == 6)
                {
                    if(m_summoner != NULL)
                        m_summoner->HandleProc(PROC_ON_TRAP_TRIGGER, NULL, pUnit, spell);
                }

                if(m_summonedGo)
                {
                    ExpireAndDelete();
                    return;
                }

                if(spell->EffectImplicitTargetA[0] == 16 ||
                    spell->EffectImplicitTargetB[0] == 16)
                    return;  // on area dont continue.
            }
        }
    }
}

void GameObject::Spawn( MapMgr* m)
{
    PushToWorld(m);
    CALL_GO_SCRIPT_EVENT(TO_GAMEOBJECT(this), OnSpawn)();
}

void GameObject::Despawn( uint32 delay, uint32 respawntime)
{
    if(delay)
    {
        sEventMgr.AddEvent(this, &GameObject::Despawn, (uint32)0, respawntime, EVENT_GAMEOBJECT_EXPIRE, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    if(!IsInWorld())
        return;

    m_loot.items.clear();

    //This is for go get deleted while looting
    if( m_spawn != NULL )
    {
        SetState(m_spawn->state);
        SetFlags(m_spawn->flags);
    }

    CALL_GO_SCRIPT_EVENT(this, OnDespawn)();

    if(respawntime)
    {
        /* Get our originiating mapcell */
        MapCell * pCell = m_mapCell;
        ASSERT(pCell);
        pCell->_respawnObjects.insert( TO_OBJECT(this) );
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnGameObject, TO_GAMEOBJECT(this), pCell, EVENT_GAMEOBJECT_ITEM_SPAWN, respawntime, 1, 0);
        Object::RemoveFromWorld(false);
        m_respawnCell=pCell;
    }
    else
    {
        Object::RemoveFromWorld(true);
        ExpireAndDelete();
    }
}

void GameObject::SaveToDB()
{
    if(m_spawn == NULL)
        return;

    std::stringstream ss;
    ss << "REPLACE INTO gameobject_spawns VALUES("
        << m_spawn->id << ","
        << GetEntry() << ","
        << GetMapId() << ","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << GetOrientation() << ","
        << ( GetByte(GAMEOBJECT_BYTES_1, 0)? 1 : 0 ) << ","
        << GetFlags() << ","
        << GetUInt32Value(GAMEOBJECT_FACTION) << ","
        << GetFloatValue(OBJECT_FIELD_SCALE_X) << ","
        << m_phaseMask << ")";

    WorldDatabase.Execute(ss.str().c_str());
}

void GameObject::InitAI()
{
    if(pInfo == NULL || initiated)
        return;

    initiated = true; // Initiate after check, so we do not spam if we return without a point.
    myScript = sScriptMgr.CreateAIScriptClassForGameObject(GetEntry(), this);

    uint32 spellid = 0;
    switch(pInfo->Type)
    {
    case GAMEOBJECT_TYPE_TRAP:
        {
            spellid = pInfo->GetSpellID();
        }break;
    case GAMEOBJECT_TYPE_SPELL_FOCUS://redirect to properties of another go
        {
            if( pInfo->TypeSpellFocus.LinkedTrapId == 0 )
                return;

            uint32 objectid = pInfo->TypeSpellFocus.LinkedTrapId;
            GameObjectInfo* gopInfo = GameObjectNameStorage.LookupEntry( objectid );
            if(gopInfo == NULL)
            {
                sLog.Warning("GameObject", "Redirected gameobject %u doesn't seem to exists in database, skipping", objectid);
                return;
            }

            if(gopInfo->RawData.ListedData[4])
                spellid = gopInfo->RawData.ListedData[4];
        }break;
    case GAMEOBJECT_TYPE_RITUAL:
        {
            m_ritualmembers = new uint32[pInfo->Arbiter.ReqParticipants];
            memset(m_ritualmembers, 0, (sizeof(uint32)*(pInfo->Arbiter.ReqParticipants)));
            return;
        }break;
    case GAMEOBJECT_TYPE_CHEST:
        {
            Lock *pLock = dbcLock.LookupEntry(pInfo->GetLockID());
            if(pLock)
            {
                for(uint32 i=0; i < 8; i++)
                {
                    if(pLock->locktype[i])
                    {
                        if(pLock->locktype[i] == 2) //locktype;
                        {
                            //herbalism and mining;
                            if(pLock->lockmisc[i] == LOCKTYPE_MINING || pLock->lockmisc[i] == LOCKTYPE_HERBALISM)
                                CalcMineRemaining(true);
                        }
                    }
                }
            }
            return;
        }break;
    case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
        {
            m_Go_Uint32Values[GO_UINT32_HEALTH] = pInfo->DestructableBuilding.IntactNumHits+pInfo->DestructableBuilding.DamagedNumHits;
            SetAnimProgress(255);
            return;
        }break;
    case GAMEOBJECT_TYPE_AURA_GENERATOR:
        {
            spellid = GetInfo()->AuraGenerator.AuraID1;
            sEventMgr.AddEvent(this, &GameObject::AuraGenSearchTarget, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            return;
        }break;
    }

    if(!spellid)
        return;

    SpellEntry *sp = dbcSpell.LookupEntry(spellid);
    if(!sp)
    {
        spellid = 0;
        spell = NULL;
        return;
    }
    else
        spell = sp;

    //ok got valid spell that will be casted on target when it comes close enough
    //get the range for that
    float r = 0;

    for(uint32 i = 0; i < 3; ++i)
    {
        if(sp->Effect[i])
        {
            float t = GetDBCRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[i]));
            if(t > r)
                r = t;
        }
    }

    if(r < 0.1)//no range
        r = GetDBCMaxRange(dbcSpellRange.LookupEntry(sp->rangeIndex));

    range = r*r;//square to make code faster
    checkrate = 20;//once in 2 seconds
}

bool GameObject::Load(GOSpawn *spawn)
{
    if(m_loadedFromDB)
        return true;

    if(!CreateFromProto(spawn->entry,0,spawn->x,spawn->y,spawn->z,spawn->facing))
        return false;

    m_spawn = spawn;
    SetPhaseMask(spawn->phase);
    SetFlags(spawn->flags);
    SetState(spawn->state);
    if(spawn->faction)
    {
        SetUInt32Value(GAMEOBJECT_FACTION,spawn->faction);
        m_factionTemplate = dbcFactionTemplate.LookupEntry(spawn->faction);
        if(m_factionTemplate)
            m_faction = dbcFaction.LookupEntry(m_factionTemplate->Faction);
    }
    SetFloatValue(OBJECT_FIELD_SCALE_X, spawn->scale);

    if( GetFlags() & GO_FLAG_IN_USE || GetFlags() & GO_FLAG_LOCKED )
        SetAnimProgress(100);

    CALL_GO_SCRIPT_EVENT(TO_GAMEOBJECT(this), OnCreate)();

    _LoadQuests();
    m_loadedFromDB = true;
    return true;
}

void GameObject::DeleteFromDB()
{
    if( m_spawn != NULL )
        WorldDatabase.Execute("DELETE FROM gameobject_spawns WHERE id=%u", m_spawn->id);
}

void GameObject::EventCloseDoor()
{
    SetState(0);
}

void GameObject::UseFishingNode(Player* player)
{
    sEventMgr.RemoveEvents( this );
    if( GetUInt32Value( GAMEOBJECT_FLAGS ) != 32 ) // Clicking on the bobber before something is hooked
    {
        player->GetSession()->OutPacket( SMSG_FISH_NOT_HOOKED );
        EndFishing( player, true );
        return;
    }

    FishingZoneEntry *entry = FishingZoneStorage.LookupEntry( GetAreaId() );
    if( entry == NULL ) // No fishing information found for area, log an error
    {
        entry = FishingZoneStorage.LookupEntry( GetZoneId() );
        if( entry == NULL ) // No fishing information found for area, log an error
        {
            sLog.outDebug( "ERROR: Fishing zone information for zone %d not found!", GetZoneId() );
            EndFishing( player, true );
            return;
        }
    }

    uint32 maxskill = entry->MaxSkill;
    uint32 minskill = entry->MinSkill;

    if( player->_GetSkillLineCurrent( SKILL_FISHING, false ) < maxskill )
        player->_AdvanceSkillLine( SKILL_FISHING, float2int32( 1.0f * sWorld.getRate( RATE_SKILLRATE ) ) );

    // Open loot on success, otherwise FISH_ESCAPED.
    if( Rand(((player->_GetSkillLineCurrent( SKILL_FISHING, true ) - minskill) * 100) / maxskill) )
    {
        lootmgr.FillFishingLoot( &m_loot, entry->ZoneID );
        player->SendLoot( GetGUID(), GetMapId(), LOOT_FISHING );
        EndFishing( player, false );
    }
    else // Failed
    {
        player->GetSession()->OutPacket( SMSG_FISH_ESCAPED );
        EndFishing( player, true );
    }

}

void GameObject::EndFishing(Player* player, bool abort )
{
    Spell* spell = player->GetCurrentSpell();

    if(spell)
    {
        if(abort)   // abort becouse of a reason
        {
            //FIXME: here 'failed' should appear over progress bar
            spell->SendChannelUpdate(0);
            spell->finish();
        }
        else        // spell ended
        {
            spell->SendChannelUpdate(0);
            spell->finish();
        }
    }

    if(!abort)
        TO_GAMEOBJECT(this)->ExpireAndDelete(20000);
    else
        ExpireAndDelete();
}

void GameObject::FishHooked(Player* player)
{
    WorldPacket  data(12);
    data.Initialize(SMSG_GAMEOBJECT_CUSTOM_ANIM);
    data << GetGUID();
    data << (uint32)0; // value < 4
    player->GetSession()->SendPacket(&data);
    SetFlags(32);
 }

/////////////
/// Quests

void GameObject::AddQuest(QuestRelation *Q)
{
    m_quests->push_back(Q);
}

void GameObject::DeleteQuest(QuestRelation *Q)
{
    list<QuestRelation *>::iterator it;
    for( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        if( ( (*it)->type == Q->type ) && ( (*it)->qst == Q->qst ) )
        {
            delete (*it);
            m_quests->erase(it);
            break;
        }
    }
}

Quest* GameObject::FindQuest(uint32 quest_id, uint8 quest_relation)
{
    list< QuestRelation* >::iterator it;
    for( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        QuestRelation* ptr = (*it);
        if( ( ptr->qst->id == quest_id ) && ( ptr->type & quest_relation ) )
        {
            return ptr->qst;
        }
    }
    return NULL;
}

uint16 GameObject::GetQuestRelation(uint32 quest_id)
{
    uint16 quest_relation = 0;
    list< QuestRelation* >::iterator it;
    for( it = m_quests->begin(); it != m_quests->end(); it++ )
    {
        if( (*it) != NULL && (*it)->qst->id == quest_id )
        {
            quest_relation |= (*it)->type;
        }
    }
    return quest_relation;
}

uint32 GameObject::NumOfQuests()
{
    return (uint32)m_quests->size();
}

void GameObject::_LoadQuests()
{
    sQuestMgr.LoadGOQuests(TO_GAMEOBJECT(this));

    // set state for involved quest objects
    if( pInfo && objmgr.GetInvolvedQuestIds(pInfo->ID) != NULL )
    {
        SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
        SetState(0);
        SetFlags(GO_FLAG_IN_USE);
    }
}

/////////////////
// Summoned Go's
//guardians are temporary spawn that will inherit master faction and will folow them. Apart from that they have their own mind
Unit* GameObject::CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, Unit* u_caster, uint8 Slot)
{
    CreatureProto * proto = CreatureProtoStorage.LookupEntry(guardian_entry);
    CreatureInfo * info = CreatureNameStorage.LookupEntry(guardian_entry);
    if(!proto || !info)
    {
        sLog.outDebug("Warning : Missing summon creature template %u !",guardian_entry);
        return NULLUNIT;
    }
    uint32 lvl = u_caster->getLevel();
    LocationVector v = GetPositionNC();
    float m_followAngle = angle + v.o;
    float x = v.x +(3*(cosf(m_followAngle)));
    float y = v.y +(3*(sinf(m_followAngle)));
    Creature* p = GetMapMgr()->CreateCreature(guardian_entry);
    if(p == NULLCREATURE)
        return NULLUNIT;

    p->SetInstanceID(GetMapMgr()->GetInstanceID());
    p->Load(proto, GetMapMgr()->iInstanceMode, x, y, v.z, angle);

    if(lvl != 0)
    {
        /* POWER */
        p->SetPowerType(p->GetProto()->Powertype);
        p->SetUInt32Value(UNIT_FIELD_MAXPOWER1 + p->GetProto()->Powertype, p->GetUInt32Value(UNIT_FIELD_MAXPOWER1 + p->GetProto()->Powertype)+28+10*lvl);
        p->SetUInt32Value(UNIT_FIELD_POWER1 + p->GetProto()->Powertype, p->GetUInt32Value(UNIT_FIELD_POWER1 + p->GetProto()->Powertype)+28+10*lvl);
        p->SetUInt32Value(UNIT_FIELD_MAXHEALTH,p->GetUInt32Value(UNIT_FIELD_MAXHEALTH)+28+30*lvl);
        p->SetUInt32Value(UNIT_FIELD_HEALTH,p->GetUInt32Value(UNIT_FIELD_HEALTH)+28+30*lvl);
        /* LEVEL */
        p->SetUInt32Value(UNIT_FIELD_LEVEL, lvl);
    }

    p->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GetGUID());
    p->SetUInt64Value(UNIT_FIELD_CREATEDBY, GetGUID());
    p->SetZoneId(GetZoneId());
    p->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,u_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    p->_setFaction();

    p->GetAIInterface()->Init(p,AITYPE_PET,MOVEMENTTYPE_NONE,u_caster);
    p->GetAIInterface()->SetUnitToFollow(TO_UNIT(this));
    p->GetAIInterface()->SetUnitToFollowAngle(angle);
    p->GetAIInterface()->SetFollowDistance(3.0f);

    p->PushToWorld(GetMapMgr());

    if(duration)
        sEventMgr.AddEvent(TO_UNIT(this), &Unit::SummonExpireSlot,Slot, EVENT_SUMMON_EXPIRE_0+Slot, duration, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );

    return p;

}

void GameObject::_Expire()
{
    if(IsInWorld())
        Object::RemoveFromWorld(true);

    Destruct();
}

void GameObject::ExpireAndDelete()
{
    ExpireAndDelete(1); // Defaults to 1, so set to 1 for non delay including calls.
}

void GameObject::ExpireAndDelete(uint32 delay)
{
    if(m_deleted)
        return;

    delay = delay <= 0 ? 1 : delay;

    if(delay == 1)// we're to be deleted next loop, don't update go anymore.
        m_deleted = true;

    if(sEventMgr.HasEvent(this,EVENT_GAMEOBJECT_EXPIRE))
        sEventMgr.ModifyEventTimeLeft(this, EVENT_GAMEOBJECT_EXPIRE, delay);
    else
        sEventMgr.AddEvent(this, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, delay, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void GameObject::OnPushToWorld()
{
    Object::OnPushToWorld();
}

void GameObject::OnRemoveInRangeObject(Object* pObj)
{
    Object::OnRemoveInRangeObject(pObj);
    if(m_summonedGo && m_summoner == pObj)
    {
        for(int i = 0; i < 4; i++)
            if (m_summoner->m_ObjectSlots[i] == GetUIdFromGUID())
                m_summoner->m_ObjectSlots[i] = 0;

        m_summoner = NULLUNIT;
        ExpireAndDelete();
    }
}

void GameObject::RemoveFromWorld(bool free_guid)
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET);
    Despawn(0, 0);
}

uint32 GameObject::GetGOReqSkill()
{
    if(GetInfo() == NULL)
        return 0;

    Lock *lock = dbcLock.LookupEntry( GetInfo()->GetLockID() );
    if(!lock)
        return 0;
    for(uint32 i=0; i < 8; ++i)
    {
        if(lock->locktype[i] == 2 && lock->minlockskill[i])
        {
            return lock->minlockskill[i];
        }
    }
    return 0;
}

void GameObject::GenerateLoot()
{

}

// Convert from radians to blizz rotation system
void GameObject::UpdateRotation(float orientation3, float orientation4)
{
    static double const rotationMath = atan(pow(2.0f, -20.0f));

    m_rotation = 0;
    double sinRotation = sin(GetOrientation() / 2.0f);
    double cosRotation = cos(GetOrientation() / 2.0f);

    if(cosRotation >= 0)
        m_rotation = (uint64)(sinRotation / rotationMath * 1.0f) & 0x1FFFFF;
    else
        m_rotation = (uint64)(sinRotation / rotationMath * -1.0f) & 0x1FFFFF;

    if(orientation3 == 0.0f && orientation4 == 0.0f)
    {
        orientation3 = (float) sinRotation;
        orientation4 = (float) cosRotation;
    }

    SetFloatValue(GAMEOBJECT_ROTATION, 0.0f);
    SetFloatValue(GAMEOBJECT_ROTATION_01, 0.0f);
    SetFloatValue(GAMEOBJECT_ROTATION_02, orientation3);
    SetFloatValue(GAMEOBJECT_ROTATION_03, orientation4);
}

void GameObject::SetDisplayId(uint32 id)
{
    SetUInt32Value( GAMEOBJECT_DISPLAYID, id );
    if(IsInWorld())
    {
        sVMapInterface.UpdateObjectModel(GetGUID(), GetMapId(), GetInstanceID(), id);
    }
}

//Destructable Buildings
void GameObject::TakeDamage(uint32 amount, Object* mcaster, Player* pcaster, uint32 spellid)
{
    if(GetType() != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return;

    if(HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED)) // Already destroyed
        return;

    uint32 IntactHealth = pInfo->DestructableBuilding.IntactNumHits;
    uint32 DamagedHealth = pInfo->DestructableBuilding.DamagedNumHits;

    if(m_Go_Uint32Values[GO_UINT32_HEALTH] > amount)
        m_Go_Uint32Values[GO_UINT32_HEALTH] -= amount;
    else
        m_Go_Uint32Values[GO_UINT32_HEALTH] = 0;

    if(HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED))
    {
        if(m_Go_Uint32Values[GO_UINT32_HEALTH] == 0)
        {
            Destroy();
            sHookInterface.OnDestroyBuilding(this);
        }
    }
    else if(!HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED) && m_Go_Uint32Values[GO_UINT32_HEALTH] <= DamagedHealth)
    {
        if(m_Go_Uint32Values[GO_UINT32_HEALTH] != 0)
        {
            Damage();
            sHookInterface.OnDamageBuilding(this);
        }
        else
        {
            Destroy();
            sHookInterface.OnDestroyBuilding(TO_GAMEOBJECT(this));
        }
    }

    WorldPacket data(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 20);
    data << GetNewGUID();
    data << mcaster->GetNewGUID();
    if(pcaster!=NULL)
        data << pcaster->GetNewGUID();
    else
        data << mcaster->GetNewGUID();
    data << uint32(amount);
    data << spellid;
    mcaster->SendMessageToSet(&data, (mcaster->IsPlayer() ? true : false));
    if(IntactHealth != 0 && DamagedHealth != 0)
        SetAnimProgress(m_Go_Uint32Values[GO_UINT32_HEALTH]*255/(IntactHealth + DamagedHealth));
}

void GameObject::Rebuild()
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED | GO_FLAG_DESTROYED);
    SetDisplayId(pInfo->DisplayID);
    uint32 IntactHealth = pInfo->DestructableBuilding.IntactNumHits;
    uint32 DamagedHealth = pInfo->DestructableBuilding.DamagedNumHits;
    m_Go_Uint32Values[GO_UINT32_HEALTH] = IntactHealth + DamagedHealth;
}

void GameObject::AuraGenSearchTarget()
{
    if(m_event_Instanceid != m_instanceId)
    {
        event_Relocate();
        return;
    }

    if(!IsInWorld() || m_deleted || !spell)
        return;

    Object::InRangeSet::iterator itr,it2;
    for( it2 = GetInRangeSetBegin(); it2 != GetInRangeSetEnd(); it2++)
    {
        itr = it2;
        Unit* thing = NULL; // Crow: Shouldn't radius be sq?
        if( (*itr)->IsUnit() && GetDistanceSq((*itr)) <= pInfo->AuraGenerator.Radius && ((*itr)->IsPlayer() || (*itr)->IsVehicle()) && !(TO_UNIT((*itr))->HasAura(spell->Id)))
        {
            thing = TO_UNIT((*itr));
            thing->AddAura(new Aura(spell, -1, thing, thing));
        }
    }
}

void GameObject::Damage()
{
    SetFlags(GO_FLAG_DAMAGED);
    if(pInfo->DestructableBuilding.DestructibleData != 0)
    {
        if(DestructibleModelDataEntry *display = NULL)//dbcDestructibleModelDataEntry.LookupEntry( pInfo->DestructableBuilding.DestructibleData ))
            SetDisplayId(display->GetDisplayId(1));
    }
    else
        SetDisplayId(pInfo->DestructableBuilding.DamagedDisplayId);
}

void GameObject::Destroy()
{
    RemoveFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED);
    SetFlags(GO_FLAG_DESTROYED);
    if(pInfo->DestructableBuilding.DestructibleData != 0)
    {
        if(DestructibleModelDataEntry *display = NULL)//dbcDestructibleModelDataEntry.LookupEntry( pInfo->DestructableBuilding.DestructibleData ))
            SetDisplayId(display->GetDisplayId(3));
    }
    else
        SetDisplayId(pInfo->DestructableBuilding.DestroyedDisplayId);
}

#define OPEN_CHEST 11437

void GameObject::Use(Player *p)
{
    m_scripted_use = false;
    Spell* spell = NULLSPELL;
    SpellEntry *spellInfo = NULL;
    SpellCastTargets targets;
    GameObjectInfo *goinfo = GetInfo();
    if (!goinfo)
        return;

    uint32 type = GetType();
    if(myScript != NULL)
        m_scripted_use = true;
    CALL_GO_SCRIPT_EVENT(this, OnActivate)(p);
    CALL_INSTANCE_SCRIPT_EVENT( p->GetMapMgr(), OnGameObjectActivate )( this, p );
    if(m_scripted_use)
        return;

    switch (type)
    {
    case GAMEOBJECT_TYPE_CHAIR:
        {
            if(goinfo->Chair.OnlyCreatorUse)
            {
                if(p->GetGUID() != GetUInt64Value(OBJECT_FIELD_CREATED_BY))
                    return;
            }

            if( p->IsMounted() )
                p->RemoveAura( p->m_MountSpellId );
            else
            {
                p->SetLastRunSpeed(0.0f);
                p->UpdateSpeed();
            }

            if (!ChairListSlots.size())
            {
                if (goinfo->Chair.Slots > 0)
                {
                    for (uint32 i = 0; i < goinfo->Chair.Slots; ++i)
                        ChairListSlots[i] = 0;
                }
                else
                    ChairListSlots[0] = 0;
            }

            uint32 nearest_slot = 0;
            float lowestDist = 90.0f;
            bool found_free_slot = false;
            float x_lowest = GetPositionX();
            float y_lowest = GetPositionY();
            float orthogonalOrientation = GetOrientation()+M_PI*0.5f;
            for (ChairSlotAndUser::iterator itr = ChairListSlots.begin(); itr != ChairListSlots.end(); ++itr)
            {
                float size = GetFloatValue(OBJECT_FIELD_SCALE_X);
                float relativeDistance = (size*itr->first)-(size*(goinfo->Chair.Slots-1)/2.0f);

                float x_i = GetPositionX() + relativeDistance * cos(orthogonalOrientation);
                float y_i = GetPositionY() + relativeDistance * sin(orthogonalOrientation);

                if (itr->second)
                    if (Player* ChairUser = objmgr.GetPlayer(itr->second))
                        if (ChairUser->IsSitting() && sqrt(ChairUser->GetDistance2dSq(x_i, y_i)) < 0.1f)
                            continue;
                        else
                            itr->second = 0;
                    else
                        itr->second = 0;

                found_free_slot = true;

                float thisDistance = p->GetDistance2dSq(x_i, y_i);

                if (thisDistance <= lowestDist)
                {
                    nearest_slot = itr->first;
                    lowestDist = thisDistance;
                    x_lowest = x_i;
                    y_lowest = y_i;
                }
            }

            if (found_free_slot)
            {
                ChairSlotAndUser::iterator itr = ChairListSlots.find(nearest_slot);
                if (itr != ChairListSlots.end())
                {
                    itr->second = p->GetGUID();
                    p->Teleport( x_lowest, y_lowest, GetPositionZ(), GetOrientation(), GetPhaseMask());
                    p->SetStandState(STANDSTATE_SIT_LOW_CHAIR+goinfo->Chair.Height);
                    return;
                }
            }
        }break;
    case GAMEOBJECT_TYPE_CHEST://cast da spell
        {
            spellInfo = dbcSpell.LookupEntry( OPEN_CHEST );
            spell = (new Spell(p, spellInfo, true, NULLAURA));
            p->SetCurrentSpell(spell);
            targets.m_unitTarget = GetGUID();
            spell->prepare(&targets);
        }break;
    case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            UseFishingNode(p);
        }break;
    case GAMEOBJECT_TYPE_DOOR:
        {
            // door
            if((GetState() == 1) && (GetFlags() == 33))
                EventCloseDoor();
            else
            {
                SetFlags(33);
                SetState(0);
                sEventMgr.AddEvent(this,&GameObject::EventCloseDoor,EVENT_GAMEOBJECT_DOOR_CLOSE,20000,1,0);
            }
        }break;
    case GAMEOBJECT_TYPE_FLAGSTAND:
        {
            // battleground/warsong gulch flag
            if(p->m_bg)
            {
                if( p->m_stealth )
                    p->RemoveAura( p->m_stealth );

                if( p->m_MountSpellId )
                    p->RemoveAura( p->m_MountSpellId );

                if( p->GetVehicle() )
                    p->GetVehicle()->RemovePassenger( p );

                if(!p->m_bgFlagIneligible)
                    p->m_bg->HookFlagStand(p, this);
                CALL_INSTANCE_SCRIPT_EVENT( p->GetMapMgr(), OnPlayerFlagStand )( p, this );
            }
            else
                sLog.outError("Gameobject Type FlagStand activated while the player is not in a battleground, entry %u", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_FLAGDROP:
        {
            // Dropped flag
            if(p->m_bg)
            {
                if( p->m_stealth )
                    p->RemoveAura( p->m_stealth );

                if( p->m_MountSpellId )
                    p->RemoveAura( p->m_MountSpellId );

                if( p->GetVehicle() )
                    p->GetVehicle()->RemovePassenger( p );

                p->m_bg->HookFlagDrop(p, this);
                CALL_INSTANCE_SCRIPT_EVENT( p->GetMapMgr(), OnPlayerFlagDrop )( p, this );
            }
            else
                sLog.outError("Gameobject Type Flag Drop activated while the player is not in a battleground, entry %u", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_QUESTGIVER:
        {
            // Questgiver
            if(HasQuests())
                sQuestMgr.OnActivateQuestGiver(this, p);
            else
                sLog.outError("Gameobject Type Questgiver doesn't have any quests entry %u (May be false positive if object has a script)", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_SPELLCASTER:
        {
            SpellEntry *info = dbcSpell.LookupEntry(goinfo->GetSpellID());
            if(!info)
            {
                sLog.outError("Gameobject Type Spellcaster doesn't have a spell to cast entry %u", goinfo->ID);
                return;
            }
            Spell* spell(new Spell(p, info, false, NULLAURA));
            SpellCastTargets targets;
            targets.m_unitTarget = p->GetGUID();
            spell->prepare(&targets);
            if(charges > 0 && !--charges)
                ExpireAndDelete();
        }break;
    case GAMEOBJECT_TYPE_RITUAL:
        {
            // store the members in the ritual, cast sacrifice spell, and summon.
            uint32 i = 0;
            if(!m_ritualmembers || !GetGOui32Value(GO_UINT32_RIT_SPELL) || !GetGOui32Value(GO_UINT32_M_RIT_CASTER))
                return;

            for(i = 0; i < goinfo->Arbiter.ReqParticipants; i++)
            {
                if(!m_ritualmembers[i])
                {
                    m_ritualmembers[i] = p->GetLowGUID();
                    p->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GetGUID());
                    p->SetUInt32Value(UNIT_CHANNEL_SPELL, GetGOui32Value(GO_UINT32_RIT_SPELL));
                    break;
                }
                else if(m_ritualmembers[i] == p->GetLowGUID())
                {
                    // we're deselecting :(
                    m_ritualmembers[i] = 0;
                    p->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
                    p->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
                    return;
                }
            }

            if(i == goinfo->Arbiter.ReqParticipants - 1)
            {
                SetGOui32Value(GO_UINT32_RIT_SPELL, 0);
                Player* plr;
                for(i = 0; i < goinfo->Arbiter.ReqParticipants; i++)
                {
                    plr = p->GetMapMgr()->GetPlayer(m_ritualmembers[i]);
                    if(plr != NULL)
                    {
                        plr->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
                        plr->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
                    }
                }

                SpellEntry *info = NULL;
                switch( goinfo->ID )
                {
                case 36727:// summon portal
                    {
                        if(!GetGOui32Value(GO_UINT32_M_RIT_TARGET))
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        if(!info)
                            break;
                        Player* target = p->GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_TARGET));
                        if(!target)
                            return;

                        spell = (new Spell(this,info,true,NULLAURA));
                        SpellCastTargets targets;
                        targets.m_unitTarget = target->GetGUID();
                        spell->prepare(&targets);
                    }break;
                case 177193:// doom portal
                    {
                        Player* psacrifice = NULLPLR;
                        Spell* spell = NULLSPELL;

                        // kill the sacrifice player
                        psacrifice = p->GetMapMgr()->GetPlayer(m_ritualmembers[(int)(RandomUInt(goinfo->Arbiter.ReqParticipants-1))]);
                        Player* pCaster = GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!psacrifice || !pCaster)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->Arbiter.CasterTargetSpell);
                        if(!info)
                            break;
                        spell = (new Spell(psacrifice, info, true, NULLAURA));
                        targets.m_unitTarget = psacrifice->GetGUID();
                        spell->prepare(&targets);

                        // summons demon
                        info = dbcSpell.LookupEntry(goinfo->Arbiter.SpellId);
                        spell = (new Spell(pCaster, info, true, NULLAURA));
                        SpellCastTargets targets;
                        targets.m_unitTarget = pCaster->GetGUID();
                        spell->prepare(&targets);
                    }break;
                case 179944:// Summoning portal for meeting stones
                    {
                        Player* plr = p->GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_TARGET));
                        if(!plr)
                            return;

                        Player* pleader = p->GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        Spell* spell(new Spell(pleader, info, true, NULLAURA));
                        SpellCastTargets targets(plr->GetGUID());
                        spell->prepare(&targets);

                        /* expire the GameObject* */
                        ExpireAndDelete();
                    }break;
                case 194108:// Ritual of Summoning portal for warlocks
                    {
                        Player* pleader = p->GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        Spell* spell(new Spell(pleader, info, true, NULLAURA));
                        SpellCastTargets targets(pleader->GetGUID());
                        spell->prepare(&targets);

                        ExpireAndDelete();
                        pleader->InterruptCurrentSpell();
                    }break;
                case 186811://Ritual of Refreshment
                case 193062:
                    {
                        Player* pleader = p->GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        Spell* spell(new Spell(pleader, info, true, NULLAURA));
                        SpellCastTargets targets(pleader->GetGUID());
                        spell->prepare(&targets);

                        ExpireAndDelete();
                        pleader->InterruptCurrentSpell();
                    }break;
                case 181622://Ritual of Souls
                case 193168:
                    {
                        Player* pleader = p->GetMapMgr()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        Spell* spell(new Spell(pleader, info, true, NULLAURA));
                        SpellCastTargets targets(pleader->GetGUID());
                        spell->prepare(&targets);
                    }break;
                }
            }
        }break;
    case GAMEOBJECT_TYPE_GOOBER:
        {
            SpellEntry * sp = dbcSpell.LookupEntry(goinfo->GetSpellID());
            if(sp == NULL)
            {
                sLog.outError("Gameobject Type Goober doesn't have a spell to cast or page to read entry %u (May be false positive if object has a script)", goinfo->ID);
                return;
            }

            p->CastSpell(p, sp, false);
        }break;
    case GAMEOBJECT_TYPE_CAMERA://eye of azora
        {
            if(goinfo->Camera.CinematicId)
            {
                WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
                data << uint32(goinfo->Camera.CinematicId);
                p->GetSession()->SendPacket(&data);
            }
            else
                sLog.outError("Gameobject Type Camera doesn't have a cinematic to play id, entry %u", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_MEETINGSTONE:  // Meeting Stone
        {
            /* Use selection */
            Player* pPlayer = objmgr.GetPlayer((uint32)p->GetSelection());
            if(!pPlayer || p->GetGroup() != pPlayer->GetGroup() || !p->GetGroup())
                return;

            GameObjectInfo * info = GameObjectNameStorage.LookupEntry(179944);
            if(!info)
                return;

            /* Create the summoning portal */
            GameObject* pGo = p->GetMapMgr()->CreateGameObject(179944);
            if( pGo == NULL || !pGo->CreateFromProto(179944, p->GetMapId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 0.0f))
                return;

            // dont allow to spam them
            GameObject* gobj = TO_GAMEOBJECT(p->GetMapMgr()->GetObjectClosestToCoords(179944, p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 999999.0f, TYPEID_GAMEOBJECT));
            if( gobj )
                ExpireAndDelete();

            pGo->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p->GetLowGUID());
            pGo->SetGOui32Value(GO_UINT32_M_RIT_TARGET, pPlayer->GetLowGUID());
            pGo->SetGOui32Value(GO_UINT32_RIT_SPELL, 61994);
            pGo->PushToWorld(p->GetMapMgr());

            /* member one: the (w00t) caster */
            pGo->m_ritualmembers[0] = p->GetLowGUID();
            p->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pGo->GetGUID());
            p->SetUInt32Value(UNIT_CHANNEL_SPELL, pGo->GetGOui32Value(GO_UINT32_RIT_SPELL));

            /* expire after 2mins*/
            sEventMgr.AddEvent(pGo, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, 120000, 1,0);
        }break;
    case GAMEOBJECT_TYPE_BARBER_CHAIR:
        {
            p->SafeTeleport( p->GetMapId(), p->GetInstanceID(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation() );
            p->SetStandState(STANDSTATE_SIT_HIGH_CHAIR);
            if( p->IsMounted() )
                p->RemoveAura( p->m_MountSpellId );
            else
            {
                p->SetLastRunSpeed(0.0f);
                p->UpdateSpeed();
            }
            WorldPacket data(SMSG_ENABLE_BARBER_SHOP, 0);
            p->GetSession()->SendPacket(&data);
        }break;
    }
}