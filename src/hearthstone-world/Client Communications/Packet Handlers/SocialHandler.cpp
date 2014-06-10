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

    string name, note;
    recv_data >> name;
    recv_data >> note;

    _player->Social_AddFriend( name.c_str(), note.size() ? note.c_str() : NULL );
}

void WorldSession::HandleDelFriendOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_DEL_FRIEND"  );

    uint64 FriendGuid;
    recv_data >> FriendGuid;

    _player->Social_RemoveFriend( (uint32)FriendGuid );
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_ADD_IGNORE" );

    std::string ignoreName = "UNKNOWN";
    recv_data >> ignoreName;

    _player->Social_AddIgnore( ignoreName.c_str() );
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recv_data )
{
    sLog.Debug( "WORLD"," Received CMSG_DEL_IGNORE" );

    uint64 IgnoreGuid;
    recv_data >> IgnoreGuid;

    _player->Social_RemoveIgnore( (uint32)IgnoreGuid );
}

void WorldSession::HandleSetFriendNote(WorldPacket & recv_data)
{
    uint64 guid;
    string note;

    recv_data >> guid >> note;
    _player->Social_SetNote( (uint32)guid, note.size() ? note.c_str() : NULL );
}
