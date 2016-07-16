/***
* Demonstrike Core
*/

#include "StdAfx.h"

FactionInterface::FactionInterface(Player *player) : m_player(player)
{

}

FactionInterface::~FactionInterface()
{

}

void FactionInterface::CreateFactionData()
{
    // Can be called outside the load function, so acquire our friendly and enemy factions here
    if(m_player->GetTeam() == TEAM_ALLIANCE)
        friendlyFactions = sFactionSystem.GetAllianceFactions(), enemyFactions = sFactionSystem.GetHordeFactions();
    else friendlyFactions = sFactionSystem.GetHordeFactions(), enemyFactions = sFactionSystem.GetAllianceFactions();

    FactionSystem::FactionEntryMap *repMap = sFactionSystem.GetRepIDFactions();
    for(auto itr = repMap->begin(); itr != repMap->end(); itr++)
        if(!HasReputationData(itr->first))
            CreateRepData(itr->second);
}

void FactionInterface::LoadFactionData(QueryResult *result)
{
    if(result == NULL)
    {   // If we have no faction data to load, we need to create some
        CreateFactionData();
        return;
    }

    // Set our friendly and enemy factions at player load
    if(m_player->GetTeam() == TEAM_ALLIANCE)
        friendlyFactions = sFactionSystem.GetAllianceFactions(), enemyFactions = sFactionSystem.GetHordeFactions();
    else friendlyFactions = sFactionSystem.GetHordeFactions(), enemyFactions = sFactionSystem.GetAllianceFactions();

    do
    {
        Field *fields = result->Fetch();
        uint32 index = fields[1].GetUInt32();

        FactionReputation *rep = GetReputation(index);
        rep->repID = index;
        rep->flag = fields[2].GetUInt8();
        rep->baseStanding = fields[3].GetInt32();
        rep->standing = fields[4].GetInt32();
    }while(result->NextRow());
}

void FactionInterface::SaveFactionData(QueryBuffer *buf)
{
    if(buf != NULL)
        buf->AddQuery("DELETE FROM character_reputation WHERE guid = '%u';", m_player->GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_reputation WHERE guid = '%u';", m_player->GetLowGUID());

    std::stringstream ss;
    for(std::map<uint16, FactionReputation>::iterator itr = m_reputations.begin(); itr != m_reputations.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";

        ss << "(" << m_player->GetLowGUID()
            << ", " << uint32(itr->first)
            << ", " << uint32(itr->second.flag)
            << ", " << int32(itr->second.baseStanding)
            << ", " << int32(itr->second.standing);
        ss << ")";
    }

    if(ss.str().empty())
        return;

    if(buf) buf->AddQuery("REPLACE INTO character_reputation VALUES %s;", ss.str().c_str());
    else CharacterDatabase.Execute("REPLACE INTO character_reputation VALUES %s;", ss.str().c_str());
}

void FactionInterface::BuildInitialFactions(ByteBuffer *buff)
{
    uint32 count = 0;
    *buff << uint32(count);
    for(uint16 i = 0; i < 0x0100; i++)
    {
        if(m_reputations.find(i) == m_reputations.end())
            continue;
        while(count != i)
        {
            *buff << uint8(0) << uint32(0);
            count++;
        }

        FactionReputation *rep = &m_reputations.at(i);
        *buff << uint8(rep->flag) << int32(rep->CalcStanding());
        count++;
    }

    buff->put<uint32>(0, count);
}

uint32 FactionInterface::GetStanding(uint32 faction)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return 0;
    if(!HasReputationData(factionEntry->RepListIndex))
        return 0;
    return GetReputation(factionEntry->RepListIndex)->CalcStanding();
}

uint32 FactionInterface::GetBaseStanding(uint32 faction)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return 0;
    if(!HasReputationData(factionEntry->RepListIndex))
        return 0;
    return GetReputation(factionEntry->RepListIndex)->baseStanding;
}

Standing FactionInterface::GetStandingRank(uint32 faction)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return STANDING_NEUTRAL;
    if(!HasReputationData(factionEntry->RepListIndex))
        return STANDING_NEUTRAL;
    return Player::GetReputationRankFromStanding(GetReputation(factionEntry->RepListIndex)->CalcStanding());
}

bool FactionInterface::IsHostileBasedOnReputation(FactionEntry *faction)
{
    return GetStandingRank(faction->ID) <= STANDING_HOSTILE;
}

void FactionInterface::SetStanding(uint32 faction, int32 standing)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return;
    if(!HasReputationData(factionEntry->RepListIndex))
        CreateRepData(factionEntry);

    FactionReputation *rep = GetReputation(factionEntry->RepListIndex);
    if(!rep->isVisible() && !rep->blockVisibility())
    {
        rep->setVisible();
        if(m_player->IsInWorld() && m_player->GetSession())
            m_player->GetSession()->OutPacket(SMSG_SET_FACTION_VISIBLE, 4, &factionEntry->RepListIndex);
    }

    int32 oldValue = rep->standing;
    rep->standing = std::min<int32>(std::max<int32>(standing, -42000), 42999);
    //if(Player::GetReputationRankFromStanding(oldValue) != Player::GetReputationRankFromStanding(rep->standing))
    //    UpdateInrangeSetsBasedOnReputation();
    //OnModStanding( faction, itr->second );
}

void FactionInterface::ModStanding(uint32 faction, int32 standing)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return;
    if(!HasReputationData(factionEntry->RepListIndex))
        CreateRepData(factionEntry);

    FactionReputation *rep = GetReputation(factionEntry->RepListIndex);
    if(!rep->isVisible() && !rep->blockVisibility())
    {
        rep->setVisible();
        if(m_player->IsInWorld() && m_player->GetSession())
            m_player->GetSession()->OutPacket(SMSG_SET_FACTION_VISIBLE, 4, &factionEntry->RepListIndex);
    }

    int32 oldValue = rep->CalcStanding();
    rep->standing = std::min<int32>(std::max<int32>(oldValue + standing, -42000), 42999);
    //if(Player::GetReputationRankFromStanding(oldValue) != Player::GetReputationRankFromStanding(rep->CalcStanding()))
    //    UpdateInrangeSetsBasedOnReputation();
    //OnModStanding( faction, itr->second );
}

bool FactionInterface::IsAtWar(uint32 faction)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return true; // Just set non rep list to at war
    // If we don't have rep data, return false
    if(!HasReputationData(factionEntry->RepListIndex))
        return false;
    return GetReputation(factionEntry->RepListIndex)->isAtWar();
}

void FactionInterface::SetAtWar(uint32 faction, uint8 state)
{
    FactionEntry *factionEntry = dbcFaction.LookupEntry(faction);
    if(factionEntry == NULL || factionEntry->RepListIndex < 0)
        return;
    // If we haven't discovered the rep, don't allow settings flags
    if(!HasReputationData(factionEntry->RepListIndex))
        return;
    // Acquire the reputation pointer
    if(FactionReputation *rep = GetReputation(factionEntry->RepListIndex))
    {
        // Already set to our target state
        if((rep->isAtWar() && state == 1) || (!rep->isAtWar() && state == 0))
            return;
        // If invisible, or unmodifiable then stop here
        if(!rep->isVisible() || rep->blockVisibility() || rep->isPeaceForced())
            return;
        // Can't change at war state when standing is low
        if(Player::GetReputationRankFromStanding(rep->CalcStanding()) <= STANDING_HOSTILE)
            return;
        if(state == 1)
            rep->setAtWar();
        else rep->setAtPeace();
        //UpdateInrangeSetsBasedOnReputation();
    }
}

void FactionInterface::Reputation_OnTalk(FactionEntry *faction)
{
    // set faction visible if not visible
    if(faction == NULL || faction->RepListIndex < 0)
        return;

    if(FactionReputation * rep = GetReputation(faction->RepListIndex))
    {
        if(rep->isVisible() || rep->blockVisibility())
            return;

        rep->setVisible();
        if(!(m_player->IsInWorld() && m_player->GetSession()))
            return;
        m_player->GetSession()->OutPacket(SMSG_SET_FACTION_VISIBLE, 4, &faction->RepListIndex);
    }
}

void FactionInterface::Reputation_OnKill(Unit *pVictim)
{
    uint8 team = m_player->GetTeam();
    /*if(ReputationModifier *modifier = objmgr.GetReputationModifier(pVictim, faction->ID))
    {
        // Apply this data.
        for( vector<ReputationMod>::iterator itr = modifier->mods.begin(); itr != modifier->mods.end(); itr++ )
        {
            if ( !(*itr).faction[team] )
                continue;

            // rep limit?
            if ( !IS_INSTANCE(GetMapId()) || (IS_INSTANCE(GetMapId()) && iInstanceType != MODE_5PLAYER_HEROIC) )
                if ( (*itr).replimit && GetStanding( (*itr).faction[team] ) >= (int32)(*itr).replimit )
                    continue;

            ModStanding( itr->faction[team], float2int32( itr->value * sWorld.getRate( RATE_KILLREPUTATION ) ) );
        }
    }
    else
    {
        if ( IS_INSTANCE( GetMapId() ) && objmgr.HandleInstanceReputationModifiers( this, pVictim ) )
            return;

        if ( pVictim->m_faction->RepListIndex < 0 )
            return;

        int32 change = int32( -5.0f * sWorld.getRate( RATE_KILLREPUTATION ) );
        ModStanding( pVictim->m_faction->ID, change );
    }*/
}

void FactionInterface::CreateRepData(FactionEntry *faction)
{
    uint8 index = 0xFF;
    bool forcedHidden = false;
    for(uint8 i = 0; i < 4; i++)
    {
        if(faction->repFlags[i] & (FACTION_FLAG_HIDDEN|FACTION_FLAG_INVISIBLE_FORCED))
            forcedHidden = true;
        if( ( faction->RaceMask[i] & m_player->getRaceMask() || ( faction->RaceMask[i] == 0 && faction->ClassMask[i] != 0 ) ) &&
            ( faction->ClassMask[i] & m_player->getClassMask() || faction->ClassMask[i] == 0 ) )
        {
            index = i;
            break;
        }
    }

    if(index == 0xFF)
    {
        FactionReputation *rep = GetReputation(faction->RepListIndex);
        rep->repID = faction->RepListIndex;
        rep->flag = uint8(forcedHidden ? FACTION_FLAG_INVISIBLE_FORCED : FACTION_FLAG_NONE);
        rep->baseStanding = rep->standing = 0;
        if(friendlyFactions && friendlyFactions->find(faction->ID) != friendlyFactions->end())
            rep->standing += 3000;
        else if(enemyFactions && enemyFactions->find(faction->ID) != enemyFactions->end())
        {
            // Set at war and standing to hated
            rep->flag |= uint8(FACTION_FLAG_AT_WAR);
            rep->standing -= 6000;
        }
        return;
    }

    FactionReputation *rep = GetReputation(faction->RepListIndex);
    rep->repID = faction->RepListIndex;
    rep->flag = uint8(faction->repFlags[index]);
    rep->baseStanding = faction->baseRepValue[index];
    rep->standing = faction->baseRepValue[index];
}

Standing Player::GetReputationRankFromStanding(int32 Standing_)
{
    if( Standing_ >= 42000 )
        return STANDING_EXALTED;
    else if( Standing_ >= 21000 )
        return STANDING_REVERED;
    else if( Standing_ >= 9000 )
        return STANDING_HONORED;
    else if( Standing_ >= 3000 )
        return STANDING_FRIENDLY;
    else if( Standing_ >= 0 )
        return STANDING_NEUTRAL;
    else if( Standing_ > -3000 )
        return STANDING_UNFRIENDLY;
    else if( Standing_ > -6000 )
        return STANDING_HOSTILE;
    return STANDING_HATED;
}
