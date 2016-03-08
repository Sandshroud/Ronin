/***
 * Demonstrike Core
 */

#include "StdAfx.h"

GameObject::GameObject(uint64 guid, uint32 fieldCount) : WorldObject(guid, fieldCount)
{
    SetTypeFlags(TYPEMASK_TYPE_GAMEOBJECT);
    m_objType = TYPEID_GAMEOBJECT;

    m_updateFlags |= UPDATEFLAG_STATIONARY_POS|UPDATEFLAG_ROTATION;

    counter = 0;
    bannerslot = bannerauraslot = -1;
    m_summonedGo = false;
    invisible = false;
    invisibilityFlag = INVIS_FLAG_NORMAL;
    spell = NULL;
    m_summoner = NULL;
    charges = -1;
    m_ritualmembers = NULL;
    m_rotation.x = m_rotation.y = m_rotation.z = m_rotation.w = 0.f;
    m_quests = NULL;
    pInfo = NULL;
    m_spawn = NULL;
    m_deleted = false;
    m_created = false;
    m_respawnCell = NULL;
    m_battleground = NULL;
    initiated = false;
    memset(m_Go_Uint32Values, 0, sizeof(uint32)*GO_UINT32_MAX);
    m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] = 1;
    ChairListSlots.clear();
}

GameObject::~GameObject()
{

}

void GameObject::Init()
{
    SetAnimProgress(0xFF);

    WorldObject::Init();
}

void GameObject::Destruct()
{
    if(m_ritualmembers)
        delete[] m_ritualmembers;

    if(uint32 guid = GetUInt32Value(GAMEOBJECT_FIELD_CREATED_BY))
    {
        Player* plr = objmgr.GetPlayer(guid);
        if(plr && plr->GetSummonedObject() == this)
            plr->SetSummonedObject(NULL);

        if(plr == m_summoner)
            m_summoner = NULL;
    }

    if(m_respawnCell!=NULL)
        m_respawnCell->RemoveRespawn(this);

    if (m_summonedGo && m_summoner)
    {
        for(int i = 0; i < 4; i++)
        {
            if (m_summoner->m_ObjectSlots[i] == GetGUID())
                m_summoner->m_ObjectSlots[i] = 0;
        }
    }

    if( m_battleground != NULL )
    {
        if( m_battleground->GetType() == BATTLEGROUND_ARATHI_BASIN )
        {
            if( bannerslot >= 0 && castPtr<ArathiBasin>(m_battleground)->m_controlPoints[bannerslot] == castPtr<GameObject>(this) )
                castPtr<ArathiBasin>(m_battleground)->m_controlPoints[bannerslot] = NULL;

            if( bannerauraslot >= 0 && castPtr<ArathiBasin>(m_battleground)->m_controlPointAuras[bannerauraslot] == castPtr<GameObject>(this) )
                castPtr<ArathiBasin>(m_battleground)->m_controlPointAuras[bannerauraslot] = NULL;
        }
        m_battleground = NULL;
    }
    WorldObject::Destruct();
}

void GameObject::Update(uint32 p_time)
{
    if(m_event_MapId != m_mapId)
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

        for(WorldObject::InRangeSet::iterator itr = GetInRangeUnitSetBegin(); itr != GetInRangeUnitSetEnd(); itr++)
        {
            Unit *pUnit = GetInRangeObject<Unit>(*itr);
            if(pUnit != m_summoner && GetDistanceSq(pUnit) <= range)
            {
                if(m_summonedGo)
                {
                    if(!m_summoner)
                    {
                        ExpireAndDelete();
                        return;
                    }
                    if(!sFactionSystem.isAttackable(m_summoner, pUnit))
                        continue;
                }

                SpellCastTargets tgt(pUnit->GetGUID());
                tgt.m_dest = GetPosition();
                if(Spell* sp = new Spell(this, spell))
                    sp->prepare(&tgt, true);

                if(m_summonedGo)
                {
                    ExpireAndDelete();
                    return;
                }

                if(spell->isSpellAreaOfEffect())
                    return;
            }
        }
    }
}

bool GameObject::CreateFromProto(uint32 entry,uint32 mapid, const LocationVector vec, float ang, float r0, float r1, float r2, float r3)
{
    return CreateFromProto(entry, mapid, vec.x, vec.y, vec.z, ang, r0, r1, r2, r3);
}

bool GameObject::CreateFromProto(uint32 entry,uint32 mapid, float x, float y, float z, float ang, float r0, float r1, float r2, float r3)
{
    if((pInfo = GameObjectNameStorage.LookupEntry(entry)) == NULL)
        return false;

    if(m_created == false)
    {
        m_created = true;
        WorldObject::_Create( mapid, x, y, z, ang );
        if(pInfo->Type == GAMEOBJECT_TYPE_TRANSPORT || pInfo->Type == GAMEOBJECT_TYPE_MO_TRANSPORT)
            m_updateFlags |= (UPDATEFLAG_DYN_MODEL|UPDATEFLAG_TRANSPORT);

        SetUInt32Value( OBJECT_FIELD_ENTRY, entry );
        UpdateRotations(r0, r1, r2, r3);
        SetDisplayId(pInfo->DisplayID);
        SetFlags(pInfo->DefaultFlags);
        SetType(pInfo->Type);
        SetState(0x01);
        InitAI();
    }
    return true;
}

void GameObject::TrapSearchTarget()
{
    Update(200);
}

void GameObject::Spawn( MapInstance* m)
{
    PushToWorld(m);
    TRIGGER_GO_EVENT(castPtr<GameObject>(this), OnSpawn);
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

    GetLoot()->gold = 0;
    GetLoot()->items.clear();

    //This is for go get deleted while looting
    if( m_spawn != NULL )
    {
        SetState(m_spawn->state);
        SetFlags(m_spawn->flags);
    }

    TRIGGER_GO_EVENT(this, OnDespawn);

    if(respawntime)
    {
        /* Get our originiating mapcell */
        MapCell * pCell = m_mapCell;
        ASSERT(pCell);
        pCell->AddRespawn( this );
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapInstance, &MapInstance::EventRespawnGameObject, castPtr<GameObject>(this), pCell, EVENT_GAMEOBJECT_ITEM_SPAWN, respawntime, 1, 0);
        WorldObject::RemoveFromWorld();
        m_respawnCell = pCell;
    }
    else
    {
        WorldObject::RemoveFromWorld();
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
        << m_rotation.x << ", "
        << m_rotation.y << ", "
        << m_rotation.z << ", "
        << m_rotation.w << ", "
        << uint32( GetByte(GAMEOBJECT_BYTES_1, 0)? 1 : 0 ) << ","
        << GetFlags() << ","
        << GetUInt32Value(GAMEOBJECT_FACTION) << ","
        << GetFloatValue(OBJECT_FIELD_SCALE_X) << ","
        << uint32(0x01) << ")";

    WorldDatabase.Execute(ss.str().c_str());
}

void GameObject::InitAI()
{
    if(pInfo == NULL || initiated)
        return;

    initiated = true; // Initiate after check, so we do not spam if we return without a point.

    uint32 spellid = 0;
    switch(pInfo->Type)
    {
    case GAMEOBJECT_TYPE_TRAP:
        {
            spellid = pInfo->GetSpellID();
        }break;
    case GAMEOBJECT_TYPE_SPELL_FOCUS://redirect to properties of another go
        {
            if( pInfo->data.spellFocus.linkedTrapId == 0 )
                return;

            uint32 objectid = pInfo->data.spellFocus.linkedTrapId;
            GameObjectInfo* gopInfo = GameObjectNameStorage.LookupEntry( objectid );
            if(gopInfo == NULL)
            {
                sLog.Warning("GameObject", "Redirected gameobject %u doesn't seem to exists in database, skipping", objectid);
                return;
            }

            if(gopInfo->data.raw.data[4])
                spellid = gopInfo->data.raw.data[4];
        }break;
    case GAMEOBJECT_TYPE_RITUAL:
        {
            m_ritualmembers = new uint32[pInfo->data.ritual.reqParticipants];
            memset(m_ritualmembers, 0, (sizeof(uint32)*(pInfo->data.ritual.reqParticipants)));
            return;
        }break;
    case GAMEOBJECT_TYPE_CHEST:
        {
            if(LockEntry *pLock = dbcLock.LookupEntry(pInfo->GetLockID()))
            {
                for(uint32 i = 0; i < 8; i++)
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
            m_Go_Uint32Values[GO_UINT32_HEALTH] = pInfo->data.building.intactNumHits+pInfo->data.building.damagedNumHits;
            SetAnimProgress(255);
            return;
        }break;
    case GAMEOBJECT_TYPE_AURA_GENERATOR:
        {
            spellid = GetInfo()->data.auraGenerator.auraID1;
            sEventMgr.AddEvent(this, &GameObject::AuraGenSearchTarget, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            return;
        }break;
    }

    if(!spellid)
        return;

    if(SpellEntry *sp = dbcSpell.LookupEntry(spellid))
        spell = sp;
    else
    {
        spell = NULL;
        spellid = 0;
        return;
    }

    //ok got valid spell that will be casted on target when it comes close enough
    //get the range for that
    float r = 0;

    for(uint32 i = 0; i < 3; ++i)
    {
        if(spell->Effect[i])
        {
            float t = spell->radiusHostile[i];
            if(t > r)
                r = t;
        }
    }

    if(r < 0.1)//no range
        r = spell->maxRange[0];

    range = r*r;//square to make code faster
    checkrate = 20;//once in 2 seconds
}

bool GameObject::Load(uint32 mapId, GOSpawn *spawn, float angle)
{
    if(!CreateFromProto(spawn->entry, mapId, spawn->x, spawn->y, spawn->z, angle, spawn->r0, spawn->r1, spawn->r2, spawn->r3))
        return false;

    m_spawn = spawn;
    if(spawn->faction)
    {
        SetUInt32Value(GAMEOBJECT_FACTION,spawn->faction);
        m_factionTemplate = dbcFactionTemplate.LookupEntry(spawn->faction);
    }

    SetFlags(spawn->flags);
    SetState(spawn->state);
    SetFloatValue(OBJECT_FIELD_SCALE_X, spawn->scale);
    if( GetFlags() & GO_FLAG_IN_USE || GetFlags() & GO_FLAG_LOCKED )
        SetAnimProgress(100);

    TRIGGER_GO_EVENT(castPtr<GameObject>(this), OnCreate);

    _LoadQuests();
    return true;
}

void GameObject::UpdateRotations(float rotation0, float rotation1, float rotation2, float rotation3)
{
    SetFloatValue(GAMEOBJECT_PARENTROTATION+0, (m_rotation.x = rotation0));
    SetFloatValue(GAMEOBJECT_PARENTROTATION+1, (m_rotation.y = rotation1));
    if (rotation2 || rotation3)
    {
        SetFloatValue(GAMEOBJECT_PARENTROTATION+2, (m_rotation.z = rotation2));
        SetFloatValue(GAMEOBJECT_PARENTROTATION+3, (m_rotation.w = rotation3));
    }
    else if(!RONIN_UTIL::fuzzyEq(GetOrientation(), 0.f))
    {
        SetFloatValue(GAMEOBJECT_PARENTROTATION+2, (m_rotation.z = std::sin(GetOrientation() / 2.f)));
        SetFloatValue(GAMEOBJECT_PARENTROTATION+3, (m_rotation.w = std::cos(GetOrientation() / 2.f)));
    }
}

int64 GameObject::PackRotation(ObjectRotation *rotation)
{
    int8 w_sign = (rotation->w >= 0 ? 1 : -1);
    int64 X = int32(rotation->x * (1 << 21)) * w_sign & ((1 << 22) - 1);
    int64 Y = int32(rotation->y * (1 << 20)) * w_sign & ((1 << 21) - 1);
    int64 Z = int32(rotation->z * (1 << 20)) * w_sign & ((1 << 21) - 1);
    return uint64(Z | (Y << 21) | (X << 42));
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

    uint32 minskill = 0, maxskill = 500;
    if( player->_GetSkillLineCurrent( SKILL_FISHING, false ) < maxskill )
        player->_AdvanceSkillLine( SKILL_FISHING, float2int32( 1.0f * sWorld.getRate( RATE_SKILLRATE ) ) );

    // Open loot on success, otherwise FISH_ESCAPED.
    if( Rand(((player->_GetSkillLineCurrent( SKILL_FISHING, true ) - minskill) * 100) / maxskill) )
    {
        lootmgr.FillFishingLoot( GetLoot(), GetZoneId() );
        player->SendLoot( GetGUID(), GetMapId(), LOOTTYPE_FISHING );
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

    ExpireAndDelete(abort ? 0 : 20000);
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
    std::list<QuestRelation *>::iterator it;
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
    std::list< QuestRelation* >::iterator it;
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
    std::list< QuestRelation* >::iterator it;
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
    sQuestMgr.LoadGOQuests(castPtr<GameObject>(this));

    // set state for involved quest objects
    if( pInfo && lootmgr.GetGameObjectQuestLoot(pInfo->ID) )
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
    Creature* p = GetMapInstance()->CreateCreature(guardian_entry);
    if(p == NULL)
    {
        sLog.outDebug("Warning : Missing summon creature template %u !",guardian_entry);
        return NULL;
    }

    LocationVector v = GetPositionNC();
    float m_followAngle = angle + v.o;
    float x = v.x +(3*(cosf(m_followAngle)));
    float y = v.y +(3*(sinf(m_followAngle)));
    p->Load(GetMapId(), x, y, v.z, angle, GetMapInstance()->iInstanceMode);
    p->SetInstanceID(GetMapInstance()->GetInstanceID());
    p->setLevel(u_caster->getLevel());

    p->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, GetGUID());
    p->SetUInt64Value(UNIT_FIELD_CREATEDBY, GetGUID());
    p->SetZoneId(GetZoneId());
    p->SetFactionTemplate(u_caster->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    p->PushToWorld(GetMapInstance());

    if(duration)
        sEventMgr.AddEvent(castPtr<Unit>(this), &Unit::SummonExpireSlot,Slot, EVENT_SUMMON_EXPIRE_0+Slot, duration, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );

    return p;

}

void GameObject::_Expire()
{
    if(IsInWorld())
        WorldObject::RemoveFromWorld();

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
    else sEventMgr.AddEvent(this, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, delay, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void GameObject::OnPushToWorld()
{
    WorldObject::OnPushToWorld();
}

void GameObject::OnRemoveInRangeObject(WorldObject* pObj)
{
    WorldObject::OnRemoveInRangeObject(pObj);
    if(m_summonedGo && m_summoner == pObj)
    {
        for(int i = 0; i < 4; i++)
        {
            if (m_summoner->m_ObjectSlots[i] == GetGUID())
                m_summoner->m_ObjectSlots[i] = 0;
        }

        m_summoner = NULL;
        ExpireAndDelete();
    }
}

void GameObject::RemoveFromWorld()
{
    WorldObject::RemoveFromWorld();
    return;
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET);
    Despawn(0, 0);
}

uint32 GameObject::GetGOReqSkill()
{
    if(GetInfo() == NULL)
        return 0;

    LockEntry *lock = dbcLock.LookupEntry( GetInfo()->GetLockID() );
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

void GameObject::SetDisplayId(uint32 id)
{
    SetUInt32Value( GAMEOBJECT_DISPLAYID, id );
    if(IsInWorld())
    {
        sVMapInterface.UpdateObjectModel(GetGUID(), GetMapId(), GetInstanceID(), id);
    }
}

//Destructable Buildings
void GameObject::TakeDamage(uint32 amount, WorldObject* mcaster, Player* pcaster, uint32 spellid)
{
    if(GetType() != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return;

    if(HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DESTROYED)) // Already destroyed
        return;

    uint32 IntactHealth = pInfo->data.building.intactNumHits;
    uint32 DamagedHealth = pInfo->data.building.damagedNumHits;

    if(m_Go_Uint32Values[GO_UINT32_HEALTH] > amount)
        m_Go_Uint32Values[GO_UINT32_HEALTH] -= amount;
    else
        m_Go_Uint32Values[GO_UINT32_HEALTH] = 0;

    if(HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED))
    {
        if(m_Go_Uint32Values[GO_UINT32_HEALTH] == 0)
            SetStatusDestroyed();
    }
    else if(!HasFlag(GAMEOBJECT_FLAGS,GO_FLAG_DAMAGED) && m_Go_Uint32Values[GO_UINT32_HEALTH] <= DamagedHealth)
    {
        if(m_Go_Uint32Values[GO_UINT32_HEALTH] != 0)
            SetStatusDamaged();
        else SetStatusDestroyed();
    }

    WorldPacket data(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 20);
    data << GetGUID();
    data << mcaster->GetGUID().asPacked();
    data << (pcaster ? pcaster->GetGUID() : mcaster->GetGUID()).asPacked();
    data << uint32(amount);
    data << spellid;
    mcaster->SendMessageToSet(&data, (mcaster->IsPlayer() ? true : false));
    if(IntactHealth != 0 && DamagedHealth != 0)
        SetAnimProgress(m_Go_Uint32Values[GO_UINT32_HEALTH]*255/(IntactHealth + DamagedHealth));
}

void GameObject::SetStatusRebuilt()
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED | GO_FLAG_DESTROYED);
    SetDisplayId(pInfo->DisplayID);
    uint32 IntactHealth = pInfo->data.building.intactNumHits;
    uint32 DamagedHealth = pInfo->data.building.damagedNumHits;
    m_Go_Uint32Values[GO_UINT32_HEALTH] = IntactHealth + DamagedHealth;
}

void GameObject::AuraGenSearchTarget()
{
    if(m_event_MapId != m_mapId)
    {
        event_Relocate();
        return;
    }

    if(!IsInWorld() || m_deleted || !spell)
        return;

    WorldObject::InRangeSet::iterator itr;
    for( itr = GetInRangeUnitSetBegin(); itr != GetInRangeUnitSetEnd(); itr++)
    {
        Unit *unit = GetInRangeObject<Unit>(*itr);
        if (GetDistanceSq(unit) > pInfo->data.auraGenerator.radius)
            continue;
        if(unit->HasAura(spell->Id))
            continue;
        unit->AddAura(new Aura(spell, unit, unit));
    }
}

void GameObject::SetStatusDamaged()
{
    SetFlags(GO_FLAG_DAMAGED);
    if(pInfo->data.building.destructibleData != 0)
    {
        if(DestructibleModelDataEntry *display = NULL)//dbcDestructibleModelDataEntry.LookupEntry( pInfo->DestructableBuilding.DestructibleData ))
            SetDisplayId(display->GetDisplayId(1));
    } else SetDisplayId(pInfo->data.building.damagedDisplayId);
}

void GameObject::SetStatusDestroyed()
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_DAMAGED);
    SetFlags(GO_FLAG_DESTROYED);
    if(pInfo->data.building.destructibleData != 0)
    {
        if(DestructibleModelDataEntry *display = NULL)//dbcDestructibleModelDataEntry.LookupEntry( pInfo->DestructableBuilding.DestructibleData ))
            SetDisplayId(display->GetDisplayId(3));
    } else SetDisplayId(pInfo->data.building.destroyedDisplayId);
}

#define OPEN_CHEST 11437

void GameObject::Use(Player *p)
{
    SpellEntry *spellInfo = NULL;
    GameObjectInfo *goinfo = GetInfo();
    if (!goinfo)
        return;

    uint32 type = GetType();
    TRIGGER_GO_EVENT(this, OnActivate);
    TRIGGER_INSTANCE_EVENT( p->GetMapInstance(), OnGameObjectActivate )( this, p );

    switch (type)
    {
    case GAMEOBJECT_TYPE_CHAIR:
        {
            if(goinfo->data.chair.onlyCreatorUse)
            {
                if(p->GetGUID() != GetUInt64Value(GAMEOBJECT_FIELD_CREATED_BY))
                    return;
            }

            if( p->IsMounted() )
                p->RemoveAura( p->m_MountSpellId );

            if (!ChairListSlots.size())
            {
                if (goinfo->data.chair.slots > 0)
                {
                    for (uint32 i = 0; i < goinfo->data.chair.slots; ++i)
                        ChairListSlots[i] = 0;
                } else ChairListSlots[0] = 0;
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
                float relativeDistance = (size*itr->first)-(size*(goinfo->data.chair.slots-1)/2.0f);

                float x_i = GetPositionX() + relativeDistance * cos(orthogonalOrientation);
                float y_i = GetPositionY() + relativeDistance * sin(orthogonalOrientation);

                if (itr->second)
                {
                    if (Player* ChairUser = objmgr.GetPlayer(itr->second))
                    {
                        if (ChairUser->IsSitting() && sqrt(ChairUser->GetDistance2dSq(x_i, y_i)) < 0.1f)
                            continue;
                    }
                    itr->second = 0;
                }

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
                    p->Teleport( x_lowest, y_lowest, GetPositionZ(), GetOrientation() );
                    p->SetStandState(STANDSTATE_SIT_LOW_CHAIR+goinfo->data.chair.height);
                    return;
                }
            }
        }break;
    case GAMEOBJECT_TYPE_CHEST://cast da spell
        {
            spellInfo = dbcSpell.LookupEntry( OPEN_CHEST );
            SpellCastTargets targets(GetGUID());
            if(Spell *spell = new Spell(p, spellInfo))
                spell->prepare(&targets, true);
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
                /*if( p->m_stealth )
                    p->RemoveAura( p->m_stealth );*/

                if( p->m_MountSpellId )
                    p->RemoveAura( p->m_MountSpellId );

                if(!p->m_bgFlagIneligible)
                    p->m_bg->HookFlagStand(p, this);
                TRIGGER_INSTANCE_EVENT( p->GetMapInstance(), OnPlayerFlagStand )( p, this );
            }
            else
                sLog.outError("Gameobject Type FlagStand activated while the player is not in a battleground, entry %u", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_FLAGDROP:
        {
            // Dropped flag
            if(p->m_bg)
            {
                /*if( p->m_stealth )
                    p->RemoveAura( p->m_stealth );*/

                if( p->m_MountSpellId )
                    p->RemoveAura( p->m_MountSpellId );

                p->m_bg->HookFlagDrop(p, this);
                TRIGGER_INSTANCE_EVENT( p->GetMapInstance(), OnPlayerFlagDrop )( p, this );
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

            SpellCastTargets targets(p->GetGUID());
            if(Spell* spell = new Spell(p, info))
                spell->prepare(&targets, false);
            if(charges > 0 && !--charges)
                ExpireAndDelete();
        }break;
    case GAMEOBJECT_TYPE_RITUAL:
        {
            // store the members in the ritual, cast sacrifice spell, and summon.
            uint32 i = 0, reqParticipants = goinfo->data.ritual.reqParticipants;
            if(!m_ritualmembers || !GetGOui32Value(GO_UINT32_RIT_SPELL) || !GetGOui32Value(GO_UINT32_M_RIT_CASTER))
                return;

            for(i = 0; i < reqParticipants; i++)
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

            if(i == reqParticipants - 1)
            {
                SetGOui32Value(GO_UINT32_RIT_SPELL, 0);
                Player* plr;
                for(i = 0; i < reqParticipants; i++)
                {
                    plr = p->GetMapInstance()->GetPlayer(m_ritualmembers[i]);
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

                        if((info = dbcSpell.LookupEntry(goinfo->GetSpellID())) == NULL)
                            break;
                        Player* target = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_TARGET));
                        if(target == NULL)
                            return;

                        SpellCastTargets targets(target->GetGUID());
                        if(Spell *spell = new Spell(this, info))
                            spell->prepare(&targets, true);
                    }break;
                case 177193:// doom portal
                    {
                        // kill the sacrifice player
                        Player* psacrifice = p->GetMapInstance()->GetPlayer(m_ritualmembers[(int)(RandomUInt(reqParticipants-1))]);
                        Player* pCaster = GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!psacrifice || !pCaster)
                            return;
                        if((info = dbcSpell.LookupEntry(goinfo->data.ritual.casterTargetSpell)) == NULL)
                            break;

                        SpellCastTargets targets(psacrifice->GetGUID());
                        if(Spell *spell = new Spell(psacrifice, info))
                            spell->prepare(&targets, true);

                        // summons demon
                        targets.m_unitTarget = pCaster->GetGUID();
                        if(info = dbcSpell.LookupEntry(goinfo->data.ritual.spellId))
                            if(Spell *spell = new Spell(pCaster, info))
                                spell->prepare(&targets, true);
                    }break;
                case 179944:// Summoning portal for meeting stones
                    {
                        Player* plr = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_TARGET));
                        if(!plr)
                            return;

                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(plr->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);

                        /* expire the GameObject* */
                        ExpireAndDelete();
                    }break;
                case 194108:// Ritual of Summoning portal for warlocks
                    {
                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(pleader->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);

                        ExpireAndDelete();
                        pleader->InterruptCurrentSpell();
                    }break;
                case 186811://Ritual of Refreshment
                case 193062:
                    {
                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(pleader->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);

                        ExpireAndDelete();
                        pleader->InterruptCurrentSpell();
                    }break;
                case 181622://Ritual of Souls
                case 193168:
                    {
                        Player* pleader = p->GetMapInstance()->GetPlayer(GetGOui32Value(GO_UINT32_M_RIT_CASTER));
                        if(!pleader)
                            return;

                        info = dbcSpell.LookupEntry(goinfo->GetSpellID());
                        SpellCastTargets targets(pleader->GetGUID());
                        if(Spell* spell = new Spell(pleader, info))
                            spell->prepare(&targets, true);
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
            if(uint32 cinematic = goinfo->data.camera.cinematicId)
            {
                WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
                data << uint32(cinematic);
                p->GetSession()->SendPacket(&data);
            }
            else
                sLog.outError("Gameobject Type Camera doesn't have a cinematic to play id, entry %u", goinfo->ID);
        }break;
    case GAMEOBJECT_TYPE_MEETINGSTONE:  // Meeting Stone
        {
            /* Use selection */
            Player* pPlayer = objmgr.GetPlayer(p->GetSelection());
            if(!pPlayer || p->GetGroup() != pPlayer->GetGroup() || !p->GetGroup())
                return;

            GameObjectInfo * info = GameObjectNameStorage.LookupEntry(179944);
            if(!info)
                return;

            /* Create the summoning portal */
            GameObject* pGo = p->GetMapInstance()->CreateGameObject(179944);
            if( pGo == NULL || !pGo->CreateFromProto(179944, p->GetMapId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 0.0f))
                return;

            // dont allow to spam them
            GameObject* gobj = castPtr<GameObject>(p->GetMapInstance()->GetObjectClosestToCoords(179944, p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 999999.0f, TYPEID_GAMEOBJECT));
            if( gobj )
                ExpireAndDelete();

            pGo->SetGOui32Value(GO_UINT32_M_RIT_CASTER, p->GetLowGUID());
            pGo->SetGOui32Value(GO_UINT32_M_RIT_TARGET, pPlayer->GetLowGUID());
            pGo->SetGOui32Value(GO_UINT32_RIT_SPELL, 61994);
            pGo->PushToWorld(p->GetMapInstance());

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

            WorldPacket data(SMSG_ENABLE_BARBER_SHOP, 0);
            p->GetSession()->SendPacket(&data);
        }break;
    }
}