/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleTaxiNodeStatusQueryOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data >> guid;

    uint8 res = 0;
    uint32 curloc = 0;
    Creature *pCreature = NULL;
    if(guid.getHigh() == HIGHGUID_TYPE_UNIT && (pCreature = _player->GetInRangeObject<Creature>(guid)))
        if((curloc = pCreature->GetTaxiNode(_player->GetTeam())) && _player->HasTaxiNode(curloc))
            res = 1;

    WorldPacket data(SMSG_TAXINODE_STATUS, 9);
    data << guid << res;
    SendPacket( &data );
}

void WorldSession::HandleTaxiQueryAvaibleNodesOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    WoWGuid guid;
    recv_data >> guid;
    Creature *pCreature = NULL;
    if(guid.getHigh() == HIGHGUID_TYPE_UNIT && (pCreature = _player->GetInRangeObject<Creature>(guid)))
        pCreature->SendTaxiList(_player);
}

void WorldSession::HandleActivateTaxiOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    sLog.Debug( "WORLD"," Received CMSG_ACTIVATETAXI" );

    uint64 guid;
    uint32 sourcenode, destinationnode;
    recv_data >> guid >> sourcenode >> destinationnode;

    if(GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(sourcenode, destinationnode);
    TaxiNodeEntry* taxinode = dbcTaxiNode.LookupEntry(sourcenode);

    if( !taxinode || !taxipath )
        return;

    uint32 curloc = taxinode->id;
    WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);

    // Check for known nodes
    if (!_player->HasTaxiNode(curloc))
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }

    // Check for valid node
    if (!taxinode)
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }

    if (!taxipath || !taxipath->GetNodeCount())
    {
        data << uint32( 2 );
        SendPacket( &data );
        return;
    }

    if (taxipath->GetID() == 766 || taxipath->GetID() == 767 || taxipath->GetID() == 771 || taxipath->GetID() == 772)
    {
        data << uint32( 2 );
        SendPacket( &data );
        return;
    }

    // Check for gold
    int32 newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - taxipath->GetPrice());
    if(newmoney < 0 )
    {
        data << uint32( 3 );
        SendPacket( &data );
        return;
    }

    // MOUNTDISPLAYID
    // bat: 1566
    // gryph: 1147
    // wyvern: 295
    // hippogryph: 479
    uint32 modelid = 0;
    CreatureData* ctrData = sCreatureDataMgr.GetCreatureData(_player->GetTeam() ? taxinode->mountIdHorde : taxinode->mountIdAlliance);
    if(ctrData == NULL || ((modelid = ctrData->displayInfo[0]) == 0))
    {
        if(_player->GetTeam())
        {
            if( taxinode->mountIdHorde == 2224 )
                modelid = 295; // In case it's a wyvern
            else modelid = 1566; // In case it's a bat or a bad id
        }
        else if( taxinode->mountIdAlliance == 3837 )
            modelid = 479; // In case it's an hippogryph
        else modelid = 1147; // In case it's a gryphon or a bad id
    }

    data << uint32( 0 );
    // 0 Ok
    // 1 Unspecified Server Taxi Error
    // 2.There is no direct path to that direction
    // 3 Not enough Money
    SendPacket( &data );
    sLog.Debug( "WORLD"," Sent SMSG_ACTIVATETAXIREPLY" );

    // 0x000004 locks you so you can't move, no msg_move updates are sent to the server
    // 0x000008 seems to enable detailed collision checking
    // 0x001000 seems to make a mount visible
    // 0x002000 seems to make you sit on the mount, and the mount move with you

    _player->taxi_model_id = modelid;
    GetPlayer()->TaxiStart(taxipath, modelid, 0);
    //sLog.outString("TAXI: Starting taxi trip. Next update in %d msec.", first_node_time);
}

void WorldSession::HandleMultipleActivateTaxiOpcode(WorldPacket & recvPacket)
{
    CHECK_INWORLD_RETURN();
    sLog.Debug( "WORLD"," Received CMSG_ACTIVATETAXI" );

    uint64 guid;
    uint32 nodecount;
    recvPacket >> guid >> nodecount;
    if(nodecount < 2)
        return;

    if(nodecount>12)
    {
        sLog.Debug("WorldSession","CMSG_ACTIVATETAXI: Client disconnected, nodecount: %u", nodecount);
        Disconnect();
        return;
    }

    std::vector<uint32> pathes;
    for(uint32 i = 0; i < nodecount; i++)
        pathes.push_back( recvPacket.read<uint32>() );

    if(GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
        return;

    // get first trip
    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(pathes[0], pathes[1]);
    TaxiNodeEntry* taxinode = dbcTaxiNode.LookupEntry(pathes[0]);

    uint32 curloc = taxinode->id;

    // Check for known nodes
    WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
    if (!_player->HasTaxiNode(curloc))
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }

    // Check for valid node
    if (!taxinode)
    {
        data << uint32( 1 );
        SendPacket( &data );
        return;
    }

    if (!taxipath || !taxipath->GetNodeCount())
    {
        data << uint32( 2 );
        SendPacket( &data );
        return;
    }

    if (taxipath->GetID() == 766 || taxipath->GetID() == 767 || taxipath->GetID() == 771 || taxipath->GetID() == 772)
    {
        data << uint32( 2 );
        SendPacket( &data );
        return;
    }

    uint32 totalcost = taxipath->GetPrice();
    for(uint32 i = 2; i < nodecount; i++)
    {
        TaxiPath * np = sTaxiMgr.GetTaxiPath(pathes[i-1], pathes[i]);
        if(!np) return;
        totalcost += np->GetPrice();
    }

    // Check for gold
    int32 newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - totalcost);
    if(newmoney < 0 )
    {
        data << uint32( 3 );
        SendPacket( &data );
        return;
    }

    // MOUNTDISPLAYID
    // bat: 1566
    // gryph: 1147
    // wyvern: 295
    // hippogryph: 479

    uint32 modelid =0;
    if( _player->GetTeam() )
    {
        if( taxinode->mountIdHorde == 2224 )
            modelid =295; // In case it's a wyvern
        else modelid =1566; // In case it's a bat or a bad id
    }
    else if( taxinode->mountIdAlliance == 3837 )
        modelid =479; // In case it's an hippogryph
    else modelid =1147; // In case it's a gryphon or a bad id

    //GetPlayer( )->setDismountCost( newmoney );

    data << uint32( 0 );
    // 0 Ok
    // 1 Unspecified Server Taxi Error
    // 2.There is no direct path to that direction
    // 3 Not enough Money
    SendPacket( &data );
    sLog.Debug( "WORLD"," Sent SMSG_ACTIVATETAXIREPLY" );

    // 0x001000 seems to make a mount visible
    // 0x002000 seems to make you sit on the mount, and the mount move with you
    // 0x000004 locks you so you can't move, no msg_move updates are sent to the server
    // 0x000008 seems to enable detailed collision checking
    _player->taxi_model_id = modelid;

    // build the rest of the path list
    for(uint32 i = 2; i < nodecount; i++)
    {
        TaxiPath * np = sTaxiMgr.GetTaxiPath(pathes[i-1], pathes[i]);
        if(!np) return;

        if (np->GetID() == 766 || np->GetID() == 767 || np->GetID() == 771 || np->GetID() == 772)
        {
            _player->m_taxiPaths.clear();
            return;
        }

        // add to the list.. :)
        _player->m_taxiPaths.push_back(np);
    }

    // start the first trip :)
    GetPlayer()->TaxiStart(taxipath, modelid, 0);
}
