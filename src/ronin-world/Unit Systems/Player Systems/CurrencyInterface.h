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
