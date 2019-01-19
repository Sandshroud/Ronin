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

#include "StdAfx.h"

PlayerCurrency::PlayerCurrency(Player *player) : m_player(player), lastWeekStart(sWorld.GetWeekStart())
{

}

PlayerCurrency::~PlayerCurrency()
{

}

void PlayerCurrency::Update()
{
    // If our last saved weekly timestamp is more than an hour behind our new weekly start, reset week data
    if((lastWeekStart+3600) > sWorld.GetWeekStart())
        return;

    for(std::map<uint32, CurrencyData>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); itr++)
        itr->second.weekCount = 0;
    m_player->PushPacket(&WorldPacket(SMSG_WEEKLY_RESET_CURRENCY));

    lastWeekStart = sWorld.GetWeekStart();
}

void PlayerCurrency::LoadFromDB(time_t lastSavedWeek, QueryResult *result)
{
    if(result == NULL)
        return;

    bool loadWeekData = lastWeekStart<(lastSavedWeek+3600);
    do
    {
        Field *fields = result->Fetch();
        CurrencyTypeEntry *entry = dbcCurrencyType.LookupEntry(fields[1].GetUInt32());
        if(entry == NULL)
            continue;
        m_currencies.insert(std::make_pair(entry->Id, CurrencyData(fields, entry->WeekCap && loadWeekData)));
    }while(result->NextRow());
}

void PlayerCurrency::SaveToDB(QueryBuffer *buf)
{
    if(buf != NULL)
        buf->AddQuery("DELETE FROM character_currency WHERE guid = '%u';", m_player->GetLowGUID());
    else CharacterDatabase.Execute("DELETE FROM character_currency WHERE guid = '%u';", m_player->GetLowGUID());
    if(m_currencies.empty())
        return;

    std::stringstream ss;
    for(std::map<uint32, CurrencyData>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); itr++)
    {
        if(ss.str().length())
            ss << ", ";

        ss << "(" << m_player->GetLowGUID()
            << ", " << uint32(itr->first)
            << ", " << uint32(itr->second.count)
            << ", " << uint32(itr->second.weekCount)
            << ", " << uint32(itr->second.totalCount)
            << ", " << uint32(itr->second.currencyFlags);
        ss << ")";
    }

    if(ss.str().empty())
        return;

    if(buf) buf->AddQuery("REPLACE INTO character_currency VALUES %s;", ss.str().c_str());
    else CharacterDatabase.Execute("REPLACE INTO character_currency VALUES %s;", ss.str().c_str());
}

void PlayerCurrency::SendInitialCurrency()
{
    BitBuffer bitBuff;
    ByteBuffer byteBuff;
    float precision = 100.f;
    uint32 count = 0, weekCap = 0;
    for(std::map<uint32, CurrencyData>::iterator itr = m_currencies.begin(); itr != m_currencies.end(); itr++)
    {
        CurrencyTypeEntry *entry = dbcCurrencyType.LookupEntry(itr->first);
        if(entry == NULL || entry->Category == 89)
            continue;

        // Recalculate precision and weekly cap
        precision = (entry->Flags&0x08) ? 100.f : 1.f;
        weekCap = entry->WeekCap;

        // Append bit compression data
        bitBuff.SetBit(weekCap && itr->second.weekCount);
        bitBuff.SetBits(itr->second.currencyFlags, 4);
        bitBuff.SetBit(weekCap);
        bitBuff.SetBit((entry->Flags & 0x300) && itr->second.totalCount);

        // Append byte data
        byteBuff << uint32(floor(itr->second.count / precision));
        // Weekly cap
        if (weekCap)
            byteBuff << uint32(floor(weekCap / precision));
        // Season total
        if ((entry->Flags & 0x300) && itr->second.totalCount)
            byteBuff << uint32(itr->second.totalCount);
        byteBuff << uint32(itr->first);
        // Week count
        if (weekCap && itr->second.weekCount)
            byteBuff << uint32(floor(itr->second.weekCount / precision));
        count++;
    }

    WorldPacket data(SMSG_INIT_CURRENCY, 200);
    data.WriteBits(count, 23);
    bitBuff.Append(&data, true);
    data.append(byteBuff.contents(), byteBuff.size());
    m_player->PushPacket(&data, true);
}

bool PlayerCurrency::HasCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) != m_currencies.end())
        if(m_currencies.at(currency).count >= amount)
            return true;
    return false;
}

bool PlayerCurrency::HasTotalCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) != m_currencies.end())
        if(m_currencies.at(currency).totalCount >= amount)
            return true;
    return false;
}

void PlayerCurrency::AddCurrency(uint32 currency, uint32 amount, bool silent)
{
    CurrencyTypeEntry *entry = dbcCurrencyType.LookupEntry(currency);
    if(entry == NULL)
        return;

    amount *= (entry->Flags&0x08) ? 100 : 1;
    // If weekly cap, set adding amount to never exceed weekly cap
    if(entry->WeekCap && amount > entry->WeekCap)
        amount = entry->WeekCap;

    uint32 tAmount = amount, pAmount = 0;
    std::map<uint32, CurrencyData>::iterator itr;
    // See if we already have currency data and add it to total
    if((itr = m_currencies.find(currency)) != m_currencies.end())
    {
        // Check if we've capped out our currency gains for this week
        if(entry->WeekCap && itr->second.weekCount == entry->WeekCap)
            return;
        // Add up our total
        tAmount += (pAmount = itr->second.count);
    }
    // Set amount to never exceed total cap
    if(entry->TotalCap && tAmount > entry->TotalCap)
        tAmount = entry->TotalCap;
    // We were capped before so we return here
    if(pAmount == tAmount)
        return;
    // We've already set our amount to match our weekly cap, but if we're adding to existing, check our limits there as well
    if(entry->WeekCap && itr != m_currencies.end())
    {   // Check and get the distance from weekly cap if we're going to break it
        if((itr->second.weekCount+amount) > entry->WeekCap)
        {
            amount = entry->WeekCap-itr->second.weekCount;
            tAmount = pAmount + amount;
        }

        // Update weekly amount since we have a weekly cap
        itr->second.weekCount += amount;
    }

    if(m_currencies.find(currency) == m_currencies.end())
    {   // We need to insert currency data then allign our iterator
        m_currencies.insert(std::make_pair(currency, CurrencyData(amount, entry->WeekCap > 0)));
        ASSERT((itr = m_currencies.find(currency)) != m_currencies.end());
    }
    else
    {   // Current count needs to be updated
        itr->second.count = tAmount;
        // Total count is overall tracker, so update when we add new amount
        itr->second.totalCount += amount;
    }

    // Send a packet containing the new currency value
    _SendCurrencyUpdate(entry, &itr->second, silent);
}

void PlayerCurrency::RemoveCurrency(uint32 currency, uint32 amount)
{
    CurrencyTypeEntry *entry = dbcCurrencyType.LookupEntry(currency);
    if(entry == NULL)
        return;
    if(m_currencies.find(currency) == m_currencies.end())
        return;
    if(m_currencies.at(currency).count < amount)
        m_currencies.at(currency).count = 0;
    else m_currencies.at(currency).count -= amount;

    _SendCurrencyUpdate(entry, &m_currencies.at(currency), false);
}

void PlayerCurrency::SetCurrencyFlags(uint32 currency, uint8 flag)
{
    if(m_currencies.find(currency) == m_currencies.end())
        return;
    m_currencies.at(currency).currencyFlags = flag;
}

void PlayerCurrency::_SendCurrencyUpdate(CurrencyTypeEntry *entry, CurrencyData *data, bool silent)
{
    float precision = (entry->Flags&0x08) ? 100.f : 1.f;
    WorldPacket packet(SMSG_UPDATE_CURRENCY, 20);
    packet.WriteBit(entry->WeekCap);
    packet.WriteBit(entry->Flags&0x300);
    packet.WriteBit(silent);
    if(entry->Flags&0x300) packet << uint32(floor(data->totalCount/precision));
    packet << uint32(floor(data->count/precision));
    packet << uint32(entry->Id);
    if(entry->WeekCap)
        packet << uint32(floor(data->weekCount/precision));
    m_player->PushPacket(&packet);
}