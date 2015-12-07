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
    for(std::map<uint32, CurrencyData>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); itr++)
    {
        if(buf != NULL)
            buf->AddQuery("INSERT INTO character_currency VALUES(%u, %u, %u);", m_player->GetLowGUID(), itr->first, itr->second);
        else CharacterDatabase.Execute("INSERT INTO character_currency VALUES(%u, %u, %u);", m_player->GetLowGUID(), itr->first, itr->second);
    }
}

void PlayerCurrency::SendInitialCurrency()
{
    uint32 weekCap = 0xFFFFFFFF;
    WorldPacket data(SMSG_INIT_CURRENCY, 200);
    data.WriteBits(m_currencies.size(), 23);
    for(std::map<uint32, CurrencyData>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); itr++)
    {
        data.WriteBit(weekCap != 0xFFFFFFFF && itr->second.weekCount);
        data.WriteBits(itr->second.currencyFlags, 4);
        data.WriteBit(weekCap != 0xFFFFFFFF);
        data.WriteBit(false); // SeasonCount
    }
    data.FlushBits();

    float precision = 1.f;
    for (std::map<uint32, CurrencyData>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); ++itr)
    {
        data << uint32(floor(itr->second.count / precision));
        if (weekCap != 0xFFFFFFFF) data << uint32(floor(weekCap / precision));
        if (false) data << uint32(0); // Season cap
        data << uint32(itr->first);
        if (weekCap != 0xFFFFFFFF && itr->second.weekCount) data << uint32(floor(itr->second.weekCount / precision));
    }

    m_player->SendPacket(&data);
}

bool PlayerCurrency::HasCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) != m_currencies.end())
        if(m_currencies.at(currency).count >= amount)
            return true;
    return false;
}

void PlayerCurrency::AddCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) == m_currencies.end())
        m_currencies.insert(std::make_pair(currency, amount));
    else m_currencies.at(currency).count += amount;
}

void PlayerCurrency::RemoveCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) == m_currencies.end())
        return;
    if(m_currencies.at(currency).count == amount)
        m_currencies.erase(currency);
    else m_currencies.at(currency).count -= amount;
}
