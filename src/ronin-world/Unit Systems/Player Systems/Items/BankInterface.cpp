/***
 * Demonstrike Core
 */

#include "StdAfx.h"

PlayerBank::PlayerBank(Player *pPlayer)
{

}

PlayerBank::~PlayerBank()
{

}

void PlayerBank::LoadFromDB(QueryResult *result)
{

}

void PlayerBank::SaveToDB(QueryBuffer *buf)
{

}

uint32 PlayerBank::BuildCreateUpdateBlocks(ByteBuffer *data)
{
    uint32 count = 0;
    for(ItemDataStorage::iterator itr = m_itemData.begin(); itr != m_itemData.end(); itr++)
        count += sItemMgr.BuildCreateBlockForData(data, m_pOwner, itr->second);
    return count;
}

void PlayerBank::DestroyForPlayer(Player* plr)
{
    if(plr != m_pOwner)
        return;

    WorldPacket data(SMSG_DESTROY_OBJECT, 9);
    data << uint64(0) << uint8(0);
    for(auto itr = m_itemData.begin(); itr != m_itemData.end(); itr++)
    {
        data.put<uint64>(0, itr->second->itemGuid.raw());
        plr->GetSession()->SendPacket( &data );
    }
}
