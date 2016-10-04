
#pragma once

#define PLAYER_ACTION_BUTTON_COUNT 144

#define MAX_SPEC_COUNT 2
#define GLYPHS_COUNT 9
#define REQ_PRIMARY_TREE_TALENTS 31

enum ActionButtonType
{
    ACTION_BUTTON_SPELL     = 0x00,
    ACTION_BUTTON_C         = 0x01,
    ACTION_BUTTON_EQSET     = 0x20,
    ACTION_BUTTON_EXPANDABLE= 0x30,
    ACTION_BUTTON_MACRO     = 0x40,
    ACTION_BUTTON_CMACRO    = ACTION_BUTTON_C | ACTION_BUTTON_MACRO,
    ACTION_BUTTON_ITEM      = 0x80
};

#define ACTION_BUTTON_ACTION(X) (uint32(X) & 0x00FFFFFF)
#define ACTION_BUTTON_TYPE(X) ((uint32(X) & 0xFF000000) >> 24)
#define ACTION_PACK_BUTTION(action, type) uint32(action | (uint32(type) << 24))
#define MAX_ACTION_BUTTON_ACTION_VALUE (0x00FFFFFF+1)

struct ActionButton
{
    uint32 PackedData;
    uint8 GetType() { return ACTION_BUTTON_TYPE(PackedData); };
    uint32 GetAction() { return ACTION_BUTTON_ACTION(PackedData); };
};

typedef std::map<uint32, uint8> TalentStorageMap;

class SERVER_DECL TalentInterface
{
public:
    TalentInterface(Player *plr);
    ~TalentInterface();

    void SaveTalentData(QueryBuffer * buf);
    void LoadTalentData(QueryResult *result);
    void SetTalentData(uint8 activeSpec, uint8 specCount, uint32 resetCounter, int32 bonusTalentPoints, uint32 activeSpecStack);

    void InitActiveSpec();
    void SendTalentInfo();

    void BuildPlayerTalentInfo(WorldPacket *data);
    void BuildPlayerActionInfo(WorldPacket *data);

    void ModTalentPoints(int32 talentPoints);

    // Specs
    void ResetAllSpecs();
    void ResetSpec(uint8 spec, bool silent=false);

    void UnlockSpec(uint8 spec);
    void ApplySpec(uint8 spec);

    void RecalculateAvailableTalentPoints();
    int32 CalculateSpentPoints(uint8 spec, int32 talentTree = -1);

    uint8 GetActiveTalentTab() { return m_specs[m_activeSpec].ActiveTalentTab; }
    void SetActiveTalentTab(uint8 talentTree) { m_specs[m_activeSpec].ActiveTalentTab = talentTree; InitActiveSpec(); }
    void GetActiveTalentTabStack(uint32 &output);

    uint8 GetSpecCount() { return m_specCount; }
    uint8 GetActiveSpec() { return m_activeSpec; }

    // Talents
    void ApplyTalent(uint32 spellId);
    void RemoveTalent(uint32 spellid);

    bool HasTalent(uint32 talentId, int8 talentRank);
    bool LearnTalent(uint32 talentId, uint8 talentRank);

    uint32 GetTalentResets() { return m_talentResetCounter; }
    int32 GetBonusTalentPoints() { return m_bonusTalentPoints; }

    // Talent map
    TalentStorageMap *getTalentMap() { return &m_specs[m_activeSpec].m_talents; }

    // Glyphs
    void SaveGlyphData(QueryBuffer * buf);
    void LoadGlyphData(QueryResult *result);

    void InitGlyphSlots();
    void InitGlyphsForLevel(uint32 level);

    uint8 ApplyGlyph(uint8 glyphSlot, uint32 glyphID);
    void UnapplyGlyph(uint8 glyphSlot);

    uint32 GetGlyphID(uint8 glyphSlot)
    {
        ASSERT(glyphSlot < GLYPHS_COUNT);
        return m_specs[m_activeSpec].Glyphs[glyphSlot];
    }

    // Action button mapping
    void SaveActionButtonData(QueryBuffer * buf);
    void LoadActionButtonData(QueryResult *result);

    void setAction(uint8 button, uint32 action, uint8 type, int8 SpecOverride = -1);
    void SendInitialActions();

    ActionButton GetActionButton(uint32 button)
    {
        ASSERT(button < PLAYER_ACTION_BUTTON_COUNT);
        return m_specs[m_activeSpec].m_actions[button];
    }

private:
    Player *m_Player;

    // Talent specs
    uint32 m_talentResetCounter;
    uint8 m_activeSpec, m_specCount;
    struct PlayerSpecs
    {
        uint8 ActiveTalentTab;
        TalentStorageMap m_talents; // <TalentID><TalentRank>

        uint16 Glyphs[GLYPHS_COUNT];
        ActionButton m_actions[PLAYER_ACTION_BUTTON_COUNT];
    }m_specs[MAX_SPEC_COUNT];
    Mutex m_specLock;

    // Talent points
    uint32 m_currentTalentPoints, m_availableTalentPoints;
    int32 m_bonusTalentPoints;
};
