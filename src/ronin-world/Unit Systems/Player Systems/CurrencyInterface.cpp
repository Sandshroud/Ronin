/***
 * Demonstrike Core
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
    m_player->SendPacket(&WorldPacket(SMSG_WEEKLY_RESET_CURRENCY));
}

void PlayerCurrency::LoadFromDB(time_t lastSavedWeek, QueryResult *result)
{
    if(result == NULL)
        return;

    bool loadWeekData = lastWeekStart<(lastSavedWeek+3600);
    do
    {
        Field *fields = result->Fetch();
        m_currencies.insert(std::make_pair(fields[1].GetUInt32(), CurrencyData(fields, loadWeekData)));
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
        weekCap = entry->WeekCap / precision;

        // Append bit compression data
        bitBuff.SetBit(weekCap && itr->second.weekCount);
        bitBuff.SetBits(itr->second.currencyFlags, 4);
        bitBuff.SetBit(weekCap > precision);
        bitBuff.SetBit(false); // SeasonTotal

        // Append byte data
        byteBuff << uint32(floor(itr->second.count / precision));
        // Weekly cap
        if (weekCap > precision)
            byteBuff << uint32(floor(weekCap / precision));
        // Season total
        if (false) byteBuff << uint32(0);
        byteBuff << uint32(itr->first);
        // Week count
        if (itr->second.weekCount > precision)
            byteBuff << uint32(floor(itr->second.weekCount / precision));
        count++;
    }

    WorldPacket data(SMSG_INIT_CURRENCY, 200);
    data.WriteBits(count, 23);
    bitBuff.Append(&data, true);
    data.append(byteBuff.contents(), byteBuff.size());
    m_player->SendPacket(&data);
}

bool PlayerCurrency::HasCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) != m_currencies.end())
        if(m_currencies.at(currency).count >= amount)
            return true;
    return false;
}

void PlayerCurrency::AddCurrency(uint32 currency, uint32 amount, bool silent)
{
    CurrencyTypeEntry *entry = dbcCurrencyType.LookupEntry(currency);
    if(entry == NULL)
        return;
    if(entry->WeekCap && amount > entry->WeekCap)
        amount = entry->WeekCap;

    float precision = (entry->Flags&0x08) ? 100.f : 1.f;
    if(m_currencies.find(currency) == m_currencies.end())
        m_currencies.insert(std::make_pair(currency, CurrencyData(amount, entry->WeekCap > 0)));
    else
    {
        uint32 current = m_currencies.at(currency).count, currentWeek = m_currencies.at(currency).weekCount;
        if(entry->WeekCap)
        {
            if(currentWeek = entry->WeekCap)
                return;
            if((currentWeek+amount) > entry->WeekCap)
                amount = entry->WeekCap-currentWeek;
            m_currencies.at(currency).weekCount += amount;
        }
        m_currencies.at(currency).count += amount;
    }

    WorldPacket data(SMSG_UPDATE_CURRENCY, 20);
    data.WriteBit(entry->WeekCap);
    data.WriteBit(0);
    data.WriteBit(silent);
    data << uint32(floor(m_currencies.at(currency).count/precision));
    data << uint32(currency);
    if(entry->WeekCap)
        data << uint32(floor(m_currencies.at(currency).weekCount/precision));
    m_player->SendPacket(&data);
}

void PlayerCurrency::RemoveCurrency(uint32 currency, uint32 amount)
{
    if(m_currencies.find(currency) == m_currencies.end())
        return;
    if(m_currencies.at(currency).count >= amount)
        m_currencies.at(currency).count = 0;
    else m_currencies.at(currency).count -= amount;
}

void PlayerCurrency::SetCurrencyFlags(uint32 currency, uint8 flag)
{
    if(m_currencies.find(currency) == m_currencies.end())
        return;
    m_currencies.at(currency).currencyFlags = flag;
}
