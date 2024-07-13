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

/////////////////////////////////////////////////
//  Debug Chat Commands
//

#include "StdAfx.h"

bool GMWarden::HandleDebugInFrontCommand(const char* args, WorldSession *m_session)
{
    Unit* unit = getSelectedUnit(m_session, true);
    if(unit == NULL)
        return false;

    Player* plr = m_session->GetPlayer();
    sChatHandler.BlueSystemMessage(m_session, "Test1 should equal Test2");
    sChatHandler.SystemMessage(m_session, format("Test1: Target is %s you.", (plr->isTargetInFront(unit) ? "infront of" : "behind")).c_str());
    sChatHandler.SystemMessage(m_session, format("Test2: You are %s the target.", (plr->isInFrontOfTarget(unit) ? "infront of" : "behind")).c_str());

    sChatHandler.BlueSystemMessage(m_session, "Test3 should equal Test4");
    sChatHandler.SystemMessage(m_session, format("Test3: Target is %s you.", (plr->isTargetInBack(unit) ?  "behind" : "infront of")).c_str());
    sChatHandler.SystemMessage(m_session, format("Test4: You are %s the target.", (plr->isInBackOfTarget(unit) ?  "behind" : "infront of")).c_str());
    return true;
}

bool GMWarden::HandleShowReactionCommand(const char* args, WorldSession *m_session)
{
    if(args == NULL)
        return false;

    Creature* creature = getSelectedCreature(m_session, true);
    if(creature == NULL)
        return false;

    uint32 Reaction = atol(args);
    if(!Reaction)
        return false;

    WorldPacket data(SMSG_AI_REACTION, 12);
    data << creature->GetGUID() << uint32(Reaction);
    m_session->SendPacket( &data );

    sChatHandler.SystemMessage(m_session,  format("Sent forced reaction %u from %s.", Reaction, creature->GetName()).c_str());
    return true;
}

bool GMWarden::HandleDistanceCommand(const char* args, WorldSession *m_session)
{
    Unit* pUnit = getSelectedChar(m_session, false, false);
    if(pUnit == NULL)
    {
        pUnit = getSelectedCreature(m_session, false);
        if(pUnit == NULL)
        {
            m_session->GetPlayer()->BroadcastMessage("You must select a Unit.");
            return false;
        }
    }

    sChatHandler.SystemMessage(m_session, "Distance: %f", m_session->GetPlayer()->GetDistanceSq(pUnit));
    return true;
}

bool GMWarden::HandleMoveInfoCommand(const char* args, WorldSession *m_session)
{
    Player* plr = m_session->GetPlayer();
    Creature* creature = getSelectedCreature(m_session, true);
    if(creature == NULL)
        return false;

    sChatHandler.SystemMessage(m_session, "Move Info:");
    sChatHandler.GreenSystemMessage(m_session, format("Distance is: %f;", sqrtf(plr->GetDistanceSq(creature))).c_str());
    sChatHandler.GreenSystemMessage(m_session, format("Mob Facing Player: %s; Player Facing Mob %s;", (creature->isTargetInFront(plr) ? "true" : "false"),
        (plr->isTargetInFront(creature) ? "true" : "false")).c_str());
    return true;
}

bool GMWarden::HandleFaceCommand(const char* args, WorldSession *m_session)
{
    Creature* obj = getSelectedCreature(m_session, false);
    if(obj == NULL)
    {
        sChatHandler.SystemMessage(m_session,  "You should select a creature.");
        return true;
    }

    uint32 Orentation = 0;
    char* pOrentation = strtok((char*)args, " ");
    if (pOrentation)
        Orentation = atoi(pOrentation);

    /* Convert to Blizzards Format */
    float theOrientation = Orentation ? Orentation/(360/float(6.28)) : 0.0f;

    obj->SetPosition(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), theOrientation);
    sChatHandler.SystemMessage(m_session, "Facing sent.");
    return true;
}

bool GMWarden::HandleSetBytesCommand(const char* args, WorldSession *m_session)
{
    WorldObject* obj = getSelectedUnit(m_session, false);
    if(obj == NULL)
        obj = m_session->GetPlayer();

    uint32 BytesIndex, RealBytesIndex = 0;
    uint8 byte1, byte2, byte3, byte4;
    if(sscanf(args, "%u %c %c %c %c", &BytesIndex, &byte1, &byte2, &byte3, &byte4) < 5)
        return false;
    uint32 bytevalue = uint32(uint32(byte1)|uint32(byte2 << 8)|uint32(byte3 << 16)|uint32(byte4 << 24));

    switch(BytesIndex)
    {
    case 0:
        RealBytesIndex = UNIT_FIELD_BYTES_0;
        break;
    case 1:
        RealBytesIndex = UNIT_FIELD_BYTES_1;
        break;
    case 2:
        RealBytesIndex = UNIT_FIELD_BYTES_2;
        break;
    case UNIT_FIELD_BYTES_0:
    case UNIT_FIELD_BYTES_1:
    case UNIT_FIELD_BYTES_2:
        RealBytesIndex = BytesIndex;
        break;
    default:
        {
            sChatHandler.RedSystemMessage(m_session, "You must supply either a simple byte field value(0, 1, 2) or a valid byte field.");
            return false;
        }break;
    }

    obj->SetUInt32Value(RealBytesIndex, bytevalue);
    sChatHandler.SystemMessage(m_session, format("Set Field %u bytes to %u(%u %u %u %u)", BytesIndex, bytevalue, byte1, byte2, byte3, byte4).c_str());
    return true;
}

bool GMWarden::HandleGetBytesCommand(const char* args, WorldSession *m_session)
{
    WorldObject* obj = getSelectedUnit(m_session, false);
    if(obj == NULL)
        obj = m_session->GetPlayer();

    uint32 BytesIndex, RealBytesIndex = 0;
    if(sscanf(args, "%u", &BytesIndex) < 1)
        return false;

    switch(BytesIndex)
    {
    case 0:
        RealBytesIndex = UNIT_FIELD_BYTES_0;
        break;
    case 1:
        RealBytesIndex = UNIT_FIELD_BYTES_1;
        break;
    case 2:
        RealBytesIndex = UNIT_FIELD_BYTES_2;
        break;
    case UNIT_FIELD_BYTES_0:
    case UNIT_FIELD_BYTES_1:
    case UNIT_FIELD_BYTES_2:
        RealBytesIndex = BytesIndex;
        break;
    default:
        {
            sChatHandler.RedSystemMessage(m_session, "You must supply either a simple byte field value(0, 1, 2) or a valid byte field.");
            return false;
        }break;
    }

    uint32 byteValue = obj->GetUInt32Value(RealBytesIndex);
    sChatHandler.SystemMessage(m_session, format("Bytes for Field %u are %u(%u %u %u %u)", BytesIndex, byteValue,
        uint8(byteValue & 0xFF), uint8((byteValue >> 8) & 0xFF), uint8((byteValue >> 16) & 0xFF), uint8((byteValue >> 24) & 0xFF)).c_str());
    return true;
}

bool GMWarden::HandleAggroRangeCommand(const char* args, WorldSession *m_session)
{
    Player* plr = m_session->GetPlayer();
    Creature* obj = getSelectedCreature(m_session, false);
    if(obj == NULL)
    {
        sChatHandler.SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    return true;
}

bool GMWarden::HandleKnockBackCommand(const char* args, WorldSession *m_session)
{
    float hspeed, vspeed;
    Unit* target = getSelectedChar(m_session, true);
    if(sscanf(args, "%f %f", &hspeed, &vspeed) != 2)
        return false;

    target->knockback(hspeed, vspeed, false);
    return true;
}

bool GMWarden::HandleModifyBitCommand(const char* args, WorldSession* m_session)
{
    return false;

/*  WorldObject* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if(!(obj = m_session->GetPlayer()->GetMapInstance()->GetUnit(guid)))
        {
            sChatHandler.SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pBit = strtok(NULL, " ");
    if (!pBit)
        return false;

    uint16 field = atoi(pField);
    uint32 bit   = atoi(pBit);

    if (field < 1 || field >= PLAYER_END)
    {
        sChatHandler.SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    if (bit < 1 || bit > 32)
    {
        sChatHandler.SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    char buf[256];

    if ( obj->HasFlag( field, (1<<(bit-1)) ) )
    {
        obj->RemoveFlag( field, (1<<(bit-1)) );
        snprintf((char*)buf,256, "Removed bit %i in field %i.", (unsigned int)bit, (unsigned int)field);
    }
    else
    {
        obj->SetFlag( field, (1<<(bit-1)) );
        snprintf((char*)buf,256, "Set bit %i in field %i.", (unsigned int)bit, (unsigned int)field);
    }

    sChatHandler.SystemMessage(m_session, buf);
    return true;*/
}

bool GMWarden::HandleModifyValueCommand(const char* args,  WorldSession* m_session)
{
    return false;
/*
    WorldObject* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if(!(obj = m_session->GetPlayer())->GetMapInstance()->GetUnit(guid))
        {
            sChatHandler.SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    } else obj = m_session->GetPlayer();

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pValue = strtok(NULL, " ");
    if (!pValue)
        return false;

    uint16 field = atoi(pField);
    uint32 value   = atoi(pValue);

    if (field < 1 || field >= PLAYER_END)
    {
        sChatHandler.SystemMessage(m_session, "Incorrect Field.");
        return true;
    }

    char buf[256];
    uint32 oldValue = obj->GetUInt32Value(field);
    obj->SetUInt32Value(field,value);

    snprintf((char*)buf,256,"Set Field %i from %i to %i.", (unsigned int)field, (unsigned int)oldValue, (unsigned int)value);

    if( obj->IsPlayer() )
        castPtr<Player>( obj )->UpdateChances();

    sChatHandler.SystemMessage(m_session, buf);

    return true;*/
}

bool GMWarden::HandleDebugGoDamage(const char* args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( GObj == NULL )
    {
        sChatHandler.RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    uint32 damage = (float)atof(args);
    GObj->TakeDamage(uint32(damage),  m_session->GetPlayer(), m_session->GetPlayer(), 5555);
    sChatHandler.BlueSystemMessage(m_session, "Gameobject Has Taken %u", damage);
    return true;
}

bool GMWarden::HandleDebugGoRepair(const char* args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( GObj == NULL )
    {
        sChatHandler.RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    GObj->SetStatusRebuilt();
    sChatHandler.BlueSystemMessage(m_session, "Gameobject rebuilt.");
    return true;

}

bool GMWarden::HandleSetPlayerStartLocation(const char* args, WorldSession *m_session)
{
    Player* p = m_session->GetPlayer();
    uint32 team[2][5] = { { RACE_HUMAN, RACE_DWARF, RACE_NIGHTELF, RACE_GNOME, RACE_DRAENEI }, { RACE_ORC, RACE_UNDEAD, RACE_TAUREN, RACE_TROLL, RACE_BLOODELF} };
    std::stringstream ss;
    ss << "UPDATE playercreateinfo SET mapID = " << p->GetMapId() << ", zoneID = " << p->GetZoneId() << ", positionX = " << p->GetPositionX()
        << ", positionY = " << p->GetPositionY() << ", positionZ = " << p->GetPositionZ() << ", Orientation = " << p->GetOrientation() << " WHERE race IN(";
    bool first = true;
    for(uint8 i = 0; i < 5; i++)
    {
        if(first)
            first = false;
        else
            ss << ", ";
        ss << "'" << team[p->GetTeam()][i] << "'";
    }
    ss << ");";
    WorldDatabase.Execute(ss.str().c_str());

    return true;
}

bool GMWarden::HandleModifySpeedCommand(const char* args, WorldSession *m_session)
{
    if(Unit* target = getSelectedChar(m_session, true))
    {
        float speed = fabs(atof(args));
        target->GetMovementInterface()->SetMoveSpeed(MOVE_SPEED_RUN, speed);
        target->GetMovementInterface()->SetMoveSpeed(MOVE_SPEED_RUN_BACK, speed/3.f);
        return true;
    }
    return false;
}

bool GMWarden::HandleModifySwimSpeedCommand(const char* args, WorldSession *m_session)
{
    if(Unit* target = getSelectedChar(m_session, true))
    {
        float speed = fabs(atof(args));
        target->GetMovementInterface()->SetMoveSpeed(MOVE_SPEED_SWIM, speed);
        target->GetMovementInterface()->SetMoveSpeed(MOVE_SPEED_SWIM_BACK, speed/3.f);
        return true;
    }
    return false;
}

bool GMWarden::HandleModifyFlightSpeedCommand(const char* args, WorldSession *m_session)
{
    if(Unit* target = getSelectedChar(m_session, true))
    {
        float speed = fabs(atof(args));
        target->GetMovementInterface()->SetMoveSpeed(MOVE_SPEED_FLIGHT, speed);
        target->GetMovementInterface()->SetMoveSpeed(MOVE_SPEED_FLIGHT_BACK, speed/3.f);
        return true;
    }
    return false;
}
