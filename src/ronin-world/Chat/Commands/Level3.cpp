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
//  Admin Chat Commands
/////////////////////////////////////////////////

#include "StdAfx.h"

bool ChatHandler::HandleWorldPortCommand(const char* args, WorldSession *m_session)
{
    float x, y, z, o;
    uint32 mapid;
    uint32 argc = sscanf(args, "%u %f %f %f %f", (unsigned int*)&mapid, &x, &y, &z, &o);
    if(argc < 4)
        return false;
    if(argc < 5)
        o = m_session->GetPlayer()->GetOrientation();

    if(x >= _maxX || x <= _minX || y <= _minY || y >= _maxY)
        return false;

    if(m_session->GetPlayer()->GetMapId() == mapid || !m_session->CheckTeleportPrerequisites(m_session, m_session->GetPlayer(), mapid))
        m_session->GetPlayer()->SafeTeleport(mapid, 0, LocationVector(x, y, z, o));
    else
    {
        WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 50);
        data << uint32(0);
        data << "You do not reach the requirements to teleport here.";
        data << uint8(0);
        m_session->SendPacket(&data);
    }
    return true;
}


bool ChatHandler::HandleClearCooldownsCommand(const char *args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(plr == NULL)
        return false;

    plr->ResetAllCooldowns();
    BlueSystemMessage(m_session, "Cleared all %s cooldowns.", plr->getClassName().c_str());
    return true;
}

bool ChatHandler::HandleLearnCommand(const char* args, WorldSession *m_session)
{
    if( !*args )
        return false;

    Player* plr = getSelectedChar( m_session, true );
    if( !plr )
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    if( !plr )
        return false;

    if( stricmp( args, "all" ) == 0 )
    {   // Get our trainer spell map for our class and add all spells
        if(ObjectMgr::TrainerSpellMap *map = objmgr.GetTrainerSpells(TRAINER_CATEGORY_TALENTS, plr->getClass()))
        {
            for(ObjectMgr::TrainerSpellMap::iterator itr = map->begin(); itr != map->end(); itr++)
                if(plr->getLevel() >= itr->second.requiredLevel)
                    plr->addSpell(itr->first);
            sWorld.LogGM(m_session, "taught %s all spells.", plr->GetName());
        }
        return true;
    }

    uint32 spell = atol((char*)args);
    if( spell == 0 )
        spell = GetSpellIDFromLink( args );

    sWorld.LogGM(m_session, "taught %s spell %u", plr->GetName(), spell);

    SpellEntry * sp = dbcSpell.LookupEntry(spell);
    if( sp == NULL )
    {
        RedSystemMessage(m_session, "Spell id %u does not exist.", spell);
        return true;
    }

    if(!plr->GetSession()->HasGMPermissions() && (sp->Effect[0]==SPELL_EFFECT_INSTANT_KILL||sp->Effect[1]==SPELL_EFFECT_INSTANT_KILL||sp->Effect[2]==SPELL_EFFECT_INSTANT_KILL))
    {
        SystemMessage(m_session, "don't be an idiot and teach players instakill spells. this action has been logged.");
        return true;
    }

    if( plr->HasSpell( spell ) ) // check to see if char already knows
    {
        std::string OutStr = plr->GetName();
        OutStr += " already knows that spell.";

        SystemMessage(m_session, OutStr.c_str());
        return true;
    }

    plr->addSpell(spell);
    BlueSystemMessageToPlr(plr, "%s taught you Spell %d", m_session->GetPlayer()->GetName(), spell);

    return true;
}

bool ChatHandler::HandleReviveCommand(const char* args, WorldSession *m_session)
{
    Player* SelectedPlayer = getSelectedChar(m_session, false);
    if(SelectedPlayer == NULL) 
    {
        if(getSelectedCreature(m_session, false))
        {
            HandleCreatureRespawnCommand(args, m_session);
            return true;
        }

        RedSystemMessage(m_session, "You have no target!");
        return false;
    }

    BlueSystemMessage(m_session, "Reviving...");
    SelectedPlayer->ResurrectPlayer(m_session->GetPlayer());
    return true;
}

bool ChatHandler::HandleExploreCheatCommand(const char* args, WorldSession *m_session)
{
    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player* chr = getSelectedChar(m_session);
    if (chr == NULL)
    {
        SystemMessage(m_session, "No character selected.");
        return true;
    }

    SystemMessage(m_session, (flag == 0 ? "%s has no more explored zones." : "%s has explored all zones now."), chr->GetName());
    SystemMessage(m_session, (flag == 0 ? "%s has hidden all zones from you." : "%s has explored all zones for you."), chr->GetName());

    for (uint16 i = 0; i < 156; i++)
        chr->SetFlag(PLAYER_EXPLORED_ZONES_1+i, flag ? 0xFFFFFFFF : 0);
    return true;
}

bool ChatHandler::HandleBanCharacterCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    // this is rather complicated due to ban reasons being able to have spaces. so we'll have to some c string magic
    // rather than just sscanf'ing it.
    char * pCharacter = (char*)args;
    char * pBanDuration = strchr(pCharacter, ' ');
    PlayerInfo * pInfo = NULL;
    if(pBanDuration == NULL)
        return false;

    char * pReason = strchr(pBanDuration+1, ' ');
    if(pReason == NULL)
        return false;

    // zero them out to create sepearate strings.
    *pBanDuration = 0;
    ++pBanDuration;
    *pReason = 0;
    ++pReason;

    int32 BanTime = RONIN_UTIL::GetTimePeriodFromString(pBanDuration);
    if(BanTime < 1)
        return false;

    Player* pPlayer = NULL;
    if(pPlayer = objmgr.GetPlayer(pCharacter, false))
    {
        SystemMessage(m_session, "Banning player '%s' ingame for '%s'.", pCharacter, pReason);
        std::string sReason = pReason;
        uint32 uBanTime = BanTime ? BanTime+(uint32)UNIXTIME : 1;
        pPlayer->SetBanned(uBanTime, sReason);
        pInfo = pPlayer->getPlayerInfo();
    }
    else
    {
        pInfo = objmgr.GetPlayerInfoByName(pCharacter);
        if(pInfo == NULL)
        {
            SystemMessage(m_session, "Player not found.");
            return true;
        }

        SystemMessage(m_session, "Banning player '%s' in database for '%s'.", pCharacter, pReason);
        std::string escaped_reason = CharacterDatabase.EscapeString(std::string(pReason));

        CharacterDatabase.Execute("UPDATE character_data SET banned = %u, banReason = '%s' WHERE guid = %u", BanTime ? BanTime+(uint32)UNIXTIME : 1, escaped_reason.c_str(), pInfo->charGuid.getLow());
    }

    SystemMessage(m_session, "This ban is due to expire %s%s.", BanTime ? "on " : "", BanTime ? RONIN_UTIL::ConvertTimeStampToDataTime(BanTime+(uint32)UNIXTIME).c_str() : "Never");
    if(pPlayer)
    {
        SystemMessage(m_session, "Kicking %s.", pPlayer->GetName());
        pPlayer->Kick();
    }

    sWorld.LogGM(m_session, "used ban character on %s reason %s for %s", pCharacter, pReason, BanTime ? RONIN_UTIL::ConvertTimeStampToString(BanTime).c_str() : "ever");
    return true;
}

bool ChatHandler::HandleUnBanCharacterCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    char Character[255];
    if(sscanf(args, "%s", Character) == 0)
    {
        RedSystemMessage(m_session, "A character name and reason is required.");
        return true;
    }

    // Check if player is in world.
    Player* pPlayer = ObjectMgr::getSingleton( ).GetPlayer(Character, false);
    if(pPlayer != 0)
    {
        GreenSystemMessage(m_session, "Unbanned player %s ingame.",pPlayer->GetName());
        pPlayer->UnSetBanned();
    }
    else
    {
        GreenSystemMessage(m_session, "Player %s not found ingame.", Character);
    }

    // Unban in database
    CharacterDatabase.Execute("UPDATE character_data SET banned = 0 WHERE name = '%s'", CharacterDatabase.EscapeString(std::string(Character)).c_str());

    SystemMessage(m_session, "Unbanned character %s in database.", Character);
    sWorld.LogGM(m_session, "used unban character on %s", Character);
    return true;
}

bool ChatHandler::HandleAddSkillCommand(const char* args, WorldSession *m_session)
{
    char buf[256];
    Player* target = objmgr.GetPlayer(m_session->GetPlayer()->GetSelection());
    if(target == NULL)
    {
        SystemMessage(m_session, "Select A Player first.");
        return true;
    }

    uint32 skillline;
    uint16 cur, max;

    char* pSkillline = strtok((char*)args, " ");
    if (!pSkillline)
        return false;

    char* pCurrent = strtok(NULL, " ");
    if (!pCurrent)
        return false;

    char* pMax = strtok(NULL, " ");
    if (!pMax)
        return false;

    skillline = (uint32)atol(pSkillline);
    cur = (uint16)atol(pCurrent);
    max = (uint16)atol(pMax);

    target->AddSkillLine(skillline, 0,max,cur);

    snprintf(buf,256,"SkillLine: %u CurrentValue %u Max Value %u Added.",(unsigned int)skillline,(unsigned int)cur,(unsigned int)max);
    sWorld.LogGM(m_session, "added skill line %u (%u/%u) to %s", skillline, cur, max, target->GetName());
    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleNpcInfoCommand(const char *args, WorldSession *m_session)
{
    Creature* crt = getSelectedCreature(m_session);
    if(crt == NULL)
        return false;
    if(crt->GetCreatureData())
        BlueSystemMessage(m_session, "Showing creature info for %s", crt->GetName());
    FactionTemplateEntry *factionTemplate = crt->GetFactionTemplate();

    SystemMessage(m_session, "LowGUID %u", crt->GetLowGUID());
    if(factionTemplate)
        SystemMessage(m_session, "Faction: %u|%u", factionTemplate->ID, factionTemplate->Faction);
    SystemMessage(m_session, "NPCFlags %u", crt->GetUInt32Value(UNIT_NPC_FLAGS));
    SystemMessage(m_session, "UnitFlags %u | %u", crt->GetUInt32Value(UNIT_FIELD_FLAGS), crt->GetUInt32Value(UNIT_FIELD_FLAGS_2));
    SystemMessage(m_session, "DisplayID %u", crt->GetUInt32Value(UNIT_FIELD_DISPLAYID));
    SystemMessage(m_session, "Scale %f", crt->GetFloatValue(OBJECT_FIELD_SCALE_X));
    SystemMessage(m_session, "EmoteState: %u", crt->GetUInt32Value(UNIT_NPC_EMOTESTATE));
    GreenSystemMessage(m_session, "Base Health: %u", crt->GetUInt32Value(UNIT_FIELD_BASE_HEALTH));
    GreenSystemMessage(m_session, "Base Armor: %u", crt->GetUInt32Value(UNIT_FIELD_RESISTANCES));

    SystemMessage(m_session, "|cff00ff00Resistance:|r|cffffffff %u|r|cffff0000 %u|r|cff00ff00 %u|r|cff00ccff %u|r|cffda70d6 %u|r|cff8B8B83 %u|r", crt->GetUInt32Value(UNIT_FIELD_RESISTANCES+1), crt->GetUInt32Value(UNIT_FIELD_RESISTANCES+2),
        crt->GetUInt32Value(UNIT_FIELD_RESISTANCES+3), crt->GetUInt32Value(UNIT_FIELD_RESISTANCES+4), crt->GetUInt32Value(UNIT_FIELD_RESISTANCES+5), crt->GetUInt32Value(UNIT_FIELD_RESISTANCES+6));

    GreenSystemMessage(m_session, "Damage min/max: %f/%f", crt->GetFloatValue(UNIT_FIELD_MINDAMAGE),crt->GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    GreenSystemMessage(m_session, "Phase: %i", crt->GetPhaseMask());
    if(crt->IsSpawn())
        GreenSystemMessage(m_session, "Vendor Mask: %i", crt->GetVendorMask());
    GreenSystemMessage(m_session, "Team: %s", crt->GetTeamName());
    ColorSystemMessage(m_session, MSG_COLOR_RED, "Entry ID: %u", crt->GetUInt32Value(OBJECT_FIELD_ENTRY));

    // show byte
    std::stringstream sstext;
    uint32 theBytes = crt->GetUInt32Value(UNIT_FIELD_BYTES_0);
    sstext << "UNIT_FIELD_BYTES_0 are " << uint16((uint8)theBytes & 0xFF) << " " << uint16((uint8)(theBytes >> 8) & 0xFF) << " ";
    sstext << uint16((uint8)(theBytes >> 16) & 0xFF) << " " << uint16((uint8)(theBytes >> 24) & 0xFF) << '\0';
    BlueSystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleIncreaseWeaponSkill(const char *args, WorldSession *m_session)
{
    char *pMin = strtok((char*)args, " ");
    uint32 cnt = 0;
    if(!pMin)
        cnt = 1;
    else cnt = atol(pMin);

    Player* pr = getSelectedChar(m_session, true);

    uint32 SubClassSkill = 0;
    if(!pr) pr = m_session->GetPlayer();
    if(!pr) return false;
    Item* it = pr->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    ItemPrototype* proto = NULL;
    if (!it)
        it = pr->GetInventory()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
    if (it)
        proto = it->GetProto();
    if (proto)
    {
        switch(proto->SubClass)
        {
            // Weapons
        case 0: // 1 handed axes
            SubClassSkill = SKILL_AXES;
            break;
        case 1: // 2 handed axes
            SubClassSkill = SKILL_2H_AXES;
            break;
        case 2: // bows
            SubClassSkill = SKILL_BOWS;
            break;
        case 3: // guns
            SubClassSkill = SKILL_GUNS;
            break;
        case 4: // 1 handed mace
            SubClassSkill = SKILL_MACES;
            break;
        case 5: // 2 handed mace
            SubClassSkill = SKILL_2H_MACES;
            break;
        case 6: // polearms
            SubClassSkill = SKILL_POLEARMS;
            break;
        case 7: // 1 handed sword
            SubClassSkill = SKILL_SWORDS;
            break;
        case 8: // 2 handed sword
            SubClassSkill = SKILL_2H_SWORDS;
            break;
        case 9: // obsolete
            SubClassSkill = 136;
            break;
        case 10: //1 handed exotic
            SubClassSkill = 136;
            break;
        case 11: // 2 handed exotic
            SubClassSkill = 0;
            break;
        case 12: // fist
            SubClassSkill = SKILL_FIST_WEAPONS;
            break;
        case 13: // misc
            SubClassSkill = 0;
            break;
        case 15: // daggers
            SubClassSkill = SKILL_DAGGERS;
            break;
        case 16: // thrown
            SubClassSkill = SKILL_THROWN;
            break;
        case 17: // spears
            SubClassSkill = SKILL_SPEARS;
            break;
        case 18: // crossbows
            SubClassSkill = SKILL_CROSSBOWS;
            break;
        case 19: // wands
            SubClassSkill = SKILL_WANDS;
            break;
        case 20: // fishing
            SubClassSkill = SKILL_FISHING;
            break;
        }
    } else SubClassSkill = 162;

    if(!SubClassSkill)
    {
        RedSystemMessage(m_session, "Can't find skill ID");
        return false;
    }

    uint32 skill = SubClassSkill;

    BlueSystemMessage(m_session, "Modifying skill line %d. Advancing %d times.", skill, cnt);
    sWorld.LogGM(m_session, "increased weapon skill of %s by %u", pr->GetName(), cnt);

    if(!pr->HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line, adding.");
        pr->AddSkillLine(skill, 0, 300, 1);
    } else pr->ModSkillLineAmount(skill,cnt,false);
    return true;
}

bool ChatHandler::HandleEarnAchievement(const char* args, WorldSession *m_session)
{
    if(Player* plr = getSelectedChar(m_session, false))
        AchieveMgr.EarnAchievement(plr, atol(args));
    return true;
}

bool ChatHandler::HandleAddCurrency(const char* args, WorldSession *m_session)
{
    uint32 currency, amount;
    if(sscanf(args, "%u %u", &currency, &amount) < 2)
        return false;

    if(Player* plr = getSelectedChar(m_session, false))
        plr->GetCurrency()->AddCurrency(currency, amount);
    return true;
}

bool ChatHandler::HandleRemoveCurrency(const char* args, WorldSession *m_session)
{
    uint32 currency, amount;
    if(sscanf(args, "%u %u", &currency, &amount) < 2)
        return false;

    if(Player* plr = getSelectedChar(m_session, false))
        plr->GetCurrency()->RemoveCurrency(currency, amount);
    return true;
}

bool ChatHandler::HandleResetTalentsCommand(const char* args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session);
    if(plr == NULL) return true;
    plr->GetTalentInterface()->ResetAllSpecs();
    SystemMessage(m_session, "Reset talents of %s.", plr->GetName());
    BlueSystemMessageToPlr(plr, "%s reset all your talents.", m_session->GetPlayer()->GetName());
    sWorld.LogGM(m_session, "reset talents of %s", plr->GetName());
    return true;
}

bool ChatHandler::HandleResetSpellsCommand(const char* args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session);
    if(plr == NULL)
        return true;

    plr->Reset_Spells();

    SystemMessage(m_session, "Reset spells of %s to level 1.", plr->GetName());
    BlueSystemMessage(m_session, "%s reset all your spells to starting values.", m_session->GetPlayer()->GetName());
    sWorld.LogGM(m_session, "reset spells of %s", plr->GetName());
    return true;
}

bool ChatHandler::HandleAccountLevelCommand(const char * args, WorldSession * m_session)
{
    if(!*args) return false;

    char account[100];
    char gmlevel[100];
    int argc = sscanf(args, "%s %s", account, gmlevel);
    if(argc != 2)
        return false;

    sLogonCommHandler.Account_SetGM( account, gmlevel );

    GreenSystemMessage(m_session, "Account '%s' level has been updated to '%s'. The change will be effective immediately.", account, gmlevel);
    sWorld.LogGM(m_session, "set account %s flags to %s", account, gmlevel);

    return true;
}

bool ChatHandler::HandleAccountUnbanCommand(const char * args, WorldSession * m_session)
{
    if(!*args) return false;
    char * pAccount = (char*)args;

    sLogonCommHandler.Account_SetBanned( pAccount, 0, "" );
    GreenSystemMessage(m_session, "Account '%s' has been unbanned. This change will be effective immediately.", pAccount);

    sWorld.LogGM(m_session, "unbanned account %s", pAccount);
    return true;
}

bool ChatHandler::HandleAccountBannedCommand(const char * args, WorldSession * m_session)
{
    if(!*args)
        return false;

    char * pAccount = (char*)args;
    char * pDuration = strchr(pAccount, ' ');
    if( pDuration == NULL )
        return false;
    *pDuration = 0;
    ++pDuration;

    char * pReason = strchr(pDuration, ' ');
    if( pReason == NULL )
        return false;

    *pReason = 0;
    ++pReason;

    int32 timeperiod = RONIN_UTIL::GetTimePeriodFromString(pDuration);
    if( timeperiod < 0 )
        return false;

    uint32 banned = (timeperiod ? (uint32)UNIXTIME+timeperiod : 1);

    sLogonCommHandler.Account_SetBanned(pAccount, banned, pReason);

    GreenSystemMessage(m_session, "Account '%s' has been banned %s%s. The change will be effective immediately.", pAccount,
        timeperiod ? "until " : "forever", timeperiod ? RONIN_UTIL::ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME).c_str() : "");

    sWorld.DisconnectUsersWithAccount(pAccount, m_session);
    sWorld.LogGM(m_session, "banned account %s until %s for %s", pAccount, timeperiod ? RONIN_UTIL::ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME).c_str() : "permanant", pReason);
    return true;
}

bool ChatHandler::HandleAccountMuteCommand(const char * args, WorldSession * m_session)
{
    if(!*args) return false;

    char * pAccount = (char*)args;
    char * pDuration = strchr(pAccount, ' ');
    if(pDuration == NULL)
        return false;
    *pDuration = 0;
    ++pDuration;

    int32 timeperiod = RONIN_UTIL::GetTimePeriodFromString(pDuration);
    if(timeperiod <= 0)
        return false;

    uint32 banned = (uint32)UNIXTIME+timeperiod;

    sLogonCommHandler.Account_SetMute( pAccount, banned );

    std::string tsstr = RONIN_UTIL::ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME);
    GreenSystemMessage(m_session, "Account '%s' has been muted until %s. The change will be effective immediately.", pAccount, tsstr.c_str());

    sWorld.LogGM(m_session, "mutex account %s until %s", pAccount, RONIN_UTIL::ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME).c_str());

    WorldSession * pSession = sWorld.FindSessionByName(pAccount);
    if( pSession != NULL )
    {
        pSession->m_muted = banned;
        pSession->SystemMessage("Your voice has been muted until %s by a GM. Until this time, you will not be able to speak in any form.", tsstr.c_str());
    }

    return true;
}

bool ChatHandler::HandleAccountUnmuteCommand(const char * args, WorldSession * m_session)
{
    sLogonCommHandler.Account_SetMute( args, 0 );

    GreenSystemMessage(m_session, "Account '%s' has been unmuted.", args);
    if( WorldSession * pSession = sWorld.FindSessionByName(args) )
    {
        pSession->m_muted = 0;
        pSession->SystemMessage("Your voice has restored. You may speak again.");
    }

    return true;
}

bool ChatHandler::HandleGetTransporterTime(const char* args, WorldSession* m_session)
{
    Creature* crt = getSelectedCreature(m_session, false);
    if( crt == NULL )
        return true;

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, 1000);
    data << uint32(0x00000102);
    data << crt->GetGUID().asPacked();
    data << m_session->GetPlayer()->GetGUID().asPacked();

    data << uint32(6);
    data << uint8(1);
    data << uint32(1);
    data << uint32(0x40c00000);
    data << uint32(6);
    data << uint32(0);
    data << uint32(0);
    data << uint32(1);
    data << uint32(0x000003e8);
    data << uint32(0);
    data << uint32(0);
    m_session->SendPacket(&data);
    return true;
}

bool ChatHandler::HandleRemoveAurasCommand(const char *args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(plr == NULL)
        return true;

    BlueSystemMessage(m_session, "Removing all auras...");
    plr->m_AuraInterface.RemoveAllNonPassiveAuras();
    return true;
}

bool ChatHandler::HandleRemoveRessurectionSickessAuraCommand(const char *args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(plr == NULL)
        return true;

    BlueSystemMessage(m_session, "Removing ressurection sickness...");
    plr->RemoveAura( 15007 );
    return true;
}

bool ChatHandler::HandleParalyzeCommand(const char* args, WorldSession *m_session)
{
    //Player* plr = getSelectedChar(m_session, true);
    //if(!plr) return false;
    Unit* plr = m_session->GetPlayer()->GetMapInstance()->GetUnit(m_session->GetPlayer()->GetSelection());
    if(!plr || plr->GetTypeId() != TYPEID_PLAYER)
    {
        RedSystemMessage(m_session, "Invalid target.");
        return true;
    }

    BlueSystemMessage(m_session, "Rooting target.");
    BlueSystemMessageToPlr( castPtr<Player>( plr ), "You have been rooted by %s.", m_session->GetPlayer()->GetName() );
    WorldPacket data(SMSG_MOVE_ROOT, 12);
    data << plr->GetGUID().asPacked();
    data << uint32(1);
    plr->SendMessageToSet(&data, true);
    return true;
}

bool ChatHandler::HandleUnParalyzeCommand(const char* args, WorldSession *m_session)
{
    //Player* plr = getSelectedChar(m_session, true);
    //if(!plr) return false;
    Unit* plr = m_session->GetPlayer()->GetMapInstance()->GetUnit(m_session->GetPlayer()->GetSelection());
    if(!plr || plr->GetTypeId() != TYPEID_PLAYER)
    {
        RedSystemMessage(m_session, "Invalid target.");
        return true;
    }

    BlueSystemMessage(m_session, "Unrooting target.");
    BlueSystemMessageToPlr( castPtr<Player>( plr ), "You have been unrooted by %s.", m_session->GetPlayer()->GetName() );
    WorldPacket data;
    data.Initialize(SMSG_MOVE_UNROOT);
    data << plr->GetGUID().asPacked();
    data << uint32(5);

    plr->SendMessageToSet(&data, true);
    return true;
}

bool ChatHandler::HandleSetMotdCommand(const char* args, WorldSession* m_session)
{
    if(!args || strlen(args) < 2)
    {
        RedSystemMessage(m_session, "You must specify a message.");
        return true;
    }

    GreenSystemMessage(m_session, "Motd has been set to: %s", args);
    World::getSingleton().SetMotd(args);
    sWorld.LogGM(m_session, "Set MOTD to %s", args);
    return true;
}

bool ChatHandler::HandleAddItemSetCommand(const char* args, WorldSession* m_session)
{
    uint32 setid, rank = 1;
    int32 randomprop = 0;

    if(strlen(args) < 1)
        return false;

    if(sscanf(args, "%u %u", &setid, &rank) < 1)
    {
        RedSystemMessage(m_session, "You must specify a setid.");
        return true;
    }

    Player* chr = getSelectedChar(m_session);
    if (chr == NULL)
        return true;

    std::list<ItemPrototype*>* l = objmgr.GetListForItemSet(setid);
    if(l == NULL)
    {
        RedSystemMessage(m_session, "Invalid item set.");
        return true;
    }

    uint32 start = getMSTime(), count = 0;
    BlueSystemMessage(m_session, "Searching item set %u...", setid);
    sWorld.LogGM(m_session, "used add item set command, set %u, target %s", setid, chr->GetName());
    for(std::list<ItemPrototype*>::iterator itr = l->begin(); itr != l->end(); itr++)
    {
        if((*itr)->ItemSetRank && (*itr)->ItemSetRank != rank)
            continue;

        //if(!chr->GetInventory()->CreateInventoryStacks(*itr, 1, m_session->GetPlayer()->GetGUID()))
        {
            RedSystemMessage(m_session, "Failed to create any inventory stacks");
            return true;
        }

        std::string itemlink = (*itr)->ConstructItemLink(0, (*itr)->RandomSuffixId, 1);
        if(chr->GetSession() != m_session) // Since we get that You Recieved Item bullcrap, we don't need this.
        {
            SystemMessage(m_session, "Adding item %u %s to %s's inventory.", (*itr)->ItemId, itemlink.c_str(), chr->GetName());
            SystemMessageToPlr(chr, "%s added item %u %s to your inventory.", m_session->GetPlayer()->GetName(), (*itr)->ItemId, itemlink.c_str());
        } else SystemMessage(m_session, "Adding item %u %s to your inventory.", (*itr)->ItemId, itemlink.c_str());
        count++;
    }

    if(count > 0)
        GreenSystemMessage(m_session, "Added set to inventory complete. Time: %u ms", getMSTime() - start);
    else RedSystemMessage(m_session, "Failed to add items");
    return true;
}

bool ChatHandler::HandleCastTimeCheatCommand(const char* args, WorldSession* m_session)
{
    Player* plyr = getSelectedChar(m_session, true);
    if(!plyr) return true;

    bool val = plyr->CastTimeCheat;
    BlueSystemMessage(m_session, "%s cast time cheat on %s.", val ? "Deactivating" : "Activating", plyr->GetName());
    GreenSystemMessageToPlr(plyr, "%s %s a cast time cheat on you.", m_session->GetPlayer()->GetName(), val ? "deactivated" : "activated");

    plyr->CastTimeCheat = !val;
    sWorld.LogGM(m_session, "%s cast time cheat on %s", val ? "disabled" : "enabled", plyr->GetName());
    return true;
}

bool ChatHandler::HandleCooldownCheatCommand(const char* args, WorldSession* m_session)
{
    Player* plyr = getSelectedChar(m_session, true);
    if(!plyr) return true;

    bool val = plyr->CooldownCheat;
    BlueSystemMessage(m_session, "%s cooldown cheat on %s.", val ? "Deactivating" : "Activating", plyr->GetName());
    GreenSystemMessageToPlr(plyr, "%s %s a cooldown cheat on you.", m_session->GetPlayer()->GetName(), val ? "deactivated" : "activated");

    plyr->CooldownCheat = !val;
    sWorld.LogGM(m_session, "%s cooldown cheat on %s", val ? "disabled" : "enabled", plyr->GetName());

    return true;
}

bool ChatHandler::HandlePowerCheatCommand(const char* args, WorldSession* m_session)
{
    Player* plyr = getSelectedChar(m_session, true);
    if(!plyr)
        return true;

    bool val = plyr->PowerCheat;
    BlueSystemMessage(m_session, "%s power cheat on %s.", val ? "Deactivating" : "Activating", plyr->GetName());
    GreenSystemMessageToPlr(plyr, "%s %s a power cheat on you.", m_session->GetPlayer()->GetName(), val ? "deactivated" : "activated");

    plyr->PowerCheat = !val;
    sWorld.LogGM(m_session, "%s powertime cheat on %s", val ? "disabled" : "enabled", plyr->GetName());
    return true;
}

bool ChatHandler::HandleShowCheatsCommand(const char* args, WorldSession* m_session)
{
    Player* plyr = getSelectedChar(m_session, true);
    if(plyr == NULL)
        return true;

    uint32 active = 0, inactive = 0;
#define print_cheat_status(CheatName, CheatVariable) SystemMessage(m_session, "%s%s: %s%s", MSG_COLOR_LIGHTBLUE, CheatName, \
        CheatVariable ? MSG_COLOR_LIGHTRED : MSG_COLOR_GREEN, CheatVariable ? "Active" : "Inactive");  \
        if(CheatVariable) \
        active++; \
        else \
        inactive++;

    GreenSystemMessage(m_session, "Showing cheat status for: %s", plyr->GetName());
    print_cheat_status("Cooldown", plyr->CooldownCheat);
    print_cheat_status("CastTime", plyr->CastTimeCheat);
    print_cheat_status("Power", plyr->PowerCheat);
    SystemMessage(m_session, "%u cheats active, %u inactive.", active, inactive);

#undef print_cheat_status

    return true;
}

bool ChatHandler::HandleFlyCommand(const char* args, WorldSession* m_session)
{
    if(Player* plyr = getSelectedChar(m_session, true))
    {
        bool canFly = plyr->GetMovementInterface()->canFly();
        plyr->GetMovementInterface()->setCanFly(!canFly);
    }
    return false;
}

bool ChatHandler::HandleDBReloadCommand(const char* args, WorldSession* m_session)
{
    char str[256];
    int ret = 0;

    if(!*args || strlen(args) < 3)
        return false;

    uint32 mstime = getMSTime();
    snprintf(str, 200, "%s%s initiated server-side reload of table `%s`. The server may experience some lag while this occurs.",
        MSG_COLOR_LIGHTRED, m_session->GetPlayer()->GetName(), args);
    sWorld.SendWorldText(str, 0);

    ret = Storage_ReloadTable(args);

    if (ret == 0)
        snprintf(str, 256, "%sDatabase reload failed.", MSG_COLOR_LIGHTRED);
    else
        snprintf(str, 256, "%sDatabase reload completed in %u ms.", MSG_COLOR_LIGHTBLUE, (unsigned int)(getMSTime() - mstime));
    sWorld.SendWorldText(str, 0);
    sWorld.LogGM(m_session, "reloaded table %s", args);
    return true;
}

bool ChatHandler::HandleModifyLevelCommand(const char* args, WorldSession* m_session)
{
    Unit* u = getSelectedUnit(m_session, false);
    if(u == NULL)
        u = m_session->GetPlayer();

    uint32 Level = args ? atol(args) : 0;
    if(Level == 0 || u->IsPlayer() && Level > castPtr<Player>(u)->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))
    {
        RedSystemMessage(m_session, "A level (numeric) is required to be specified after this command.");
        return true;
    }

    // Set level message
    BlueSystemMessage(m_session, "Setting the level of %s to %u.", u->GetName(), Level);
    if(u->IsPlayer() && castPtr<Player>(u) != m_session->GetPlayer())
        GreenSystemMessageToPlr(castPtr<Player>(u), "%s set your level to %u.", m_session->GetPlayer()->GetName(), Level);

    sWorld.LogGM(m_session, "used modify level on %s %s, level %u", u->IsPlayer() ? "Player" : "Unit", u->GetName(), Level);
    u->setLevel(Level);
    // If we're a player set our XP to 0
    if(u->IsPlayer()) u->SetUInt32Value(PLAYER_XP, 0);
    return true;
}

bool ChatHandler::HandleAddTitleCommand(const char* args, WorldSession* m_session)
{
    Player* plr = getSelectedChar(m_session);
    if(plr == NULL || args == NULL)
        return true;
    CharTitleEntry *entry = dbcCharTitle.LookupEntry(atoi(args));
    if(entry == NULL)
    {
        RedSystemMessage(m_session, "An existing title entry is required to be specified after this command.");
        return true;
    }

    std::string title = format(entry->titleFormat, plr->GetName());
    BlueSystemMessage(m_session, "Adding title %s", title.c_str());
    plr->SetKnownTitle(entry->Id, true);
    if(m_session->GetPlayer() != plr)
        GreenSystemMessageToPlr(plr, "%s added title %s to you.", m_session->GetPlayer()->GetName(), title.c_str());
    sWorld.LogGM(m_session, "added title number %u to %s", entry->Id, plr->GetName());
    return true;
}
bool ChatHandler::HandleRemoveTitleCommand(const char* args, WorldSession* m_session)
{
    Player* plr = getSelectedChar(m_session);
    if(plr == NULL || args == NULL)
        return true;
    CharTitleEntry *entry = dbcCharTitle.LookupEntry(atoi(args));
    if(entry == NULL)
    {
        RedSystemMessage(m_session, "A title number (numeric) is required to be specified after this command.");
        return true;
    }

    std::string title = format(entry->titleFormat, plr->GetName());
    BlueSystemMessage(m_session, "Removing title %s.", title.c_str());
    plr->SetKnownTitle(entry->Id, false);
    if(m_session->GetPlayer() != plr)
        GreenSystemMessageToPlr(plr, "%s removed title %s from you.", m_session->GetPlayer()->GetName(), title.c_str());
    sWorld.LogGM(m_session, "removed title number %u from %s", entry->Id, plr->GetName());
    return true;
}

bool ChatHandler::HandleGetKnownTitlesCommand(const char* args, WorldSession* m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(plr == NULL)
        return true;

    GreenSystemMessage(m_session, "Starting title listing...");
    for(uint32 i = 0; i < dbcCharTitle.GetNumRows(); i++)
    {
        CharTitleEntry *entry = dbcCharTitle.LookupRow(i);
        if(entry == NULL)
            continue;
        if(!plr->HasKnownTitleByIndex(entry->bit_index))
            continue;
        BlueSystemMessage(m_session, entry->titleFormat, plr->GetName());
    }
    GreenSystemMessage(m_session, "Finished title listing.");
    return true;
}

bool ChatHandler::HandleSetChosenTitleCommand(const char* args, WorldSession* m_session)
{
    Player* plr = getSelectedChar(m_session, true);
    if(plr == NULL || args == NULL)
        return true;

    CharTitleEntry *charTitles = dbcCharTitle.LookupEntry(atoi(args));
    if(charTitles == NULL)
    {
        RedSystemMessage(m_session, "A title valid entry is required to be specified after this command.");
        return true;
    }

    sWorld.LogGM(m_session, "set title number %u for %s", charTitles->Id, plr->GetName());
    if(!plr->HasKnownTitleByIndex(charTitles->bit_index))
    {
        RedSystemMessage(m_session, "Selected player doesn't know this title.");
        return true;
    }

    BlueSystemMessage(m_session, "Setting title number %u for %s.", charTitles->Id, plr->GetName());
    plr->SetUInt32Value(PLAYER_CHOSEN_TITLE, charTitles->bit_index);
    GreenSystemMessageToPlr(plr, "%s set title number %u for you.", m_session->GetPlayer()->GetName(), charTitles->Id);
    return true;
}

bool ChatHandler::HandleShutdownCommand(const char* args, WorldSession* m_session)
{
    uint32 shutdowntime = atol(args);
    if( !args )
        shutdowntime = 5;

    sWorld.LogGM(m_session, "initiated server shutdown, timer %u sec", shutdowntime);
    sWorld.QueueShutdown(shutdowntime, SERVER_SHUTDOWN_TYPE_SHUTDOWN);
    return true;
}

bool ChatHandler::HandleRestartCommand(const char* args, WorldSession* m_session)
{
    uint32 restarttime = atol(args);
    if( !args )
        restarttime = 5;

    sWorld.LogGM(m_session, "initiated server restart, timer %u sec", restarttime);
    sWorld.QueueShutdown(restarttime, SERVER_SHUTDOWN_TYPE_RESTART);
    return true;
}

bool ChatHandler::HandleAllowWhispersCommand(const char* args, WorldSession* m_session)
{
    if(args == 0 || strlen(args) < 2)
        return false;

    Player* plr = objmgr.GetPlayer(args, false);
    if(!plr)
    {
        RedSystemMessage(m_session, "Player not found.");
        return true;
    }

    if(m_session->GetPlayer() == plr) // Ourselves.
    {
        RedSystemMessage(m_session, "Whispering yourself is always allowed.");
        return true;
    }

    BlueSystemMessage(m_session, "Now accepting whispers from %s.", plr->GetName());
    return true;
}

bool ChatHandler::HandleBlockWhispersCommand(const char* args, WorldSession* m_session)
{
    if(args == 0 || strlen(args) < 2)
        return false;

    Player* plr = objmgr.GetPlayer(args, false);
    if(!plr)
    {
        RedSystemMessage(m_session, "Player not found.");
        return true;
    }

    BlueSystemMessage(m_session, "Now blocking whispers from %s.", plr->GetName());
    return true;
}

bool ChatHandler::HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session)
{
    uint32 amt = args ? atol(args) : 0;
    if(!amt)
    {
        RedSystemMessage(m_session, "An amount to increment is required.");
        return true;
    }

    Player* plr = getSelectedChar(m_session);
    if(!plr)
        return true;

    plr->_AdvanceAllSkills(amt);
    GreenSystemMessageToPlr(plr, "Advanced all your skill lines by %u points.", amt);
    if(plr->GetSession() != m_session)
        sWorld.LogGM(m_session, "advanced all skills by %u on %s", amt, plr->GetName());
    return true;
}

bool ChatHandler::HandleKillByPlayerCommand(const char* args, WorldSession* m_session)
{
    if(!args || strlen(args) < 2)
    {
        RedSystemMessage(m_session, "A player's name is required.");
        return true;
    }

    sWorld.DisconnectUsersWithPlayerName(args,m_session);
    return true;
}

bool ChatHandler::HandleKillBySessionCommand(const char* args, WorldSession* m_session)
{
    if(!args || strlen(args) < 2)
    {
        RedSystemMessage(m_session, "A player's name is required.");
        return true;
    }

    sWorld.DisconnectUsersWithAccount(args,m_session);
    return true;
}

bool ChatHandler::HandleMassSummonCommand(const char* args, WorldSession* m_session)
{
    if( !m_session->GetPlayer()->m_massSummonEnabled )
    {
        m_session->GetPlayer()->BroadcastMessage("Please re-enter the command to confirm you wish to summon all players online to your position.");
        m_session->GetPlayer()->BroadcastMessage("This will reset in 30 seconds.");
        m_session->GetPlayer()->m_massSummonEnabled = true;
    }

    ObjectMgr::PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    Player* summoner = m_session->GetPlayer();
    Player* plr;
    uint32 c=0;
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        plr = itr->second;
        if(plr->GetSession() && plr->IsInWorld())
        {
            //plr->SafeTeleport(summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
            /* let's do this the blizz way */
            plr->SummonRequest(summoner, summoner->GetZoneId(), summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
            ++c;
        }
    }
    sWorld.LogGM(m_session, "requested a mass summon of %u players.", c);
    objmgr._playerslock.ReleaseReadLock();
    m_session->GetPlayer()->m_massSummonEnabled = false;
    return true;
}

bool ChatHandler::HandleCastAllCommand(const char* args, WorldSession* m_session)
{
    if(!args || strlen(args) < 2)
    {
        RedSystemMessage(m_session, "No spellid specified.");
        return true;
    }
    Player* plr;
    uint32 spellid = atol(args);
    if(spellid == 0)
        spellid = GetSpellIDFromLink( args );

    SpellEntry * info = dbcSpell.LookupEntry(spellid);
    if(info == NULL)
    {
        RedSystemMessage(m_session, "Invalid spell specified.");
        return true;
    }

    // this makes sure no moron casts a learn spell on everybody and wrecks the server
    for (int i = 0; i < 3; i++)
    {
        if (info->Effect[i] == 36) //SPELL_EFFECT_LEARN_SPELL - 36
        {
            sWorld.LogGM(m_session, "used wrong / learnall castall command, spellid %u", spellid);
            RedSystemMessage(m_session, "Learn spell specified.");
            return true;
        }
    }

    sWorld.LogGM(m_session, "used castall command, spellid %u", spellid);

    ObjectMgr::PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        plr = itr->second;
        if(plr->GetSession() && plr->IsInWorld())
        {
            if(plr->GetMapInstance() != m_session->GetPlayer()->GetMapInstance())
            {

            }
            else
            {
                SpellCastTargets targets(plr->GetGUID());
                if(Spell* sp = new Spell(plr, info))
                    sp->prepare(&targets, true);
            }
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    BlueSystemMessage(m_session, "Casted spell %u on all players!", spellid);
    return true;
}

bool ChatHandler::HandleNpcReturnCommand(const char* args, WorldSession* m_session)
{
    Creature* creature = getSelectedCreature(m_session);
    if(!creature || !creature->IsSpawn()) 
        return true;

    return true;
}

bool ChatHandler::HandleModPeriodCommand(const char* args, WorldSession * m_session)
{
    Transporter* trans = m_session->GetPlayer()->m_CurrentTransporter;
    if(trans == 0)
    {
        RedSystemMessage(m_session, "You must be on a transporter.");
        return true;
    }

    uint32 np = args ? atol(args) : 0;
    if(np == 0)
    {
        RedSystemMessage(m_session, "A time in ms must be specified.");
        return true;
    }

    trans->SetPeriod(np);
    BlueSystemMessage(m_session, "Period of %s set to %u.", trans->GetInfo()->Name, np);
    return true;
}

bool ChatHandler::HandleNpcFollowCommand(const char* args, WorldSession * m_session)
{

    return true;
}

bool ChatHandler::HandleNullFollowCommand(const char* args, WorldSession * m_session)
{

    return true;
}

bool ChatHandler::HandleStackCheatCommand(const char* args, WorldSession * m_session)
{

    return true;
}

bool ChatHandler::HandleTriggerpassCheatCommand(const char* args, WorldSession * m_session)
{

    return true;
}

bool ChatHandler::HandleVendorPassCheatCommand(const char* args, WorldSession * m_session)
{

    return true;
}

bool ChatHandler::HandleItemReqCheatCommand(const char* args, WorldSession * m_session)
{

    return true;
}

bool ChatHandler::HandleResetSkillsCommand(const char* args, WorldSession * m_session)
{
    SkillLineEntry * se;
    Player* plr = getSelectedChar(m_session, true);
    if(!plr) return true;

    // Load skills from create info.
    PlayerCreateInfo * info = objmgr.GetPlayerCreateInfo(plr->getRace(), plr->getClass());
    if(!info) return true;

    //Chances depend on stats must be in this order!
    plr->_UpdateMaxSkillCounts();
    plr->_AddLanguages(false);
    BlueSystemMessage(m_session, "Reset skills to default.");
    sWorld.LogGM(m_session, "reset skills of %s to default", plr->GetName());
    return true;
}

bool ChatHandler::HandlePlayerInfo(const char* args, WorldSession * m_session)
{
    Player* plr;
    if(strlen(args) >= 2) // char name can be 2 letters
    {
        plr = objmgr.GetPlayer(args, false);
        if(!plr)
        {
            RedSystemMessage(m_session, "Unable to locate player %s.", args);
            return true;
        }
    } else if((plr = getSelectedChar(m_session, true)) == NULL)
        return true;

    if(!plr->GetSession())
    {
        RedSystemMessage(m_session, "ERROR: this player hasn't got any session !");
        return true;
    }
    if(!plr->GetSession()->GetSocket())
    {
        RedSystemMessage(m_session, "ERROR: this player hasn't got any socket !");
        return true;
    }
    WorldSession* sess = plr->GetSession();

    GreenSystemMessage(m_session, "%s[%u] is a %s %u %s", plr->GetName(), plr->GetLowGUID(), (plr->getGender()?"Female":"Male"), plr->getRace(), plr->getClassName().c_str());
    BlueSystemMessage(m_session, "%s is connecting from account '%s'[%u] with permissions '%s'", (plr->getGender()?"She":"He"), sess->GetAccountName().c_str(), sess->GetAccountId(), sess->GetPermissions());

    const char *client;
    if(sess->HasFlag(ACCOUNT_FLAG_XPACK_03))
        client = "Cataclysm";
    else if(sess->HasFlag(ACCOUNT_FLAG_XPACK_02))
        client = "Wrath of the Lich King";
    else if(sess->HasFlag(ACCOUNT_FLAG_XPACK_01))
        client = "The Burning Crusade";
    else client = "World of Warcraft";

    BlueSystemMessage(m_session, "%s uses %s (build %u)", (plr->getGender()?"She":"He"), client, sess->GetClientBuild());
    BlueSystemMessage(m_session, "%s IP is '%s:%u', and has a latency of %ums", (plr->getGender()?"Her":"His"), sess->GetSocket()->GetIP(), sess->GetSocket()->GetPort(), sess->GetLatency());
    return true;
}

bool ChatHandler::HandleGlobalPlaySoundCommand(const char* args, WorldSession * m_session)
{
    if(!*args) 
        return false;
    uint32 sound = atoi(args);
    if(!sound) 
        return false;

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound;
    sWorld.SendGlobalMessage(&data, 0);
    BlueSystemMessage(m_session, "Broadcasted sound %u to server.", sound);
    sWorld.LogGM(m_session, "used play all command soundid %u", sound);
    return true;
}

bool ChatHandler::HandleIPBanCommand(const char * args, WorldSession * m_session)
{
    char * pIp = (char*)args;
    char * pDuration = strchr(pIp, ' ');
    if(pDuration == NULL)
        return false;

    std::string sip = std::string(pIp);

    std::string::size_type i = sip.find("/");
    if( i == std::string::npos )
        return false;

    *pDuration = 0;
    ++pDuration;

    int32 timeperiod = RONIN_UTIL::GetTimePeriodFromString(pDuration);
    if(timeperiod < 1)
        return false;

    char * pReason = strchr(pDuration, ' ');
    if( pReason == NULL )
        return false;

    *pReason = 0;
    ++pReason;

    uint32 o1, o2, o3, o4;
    if ( sscanf(pIp, "%3u.%3u.%3u.%3u", (unsigned int*)&o1, (unsigned int*)&o2, (unsigned int*)&o3, (unsigned int*)&o4) != 4
            || o1 > 255 || o2 > 255 || o3 > 255 || o4 > 255)
    {
        RedSystemMessage(m_session, "Invalid IPv4 address [%s]", pIp);
        return true;    // error in syntax, but we wont remind client of command usage
    }

    time_t expire_time;
    if ( timeperiod == 0)       // permanent ban
        expire_time = 0;
    else
        expire_time = UNIXTIME + (time_t)timeperiod;

    SystemMessage(m_session, "Adding [%s] to IP ban table, expires %s", pIp, (expire_time == 0)? "Never" : ctime( &expire_time ));
    sLogonCommHandler.IPBan_Add( pIp, (uint32)expire_time, pReason );
    sWorld.DisconnectUsersWithIP(pIp, m_session);
    sWorld.LogGM(m_session, "banned ip address %s, expires %s", pIp, (expire_time == 0)? "Never" : ctime( &expire_time ));
    return true;
}

bool ChatHandler::HandleIPUnBanCommand(const char * args, WorldSession * m_session)
{
    char ip[16] = {0};      // IPv4 address

    // we require at least one argument, the network address to unban
    if ( sscanf(args, "%15s", ip) < 1)
        return false;

    /**
     * We can afford to be less fussy with the validty of the IP address given since
     * we are only attempting to remove it.
     * Sadly, we can only blindly execute SQL statements on the logonserver so we have
     * no idea if the address existed and so the account/IPBanner cache requires reloading.
     */

    SystemMessage(m_session, "Removing [%s] from IP ban table if it exists", ip);
    sLogonCommHandler.IPBan_Remove( ip );
    sWorld.LogGM(m_session, "unbanned ip address %s", ip);
    return true;
}

bool ChatHandler::HandleCreatureSpawnCommand(const char *args, WorldSession *m_session)
{
    Player* plr = m_session->GetPlayer();
    if(m_session == NULL || plr == NULL)
        return true;
    if(!plr->IsInWorld())
        return false;

    uint32 entry, save;
    if( sscanf(args, "%u %u", &entry, &save) != 2 )
    {
        if( sscanf(args, "%u", &entry) != 1 )
            return false;

        save = (uint32)m_session->CanUseCommand('z');
    }

    MapInstance *mgr = plr->GetMapInstance();
    CreatureData *ctrData = sCreatureDataMgr.GetCreatureData(entry);
    if(ctrData == NULL)
    {
        RedSystemMessage(m_session, "Invalid entry id(%u).", entry);
        return true;
    }

    //Are we on a transporter?
    if(!plr->GetTransportGuid().empty())
    {
        BlueSystemMessage(m_session, "Spawning on transports is not allowed. Spawn has been denied");
        return true;
    }

    Creature* p = plr->GetMapInstance()->CreateCreature(entry);
    if(p == NULL)
    {
        RedSystemMessage(m_session, "Could not create spawn.");
        return true;
    }
    ASSERT(p);

    uint32 mode = mgr->iInstanceMode;
    CreatureSpawn * sp = NULL;

    p->Load(mgr->GetMapId(), plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetOrientation(), mode, sp);
    p->PushToWorld(mgr);

    BlueSystemMessage(m_session, "Spawned a creature `%s` with entry %u at %f %f %f on map %u in phase %u", ctrData->GetFullName(),
        entry, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetMapId(), plr->GetPhaseMask());

    sWorld.LogGM(m_session, "spawned a %s at %u %f %f %f", ctrData->GetFullName(), plr->GetMapId(), plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ() );
    return true;
}

bool ChatHandler::HandleCreatureRespawnCommand(const char *args, WorldSession *m_session)
{
    Creature * cCorpse = getSelectedCreature( m_session, false );
    if( cCorpse == NULL )
        return false;

    if(!cCorpse->IsCreature())
        return false;

    if(!cCorpse->hasStateFlag(UF_CORPSE))
    {
        RedSystemMessage( m_session, "You must select a corpse to respawn a creature.");
        return true;
    }

    if(cCorpse->GetLowGUID() != 0)
    {
        BlueSystemMessage( m_session, "Respawning a Creature: `%s` with entry: %u on map: %u sqlid: %u", cCorpse->GetName(), cCorpse->GetEntry(), cCorpse->GetMapInstance()->GetMapId(), cCorpse->GetLowGUID() );
        sWorld.LogGM(m_session, "Respawned a Creature: `%s` with entry: %u on map: %u sqlid: %u", cCorpse->GetName(), cCorpse->GetEntry(), cCorpse->GetMapInstance()->GetMapId(), cCorpse->GetLowGUID() );
        cCorpse->Respawn(false, false);
        return true;
    }

    RedSystemMessage( m_session, "You must select a valid CreatureSpawn in order to respawn a creature." );
    return true;
}

bool ChatHandler::HandleRemoveItemCommand(const char * args, WorldSession * m_session)
{
    uint32 item_id, count;
    int argc = sscanf(args, "%u %u", (unsigned int*)&item_id, (unsigned int*)&count);
    if(argc == 1) count = 1;
    else if(argc != 2 || !count)
        return false;

    Player* plr = getSelectedChar(m_session, true);
    if(plr == NULL)
        return true;

    //plr->GetInventory()->RemoveInventoryStacks(item_id, count, true);
    sWorld.LogGM(m_session, "used remove item id %u count %u from %s", item_id, count, plr->GetName());
    BlueSystemMessage(m_session, "Removing %u copies of item %u from %s's inventory.", count, item_id, plr->GetName());
    BlueSystemMessage(plr->GetSession(), "%s removed %u copies of item %u from your inventory.", m_session->GetPlayer()->GetName(), count, item_id);
    return true;
}

bool ChatHandler::HandleForceRenameCommand(const char * args, WorldSession * m_session)
{
    // prevent buffer overflow
    if(strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    PlayerInfo * pi = objmgr.GetPlayerInfoByName(tmp.c_str());
    if(pi == NULL)
    {
        RedSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    if(Player* plr = objmgr.GetPlayer(pi->charGuid))
        BlueSystemMessageToPlr(plr, "%s forced your character to be renamed next logon.", m_session->GetPlayer()->GetName());
    else CharacterDatabase.Execute("UPDATE character_data SET forced_rename_pending = 1 WHERE guid = %u", pi->charGuid.getLow());

    CharacterDatabase.Execute("INSERT INTO banned_names VALUES('%s')", CharacterDatabase.EscapeString(pi->charName).c_str());
    GreenSystemMessage(m_session, "Forcing %s to rename his character next logon.", args);
    sWorld.LogGM(m_session, "forced %s to rename his character (%u)", pi->charName.c_str(), pi->charGuid.getLow());
    return true;
}

bool ChatHandler::HandleRecustomizeCharCommand(const char * args, WorldSession * m_session)
{
    // prevent buffer overflow
    if(strlen(args) > 100)
        return false;

    std::string tmp = std::string(args);
    PlayerInfo * pi = objmgr.GetPlayerInfoByName(tmp.c_str());
    if(pi == NULL)
    {
        RedSystemMessage(m_session, "Player with that name not found.");
        return true;
    }

    if(Player* plr = objmgr.GetPlayer(pi->charGuid))
        BlueSystemMessageToPlr(plr, "%s granted you a character recustomization, please relog.", m_session->GetPlayer()->GetName());
    else CharacterDatabase.Execute("UPDATE character_data SET customizable = 1 WHERE guid = %u", pi->charGuid.getLow());

    GreenSystemMessage(m_session, "Granting %s a character recustomization on his/her next character logon.", args);
    sWorld.LogGM(m_session, "Granted %s a character recustomization (%u)", pi->charName.c_str(), pi->charGuid.getLow());
    return true;
}

bool ChatHandler::HandleGetStandingCommand(const char * args, WorldSession * m_session)
{
    uint32 faction = atoi(args);
    Player* plr = getSelectedChar(m_session, true);
    if(!plr)
        return true;

    int32 standing = plr->GetFactionInterface()->GetStanding(faction);
    int32 bstanding = plr->GetFactionInterface()->GetBaseStanding(faction);

    GreenSystemMessage(m_session, "Reputation for faction %u:", faction);
    SystemMessage(m_session, "Base Standing: %d", bstanding);
    BlueSystemMessage(m_session, "Standing: %d", standing);
    return true;
}

bool ChatHandler::HandleSetStandingCommand(const char * args, WorldSession * m_session)
{
    uint32 faction;
    int32 standing;
    if(sscanf(args, "%u %d", (unsigned int*)&faction, (unsigned int*)&standing) != 2)
        return false;
    Player* plr = getSelectedChar(m_session, true);
    if(!plr)
        return true;

    BlueSystemMessage(m_session, "Setting standing of %u to %d on %s.", faction, standing, plr->GetName());
    plr->GetFactionInterface()->SetStanding(faction, standing);
    GreenSystemMessageToPlr(plr, "%s set your standing of faction %u to %d.", m_session->GetPlayer()->GetName(), faction, standing);
    sWorld.LogGM(m_session, "set standing of faction %u to %u for %s", faction,standing,plr->GetName());
    return true;
}

void SendHighlightedName(WorldSession * m_session, char* full_name, std::string& lowercase_name, std::string& highlight, uint32 id, bool item)
{
    char message[1024];
    char start[50];
    start[0] = message[0] = 0;

    snprintf(start, 50, "%s %u: %s", item ? "Item" : "Creature", (unsigned int)id, MSG_COLOR_WHITE);

    std::string::size_type hlen = highlight.length();
    std::string fullname = std::string(full_name);
    std::string::size_type offset = lowercase_name.find(highlight);
    std::string::size_type remaining = fullname.size() - offset - hlen;
    strcat(message, start);
    strncat(message, fullname.c_str(), offset);
    if(remaining > 0)
    {
        strcat(message, MSG_COLOR_LIGHTRED);
        strncat(message, (fullname.c_str() + offset), hlen);
        strcat(message, MSG_COLOR_WHITE);
        strncat(message, (fullname.c_str() + offset + hlen), remaining);
    }

    sChatHandler.SystemMessage(m_session, message);
}

bool ChatHandler::HandleLookupItemCommand(const char * args, WorldSession * m_session)
{
    if(!*args)
        return false;

    uint32 t = getMSTime();
    uint32 itemid = 0, count = 0;
    GetItemIDFromLink(args, &itemid);
    if(itemid != NULL)
    {
        ItemPrototype* proto = sItemMgr.LookupEntry(itemid);
        if(proto == NULL)
            return false;

        BlueSystemMessage(m_session, "Starting search of item...");
        char messagetext[500];
        std::string itemlink = proto->ConstructItemLink(proto->RandomPropId, proto->RandomSuffixId, 1);
        snprintf(messagetext, 500, "Item %u %s", proto->ItemId,itemlink.c_str());
        SystemMessage(m_session, messagetext);  // This is for the luls.
        BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
        return true;
    }

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    if(x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 5 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of item `%s`...", x.c_str());
    t = getMSTime();
    ItemPrototype * it;
    ItemManager::iterator itr = sItemMgr.itemPrototypeBegin();
    while(itr != sItemMgr.itemPrototypeEnd())
    {
        it = (*itr)->second;
        if(RONIN_UTIL::FindXinYString(x, it->lowercase_name))
        {
            // Print out the name in a cool highlighted fashion
            char messagetext[500];
            std::string itemlink = it->ConstructItemLink(it->RandomPropId, it->RandomSuffixId, 1);
            snprintf(messagetext, 500, "Item %u %s", it->ItemId, itemlink.c_str());
            SystemMessage(m_session, messagetext);
            ++count;
            if(count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
        ++itr;
    }

    BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

bool ChatHandler::HandleLookupItemSetCommand(const char * args, WorldSession * m_session)
{
    if(!*args)
        return false;

    uint32 item = 0;
    if(sscanf(args, "%u", &item) < 1)
    {
        // check for item link
        GetItemIDFromLink(args, &item);
        if(item == 0)
            return false;
    }

    ItemPrototype* proto = sItemMgr.LookupEntry(item);
    if(proto == NULL || !proto->ItemSet)
        return false;

    BlueSystemMessage(m_session, "Item exists in set %u rank %u.", proto->ItemSet, proto->ItemSetRank);
    return true;
}

bool ChatHandler::HandleLookupObjectCommand(const char * args, WorldSession * m_session)
{
    if(!*args) 
        return false;

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    StorageContainerIterator<GameObjectInfo> * itr = GameObjectNameStorage.MakeIterator();

    GreenSystemMessage(m_session, "Starting search of object `%s`...", x.c_str());
    uint32 t = getMSTime();
    GameObjectInfo * i;
    uint32 count = 0;
    std::string y, recout;
    while(!itr->AtEnd())
    {
        i = itr->Get();
        y = RONIN_UTIL::TOLOWER_RETURN(i->Name);
        if(RONIN_UTIL::FindXinYString(x, y))
        {
            std::string Name;
            std::stringstream strm;
            strm<<i->ID;
            const char*objectName=i->Name;
            recout="|cfffff000Object ";
            recout+=strm.str();
            recout+="|cffFFFFFF: ";
            recout+=objectName;
            recout = recout + Name;
            SendMultilineMessage(m_session,recout.c_str());

            ++count;
            if(count==25 || count > 25)
            {
                RedSystemMessage(m_session,"More than 25 results returned. aborting.");
                break;
            }
        }
        if(!itr->Inc()) break;
    }
    itr->Destruct();
    if (count== 0)
    {
        recout="|cff00ccffNo matches found.";
        SendMultilineMessage(m_session,recout.c_str());
    }
    BlueSystemMessage(m_session,"Search completed in %u ms.",getMSTime()-t);
    return true;
}

bool ChatHandler::HandleLookupSpellCommand(const char * args, WorldSession * m_session)
{
    if(!*args)
        return false;

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    if(x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of spell `%s`...", x.c_str());
    uint32 t = getMSTime();
    uint32 count = 0;
    char itoabuf[12];
    std::string recout;
    for (uint32 index = 0; index < dbcSpell.GetNumRows(); index++)
    {
        SpellEntry* spell = dbcSpell.LookupRow(index);
        std::string y = RONIN_UTIL::TOLOWER_RETURN(spell->Name);
        if(RONIN_UTIL::FindXinYString(x, y))
        {
            sprintf((char*)itoabuf,"%u",spell->Id);
            recout = (const char*)itoabuf;
            recout += ": |cff71d5ff|Hspell:";
            recout += (const char*)itoabuf;
            recout += "|h[";
            recout += spell->Name;
            recout += "]|h|r";
            std::string::size_type pos = recout.find('%');
            if( pos != std::string::npos )
                recout.insert( pos + 1, "%");
            SendMultilineMessage(m_session, recout.c_str());
            ++count;
            if(count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

bool ChatHandler::HandleLookupSpellSpecificCommand(const char * args, WorldSession * m_session)
{
    if(!*args)
        return false;

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    if(x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of spell `%s`...", x.c_str());
    uint32 t = getMSTime();
    std::string recout;
    char itoabuf[12];
    for (uint32 index = 0; index < dbcSpell.GetNumRows(); index++)
    {
        SpellEntry* spell = dbcSpell.LookupRow(index);
        std::string y = RONIN_UTIL::TOLOWER_RETURN(spell->Name);
        if(!strcmp(x.c_str(), y.c_str()))
        {
            sprintf((char*)itoabuf,"%u",spell->Id);
            recout = (const char*)itoabuf;
            recout += ": |cff71d5ff|Hspell:";
            recout += (const char*)itoabuf;
            recout += "|h[";
            recout += spell->Name;
            recout += "]|h|r";
            std::string::size_type pos = recout.find('%');
            if( pos != std::string::npos )
                recout.insert( pos + 1, "%");
            SendMultilineMessage(m_session, recout.c_str());
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

bool ChatHandler::HandleLookupSpellNameCommand(const char * args, WorldSession * m_session)
{
    if(!*args)
        return false;

    uint32 id = atol(args);
    if(!id)
        return false;

    SpellEntry* se = dbcSpell.LookupEntry(id);
    if(se == NULL)
        return false;

    BlueSystemMessage(m_session, "|Spell name: %s||Description: %s|", se->Name, se->Description);
    return true;
}

bool ChatHandler::HandleLookupCreatureCommand(const char * args, WorldSession * m_session)
{
    if(!*args) 
        return false;

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    if(x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of creature `%s`...", x.c_str());
    uint32 t = getMSTime(), count = 0;
    CreatureData *data = NULL;
    CreatureDataManager::iterator itr = sCreatureDataMgr.begin();
    while(itr != sCreatureDataMgr.end())
    {
        data = (*itr)->second;
        if(RONIN_UTIL::FindXinYString(x, data->maleName.c_str()))
        {
            char messagetext[500];
            snprintf(messagetext, 500, "Creature[%05u]: %s", data->entry, data->maleName.c_str());
            SystemMessage(m_session, messagetext);
        }
        else if(RONIN_UTIL::FindXinYString(x, data->femaleName.c_str()))
        {
            char messagetext[500];
            snprintf(messagetext, 500, "Creature[%05u]: %s", data->entry, data->femaleName.c_str());
            SystemMessage(m_session, messagetext);
        }
        ++itr;
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

bool ChatHandler::HandleLookupTitleCommand(const char *args, WorldSession *m_session)
{
    if(!*args)
        return false;

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    if(x.length() < 3)
    {
        RedSystemMessage(m_session, "Your search string must be at least 3 characters long.");
        return true;
    }

    uint32 count = 0;
    for(uint32 i = 0; i < dbcCharTitle.GetNumRows(); i++)
    {
        CharTitleEntry *entry = dbcCharTitle.LookupRow(i);
        if(!RONIN_UTIL::FindXinYString(x, RONIN_UTIL::TOLOWER_RETURN(entry->titleFormat)))
            continue;
        BlueSystemMessage(m_session, "Title %03u: %s", entry->Id, format(entry->titleFormat, m_session->GetPlayer()->GetName()).c_str());
        if(++count == 25)
            break;
    }

    if(count == 0) RedSystemMessage(m_session, "Unable to find any titles with %s in the name", x.c_str());
    return true;
}

bool ChatHandler::HandleLookupCurrencyCommand(const char *args, WorldSession *m_session)
{
    if(!*args)
        return false;

    std::string x = RONIN_UTIL::TOLOWER_RETURN(args);
    if(x.length() < 3)
    {
        RedSystemMessage(m_session, "Your search string must be at least 3 characters long.");
        return true;
    }

    uint32 count = 0;
    for(uint32 i = 0; i < dbcCurrencyType.GetNumRows(); i++)
    {
        CurrencyTypeEntry *entry = dbcCurrencyType.LookupRow(i);
        if(entry->Flags > 100)
            sLog.printf("");
        if(!RONIN_UTIL::FindXinYString(x, RONIN_UTIL::TOLOWER_RETURN(entry->name)))
            continue;
        BlueSystemMessage(m_session, "Currency %04u: %s", entry->Id, format(entry->name, m_session->GetPlayer()->GetName()).c_str());
        if(++count == 25)
            break;
    }

    if(count == 0) RedSystemMessage(m_session, "Unable to find any titles with %s in the name", x.c_str());
    return true;
}

bool ChatHandler::HandleGORotate(const char * args, WorldSession * m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *go = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( go == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    float radius = atof(args)+go->GetOrientation();
    go->SetOrientation(radius);
    go->RemoveFromWorld();
    go->PushToWorld(m_session->GetPlayer()->GetMapInstance());
    go->SaveToDB();
    return true;
}

bool ChatHandler::HandleGOMove(const char * args, WorldSession * m_session)
{
    // move the go to player's coordinates
    Player *plr = m_session->GetPlayer();
    GameObject *go = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( go == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    go->RemoveFromWorld();
    go->SetPosition(m_session->GetPlayer()->GetPosition());
    go->SaveToDB();
    go->PushToWorld(m_session->GetPlayer()->GetMapInstance());
    return true;
}

bool ChatHandler::HandleNpcPossessCommand(const char * args, WorldSession * m_session)
{
    Unit* pTarget = getSelectedChar(m_session, false);
    if(!pTarget)
    {
        pTarget = getSelectedCreature(m_session, false);
        if(pTarget && pTarget->GetUInt32Value(UNIT_FIELD_CREATEDBY))
            return false;
    }

    if(!pTarget)
    {
        RedSystemMessage(m_session, "You must select a player/creature.");
        return true;
    }

    m_session->GetPlayer()->Possess(pTarget);
    BlueSystemMessage(m_session, "Possessed %llu", pTarget->GetGUID());
    return true;
}

bool ChatHandler::HandleNpcUnPossessCommand(const char * args, WorldSession * m_session)
{
    Creature* creature = getSelectedCreature(m_session);
    m_session->GetPlayer()->UnPossess();

    GreenSystemMessage(m_session, "Removed any possessed targets.");
    return true;
}

bool ChatHandler::HandleRehashCommand(const char * args, WorldSession * m_session)
{
    /* rehashes */
    char msg[250];
    snprintf(msg, 250, "%s is rehashing config file.", m_session->GetPlayer()->GetName());
    sWorld.SendWorldWideScreenText(msg, 0);
    sWorld.SendWorldText(msg, 0);
    sWorld.Rehash(false);
    return true;
}

bool ChatHandler::HandleGuildRemovePlayerCommand(const char* args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session);
    if(plr == NULL || !plr->IsInGuild())
        return false;

    guildmgr.RemoveMember(m_session->GetPlayer(), plr->getPlayerInfo());
    GreenSystemMessage(m_session, "Member removed succesfully.");
    return true;
}

bool ChatHandler::HandleGuildDisbandCommand(const char* args, WorldSession *m_session)
{
    if(!std::string(args).length())
    {
        Player* plr = getSelectedChar(m_session);
        if(plr == NULL || !plr->IsInGuild())
            return false;

        if(!guildmgr.Disband(plr->GetGuildId()))
            return false;
    }
    else
    {
        GuildInfo* gInfo = guildmgr.GetGuildByGuildName(std::string(args));
        if(gInfo == NULL)
            return false;

        uint32 GuildId = gInfo->m_guildId;
        gInfo = NULL;

        if(!guildmgr.Disband(GuildId))
            return false;
    }

    GreenSystemMessage(m_session,"Guild disbanded succesfully.");
    return true;
}

bool ChatHandler::HandleGuildModifyLevelCommand(const char *args, WorldSession *m_session)
{
    Player *plr = getSelectedChar(m_session);
    if(plr == NULL || !plr->IsInGuild())
        return false;

    if(GuildInfo *gInfo = guildmgr.GetGuildInfo(plr->GetGuildId()))
        guildmgr.ModifyGuildLevel(gInfo, atol(args));
    return true;
}

bool ChatHandler::HandleGuildGainXPCommand(const char *args, WorldSession *m_session)
{
    Player *plr = getSelectedChar(m_session);
    if(plr == NULL || !plr->IsInGuild())
        return false;

    guildmgr.GuildGainXP(plr, labs(atol(args)));
    return true;
}

bool ChatHandler::HandleCreateArenaTeamCommands(const char * args, WorldSession * m_session)
{
    return true;
}

bool ChatHandler::HandleWhisperBlockCommand(const char * args, WorldSession * m_session)
{
    if(m_session->GetPlayer()->hasGMTag())
        return false;

    return true;
}

bool ChatHandler::HandleShowItems(const char * args, WorldSession * m_session)
{
    if(Player* plr = getSelectedChar(m_session, true))
    {

    }

    return true;
}

bool ChatHandler::HandleCollisionTestIndoor(const char * args, WorldSession * m_session)
{
    Player* plr = m_session->GetPlayer();
    if(plr != NULL)
    {
        if(sWorld.Collision)
        {
            if (plr->GetMapInstance()->CanUseCollision(plr))
            {
                const LocationVector & loc = plr->GetPosition();
                bool res = false;//sVMapInterface.IsIndoor(plr->GetMapId(), loc.x, loc.y, loc.z + 2.0f);
                SystemMessage(m_session, "Result was: %s.", res ? "indoors" : "outside");
            } else SystemMessage(m_session, "Collision is not available here.");
        } else SystemMessage(m_session, "Ronin was does not have collision enabled.");
    }
    return true;
}

bool ChatHandler::HandleCollisionTestLOS(const char * args, WorldSession * m_session)
{
    if(sWorld.Collision)
    {
        WorldObject* pObj = getSelectedUnit(m_session, false);
        if(pObj == NULL)
        {
            SystemMessage(m_session, "Invalid target.");
            return true;
        }

        if(pObj->GetMapInstance()->CanUseCollision(pObj))
        {
            const LocationVector & loc2 = pObj->GetPosition();
            const LocationVector & loc1 = m_session->GetPlayer()->GetPosition();
            bool res = sVMapInterface.CheckLOS(pObj->GetMapId(), pObj->GetInstanceID(), pObj->GetPhaseMask(), loc1.x, loc1.y, loc1.z, loc2.x, loc2.y, loc2.z);
            bool res1 = sVMapInterface.CheckLOS(pObj->GetMapId(), pObj->GetInstanceID(), pObj->GetPhaseMask(), loc1.x, loc1.y, loc1.z+1.0f, loc2.x, loc2.y, loc2.z+1.0f);
            bool res2 = sVMapInterface.CheckLOS(pObj->GetMapId(), pObj->GetInstanceID(), pObj->GetPhaseMask(), loc1.x, loc1.y, loc1.z+2.0f, loc2.x, loc2.y, loc2.z+2.0f);
            bool res5 = sVMapInterface.CheckLOS(pObj->GetMapId(), pObj->GetInstanceID(), pObj->GetPhaseMask(), loc1.x, loc1.y, loc1.z+5.0f, loc2.x, loc2.y, loc2.z+5.0f);
            bool objectfunction = m_session->GetPlayer()->IsInLineOfSight(pObj);
            SystemMessage(m_session, "Difference 0: Result was: %s.", res ? "in LOS" : "not in LOS");
            SystemMessage(m_session, "Difference 1: Result was: %s.", res1 ? "in LOS" : "not in LOS");
            SystemMessage(m_session, "Difference 2: Result was: %s.", res2 ? "in LOS" : "not in LOS");
            SystemMessage(m_session, "Difference 5: Result was: %s.", res5 ? "in LOS" : "not in LOS");
            SystemMessage(m_session, "ObjectFunction: Result was: %s.", objectfunction ? "in LOS" : "not in LOS");
            return true;
        }

        SystemMessage(m_session, "Collision is not available here.");
        return true;
    }

    SystemMessage(m_session, "Ronin was does not have collision enabled.");
    return true;
}

bool ChatHandler::HandleCollisionGetHeight(const char * args, WorldSession * m_session)
{
    Player* plr = m_session->GetPlayer();
    if(plr == NULL)
        return true;

    SystemMessage(m_session, "Results: Curr pos: %f; Water: %f;", plr->GetGroundHeight(), plr->GetLiqHeight());
    return true;
}

bool ChatHandler::HandleFixScaleCommand(const char * args, WorldSession * m_session)
{
    Creature * pCreature = getSelectedCreature(m_session, true);
    if( pCreature == NULL )
        return true;

    float sc = (float)atof(args);
    if(sc < 0.1f)
    {
        sc = GetDBCScale( dbcCreatureDisplayInfo.LookupEntry(pCreature->GetUInt32Value(UNIT_FIELD_DISPLAYID)));
        SystemMessage(m_session, "Using scale %f from DBCDisplayInfo.", sc);
    } else SystemMessage(m_session, "Scale override set to %f (DBCDisplayInfo = %f).", sc, GetDBCScale( dbcCreatureDisplayInfo.LookupEntry( pCreature->GetUInt32Value(UNIT_FIELD_DISPLAYID) )));

    pCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, sc);
    return true;
}

bool ChatHandler::HandleClearBonesCommand(const char *args, WorldSession *m_session)
{
    Player* p = m_session->GetPlayer();
    sWorld.LogGM(m_session, "cleared bones on map %u at %f %f %f", p->GetMapId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ());

    WorldObject* obj;
    for( WorldObject::InRangeHashMap::iterator itr = p->GetInRangeMapBegin(); itr != p->GetInRangeMapEnd(); itr++)
    {
        if((obj = itr->second) == NULL)
            continue;

        if( obj->GetTypeId() == TYPEID_CORPSE && castPtr<Corpse>(obj)->GetCorpseState() == CORPSE_STATE_BONES )
            castPtr<Corpse>(obj)->Despawn();
    }

    SystemMessage(m_session, "Completed.");
    return true;
}

bool ChatHandler::HandleClearCorpsesCommand(const char *args, WorldSession *m_session)
{
    Player* p = m_session->GetPlayer();
    sWorld.LogGM(m_session, "cleared corpses on map %u at %f %f %f", p->GetMapId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ());

    WorldObject* obj;
    for( WorldObject::InRangeHashMap::iterator itr = p->GetInRangeMapBegin(); itr != p->GetInRangeMapEnd(); itr++)
    {
        if((obj = itr->second) == NULL)
            continue;

        if( obj->GetTypeId() == TYPEID_CORPSE && castPtr<Corpse>(obj)->GetCorpseState() == CORPSE_STATE_BODY )
            castPtr<Corpse>(obj)->Despawn();
    }

    SystemMessage(m_session, "Completed.");
    return true;
}

bool ChatHandler::HandleMultiMuteCommand(const char *args, WorldSession *m_session)
{
    std::vector<std::string> real_args = RONIN_UTIL::StrSplit(args, " ");
    if( real_args.size() < 3 )
        return false;

    const char *reason = real_args[0].c_str();
    int32 timespan = RONIN_UTIL::GetTimePeriodFromString(real_args[1].c_str());
    if( timespan <= 0 )
        return false;

    std::string tsstr = RONIN_UTIL::ConvertTimeStampToDataTime((uint32)timespan+(uint32)UNIXTIME);
    uint32 i;
    char msg[200];

    for(i = 2; i < real_args.size(); i++)
    {
        Player* pPlayer = objmgr.GetPlayer(real_args[i].c_str(), false);
        if( pPlayer == NULL )
        {
            SystemMessage(m_session, "Could not find player, %s.\n", real_args[i].c_str());
            continue;
        }

        pPlayer->GetSession()->SystemMessage("Your voice has been muted until %s by a GM. Until this time, you will not be able to speak in any form. Reason: %s", tsstr.c_str(), reason);
        sLogonCommHandler.Account_SetMute(pPlayer->GetSession()->GetAccountNameS(), (uint32)timespan + (uint32)UNIXTIME);
        sWorld.LogGM(m_session, "muted account %s until %s", pPlayer->GetSession()->GetAccountNameS(), RONIN_UTIL::ConvertTimeStampToDataTime((uint32)timespan+(uint32)UNIXTIME).c_str());

        snprintf(msg, 200, "%s%s was muted by %s (%s)", MSG_COLOR_WHITE, pPlayer->GetName(), m_session->GetPlayer()->GetName(), reason);
        sWorld.SendWorldText(msg, NULL);
    }

    return true;
}

bool ChatHandler::HandleMultiKickCommand(const char *args, WorldSession *m_session)
{
    std::vector<std::string> real_args = RONIN_UTIL::StrSplit(args, " ");
    if( real_args.size() < 2 )
        return false;

    const char *reason = real_args[0].c_str();
    uint32 i;
    char msg[200];

    for(i = 1; i < real_args.size(); i++)
    {
        Player* pPlayer = objmgr.GetPlayer(real_args[i].c_str(), false);
        if( pPlayer == NULL )
        {
            SystemMessage(m_session, "Could not find player, %s.\n", real_args[i].c_str());
            continue;
        }

        snprintf(msg, 200, "%s%s was kicked by %s (%s)", MSG_COLOR_WHITE, pPlayer->GetName(), m_session->GetPlayer()->GetName(), reason);
        sWorld.SendWorldText(msg, NULL);
        pPlayer->Kick(6000);
    }

    return true;
}

bool ChatHandler::HandleMultiBanCommand(const char *args, WorldSession *m_session)
{
    std::vector<std::string> real_args = RONIN_UTIL::StrSplit(args, " ");
    if( real_args.size() < 3 )
        return false;

    const char *reason = real_args[0].c_str();
    int32 timespan = RONIN_UTIL::GetTimePeriodFromString(real_args[1].c_str());
    if( timespan <= 0 )
        return false;

    std::string tsstr = RONIN_UTIL::ConvertTimeStampToDataTime((uint32)timespan+(uint32)UNIXTIME);
    uint32 i;
    char msg[200];

    for(i = 2; i < real_args.size(); i++)
    {
        Player* pPlayer = objmgr.GetPlayer(real_args[i].c_str(), false);
        if( pPlayer == NULL )
        {
            SystemMessage(m_session, "Could not find player, %s.\n", real_args[i].c_str());
            continue;
        }

        pPlayer->GetSession()->SystemMessage("Your have been character banned until %s by a GM. Until this time, you will not be able login on this character. Reason was: %s", tsstr.c_str(), reason);
        pPlayer->SetBanned((uint32)timespan+(uint32)UNIXTIME, real_args[0]);
        pPlayer->Kick(15000);
        sWorld.LogGM(m_session, "banned player %s until %s for %s", pPlayer->GetName(), RONIN_UTIL::ConvertTimeStampToDataTime((uint32)timespan+(uint32)UNIXTIME).c_str(), reason);

        snprintf(msg, 200, "%s%s was banned by %s (%s)", MSG_COLOR_WHITE, pPlayer->GetName(), m_session->GetPlayer()->GetName(), reason);
        sWorld.SendWorldText(msg, NULL);
    }

    return true;
}

bool ChatHandler::HandleMultiAccountBanCommand(const char *args, WorldSession *m_session)
{
    std::vector<std::string> real_args = RONIN_UTIL::StrSplit(args, " ");
    if( real_args.size() < 3 )
        return false;

    const char *reason = real_args[0].c_str();
    int32 timespan = RONIN_UTIL::GetTimePeriodFromString(real_args[1].c_str());
    if( timespan <= 0 )
        return false;

    std::string tsstr = RONIN_UTIL::ConvertTimeStampToDataTime((uint32)timespan+(uint32)UNIXTIME);
    uint32 i;
    char msg[200];

    for(i = 2; i < real_args.size(); i++)
    {
        Player* pPlayer = objmgr.GetPlayer(real_args[i].c_str(), false);
        if( pPlayer == NULL )
        {
            SystemMessage(m_session, "Could not find player, %s.\n", real_args[i].c_str());
            continue;
        }

        pPlayer->GetSession()->SystemMessage("Your have been account banned until %s by a GM. Until this time, you will not be able to log in on this account. Reason: %s", tsstr.c_str(), reason);
        sLogonCommHandler.Account_SetBanned(pPlayer->GetSession()->GetAccountNameS(), (uint32)timespan + (uint32)UNIXTIME, reason);
        sWorld.LogGM(m_session, "banned account %s until %s", pPlayer->GetSession()->GetAccountNameS(), RONIN_UTIL::ConvertTimeStampToDataTime((uint32)timespan+(uint32)UNIXTIME).c_str());

        snprintf(msg, 200, "%s%s was account banned by %s (%s)", MSG_COLOR_WHITE, pPlayer->GetName(), m_session->GetPlayer()->GetName(), reason);
        sWorld.SendWorldText(msg, NULL);
        pPlayer->Kick(15000);
    }

    return true;
}

bool ChatHandler::HandleEnableAH(const char *args, WorldSession *m_session)
{
    BlueSystemMessage(m_session, "Auction House Enabled, staff has been alerted.");
    sWorld.SendMessageToGMs(m_session, "%s has disabled the auction house", (m_session->GetPlayer() ? m_session->GetPlayer()->GetName() : m_session->GetAccountNameS()));
    sWorld.AHEnabled = true;
    return true;
}

bool ChatHandler::HandleDisableAH(const char *args, WorldSession *m_session)
{
    BlueSystemMessage(m_session, "Auction house Disabled, staff has been alerted.");
    sWorld.SendMessageToGMs(m_session, "%s has enabled the auction house", (m_session->GetPlayer() ? m_session->GetPlayer()->GetName() : m_session->GetAccountNameS()));
    sWorld.AHEnabled = false;
    return true;
}

bool ChatHandler::HandleFactionSetStanding(const char *args, WorldSession *m_session)
{
    Player* pPlayer = getSelectedChar(m_session, true);
    if(pPlayer == NULL)
        return true;

    uint32 standing, faction;
    if(sscanf(args, "%u %u", &faction, &standing) != 2)
        return false;

    FactionEntry* RealFaction = dbcFaction.LookupEntry(faction);
    if(RealFaction == NULL || RealFaction->RepListIndex < 0)
    {
        std::stringstream ss;
        ss << "Incorrect faction, searching...";
        FactionTemplateEntry* FactionTemplate = dbcFactionTemplate.LookupEntry(faction);
        if(FactionTemplate == NULL)
        {
            ss << " Faction template not found.";
            RedSystemMessage(m_session, ss.str().c_str());
            return true;
        }
        RealFaction = dbcFaction.LookupEntry(FactionTemplate->Faction);
        ss << " Template found!";
        BlueSystemMessage(m_session, ss.str().c_str());
    }

    if(RealFaction->RepListIndex < 0)
    {
        RedSystemMessage(m_session, "Not a valid faction!");
        return true;
    }

    pPlayer->GetFactionInterface()->SetStanding(faction, standing);
    return true;
}

bool ChatHandler::HandleFactionModStanding(const char *args, WorldSession *m_session)
{
    Player* pPlayer = getSelectedChar(m_session, true);
    if(pPlayer == NULL)
        return true;

    uint32 faction;
    int32 standing;
    if(sscanf(args, "%u %i", &faction, &standing) != 2)
        return false;

    FactionEntry* RealFaction = dbcFaction.LookupEntry(faction);
    if(RealFaction == NULL || RealFaction->RepListIndex < 0)
    {
        std::stringstream ss;
        ss << "Incorrect faction, searching...";
        FactionTemplateEntry* FactionTemplate = dbcFactionTemplate.LookupEntry(faction);
        if(FactionTemplate == NULL)
        {
            ss << " Faction template not found.";
            RedSystemMessage(m_session, ss.str().c_str());
            return true;
        }
        RealFaction = dbcFaction.LookupEntry(FactionTemplate->Faction);
        ss << " Template found!";
        BlueSystemMessage(m_session, ss.str().c_str());
    }

    if(RealFaction->RepListIndex < 0)
    {
        RedSystemMessage(m_session, "Not a valid faction!");
        return true;
    }

    pPlayer->GetFactionInterface()->ModStanding(RealFaction->ID, standing);
    return true;
}
