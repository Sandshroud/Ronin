/***
 * Demonstrike Core
 */

#pragma once

class AIInterface;
class CreatureTemplate;

#define MAX_CREATURE_ITEMS 128
#define MAX_CREATURE_LOOT 8
#define MAX_PET_SPELL 4
#define VENDOR_ITEMS_UPDATE_TIME 3600000

struct CreatureItem
{
    uint32 itemid;
    uint32 available_amount, max_amount;
    uint32 incrtime;
    uint32 vendormask;
    ItemExtendedCostEntry *extended_cost;
    bool IsDependent;
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
    NOUNITTYPE      = 0,
    BEAST           = 1,
    DRAGONSKIN      = 2,
    DEMON           = 3,
    ELEMENTAL       = 4,
    GIANT           = 5,
    UNDEAD          = 6,
    HUMANOID        = 7,
    CRITTER         = 8,
    MECHANICAL      = 9,
    UNIT_TYPE_MISC  = 10,
    UNIT_TYPE_TOTEM = 11,
    UNIT_TYPE_NONCOMBAT_PET = 12,
    UNIT_TYPE_GAS_CLOUD = 13,
};

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

class AuctionHouse;

#define TRIGGER_AI_EVENT(obj, func)

///////////////////
/// Creature object

class SERVER_DECL Creature : public Unit
{
    friend class AIInterface;
public:
    Creature(CreatureData *data, uint64 guid);
    virtual ~Creature();
    virtual void Init();
    virtual void Destruct();

    const char* GetName()
    {
        if(getGender() && !_creatureData->femaleName.empty())
            return _creatureData->femaleName.c_str();
        return _creatureData->maleName.c_str();
    }

    /// Updates
    virtual void Update(uint32 msTime, uint32 uiDiff);

    float GetPowerMod() { return _creatureData->powerMod; }
    float GetHealthMod() { return _creatureData->healthMod; }
    int32 GetBonusMana() { return 0; }
    int32 GetBonusHealth() { return 5; }
    int32 GetBonusStat(uint8 type) { return 0; };
    int32 GetBaseAttackTime(uint8 weaponType);
    int32 GetBaseMinDamage(uint8 weaponType) { return _creatureData->minDamage; }
    int32 GetBaseMaxDamage(uint8 weaponType) { return _creatureData->maxDamage; }
    int32 GetBonusAttackPower() { return 0; };
    int32 GetBonusRangedAttackPower() { return 0; };
    int32 GetBonusResistance(uint8 school) { return _creatureData->resistances[school]; }

    void Load(uint32 mapId, float x, float y, float z, float o, uint32 mode, CreatureSpawn *spawn = NULL);

    bool CanAddToWorld();
    void OnPushToWorld();
    void Respawn(bool addrespawnevent, bool free_guid);

    // AIInterface
    RONIN_INLINE AIInterface *GetAIInterface() { return &m_aiInterface; }

    /// Arena organizers
    RONIN_INLINE bool ArenaOrganizersFlags() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDCHANGER ); }

    RONIN_INLINE uint32 GetSQL_id() { return IsSpawn() ? GetSpawn()->id : 0; };

    /// Creature inventory
    RONIN_INLINE uint32 GetItemIdBySlot(uint32 slot) { return m_SellItems->at(slot).itemid; }
    RONIN_INLINE bool HasItems() { return ((m_SellItems != NULL) ? true : false); }
    RONIN_INLINE int32 GetVendorMask() { return (m_spawn ? m_spawn->vendormask : 0x01); }

    int32 GetSlotByItemId(uint32 itemid)
    {
        uint32 slot = 0;
        for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
        {
            if(GetVendorMask() > 0 && itr->second.vendormask > 0)
            {
                if(GetVendorMask() != itr->second.vendormask)
                {
                    ++slot;
                    continue;
                }
            }

            if(itr->second.itemid == itemid)
                return slot;
            ++slot;
        }
        return -1;
    }

    RONIN_INLINE CreatureItem *GetSellItemBySlot(uint32 slot) { return &m_SellItems->at(slot); }
    RONIN_INLINE CreatureItem *GetSellItemByItemId(uint32 itemid)
    {
        for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
            if(itr->second.itemid == itemid)
                return &itr->second;
        return NULL;
    }

    void SendInventoryList(Player *plr);
    RONIN_INLINE std::map<uint32, CreatureItem>::iterator GetSellItemBegin() { return m_SellItems->begin(); }
    RONIN_INLINE std::map<uint32, CreatureItem>::iterator GetSellItemEnd()   { return m_SellItems->end(); }
    RONIN_INLINE void RemoveSellItem(std::map<uint32, CreatureItem>::iterator itr) { m_SellItems->erase(itr); }
    RONIN_INLINE size_t GetSellItemCount() { return m_SellItems->size(); }
    void RemoveVendorItem(uint32 itemid)
    {
        for(std::map<uint32, CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); itr++)
        {
            if(itr->second.itemid == itemid)
            {
                m_SellItems->erase(itr);
                return;
            }
        }
    }

    void AddVendorItem(uint32 itemid, uint32 vendormask, uint32 ec = 0);
    void ModAvItemAmount(uint32 itemid, uint32 value);
    void UpdateItemAmount(uint32 itemid);

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

    void RegenerateHealth(bool isinterrupted);
    void RegenerateMana(bool isinterrupted);

    bool CanSee(Unit* obj) // * Invisibility & Stealth Detection - Partha *
    {
        if(!obj)
            return false;

        if(obj->IsInvisible()) // Invisibility - Detection of Players and Units
        {
            if(obj->getDeathState() == CORPSE) // can't see dead players' spirits
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
    void LoadAIAgents(CreatureTemplate * t);
    void LoadAIAgents();
    void DeleteFromDB();

    void OnJustDied();
    void OnRemoveCorpse();
    void OnRespawn(MapInstance* m);
    void SafeDelete();

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
    void Despawn(uint32 delay, uint32 respawntime);

    void DeleteMe();
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
    void GenerateLoot();
    uint32 GetRequiredLootSkill();

    // updates the loot state, whether it is tagged or lootable, or no longer has items
    void UpdateLootAnimation(Player* Looter);

    // clears tag, clears "tagged" visual grey
    void ClearTag();

    // tags the object by a certain player.
    void Tag(Player* plr);

    // used by bgs
    bool IsLightwell(uint32 entry) { return (GetEntry() == 31883 || GetEntry() == 31893 || GetEntry() == 31894 || GetEntry() == 31895 || GetEntry() == 31896 || GetEntry() == 31897); }

    ItemPrototype *GetShieldProto() { return m_shieldProto; }

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

    bool m_skinned;
    int8 m_lootMethod;
    uint32 m_taggingGroup, m_taggingPlayer;

    ItemPrototype* m_shieldProto;

    // AI
    AIInterface m_aiInterface;

protected:
    uint32 m_AreaUpdateTimer;

public:

    CreatureSpawn * m_spawn;
    CreatureData *_creatureData;
    CreatureInfoExtra * _extraInfo;

    CreatureFamilyEntry *myFamily;
    AuctionHouse *auctionHouse;

    MapCell *m_respawnCell;

    void _LoadMovement();

    /// Vendor data
    std::map<uint32, CreatureItem> *m_SellItems;

    /// Taxi data
    uint32 m_taxiNode[2];

    /// Quest data
    std::list<QuestRelation *>* m_quests;

    /// Pet
    uint32 m_enslaveCount;
    uint32 m_enslaveSpell;

};
