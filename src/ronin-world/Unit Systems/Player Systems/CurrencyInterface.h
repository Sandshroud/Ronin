/***
 * Demonstrike Core
 */

#pragma once

struct CurrencyData
{
    CurrencyData(uint32 amount, bool weekCap) : count(amount), weekCount(weekCap ? amount : 0), totalCount(amount), currencyFlags(0) {};
    CurrencyData(Field *fields, bool loadWeekly)
    {
        count = fields[2].GetUInt32();
        weekCount = loadWeekly ? fields[3].GetUInt32() : 0;
        totalCount = fields[4].GetUInt32();
        currencyFlags = fields[5].GetUInt8();
    }

    uint32 count, weekCount, totalCount;
    uint8 currencyFlags;
};

class SERVER_DECL PlayerCurrency
{
public:
    PlayerCurrency(Player *player);
    ~PlayerCurrency();

    void Update();

    void LoadFromDB(time_t lastSavedWeek, QueryResult *result);
    void SaveToDB(QueryBuffer *buff);

    void SendInitialCurrency();

    bool HasCurrency(uint32 currency, uint32 amount);
    bool HasTotalCurrency(uint32 currency, uint32 amount);

    void AddCurrency(uint32 currency, uint32 amount, bool silent = false);
    void RemoveCurrency(uint32 currency, uint32 amount);

    void SetCurrencyFlags(uint32 currency, uint8 flags);

protected:
    void _SendCurrencyUpdate(CurrencyTypeEntry *entry, CurrencyData *data, bool silent);

private:
    Player *m_player;

    time_t lastWeekStart;
    std::map<uint32, CurrencyData> m_currencies;
};
