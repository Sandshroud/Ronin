/***
 * Demonstrike Core
 */

#include "StdAfx.h"

PlayerCurrency::PlayerCurrency(Player *player) : m_player(player)
{

}

PlayerCurrency::~PlayerCurrency()
{

}

void PlayerCurrency::LoadFromDB(QueryResult *result)
{

}

void PlayerCurrency::SaveToDB(QueryBuffer *buf)
{
    if(buf != NULL)
        buf->AddQuery("DELETE FROM character_currency WHERE guid = '';", m_player->GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_currency WHERE guid = '';", m_player->GetLowGUID());
    for(std::map<uint32, uint32>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); itr++)
    {
        if(buf != NULL)
            buf->AddQuery("INSERT INTO character_currency VALUES(%u, %u, %u);", m_player->GetLowGUID(), itr->first, itr->second);
        else CharacterDatabase.Execute("INSERT INTO character_currency VALUES(%u, %u, %u);", m_player->GetLowGUID(), itr->first, itr->second);
    }
}

void PlayerCurrency::SendInitialCurrency()
{
    WorldPacket data(SMSG_INIT_CURRENCY, 0);

    m_player->SendPacket(&data);
}

bool PlayerCurrency::HasCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) != m_currencies.end())
        if(m_currencies.at(currency) >= amount)
            return true;
    return false;
}

void PlayerCurrency::AddCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) == m_currencies.end())
        m_currencies.insert(std::make_pair(currency, amount));
    else m_currencies[currency] += amount;
}

void PlayerCurrency::RemoveCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) == m_currencies.end())
        return;
    if(m_currencies.at(currency) == amount)
        m_currencies.erase(currency);
    else m_currencies[currency] -= amount;
}
