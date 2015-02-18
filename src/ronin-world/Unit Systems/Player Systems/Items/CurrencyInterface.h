/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL PlayerCurrency
{
public:
    PlayerCurrency(Player *player);
    ~PlayerCurrency();

    void SendInitialCurrency();

    bool HasCurrency(uint32 currency, uint32 amount);

    void AddCurrency(uint32 currency, uint32 amount);
    void RemoveCurrency(uint32 currency, uint32 amount);

    void LoadFromDB(QueryResult *result);
    void SaveToDB(QueryBuffer *buff);

private:
    Player *m_player;

    std::set<uint32> m_dirtyCurrencies;
    std::map<uint32, std::pair<uint32, uint32> > m_currencies;
};
