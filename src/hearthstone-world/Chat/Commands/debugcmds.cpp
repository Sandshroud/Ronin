/***
 * Demonstrike Core
 */

/////////////////////////////////////////////////
//  Debug Chat Commands
//

#include "StdAfx.h"

bool ChatHandler::HandleDebugInFrontCommand(const char* args, WorldSession *m_session)
{
    Unit* unit = getSelectedUnit(m_session, true);
    if(unit == NULL)
        return false;

    Player* plr = m_session->GetPlayer();
    BlueSystemMessage(m_session, "Test1 should equal Test2");
    SystemMessage(m_session, format("Test1: Target is %s you.", (plr->isTargetInFront(unit) ? "infront of" : "behind")).c_str());
    SystemMessage(m_session, format("Test2: You are %s the target.", (plr->isInFrontOfTarget(unit) ? "infront of" : "behind")).c_str());

    BlueSystemMessage(m_session, "Test3 should equal Test4");
    SystemMessage(m_session, format("Test3: Target is %s you.", (plr->isTargetInBack(unit) ?  "behind" : "infront of")).c_str());
    SystemMessage(m_session, format("Test4: You are %s the target.", (plr->isInBackOfTarget(unit) ?  "behind" : "infront of")).c_str());
    return true;
}

bool ChatHandler::HandleShowReactionCommand(const char* args, WorldSession *m_session)
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

    SystemMessage(m_session,  format("Sent forced reaction %u from %s.", Reaction, creature->GetName()).c_str());
    return true;
}

bool ChatHandler::HandleDistanceCommand(const char* args, WorldSession *m_session)
{
    Unit* pUnit = getSelectedChar(m_session, false);
    if(pUnit == NULL)
    {
        pUnit = getSelectedCreature(m_session, false);
        if(pUnit == NULL)
        {
            m_session->GetPlayer()->BroadcastMessage("You must select a Unit.");
            return false;
        }
    }

    SystemMessage(m_session, format("Distance: %f", m_session->GetPlayer()->CalcDistance(pUnit)).c_str());
    return true;
}

bool ChatHandler::HandleMoveInfoCommand(const char* args, WorldSession *m_session)
{
    Player* plr = m_session->GetPlayer();
    Creature* creature = getSelectedCreature(m_session, true);
    if(creature == NULL)
        return false;

    Unit *unitToFollow = creature->GetAIInterface()->getUnitToFollow();
    SystemMessage(m_session, "Move Info:");
    if(unitToFollow != NULL)
        GreenSystemMessage(m_session, format("Following Unit: Low: %u; High: %u;", unitToFollow->GetLowGUID(), unitToFollow->GetHighGUID()).c_str());
    GreenSystemMessage(m_session, format("Distance is: %f;", plr->CalcDistance(creature)).c_str());
    GreenSystemMessage(m_session, format("Mob Facing Player: %s; Player Facing Mob %s;", (creature->isTargetInFront(plr) ? "true" : "false"),
        (plr->isTargetInFront(creature) ? "true" : "false")).c_str());
    GreenSystemMessage(m_session, format("Attackers Count: %u;", uint32(creature->GetAIInterface()->getAITargetsCount())).c_str());
    GreenSystemMessage(m_session, format("Creature State: %u; Run: %s;", creature->GetAIInterface()->m_creatureState,
        (creature->GetAIInterface()->getMoveRunFlag() ? "true" : "false")).c_str());
    GreenSystemMessage(m_session, format("AIState: %u; AIType: %u;", creature->GetAIInterface()->getAIState(), creature->GetAIInterface()->getAIType()).c_str());
    GreenSystemMessage(m_session, format("Movetype: %u; Current Waypoint: %u", creature->GetAIInterface()->getMoveType(),
        creature->GetAIInterface()->getCurrentWaypoint()).c_str());
    return true;
}

bool ChatHandler::HandleDebugSetPhase(const char* args, WorldSession *m_session)
{
    Object* pObj =  getSelectedCreature(m_session, false);
    if(!pObj)
    {
        pObj = getSelectedChar(m_session, false);
        if(!pObj)
        {
            pObj = m_session->GetPlayer()->m_GM_SelectedGO;
            if(!pObj)
            {
                m_session->GetPlayer()->BroadcastMessage("You must select an Object.");
                return true;
            }
        }
    }

    int32 phaseId = 0;
    int save = 0;
    if(sscanf(args, "%u %u", &phaseId, &save) < 1)
        return false;

    if(phaseId < -1)
    {
        m_session->GetPlayer()->BroadcastMessage("You must specify a valid phase id.");
        return true;
    }

    if(phaseId == 0)
    {
        pObj->SetPhaseMask(1);
        m_session->GetPlayer()->BroadcastMessage("Resetting Phase.");
        return true;
    }

    if(pObj->IsPlayer())
        TO_PLAYER(pObj)->SetPhaseMask( phaseId, (save ? true : false) );
    else
        pObj->SetPhaseMask( phaseId );
    if(save && !pObj->IsPlayer())
    {
        if(pObj->IsGameObject())
            TO_GAMEOBJECT(pObj)->SaveToDB();
        else if(pObj->IsCreature())
            TO_CREATURE(pObj)->SaveToDB();
    }

    m_session->GetPlayer()->BroadcastMessage("Target's phase altered successfully.");
    return true;
}

bool ChatHandler::HandleFaceCommand(const char* args, WorldSession *m_session)
{
    Creature* obj = getSelectedCreature(m_session, false);
    if(obj == NULL)
    {
        SystemMessage(m_session,  "You should select a creature.");
        return true;
    }

    uint32 Orentation = 0;
    char* pOrentation = strtok((char*)args, " ");
    if (pOrentation)
        Orentation = atoi(pOrentation);

    /* Convert to Blizzards Format */
    float theOrientation = Orentation ? Orentation/(360/float(6.28)) : 0.0f;

    obj->SetPosition(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), theOrientation);
    SystemMessage(m_session, "Facing sent.");
    return true;
}

bool ChatHandler::HandleSetBytesCommand(const char* args, WorldSession *m_session)
{
    Object* obj = getSelectedUnit(m_session, false);
    if(obj == NULL)
        obj = m_session->GetPlayer();

    uint32 BytesIndex, RealBytesIndex = 0;
    uint8 byte1, byte2, byte3, byte4;
    if(sscanf(args, "%u %u %u %u %u", &BytesIndex, &byte1, &byte2, &byte3, &byte4) < 5)
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
            RedSystemMessage(m_session, "You must supply either a simple byte field value(0, 1, 2) or a valid byte field.");
            return false;
        }break;
    }

    obj->SetUInt32Value(RealBytesIndex, bytevalue);
    SystemMessage(m_session, format("Set Field %u bytes to %u(%u %u %u %u)", BytesIndex, bytevalue, byte1, byte2, byte3, byte4).c_str());
    return true;
}

bool ChatHandler::HandleGetBytesCommand(const char* args, WorldSession *m_session)
{
    Object* obj = getSelectedUnit(m_session, false);
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
            RedSystemMessage(m_session, "You must supply either a simple byte field value(0, 1, 2) or a valid byte field.");
            return false;
        }break;
    }

    uint32 byteValue = obj->GetUInt32Value(RealBytesIndex);
    SystemMessage(m_session, format("Bytes for Field %u are %u(%u %u %u %u)", BytesIndex, byteValue,
        uint8(byteValue & 0xFF), uint8((byteValue >> 8) & 0xFF), uint8((byteValue >> 16) & 0xFF), uint8((byteValue >> 24) & 0xFF)).c_str());
    return true;
}

bool ChatHandler::HandleDebugLandWalk(const char* args, WorldSession *m_session)
{
    Player* chr = getSelectedChar(m_session);
    if (chr == NULL) // Ignatich: what should NOT happen but just in case...
    {
        SystemMessage(m_session, "No character selected.");
        return false;
    }

    chr->SetMovement(MOVE_LAND_WALK,8);
    SystemMessage(m_session, "Land Walk Test Ran.");
    return true;
}

bool ChatHandler::HandleDebugWaterWalk(const char* args, WorldSession *m_session)
{
    Player* chr = getSelectedChar(m_session);
    if (chr == NULL) // Ignatich: what should NOT happen but just in case...
    {
        SystemMessage(m_session, "No character selected.");
        return false;
    }

    chr->SetMovement(MOVE_WATER_WALK, 4);
    SystemMessage(m_session,  "Water Walk Test Ran.");
    return true;
}

bool ChatHandler::HandleDebugUnroot(const char* args, WorldSession *m_session)
{
    Player* chr = getSelectedChar(m_session);
    if (chr == NULL) // Ignatich: what should NOT happen but just in case...
    {
        SystemMessage(m_session,  "No character selected.");
        return false;
    }

    chr->SetMovement(MOVE_UNROOT,5);
    SystemMessage(m_session, "UnRoot Test Ran.");
    return true;
}

bool ChatHandler::HandleDebugRoot(const char* args, WorldSession *m_session)
{
    Player* chr = getSelectedChar(m_session);
    if (chr == NULL) // Ignatich: what should NOT happen but just in case...
    {
        SystemMessage(m_session, "No character selected.");
        return true;
    }

    chr->SetMovement(MOVE_ROOT,1);
    SystemMessage(m_session, "Root Test Ran.");
    return true;
}

bool ChatHandler::HandleAggroRangeCommand(const char* args, WorldSession *m_session)
{
    Player* plr = m_session->GetPlayer();
    Unit* obj = getSelectedCreature(m_session, false);
    if(obj == NULL)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    float aggroRange = obj->GetAIInterface()->_CalcAggroRange(plr);
    SystemMessage(m_session, "Aggrorange is %f", sqrtf(aggroRange));
    return true;
}

bool ChatHandler::HandleKnockBackCommand(const char* args, WorldSession *m_session)
{
    float hspeed, vspeed;
    Unit* target = getSelectedChar(m_session, true);
    if(sscanf(args, "%f %f", &hspeed, &vspeed) != 2)
        return false;

    target->knockback(hspeed, vspeed, false);
    return true;
}

bool ChatHandler::HandleFadeCommand(const char* args, WorldSession *m_session)
{
    if(!args)
        return false;

    Unit* target = getSelectedUnit(m_session, true);
    if(target == NULL)
        return true;

    target->ModThreatModifier(atoi(args));
    SystemMessage(m_session, "Threat is now reduced by: %i%", target->GetThreatModifier());
    return true;
}

bool ChatHandler::HandleThreatModCommand(const char* args, WorldSession *m_session)
{
    if(!args)
        return false;

    Unit* target = getSelectedUnit(m_session, true);
    if(target == NULL)
        return true;

    target->ModGeneratedThreatModifier(atoi(args));
    SystemMessage(m_session, "New threat caused is now reduced by: i%", target->GetGeneratedThreatModifier());
    return true;
}

bool ChatHandler::HandleCalcThreatCommand(const char* args, WorldSession *m_session)
{
    Unit* target = getSelectedCreature(m_session, false);
    if(!target)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    char* dmg = strtok((char*)args, " ");
    if(!dmg)
        return false;
    char* spellId = strtok(NULL, " ");
    if(!spellId)
        return false;
    SpellEntry* sp = dbcSpell.LookupEntry( atoi( spellId ) );
    if(!sp)
        return false;

    uint32 threat = target->GetAIInterface()->_CalcThreat(atol(dmg), sp, m_session->GetPlayer());
    SystemMessage(m_session, "Generated threat is: %u", threat);
    return true;
}

bool ChatHandler::HandleThreatListCommand(const char* args, WorldSession *m_session)
{
    Unit* target = NULLUNIT;
    target = m_session->GetPlayer()->GetMapMgr()->GetUnit(m_session->GetPlayer()->GetSelection());
    if(!target)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    std::stringstream sstext;
    sstext << "threatlist of creature: " << GUID_LOPART(m_session->GetPlayer()->GetSelection()) << " " << GUID_HIPART(m_session->GetPlayer()->GetSelection()) << '\n';
    TargetMap::iterator itr;
    for(itr = target->GetAIInterface()->GetAITargets()->begin(); itr != target->GetAIInterface()->GetAITargets()->end();)
    {
        if(!itr->second)
        {
            ++itr;
            continue;
        }
        sstext << "guid: " << itr->first->GetGUID() << " | threat: " << itr->second << "| threat after mod: " << (itr->second + itr->first->GetThreatModifier()) << "\n";
        ++itr;
    }

    SendMultilineMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleModifyBitCommand(const char* args, WorldSession* m_session)
{
    return false;

/*  Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if(!(obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)))
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = TO_OBJECT(m_session->GetPlayer());

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
        SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    if (bit < 1 || bit > 32)
    {
        SystemMessage(m_session, "Incorrect values.");
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

    SystemMessage(m_session, buf);
    return true;*/
}

bool ChatHandler::HandleModifyValueCommand(const char* args,  WorldSession* m_session)
{
    return false;
/*
    Object* obj;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if(!(obj = TO_OBJECT(m_session->GetPlayer())->GetMapMgr()->GetUnit(guid)))
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = TO_OBJECT(m_session->GetPlayer());

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
        SystemMessage(m_session, "Incorrect Field.");
        return true;
    }

    char buf[256];
    uint32 oldValue = obj->GetUInt32Value(field);
    obj->SetUInt32Value(field,value);

    snprintf((char*)buf,256,"Set Field %i from %i to %i.", (unsigned int)field, (unsigned int)oldValue, (unsigned int)value);

    if( obj->IsPlayer() )
        TO_PLAYER( obj )->UpdateChances();

    SystemMessage(m_session, buf);

    return true;*/
}

bool ChatHandler::HandleDebugGoDamage(const char* args, WorldSession *m_session)
{
    GameObject* GObj = m_session->GetPlayer()->m_GM_SelectedGO;
    if( !GObj )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    uint32 damage = (float)atof(args);
    GObj->TakeDamage(uint32(damage),  m_session->GetPlayer(), m_session->GetPlayer(), 5555);
    BlueSystemMessage(m_session, "Gameobject Has Taken %u", damage);
    return true;
}

bool ChatHandler::HandleDebugGoRepair(const char* args, WorldSession *m_session)
{
    GameObject* GObj = m_session->GetPlayer()->m_GM_SelectedGO;
    if( !GObj )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    GObj->Rebuild();
    BlueSystemMessage(m_session, "Gameobject rebuilt.");
    return true;

}

bool ChatHandler::HandleSetPlayerStartLocation(const char* args, WorldSession *m_session)
{
    Player* p = m_session->GetPlayer();
    uint32 team[2][5] = { { RACE_HUMAN, RACE_DWARF, RACE_NIGHTELF, RACE_GNOME, RACE_DRAENEI }, { RACE_ORC, RACE_UNDEAD, RACE_TAUREN, RACE_TROLL, RACE_BLOODELF} };
    stringstream ss;
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
