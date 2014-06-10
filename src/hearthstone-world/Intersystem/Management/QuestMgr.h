/***
 * Demonstrike Core
 */

#pragma once

#define QUEST_LOG_COUNT 50

enum RepeatableTypes
{
    REPEATABLE_QUEST    = 1,
    REPEATABLE_DAILY    = 2,
};

struct QuestRelation
{
    Quest *qst;
    uint8 type;
};

struct QuestAssociation
{
    Quest *qst;
    uint8 item_count;
};

class Item;

typedef std::map<uint32, Quest*> QuestStorageMap;
typedef std::list<QuestRelation *> QuestRelationList;
typedef std::list<QuestAssociation *> QuestAssociationList;

class SERVER_DECL QuestMgr :  public Singleton < QuestMgr >
{
public:
    QuestMgr();
    ~QuestMgr();

    Mutex LoadLocks;
    void LoadQuests(); // Actually load our quests, do not call outside startup.
    void LoadExtraQuestStuff();
    bool QuestExists(uint32 entry) { if(QuestStorage.find(entry) != QuestStorage.end()) return true; return false; };
    Quest* GetQuestPointer(uint32 entry) { if(QuestStorage.find(entry) != QuestStorage.end()) return QuestStorage.at(entry); return NULL; };

    QuestStorageMap::iterator GetQuestStorageBegin() { return QuestStorage.begin(); };
    QuestStorageMap::iterator GetQuestStorageEnd() { return QuestStorage.end(); };

    uint32 PlayerMeetsReqs(Player* plr, Quest* qst, bool skiplevelcheck);

    uint32 CalcStatus(Object* quest_giver, Player* plr);
    uint32 CalcQuestStatus(Player* plr, QuestRelation* qst);
    uint32 CalcQuestStatus(Player* plr, Quest* qst, uint8 type, bool skiplevelcheck);
    uint32 ActiveQuestsCount(Object* quest_giver, Player* plr);

    //Packet Forging...
    void BuildOfferReward(WorldPacket* data,Quest* qst, Object* qst_giver, uint32 menutype, Player* plr);
    void BuildQuestDetails(WorldPacket* data, Quest* qst, Object* qst_giver, uint32 menutype, Player* plr);
    void BuildRequestItems(WorldPacket* data, Quest* qst, Object* qst_giver, uint32 status);
    void BuildQuestComplete(Player* , Quest* qst);
    void BuildQuestList(WorldPacket* data, Object* qst_giver, Player* plr);
    bool OnActivateQuestGiver(Object* qst_giver, Player* plr);

    void SendQuestUpdateAddKill(Player* plr, uint32 questid, uint32 entry, uint32 count, uint32 tcount, uint64 guid);
    void BuildQuestUpdateAddItem(WorldPacket* data, uint32 itemid, uint32 count);
    void BuildQuestUpdateComplete(WorldPacket* data, Quest* qst);
    void BuildQuestFailed(WorldPacket* data, uint32 questid);
    void SendPushToPartyResponse(Player* plr, Player* pTarget, uint32 response);

    bool OnGameObjectActivate(Player* plr, GameObject* go);
    void _OnPlayerKill(Player* plr, uint32 creature_entry);
    void OnPlayerKill(Player* plr, Creature* victim);
    void OnPlayerCast(Player* plr, uint32 spellid, uint64& victimguid);
    void OnPlayerItemPickup(Player* plr, Item* item, uint32 pickedupstacksize);
    void OnPlayerDropItem(Player* plr, uint32 entry);
    void OnPlayerExploreArea(Player* plr, uint32 AreaID);
    void OnPlayerAreaTrigger(Player* plr, uint32 areaTrigger);
    void OnPlayerSlain(Player* plr, Player* victim);
    void OnQuestAccepted(Player* plr, Quest* qst, Object* qst_giver);
    void OnQuestFinished(Player* plr, Quest* qst, Object* qst_giver, uint32 reward_slot);
    bool SkippedKills( uint32 QuestID );

    void GiveQuestRewardReputation(Player* plr, Quest* qst, Object* qst_giver);
    void GiveQuestTitleReward(Player* plr, Quest* qst);

    uint32 GenerateQuestXP(Player* pl, Quest *qst);
    uint32 GenerateRewardMoney(Player* pl, Quest *qst);
    uint32 GenerateMaxLvlRewardMoney(Player* pl, Quest *qst);

    void SendQuestFailed(FAILED_REASON failed, Quest *qst, Player* plyr);
    void SendQuestUpdateFailed(Quest *pQuest, Player* plyr);
    void SendQuestUpdateFailedTimer(Quest *pQuest, Player* plyr);
    void SendQuestLogFull(Player* plyr);

    void LoadNPCQuests(Creature* qst_giver);
    void LoadGOQuests(GameObject* go);

    QuestRelationList* GetCreatureQuestList(uint32 entryid);
    QuestRelationList* GetGOQuestList(uint32 entryid);
    QuestAssociationList* GetQuestAssociationListForItemId (uint32 itemId);

    HEARTHSTONE_INLINE bool IsQuestRepeatable(Quest *qst) { return (qst->qst_is_repeatable != 0 ? true : false); }
    HEARTHSTONE_INLINE bool IsQuestDaily(Quest *qst) { return (qst->qst_is_repeatable == 2 ? true : false); }

    bool CanStoreReward(Player* plyr, Quest *qst, uint32 reward_slot);

    HEARTHSTONE_INLINE int32 QuestHasMob(Quest* qst, uint32 mob)
    {
        for(uint32 i = 0; i < 4; i++)
            if(qst->required_mob[i] == mob)
                return qst->required_mobcount[i];
        return -1;
    }

    HEARTHSTONE_INLINE int32 GetOffsetForMob(Quest *qst, uint32 mob)
    {
        for(uint32 i = 0; i < 4; i++)
            if(qst->required_mob[i] == mob)
                return i;
        return -1;
    }

    HEARTHSTONE_INLINE int32 GetOffsetForItem(Quest *qst, uint32 itm)
    {
        for(uint32 i = 0; i < 6; i++)
            if(qst->required_item[i] == itm)
                return i;
        return -1;
    }

private:
    QuestStorageMap QuestStorage;

    HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* > m_npc_quests;
    HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* > m_obj_quests;
    HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* > m_itm_quests;
    list<uint32> m_extraqueststuff_list;

    HM_NAMESPACE::hash_map<uint32, list<QuestAssociation *>* > m_quest_associations;
    HM_NAMESPACE::hash_map<uint32, list<QuestAssociation *>* >& GetQuestAssociationList(){return m_quest_associations;}

    HM_NAMESPACE::hash_map<uint32, uint32>        m_ObjectLootQuestList;

    template <class T> void _AddQuest(uint32 entryid, Quest *qst, uint8 type);

    template <class T> HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* >& _GetList();

    void AddItemQuestAssociation( uint32 itemId, Quest *qst, uint8 item_count);

    // Quest Loading
    void _RemoveChar(char* c, std::string *str);
    void _CleanLine(std::string *str);
};

template<> HEARTHSTONE_INLINE HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* >& QuestMgr::_GetList<Creature>()
    {return m_npc_quests;}
template<> HEARTHSTONE_INLINE HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* >& QuestMgr::_GetList<GameObject>()
    {return m_obj_quests;}
template<> HEARTHSTONE_INLINE HM_NAMESPACE::hash_map<uint32, list<QuestRelation *>* >& QuestMgr::_GetList<Item>()
    {return m_itm_quests;}


#define sQuestMgr QuestMgr::getSingleton()
