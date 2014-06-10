/***
 * Demonstrike Core
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
    snprintf(msg, 1024, "[Server Notice]"MSG_COLOR_GREEN" %s: %s|r", m_session->GetPlayer()->GetName(), args);
    sWorld.SendWorldText(msg); // send message
    sWorld.LogGM(m_session, "used announce command, [%s]", args);
    return true;
}

bool ChatHandler::HandleAdminAnnounceCommand(const char* args, WorldSession *m_session)
{
    if(!*args || !m_session->CanUseCommand('z'))
        return false;

    char GMAnnounce[1024];
    snprintf(GMAnnounce, 1024, MSG_COLOR_GOLD"[Admin] "MSG_COLOR_ORANGEY"|Hplayer:%s|h[%s]|h:"MSG_COLOR_LIGHTBLUE" %s", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), args);
    sWorld.SendGMWorldText(GMAnnounce, true);
    return true;
}

bool ChatHandler::HandleGMAnnounceCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    char GMAnnounce[1024];
    snprintf(GMAnnounce, 1024, MSG_COLOR_GREEN"[GM] "MSG_COLOR_TORQUISEBLUE"|Hplayer:%s|h[%s]|h:"MSG_COLOR_YELLOW" %s", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), args);
    sWorld.SendGMWorldText(GMAnnounce);
    return true;
}

bool ChatHandler::HandleWAnnounceCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    char pAnnounce[1024];
    string input2;

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
    if(gm->bGMTagOn)
        RedSystemMessage(m_session, "Permission flags are already set. Use .gm off to disable them.");
    else
    {
        gm->bGMTagOn = true;
        if(m_session->CanUseCommand('z') && !gm->DisableDevTag)
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
    }
    return true;
}

bool ChatHandler::HandleGMOffCommand(const char* args, WorldSession *m_session)
{
    Player* gm = m_session->GetPlayer();
    if(!gm->bGMTagOn)
        RedSystemMessage(m_session, "Permission flags not set. Use .gm on to enable it.");
    else
    {
        gm->bGMTagOn = false;
        gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);           // <GM>
        gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);    // <Dev>
        BlueSystemMessage(m_session, "Permission flags removed. Tags will no longer show in chat messages or above your name.");
    }
    return true;
}

bool ChatHandler::HandleToggleDevCommand(const char* args, WorldSession *m_session)
{
    Player* gm = m_session->GetPlayer();
    if(gm->DisableDevTag)
    {
        if(gm->bGMTagOn)
        {   // Change the flag to the correct one
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);           // <GM>
            gm->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);       // <Dev>
        }
        else
        {   // Remove both flags
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);           // <GM>
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);    // <Dev>
        }

        gm->DisableDevTag = false;
    }
    else
    {
        if(gm->bGMTagOn)
        {   // Change the flag to the correct one
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);    // <GM>
            gm->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);              // <Dev>
        }
        else
        {   // Remove both flags
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);           // <GM>
            gm->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);    // <Dev>
        }

        gm->DisableDevTag = true;
    }
    GreenSystemMessage(m_session, "Toggling Permission flags");
    return true;
}

bool ChatHandler::HandleGPSCommand(const char* args, WorldSession *m_session)
{
    Object* obj = TO_OBJECT(getSelectedUnit(m_session, false));
    if(obj == NULL)
    {
        if(m_session->GetPlayer()->m_GM_SelectedGO)
            obj = m_session->GetPlayer()->m_GM_SelectedGO;
        else
            obj = m_session->GetPlayer();
    }

    char buf[512];
    snprintf((char*)buf, 512, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%u |cffffffffInst: |cff00ff00%u |cffffffffPhase: |cff00ff00%u"
        " |cffffffff Area: |cff00ff00%u |cffffffffZone: |cff00ff00%u |cffffffffAreaFlags: |cff00ff00%u |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f|r",
        obj->GetMapId(), obj->GetInstanceID(), obj->GetPhaseMask(), obj->GetAreaId(), obj->GetZoneId(), obj->GetAreaFlags(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());

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

    ItemPrototype* it = ItemPrototypeStorage.LookupEntry(itemid);
    if(it)
    {
        sWorld.LogGM(m_session, "used add item command, item id %u [%s] to %s", it->ItemId, it->Name1, chr->GetName());

        if(!chr->GetItemInterface()->AddItemById(itemid, count, randomprop, false, m_session->GetPlayer()))
        {
            m_session->SendNotification("No free slots were found in your inventory!");
            return true;
        }

        if(chr->GetSession() != m_session) // Since we get that You Recieved Item bullcrap, we don't need this.
        {
            string itemlink = it->ConstructItemLink(randomprop, it->RandomSuffixId, count);
            SystemMessage(m_session, "Adding item %u %s to %s's inventory.", it->ItemId, itemlink.c_str(), chr->GetName());
            SystemMessageToPlr(chr, "%s added item %u %s to your inventory.", m_session->GetPlayer()->GetName(), itemid, itemlink.c_str());
        }
        else
        {
            string itemlink = it->ConstructItemLink(randomprop, it->RandomSuffixId, count);
            SystemMessage(m_session, "Adding item %u %s to your inventory.", it->ItemId, itemlink.c_str());
        }
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Item %d is not a valid item!",itemid);
        return true;
    }
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

        if(!m_session->GetPlayer()->m_isGmInvisible)
        {
            // send message to player
            snprintf((char*)buf0,256, "You are being summoned by %s.", m_session->GetPlayer()->GetName());
            SystemMessageToPlr(chr, buf0);
        }

        Player* plr = m_session->GetPlayer();

        if(plr->GetMapMgr() == chr->GetMapMgr())
            chr->_Relocate(plr->GetMapId(),plr->GetPosition(),false,false,plr->GetInstanceID());
        else
            sEventMgr.AddEvent(chr,&Player::EventPortToGM,plr->GetLowGUID(),0,1,1,0);
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
            snprintf((char*) &query,512, "UPDATE characters SET mapId = %u, positionX = %f, positionY = %f, positionZ = %f, zoneId = %u WHERE guid = %u;", pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetZoneId(), pinfo->guid);
            CharacterDatabase.Execute(query);
            char buf[256];
            snprintf((char*)buf,256,"(Offline) %s has been summoned.", pinfo->name);
            SystemMessage(m_session, buf);
            sWorld.LogGM(m_session, "Summoned offline player %s", pinfo->name);
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

        if(!m_session->GetPlayer()->m_isGmInvisible)
            SystemMessageToPlr(chr, "%s appeared to your location.", m_session->GetPlayer()->GetName());

        //If the GM is on the same map as the player, use the normal safeteleport method
        if ( m_session->GetPlayer()->GetMapId() == chr->GetMapId() && m_session->GetPlayer()->GetInstanceID() == chr->GetInstanceID() )
            m_session->GetPlayer()->SafeTeleport(chr->GetMapId(),chr->GetInstanceID(),chr->GetPosition());
        else
            m_session->GetPlayer()->SafeTeleport(chr->GetMapMgr(), chr->GetPosition());
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
            else
                ss << "Player " << args << " is not available.";

            if(PI->lastpositionx != 0.0f && PI->lastpositiony != 0.0f)
            {
                m_session->GetPlayer()->SafeTeleport(PI->lastmapid, PI->curInstanceID, PI->lastpositionx, PI->lastpositiony, PI->lastpositionz+1.0f, PI->lastorientation);
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
    }
    else
        GreenSystemMessage(m_session, "Unlocking all taxi nodes.");

    uint32 taximask = flag ? 0xFFFFFFFF : 0;
    for (uint8 i = 0; i < MAX_TAXI; i++)
        chr->SetTaximask(i, taximask);
    return true;
}

bool ChatHandler::HandleModifySpeedCommand(const char* args, WorldSession *m_session)
{
    if (!*args)
        return false;

    int32 type = -1;
    float Speed = 0.0f;
    if( sscanf(args, "%f %i", &Speed, &type) < 1 )
        return false;

    if (Speed > 255 || Speed < 0)
    {
        RedSystemMessage(m_session, "Incorrect value. Range is 1..255");
        return true;
    }

    if(Speed == 0.0f)
        type = -1;

    std::string speedname = "run";
    switch(type)
    {
    case -1:
        speedname = "total";
        break;
    case RUN:
        speedname = "run";
        break;
    case RUNBACK:
        speedname = "backwards run";
        break;
    case SWIM:
        speedname = "swim";
        break;
    case SWIMBACK:
        speedname = "backwards swim";
        break;
    case TURN:
        speedname = "turn(WARNING!!! May cause serious injury to eyes)";
        break;
    case FLY:
        speedname = "flying";
        break;
    case FLYBACK:
        speedname = "backwards flying";
        break;
    case PITCH_RATE:
        speedname = "pitch";
        break;
    default:
        return false;
    }
    Unit * u = getSelectedUnit(m_session, false);
    if(!u)
        u = m_session->GetPlayer();

    if(Speed == 0.0f)
        Speed = u->m_base_runSpeed*(1.0f + ((float)u->m_speedModifier)/100.0f);

    BlueSystemMessage(m_session, "You set the %s speed of %s to %2.2f.", speedname.c_str(),  u->GetName(), Speed);
    if(u->IsPlayer() && TO_PLAYER(u) != m_session->GetPlayer())
        SystemMessage(TO_PLAYER(u)->GetSession(), "%s set your %s speed to %2.2f.", speedname.c_str(),  m_session->GetPlayer()->GetName(), Speed);

    if(type == -1)
    {
        u->SetSpeed(RUN, Speed);
        u->SetSpeed(RUNBACK, (Speed * 0.5));
        u->SetSpeed(SWIM, Speed);
        u->SetSpeed(SWIMBACK, (Speed * 0.5));
        u->SetSpeed(TURN, 3.141593f);
        u->SetSpeed(FLY, Speed*3);
        u->SetSpeed(FLYBACK, (Speed * 1.5));
        u->SetSpeed(PITCH_RATE, 3.141593f);
    }
    else
        u->SetSpeed(type, Speed);
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

    plr->_AddSkillLine(skill, min, max);

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

    if(!plr->_HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line, adding.");
        plr->_AddSkillLine(skill, 1, 300);
    } else {
        plr->_AdvanceSkillLine(skill,cnt);
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

    if (!plr->_HasSkillLine(skill))
    {
        BlueSystemMessage(m_session, "Player does not have %s skill.", SkillName);
        return false;
    }

    uint32 nobonus = plr->_GetSkillLineCurrent(skill,false);
    uint32 bonus = plr->_GetSkillLineCurrent(skill,true) - nobonus;
    uint32 max = plr->_GetSkillLineMax(skill);

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
        if (plr->_HasSkillLine(SkillId))
        {
            char * SkillName = SkillNameManager->SkillNames[SkillId];
            if (!SkillName)
            {
                RedSystemMessage(m_session, "Invalid skill: %u", SkillId);
                continue;
            }

            nobonus = plr->_GetSkillLineCurrent(SkillId,false);
            bonus = plr->_GetSkillLineCurrent(SkillId,true) - nobonus;
            max = plr->_GetSkillLineMax(SkillId);

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
    plr->_RemoveSkillLine(skill);
    SystemMessageToPlr(plr, "%s removed skill line %d from you. ", m_session->GetPlayer()->GetName(), skill);
    return true;
}

bool ChatHandler::HandleEmoteCommand(const char* args, WorldSession *m_session)
{
    uint32 emote = atoi((char*)args);
    Creature* target = getSelectedCreature(m_session);
    if(!target) 
        return false;

    target->original_emotestate = emote;
    target->SetUInt32Value(UNIT_NPC_EMOTESTATE, emote);
    if(target->m_spawn && m_session->CanUseCommand('z'))
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
    if(target->m_spawn && m_session->CanUseCommand('z'))
        target->SaveToDB();
    return true;
}

bool ChatHandler::HandleGenderChanger(const char* args, WorldSession *m_session)
{
    int gender;
    Player* target = objmgr.GetPlayer((uint32)m_session->GetPlayer()->GetSelection());
    if(!target) {
        SystemMessage(m_session, "Select A Player first.");
        return true;
    }
    if (!*args)
        gender = (target->getGender()== 1 ? 0 : 1);
    else
        gender = ( min((int)atoi((char*)args),1) > 0 ? 1: 0);

    target->setGender(gender);
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

    int32 total   = atoi( (char*)args );
    uint32 gold   = (uint32) floor( (float)int32abs( total ) / 10000.0f );
    uint32 silver = (uint32) floor( ((float)int32abs( total ) / 100.0f) ) % 100;
    uint32 copper = int32abs2uint32( total ) % 100;

    sWorld.LogGM( m_session, "used modify gold on %s, gold: %i", chr->GetName(), total );

    int32 newgold = chr->GetUInt32Value( PLAYER_FIELD_COINAGE ) + total;

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

    chr->SetUInt32Value( PLAYER_FIELD_COINAGE, newgold );
    return true;
}

bool ChatHandler::HandleTriggerCommand(const char* args, WorldSession* m_session)
{
    int32 instance_id;
    uint32 trigger_id;
    int valcount = sscanf(args, "%u %i", (unsigned int*)&trigger_id, (int*)&instance_id);
    if(!valcount)
        return false;
    if(valcount == 1)
        instance_id = 0;

    AreaTrigger *pTrigger = AreaTriggerStorage.LookupEntry(trigger_id);
    if(trigger_id == 0 || pTrigger == NULL)
    {
        RedSystemMessage(m_session, "Could not find trigger %s", (args == NULL ? "NULL" : args));
        return true;
    }

    m_session->GetPlayer()->SafeTeleport(pTrigger->Mapid, instance_id, LocationVector(pTrigger->x, pTrigger->y, pTrigger->z, pTrigger->o));
    BlueSystemMessage(m_session, "Teleported to trigger %u on [%u][%.2f][%.2f][%.2f]", pTrigger->AreaTriggerID, pTrigger->Mapid, pTrigger->x, pTrigger->y, pTrigger->z);
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
    }
    else
        RedSystemMessage(m_session, "That player does not have spell %u learnt.", SpellId);

    return true;
}

bool ChatHandler::HandleNpcSpawnLinkCommand(const char* args, WorldSession *m_session)
{
    uint32 id;
    char sql[512];
    Creature* target = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(m_session->GetPlayer()->GetSelection()));
    if (!target)
        return false;

    int valcount = sscanf(args, "%u", (unsigned int*)&id);
    if(valcount && target->m_spawn != NULL)
    {
        snprintf(sql, 512, "UPDATE creature_spawns SET respawnlink = '%u' WHERE id = '%u'", id, target->GetSQL_id());
        WorldDatabase.Execute( sql );
        BlueSystemMessage(m_session, "Spawn linking for this NPC has been updated: %u", id);
    }
    else
        RedSystemMessage(m_session, "Sql entry invalid %u", id);

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

    Pl->smsg_TalentsInfo(false);
    sWorld.LogGM(m_session, "Modified %s talents to %u", Pl->GetName(), TP1);
    return true;
}
