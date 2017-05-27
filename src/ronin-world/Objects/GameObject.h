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

class Player;

enum GAMEOBJECT_BYTES
{
    GAMEOBJECT_BYTES_STATE          = 0,
    GAMEOBJECT_BYTES_TYPE_ID        = 1,
    GAMEOBJECT_BYTES_ART_KIT        = 2,
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
    GO_FLAG_TRANSPORT       = 0x008,        // Any kind of transport? object can transport (elevator, boat, car)
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

struct GameObjectData
{
    union
    {
        //0 GAMEOBJECT_TYPE_DOOR
        struct
        {
            uint32 startOpen;                               //0 used client side to determine GO_ACTIVATED means open/closed
            uint32 lockId;                                  //1 -> Lock.dbc
            uint32 autoCloseTime;                           //2 secs till autoclose = autoCloseTime / 0x10000
            uint32 noDamageImmune;                          //3 break opening whenever you recieve damage?
            uint32 openTextID;                              //4 can be used to replace castBarCaption?
            uint32 closeTextID;                             //5
            uint32 ignoredByPathing;                        //6
        } door;
        //1 GAMEOBJECT_TYPE_BUTTON
        struct
        {
            uint32 startOpen;                               //0
            uint32 lockId;                                  //1 -> Lock.dbc
            uint32 autoCloseTime;                           //2 secs till autoclose = autoCloseTime / 0x10000
            uint32 linkedTrap;                              //3
            uint32 noDamageImmune;                          //4 isBattlegroundObject
            uint32 large;                                   //5
            uint32 openTextID;                              //6 can be used to replace castBarCaption?
            uint32 closeTextID;                             //7
            uint32 losOK;                                   //8
        } button;
        //2 GAMEOBJECT_TYPE_QUESTGIVER
        struct
        {
            uint32 lockId;                                  //0 -> Lock.dbc
            uint32 questList;                               //1
            uint32 pageMaterial;                            //2
            uint32 gossipID;                                //3
            uint32 customAnim;                              //4
            uint32 noDamageImmune;                          //5
            uint32 openTextID;                              //6 can be used to replace castBarCaption?
            uint32 losOK;                                   //7
            uint32 allowMounted;                            //8 Is usable while on mount/vehicle. (0/1)
            uint32 large;                                   //9
        } questgiver;
        //3 GAMEOBJECT_TYPE_CHEST
        struct
        {
            uint32 lockId;                                  //0 -> Lock.dbc
            uint32 lootId;                                  //1
            uint32 chestRestockTime;                        //2
            uint32 consumable;                              //3
            uint32 minSuccessOpens;                         //4 Deprecated, pre 3.0 was used for mining nodes but since WotLK all mining nodes are usable once and grant all loot with a single use
            uint32 maxSuccessOpens;                         //5 Deprecated, pre 3.0 was used for mining nodes but since WotLK all mining nodes are usable once and grant all loot with a single use
            uint32 eventId;                                 //6 lootedEvent
            uint32 linkedTrapId;                            //7
            uint32 questId;                                 //8 not used currently but store quest required for GO activation for player
            uint32 level;                                   //9
            uint32 losOK;                                   //10
            uint32 leaveLoot;                               //11
            uint32 notInCombat;                             //12
            uint32 logLoot;                                 //13
            uint32 openTextID;                              //14 can be used to replace castBarCaption?
            uint32 groupLootRules;                          //15
            uint32 floatingTooltip;                         //16
        } chest;
        //4 GAMEOBJECT_TYPE_BINDER - empty
        //5 GAMEOBJECT_TYPE_GENERIC
        struct
        {
            uint32 floatingTooltip;                         //0
            uint32 highlight;                               //1
            uint32 serverOnly;                              //2
            uint32 large;                                   //3
            uint32 floatOnWater;                            //4
            int32 questID;                                  //5
        } _generic;
        //6 GAMEOBJECT_TYPE_TRAP
        struct
        {
            uint32 lockId;                                  //0 -> Lock.dbc
            uint32 level;                                   //1
            uint32 radius;                                  //2 diameter for trap activation
            uint32 spellId;                                 //3
            uint32 type;                                    //4 0 trap with no despawn after cast. 1 trap despawns after cast. 2 bomb casts on spawn.
            uint32 cooldown;                                //5 time in secs
            int32 autoCloseTime;                            //6
            uint32 startDelay;                              //7
            uint32 serverOnly;                              //8
            uint32 stealthed;                               //9
            uint32 large;                                   //10
            uint32 invisible;                               //11
            uint32 openTextID;                              //12 can be used to replace castBarCaption?
            uint32 closeTextID;                             //13
            uint32 ignoreTotems;                            //14
        } trap;
        //7 GAMEOBJECT_TYPE_CHAIR
        struct
        {
            uint32 slots;                                   //0
            uint32 height;                                  //1
            uint32 onlyCreatorUse;                          //2
            uint32 triggeredEvent;                          //3
        } chair;
        //8 GAMEOBJECT_TYPE_SPELL_FOCUS
        struct
        {
            uint32 focusId;                                 //0
            uint32 dist;                                    //1
            uint32 linkedTrapId;                            //2
            uint32 serverOnly;                              //3
            uint32 questID;                                 //4
            uint32 large;                                   //5
            uint32 floatingTooltip;                         //6
        } spellFocus;
        //9 GAMEOBJECT_TYPE_TEXT
        struct
        {
            uint32 pageID;                                  //0
            uint32 language;                                //1
            uint32 pageMaterial;                            //2
            uint32 allowMounted;                            //3 Is usable while on mount/vehicle. (0/1)
        } text;
        //10 GAMEOBJECT_TYPE_GOOBER
        struct
        {
            uint32 lockId;                                  //0 -> Lock.dbc
            int32 questId;                                  //1
            uint32 eventId;                                 //2
            uint32 autoCloseTime;                           //3
            uint32 customAnim;                              //4
            uint32 consumable;                              //5
            uint32 cooldown;                                //6
            uint32 pageId;                                  //7
            uint32 language;                                //8
            uint32 pageMaterial;                            //9
            uint32 spellId;                                 //10
            uint32 noDamageImmune;                          //11
            uint32 linkedTrapId;                            //12
            uint32 large;                                   //13
            uint32 openTextID;                              //14 can be used to replace castBarCaption?
            uint32 closeTextID;                             //15
            uint32 losOK;                                   //16 isBattlegroundObject
            uint32 allowMounted;                            //17 Is usable while on mount/vehicle. (0/1)
            uint32 floatingTooltip;                         //18
            uint32 gossipID;                                //19
            uint32 WorldStateSetsState;                     //20
        } goober;
        //11 GAMEOBJECT_TYPE_TRANSPORT
        struct
        {
            int32 stopFrame1;                               //0
            uint32 startOpen;                               //1
            uint32 autoCloseTime;                           //2 secs till autoclose = autoCloseTime / 0x10000
            uint32 pause1EventID;                           //3
            uint32 pause2EventID;                           //4
            uint32 mapId;                                   //5
            int32 stopFrame2;                               //6
            uint32 unknown;
            int32 stopFrame3;                               //8
            uint32 unknown2;
            int32 stopFrame4;                               //10
        } transport;
        //12 GAMEOBJECT_TYPE_AREADAMAGE
        struct
        {
            uint32 lockId;                                  //0
            uint32 radius;                                  //1
            uint32 damageMin;                               //2
            uint32 damageMax;                               //3
            uint32 damageSchool;                            //4
            uint32 autoCloseTime;                           //5 secs till autoclose = autoCloseTime / 0x10000
            uint32 openTextID;                              //6
            uint32 closeTextID;                             //7
        } areadamage;
        //13 GAMEOBJECT_TYPE_CAMERA
        struct
        {
            uint32 lockId;                                  //0 -> Lock.dbc
            uint32 cinematicId;                             //1
            uint32 eventID;                                 //2
            uint32 openTextID;                              //3 can be used to replace castBarCaption?
        } camera;
        //14 GAMEOBJECT_TYPE_MAPOBJECT - empty
        //15 GAMEOBJECT_TYPE_MO_TRANSPORT
        struct
        {
            uint32 taxiPathId;                              //0
            uint32 moveSpeed;                               //1
            uint32 accelRate;                               //2
            uint32 startEventID;                            //3
            uint32 stopEventID;                             //4
            uint32 transportPhysics;                        //5
            uint32 mapID;                                   //6
            uint32 worldState1;                             //7
            uint32 canBeStopped;                            //8
        } moTransport;
        //16 GAMEOBJECT_TYPE_DUELFLAG - empty
        //17 GAMEOBJECT_TYPE_FISHINGNODE - empty
        //18 GAMEOBJECT_TYPE_SUMMONING_RITUAL
        struct
        {
            uint32 reqParticipants;                         //0
            uint32 spellId;                                 //1
            uint32 animSpell;                               //2
            uint32 ritualPersistent;                        //3
            uint32 casterTargetSpell;                       //4
            uint32 casterTargetSpellTargets;                //5
            uint32 castersGrouped;                          //6
            uint32 ritualNoTargetCheck;                     //7
        } ritual;
        //19 GAMEOBJECT_TYPE_MAILBOX - empty
        //20 GAMEOBJECT_TYPE_DONOTUSE - empty
        //21 GAMEOBJECT_TYPE_GUARDPOST
        struct
        {
            uint32 creatureID;                              //0
            uint32 charges;                                 //1
        } guardpost;
        //22 GAMEOBJECT_TYPE_SPELLCASTER
        struct
        {
            uint32 spellId;                                 //0
            uint32 charges;                                 //1
            uint32 partyOnly;                               //2
            uint32 allowMounted;                            //3 Is usable while on mount/vehicle. (0/1)
            uint32 large;                                   //4
        } spellcaster;
        //23 GAMEOBJECT_TYPE_MEETINGSTONE
        struct
        {
            uint32 minLevel;                                //0
            uint32 maxLevel;                                //1
            uint32 areaID;                                  //2
        } meetingstone;
        //24 GAMEOBJECT_TYPE_FLAGSTAND
        struct
        {
            uint32 lockId;                                  //0
            uint32 pickupSpell;                             //1
            uint32 radius;                                  //2
            uint32 returnAura;                              //3
            uint32 returnSpell;                             //4
            uint32 noDamageImmune;                          //5
            uint32 openTextID;                              //6
            uint32 losOK;                                   //7
        } flagstand;
        //25 GAMEOBJECT_TYPE_FISHINGHOLE
        struct
        {
            uint32 radius;                                  //0 how close bobber must land for sending loot
            uint32 lootId;                                  //1
            uint32 minSuccessOpens;                         //2
            uint32 maxSuccessOpens;                         //3
            uint32 lockId;                                  //4 -> Lock.dbc; possibly 1628 for all?
        } fishinghole;
        //26 GAMEOBJECT_TYPE_FLAGDROP
        struct
        {
            uint32 lockId;                                  //0
            uint32 eventID;                                 //1
            uint32 pickupSpell;                             //2
            uint32 noDamageImmune;                          //3
            uint32 openTextID;                              //4
        } flagdrop;
        //27 GAMEOBJECT_TYPE_MINI_GAME
        struct
        {
            uint32 gameType;                                //0
        } miniGame;
        //29 GAMEOBJECT_TYPE_CAPTURE_POINT
        struct
        {
            uint32 radius;                                  //0
            uint32 spell;                                   //1
            uint32 worldState1;                             //2
            uint32 worldstate2;                             //3
            uint32 winEventID1;                             //4
            uint32 winEventID2;                             //5
            uint32 contestedEventID1;                       //6
            uint32 contestedEventID2;                       //7
            uint32 progressEventID1;                        //8
            uint32 progressEventID2;                        //9
            uint32 neutralEventID1;                         //10
            uint32 neutralEventID2;                         //11
            uint32 neutralPercent;                          //12
            uint32 worldstate3;                             //13
            uint32 minSuperiority;                          //14
            uint32 maxSuperiority;                          //15
            uint32 minTime;                                 //16
            uint32 maxTime;                                 //17
            uint32 large;                                   //18
            uint32 highlight;                               //19
            uint32 startingValue;                           //20
            uint32 unidirectional;                          //21
        } capturePoint;
        //30 GAMEOBJECT_TYPE_AURA_GENERATOR
        struct
        {
            uint32 startOpen;                               //0
            uint32 radius;                                  //1
            uint32 auraID1;                                 //2
            uint32 conditionID1;                            //3
            uint32 auraID2;                                 //4
            uint32 conditionID2;                            //5
            uint32 serverOnly;                              //6
        } auraGenerator;
        //31 GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY
        struct
        {
            uint32 mapID;                                   //0
            uint32 difficulty;                              //1
        } dungeonDifficulty;
        //32 GAMEOBJECT_TYPE_BARBER_CHAIR
        struct
        {
            uint32 chairheight;                             //0
            uint32 heightOffset;                            //1
        } barberChair;
        //33 GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
        struct
        {
            uint32 intactNumHits;                           //0
            uint32 creditProxyCreature;                     //1
            uint32 state1Name;                              //2
            uint32 intactEvent;                             //3
            uint32 damagedDisplayId;                        //4
            uint32 damagedNumHits;                          //5
            uint32 empty3;                                  //6
            uint32 empty4;                                  //7
            uint32 empty5;                                  //8
            uint32 damagedEvent;                            //9
            uint32 destroyedDisplayId;                      //10
            uint32 empty7;                                  //11
            uint32 empty8;                                  //12
            uint32 empty9;                                  //13
            uint32 destroyedEvent;                          //14
            uint32 empty10;                                 //15
            uint32 debuildingTimeSecs;                      //16
            uint32 empty11;                                 //17
            uint32 destructibleData;                        //18
            uint32 rebuildingEvent;                         //19
            uint32 empty12;                                 //20
            uint32 empty13;                                 //21
            uint32 damageEvent;                             //22
            uint32 empty14;                                 //23
        } building;
        //34 GAMEOBJECT_TYPE_GUILDBANK - empty
        //35 GAMEOBJECT_TYPE_TRAPDOOR
        struct
        {
            uint32 whenToPause;                             // 0
            uint32 startOpen;                               // 1
            uint32 autoClose;                               // 2
        } trapDoor;

        // not use for specific field access (only for output with loop by all filed), also this determinate max union size
        struct { uint32 data[32]; } raw;
    };
};

struct GameObjectInfo
{
    uint32 ID;
    uint32 Type;
    uint32 DisplayID;
    char * Name;
    char * Icon;
    char * CastBarText;
    float sizeMod;
    uint32 questItems[6];
    GameObjectData data;
    int32 RespawnTimer;
    uint32 DefaultFlags;

    uint32 GetSpellID()
    {
        uint32 spellid = 0;
        switch(Type)
        {
        case GAMEOBJECT_TYPE_TRAP: spellid = data.trap.spellId; break;
        case GAMEOBJECT_TYPE_GOOBER: spellid = data.goober.spellId; break;
        case GAMEOBJECT_TYPE_RITUAL: spellid = data.ritual.spellId; break;
        case GAMEOBJECT_TYPE_SPELLCASTER: spellid = data.spellcaster.spellId; break;
        case GAMEOBJECT_TYPE_CAPTURE_POINT: spellid = data.capturePoint.spell; break;
        }
        return spellid;
    }

    uint32 GetLockID()
    {
        uint32 lockid = 0;
        switch(Type)
        {
        case GAMEOBJECT_TYPE_DOOR: lockid = data.door.lockId; break;
        case GAMEOBJECT_TYPE_BUTTON: lockid = data.button.lockId; break;
        case GAMEOBJECT_TYPE_QUESTGIVER: lockid = data.questgiver.lockId; break;
        case GAMEOBJECT_TYPE_CHEST: lockid = data.chest.lockId; break;
        case GAMEOBJECT_TYPE_TRAP: lockid = data.trap.lockId; break;
        case GAMEOBJECT_TYPE_GOOBER: lockid = data.goober.lockId; break;
        case GAMEOBJECT_TYPE_AREADAMAGE: lockid = data.areadamage.lockId; break;
        case GAMEOBJECT_TYPE_CAMERA: lockid = data.camera.lockId; break;
        case GAMEOBJECT_TYPE_FLAGSTAND: lockid = data.flagstand.lockId; break;
        case GAMEOBJECT_TYPE_FISHINGHOLE: lockid = data.fishinghole.lockId; break;
        case GAMEOBJECT_TYPE_FLAGDROP: lockid = data.flagdrop.lockId; break;
        }
        return lockid;
    }

    uint32 GetSequenceTimer()
    {
        uint32 timer = 2;
        switch(Type)
        {
        case GAMEOBJECT_TYPE_TRAP:          timer = (data.trap.cooldown ? data.trap.cooldown : 4) * 1000; break;
        case GAMEOBJECT_TYPE_GOOBER:        timer = (data.goober.cooldown ? data.goober.cooldown : 4) * 1000; break;
        case GAMEOBJECT_TYPE_AREADAMAGE:    timer = data.areadamage.autoCloseTime; break;
        }
        return std::max<uint32>(1000, timer);
    }
};

#pragma pack(PRAGMA_POP)

#define TRIGGER_GO_EVENT(obj, func)

class SERVER_DECL GameObject : public WorldObject
{
    struct ObjectRotation
    {
        float x, y, z, w;

        float toAxisAngleRotation() const
        {
            // Decompose the quaternion into an angle
            float modifier = 2 * acosf(w);
            return z > 0 ? modifier : 6.28318531-modifier;
        }
    };

public:
    GameObject(GameObjectInfo *info, WoWGuid guid, uint32 fieldCount = GAMEOBJECT_END);
    ~GameObject( );

    virtual void Init();
    virtual void Destruct();

    virtual void Update(uint32 msTime, uint32 p_time);
    virtual void OnFieldUpdated(uint16 index);

    virtual bool IsGameObject() { return true; }

    virtual bool IsActiveObject() { return true; }
    virtual uint32 getEventID() { return m_spawn ? m_spawn->eventId : 0; }
    virtual uint32 getConditionID() { return m_spawn ? m_spawn->conditionId : 0; }

    virtual void OnPrePushToWorld();
    virtual void RemoveFromWorld();

    virtual void Reactivate();

    RONIN_INLINE uint8 GetGameObjectPool() { return m_gameobjectPool; }
    RONIN_INLINE void AssignGameObjectPool(uint8 pool) { m_gameobjectPool = pool; }

    RONIN_INLINE GameObjectInfo* GetInfo() { return pInfo; }
    RONIN_INLINE void SetInfo(GameObjectInfo * goi) { pInfo = goi; }

    void Load(uint32 mapId, float x, float y, float z, float angleOverride = 0.f, float rX = 0.f, float rY = 0.f, float rZ = 0.f, float rAngle = 0.f, GameObjectSpawn *spawn = NULL);

    uint32 BuildStopFrameData(ByteBuffer *buff);
    void UpdateRotations(float rotX, float rotY, float rotZ, float rotAngle);
    static int64 PackRotation(GameObject::ObjectRotation *rotation);

    // Serialization
    void SaveToDB();
    void DeleteFromDB();
    void EventCloseDoor();

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

    Unit *GetSummoner() { return m_summoner; }

    Unit* CreateTemporaryGuardian(uint32 guardian_entry,uint32 duration,float angle, Unit* u_caster, uint8 Slot);

    /// Quest data
    std::list<QuestRelation *>* m_quests;

    uint32 *m_ritualmembers;
    ObjectRotation m_rotation;

    void InitAI();

    bool m_created;
    uint16 counter;
    int32 charges;//used for type==22,to limit number of usages.
    bool invisible;//invisible
    uint8 invisibilityFlag;
    Unit* m_summoner;
    int8 bannerslot;
    int8 bannerauraslot;

    RONIN_INLINE bool HasAI() { return m_triggerSpell != 0; }
    GameObjectSpawn * m_spawn;

    RONIN_INLINE bool CanMine(){return (m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] > 0);}
    RONIN_INLINE void UseMine(){ if(m_Go_Uint32Values[GO_UINT32_MINES_REMAINING]) m_Go_Uint32Values[GO_UINT32_MINES_REMAINING]--;}
    void CalcMineRemaining(bool force)
    {
        m_Go_Uint32Values[GO_UINT32_MINES_REMAINING] = 0; // 3.0.9
    }

    uint32 GetGOReqSkill();

    // loooot
    void GenerateLoot();

    RONIN_INLINE void SetState(uint8 state);
    RONIN_INLINE uint8 GetState() { return GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE); }
    RONIN_INLINE void SetArtKit(uint8 kit) { SetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ART_KIT, kit); }
    RONIN_INLINE uint8 GetArtKit() { return GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ART_KIT); }
    RONIN_INLINE void SetAnimProgress(uint32 animprogress) { SetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS, animprogress ); }
    RONIN_INLINE uint32 GetAnimProgress() { return GetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_ANIMPROGRESS ); }
    RONIN_INLINE void SetFlags(uint32 flags) { SetUInt32Value(GAMEOBJECT_FLAGS, flags ); }
    RONIN_INLINE uint32 GetFlags() { return GetUInt32Value( GAMEOBJECT_FLAGS ); }
    RONIN_INLINE void SetType(uint8 type) { SetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID, type ); }
    RONIN_INLINE uint8 GetType() { return GetByte( GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID ); }
    RONIN_INLINE void SetLevel(uint32 level) { SetUInt32Value( GAMEOBJECT_LEVEL, level ); }
    RONIN_INLINE uint32 getLevel() { return GetUInt32Value( GAMEOBJECT_LEVEL ); }

    void SetDisplayId(uint32 id);
    RONIN_INLINE uint32 GetDisplayId() { return GetUInt32Value( GAMEOBJECT_DISPLAYID ); }

    RONIN_INLINE bool isQuestGiver() { return GetType() == GAMEOBJECT_TYPE_QUESTGIVER; }

    //Destructable Building
    void TakeDamage(uint32 amount, WorldObject* mcaster, Player* pcaster, uint32 spellid = 0);
    void SetStatusDestroyed();
    void SetStatusDamaged();
    void SetStatusRebuilt();

    RONIN_INLINE uint32 GetGOui32Value(uint32 id)
    {
        if(id < GO_UINT32_MAX)
            return m_Go_Uint32Values[id];

        return 0;
    }

    RONIN_INLINE void SetGOui32Value(uint32 id, uint32 value)
    {
        if(id < GO_UINT32_MAX)
            m_Go_Uint32Values[id] = value;
    }

    void Use(Player *p);

protected:
    bool m_summonedGo, m_deleted;
    GameObjectInfo *pInfo;
    uint8 m_gameobjectPool;
    uint32 m_Go_Uint32Values[GO_UINT32_MAX];

    float m_triggerRange;
    SpellEntry* m_triggerSpell;

    bool m_zoneVisibleSpawn;

    struct seatData
    {
        WoWGuid user;
        float x, y, z;
    };
    typedef std::map<uint32, seatData> ChairSlotAndUser;

    ChairSlotAndUser m_chairData;
    void _recalculateChairSeats();

    std::set<WoWGuid> m_inTriggerRangeObjects;

    void _searchNearbyUnits();
};
