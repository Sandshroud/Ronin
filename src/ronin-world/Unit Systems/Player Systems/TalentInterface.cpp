
#include "StdAfx.h"

TalentInterface::TalentInterface(Player *plr) : m_Player(plr)
{
    m_specCount = 1;
    m_activeSpec = 0;
    m_talentResetCounter = m_availableTalentPoints = m_bonusTalentPoints = 0;
    m_specs[0].ActiveTalentTab = m_specs[1].ActiveTalentTab = 0xFF;
    memset(&m_specs[0].Glyphs, 0, sizeof(uint16)*GLYPHS_COUNT);
    memset(&m_specs[1].Glyphs, 0, sizeof(uint16)*GLYPHS_COUNT);
    memset(&m_specs[0].m_actions, 0, PLAYER_ACTION_BUTTON_COUNT*sizeof(ActionButton));
    memset(&m_specs[1].m_actions, 0, PLAYER_ACTION_BUTTON_COUNT*sizeof(ActionButton));
}

TalentInterface::~TalentInterface()
{

}

void TalentInterface::SaveTalentData(QueryBuffer * buf)
{
    ASSERT(m_specCount > 0);

    // delete old talents first
    if(buf == NULL)
        CharacterDatabase.Execute("DELETE FROM character_talents WHERE guid = %u", m_Player->GetLowGUID() );
    else buf->AddQuery("DELETE FROM character_talents WHERE guid = %u", m_Player->GetLowGUID() );

    m_specLock.Acquire();
    std::stringstream ss;
    for(uint8 s = 0; s < m_specCount; s++)
    {
        TalentStorageMap *talents = &m_specs[s].m_talents;
        if(talents->size())
        {
            for(std::map<uint32, uint8>::iterator itr = talents->begin(); itr != talents->end(); itr++)
            {
                if(ss.str().length())
                    ss << ", ";
                ss << "(" << m_Player->GetLowGUID() << "," << uint32(s) << "," << itr->first << "," << uint32(itr->second) << ")";
            }
        }
    }

    m_specLock.Release();
    if(ss.str().empty())
        return;

    if(buf == NULL)
        CharacterDatabase.Execute("INSERT INTO character_talents VALUES %s", ss.str().c_str());
    else buf->AddQuery("INSERT INTO character_talents VALUES %s", ss.str().c_str());
}

void TalentInterface::LoadTalentData(QueryResult *result)
{
    if(result)
    {
        do // Load info from DB
        {
            uint32 talentId;
            Field *fields = result->Fetch();
            uint8 talentRank = 0, spec = fields[1].GetInt8();
            if(spec >= m_specCount)
            {
                sLog.outDebug("Out of range spec number [%d] for player with GUID [%d] in character_talents", spec, m_Player->GetLowGUID());
                continue;
            }

            talentId = fields[2].GetUInt32();
            talentRank = fields[3].GetUInt8();
            m_specs[spec].m_talents.insert(std::make_pair(talentId, talentRank));
        } while(result->NextRow());
    }
}

void TalentInterface::SetTalentData(uint8 activeSpec, uint8 specCount, uint32 resetCounter, int32 bonusTalentPoints, uint32 activeSpecStack)
{
    m_activeSpec = activeSpec, m_specCount = specCount;
    if(m_specCount > MAX_SPEC_COUNT) m_specCount = MAX_SPEC_COUNT;
    if(m_activeSpec >= m_specCount ) m_activeSpec = 0;
    m_talentResetCounter = resetCounter;
    m_bonusTalentPoints = bonusTalentPoints;
    m_specs[0].ActiveTalentTab = uint8(activeSpecStack&0xFF);
    m_specs[1].ActiveTalentTab = uint8(activeSpecStack>>8);
}

void TalentInterface::InitActiveSpec()
{
    m_availableTalentPoints = dbcNumTalents.LookupEntry(std::min<uint32>(MAXIMUM_ATTAINABLE_LEVEL, m_Player->getLevel()))->talentPoints;

    // We could use the pre-created function but we need to apply the talents
    uint32 spentPoints = 0;//CalculateSpentPoints(m_activeSpec);
    for(TalentStorageMap::iterator itr = m_specs[m_activeSpec].m_talents.begin(); itr != m_specs[m_activeSpec].m_talents.end(); itr++)
    {
        spentPoints++;
        TalentEntry * talentInfo = dbcTalent.LookupEntry(itr->first);
        if(talentInfo == NULL || itr->second > 4)
            continue;

        ApplyTalent(talentInfo->RankID[itr->second]);
        spentPoints += itr->second;
    }

    m_currentTalentPoints = std::max<int32>(0, m_availableTalentPoints+m_bonusTalentPoints);
    if(m_currentTalentPoints <= spentPoints)
        m_currentTalentPoints = 0;
    else m_currentTalentPoints -= spentPoints;

    // Add our spec primary spells
    StatSystem::TalentSpellPair talentPair = sStatSystem.GetTalentPrimarySpells(m_Player->getClass(), m_specs[m_activeSpec].ActiveTalentTab);
    if(talentPair.first != talentPair.second)
        for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
            m_Player->addSpell(itr->second);

    // Add our spec mastery spells(usually 1)
    talentPair = sStatSystem.GetTalentMasterySpells(m_Player->getClass(), m_specs[m_activeSpec].ActiveTalentTab);
    if(talentPair.first != talentPair.second)
        for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
            m_Player->addSpell(itr->second);

    // Apply glyphs
    for(uint8 i = 0; i < GLYPHS_COUNT; i++)
        if(uint16 glyphId = m_specs[m_activeSpec].Glyphs[i])
            ApplyGlyph(i, glyphId);
}

void TalentInterface::SendTalentInfo()
{
    WorldPacket data(SMSG_TALENTS_INFO, 400);
    BuildPlayerTalentInfo(&data);
    m_Player->PushPacket(&data, true);
}

void TalentInterface::BuildPlayerTalentInfo(WorldPacket *packet)
{
    *packet << uint8(0); // Packet identifier: Player's talents
    *packet << uint32(m_currentTalentPoints);
    *packet << uint8(m_specCount) << uint8(m_activeSpec);
    m_specLock.Acquire();
    for(uint8 s = 0; s < m_specCount; ++s)
    {
        if(m_specs[s].ActiveTalentTab == 0xFF)
            *packet << uint32(0) << uint8(0);
        else
        {
            *packet << uint32(TalentTreesPerClass[m_Player->getClass()][m_specs[s].ActiveTalentTab]); // Spec tree
            *packet << uint8(m_specs[s].m_talents.size());
            for(TalentStorageMap::iterator itr = m_specs[s].m_talents.begin(); itr != m_specs[s].m_talents.end(); itr++)
            {
                *packet << uint32(itr->first); // TalentId
                *packet << uint8(itr->second); // TalentRank
            }
        }

        // Send Glyph info
        *packet << uint8(GLYPHS_COUNT);
        for(uint8 i = 0; i < GLYPHS_COUNT; i++)
            *packet << uint16(m_specs[s].Glyphs[i]);
    }
    m_specLock.Release();
}

void TalentInterface::BuildPlayerActionInfo(WorldPacket *packet, uint8 reason)
{
    m_specLock.Acquire();
    for(uint8 i = 0; i < PLAYER_ACTION_BUTTON_COUNT; i++)
        *packet << m_specs[m_activeSpec].m_actions[i].PackedData;
    *packet << uint8(reason); // Action button list packet
    m_specLock.Release();
}

void TalentInterface::ModTalentPoints(int32 talentPoints)
{
    if(talentPoints == 0)
        return;

    m_bonusTalentPoints += talentPoints;
    RecalculateAvailableTalentPoints();
}

void TalentInterface::ResetAllSpecs()
{
    RecalculateAvailableTalentPoints();
    for(uint8 i = 0; i < m_specCount; i++)
        ResetSpec(i, true);
    SendTalentInfo();
}

void TalentInterface::ResetSpec(uint8 spec, bool silent)
{
    ASSERT(spec < MAX_SPEC_COUNT);

    TalentStorageMap *talentMap = &m_specs[spec].m_talents;
    if(spec == m_activeSpec)
    {
        for(TalentStorageMap::iterator itr = talentMap->begin(); itr != talentMap->end(); itr++)
            if(TalentEntry *te = dbcTalent.LookupEntry(itr->first))
                RemoveTalent(te->RankID[itr->second]);

        // Remove our spec primary spells
        StatSystem::TalentSpellPair talentPair = sStatSystem.GetTalentPrimarySpells(m_Player->getClass(), m_specs[m_activeSpec].ActiveTalentTab);
        if(talentPair.first != talentPair.second)
            for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
                m_Player->removeSpell(itr->second);

        // Remove our spec mastery spells(usually 1)
        talentPair = sStatSystem.GetTalentMasterySpells(m_Player->getClass(), m_specs[m_activeSpec].ActiveTalentTab);
        if(talentPair.first != talentPair.second)
            for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
                m_Player->removeSpell(itr->second);

        // The dual wield skill for shamans can only be added by talents.
        // so when reset, the dual wield skill should be removed too.
        // (see also Spell::SpellEffectDualWield)
        if( m_Player->getClass() == SHAMAN && m_Player->HasSkillLine( SKILL_DUAL_WIELD ) )
            m_Player->RemoveSkillLine( SKILL_DUAL_WIELD );

        m_currentTalentPoints = std::max<int32>(0, m_availableTalentPoints+m_bonusTalentPoints);
    }
    talentMap->clear();
    m_specs[spec].ActiveTalentTab = 0xFF;

    if(silent == false)
        SendTalentInfo();
    m_talentResetCounter++;
}

void TalentInterface::UnlockSpec(uint8 spec)
{
    m_specCount = spec;
    SendTalentInfo();
}

void TalentInterface::ApplySpec(uint8 spec)
{
    ASSERT(spec < MAX_SPEC_COUNT);
    if(spec == m_activeSpec)
        return;

    WorldPacket data(SMSG_ACTION_BUTTONS, 1);
    data << uint8(ACTION_BUTTON_LIST_RESET);
    m_Player->PushPacket(&data, true);

    for(TalentStorageMap::iterator itr = m_specs[m_activeSpec].m_talents.begin(); itr != m_specs[m_activeSpec].m_talents.end(); itr++)
    {
        TalentEntry * talentInfo = dbcTalent.LookupEntry(itr->first);
        if(talentInfo == NULL || itr->second > 4)
            continue;

        RemoveTalent(talentInfo->RankID[itr->second]);
    }
    for(uint8 i = 0; i < GLYPHS_COUNT; i++)
        UnapplyGlyph(i);

    // Remove our spec primary spells
    StatSystem::TalentSpellPair talentPair = sStatSystem.GetTalentPrimarySpells(m_Player->getClass(), m_specs[m_activeSpec].ActiveTalentTab);
    if(talentPair.first != talentPair.second)
        for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
            m_Player->removeSpell(itr->second);

    // Remove our spec mastery spells(usually 1)
    talentPair = sStatSystem.GetTalentMasterySpells(m_Player->getClass(), m_specs[m_activeSpec].ActiveTalentTab);
    if(talentPair.first != talentPair.second)
        for(StatSystem::TalentSpellStorage::iterator itr = talentPair.first; itr != talentPair.second; itr++)
            m_Player->removeSpell(itr->second);

    m_activeSpec = spec;
    InitActiveSpec();
}

void TalentInterface::RecalculateAvailableTalentPoints()
{
    uint32 availableTalents = dbcNumTalents.LookupEntry(std::min<uint32>(MAXIMUM_ATTAINABLE_LEVEL, m_Player->getLevel()))->talentPoints;
    bool reset = availableTalents < m_availableTalentPoints;
    m_availableTalentPoints = availableTalents;
    availableTalents += m_bonusTalentPoints;
    for(uint8 i = 0; i < m_specCount; i++)
    {
        uint32 spentPoints = CalculateSpentPoints(i);
        if(reset && availableTalents < spentPoints)
            ResetSpec(i, true);
        else if(i == m_activeSpec)
            m_currentTalentPoints = std::max<int32>(0, availableTalents-spentPoints);
    }
    SendTalentInfo();
}

int32 TalentInterface::CalculateSpentPoints(uint8 spec, int32 talentTree)
{
    int32 spentPoints = 0;
    for(TalentStorageMap::iterator itr = m_specs[spec].m_talents.begin(); itr != m_specs[spec].m_talents.end(); itr++)
    {
        spentPoints++;// Talent exists in map so increment
        TalentEntry *talentInfo = dbcTalent.LookupEntry(itr->first);
        if(talentInfo == NULL || itr->second > 4)
            continue;
        if(talentTree > 0 && talentInfo->TalentTree != talentTree)
            continue;

        spentPoints += itr->second; // Add our talent rank to the cost
    }
    return spentPoints;
}

void TalentInterface::GetActiveTalentTabStack(uint32 &output)
{
    for(uint8 i = 0; i < MAX_SPEC_COUNT; i++)
        output |= uint32(m_specs[i].ActiveTalentTab)<<(i*8);
}

void TalentInterface::ApplyTalent(uint32 spellid)
{
    SpellEntry *spellInfo = dbcSpell.LookupEntry( spellid ), *spellInfo2 = NULL;
    if(!spellInfo)
        return; // not found

    if(!spellInfo->isPassiveSpell())
    {
        if(spellInfo->RankNumber)
        {
            if(!m_Player->FindHigherRankingSpellWithNamehash(spellInfo->NameHash, spellInfo->RankNumber))
                m_Player->addSpell(spellid);  // in this case we need to learn the spell itself
        } else m_Player->addSpell(spellid);  // in this case we need to learn the spell itself
        return;
    }

    if(spellInfo->HasEffect(SPELL_EFFECT_LEARN_SPELL))
    {
        for(uint8 i = 0; i < 3; i++)
        {
            if(spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
            {
                if((spellInfo2 = dbcSpell.LookupEntry(spellInfo->EffectTriggerSpell[i])) != NULL && spellInfo2->RankNumber
                    && m_Player->FindHigherRankingSpellWithNamehash(spellInfo2->NameHash, spellInfo2->RankNumber))
                    return;
            }
        }
    }

    m_Player->GetSpellInterface()->TriggerSpell(spellInfo, m_Player);
}

void TalentInterface::RemoveTalent(uint32 spellid)
{
    SpellEntry * sp = dbcSpell.LookupEntry(spellid);
    if(!sp)
        return; // not found

    for(uint8 k = 0; k < 3; ++k)
    {
        if(sp->Effect[k] == SPELL_EFFECT_LEARN_SPELL)
        {
            SpellEntry * sp2 = dbcSpell.LookupEntry(sp->EffectTriggerSpell[k]);
            if(!sp2)
                continue;
            m_Player->removeSpellByNameHash(sp2->NameHash);
        }
    }

    m_Player->removeSpellByNameHash(sp->NameHash);
    m_Player->m_AuraInterface.RemoveAllAurasByNameHash(sp->NameHash, true);
}

bool TalentInterface::HasTalent(uint32 talentId, int8 talentRank)
{
    bool res = false;
    m_specLock.Acquire();
    TalentStorageMap::iterator itr = m_specs[m_activeSpec].m_talents.find(talentId);
    if(itr != m_specs[m_activeSpec].m_talents.end())
        res = (talentRank == -1 || itr->second == talentRank);
    m_specLock.Release();
    return res;
}

bool TalentInterface::LearnTalent(uint32 talentId, uint8 talentRank)
{
    ASSERT(talentRank < 5);
    if(m_specs[m_activeSpec].ActiveTalentTab == 0xFF)
        return false;

    TalentEntry *talentInfo = dbcTalent.LookupEntry(talentId);
    if(talentInfo == NULL)
        return false;

    uint8 currentRank = 0;
    if(m_specs[m_activeSpec].m_talents.find(talentId) != m_specs[m_activeSpec].m_talents.end())
        currentRank = 1+m_specs[m_activeSpec].m_talents.at(talentId);
    if(currentRank >= talentRank+1)
        return false; // player already knows requested or higher rank for this talent

    uint32 RequiredTalentPoints = talentRank + 1 - currentRank;
    if(m_currentTalentPoints < RequiredTalentPoints )
        return false; // player doesn't have enough points to get this rank for this talent

    // Check if it requires another talent
    if (talentInfo->DependsOn > 0)
    {
        TalentEntry *depTalentInfo = dbcTalent.LookupEntry(talentInfo->DependsOn);
        if(depTalentInfo == NULL)
            return false;
        if(m_specs[m_activeSpec].m_talents.find(talentInfo->DependsOn) == m_specs[m_activeSpec].m_talents.end())
            return false; // player doesn't have the talent this one depends on
        if(talentInfo->DependsOnRank > m_specs[m_activeSpec].m_talents.at(talentInfo->DependsOn))
            return false; // player doesn't have the talent rank this one depends on
    }

    // Check that the requested talent belongs to a tree from player's class
    uint32 tTree = talentInfo->TalentTree;
    uint32 cl = m_Player->getClass();

    for(uint8 i = 0; i < 3; i++)
    {
        if(tTree == TalentTreesPerClass[cl][i])
            break;

        if(i == 2)
        {
            // cheater!
            m_Player->Kick(2000);
            return false;
        }
    }

    uint32 spentPoints = CalculateSpentPoints(m_activeSpec);
    if(spentPoints < REQ_PRIMARY_TREE_TALENTS && talentInfo->TalentTree != TalentTreesPerClass[m_Player->getClass()][m_specs[m_activeSpec].ActiveTalentTab])
        return false;
    // We require that for each row a player spends 5 talents before reaching the next
    spentPoints = CalculateSpentPoints(m_activeSpec, talentInfo->TalentTree);
    if (talentInfo->Row > 0 && spentPoints < (talentInfo->Row * 5))
        return false;

    uint32 spellid = talentInfo->RankID[talentRank];
    if( spellid == 0 )
    {
        sLog.outDebug("Talent: %u Rank: %u = 0", talentId, talentRank);
        return false;
    }

    m_currentTalentPoints -= RequiredTalentPoints;
    m_specs[m_activeSpec].m_talents[talentInfo->TalentID] = talentRank;

    if(talentRank > 0) // remove old rank aura
        if(uint32 respellid = talentInfo->RankID[currentRank - 1])
            m_Player->RemoveAura(respellid);

    ApplyTalent(spellid);
    return true;
}

void TalentInterface::LoadGlyphData(QueryResult * result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 spec = fields[1].GetUInt8();
        uint8 index = fields[2].GetUInt8();
        if(spec >= MAX_SPEC_COUNT || index >= GLYPHS_COUNT)
        {
            sLog.outDebug("Out of range glyph data [%d] for player with GUID [%d]", spec, fields[0].GetUInt64());
            continue;
        }

        m_specs[spec].Glyphs[index] = fields[3].GetUInt16();
    } while(result->NextRow());
}

void TalentInterface::SaveGlyphData(QueryBuffer * buf)
{
    if(buf)buf->AddQuery("DELETE FROM character_glyphs WHERE guid = '%u';", m_Player->GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_glyphs WHERE guid = '%u';", m_Player->GetLowGUID());

    m_specLock.Acquire();
    std::stringstream ss;
    for(uint8 s = 0; s < m_activeSpec; s++)
    {
        for(uint32 i = 0; i < GLYPHS_COUNT; i++)
        {
            if(m_specs[s].Glyphs[i] == 0)
                continue;

            if(ss.str().length())
                ss << ", ";
            ss << "(" << m_Player->GetLowGUID() << ", " << uint32(s) << ", " << uint32(i) << ", " << uint32(m_specs[s].Glyphs[i]) << ")";
        }
    }
    m_specLock.Release();

    if(ss.str().length())
    {
        if(buf)buf->AddQuery("REPLACE INTO character_glyphs VALUES %s;", ss.str().c_str());
        else CharacterDatabase.Execute("REPLACE INTO character_glyphs VALUES %s;", ss.str().c_str());
    }
}

// Update glyphs after level change
void TalentInterface::InitGlyphsForLevel(uint32 level)
{
    uint32 glyph_mask = 0; // Enable number of glyphs depending on level
    if (level >= 25) glyph_mask |= 0x01 | 0x02 | 0x40;
    if (level >= 50) glyph_mask |= 0x04 | 0x08 | 0x80;
    if (level >= 75) glyph_mask |= 0x10 | 0x20 | 0x100;
    m_Player->SetUInt32Value(PLAYER_GLYPHS_ENABLED, glyph_mask);
}

static const uint32 GlyphSlotDataDump[GLYPHS_COUNT] = {21, 22, 23, 24, 25, 26, 41, 42, 43};

void TalentInterface::InitGlyphSlots()
{
    for(uint32 i = 0; i < GLYPHS_COUNT; i++)
        m_Player->SetUInt32Value(PLAYER_FIELD_GLYPH_SLOTS_1 + i, GlyphSlotDataDump[i]);
}

void TalentInterface::UnapplyGlyph(uint8 slot)
{
    if(slot >= GLYPHS_COUNT)
        return; // Glyph doesn't exist
    // Get info
    uint32 glyphId = m_Player->GetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot);
    if(glyphId == 0)
        return;
    GlyphProperties *glyph = dbcGlyphProperties.LookupEntry(glyphId);
    if(glyph == NULL)
        return;
    m_specs[m_activeSpec].Glyphs[slot] = 0;
    m_Player->SetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot, 0);
    m_Player->m_AuraInterface.RemoveAllAuras(glyph->SpellId);
}

static const uint32 glyphType[9] = {0, 1, 1, 0, 1, 0, 2, 2, 2};

uint8 TalentInterface::ApplyGlyph(uint8 slot, uint32 glyphId)
{
    if(slot >= GLYPHS_COUNT || glyphId==0)
        return SPELL_FAILED_INVALID_GLYPH;

    // Get info
    GlyphProperties *glyph = dbcGlyphProperties.LookupEntry(glyphId);
    if(glyph == NULL)
        return SPELL_FAILED_INVALID_GLYPH;
    SpellEntry *glyphSpell = dbcSpell.LookupEntry(glyph->SpellId);
    if(glyphSpell == NULL)
        return SPELL_FAILED_INVALID_GLYPH;

    for(uint32 x = 0; x < GLYPHS_COUNT; ++x)
    {
        if(m_specs[m_activeSpec].Glyphs[x] == glyphId && slot != x)
            return SPELL_FAILED_UNIQUE_GLYPH;
    }

    // Glyph type does not match or slot is not enabled
    if( glyphType[slot] != glyph->TypeFlags || (m_Player->GetUInt32Value(PLAYER_GLYPHS_ENABLED) & (1 << slot)) == 0)
        return SPELL_FAILED_INVALID_GLYPH;

    UnapplyGlyph(slot);
    m_Player->SetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot, glyphId);
    m_specs[m_activeSpec].Glyphs[slot] = glyphId;
    m_Player->GetSpellInterface()->TriggerSpell(glyphSpell, m_Player);
    return 0;
}

void TalentInterface::SaveActionButtonData(QueryBuffer *buf)
{
    std::stringstream ss;
    for(uint8 i = 0; i < m_specCount; i++)
    {
        for(uint8 x = 0; x < PLAYER_ACTION_BUTTON_COUNT; x++)
        {
            if(m_specs[i].m_actions[x].PackedData == 0)
                continue;

            if(ss.str().length())
                ss << ", ";

            ss << "(" << m_Player->GetLowGUID() << ", " << uint32(i) << ", " << uint32(x) << ", " << m_specs[i].m_actions[x].PackedData << ")";
        }
    }

    if(buf)
    {
        buf->AddQuery("DELETE FROM character_actions WHERE guid = '%u';", m_Player->GetLowGUID());
        if(ss.str().empty())
            return;
        buf->AddQuery("REPLACE INTO character_actions VALUES %s", ss.str().c_str());
        return;
    }

    CharacterDatabase.Execute("DELETE FROM character_actions WHERE guid = '%u';", m_Player->GetLowGUID());
    if(ss.str().empty())
        return;
    CharacterDatabase.Execute("REPLACE INTO character_actions VALUES %s", ss.str().c_str());
}

void TalentInterface::LoadActionButtonData(QueryResult *result)
{
    if(result == NULL)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint8 spec = fields[1].GetUInt8();
        uint8 offset = fields[2].GetUInt8();
        uint32 value = fields[3].GetUInt32();
        m_specs[spec].m_actions[offset].PackedData = value;
    }while(result->NextRow());
}

void TalentInterface::SendInitialActions()
{
    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_COUNT*4 + 1);
    BuildPlayerActionInfo(&data, ACTION_BUTTON_LIST_REASON_INITIAL);
    m_Player->PushPacket(&data, true);
}

void TalentInterface::setAction(uint8 button, uint32 action, uint8 type, int8 SpecOverride)
{
    if( button >= PLAYER_ACTION_BUTTON_COUNT )
        return;

    if(SpecOverride == -1) SpecOverride = GetActiveSpec();
    m_specs[SpecOverride].m_actions[button].PackedData = ACTION_PACK_BUTTION(action, type);
}
