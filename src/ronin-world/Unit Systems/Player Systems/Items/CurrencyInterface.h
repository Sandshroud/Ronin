/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL PlayerCurrency
{
public:
    PlayerCurrency(Player *player);
    ~PlayerCurrency();

    void LoadFromDB(QueryResult *result);
    void SaveToDB(QueryBuffer *buff);

    void SendInitialCurrency();

    bool HasCurrency(uint32 currency, uint32 amount);

    void AddCurrency(uint32 currency, uint32 amount);
    void RemoveCurrency(uint32 currency, uint32 amount);

private:
    Player *m_player;

    std::map<uint32, uint32> m_currencies;
};
