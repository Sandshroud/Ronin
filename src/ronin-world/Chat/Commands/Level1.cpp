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
//  GM Chat Commands
//

#include "StdAfx.h"

bool ChatHandler::HandleAnnounceCommand(const char* args, WorldSession *m_session)
{
    if( !*args || strlen(args) < 3 || strchr(args, '%'))
    {
        m_session->SystemMessage("Announces cannot contain the %% character and must be at least 3 characters.");
        return true;
    }

    char msg[1024];
    snprintf(msg, 1024, "[Server Notice]%s %s: %s|r", MSG_COLOR_GREEN, m_session->GetPlayer()->GetName(), args);
    sWorld.SendWorldText(msg); // send message
    sWorld.LogGM(m_session, "used announce command, [%s]", args);
    return true;
}

bool ChatHandler::HandleAdminAnnounceCommand(const char* args, WorldSession *m_session)
{
    if(!*args || !m_session->CanUseCommand('z'))
        return false;

    char GMAnnounce[1024];
    snprintf(GMAnnounce, 1024, "%s[Admin] %s|Hplayer:%s|h[%s]|h:%s %s", MSG_COLOR_GOLD, MSG_COLOR_ORANGEY, m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), MSG_COLOR_LIGHTBLUE, args);
    sWorld.SendGMWorldText(GMAnnounce, true);
    return true;
}

bool ChatHandler::HandleGMAnnounceCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    char GMAnnounce[1024];
    snprintf(GMAnnounce, 1024, "%s[GM] %s|Hplayer:%s|h[%s]|h:%s %s", MSG_COLOR_GREEN, MSG_COLOR_TORQUISEBLUE, m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), MSG_COLOR_YELLOW, args);
    sWorld.SendGMWorldText(GMAnnounce);
    return true;
}

bool ChatHandler::HandleWAnnounceCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    char pAnnounce[1024];
    std::string input2;

    input2 = "|cffff6060<";
    if(m_session->CanUseCommand('z')) input2+="Admin";
    else if(m_session->GetPermissionCount()) input2+="GM";
    input2+=">|r|c1f40af20";
    input2+=m_session->GetPlayer()->GetName();
    input2+=":|r ";
    snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), args);

    sWorld.SendWorldWideScreenText(pAnnounce); // send message
    sWorld.LogGM(m_session, "used wannounce command [%s]", args);
    return true;
}

bool ChatHandler::HandleGMOnCommand(const char* args, WorldSession *m_session)
{
    Player* gm = m_session->GetPlayer();
    if(gm->hasGMTag())
        RedSystemMessage(m_session, "Permission flags are already set. Use .gm off to disable them.");
    else
    {
        if(m_session->CanUseCommand('z') && !gm->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER))
        {
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);           // <GM>
            gm->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);       // <Dev>
        }
        else
        {
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);    // <Dev>
            gm->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);              // <GM>
        }

        BlueSystemMessage(m_session, "Permission flags set. It will appear above your name and in chat messages until you use .gm off.");
        gm->GetMapInstance()->ChangeObjectLocation(gm);
    }
    return true;
}

bool ChatHandler::HandleGMOffCommand(const char* args, WorldSession *m_session)
{
    Player* gm = m_session->GetPlayer();
    if(!gm->hasGMTag())
        RedSystemMessage(m_session, "Permission flags not set. Use .gm on to enable it.");
    else
    {
        gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);           // <GM>
        gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);    // <Dev>
        BlueSystemMessage(m_session, "Permission flags removed. Tags will no longer show in chat messages or above your name.");
        gm->GetMapInstance()->ChangeObjectLocation(gm);
    }
    return true;
}

bool ChatHandler::HandleGMSightTypeCommand(const char *args, WorldSession *m_session)
{
    Player* gm = m_session->GetPlayer();
    uint32 sightType = 0; uint32 arg1 = 0;
    if(sscanf(args, "%u %u", &sightType, &arg1) < 1 || !gm->isGM())
        return false;

    gm->setGMEventSight(0);
    gm->setGMPhaseSight(0);
    static const char *sightNames[] = {"Disabled", "Event", "Phase", "Death"};
    switch(sightType)
    {
    case 1: gm->setGMEventSight(arg1); break;
    case 2: gm->setGMPhaseSight(arg1); break;
    case 3: break; // Death sight
    default: sightType = 0; break;
    }

    gm->setGMSight(sightType);
    GreenSystemMessage(m_session, "GM sight set to %s", sightNames[sightType]);
    gm->GetMapInstance()->ChangeObjectLocation(gm);
    return true;
}

bool ChatHandler::HandleGPSCommand(const char* args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();
    WorldObject* obj = getSelectedUnit(m_session, false);
    if(obj == NULL && (plr->m_selectedGo.empty() || (obj = plr->GetInRangeObject<WorldObject>(plr->m_selectedGo)) == NULL))
        obj = plr;

    char buf[256];
    snprintf((char*)buf, 256, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%u |cffffffffInst: |cff00ff00%u |cffffffffPhase: |cff00ff00%u|r", obj->GetMapId(), obj->GetInstanceID(), obj->GetPhaseMask());
    SystemMessage(m_session, buf);
    snprintf((char*)buf, 256, "|cffffffff WMO: |cff00ff00[%u]%s |r", obj->GetWMOId(), "");
    SystemMessage(m_session, buf);

    snprintf((char*)buf, 256, "|cffffffff Area: |cff00ff00%u |cffffffffZone: |cff00ff00%u |cffffffffAreaFlags: |cff00ff00%u |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f|r", obj->GetAreaId(), obj->GetZoneId(), obj->GetAreaFlags(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
    SystemMessage(m_session, buf);

    return true;
}


bool ChatHandler::HandleKickCommand(const char* args, WorldSession *m_session)
{
    char pname[20];
    if(sscanf(args, "%s", &pname) != 1)
        return false;

    if(!pname)
    {
        RedSystemMessage(m_session, "No name specified.");
        return true;
    }

    Player* chr = objmgr.GetPlayer((char*)pname, false);
    if (chr)
    {
        char* reason = strtok((char*)args, "");
        std::string kickreason = "No reason";
        if(reason)
            kickreason = reason;
        if(!m_session->CanUseCommand('z') && chr->GetSession()->CanUseCommand('z'))
        {
            RedSystemMessage(m_session, "You cannot kick %s, as they are a higher level gm than you.", chr->GetName());
            sWorld.LogGM(m_session, "Attempted to kicked admin %s from the server for %s", chr->GetName(), kickreason.c_str());
            return true;
        }
        BlueSystemMessage(m_session, "Kicked %s from the server for \"%s\".", chr->GetName(), kickreason.c_str());
        sWorld.LogGM(m_session, "Kicked player %s from the server for %s", chr->GetName(), kickreason.c_str());
        char msg[200];
        snprintf(msg, 200, "%s%s was kicked by %s (%s)", MSG_COLOR_WHITE, chr->GetName(), m_session->GetPlayer()->GetName(), kickreason.c_str());
        sWorld.SendWorldText(msg, NULL);
        SystemMessageToPlr(chr, "You are being kicked from the server by %s. Reason: %s", m_session->GetPlayer()->GetName(), kickreason.c_str());
        chr->Kick(6000);
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Player is not online at the moment.");
        return true;
    }
}

bool ChatHandler::HandleItemInfoCommand(const char *args, WorldSession *m_session)
{
    if(strlen(args) < 1)
        return false;

    uint32 itemid = 0;
    if(sscanf(args, "%u", &itemid) < 1)
    {
        // check for item link
        uint16 ofs = GetItemIDFromLink(args, &itemid);
        if(itemid == 0)
            return false;
    }

    ItemPrototype *proto = sItemMgr.LookupEntry(itemid);
    if(proto == NULL)
        RedSystemMessage(m_session, "Item %d is not a valid item!",itemid);
    else
    {
        SystemMessage(m_session, "Item Info for %s", proto->ConstructItemLink(0, 0, 1).c_str());
        SystemMessage(m_session, "ID:%u Class:%u SubClass:%u InventoryType:%u Display:%u", proto->ItemId, proto->Class, proto->SubClass, proto->InventoryType, proto->DisplayInfoID);
        std::string extra;
        char buff[45];
        for(uint8 i = 0; i < 10; i++)
        {
            if(proto->Stats[i].Value == 0)
                continue;
            sprintf(buff, "Stat%u:%u", i, proto->Stats[i].Type);
            if(extra.length())
                extra.append(" ");
            extra.append(buff);
        }
        for(uint8 i = 0; i < 3; i++)
        {
            if(proto->ItemSocket[i] == 0)
                continue;
            sprintf(buff, "Socket%u:%u", i, proto->ItemSocket[i]);
            if(extra.length())
                extra.append(" ");
            extra.append(buff);
        }
        if(proto->SocketBonus)
        {
            sprintf(buff, "SocketBonus:%u", proto->SocketBonus);
            if(extra.length())
                extra.append(" ");
            extra.append(buff);
        }
        SystemMessage(m_session, extra.c_str());
    }
    return true;
}

bool ChatHandler::HandleAddInvItemCommand(const char *args, WorldSession *m_session)
{
    uint32 itemid, count = 1;
    int32 randomprop = 0;

    if(strlen(args) < 1)
        return false;

    if(sscanf(args, "%u %u %i", &itemid, &count, &randomprop) < 1)
    {
        // check for item link
        uint16 ofs = GetItemIDFromLink(args, &itemid);
        if(itemid == 0)
            return false;
        sscanf(args+ofs,"%u %i", &count, &randomprop); // these may be empty
    }

    if(count < 1)
        count = 1;

    Player* chr = getSelectedChar(m_session);
    if (chr == NULL)
        return true;

    if(ItemPrototype* it = sItemMgr.LookupEntry(itemid))
    {
        sWorld.LogGM(m_session, "used add item command, item id %u [%s] to %s", it->ItemId, it->Name.c_str(), chr->GetName());

        if(!chr->GetInventory()->AddItemById(itemid, count, randomprop, false, m_session->GetPlayer()))
        {
            m_session->SendNotification("No free slots were found in your inventory!");
            return true;
        }

        std::string itemlink = it->ConstructItemLink(randomprop, it->RandomSuffixId, count);
        if(chr->GetSession() != m_session) // Since we get that You Recieved Item bullcrap, we don't need this.
        {
            SystemMessage(m_session, "Adding item %u %s to %s's inventory.", it->ItemId, itemlink.c_str(), chr->GetName());
            SystemMessageToPlr(chr, "%s added item %u %s to your inventory.", m_session->GetPlayer()->GetName(), itemid, itemlink.c_str());
        } else SystemMessage(m_session, "Adding item %u %s to your inventory.", it->ItemId, itemlink.c_str());
        return true;
    }

    RedSystemMessage(m_session, "Item %d is not a valid item!",itemid);
    return true;
}

bool ChatHandler::HandleSummonCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    sWorld.LogGM(m_session, "summoned %s", args);

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {
        // send message to user
        char buf[256];
        char buf0[256];
        snprintf((char*)buf,256, "You are summoning %s.", chr->GetName());
        SystemMessage(m_session, buf);

        // send message to player
        snprintf((char*)buf0,256, "You are being summoned by %s.", m_session->GetPlayer()->GetName());
        SystemMessageToPlr(chr, buf0);

        Player* plr = m_session->GetPlayer();

        if ( plr->GetMapId() == chr->GetMapId() && plr->GetInstanceID() == chr->GetInstanceID() )
            chr->SafeTeleport(plr->GetMapId(),plr->GetInstanceID(),plr->GetPosition());
        else chr->SafeTeleport(plr->GetMapInstance(), plr->GetPosition());

        sWorld.LogGM(m_session, "Summoned player %s", plr->GetName());
    }
    else
    {
        PlayerInfo * pinfo = objmgr.GetPlayerInfoByName(args);
        if(!pinfo)
        {
            char buf[256];
            snprintf((char*)buf,256,"Player (%s) does not exist.", args);
            SystemMessage(m_session, buf);
        }
        else
        {
            Player* pPlayer = m_session->GetPlayer();
            char query[512];
            snprintf((char*) &query,512, "UPDATE character_data SET mapId = %u, positionX = %f, positionY = %f, positionZ = %f, zoneId = %u, lastOnline = '%llu' WHERE guid = %u;", pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetZoneId(), UNIXTIME, pinfo->charGuid.getLow());
            CharacterDatabase.Execute(query);
            char buf[256];
            snprintf((char*)buf,256,"(Offline) %s has been summoned.", pinfo->charName.c_str());
            SystemMessage(m_session, buf);
            sWorld.LogGM(m_session, "Summoned offline player %s", pinfo->charName.c_str());
        }
    }
    return true;
}


bool ChatHandler::HandleAppearCommand(const char* args, WorldSession *m_session)
{
    if( !*args )
        return false;

    Player* chr = objmgr.GetPlayer( args, false );
    if(chr && chr->IsInWorld())
    {
        SystemMessage(m_session, "Appearing at %s's location.", chr->GetName());

        //If the GM is on the same map as the player, use the normal safeteleport method
        if ( m_session->GetPlayer()->GetMapId() == chr->GetMapId() && m_session->GetPlayer()->GetInstanceID() == chr->GetInstanceID() )
            m_session->GetPlayer()->SafeTeleport(chr->GetMapId(),chr->GetInstanceID(),chr->GetPosition());
        else
            m_session->GetPlayer()->SafeTeleport(chr->GetMapInstance(), chr->GetPosition());
        //The player and GM are not on the same map. We use this method so we can port to BG's (Above method doesn't support them)
        sWorld.LogGM(m_session, "Appeared to %s", chr->GetName());
    }
    else
    {
        std::stringstream ss;
        PlayerInfo* PI = objmgr.GetPlayerInfoByName(args);
        if(PI)
        {
            if(!chr) // Send message telling
                ss << "Player " << args << " is not logged in.";
            else ss << "Player " << args << " is not available.";

            if(PI->lastPositionX != 0.0f && PI->lastPositionY != 0.0f)
            {
                m_session->GetPlayer()->SafeTeleport(PI->lastMapID, PI->lastInstanceID, PI->lastPositionX, PI->lastPositionY, PI->lastPositionZ+1.0f, PI->lastOrientation);
                ss << "\nTeleporting to last known location of player " << args;
                sWorld.LogGM(m_session, "Appeared to the last known location of %s", args);
            }
        }
        else
            ss << "Player " << args << " does not exist.";

        SystemMessage(m_session, ss.str().c_str());
    }

    return true;
}

bool ChatHandler::HandleTeleportCommand(const char* args, WorldSession *m_session)
{
    if( !*args || m_session->GetPlayer() == NULL)
        return false;

    Player *plr = m_session->GetPlayer();
    std::stringstream ss;
    ss << "SELECT MapId, positionX, positionY, positionZ, name FROM recall WHERE `name` LIKE '%" << args << "%'";
    // Queries using LIKE have to pass through NA to avoid VAlist breaking them
    if(QueryResult *result = WorldDatabase.QueryNA(ss.str().c_str()))
    {
        const char *name = result->Fetch()[4].GetString();
        uint32 mapId = result->Fetch()[0].GetUInt32(); LocationVector loc(result->Fetch()[1].GetFloat(), result->Fetch()[2].GetFloat(), result->Fetch()[3].GetFloat());
        delete result;

        if(mapId == plr->GetMapId())
            plr->Teleport(loc.x, loc.y, loc.z, 0.f);
        else plr->_Relocate(mapId, loc, true, 0);
        return true;
    }

    return false;
}

bool ChatHandler::HandleTeleportXYZCommand(const char* args, WorldSession *m_session)
{
    if( !*args || m_session->GetPlayer() == NULL)
        return false;

    Player *plr = m_session->GetPlayer();
    int32 mapId = -1; LocationVector loc;
    if(sscanf(args, "%f %f %f %u", &loc.x, &loc.y, &loc.z, &mapId) < 3)
        return false;
    else if(mapId == -1)
    {
        sscanf(args, "%f %f %f", &loc.x, &loc.y, &loc.z);
        plr->Teleport(loc.x, loc.y, loc.z, 0.f);
        return true;
    }

    plr->_Relocate(mapId, loc, true, 0);
    return true;
}

bool ChatHandler::HandleTaxiCheatCommand(const char* args, WorldSession *m_session)
{
    if (!*args)
    {
        RedSystemMessage(m_session, "You must supply a flag");
        return true;
    }

    int flag = atoi((char*)args);
    Player* chr = getSelectedChar(m_session);
    if (chr == NULL)
        return true;

    if(chr->GetSession() != m_session)
    {
        GreenSystemMessage(m_session, "Unlocking all taxi nodes for %s.", chr->GetName());
        GreenSystemMessageToPlr(chr, "%s Unlocked all taxi nodes for you.", m_session->GetPlayer()->GetName());
    } else GreenSystemMessage(m_session, "Unlocking all taxi nodes.");

    UpdateMask mask(8*114);
    if(flag) mask = *sTaxiMgr.GetAllTaxiMasks();
    chr->SetTaxiMask(mask);
    return true;
}

bool ChatHandler::HandleLearnSkillCommand(const char *args, WorldSession *m_session)
{
    uint32 skill, min, max;
    min = max = 1;
    char *pSkill = strtok((char*)args, " ");
    if(!pSkill)
        return false;
    else
        skill = atol(pSkill);

    BlueSystemMessage(m_session, "Adding skill line %d", skill);

    char *pMin = strtok(NULL, " ");
    if(pMin)
    {
        min = atol(pMin);
        char *pMax = strtok(NULL, "\n");
        if(pMax)
            max = atol(pMax);
    } else {
        return false;
    }

    Player* plr = getSelectedChar(m_session, true);
    if(!plr) return false;
    if(plr->GetTypeId() != TYPEID_PLAYER) return false;
    sWorld.LogGM(m_session, "used add skill of %u %u %u on %s", skill, min, max, plr->GetName());

    plr->AddSkillLine(skill, 0, max, min);

    return true;
}

bool ChatHandler::HandleModifySkillCommand(const char *args, WorldSession *m_session)
{
    uint32 skill, min, max;
    min = max = 1;
    char *pSkill = strtok((char*)args, " ");
    if(!pSkill)
        return false;
    else
        skill = atol(pSkill);

    char *pMin = strtok(NULL, " ");
    uint32 cnt = 0;
    if(!pMin)
        cnt = 1;
    else
        cnt = atol(pMin);

    skill = atol(pSkill);

    BlueSystemMessage(m_session, "Modifying skill line %d. Advancing %d times.", skill, cnt);

    Player* plr = getSelectedChar(m_session, true);
    if(!plr) plr = m_session->GetPlayer();
    if(!plr) return false;
    sWorld.LogGM(m_session, "used modify skill of %u %u on %s", skill, cnt, plr->GetName());

    if(!plr->HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line, adding.");
        plr->AddSkillLine(skill, 0, 300, 1);
    } else {
        plr->ModSkillLineAmount(skill, cnt, false);
    }

    return true;
}

/// DGM: Get skill level command for getting information about a skill
bool ChatHandler::HandleGetSkillLevelCommand(const char *args, WorldSession *m_session)
{
    uint32 skill = 0;
    char *pSkill = strtok((char*)args, " ");
    if(!pSkill)
        return false;
    else
        skill = atol(pSkill);

    Player* plr = getSelectedChar(m_session, true);
    if(!plr)
        return false;

    if(skill > SkillNameManager->maxskill)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }

    char * SkillName = SkillNameManager->SkillNames[skill];

    if (SkillName==0)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }

    if (!plr->HasSkillLine(skill))
    {
        BlueSystemMessage(m_session, "Player does not have %s skill.", SkillName);
        return false;
    }

    uint32 nobonus = plr->getSkillLineVal(skill,false);
    uint32 bonus = plr->getSkillLineVal(skill,true) - nobonus;
    uint32 max = plr->getSkillLineMax(skill);

    BlueSystemMessage(m_session, "Player's %s skill has level: %u maxlevel: %u. (+ %u bonus)", SkillName,max,nobonus, bonus);
    return true;
}

bool ChatHandler::HandleGetSkillsInfoCommand(const char *args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(!plr)
        return false;

    uint32 nobonus = 0;
    int32  bonus = 0;
    uint32 max = 0;

    BlueSystemMessage(m_session, "Player: %s has skills", plr->GetName() );

    for (uint32 SkillId = 0; SkillId <= SkillNameManager->maxskill; SkillId++)
    {
        if (plr->HasSkillLine(SkillId))
        {
            char * SkillName = SkillNameManager->SkillNames[SkillId];
            if (!SkillName)
            {
                RedSystemMessage(m_session, "Invalid skill: %u", SkillId);
                continue;
            }

            nobonus = plr->getSkillLineVal(SkillId,false);
            bonus = plr->getSkillLineVal(SkillId,true) - nobonus;
            max = plr->getSkillLineMax(SkillId);

            BlueSystemMessage(m_session, "  %s: Value: %u, MaxValue: %u. (+ %d bonus)", SkillName, nobonus,max, bonus);
        }
    }

    return true;
}


bool ChatHandler::HandleRemoveSkillCommand(const char *args, WorldSession *m_session)
{
    uint32 skill = 0;
    char *pSkill = strtok((char*)args, " ");
    if(!pSkill)
        return false;
    else
        skill = atol(pSkill);
    BlueSystemMessage(m_session, "Removing skill line %d", skill);

    Player* plr = getSelectedChar(m_session, true);
    if(!plr) return true;
    sWorld.LogGM(m_session, "used remove skill of %u on %s", skill, plr->GetName());
    plr->RemoveSkillLine(skill);
    SystemMessageToPlr(plr, "%s removed skill line %d from you. ", m_session->GetPlayer()->GetName(), skill);
    return true;
}

bool ChatHandler::HandleEmoteCommand(const char* args, WorldSession *m_session)
{
    uint32 emote = atoi((char*)args);
    Creature* target = getSelectedCreature(m_session);
    if(!target) 
        return false;

    target->SetUInt32Value(UNIT_NPC_EMOTESTATE, emote);
    if(target->IsSpawn() && m_session->CanUseCommand('z'))
        target->SaveToDB();
    return true;
}

bool ChatHandler::HandleStandStateCommand(const char* args, WorldSession *m_session)
{
    uint32 state = atoi((char*)args);
    Creature* target = getSelectedCreature(m_session);
    if(!target) 
        return false;

    target->SetStandState(state);
    if(target->IsSpawn() && m_session->CanUseCommand('z'))
        target->SaveToDB();
    return true;
}

bool ChatHandler::HandleGenderChanger(const char* args, WorldSession *m_session)
{
    int gender;
    Player* target = objmgr.GetPlayer(m_session->GetPlayer()->GetSelection());
    if(!target) {
        SystemMessage(m_session, "Select A Player first.");
        return true;
    }
    if (!*args)
        gender = (target->getGender()== 1 ? 0 : 1);
    else gender = ( std::min((int)atoi((char*)args),1) > 0 ? 1: 0);


    SystemMessage(m_session, "Gender changed to %u",gender);
    GreenSystemMessageToPlr(target, "%s has changed your gender.", m_session->GetPlayer()->GetName());
    sWorld.LogGM( m_session, "used modify gender on %s", target->GetName());
    return true;
}

bool ChatHandler::HandleModifyGoldCommand(const char* args, WorldSession *m_session)
{
    if ( *args == 0 )
        return false;

    Player* chr = getSelectedChar( m_session, true );
    if( chr == NULL )
        return true;

    int64 total   = atoi( (char*)args );
    uint64 gold   = (uint32) floor( (float)int32abs( total ) / 10000.0f );
    uint64 silver = (uint32) floor( ((float)int32abs( total ) / 100.0f) ) % 100;
    uint64 copper = int32abs2uint32( total ) % 100;

    sWorld.LogGM( m_session, "used modify gold on %s, gold: %i", chr->GetName(), total );

    int64 newgold = chr->GetUInt32Value( PLAYER_FIELD_COINAGE ) + total;

    if(newgold < 0)
    {
        BlueSystemMessage( m_session, "Taking all gold from %s's backpack...", chr->GetName() );
        GreenSystemMessageToPlr(chr, "%s took the all gold from your backpack.", m_session->GetPlayer()->GetName());
        newgold = 0;
    }
    else
    {
        if(total >= 0)
        {
            BlueSystemMessage( m_session, "Adding %u gold, %u silver, %u copper to %s's backpack...", gold, silver, copper, chr->GetName() );
            GreenSystemMessageToPlr( chr, "%s added %u gold, %u silver, %u copper to your backpack.", m_session->GetPlayer()->GetName(), gold, silver, copper );
        }
        else
        {
            BlueSystemMessage( m_session, "Taking %u gold, %u silver, %u copper from %s's backpack...", gold, silver, copper, chr->GetName() );
            GreenSystemMessageToPlr( chr, "%s took %u gold, %u silver, %u copper from your backpack.", m_session->GetPlayer()->GetName(), gold, silver, copper );
        }
    }

    chr->SetUInt64Value( PLAYER_FIELD_COINAGE, newgold );
    return true;
}

bool ChatHandler::HandleUnlearnCommand(const char* args, WorldSession * m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(plr == 0)
        return true;

    uint32 SpellId = atol(args);
    if(SpellId == 0)
        SpellId = GetSpellIDFromLink( args );

    if(SpellId == 0)
    {
        RedSystemMessage(m_session, "You must specify a spell id.");
        return true;
    }

    sWorld.LogGM(m_session, "removed spell %u from %s", SpellId, plr->GetName());

    if(plr->HasSpell(SpellId))
    {
        GreenSystemMessageToPlr(plr, "Removed spell %u.", SpellId);
        plr->removeSpell(SpellId);
    } else RedSystemMessage(m_session, "That player does not have spell %u learnt.", SpellId);

    return true;
}

bool ChatHandler::HandleNpcSpawnLinkCommand(const char* args, WorldSession *m_session)
{
    uint32 id;
    char sql[512];
    Creature* target = m_session->GetPlayer()->GetMapInstance()->GetCreature(m_session->GetPlayer()->GetSelection());
    if (!target)
        return false;

    int valcount = sscanf(args, "%u", (unsigned int*)&id);
    if(valcount && target->IsSpawn())
    {
        snprintf(sql, 512, "UPDATE creature_spawns SET respawnlink = '%u' WHERE id = '%u'", id, target->GetLowGUID());
        WorldDatabase.Execute( sql );
        BlueSystemMessage(m_session, "Spawn linking for this NPC has been updated: %u", id);
    } else RedSystemMessage(m_session, "Sql entry invalid %u", id);
    return true;
}

bool ChatHandler::HandleModifyTPsCommand(const char* args, WorldSession *m_session)
{
    if(!args)
        return false;

    Player * Pl = getSelectedChar(m_session);
    if(!Pl)
        return true;

    uint32 TP1 = 0;

    if(sscanf(args, "%u", &TP1) != 1)
    {
        SystemMessage(m_session, "Enter an amount to modify your target's specs to.");
        return true;
    }

    Pl->GetTalentInterface()->ModTalentPoints(TP1);
    sWorld.LogGM(m_session, "Modified %s talents to %u", Pl->GetName(), TP1);
    return true;
}
