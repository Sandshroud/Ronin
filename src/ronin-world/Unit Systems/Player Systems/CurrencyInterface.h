/***
 * Demonstrike Core
 */

#pragma once

enum PlayerCurrencyFlag
{
    PLAYERCURRENCY_FLAG_NONE                = 0x0,
    PLAYERCURRENCY_FLAG_UNK1                = 0x1,  // unused?
    PLAYERCURRENCY_FLAG_UNK2                = 0x2,  // unused?
    PLAYERCURRENCY_FLAG_SHOW_IN_BACKPACK    = 0x4,
    PLAYERCURRENCY_FLAG_UNUSED              = 0x8,

    PLAYERCURRENCY_MASK_USED_BY_CLIENT = PLAYERCURRENCY_FLAG_SHOW_IN_BACKPACK | PLAYERCURRENCY_FLAG_UNUSED
};

struct CurrencyData
{
    CurrencyData(uint32 amount) : count(amount), weekCount(0), currencyFlags(0) {};
    uint32 count;
    uint32 weekCount;
    uint8 currencyFlags;
};

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

    std::map<uint32, CurrencyData> m_currencies;
};
