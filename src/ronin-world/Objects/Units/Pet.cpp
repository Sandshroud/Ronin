/***
 * Demonstrike Core
 */

#include "StdAfx.h"

#define GROWL_RANK_1 2649
#define GROWL_RANK_2 14916
#define WATER_ELEMENTAL 510

uint32 GetAutoCastTypeForSpell(SpellEntry * ent)
{
    switch(ent->NameHash)
    {
    /************************************************************************/
    /* Warlock Pet Spells                                                   */
    /************************************************************************/

    case SPELL_HASH_FIREBOLT:           // Firebolt
    case SPELL_HASH_LASH_OF_PAIN:       // Lash of Pain
    case SPELL_HASH_TORMENT:            // Torment
    case SPELL_HASH_SUFFERING:
    case SPELL_HASH_SOOTHING_KISS:
    case SPELL_HASH_SEDUCTION:
    case SPELL_HASH_CLEAVE:
    case SPELL_HASH_INTERCEPT:
    case SPELL_HASH_DEVOUR_MAGIC:
    case SPELL_HASH_SPELL_LOCK:
        return AUTOCAST_EVENT_ATTACK;
        break;

    case SPELL_HASH_BLOOD_PACT:         // Blood Pact
    case SPELL_HASH_AVOIDANCE:
    case SPELL_HASH_PARANOIA:
        return AUTOCAST_EVENT_ON_SPAWN;
        break;

    case SPELL_HASH_FIRE_SHIELD:        // Fire Shield
        return AUTOCAST_EVENT_OWNER_ATTACKED;
        break;

    case SPELL_HASH_PHASE_SHIFT:        // Phase Shift
    case SPELL_HASH_CONSUME_SHADOWS:
    case SPELL_HASH_LESSER_INVISIBILITY:
        return AUTOCAST_EVENT_LEAVE_COMBAT;
        break;

    /************************************************************************/
    /* Hunter Pet Spells                                                    */
    /************************************************************************/

    case SPELL_HASH_BITE:               // Bite
    case SPELL_HASH_CHARGE:             // Charge
    case SPELL_HASH_CLAW:               // Claw
    case SPELL_HASH_COWER:              // Cower
    case SPELL_HASH_DASH:               // Dash
    case SPELL_HASH_DIVE:               // Dive
    case SPELL_HASH_FIRE_BREATH:        // Fire Breath
    case SPELL_HASH_FURIOUS_HOWL:       // Furious Howl
    case SPELL_HASH_GORE:               // Gore
    case SPELL_HASH_GROWL:              // Growl
    case SPELL_HASH_LIGHTNING_BREATH:   // Lightning Breath
    case SPELL_HASH_POISON_SPIT:        // Poison Spit
    case SPELL_HASH_PROWL:              // Prowl
    case SPELL_HASH_SCORPID_POISON:     // Scorpid Poison
    case SPELL_HASH_SCREECH:            // Screech
    case SPELL_HASH_SHELL_SHIELD:       // Shell Shield
    case SPELL_HASH_THUNDERSTOMP:       // Thunderstomp
    case SPELL_HASH_WARP:               // Warp
        return AUTOCAST_EVENT_ATTACK;
        break;

    /************************************************************************/
    /* Mage Pet Spells                                                      */
    /************************************************************************/

    case SPELL_HASH_WATERBOLT:          // Waterbolt
        return AUTOCAST_EVENT_ATTACK;
        break;
    }

    return AUTOCAST_EVENT_NONE;
}

Pet::Pet(CreatureData *ctrData, uint64 guid) : Creature(ctrData, guid)
{
    m_PetXP = 0;
    Summon = false;
    memset(&ActionBar, 0, sizeof(uint32)*10);

    m_AutoCombatSpell = 0;

    m_PartySpellsUpdateTimer = 0;
    m_HappinessTimer = PET_HAPPINESS_UPDATE_TIMER;
    m_PetNumber = 0;

    m_State = PET_STATE_DEFENSIVE;
    m_Action = PET_ACTION_FOLLOW;
    m_ExpireTime = 0;
    bExpires = false;
    m_Diet = 0;
    m_Action = PET_ACTION_FOLLOW;
    m_State = PET_STATE_DEFENSIVE;
    m_dismissed = false;

    for( uint8 i = 0; i < AUTOCAST_EVENT_COUNT; i++ )
        m_autoCastSpells[i].clear();

    m_AISpellStore.clear();
    mSpells.clear();
    m_talents.clear();
}

Pet::~Pet()
{

}

void Pet::Init()
{
    Creature::Init();
    SetUInt32Value(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER + POWER_TYPE_FOCUS, 20);
}

void Pet::Destruct()
{
    mSpells.clear();

    for(std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.begin(); itr != m_AISpellStore.end(); itr++)
        delete itr->second;
    m_AISpellStore.clear();

    if(IsInWorld())
        Remove(false, true, true);

    if( m_Owner )
    {
        m_Owner->SetSummon(NULL);
        ClearPetOwner();
    }

    //TODO: clean this up in _SavePet ->  m_talents.clear();
    //add spent talents to player pet struct
    //drop unused fields from the db
    //m_talents.clear();
    Creature::Destruct();
}

void Pet::CreateAsSummon(Creature* created_from_creature, Unit* owner, LocationVector* Position, SpellEntry* created_by_spell, uint32 type, uint32 expiretime)
{
    Load(owner->GetMapId(), Position ? Position->x : owner->GetPositionX(), Position ? Position->y : owner->GetPositionY(), Position ? Position->z : owner->GetPositionZ(), Position ? Position->o : owner->GetOrientation(), 0);

    EventModelChange();
    m_Owner = castPtr<Player>(owner);
    m_OwnerGuid = owner->GetGUID();
    if( myFamily == NULL || myFamily->name == NULL )
        m_name = "Pet";
    else m_name.assign( myFamily->name );

    // Fields common to both lock summons and pets
    uint32 level = (m_Owner->GetUInt32Value( UNIT_FIELD_LEVEL ) + (m_Owner->getClass() == HUNTER ? - 5 : 0)) < 1 ?
        1 : (m_Owner->GetUInt32Value( UNIT_FIELD_LEVEL ) + (m_Owner->getClass() == HUNTER ? - 5 : 0));

    if( type & 0x2 && created_from_creature != NULL && created_from_creature->getLevel() > level)
        level = created_from_creature->getLevel();

    SetUInt32Value(UNIT_FIELD_LEVEL, level);
    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
    SetUInt64Value(UNIT_FIELD_CREATEDBY, owner->GetGUID());

    if(type & 0x1 && created_by_spell != NULL)
        SetUInt32Value(UNIT_CREATED_BY_SPELL, created_by_spell->Id);

    if(created_from_creature == NULL)
        Summon = true;
    SetUInt32Value(UNIT_FIELD_BYTES_0, 2048 | (0 << 24));
    SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
    SetUInt32Value(UNIT_FIELD_BYTES_2, (0x01 | (0x2 << 24)));
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    SetUInt32Value(UNIT_FIELD_PETNUMBER, GetLowGUID());
    if(GetEntry() == WATER_ELEMENTAL)
        m_name = "Water Elemental";
    else if(GetEntry() == 19668)
        m_name = "Shadowfiend";
    else if(GetEntry() == 30230)
        m_name = "Risen Ally";
    else m_name = sWorld.GenerateName();

    if(type & 0x2)
    {
        // These need to be checked.
        SetHappiness(PET_HAPPINESS_UPDATE_VALUE >> 1);//happiness
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, GetNextLevelXP(getLevel()));

        // 0x3 -> Enable pet rename.
        SetUInt32Value(UNIT_FIELD_BYTES_2, 1 | (0x3 << 16));

        // Change the power type to FOCUS
        SetPowerType(POWER_TYPE_FOCUS);

        InitTalentsForLevel(true);

        m_PetNumber = castPtr<Player>(owner)->GeneratePetNumber();
    }

    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, owner->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));
    m_PartySpellsUpdateTimer = 0;

    m_ExpireTime = expiretime;
    bExpires = m_ExpireTime > 0 ? true : false;

    if(owner->IsPlayer())
    {
        if(castPtr<Player>(owner)->IsPvPFlagged())
            SetPvPFlag();

        if(!bExpires)
        {
            // Create PlayerPet struct (Rest done by UpdatePetInfo)
            PlayerPet *pp = new PlayerPet;
            pp->number = m_PetNumber;
            pp->stablestate = STABLE_STATE_ACTIVE;
            castPtr<Player>(owner)->AddPlayerPet(pp, pp->number);
        }
    }

    InitializeMe();
}

void Pet::Update(uint32 time)
{
    if(m_Owner == NULL)
        return;

    Creature::Update(time); // passthrough

    if(!bExpires)
    {
        //Happiness
        if(m_HappinessTimer == 0)
        {
            int32 val = GetHappiness();
            //amount of burned happiness is loyalty_lvl depended
            int32 burn = 1042;
            if( CombatStatus.IsInCombat() )
                burn = burn >> 1; //in combat reduce burn by half (guessed)
            if((val - burn)<0)
                val = 0;
            else
                val -= burn;
            SetHappiness(val);// Set the value
            m_HappinessTimer = PET_HAPPINESS_UPDATE_TIMER;// reset timer
        }
        else
        {
            if( time > m_HappinessTimer )
                m_HappinessTimer = 0;
            else
                m_HappinessTimer -= time;
        }
    }

    if(bExpires)
    {
        if(m_ExpireTime == 0)
        {
            // remove
            Dismiss(false);
            return;
        } else {
            if(time > m_ExpireTime)
            {
                m_ExpireTime = 0;
            } else {
                m_ExpireTime -= time;
            }
        }
    }
}

void Pet::SendSpellsToOwner()
{
    if(m_Owner == NULL)
        return;

    WorldPacket data(SMSG_PET_SPELLS, 60);
    data << GetGUID();
    if(myFamily != NULL)
        data << uint16(myFamily->ID);
    else
        data << uint16(0);
    data << uint32(0);//unk2
    data << uint8(m_State) << uint8(m_Action) << uint16(0);//unk3

    // Send the actionbar
    for(uint32 i = 0; i < 10; i++)
    {
        if(ActionBar[i] & 0x4000000)        // Command
            data << uint32(ActionBar[i]);
        else
        {
            if(uint16(ActionBar[i]))
                data << uint32(ACTION_PACK_BUTTION(ActionBar[i], GetSpellState(ActionBar[i])));
            else
                data << uint32(ACTION_PACK_BUTTION(0, 0x81));
        }
    }

    // Send the rest of the spells.
    data << uint8(mSpells.size());
    for(PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
        data << uint32(ACTION_PACK_BUTTION(itr->first->Id, itr->second));

    // Send our cooldowns
    data << uint8(mSpells.size());
    for(PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); ++itr)
        data << uint32(itr->first->Id) << uint16(0) << uint32(0) << uint32(0);

    m_Owner->SendPacket(&data);
}

void Pet::SendNULLsToOwner()
{
    if(m_Owner == NULL)
        return;

    WorldPacket data(SMSG_PET_SPELLS, 8);
    data << uint64(0);
    m_Owner->GetSession()->SendPacket(&data);
}

void Pet::InitializeSpells()
{
    for(PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); itr++)
    {
        SpellEntry *info = itr->first;
        if(!info)
            continue;

        // Check that the spell isn't passive
        if( info->isPassiveSpell() )
        {
            // Cast on self..
            SpellCastTargets targets(GetGUID());
            if(Spell* sp = new Spell(this, info))
                sp->prepare(&targets, true);
            continue;
        }

        AI_Spell*sp = CreateAISpell(info);
        SetAutoCast(sp, itr->second == AUTOCAST_SPELL_STATE);
    }
}

AI_Spell*Pet::CreateAISpell(SpellEntry * info)
{
    // Create an AI_Spell
    std::map<uint32,AI_Spell*>::iterator itr = m_AISpellStore.find( info->Id );
    if( itr != m_AISpellStore.end() )
        return itr->second;

    AI_Spell *sp = new AI_Spell();
    sp->info = info;
    sp->TargetType = TargetGen_Current;

    sp->casttime = GetDBCCastTime(dbcSpellCastTime.LookupEntry( info->CastingTimeIndex ));

    sp->maxdist2cast = GetDBCMaxRange( dbcSpellRange.LookupEntry( info->rangeIndex ) );
    sp->mindist2cast = GetDBCMinRange( dbcSpellRange.LookupEntry( info->rangeIndex ) );

    sp->cooldown = objmgr.GetPetSpellCooldown(info->Id);
    if(sp->cooldown == 0) sp->cooldown = info->StartRecoveryTime; //avoid spell spamming
    if(sp->cooldown == 0) sp->cooldown = info->StartRecoveryCategory; //still 0 ?
    if(sp->cooldown == 0) sp->cooldown = 2000; // avoid spell spam
    m_AISpellStore[info->Id] = sp;
    return sp;
}

void Pet::LoadFromDB(Player* owner, PlayerPet * playerPetInfo)
{
    m_Owner = owner;
    m_OwnerGuid = m_Owner->GetGUID();
    m_PlayerPetInfo = playerPetInfo;

    myFamily = dbcCreatureFamily.LookupEntry(GetCreatureData()->family);
    Load(owner->GetMapId(), owner->GetPositionX() + 2.f, owner->GetPositionY() + 2.f, owner->GetPositionZ(), owner->GetOrientation(), 0);

    //LoadValues(m_PlayerPetInfo->fields.c_str());
    if(getLevel() == 0)
    {
        m_PlayerPetInfo->level = m_Owner->getLevel();
        SetUInt32Value(UNIT_FIELD_LEVEL, m_PlayerPetInfo->level);
    }
    ApplyStatsForLevel();

    //SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, owner->GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE));

    m_PetNumber = m_PlayerPetInfo->number;
    m_PetXP = m_PlayerPetInfo->xp;
    m_name = m_PlayerPetInfo->name;
    Summon = m_PlayerPetInfo->summon;

    m_HappinessTimer = m_PlayerPetInfo->happinessupdate;

    bExpires = false;

    if(m_Owner && getLevel() > m_Owner->getLevel())
    {
        SetUInt32Value(UNIT_FIELD_LEVEL, m_Owner->getLevel());
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, GetNextLevelXP(m_Owner->getLevel()));
    }

    ActionBar[0] = PET_SPELL_ATTACK;
    ActionBar[1] = PET_SPELL_FOLLOW;
    ActionBar[2] = PET_SPELL_STAY;
    ActionBar[7] = PET_SPELL_AGRESSIVE;
    ActionBar[8] = PET_SPELL_DEFENSIVE;
    ActionBar[9] = PET_SPELL_PASSIVE;
    for(uint8 i = 3, i2 = 0; i < 7, i2 < 4; i++, i2++)
    {
        ActionBar[i] = m_PlayerPetInfo->actionbarspell[i2];
        if(!(ActionBar[i] & 0x4000000) && m_PlayerPetInfo->actionbarspell[i2])
        {
            SpellEntry * sp = dbcSpell.LookupEntry(m_PlayerPetInfo->actionbarspell[i2]);
            if(!sp)
            {
                ActionBar[i] = m_PlayerPetInfo->actionbarspell[i2] = 0;
                continue;
            }
            mSpells[sp] = m_PlayerPetInfo->actionbarspellstate[i2];
        }
    }

    InitializeMe();
}

void Pet::InitializeMe()
{
    if( m_Owner->GetSummon() != NULL )
    {
        // 2 pets???!
//      m_Owner->GetSummon()->Remove(true, true, true);
//      m_Owner->SetSummon( castPtr<Pet>(this) );
    } else m_Owner->SetSummon( castPtr<Pet>(this) );

    // set up ai and shit
    GetAIInterface()->Init(castPtr<Unit>(this) ,AITYPE_PET,MOVEMENTTYPE_NONE,m_Owner);
    GetAIInterface()->SetUnitToFollow(m_Owner);
    GetAIInterface()->SetFollowDistance(3.0f);

    m_Owner->SetUInt64Value(UNIT_FIELD_SUMMON, GetGUID());
    SetUInt32Value(UNIT_FIELD_PETNUMBER, GetLowGUID());
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, (uint32)UNIXTIME);
    myFamily = dbcCreatureFamily.LookupEntry(GetCreatureData()->family);
    SetPetDiet();
    _setFaction();
    m_State = 1;        // dont set agro on spawn

    EventModelChange();

    if(GetEntry() == 416)
        m_aiInterface.disable_melee = true;

    ApplyStatsForLevel();

    if(Summon == false)
    {
        // Pull from database... :/
        QueryResult * query = CharacterDatabase.Query("SELECT * FROM pet_spells WHERE ownerguid=%u AND petnumber=%u", m_Owner->GetLowGUID(), m_PetNumber);
        if(query)
        {
            do
            {
                Field * f = query->Fetch();
                SpellEntry* spell = dbcSpell.LookupEntry(f[2].GetUInt32());
                uint16 flags = f[3].GetUInt16();
                if(spell != NULL && mSpells.find(spell) == mSpells.end())
                    mSpells.insert ( std::make_pair( spell, flags ) );

            } while(query->NextRow());
            delete query;
        }

        // Pull from database... :/
        uint8 spentPoints = 0;
        QueryResult * query2 = CharacterDatabase.Query("SELECT * FROM pet_talents WHERE ownerguid=%u AND petnumber=%u",m_Owner->GetLowGUID(), m_PetNumber);
        if(query2)
        {
            do
            {
                Field * f = query2->Fetch();
                uint32 talentid = f[2].GetUInt32();
                uint8 rank = f[3].GetUInt8();
                if(rank)
                {
                    spentPoints += rank;
                }
                else
                    spentPoints++;
                if(m_talents.find(talentid) == m_talents.end())
                {
                    m_talents.insert( std::make_pair( talentid, rank ) );
                }

            } while(query2->NextRow());
            delete query2;
        }

        if(spentPoints > GetSpentPetTalentPoints())
            ResetTalents(false);
    }

    InitializeSpells();
    PushToWorld(m_Owner->GetMapMgr());

    // Set up default actionbar
    SetDefaultActionbar();

    SendSpellsToOwner();

    // set to active
    if(!bExpires)
        UpdatePetInfo(false);

    sEventMgr.AddEvent(castPtr<Pet>(this), &Pet::HandleAutoCastEvent, uint32(AUTOCAST_EVENT_ON_SPAWN), EVENT_UNK, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    sEventMgr.AddEvent(castPtr<Pet>(this), &Pet::HandleAutoCastEvent, uint32(AUTOCAST_EVENT_LEAVE_COMBAT), EVENT_UNK, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Pet::UpdatePetInfo(bool bSetToOffline)
{
    if(bExpires || m_Owner==NULL)
        return; // don't update expiring pets

    PlayerPet *pi = m_Owner->GetPlayerPet(m_PetNumber);
    pi->active = !bSetToOffline;
    pi->entry = GetEntry();
    std::stringstream ss;
    for( uint32 index = 0; index < UNIT_END; index ++ )
    {
        ss << GetUInt32Value(index) << " ";
    }
    pi->fields = ss.str();
    pi->name = GetName();
    pi->number = m_PetNumber;
    pi->xp = m_PetXP;
    pi->level = GetUInt32Value(UNIT_FIELD_LEVEL);
    pi->happiness = GetHappiness();
    pi->happinessupdate = m_HappinessTimer;

    // save action bar
    for(uint8 i = 3, i2 = 0; i < 7, i2 < 4; i++, i2++)
    {
        if(ActionBar[i] & 0x4000000) //command
        {
            pi->actionbarspell[i2] = ActionBar[i];
            pi->actionbarspellstate[i2] = 0;
        }
        else if(ActionBar[i]) //spell
        {
            pi->actionbarspell[i2] = ActionBar[i];
            pi->actionbarspellstate[i2] = uint32(GetSpellState(ActionBar[i]));
        }
        else //empty
        {
            pi->actionbarspell[i2] = 0;
            pi->actionbarspellstate[i2] = DEFAULT_SPELL_STATE;
        }
    }

    pi->summon = Summon;
}

void Pet::Dismiss(bool bSafeDelete)//Abandon pet
{
    // already deleted
    if( m_dismissed )
        return;

    // Delete any petspells for us.
    if( !bExpires )
    {
        if(!Summon && m_Owner)
        {
            CharacterDatabase.Execute("DELETE FROM pet_spells WHERE ownerguid=%u AND petnumber=%u", m_Owner->GetLowGUID(), m_PetNumber);
            CharacterDatabase.Execute("DELETE FROM pet_talents WHERE ownerguid=%u AND petnumber=%u", m_Owner->GetLowGUID(), m_PetNumber);
        }

        if(m_Owner != NULL)
            m_Owner->RemovePlayerPet( m_PetNumber );
    }

    // find out playerpet entry, delete it
    Remove(bSafeDelete, false, true);
}

void Pet::Remove(bool bSafeDelete, bool bUpdate, bool bSetOffline)
{
    if( m_dismissed )
        return;

    m_AuraInterface.RemoveAllAuras(); // Prevent pet overbuffing
    if(m_Owner)
    {
        // remove association with player
        m_Owner->SetUInt64Value(UNIT_FIELD_SUMMON, 0);
        if(IsSummon())
            m_Owner->RemoveSummon(this);

        if(bUpdate)
        {
            if(!bExpires)
                UpdatePetInfo(bSetOffline);
            if(!IsSummonedPet() && !bExpires)
                m_Owner->_SavePet(NULL);//not perfect but working
        }
        m_Owner->SetSummon(NULL);
        SendNULLsToOwner();
    }
    ClearPetOwner();

    // has to be next loop - reason because of RemoveFromWorld, iterator gets broke.
    /*if(IsInWorld() && Active) Deactivate(m_mapMgr);*/
    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(castPtr<Pet>(this), &Pet::PetSafeDelete, EVENT_CREATURE_SAFE_DELETE, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    m_dismissed = true;
}

void Pet::PetSafeDelete()
{
    if(IsInWorld())
    {
        // remove from world, and delete
        RemoveFromWorld(false, true);
    }

    //sEventMgr.AddEvent(World::getSingletonPtr(), &World::DeleteObject, this, EVENT_CREATURE_SAFE_DELETE, 1000, 1);
    Creature::SafeDelete();
}

void Pet::DelayedRemove(bool bTime, bool bDeath)
{
    if(GetMapMgr()!= NULL)
        m_Owner = GetMapMgr()->GetPlayer((uint32)m_OwnerGuid);

    if(bTime)
    {
        if(GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0 || bDeath)
            Dismiss(true);  // remove us..
        else
            Remove(true, true, true);
    }
    else
        sEventMgr.AddEvent(castPtr<Pet>(this), &Pet::DelayedRemove, true, bDeath, EVENT_PET_DELAYED_REMOVE, PET_DELAYED_REMOVAL_TIME, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Pet::GiveXP( uint32 xp )
{
    if( m_Owner == NULL || Summon )
        return;

    if( getLevel() >= m_Owner->getLevel() )     //pet do not get xp if its level >= owners level
        return;

    if(m_Owner->m_XPoff)
        return;

    xp += GetUInt32Value(UNIT_FIELD_PETEXPERIENCE);
    uint32 nxp = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    bool changed = false;

    while(xp >= nxp)
    {
        ModUnsigned32Value(UNIT_FIELD_LEVEL, 1);
        xp -= nxp;
        nxp = GetNextLevelXP(GetUInt32Value(UNIT_FIELD_LEVEL));
        changed = true;
    }

    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, xp);
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, nxp);
    if(changed)
    {
        ApplyStatsForLevel();
        InitTalentsForLevel();
    }
}

uint32 Pet::GetNextLevelXP(uint32 currentlevel)
{
    // Calculate next level XP
    uint32 nextLvlXP = 0;
    uint32 Level = currentlevel + 1;

    // copy and paste :D taken from objectmgr.cpp
    // we'll just grab the xp from this table and divide it, see note below about
    // pet xp calculation
    // this is a fixed table taken from 2.3.0 wow. This can't get more blizzlike with the "if" cases ;)
    if( ( currentlevel ) < MAX_PREDEFINED_NEXTLEVELXP )
    {
        nextLvlXP = NextLevelXp[( currentlevel )];
    }
    else
    {
        // 2.2
        //float MXP = 45 + ( 5 * level );
        // 2.3
        float MXP = 235 + ( 5 * Level );
        float DIFF = Level < 29 ? 0.0 : Level < 30 ? 1.0 : Level < 31 ? 3.0 : Level < 32 ? 6.0 : 5.0 * ( float( Level ) - 30.0 );
        float XP = ( ( 8.0 * float( Level ) ) + DIFF ) * MXP;
        nextLvlXP = (int)( ( XP / 100.0 ) + 0.5 ) * 100;
    }
    // Source: http://www.wow-petopia.com/ Previously, pets needed 1/6th (or about 16%) of the experience
    // that a character of the same level would need.
    // Now they only need 1/10th (or about 10%) -- which is a 66% improvement!
    float xp = float(nextLvlXP) / 10.f;
    return float2int32(xp);
}

void Pet::LevelUpTo(uint32 level)
{
    uint32 currentlevel = getLevel();
    uint32 XPtoGive = 0;
    while(level > currentlevel)
    {
        XPtoGive += NextLevelXp[currentlevel];
        currentlevel++;
    }
    GiveXP(XPtoGive);

}

void Pet::AddSpell(SpellEntry * sp, bool learning, bool sendspells)
{
    // Cast on self if we're a passive spell
    if( sp->isPassiveSpell() )
    {
        if(IsInWorld())
        {
            SpellCastTargets targets(GetGUID());
            if(Spell* spell = new Spell(this, sp))
                spell->prepare(&targets,true);
            mSpells[sp] = 0x0100;
        }
    }
    else
    {
       // Active spell add to the actionbar.
        bool has=false;
        for(int i = 0; i < 10; i++)
        {
            if(ActionBar[i] == sp->Id)
            {
                has=true;
                break;
            }
        }

        if(!has)
        {
            for(int i = 0; i < 10; i++)
            {
                if(ActionBar[i] == 0)
                {
                    ActionBar[i] = sp->Id;
                    break;
                }
            }
        }

        bool done=false;
        if(learning)
        {
            for(PetSpellMap::iterator itr = mSpells.begin(); itr != mSpells.end(); itr++)
            {
                if(sp->NameHash == itr->first->NameHash)
                {
                    // replace the spell in the action bar
                    for(int i = 0; i < 10; i++)
                    {
                        if(ActionBar[i] == itr->first->Id)
                        {
                            ActionBar[i] = sp->Id;
                            break;
                        }
                    }

                    // Create the AI_Spell
                    AI_Spell*asp = CreateAISpell(sp);

                    // apply the spell state
                    uint16 ss = GetSpellState(itr->first);
                    mSpells[sp] = ss;
                    if(ss==AUTOCAST_SPELL_STATE)
                        SetAutoCast(asp, true);

                    if(asp->autocast_type==AUTOCAST_EVENT_ON_SPAWN)
                        CastSpell(castPtr<Unit>(this), sp, false);

                    RemoveSpell(itr->first);
                    done=true;
                    break;
                }
            }
        }

        if(done==false)
        {
            if(mSpells.find(sp) != mSpells.end())
                return;

            if(learning)
            {
                AI_Spell*asp = CreateAISpell(sp);
                uint16 ss = (asp->autocast_type > 0) ? AUTOCAST_SPELL_STATE : DEFAULT_SPELL_STATE;
                mSpells[sp] = ss;
                if(ss==AUTOCAST_SPELL_STATE)
                    SetAutoCast(asp,true);

                if(asp->autocast_type==AUTOCAST_EVENT_ON_SPAWN)
                    CastSpell(castPtr<Unit>(this), sp, false);
            } else mSpells[sp] = DEFAULT_SPELL_STATE;
        }
    }
    if(IsInWorld() && sendspells)
        SendSpellsToOwner();
}

void Pet::SetSpellState(SpellEntry* sp, uint16 State)
{
    PetSpellMap::iterator itr = mSpells.find(sp);
    if(itr == mSpells.end())
        return;

    uint16 oldstate = itr->second;
    itr->second = State;

    if(State == AUTOCAST_SPELL_STATE || oldstate == AUTOCAST_SPELL_STATE)
    {
        AI_Spell*sp2 = GetAISpellForSpellId(sp->Id);
        if(sp2)
        {
            if(State == AUTOCAST_SPELL_STATE)
                SetAutoCast(sp2, true);
            else
                SetAutoCast(sp2,false);
        }
    }
}

uint16 Pet::GetSpellState(SpellEntry* sp)
{
    PetSpellMap::iterator itr = mSpells.find(sp);
    if(itr == mSpells.end())
        return DEFAULT_SPELL_STATE;

    return itr->second;
}

void Pet::SetDefaultActionbar()
{
    // Set up the default actionbar.
    ActionBar[0] = PET_SPELL_ATTACK;
    ActionBar[1] = PET_SPELL_FOLLOW;
    ActionBar[2] = PET_SPELL_STAY;

    // Fill up 4 slots with our spells
    if(mSpells.size() > 0)
    {
        PetSpellMap::iterator itr = mSpells.begin();
        uint32 pos = 0;
        for(; itr != mSpells.end() && pos < 4; itr++, pos++)
            ActionBar[3+pos] = itr->first->Id;
    }

    ActionBar[7] = PET_SPELL_AGRESSIVE;
    ActionBar[8] = PET_SPELL_DEFENSIVE;
    ActionBar[9] = PET_SPELL_PASSIVE;
}

void Pet::WipeSpells()
{
    while( mSpells.size() > 0 )
    {
        RemoveSpell( mSpells.begin()->first );
    }
    SendSpellsToOwner();
}

void Pet::LearnSpell(uint32 spellid)
{
    SpellEntry * sp = dbcSpell.LookupEntry(spellid);
    if(!sp)
        return;

    AddSpell(sp, true);

    if(m_Owner && m_Owner->IsPlayer())
    {
        WorldPacket data(SMSG_PET_LEARNED_SPELL, 2);
        data << uint32(spellid);
        m_Owner->GetSession()->SendPacket(&data);
        SendSpellsToOwner();
    }
}

void Pet::LearnLevelupSpells()
{
    uint32 family_id = (Summon ? GetEntry() : (myFamily ? myFamily->ID : 0));
    if(PetLevelupSpellSet const *levelupSpells = objmgr.GetPetLevelupSpellList(family_id))
    {
        uint32 petlevel = getLevel();
        std::map<uint32, uint32>::const_iterator itr = levelupSpells->find(petlevel);
        if(itr != levelupSpells->end())
            LearnSpell(itr->second);
    }
}

void Pet::RemoveSpell(SpellEntry * sp)
{
    mSpells.erase(sp);
    std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.find(sp->Id);
    if(itr != m_AISpellStore.end())
    {
        if( itr->second->autocast_type != AUTOCAST_EVENT_NONE )
        {
            std::map<uint32, AI_Spell*>::iterator it3;
            for(std::map<uint32, AI_Spell*>::iterator it2 = m_autoCastSpells[itr->second->autocast_type].begin(); it2 != m_autoCastSpells[itr->second->autocast_type].end(); )
            {
                it3 = it2++;
                if( it3->second == itr->second )
                {
                    m_autoCastSpells[itr->second->autocast_type].erase(it3);
                }
            }
        }

        for(std::map<uint32, AI_Spell*>::iterator it = m_aiInterface.m_spells.begin(); it != m_aiInterface.m_spells.end(); ++it)
        {
            if(it->second == itr->second)
            {
                m_aiInterface.m_spells.erase(it);
                break;
            }
        }

        delete itr->second;
        m_AISpellStore.erase(itr);
    }
    else
    {
        for(std::map<uint32, AI_Spell*>::iterator it = m_aiInterface.m_spells.begin(); it != m_aiInterface.m_spells.end(); ++it)
        {
            if(it->second->info == sp)
            {
                // woot?
                AI_Spell * spe = it->second;
                m_aiInterface.m_spells.erase(it);
                delete spe;
                break;
            }
        }
    }

    //Remove spell from action bar as well
    for( uint32 pos = 0; pos < 10; pos++ )
    {
        if( ActionBar[pos] == sp->Id )
            ActionBar[pos] = 0;
    }
}

void Pet::Rename(std::string NewName)
{
    m_name = NewName;
    // update petinfo
    UpdatePetInfo(false);

    // update timestamp to force a re-query
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, (uint32)UNIXTIME);
}

void Pet::ApplySummonLevelAbilities()
{
    uint32 level = GetUInt32Value(UNIT_FIELD_LEVEL);

    // Determine our stat index.
    int stat_index = -1;
    //float scale = 1;
    bool has_mana = true;

    switch(GetEntry())
    {
    case 416: //Imp
        stat_index = 0;
        m_aiInterface.disable_melee = true;
        break;
    case 1860: //VoidWalker
        stat_index = 1;
        break;
    case 1863: //Succubus
        stat_index = 2;
        break;
    case 417: //Felhunter
        stat_index = 3;
        break;
    case 89:    // Infernal
        has_mana = false;
    case 11859: // Doomguard
    case 17252: // Felguard
        stat_index = 4;
        break;
    case 510:   // Mage's water elemental
        stat_index = 5;
        m_aiInterface.disable_melee = true;
        break;
    }
    if(stat_index < 0)
    {
        sLog.outDebug("PETSTAT: No stat index found for entry %u, `%s`!", GetEntry(), GetName());
        return;
    }

    static float R_base_str[6] = {18.1884058f, -15.f, -15.f, -15.f, -15.f, -15.f};
    static float R_mod_str[6] = {1.811594203f, 2.4f, 2.4f, 2.4f, 2.4f, 2.4f};
    static float R_base_agi[6] = {19.72463768f, -1.25f, -1.25f, -1.25f, -1.25f, -1.25f};
    static float R_mod_agi[6] = {0.275362319f, 1.575f, 1.575f, 1.575f, 1.575f, 1.575f};
    static float R_base_sta[6] = {17.23188406f, -17.75f, -17.75f, -17.75f, -17.75f, 0.f};
    static float R_mod_sta[6] = {2.768115942f, 4.525f, 4.525f, 4.525f, 4.525f, 4.044f};
    static float R_base_int[6] = {19.44927536f, 12.75f, 12.75f, 12.75f, 12.75f, 20.f};
    static float R_mod_int[6] = {4.550724638f, 1.875f, 1.875f, 1.875f, 1.875f, 2.8276f};
    static float R_base_spr[6] = {19.52173913f, -2.25f, -2.25f, -2.25f, -2.25f, 20.5f};
    static float R_mod_spr[6] = {3.47826087f, 1.775f, 1.775f, 1.775f, 1.775f, 3.5f};
    static float R_base_pwr[6] = {7.202898551f, -101.f, -101.f, -101.f, -101.f, -101.f};
    static float R_mod_pwr[6] = {2.797101449f, 6.5f, 6.5f, 6.5f, 6.5f, 6.5f};
    static float R_base_armor[6] = {-11.69565217f, -702.f, -929.4f, -1841.25f, -1157.55f, -1000.f};
    static float R_mod_armor[6] = {31.69565217f, 139.6f, 74.62f, 89.175f, 101.1316667f, 100.f};
    static float R_pet_sta_to_hp[6] = {6.405982906f, 15.91304348f, 7.956521739f, 10.79813665f, 11.55590062f, 10.0f};
    static float R_base_min_dmg[6] = {0.550724638f, 4.566666667f, 26.82f, 29.15f, 20.17888889f, 20.f};
    static float R_mod_min_dmg[6] = {1.449275362f, 1.433333333f, 2.18f, 1.85f, 1.821111111f, 1.f};
    static float R_base_max_dmg[6] = {1.028985507f, 7.133333333f, 36.16f, 39.6f, 27.63111111f, 20.f};
    static float R_mod_max_dmg[6] = {1.971014493f, 1.866666667f, 2.84f, 2.4f, 2.368888889f, 1.1f};

    float base_str = R_base_str[stat_index];
    float mod_str = R_mod_str[stat_index];
    float base_agi = R_base_agi[stat_index];
    float mod_agi = R_mod_agi[stat_index];
    float base_sta = R_base_sta[stat_index];
    float mod_sta = R_mod_sta[stat_index];
    float base_int = R_base_int[stat_index];
    float mod_int = R_mod_int[stat_index];
    float base_spr = R_base_spr[stat_index];
    float mod_spr = R_mod_spr[stat_index];
    float base_pwr = R_base_pwr[stat_index];
    float mod_pwr = R_mod_pwr[stat_index];
    float base_armor = R_base_armor[stat_index];
    float mod_armor = R_mod_armor[stat_index];
    float base_min_dmg = R_base_min_dmg[stat_index];
    float mod_min_dmg = R_mod_min_dmg[stat_index];
    float base_max_dmg = R_base_max_dmg[stat_index];
    float mod_max_dmg = R_mod_max_dmg[stat_index];
    float pet_sta_to_hp = R_pet_sta_to_hp[stat_index];

    // Calculate bonuses
    float pet_sta_bonus = 0.3 * (float)m_Owner->GetBonusStat(STAT_STAMINA);      // + sta_buffs
    float pet_int_bonus = 0.3 * (float)m_Owner->GetBonusStat(STAT_INTELLECT);    // + int_buffs
    float pet_arm_bonus = 0.35 * (float)m_Owner->GetBonusResistance(RESISTANCE_ARMOR);   // + arm_buffs

    float pet_str = base_str + float(level) * mod_str;
    float pet_agi = base_agi + float(level) * mod_agi;
    float pet_sta = base_sta + float(level) * mod_sta + pet_sta_bonus;
    float pet_int = base_int + float(level) * mod_int + pet_int_bonus;
    float pet_spr = base_spr + float(level) * mod_spr;
    float pet_pwr = base_pwr + float(level) * mod_pwr;
    float pet_arm = base_armor + float(level) * mod_armor + pet_arm_bonus;

    // Calculate values
    SetUInt32Value(UNIT_FIELD_STRENGTH, float2int32(pet_str));
    SetUInt32Value(UNIT_FIELD_AGILITY, float2int32(pet_agi));
    SetUInt32Value(UNIT_FIELD_STAMINA, float2int32(pet_sta));
    SetUInt32Value(UNIT_FIELD_INTELLECT, float2int32(pet_int));
    SetUInt32Value(UNIT_FIELD_SPIRIT, float2int32(pet_spr));

    float pet_min_dmg = base_min_dmg + float(level) * mod_min_dmg;
    float pet_max_dmg = base_max_dmg + float(level) * mod_max_dmg;
    SetFloatValue(UNIT_FIELD_MINDAMAGE, pet_min_dmg);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, pet_max_dmg);

    // Apply armor and attack power.
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, float2int32(pet_pwr));
    SetUInt32Value(UNIT_FIELD_RESISTANCES, float2int32(pet_arm));

    // Calculate health / mana
    float health = pet_sta * pet_sta_to_hp;
    float mana = has_mana ? (pet_int * 15) : 0.0;
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, float2int32(health));
    SetUInt32Value(UNIT_FIELD_BASE_MANA, float2int32(mana));
}

void Pet::ApplyPetLevelAbilities()
{
    uint32 level = GetUInt32Value(UNIT_FIELD_LEVEL);
    float dlevel = (float)level;

    // All pets have identical 5% bonus. http://www.wowwiki.com/Hunter_pet
    float pet_mod_sta = 1.05, pet_mod_arm = 1.05, pet_mod_dps = 1.05;

    // As of patch 3.0 the pet gains 45% of the hunters stamina
    float pet_sta_bonus = 0.45 * (float)m_Owner->GetUInt32Value(UNIT_FIELD_STAMINA);
    float pet_arm_bonus = 0.35 * (float)m_Owner->GetBonusResistance(RESISTANCE_ARMOR);       // Armor
    float pet_ap_bonus = 0.22 * (float)m_Owner->GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER);

    //Base attributes from http://petopia.brashendeavors.net/html/art...ttributes.shtml
    static uint32 R_pet_base_armor[80] = {
        15, 16, 41, 76, 120, 174, 239, 316, 406, 512,
        538, 573, 608, 642, 677, 713, 748, 782, 817, 852,
        888, 922, 957, 992, 1026, 1061, 1097, 1130, 1165, 1200,
        1234, 1270, 1304, 1340, 1373, 1480, 1593, 1709, 1834, 1964,
        2101, 2246, 2397, 2557, 2725, 2780, 2835, 2888, 2944, 2999,
        3052, 3108, 3163, 3216, 3271, 3327, 3380, 3435, 3489, 3791,
        4091, 4391, 4691, 4991, 5291, 5591, 5892, 6192, 6492, 6792,
        7092, 7392, 7692, 7992, 8292, 8592, 8892, 9192, 9492, 9792 };

    static float R_pet_base_hp[80] = {
        42, 55, 71, 86, 102, 120, 137, 156, 176, 198,
        222, 247, 273, 300, 328, 356, 386, 417, 449, 484,
        521, 562, 605, 651, 699, 750, 800, 853, 905, 955,
        1006, 1057, 1110, 1163, 1220, 1277, 1336, 1395, 1459, 1524,
        1585, 1651, 1716, 1782, 1848, 1919, 1990, 2062, 2138, 2215,
        2292, 2371, 2453, 2533, 2614, 2699, 2784, 2871, 2961, 3052,
        3144, 3237, 3331, 3425, 3524, 3624, 3728, 3834, 3941, 4049,
        5221, 5416, 5875, 6139, 6426, 6735, 7069, 8425, 8804, 9202}; // this row is guessed :o

    // Calculate HP
    //patch from darken
    float pet_hp;
    float pet_armor;
    if(level-1<80)
    {
        pet_hp= ( ( ( R_pet_base_hp[level-1]) + ( pet_sta_bonus * 10 ) ) * pet_mod_sta);
        pet_armor= ( (R_pet_base_armor[level-1] ) * pet_mod_arm + pet_arm_bonus );
    }
    else
    {
        pet_hp  = ( ( ( 0.6 * dlevel * dlevel + 10.6 * dlevel + 33 ) + ( pet_sta_bonus * 10 ) ) * pet_mod_sta);
        pet_armor = ( ( -75 + 50 * dlevel ) * pet_mod_arm + pet_arm_bonus );
    }

    float pet_attack_power = ( ( 7.9 * ( ( dlevel * dlevel ) / ( dlevel * 3 ) ) ) + ( pet_ap_bonus ) ) * pet_mod_dps;

    if(pet_attack_power <= 0.0f) pet_attack_power = 1;
    if(pet_armor <= 0.0f) pet_armor = 1;

    // Set base values.
    SetUInt32Value(UNIT_FIELD_BASE_HEALTH, float2int32(pet_hp));
    SetUInt32Value(UNIT_FIELD_RESISTANCES, float2int32(pet_armor));
    // source www.wow-petopia.com
    // Pets now get a base resistance to all elements equal to their level.
    // For example, a level 11 pet will have a base of 11 resistance to Arcane,
    // Fire, Frost, Nature and Shadow damage.
    for(uint8 i = 1; i < 7; i++)
        SetUInt32Value(UNIT_FIELD_RESISTANCES+i, getLevel());

    // Calculate damage.
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER, float2int32(pet_attack_power));

    /* These are just for visuals, no other actual purpose.
    BaseStats[STAT_STRENGTH] = uint32(20+getLevel()*1.55);
    BaseStats[STAT_AGILITY] = uint32(20+getLevel()*0.64);
    // Reverse the health value to calculate stamina
    BaseStats[STAT_STAMINA] = float2int32(pet_hp / 10);
    BaseStats[STAT_INTELLECT] = uint32(20+getLevel()*0.18);
    BaseStats[STAT_SPIRIT] = uint32(20+getLevel()*0.36);*/

    LearnLevelupSpells();
}

void Pet::ApplyStatsForLevel()
{
    if(GetUInt32Value(UNIT_CREATED_BY_SPELL))      // Summon
        ApplySummonLevelAbilities();
    else ApplyPetLevelAbilities();

    // Apply common stuff
    // Apply scale for this family.
    if(myFamily != NULL)
    {
        float pet_level = float(getLevel());
        float level_diff = float(myFamily->maxlevel - myFamily->minlevel);
        float scale_diff = float(myFamily->maxsize - myFamily->minsize);
        float factor = scale_diff / level_diff;
        float scale = factor * pet_level + myFamily->minsize;
        SetObjectScale(scale);
    }

    SetHealth(GetUInt32Value(UNIT_FIELD_MAXHEALTH));
    SetPower(POWER_TYPE_MANA, GetMaxPower(POWER_TYPE_MANA));   // mana
    SetPower(POWER_TYPE_FOCUS, GetMaxPower(POWER_TYPE_FOCUS));   // focus
}

HappinessState Pet::GetHappinessState()
{
    //gets happiness state from happiness points
    uint32 pts = GetHappiness();
    if( pts < PET_HAPPINESS_UPDATE_VALUE )
        return UNHAPPY;
    else if( pts >= PET_HAPPINESS_UPDATE_VALUE << 1 )
        return HAPPY;
    return CONTENT;
}

void Pet::AddPetSpellToOwner(uint32 spellId)
{
    //exit if owner hasn't Beast training ability (id 5149)
    if(!m_Owner || !m_Owner->HasSpell(5149))
        return;

    //find appropriate teaching spell...
    if(uint32 TeachingSpellID = 0)//sWorld.GetTeachingSpell(spellId))
    {
        if(m_Owner->HasSpell(TeachingSpellID))
            return;
        //...and add it to pet owner to be able teach other pets
        m_Owner->addSpell(TeachingSpellID);
        return;
    } else sLog.outDebug("WORLD: Could not find teaching spell for spell %u", spellId);
}

uint32 Pet::GetHighestRankSpell(uint32 spellId)
{
    //get the highest rank of spell from known spells
    SpellEntry *sp = dbcSpell.LookupEntry(spellId);
    SpellEntry *tmp = 0;
    if(sp && mSpells.size() > 0)
    {
        PetSpellMap::iterator itr = mSpells.begin();
        for(; itr != mSpells.end(); itr++)
            if(sp->NameHash == itr->first->NameHash)
                if((!tmp || tmp->RankNumber < itr->first->RankNumber))
                     tmp = itr->first;
    }
    return tmp ? tmp->Id : 0;
}

AI_Spell * Pet::HandleAutoCastEvent()
{
    std::map<uint32, AI_Spell*>::iterator itr,itr2;
    bool chance = true;
    uint32 size = 0;

    for( itr2 = m_autoCastSpells[AUTOCAST_EVENT_ATTACK].begin(); itr2 != m_autoCastSpells[AUTOCAST_EVENT_ATTACK].end(); )
    {
        itr = itr2;
        itr2++;
        size = (uint32)m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].size();
        if( size > 1 )
            chance = Rand( 100.0f / size );

        if(itr->second->autocast_type == AUTOCAST_EVENT_ATTACK)
        {
            // spells still spammed, I think the cooldowntime is being set incorrectly somewhere else
            if( chance && itr->second->info && getMSTime() >= itr->second->cooldown && //cebernic:crashfix
                GetPower(PowerType(itr->second->info->powerType)) >= itr->second->info->ManaCost )
            {
                return itr->second;
            }
        }
        else
        {
            // bad pointers somehow end up here :S
            sLog.outDebug("Bad AI_Spell detected in AutoCastEvent!");
            m_autoCastSpells[AUTOCAST_EVENT_ATTACK].erase(itr);
        }
    }

    return NULL;
}

void Pet::HandleAutoCastEvent(uint32 Type)
{
    std::map<uint32, AI_Spell*>::iterator itr, it2;
    AI_Spell * sp;
    if( m_Owner == NULL )
        return;

    if( Type == AUTOCAST_EVENT_ATTACK )
    {
        if( m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].size() > 1 )
        {
            for( itr = m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].begin(); itr != m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].end(); itr++ )
            {
                if( itr == m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].end() )
                {
                    if( getMSTime() >= itr->second->cooldown )
                        m_aiInterface.m_CastNext = itr->second;
                    else return;
                    break;
                }
                else
                {
                    if( itr->second->cooldown > getMSTime() )
                        continue;

                    m_aiInterface.m_CastNext = itr->second;
                }
            }
        }
        else if( m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].size() )
        {
            sp = m_autoCastSpells[ AUTOCAST_EVENT_ATTACK ].begin()->second;
            if( sp->cooldown && getMSTime() < sp->cooldown )
                return;

            m_aiInterface.m_CastNext = sp;
        }

        return;
    }

    for( itr = m_autoCastSpells[Type].begin(); itr != m_autoCastSpells[Type].end(); )
    {
        it2 = itr++;
        sp = it2->second;

        if( sp->info == NULL )
        {
            sLog.outDebug("Found corrupted spell at m_autoCastSpells, skipping");
            continue;
        }
        else if( sp->autocast_type != int32(Type) )
        {
            sLog.outDebug("Found corrupted spell (%lu) at m_autoCastSpells, skipping", it2->first);
            continue;
        }

        if( sp->TargetType == TargetGen_SummonOwner )
        {
            if( !m_Owner->HasAura( sp->info->Id ) )
                CastSpell( m_Owner, sp->info, false );
        }
        else
        {
            //modified by Zack: Spell targeting will be generated in the castspell function now.You cannot force to target self all the time
            CastSpell( castPtr<Unit>(NULL), sp->info, false);
        }
    }
}

void Pet::SetAutoCast(AI_Spell*sp, bool on)
{
    if(sp->autocast_type > 0)
    {
        if(!on)
        {
            for(std::map<uint32, AI_Spell*>::iterator itr = m_autoCastSpells[sp->autocast_type].begin();
                itr != m_autoCastSpells[sp->autocast_type].end(); ++itr)
            {
                if(itr->second == sp )
                {
                    m_autoCastSpells[sp->autocast_type].erase(itr);
                    break;
                }
            }
        }
        else
        {
            for(std::map<uint32, AI_Spell*>::iterator itr = m_autoCastSpells[sp->autocast_type].begin();
                itr != m_autoCastSpells[sp->autocast_type].end(); ++itr)
            {
                if(itr->second == sp)
                    return;
            }

            m_autoCastSpells[sp->autocast_type].insert(std::make_pair(sp->info->Id, sp));
        }
    }
}

uint32 Pet::GetUntrainCost()
{
    // TODO: implement penalty
    // costs are: 10s, 50s, 1g, 2g, ..(+1g).. 10g cap
    return 1000;
}

bool Pet::ResetTalents(bool costs)
{
    if (!m_Owner || !m_Owner->IsPlayer())
        return false;

    uint32 talentPointsForLevel = GetPetTalentPointsAtLevel();

    if (GetSpentPetTalentPoints() == 0)
    {
        SetUnspentPetTalentPoints(talentPointsForLevel);
        return false;
    }

    uint32 money = 0;

    if(costs)
    {
        money = GetUntrainCost();

        if (m_Owner->GetUInt32Value( PLAYER_FIELD_COINAGE ) < money)
        {
            WorldPacket data(SMSG_BUY_FAILED, 12);
            data << uint64( m_Owner->GetGUID() );
            data << uint32( 0 );
            data << uint8( 2 );     //not enough money
            return false;
        }
    }

    // check if we have any talents in our map
    // if not we don't need to do anything here
    if(m_talents.size())
    {
        PetTalentMap::iterator itr = m_talents.begin();

        for ( ; itr != m_talents.end(); itr++)
        {
            // find our talent
            TalentEntry *talentEntry = dbcTalent.LookupEntry( itr->first );
            if( talentEntry == NULL )
                continue;

            uint32 spellid = talentEntry->RankID[itr->second];

            SpellEntry * sp = dbcSpell.LookupEntry( spellid );
            if( sp )
            {
                if(mSpells.find( sp ) != mSpells.end())
                    RemoveSpell( spellid );

                for(uint32 k = 0; k < 3; ++k)
                {
                    if(sp->Effect[k] == SPELL_EFFECT_LEARN_SPELL)
                    {
                        SpellEntry * sp2 = dbcSpell.LookupEntry(sp->EffectTriggerSpell[k]);
                        if(!sp2) continue;
                        RemoveSpell(sp2);
                    }
                }
            }
        }
        m_talents.clear();
    }

    SetUnspentPetTalentPoints(talentPointsForLevel);

    if(costs)
        m_Owner->ModUnsigned32Value( PLAYER_FIELD_COINAGE , -(int32)money);

    SendSpellsToOwner();
    return true;
}

void Pet::InitTalentsForLevel(bool creating)
{
    if(!m_Owner)
        return;

    uint8 talentPointsForLevel = GetPetTalentPointsAtLevel();

    if(creating)
    {
        SetUnspentPetTalentPoints(talentPointsForLevel);
        return;
    }

    uint8 spentPoints = GetSpentPetTalentPoints();
    // check  for cheats,
    if(talentPointsForLevel == 0 || spentPoints > talentPointsForLevel)
    {
        // Remove all talent points
        ResetTalents(false);
        return;
    }
    uint8 newPoints = talentPointsForLevel - spentPoints;

    SetUnspentPetTalentPoints(newPoints);
}

uint8 Pet::GetPetTalentPointsAtLevel()
{
    uint16 level = getLevel();
    if(level < 19)
        return 0;

    uint8 points = ((level-19)/4);

    // take into account any points we have gained
    // from SPELL_AURA_MOD_PET_TALENT_POINTS
    if(m_Owner)
        points += m_Owner->m_PetTalentPointModifier;

    // calculations are done return total points
    return points;
}

void Pet::InitializeTalents()
{
    uint32 talentid = 0;
    uint32 rank = 0;
    uint32 spellId = 0;
    for(PetTalentMap::iterator itr = m_talents.begin(); itr != m_talents.end(); itr++)
    {
        talentid = itr->first;
        rank = itr->second;
        sLog.outString("Pet Talent: Attempting to load talent %u", talentid);

        // find our talent
        TalentEntry * talentEntry = dbcTalent.LookupEntry(talentid);
        if(!talentEntry)
        {
            spellId = 0;
            sLog.Warning("Pet Talent", "Tried to load non-existent talent %u", talentid);
            continue;
        }

        // get the spell id for the talent rank we need to add
        spellId = talentEntry->RankID[rank];

        // add the talent spell to our pet
        LearnSpell( spellId );
    }
}

void Pet::SendActionFeedback( PetActionFeedback value )
{
    if( m_Owner == NULL || castPtr<Player>(m_Owner)->GetSession() == NULL)
        return;
    castPtr<Player>(m_Owner)->GetSession()->OutPacket( SMSG_PET_ACTION_FEEDBACK, 1, &value );
}

void Pet::SendCastFailed( uint32 spellid, uint8 fail )
{
    if( m_Owner == NULL || castPtr<Player>(m_Owner)->GetSession() == NULL)
        return;

    WorldPacket data( SMSG_PET_CAST_FAILED, 5 );
    data << uint32(spellid);
    data << uint8(fail);
    castPtr<Player>(m_Owner)->GetSession()->SendPacket( &data );
}