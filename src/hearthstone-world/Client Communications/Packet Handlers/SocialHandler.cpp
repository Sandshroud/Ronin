/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleFriendListOpcode( WorldPacket & recv_data )
{
    uint32 flag;
    recv_data >> flag;
    _player->Social_SendFriendList( flag );
}

void WorldSession::HandleAddFriendOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_ADD_FRIEND"  );

    std::string name, note;
    recv_data >> name >> note;

    _player->Social_AddFriend(name, note);
}

void WorldSession::HandleDelFriendOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_DEL_FRIEND"  );

    WoWGuid FriendGuid;
    recv_data >> FriendGuid;

    _player->Social_RemoveFriend(FriendGuid);
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_ADD_IGNORE" );

    std::string ignoreName;
    recv_data >> ignoreName;

    _player->Social_AddIgnore(ignoreName);
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_DEL_IGNORE" );

    WoWGuid IgnoreGuid;
    recv_data >> IgnoreGuid;

    _player->Social_RemoveIgnore(IgnoreGuid);
}

void WorldSession::HandleSetFriendNote(WorldPacket & recv_data)
{
    WoWGuid guid;
    std::string note;

    recv_data >> guid >> note;
    _player->Social_SetNote(guid, note);
}
