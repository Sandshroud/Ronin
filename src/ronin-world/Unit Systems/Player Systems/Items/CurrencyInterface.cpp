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

void PlayerCurrency::SendInitialCurrency()
{
    WorldPacket data(SMSG_INIT_CURRENCY, 0);

    m_player->SendPacket(&data);
}
