/***
 * Demonstrike Core
 */

#pragma once

class Player;
class GameObjectAIScript;
class GameObjectTemplate;

enum GAMEOBJECT_BYTES
{
    GAMEOBJECT_BYTES_STATE          = 0,
    GAMEOBJECT_BYTES_TYPE_ID        = 1,
    GAMEOBJECT_BYTES_UNK            = 2, // todo: unknown atm
    GAMEOBJECT_BYTES_ANIMPROGRESS   = 3,
};

enum GAMEOBJECT_TYPES
{
    GAMEOBJECT_TYPE_DOOR                    = 0,
    GAMEOBJECT_TYPE_BUTTON                  = 1,
    GAMEOBJECT_TYPE_QUESTGIVER              = 2,
    GAMEOBJECT_TYPE_CHEST                   = 3,
    GAMEOBJECT_TYPE_BINDER                  = 4,
    GAMEOBJECT_TYPE_GENERIC                 = 5,
    GAMEOBJECT_TYPE_TRAP                    = 6,
    GAMEOBJECT_TYPE_CHAIR                   = 7,
    GAMEOBJECT_TYPE_SPELL_FOCUS             = 8,
    GAMEOBJECT_TYPE_TEXT                    = 9,
    GAMEOBJECT_TYPE_GOOBER                  = 10,
    GAMEOBJECT_TYPE_TRANSPORT               = 11,
    GAMEOBJECT_TYPE_AREADAMAGE              = 12,
    GAMEOBJECT_TYPE_CAMERA                  = 13,
    GAMEOBJECT_TYPE_MAP_OBJECT              = 14,
    GAMEOBJECT_TYPE_MO_TRANSPORT            = 15,
    GAMEOBJECT_TYPE_DUEL_ARBITER            = 16,
    GAMEOBJECT_TYPE_FISHINGNODE             = 17,
    GAMEOBJECT_TYPE_RITUAL                  = 18,
    GAMEOBJECT_TYPE_MAILBOX                 = 19,
    GAMEOBJECT_TYPE_AUCTIONHOUSE            = 20,
    GAMEOBJECT_TYPE_GUARDPOST               = 21,
    GAMEOBJECT_TYPE_SPELLCASTER             = 22,
    GAMEOBJECT_TYPE_MEETINGSTONE            = 23,
    GAMEOBJECT_TYPE_FLAGSTAND               = 24,
    GAMEOBJECT_TYPE_FISHINGHOLE             = 25,
    GAMEOBJECT_TYPE_FLAGDROP                = 26,
    GAMEOBJECT_TYPE_MINI_GAME               = 27,
    GAMEOBJECT_TYPE_LOTTERY_KIOSK           = 28,
    GAMEOBJECT_TYPE_CAPTURE_POINT           = 29,
    GAMEOBJECT_TYPE_AURA_GENERATOR          = 30,
    GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY      = 31,
    GAMEOBJECT_TYPE_BARBER_CHAIR            = 32,
    GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING   = 33,
    GAMEOBJECT_TYPE_GUILD_BANK              = 34,
    GAMEOBJECT_TYPE_TRAPDOOR                = 35
};

enum GameObjectFlags
{
    GO_FLAG_IN_USE          = 0x001,        // Disables interaction while animated
    GO_FLAG_LOCKED          = 0x002,        // Require key, spell, event, etc to be opened. Makes "Locked" appear in tooltip
    GO_FLAG_INTERACT_COND   = 0x004,        // Cannot interact (condition to interact)
    GO_FLAG_TRANSPORT       = 0x008,        // Any kind of transport? Object can transport (elevator, boat, car)
    GO_FLAG_UNK1            = 0x010,
    GO_FLAG_NODESPAWN       = 0x020,        // Never despawn, typically for doors, they just change state
    GO_FLAG_TRIGGERED       = 0x040,        // Typically, summoned objects. Triggered by spell or other events
    GO_FLAG_DAMAGED         = 0x200,
    GO_FLAG_DESTROYED       = 0x400
};

enum GameObjectDynFlags
{
    GO_DYNFLAG_QUEST        = 0x09,
};

enum GoUInt32Types
{
    GO_UINT32_HEALTH            = 0,
    GO_UINT32_MINES_REMAINING   = 1, // Used for mining to mark times it can be mined
    GO_UINT32_M_RIT_CASTER      = 2,
    GO_UINT32_M_RIT_TARGET      = 3,
    GO_UINT32_RIT_SPELL         = 4,
    GO_UINT32_MAX
};

#pragma pack(PRAGMA_PACK)

struct GameObjectInfo
{
    uint32 ID;
    uint32 Type;
    uint32 DisplayID;
    char * Name;
    char * Icon;
    char * CastBarText;
    int32 RespawnTimer;
    uint32 DefaultFlags;

    /* Crow: Time for some ugly unions, thanks for the section meanings Tom.
    ** Note that all slot ones replace Spell Focus, the rest are sound and unks
    */
    union
    {
        // GAMEOBJECT_TYPE_DOOR
        struct
        {
            uint32 StartOpen; /* 1 */
            uint32 LockId; uint32 AutoCloseTime; uint32 NoDamageImmune;
            uint32 OpenTextID; uint32 CloseTextID; uint32 IgnoredByPathing;
        } Door;

        // GAMEOBJECT_TYPE_BUTTON
        struct
        {
            uint32 StartOpen; /* 1 */
            uint32 LockId; uint32 AutoCloseTime; uint32 LinkedTrap;
            uint32 NoDamageImmune; uint32 Large; uint32 OpenTextID; uint32 CloseTextID; uint32 LosOK;
        } Button;

        // GAMEOBJECT_TYPE_QUESTGIVER
        struct
        {
            uint32 LockId; /* 1 */
            uint32 QuestList; uint32 PageMaterial; uint32 GossipID; uint32 CustomAnim;
            uint32 NoDamageImmune; uint32 OpenTextID; uint32 LosOK; uint32 AllowMounted; uint32 Large;
        } QuestGiver;

        // GAMEOBJECT_TYPE_CHEST
        struct
        {
            uint32 LockId; /* 1 */
            uint32 LootId; uint32 ChestRestockTime; uint32 Consumable; uint32 MinSuccessOpens; uint32 MaxSuccessOpens; uint32 EventId; uint32 LinkedTrapId;
            uint32 QuestId; uint32 Level; uint32 LosOK; uint32 LeaveLoot; uint32 NotInCombat; uint32 LogLoot; uint32 OpenTextID; uint32 GroupLootRules; uint32 FloatingTooltip;
        } Chest;

        // GAMEOBJECT_TYPE_BINDER - GAMEOBJECT_TYPE_GENERIC
        struct
        {
            uint32 FloatingTooltip; /* 1 */
            uint32 Highlight; uint32 ServerOnly;
            uint32 Large; uint32 FloatOnWater; int32 QuestID;
        } Generic;

        // GAMEOBJECT_TYPE_TRAP
        struct
        {
            uint32 LockId; /* 1 */
            uint32 Level; uint32 Radius; uint32 SpellId; uint32 Charges; uint32 Cooldown; int32 AutoCloseTime; uint32 StartDelay;
            uint32 ServerOnly; uint32 Stealthed; uint32 Large; uint32 Invisible; uint32 OpenTextID; uint32 CloseTextID; uint32 IgnoreTotems;
        } Trap;

        // GAMEOBJECT_TYPE_CHAIR
        struct { uint32 Slots; /* 1 */ uint32 Height; uint32 OnlyCreatorUse; uint32 TriggeredEvent; } Chair;

        // GAMEOBJECT_TYPE_SPELL_FOCUS
        struct
        {
            uint32 FocusId; /* 1 */
            uint32 Dist/* sound1 */; uint32 LinkedTrapId/* sound2 */; uint32 ServerOnly/* sound3 */;
            uint32 QuestID/* sound4 */; uint32 Large/* sound5 */; uint32 FloatingTooltip/* sound6 */;
        } TypeSpellFocus;

        // GAMEOBJECT_TYPE_TEXT
        struct { uint32 PageID; /* 1 */ uint32 Language; uint32 PageMaterial; uint32 AllowMounted; } Text;

        // GAMEOBJECT_TYPE_GOOBER
        struct
        {
            uint32 LockId; /* 1 */
            int32 QuestId; uint32 EventId; uint32 AutoCloseTime; uint32 CustomAnim; uint32 Consumable; uint32 Cooldown; uint32 PageId;
            uint32 Language; uint32 PageMaterial; uint32 SpellId; uint32 NoDamageImmune; uint32 LinkedTrapId; uint32 Large; uint32 OpenTextID;
            uint32 CloseTextID; uint32 LosOK; uint32 AllowMounted; uint32 FloatingTooltip; uint32 GossipID; uint32 WorldStateSetsState;
        } Goober;

        // GAMEOBJECT_TYPE_TRANSPORT
        struct { uint32 Pause; /* 1 */ uint32 StartOpen; uint32 AutoCloseTime; uint32 Pause1EventID; uint32 Pause2EventID; } Transport;

        // GAMEOBJECT_TYPE_AREADAMAGE
        struct
        {
            uint32 LockId; /* 1 */
            uint32 Radius; uint32 DamageMin; uint32 DamageMax;
            uint32 DamageSchool; uint32 AutoCloseTime; uint32 OpenTextID; uint32 CloseTextID;
        } AreaDamage;

        // GAMEOBJECT_TYPE_CAMERA
        struct { uint32 LockId; /* 1 */ uint32 CinematicId; uint32 EventID; uint32 OpenTextID; } Camera;

        // GAMEOBJECT_TYPE_MAPOBJECT - GAMEOBJECT_TYPE_MO_TRANSPORT
        struct
        {
            uint32 TaxiPathId; /* 1 */
            uint32 MoveSpeed; uint32 AccelRate; uint32 StartEventID;
            uint32 StopEventID; uint32 TransportPhysics; uint32 MapID; uint32 WorldState1;
        } MoTransport;

        // GAMEOBJECT_TYPE_DUEL_ARBITER - GAMEOBJECT_TYPE_FISHINGNODE - GAMEOBJECT_TYPE_RITUAL
        struct
        {
            uint32 ReqParticipants; /* 1 */
            uint32 SpellId; uint32 AnimSpell; uint32 RitualPersistent; uint32 CasterTargetSpell;
            uint32 CasterTargetSpellTargets; uint32 CastersGrouped; uint32 RitualNoTargetCheck;
        } Arbiter;

        // GAMEOBJECT_TYPE_MAILBOX - GAMEOBJECT_TYPE_DONOTUSE - GAMEOBJECT_TYPE_GUARDPOST
        struct { uint32 CreatureID; /* 1 */ uint32 Charges; } GuardPost;

        // GAMEOBJECT_TYPE_SPELLCASTER
        struct { uint32 SpellId; /* 1 */ uint32 Charges; uint32 PartyOnly; uint32 AllowMounted; uint32 Large; } SpellCaster;

        // GAMEOBJECT_TYPE_MEETINGSTONE
        struct { uint32 MinLevel; /* 1 */ uint32 MaxLevel; uint32 AreaID; } MeetingStone;

        // GAMEOBJECT_TYPE_FLAGSTAND
        struct
        {
            uint32 LockId; /* 1 */
            uint32 PickupSpell; uint32 Radius; uint32 ReturnAura;
            uint32 ReturnSpell; uint32 NoDamageImmune; uint32 OpenTextID; uint32 LosOK;
        } Flagstand;

        // GAMEOBJECT_TYPE_FISHINGHOLE
        struct { uint32 Radius; /* 1 */ uint32 LootId; uint32 MinSuccessOpens; uint32 MaxSuccessOpens; uint32 LockId; } FishingHole;

        // GAMEOBJECT_TYPE_FLAGDROP
        struct { uint32 LockId; /* 1 */ uint32 EventID; uint32 PickupSpell; uint32 NoDamageImmune; uint32 OpenTextID;} FlagDrop;

        // GAMEOBJECT_TYPE_MINI_GAME
        struct { uint32 GameType; /* 1 */ } Minigame;

        // GAMEOBJECT_TYPE_CAPTURE_POINT
        struct
        {
            uint32 Radius; /* 1 */
            uint32 Spell; uint32 WorldState1; uint32 Worldstate2; uint32 WinEventID1; uint32 WinEventID2;
            uint32 ContestedEventID1; uint32 ContestedEventID2; uint32 ProgressEventID1; uint32 ProgressEventID2;
            uint32 NeutralEventID1; uint32 NeutralEventID2; uint32 NeutralPercent; uint32 Worldstate3;
            uint32 MinSuperiority; uint32 MaxSuperiority; uint32 MinTime; uint32 MaxTime;
            uint32 Large; uint32 Highlight; uint32 StartingValue; uint32 Unidirectional;
        } CapturePoint;

        // GAMEOBJECT_TYPE_AURA_GENERATOR
        struct { uint32 StartOpen; /* 1 */ uint32 Radius; uint32 AuraID1; uint32 ConditionID1; uint32 AuraID2; uint32 ConditionID2; uint32 ServerOnly; } AuraGenerator;

        // GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY
        struct { uint32 MapID; /* 1 */ uint32 Difficulty; } DungeonDifficulty;

        // GAMEOBJECT_TYPE_BARBER_CHAIR
        struct { uint32 Chairheight; /* 1 */ uint32 HeightOffset; } BarberChair;

        // GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
        struct
        {
            uint32 IntactNumHits; /* 1 */
            uint32 CreditProxyCreature; uint32 State1Name; uint32 IntactEvent; uint32 DamagedDisplayId; uint32 DamagedNumHits;
            uint32 Empty3; uint32 Empty4; uint32 Empty5; uint32 DamagedEvent; uint32 DestroyedDisplayId; uint32 Empty7; uint32 Empty8; uint32 Empty9;
            uint32 DestroyedEvent; uint32 Empty10; uint32 RebuildingTimeSecs; uint32 Empty11; uint32 DestructibleData; uint32 RebuildingEvent;
            uint32 Empty12; uint32 Empty13; uint32 DamageEvent;
        } DestructableBuilding;

        // GAMEOBJECT_TYPE_GUILDBANK - GAMEOBJECT_TYPE_TRAPDOOR
        struct { uint32 WhenToPause; /* 1 */ uint32 StartOpen; uint32 AutoClose; } TrapDoor;

        // not use for specific field access (only for output with loop by all filed), also this determinate max union size
        struct { uint32 ListedData[24]; } RawData;
    };

    uint32 GetSpellID()
    {
        uint32 spellid = 0;
        switch(Type)
        {
        case GAMEOBJECT_TYPE_TRAP:
            spellid = Trap.SpellId;
            break;
        case GAMEOBJECT_TYPE_GOOBER:
            spellid = Goober.SpellId;
            break;
        case GAMEOBJECT_TYPE_DUEL_ARBITER:
        case GAMEOBJECT_TYPE_FISHINGNODE:
        case GAMEOBJECT_TYPE_RITUAL:
            spellid = Arbiter.SpellId;
            break;
        case GAMEOBJECT_TYPE_SPELLCASTER:
            spellid = SpellCaster.SpellId;
            break;
        case GAMEOBJECT_TYPE_CAPTURE_POINT:
            spellid = CapturePoint.Spell;
            break;
        }
        return spellid;
    }

    uint32 GetLockID()
    {
        uint32 lockid = 0;
        switch(Type)
        {
        case GAMEOBJECT_TYPE_DOOR:
            lockid = Door.LockId;
            break;
        case GAMEOBJECT_TYPE_BUTTON:
            lockid = Button.LockId;
            break;
        case GAMEOBJECT_TYPE_QUESTGIVER:
            lockid = QuestGiver.LockId;
            break;
        case GAMEOBJECT_TYPE_CHEST:
            lockid = Chest.LockId;
            break;
        case GAMEOBJECT_TYPE_TRAP:
            lockid = Trap.LockId;
            break;
        case GAMEOBJECT_TYPE_GOOBER:
            lockid = Goober.LockId;
            break;
        case GAMEOBJECT_TYPE_AREADAMAGE:
            lockid = AreaDamage.LockId;
            break;
        case GAMEOBJECT_TYPE_CAMERA:
            lockid = Camera.LockId;
            break;
        case GAMEOBJECT_TYPE_FLAGSTAND:
            lockid = Flagstand.LockId;
            break;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
            lockid = FishingHole.LockId;
            break;
        case GAMEOBJECT_TYPE_FLAGDROP:
            lockid = FlagDrop.LockId;
            break;
        }
        return lockid;
    }
};

#pragma pack(PRAGMA_POP)

#define CALL_GO_SCRIPT_EVENT(obj, func) if(obj->GetTypeId() == TYPEID_GAMEOBJECT && obj->GetScript() != NULL) obj->GetScript()->func

class SERVER_DECL GameObject : public Object
{
public:
    GameObject(uint64 guid);
    ~GameObject( );
    virtual void Init();
    virtual void Destruct();

    HEARTHSTONE_INLINE GameObjectInfo* GetInfo() { return pInfo; }
    HEARTHSTONE_INLINE void SetInfo(GameObjectInfo * goi) { pInfo = goi; }

    bool CreateFromProto(uint32 entry,uint32 mapid, const LocationVector vec);
    bool CreateFromProto(uint32 entry,uint32 mapid, float x, float y, float z, float ang);

    bool Load(GOSpawn *spawn);

    virtual void Update(uint32 p_time);

    void Spawn( MapMgr* m);
    void Despawn( uint32 delay, uint32 respawntime);
    void UpdateTrapState();

    // Serialization
    void SaveToDB();
    void DeleteFromDB();
    void EventCloseDoor();
    void UpdateRotation(float orientation3 = 0.0f, float orientation4 = 0.0f);

    //Fishing stuff
    void UseFishingNode(Player* player);
    void EndFishing(Player* player,bool abort);
    void FishHooked(Player* player);

    // Quests
    void _LoadQuests();
    bool HasQuests() { return m_quests != NULL; };
    void AddQuest(QuestRelation *Q);
    void DeleteQuest(QuestRelation *Q);
    Quest *FindQuest(uint32 quest_id, uint8 quest_relation);
    uint16 GetQuestRelation(uint32 quest_id);
    uint32 NumOfQuests();
    std::list<QuestRelation *>::iterator QuestsBegin() { return m_quests->begin(); };
    std::list<QuestRelation *>::iterator QuestsEnd() { return m_quests->end(); };
    void SetQuestList(std::list<QuestRelation *>* qst_lst) { m_quests = qst_lst; };

    void SetSummoned(Unit* mob)
    {
        m_summoner = mob;
        m_summonedGo = true;
    }

    Unit* CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, Unit* u_caster, uint8 Slot);
    void _Expire();

    void ExpireAndDelete();
    void ExpireAndDelete(uint32 delay);

    HEARTHSTONE_INLINE bool isQuestGiver()
    {
        if( GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID) == 2)
            return true;
        else
            return false;
    };

    /// Quest data
    std::list<QuestRelation *>* m_quests;

    uint32 *m_ritualmembers;
    uint64 m_rotation;

    void InitAI();
    SpellEntry* spell;

    bool initiated;
    bool m_created;
    bool m_scripted_use;
    float range;
    uint8 checkrate;
    uint16 counter;
    int32 charges;//used for type==22,to limit number of usages.
    bool invisible;//invisible
    uint8 invisibilityFlag;
    Unit* m_summoner;
    int8 bannerslot;
    int8 bannerauraslot;
    CBattleground* m_battleground;

    HEARTHSTONE_INLINE GameObjectAIScript* GetScript() { return myScript; }

    void TrapSearchTarget();    // Traps need to find targets faster :P

    HEARTHSTONE_INLINE bool HasAI() { return spell != 0; }
    GOSpawn * m_spawn;
    void OnPushToWorld();
    void OnRemoveInRangeObject(Object* pObj);
    void RemoveFromWorld(bool free_guid);

    HEARTHSTONE_INLINE bool CanMine(){return (m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] > 0);}
    HEARTHSTONE_INLINE void UseMine(){ if(m_Go_Uint32Values[GO_UINT32_MINES_REMAINING]) m_Go_Uint32Values[GO_UINT32_MINES_REMAINING]--;}
    void CalcMineRemaining(bool force)
    {
        m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] = 0; // 3.0.9
    }

    uint32 GetGOReqSkill();
    MapCell * m_respawnCell;

    HEARTHSTONE_INLINE void SetScript(GameObjectAIScript *pScript) { myScript = pScript; }

    // loooot
    void GenerateLoot();

//  custom functions for scripting
    void SetState(uint8 state) { SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE, state); }
    uint8 GetState() { return GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE); }
    void SetAnimProgress(uint32 animprogress) { SetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS, animprogress ); }
    uint32 GetAnimProgress() { return GetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS ); }
    void SetFlags(uint32 flags) { SetUInt32Value(GAMEOBJECT_FLAGS, flags ); }
    uint32 GetFlags() { return GetUInt32Value( GAMEOBJECT_FLAGS ); }
    void SetType(uint8 type) { SetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID, type ); }
    uint8 GetType() { return GetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID ); }
    void SetLevel(uint32 level) { SetUInt32Value( GAMEOBJECT_LEVEL, level ); }
    uint32 GetLevel() { return GetUInt32Value( GAMEOBJECT_LEVEL ); }
    void SetDisplayId(uint32 id);
    uint32 GetDisplayId() { return GetUInt32Value( GAMEOBJECT_DISPLAYID ); }

    //Destructable Building
    void TakeDamage(uint32 amount, Object* mcaster, Player* pcaster, uint32 spellid = 0);
    void Destroy();
    void Damage();
    void Rebuild();
    //Aura Generator
    void AuraGenSearchTarget();

    uint32 GetGOui32Value(uint32 id)
    {
        if(id < GO_UINT32_MAX)
            return m_Go_Uint32Values[id];

        return 0;
    };

    void SetGOui32Value(uint32 id, uint32 value)
    {
        if(id < GO_UINT32_MAX)
            m_Go_Uint32Values[id] = value;
    };

    void Use(Player *p);

protected:
    bool m_summonedGo;
    bool m_deleted;
    GameObjectInfo *pInfo;
    GameObjectAIScript * myScript;
    uint32 _fields[GAMEOBJECT_END];
    uint32 m_Go_Uint32Values[GO_UINT32_MAX]; // Crow: We could probably get away with using doubles...
    typedef std::map<uint32,uint64> ChairSlotAndUser;
    ChairSlotAndUser ChairListSlots;
};
