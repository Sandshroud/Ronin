/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

class AIInterface;
class CreatureTemplate;

#define MAX_CREATURE_ITEMS 128
#define MAX_CREATURE_LOOT 8
#define MAX_PET_SPELL 4
#define VENDOR_ITEMS_UPDATE_TIME 3600000

struct AvailableCreatureItem
{
    AvailableCreatureItem() : proto(NULL), availableAmount(0), max_amount(0), incrtime(0), extended_cost(NULL), refreshTime(0), IsDependent(false) {}

    ItemPrototype *proto;
    uint32 availableAmount, max_amount, incrtime;
    ItemExtendedCostEntry *extended_cost;
    time_t refreshTime;
    bool IsDependent;
};

struct TrainerSpell
{
    SpellEntry *entry;
    uint32 spellCost;
    uint32 requiredLevel;
    uint32 reqSkill;
    uint32 reqSkillValue;
    bool useSpellRequiredLevel;
};

struct TrainerData
{
    uint8 category;
    uint8 subCategory;
    uint32 reqSkill;
    uint32 reqSkillValue;
    std::string trainerTitle;
};

#pragma pack(PRAGMA_PACK)
struct SeatInfo
{
    uint32 accessoryentry;
    bool ejectfromvehicleondeath;
    bool unselectableaccessory;
};

struct CreatureVehicleData
{
    uint32 vehicle_creature_entry; // Entry.
    bool healthfromdriver; // Effects only driver.
    uint32 healthunitfromitemlev;
    uint32 VehicleSpells[6]; // Vehicle spells.
    uint32 MovementFlags;
    SeatInfo seats[8]; // Accessories.
};

struct CreatureInfoExtra
{
    uint32 entry;
    uint32 default_emote_state;
    uint32 default_flags;
    uint16 default_stand_state;
    uint32 default_MountedDisplayID;
    bool   m_canRangedAttack;
    bool   m_canCallForHelp;
    float  m_callForHelpHP;
    bool   m_canFlee;
    float  m_fleeHealth;
    uint32 m_fleeDuration;
    int    sendflee_message;
    std::string flee_message;
    bool   no_skill_up;
    bool   no_xp;
    bool   isBoss;
};

#pragma pack(PRAGMA_POP)

struct Formation{
    uint32 fol;
    float ang;
    float dist;
};

enum UNIT_TYPE
{
    UT_NOUNITTYPE       = 0,
    UT_BEAST            = 1,
    UT_DRAGONKIN        = 2,
    UT_DEMON            = 3,
    UT_ELEMENTAL        = 4,
    UT_GIANT            = 5,
    UT_UNDEAD           = 6,
    UT_HUMANOID         = 7,
    UT_CRITTER          = 8,
    UT_MECHANICAL       = 9,
    UT_UNSPECIFIED      = 10,
    UT_TOTEM            = 11,
    UT_NONCOMBAT_PET    = 12,
    UT_GAS_CLOUD        = 13,
};

static const char *unitTypeNames[14] = { "NoType", "Beast", "Dragonkin", "Demon", "Elemental", "Giant", "Undead", "Humanoid", "Critter", "Mechanical", "Unspecified", "Totem", "NonCombatPet", "GasCloud" };

enum FAMILY
{
    FAMILY_WOLF             = 1,
    FAMILY_CAT,
    FAMILY_SPIDER,
    FAMILY_BEAR,
    FAMILY_BOAR,
    FAMILY_CROCILISK,
    FAMILY_CARRION_BIRD,
    FAMILY_CRAB,
    FAMILY_GORILLA,
    FAMILY_RAPTOR           = 11,
    FAMILY_TALLSTRIDER ,
    FAMILY_FELHUNTER        = 15,
    FAMILY_VOIDWALKER,
    FAMILY_SUCCUBUS,
    FAMILY_DOOMGUARD        = 19,
    FAMILY_SCORPID,
    FAMILY_TURTLE,
    FAMILY_IMP              = 23,
    FAMILY_BAT,
    FAMILY_HYENA,
    FAMILY_BIRD_OF_PREY,
    FAMILY_WIND_SERPENT,
    FAMILY_REMOTE_CONTROL,
    FAMILY_FELGUARD,
    FAMILY_DRAGONHAWK,
    FAMILY_RAVAGER,
    FAMILY_WARP_STALKER,
    FAMILY_SPOREBAT,
    FAMILY_NETHER_RAY,
    FAMILY_SERPENT,
    FAMILY_MOTH             = 37,
    FAMILY_CHIMAERA,
    FAMILY_DEVILSAUR,
    FAMILY_GHOUL,
    FAMILY_SILITHID,
    FAMILY_WORM,
    FAMILY_RHINO,
    FAMILY_WASP,
    FAMILY_CORE_HOUND,
    FAMILY_SPIRIT_BEAST,
    FAMILY_FAKE_IMP         = 416,
    FAMILY_FAKE_FELHUNTER,
    FAMILY_FAKE_VOIDWALKER  = 1860,
    FAMILY_FAKE_SUCCUBUS    = 1863,
    FAMILY_FAKE_FELGUARD    = 17252
};

enum CreatureFlags1
{
    CREATURE_FLAGS1_TAMEABLE   = 0x0001,
    CREATURE_FLAGS1_BOSS       = 0x6C,
    CREATURE_FLAGS1_HERBLOOT   = 0x0100,
    CREATURE_FLAGS1_MININGLOOT = 0x0200,
    CREATURE_FLAGS1_ENGINEERLOOT = 0x08000,
};

enum ELITE
{
    ELITE_NORMAL = 0,
    ELITE_ELITE,
    ELITE_RAREELITE,
    ELITE_WORLDBOSS,
    ELITE_RARE
};

enum TIME_REMOVE_CORPSE
{
    TIME_CREATURE_REMOVE_CORPSE = 180000,
    TIME_CREATURE_REMOVE_RARECORPSE = 180000*3,
    TIME_CREATURE_REMOVE_BOSSCORPSE = 180000*5,
};

struct PetSpellCooldown
{
    uint32 spellId;
    int32 cooldown;
};

struct CreatureSpell
{
    uint32 castTimer;
    uint32 cooldownTimer;
    SpellEntry *spellEntry;
};

class AuctionHouse;

#define TRIGGER_AI_EVENT(obj, func)

///////////////////
/// Creature object

class SERVER_DECL Creature : public Unit
{
    friend class AIInterface;

protected:
    friend class MapInstance;
    friend class MapCell;

    // DO NOT CALL DIRECTLY, USE MAP INSTANCE ALLOCATION SYSTEM
    Creature();
    void Construct(CreatureData *data, WoWGuid guid);

public:
    virtual ~Creature();
    virtual void Init();
    virtual void Destruct();
    virtual void Reactivate();

    virtual void Update(uint32 msTime, uint32 uiDiff);

    virtual void RemoveFromWorld();

    virtual bool IsCreature() { return true; }

    virtual bool IsActiveObject() { return true; }
    virtual uint32 getEventID() { return m_spawn ? m_spawn->eventId : 0; }

    /// Updates
    virtual void UpdateFieldValues();
    virtual void OnAuraModChanged(uint32 modType);

    const char* GetName()
    {
        if(getGender() && !_creatureData->femaleName.empty())
            return _creatureData->femaleName.c_str();
        return _creatureData->maleName.c_str();
    }

    void EventUpdateCombat(uint32 msTime, uint32 uiDiff);
    void EventAttackStop();

    float GetPowerMod() { return _creatureData->powerMod; }
    float GetHealthMod() { return _creatureData->healthMod; }
    int32 GetBonusMana() { return 0; }
    int32 GetBonusHealth() { return 0; }
    int32 GetBonusStat(uint8 type) { return 0; };
    int32 GetBaseAttackTime(uint8 weaponType);
    int32 GetBaseMinDamage(uint8 weaponType) { return 1.0f; }
    int32 GetBaseMaxDamage(uint8 weaponType) { return 2.0f; }
    int32 GetBonusAttackPower() { return 0; };
    int32 GetBonusRangedAttackPower() { return 0; };
    int32 GetBonusResistance(uint8 school) { return _creatureData->resistances[school]; }

    void Load(uint32 mapId, float x, float y, float z, float o, uint32 mode, CreatureSpawn *spawn = NULL, bool reload = false);

    bool CanAddToWorld();
    void OnPushToWorld();
    virtual void DetatchFromSummoner() { Cleanup(); }

    void BuildTrainerData(WorldPacket *data, Player *plr);
    bool CanTrainPlayer(Player *plr);

    uint8 GetTrainerCategory() { return m_trainerData ? m_trainerData->category : 0; }
    uint8 GetTrainerSubCategory() { return m_trainerData ? m_trainerData->subCategory : 0; }
    bool IsProfessionTrainer() { return m_trainerData && m_trainerData->category == TRAINER_CATEGORY_TRADESKILLS; }
    bool IsClassTrainer() { return m_trainerData && m_trainerData->category == TRAINER_CATEGORY_TALENTS; }
    bool IsPetTrainer() { return m_trainerData && m_trainerData->category == TRAINER_CATEGORY_PET; }

    virtual void UpdateAreaInfo(MapInstance *instance = NULL);

    // Creature data based functions
    uint32 GetCreatureType() { return _creatureData->type; }
    bool isBeast() { return _creatureData->type == UT_BEAST; }
    bool isCritter() { return _creatureData->type == UT_CRITTER; }
    bool isFodderSpawn() { return _creatureData->extraFlags & CREATURE_DATA_EX_FLAG_FODDER_SPAWN; }
    bool isTrainingDummy() { return _creatureData->extraFlags & CREATURE_DATA_EX_FLAG_TRAINING_DUMMY; }
    bool isWorldTrigger() { return _creatureData->extraFlags & CREATURE_DATA_EX_FLAG_WORLD_TRIGGER; }

    float GetAggroRange();

    void Respawn(bool addrespawnevent, bool free_guid);

    // AIInterface
    RONIN_INLINE AIInterface *GetAIInterface() { return &m_aiInterface; }

    // Arena organizers
    RONIN_INLINE bool ArenaOrganizersFlags() const { return !HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDCHANGER ); }

    // Creature inventory
    RONIN_INLINE bool HasItems() { return !m_vendorItems.empty(); }
    RONIN_INLINE int32 GetVendorMask() { return (m_spawn ? m_spawn->vendormask : 0x01); }

    uint32 GetItemIdBySlot(uint16 slot);
    uint32 GetSlotByItemId(uint32 itemid);
    int32 GetAvailableAmount(uint16 slot, int32 defaultVal);

    AvailableCreatureItem *GetSellItemBySlot(uint16 slot);

    void SendInventoryList(Player *plr);
    RONIN_INLINE std::vector<AvailableCreatureItem>::iterator GetSellItemBegin() { return m_vendorItems.begin(); }
    RONIN_INLINE std::vector<AvailableCreatureItem>::iterator GetSellItemEnd()   { return m_vendorItems.end(); }
    RONIN_INLINE size_t GetSellItemCount() { return m_vendorItems.size(); }

    void AddVendorItem(uint32 itemid, uint32 vendormask, uint32 ec = 0);
    int32 ModAvItemAmount(uint32 slot, uint32 value);

    /// Quests
    void _LoadQuests();
    bool HasQuests() { return m_quests != NULL; };
    bool HasQuest(uint32 id, uint32 type)
    {
        if(!m_quests) return false;
        for(std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); itr++)
        {
            if((*itr)->qst->id == id && (*itr)->type & type)
                return true;
        }
        return false;
    }

    void AddQuest(QuestRelation *Q);
    void DeleteQuest(QuestRelation *Q);
    Quest *FindQuest(uint32 quest_id, uint8 quest_relation);
    uint16 GetQuestRelation(uint32 quest_id);
    uint32 NumOfQuests();
    std::list<QuestRelation *>::iterator QuestsBegin() { return m_quests->begin(); };
    std::list<QuestRelation *>::iterator QuestsEnd() { return m_quests->end(); };
    void SetQuestList(std::list<QuestRelation *>* qst_lst) { m_quests = qst_lst; };

    RONIN_INLINE void SetSheatheForAttackType(uint8 type) { SetByte(UNIT_FIELD_BYTES_2, 0, type); }
    RONIN_INLINE bool isQuestGiver() { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ); };

    void RegenerateHealth(bool inCombat);
    void RegeneratePower(bool isinterrupted);

    bool CanSee(Unit* obj) // * Invisibility & Stealth Detection - Partha *
    {
        if(!obj)
            return false;

        if(obj->IsInvisible()) // Invisibility - Detection of Players and Units
        {
            if(obj->getDeathState()) // can't see dead players' spirits
                return false;

            if(m_invisDetect[obj->m_invisFlag] < 1) // can't see invisible without proper detection
                return false;
        }

        if(obj->HasDummyAura(0xFFFFFFFF)) // Stealth Detection (  I Hate Rogues :P  )
        {
            // TODO STEALTH DETECTION SEMPAI
            detectRange += GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); // adjust range for size of creature
            detectRange += obj->GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS); // adjust range for size of stealthed player

            if(GetDistance2dSq(obj) > detectRange * detectRange)
                return false;
        }

        return true;
    }

    //Make this unit face another unit
    bool setInFront(Unit* target);

    virtual void SetDeathState(DeathState s);

    // Serialization
    void SaveToDB(bool saveposition = false);
    void DeleteFromDB();

    void OnRemoveCorpse();

    // Demon
    void EnslaveExpire();

    // Pet
    void UpdatePet();
    uint32 GetEnslaveCount() { return m_enslaveCount; }
    void SetEnslaveCount(uint32 count) { m_enslaveCount = count; }
    uint32 GetEnslaveSpell() { return m_enslaveSpell; }
    void SetEnslaveSpell(uint32 spellId) { m_enslaveSpell = spellId; }
    bool RemoveEnslave();

    RONIN_INLINE bool IsPickPocketed() { return m_pickPocketed; }
    RONIN_INLINE void SetPickPocketed(bool val = true) { m_pickPocketed = val; }

    bool isBoss();
    bool isCivilian();
    RONIN_INLINE bool IsSpawn() { return m_spawn != NULL; }
    RONIN_INLINE CreatureSpawn *GetSpawn() { return m_spawn; }
    RONIN_INLINE CreatureData *GetCreatureData() { return _creatureData; }
    RONIN_INLINE CreatureInfoExtra *GetExtraInfo() { return _extraInfo; }

    void FormationLinkUp(uint32 SqlId);
    uint32 GetRespawnTime() { return _creatureData ? _creatureData->respawnTime : 0; }
    void Despawn(uint32 respawntime);

    bool IsInLimboState() { return m_limbostate; }
    uint32 GetLineByFamily(CreatureFamilyEntry * family) { return family->skillLine[0] ? family->skillLine[0] : 0; };
    void RemoveLimboState(Unit* healer);
    uint32 GetCanMove() { return _creatureData->movementMask; }

    bool HasNpcFlag(uint32 Flag)
    {
        if(GetUInt32Value(UNIT_NPC_FLAGS) & Flag)
            return true;
        return false;
    }

    uint32 GetTaxiNode(uint8 team) { ASSERT(team < 2); return m_taxiNode[team]; }
    void SendTaxiList(Player *plr);

public:
    RONIN_INLINE uint32 GetProtoItemDisplayId(uint8 i) { return GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + i); }

    // loooooot
    virtual void GenerateLoot();
    uint32 GetRequiredLootSkill();

    // updates the loot state, whether it is tagged or lootable, or no longer has items
    void UpdateLootAnimation();

    // clears tag, clears "tagged" visual grey
    void ClearTag();

    // tags the object by a certain player.
    void Tag(Player* plr);

    // checks if we meet tag requirements
    bool IsTaggedByPlayer(Player *plr);

    // used by bgs
    bool IsLightwell(uint32 entry) { return (GetEntry() == 31883 || GetEntry() == 31893 || GetEntry() == 31894 || GetEntry() == 31895 || GetEntry() == 31896 || GetEntry() == 31897); }

    ItemPrototype *GetShieldProto() { return m_shieldProto; }

    // Scripted hostile NPC for inrange interaction
    bool IsScriptedNPCHostile() { return false; }//m_script && m_script->CanInteractWithNearbyNPCs();}

public: // values
    bool m_isGuard;
    bool b_has_shield;
    bool m_corpseEvent;
    bool m_noRespawn;
    bool has_waypoint_text;
    bool has_combat_text;
    bool haslinkupevent;
    bool m_canRegenerateHP;
    bool m_noDeleteAfterDespawn;
    bool m_limbostate;
    bool m_pickPocketed;
    bool m_zoneVisibleSpawn, m_areaVisibleSpawn;

    bool m_skinned;
    int8 m_lootMethod;
    WoWGuid m_taggedGroup, m_taggedPlayer;

    ItemPrototype* m_shieldProto;

    // AI
    AIInterface m_aiInterface;

protected:
    float m_aggroRangeMod;

public:

    CreatureSpawn * m_spawn;
    CreatureData *_creatureData;
    CreatureInfoExtra * _extraInfo;
    CreatureFamilyEntry *myFamily;
    AuctionHouse *auctionHouse;

    uint32 m_despawnTimer;

    /// Vendor data
    std::vector<AvailableCreatureItem> m_vendorItems;

    /// Trainer data
    TrainerData *m_trainerData;

    /// Taxi data
    uint32 m_taxiNode[2];

    /// Quest data
    std::list<QuestRelation *>* m_quests;

    /// Pet
    uint32 m_enslaveCount;
    uint32 m_enslaveSpell;

    //CreatureScript *m_script;

    /// Creature spell casting system
    std::vector<CreatureSpell*> m_combatSpells;

};
