/***
* Demonstrike Core
*/

#pragma once

enum FactionRepFlags
{
    FACTION_FLAG_NONE               = 0x00,
    FACTION_FLAG_VISIBLE            = 0x01,
    FACTION_FLAG_AT_WAR             = 0x02,
    FACTION_FLAG_HIDDEN             = 0x04,
    // Forced interaction
    FACTION_FLAG_INVISIBLE_FORCED   = 0x08,
    FACTION_FLAG_PEACE_FORCED       = 0x10
};

/*
Exalted         1,000    Access to racial mounts. Capped at 999.7
Revered         21,000   Heroic mode keys for Outland dungeons
Honored         12,000   10% discount from faction vendors
Friendly        6,000
Neutral         3,000
Unfriendly      -3,000    Cannot buy, sell or interact.
Hostile         -3,000    You will always be attacked on sight
Hated           -36,000
*/
enum Standing
{
    STANDING_HATED = 0,
    STANDING_HOSTILE,
    STANDING_UNFRIENDLY,
    STANDING_NEUTRAL,
    STANDING_FRIENDLY,
    STANDING_HONORED,
    STANDING_REVERED,
    STANDING_EXALTED
};

struct FactionReputation
{
    uint32 repID;
    int32 standing;
    int32 baseStanding;
    uint8 flag;

    // Force flag functions
    RONIN_INLINE void setVisible() { flag |= FACTION_FLAG_VISIBLE; }
    RONIN_INLINE void setAtWar() { flag |= FACTION_FLAG_AT_WAR; }
    RONIN_INLINE void setAtPeace() { flag &= ~FACTION_FLAG_AT_WAR; }
    RONIN_INLINE void setHidden() { flag |= FACTION_FLAG_HIDDEN; }
    RONIN_INLINE void setForcedInvisibility() { flag |= FACTION_FLAG_INVISIBLE_FORCED; }
    RONIN_INLINE void setForcedPeace() { flag |= FACTION_FLAG_PEACE_FORCED; }

    // Check flag functions
    RONIN_INLINE bool isVisible() { return flag & FACTION_FLAG_VISIBLE; }
    RONIN_INLINE bool isAtWar() { return flag & FACTION_FLAG_AT_WAR; }
    RONIN_INLINE bool isHidden() { return flag & FACTION_FLAG_HIDDEN; }
    RONIN_INLINE bool isInvisibleForced() { return flag & FACTION_FLAG_INVISIBLE_FORCED; }
    RONIN_INLINE bool isPeaceForced() { return flag & FACTION_FLAG_PEACE_FORCED; }

    // Macro functions
    RONIN_INLINE bool blockVisibility() { return isHidden() || isInvisibleForced(); }
    // When sent to client, we need to subtract the base standing and send an offset
    RONIN_INLINE int32 ClientStanding() { return standing - baseStanding; }
    // Current
    RONIN_INLINE int32 CurrentStanding() { return standing; }
    RONIN_INLINE bool Positive() { return CurrentStanding() >= 0; }

    static const int32 minRep = -42000;
    static const int32 maxRep = 42999;

    bool SetStanding(int32 amt);
    bool ModStanding(int32 amt);
};

class SERVER_DECL FactionInterface
{
public:
    FactionInterface(Player *player);
    ~FactionInterface();

    void LoadFactionData(QueryResult *result);
    void SaveFactionData(QueryBuffer *buf);

    // Used for rep data at player login
    void BuildInitialFactions(ByteBuffer *buff);
    // Used for character creation
    void CreateFactionData();

    uint32 GetStanding(uint32 faction);
    uint32 GetBaseStanding(uint32 faction);
    Standing GetStandingRank(uint32 faction);
    bool IsHostileBasedOnReputation(FactionEntry *faction);

    void SetStanding(uint32 faction, int32 mod);
    void ModStanding(uint32 faction, int32 mod);

    bool IsAtWar(uint32 faction);
    void SetAtWar(uint32 faction, uint8 state);

    void Reputation_OnTalk(FactionEntry *faction);
    void Reputation_OnKill(Unit *pVictim);

private:
    Player *m_player;

    void CreateRepData(FactionEntry *faction);
    bool HasReputationData(uint16 repIndex) { return m_reputations.find(repIndex) != m_reputations.end(); }
    FactionReputation *GetReputation(uint16 repIndex) { return &m_reputations[repIndex]; }

    FactionSystem::FactionEntryMap *friendlyFactions, *enemyFactions;
    std::map<uint16, FactionReputation> m_reputations;
};
