/***
 * Demonstrike Core
 */

///////////////////////////////////////////////
//  Admin Movement Commands
//

#include "StdAfx.h"

bool ChatHandler::HandleResetReputationCommand(const char *args, WorldSession *m_session)
{
    Player* plr = getSelectedChar(m_session);
    if(plr == NULL)
    {
        SystemMessage(m_session, "Select a player or yourself first.");
        return true;
    }

    SystemMessage(m_session, "Done. Relog for changes to take effect.");
    sWorld.LogGM(m_session, "used reset reputation for %s", plr->GetName());
    return true;
}

bool ChatHandler::CreateGuildCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    Player* ptarget = getSelectedChar(m_session);
    if(ptarget == NULL)
        return true;

    std::string guildName = args;
    if(guildName.length() > 75)
    {
        // send message to user
        char buf[256];
        snprintf((char*)buf,256,"The name was too long by %zi", guildName.length()-75);
        SystemMessage(m_session, buf);
        return true;
    }

    for (uint32 i = 0; i < guildName.length(); i++)
    {
        if(!isalpha(guildName[i]) && guildName[i] != ' ')
        {
            SystemMessage(m_session, "Error, name can only contain chars A-Z and a-z.");
            return true;
        }
    }

    GuildInfo* pGuild = guildmgr.GetGuildByGuildName(guildName);
    if(pGuild)
    {
        RedSystemMessage(m_session, "Guild name is already taken.");
        return true;
    }

    guildmgr.CreateGuildFromCommand(guildName, ptarget->GetLowGUID());
    SystemMessage(m_session, "Guild created");
    return true;
}

bool ChatHandler::HandleDeleteCommand(const char* args, WorldSession *m_session)
{
    Creature* unit = getSelectedCreature(m_session, false);
    if(!unit)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }
    else if(unit->IsSummon())
    {
        SystemMessage(m_session, "You can't delete playerpets.");
        return true;
    }

    if( unit->IsSpawn() && !m_session->CanUseCommand('z') )
    {
        SystemMessage(m_session, "You do not have permission to do that. Please contact higher staff for removing of saved spawns.");
        return true;
    }

    sWorld.LogGM(m_session, "used npc delete, sqlid %u, creature %s, pos %f %f %f", unit->GetLowGUID(), unit->GetName(), unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());
    BlueSystemMessage(m_session, "Deleted creature ID %u", unit->GetLowGUID());
    unit->DeleteFromDB();
    if(!unit->IsInWorld())
        return true;

    MapInstance* unitInstance = unit->GetMapInstance();
    if(unit->IsSpawn())
    {
        uint32 cellx = unitInstance->GetPosX(unit->GetSpawn()->x);
        uint32 celly = unitInstance->GetPosX(unit->GetSpawn()->y);
        if(cellx <= _sizeX && celly <= _sizeY )
        {
            if(CellSpawns *c = unitInstance->GetBaseMap()->GetSpawnsList(cellx, celly))
            {
                for(CreatureSpawnList::iterator itr = c->CreatureSpawns.begin(); itr != c->CreatureSpawns.end();)
                {
                    if(*itr == unit->GetSpawn())
                    {
                        c->CreatureSpawns.erase(itr);
                        delete unit->GetSpawn();
                        break;
                    }
                    ++itr;
                }
            }
        }
    }
    unit->RemoveFromWorld();
    unit->Destruct();

    m_session->GetPlayer()->SetSelection(NULL);
    return true;
}

bool ChatHandler::HandleDeMorphCommand(const char* args, WorldSession *m_session)
{
    Unit *unit = getSelectedUnit(m_session);
    if(!unit)
        return false;
    unit->DeMorph();
    return true;
}

bool ChatHandler::HandleItemCommand(const char* args, WorldSession *m_session)
{
    if(strlen(args) < 1)
        return false;

    Creature* pCreature = getSelectedCreature(m_session, false);
    if(!pCreature || !pCreature->IsSpawn() || !(pCreature->HasNpcFlag(UNIT_NPC_FLAG_VENDOR) || pCreature->HasNpcFlag(UNIT_NPC_FLAG_ARMORER)))
    {
        SystemMessage(m_session, "You should select a vendor.");
        return true;
    }

    uint32 item = 0, extendedcost = 0, vendormask = 0;
    if(sscanf(args, "%u %u %u", &item, &extendedcost, &vendormask) < 1)
    {
        // check for item link
        GetItemIDFromLink(args, &item);
        if(item == 0)
            return false;
    }
    if(item == 0)
        return false;

    if(vendormask == 0)
        vendormask = pCreature->GetVendorMask();

    ItemPrototype* tmpItem = sItemMgr.LookupEntry(item);
    std::stringstream sstext;
    if(tmpItem)
    {
        std::stringstream ss;
        ss << "INSERT INTO vendors(entry, item, extendedcost, vendormask) VALUES ('" << pCreature->GetEntry() << "', '" << item << "', " << extendedcost << ", " << vendormask << " );";
        WorldDatabase.Execute( ss.str().c_str() );

        pCreature->AddVendorItem(item, vendormask, extendedcost);

        sstext << "Item '" << item << "' '" << tmpItem->Name.c_str() << "' Added to list" << '\0';
    } else sstext << "Item '" << item << "' Not Found in Database." << '\0';

    sWorld.LogGM(m_session, "added item %u to vendor %u", item, pCreature->GetEntry());
    SystemMessage(m_session,  sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleItemRemoveCommand(const char* args, WorldSession *m_session)
{
    if (!args)
        return false;

    Creature* pCreature = getSelectedCreature(m_session, false);
    if(!pCreature || !(pCreature->HasNpcFlag(UNIT_NPC_FLAG_VENDOR) || pCreature->HasNpcFlag(UNIT_NPC_FLAG_ARMORER)))
    {
        SystemMessage(m_session, "You should select a vendor.");
        return true;
    }

    uint32 itemguid = 0;
    if(sscanf(args, "%u", &itemguid) < 1)
    {
        // check for item link
        GetItemIDFromLink(args, &itemguid);
        if(itemguid == 0)
            return false;
    }
    if(itemguid == 0)
        return false;

    std::stringstream sstext;
    if(pCreature->GetSlotByItemId(itemguid) != 0xFFFFFFFF)
    {
        std::stringstream ss;
        ss << "DELETE FROM vendors WHERE entry = '" << pCreature->GetEntry() << "' AND vendormask = '" << pCreature->GetVendorMask() << "' AND item = " << itemguid << " LIMIT 1;";
        WorldDatabase.Execute( ss.str().c_str() );

        if(ItemPrototype* tmpItem = sItemMgr.LookupEntry(itemguid))
            sstext << "Item '" << itemguid << "' '" << tmpItem->Name.c_str() << "' Deleted from list" << '\0';
        else sstext << "Item '" << itemguid << "' Deleted from list" << '\0';
    }else sstext << "Item '" << itemguid << "' Not Found in List." << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleNPCFlagCommand(const char* args, WorldSession *m_session)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature(m_session, false);
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 npcFlags = (uint32) atoi((char*)args);
    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);
    pCreature->SaveToDB();
    WorldDatabase.Execute("UPDATE creature_proto SET npcflags = %u WHERE entry = %u", npcFlags, pCreature->GetEntry());

    SystemMessage(m_session, "Value saved, you may need to rejoin or clean your client cache.");
    return true;
}

bool ChatHandler::HandleSaveAllCommand(const char *args, WorldSession *m_session)
{
    ObjectMgr::PlayerStorageMap::const_iterator itr;
    uint32 stime = getMSTime(), count = 0;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
    {
        if(itr->second->GetSession())
        {
            itr->second->SaveToDB(false);
            count++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();
    char msg[100];
    snprintf(msg, 100, "Saved %u online players in %ums.", count, getMSTime() - stime);
    sWorld.SendWorldText(msg);
    sWorld.SendWorldWideScreenText(msg);
    sWorld.LogGM(m_session, "saved all players");
    return true;
}

bool ChatHandler::HandleKillCommand(const char *args, WorldSession *m_session)
{
    Unit* unit = getSelectedUnit(m_session);
    if(!unit)
        return true;

    sWorld.LogGM(m_session, "used kill command on %s %s", unit->IsPlayer() ? "Player" : "Creature", unit->GetName());

    if(unit->IsPlayer())
    {
        // If we're killing a player, send a message indicating a gm killed them.
        BlueSystemMessageToPlr(castPtr<Player>(unit), "%s killed you with a GM command.", m_session->GetPlayer()->GetName());
        castPtr<Player>(unit)->SetUInt32Value(UNIT_FIELD_HEALTH, 0); // Die, insect
        castPtr<Player>(unit)->KillPlayer();
        GreenSystemMessage(m_session, "Killed player %s.", unit->GetName());
    }
    else
    {
        m_session->GetPlayer()->DealDamage(unit, 0xFFFFFFFF, 0, 0, 0);
        GreenSystemMessage(m_session, "Killed unit %s.", unit->GetName());
    }
    return true;
}

bool ChatHandler::HandleKillByPlrCommand( const char *args , WorldSession *m_session )
{
    Player* plr = objmgr.GetPlayer(args, false);
    if(!plr)
    {
        RedSystemMessage(m_session, "Player %s is not online or does not exist.", args);
        return true;
    }

    if(plr->isDead())
    {
        RedSystemMessage(m_session, "Player %s is already dead.", args);
    } 
    else 
    {
        plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0); // Die, insect
        plr->KillPlayer();
        BlueSystemMessageToPlr(plr, "You were killed by %s with a GM command.", m_session->GetPlayer()->GetName());
        GreenSystemMessage(m_session, "Killed player %s.", args);
        sWorld.LogGM(m_session, "remote killed  %s", plr->GetName() );
    }
    return true;
}

bool ChatHandler::HandleCastSpellCommand(const char* args, WorldSession *m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = getSelectedUnit(m_session, false);
    if(!target)
        target = caster;

    uint32 spellid = atol(args);
    if(spellid == 0)
        spellid = GetSpellIDFromLink( args );

    SpellEntry *spellentry = dbcSpell.LookupEntry(spellid);
    if(spellentry == NULL)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    BlueSystemMessage(m_session, "Casting spell %d on target.", spellid);
    SpellCastTargets targets;
    targets.m_unitTarget = target->GetGUID();
    if(Spell* sp = new Spell(caster, spellentry))
        sp->prepare(&targets, false);
    sWorld.LogGM(m_session, "Used castspell command on %s" , target->GetName());
    return true;
}

bool ChatHandler::HandleMonsterCastCommand(const char * args, WorldSession * m_session)
{
    Unit* crt = getSelectedCreature(m_session, false);
    if(crt == NULL)
    {
        RedSystemMessage(m_session, "Please select a creature before using this command.");
        return true;
    }

    uint32 spellId = (uint32)atoi(args);
    if(spellId == 0)
        spellId = GetSpellIDFromLink( args );

    SpellEntry * tmpsp = dbcSpell.LookupEntry(spellId);
    if(tmpsp == NULL)
        return false;

    crt->CastSpell(reinterpret_cast<Unit*>(NULL), tmpsp, false);
    sWorld.LogGM(m_session, "Used npc cast command on %s", crt->GetName());
    return true;
}

bool ChatHandler::HandleNPCEquipCommand(const char * args, WorldSession * m_session)
{
    Creature* crt = getSelectedCreature(m_session, false);
    if(crt == NULL)
    {
        RedSystemMessage(m_session, "Please select a creature before using this command.");
        return true;
    }

    if(strlen(args) < 1)
        return false;

    uint32 slot = 0, itemid = 0;
    if(sscanf(args, "%u %u", &slot, &itemid) < 1)
        return false;

    if(slot > 2)
        return false;

    crt->SetWeaponDisplayId(slot, itemid);
    if(crt->IsSpawn())
        crt->SaveToDB();

    BlueSystemMessage(m_session, "Equipped item %u in creature's %s", itemid, ((slot == 0) ? "Main hand" : (slot == 1) ? "Off hand" : "Ranged slot"));
    sWorld.LogGM(m_session, "Equipped item %u in creature's %s", itemid, ((slot == 0) ? "Main hand" : (slot == 1) ? "Off hand" : "Ranged slot"));
    return true;
}

bool ChatHandler::HandleNPCSetOnObjectCommand(const char * args, WorldSession * m_session)
{
    Creature* crt = getSelectedCreature(m_session, false);
    if(crt == NULL)
    {
        RedSystemMessage(m_session, "Please select a creature before using this command.");
        return true;
    }

    if(!crt->IsSpawn())
    {
        RedSystemMessage(m_session, "Creature must be a valid spawn.");
        return true;
    }

    BlueSystemMessage(m_session, "Setting creature on object(%u)", crt->GetCanMove());
    sWorld.LogGM(m_session, "Set npc %s, spawn id %u on object", crt->GetName(), crt->GetLowGUID());
    return true;
}

bool ChatHandler::HandleNPCSaveCommand(const char * args, WorldSession * m_session)
{
    Creature* crt = getSelectedCreature(m_session, false);
    if(crt == NULL)
    {
        RedSystemMessage(m_session, "Please select a creature before using this command.");
        return true;
    }

    uint32 saveposition = atol(args);
    crt->SaveToDB(saveposition ? true : false);
    return true;
}

bool ChatHandler::HandleNPCSetVendorMaskCommand(const char * args, WorldSession * m_session)
{
    Creature* crt = getSelectedCreature(m_session, false);
    if(crt == NULL || !crt->IsSpawn())
    {
        RedSystemMessage(m_session, "Please select a saved creature before using this command.");
        return true;
    }

    crt->GetSpawn()->vendormask = atol(args);
    crt->SaveToDB();
    return true;
}

bool ChatHandler::HandleNPCGetSpeed(const char * args, WorldSession * m_session)
{
    Unit *unit = getSelectedUnit(m_session, false);
    if(unit == NULL)
    {
        RedSystemMessage(m_session, "Please select a unit before using this command.");
        return true;
    }

    int speedType = atoi(args);
    if(speedType == -1)
    {
        for(uint8 i = 0; i < MOVE_SPEED_MAX; i++)
            GreenSystemMessage(m_session, "Current speed %f for type %i", unit->GetMoveSpeed(MovementSpeedTypes(i)), i);
    }
    else if(speedType < MOVE_SPEED_MAX)
        GreenSystemMessage(m_session, "Current speed %f for type %i", unit->GetMoveSpeed(MovementSpeedTypes(speedType)), speedType);
    return true;
}

bool ChatHandler::HandleNPCGetResist(const char * args, WorldSession * m_session)
{
    Player *plr = m_session->GetPlayer();
    Unit *unit = getSelectedUnit(m_session, false);
    if(unit == NULL || plr == NULL || !plr->IsInWorld())
    {
        RedSystemMessage(m_session, "Please select a unit before using this command.");
        return true;
    }

    int resistType = atoi(args);
    float reduction = 0.f;
    GreenSystemMessage(m_session, "Current resistance %f for type %i", reduction, resistType);
    return true;
}

bool ChatHandler::HandleCastSpellNECommand(const char* args, WorldSession *m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = getSelectedUnit(m_session, false);
    if(!target)
        target = caster;

    uint32 spellId = atol(args);
    SpellEntry *spellentry = dbcSpell.LookupEntry(spellId);
    if(spellentry == NULL)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }
    BlueSystemMessage(m_session, "Casting spell %d on target.", spellId);

    WorldPacket data(SMSG_SPELL_GO, 50);
    data << caster->GetGUID().asPacked();
    data << caster->GetGUID().asPacked();
    data << uint8(0) << spellId;
    data << uint32(256) << uint32(0) << uint32(0);
    data << uint8(1) << target->GetGUID() << uint8(0);
    data << uint32(2) << target->GetGUID().asPacked();
    m_session->SendPacket( &data );
    return true;
}

bool ChatHandler::HandleMonsterSayCommand(const char* args, WorldSession *m_session)
{
    Unit* crt = getSelectedUnit(m_session);
    if(!crt)
        return true;

    if(crt->IsPlayer())
    {
        WorldPacket data;
        FillMessageData(&data, false, CHAT_MSG_SAY, LANG_UNIVERSAL, crt->GetGUID(), 0, crt->GetName(), args, "", 0);
        crt->SendMessageToSet(&data, true);
    } else crt->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, args);
    sWorld.LogGM(m_session, "Used npc say command on %s %s", crt->IsPlayer() ? "Player" : "Creature", crt->GetName());
    return true;
}

bool ChatHandler::HandleMonsterYellCommand(const char* args, WorldSession *m_session)
{
    Unit* crt = getSelectedUnit(m_session, false);
    if(!crt)
        return true;

    if(crt->IsPlayer())
    {
        WorldPacket data;
        FillMessageData(&data, false, CHAT_MSG_YELL, LANG_UNIVERSAL, crt->GetGUID(), 0, crt->GetName(), args, "", 0);
        crt->SendMessageToSet(&data, true);
    } else crt->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, args);
    sWorld.LogGM(m_session, "Used npc yell command on %s %s", crt->IsPlayer() ? "Player" : "Creature", crt->GetName());
    return true;
}


bool ChatHandler::HandleGOSelect(const char *args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();

    GameObject* GObj = NULL;
    float cDist = 9999.f, nDist = 0.f;
    for(WorldObject::InRangeSet::iterator itr = plr->GetInRangeGameObjectSetBegin(); itr != plr->GetInRangeGameObjectSetEnd(); itr++ )
    {
        if(GameObject *gob = plr->GetInRangeObject<GameObject>(*itr))
        {
            if( (nDist = plr->GetDistanceSq(gob)) < cDist )
            {
                GObj = gob;
                cDist = nDist;
            }
        }
    }

    if( GObj == NULL )
    {
        RedSystemMessage(m_session, "No inrange GameObject found.");
        return true;
    }

    plr->m_selectedGo = GObj->GetGUID();
    GreenSystemMessage(m_session, "Selected GameObject [%u][%s] which is %.3f meters away from you.", GObj->GetEntry(), GameObjectNameStorage.LookupEntry(GObj->GetEntry())->Name, cDist);
    return true;
}

bool ChatHandler::HandleGODelete(const char *args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    // Call a clean to selected gameobject even if it's not inrange
    m_session->GetPlayer()->m_selectedGo.Clean();
    if( GObj == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    bool foundonmap = true;
    if(GObj->m_spawn && GObj->IsInWorld())
    {
        uint32 cellx = GObj->GetMapInstance()->GetPosX(GObj->m_spawn->x);
        uint32 celly = GObj->GetMapInstance()->GetPosY(GObj->m_spawn->y);

        if(cellx < _sizeX && celly < _sizeY)
        {
            foundonmap = false;
            GameObjectSpawnList::iterator itr;
            ASSERT(GObj->GetMapInstance()->GetBaseMap() != NULL)
            CellSpawns *c = GObj->GetMapInstance()->GetBaseMap()->GetSpawnsList(cellx, celly);
            if(c != NULL)
            {
                for(itr = c->GameObjectSpawns.begin(); itr != c->GameObjectSpawns.end(); itr++)
                {
                    if((*itr) == GObj->m_spawn)
                    {
                        foundonmap = true;
                        c->GameObjectSpawns.erase(itr);
                        break;
                    }
                }
            }
        }

        GObj->DeleteFromDB();

        if(foundonmap)
        {
            delete GObj->m_spawn;
            GObj->m_spawn = NULL;
        }
    }

    GObj->Deactivate(0); // Deleted through ExpireAndDelete
    GObj = NULL;
    if(foundonmap)
        BlueSystemMessage(m_session, "Deleted selected object and erased it from spawn map.");
    else BlueSystemMessage(m_session, "Deleted selected object.");

    return true;
}

bool ChatHandler::HandleGOSpawn(const char *args, WorldSession *m_session)
{
    uint32 entryID = 0, save = 0;
    if(sscanf(args, "%u %u", &entryID, &save) == 0)
        return false;

    if((GameObjectNameStorage.LookupEntry(entryID) == NULL) || (objmgr.SQLCheckExists("gameobject_names", "entry", entryID) == NULL))
    {
        RedSystemMessage(m_session, "Invalid Gameobject ID(%u).", entryID);
        return true;
    }

    bool Save = m_session->HasGMPermissions() && save ? true : false;
    GameObject* go = m_session->GetPlayer()->GetMapInstance()->CreateGameObject(entryID);
    if(go == NULL)
    {
        RedSystemMessage(m_session, "Spawn of Gameobject(%u) failed.", entryID);
        return true;
    }

    Player* chr = m_session->GetPlayer();
    uint32 mapid = chr->GetMapId();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();
    BlueSystemMessage(m_session, "Spawning Gameobject(%u) at current position", entryID);

    go->Load(mapid, x, y, z, o, 0.f, 0.f, 0.f, 0.f, NULL);
    go->SetInstanceID(chr->GetInstanceID());
    go->PushToWorld(chr->GetMapInstance());

    sWorld.LogGM(m_session, "Spawned gameobject %u at %f %f %f (%s)", entryID, x, y, z, Save ? "Saved" : "Not Saved");
    return true;
}

bool ChatHandler::HandleGOInfo(const char *args, WorldSession *m_session)
{
    std::stringstream sstext;
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( GObj == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    WhiteSystemMessage(m_session, "Information:");
    GreenSystemMessage(m_session, "Entry: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetEntry());
    GreenSystemMessage(m_session, "Model: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetDisplayId());
    GreenSystemMessage(m_session, "State: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetState());
    GreenSystemMessage(m_session, "flags: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetFlags());
    GreenSystemMessage(m_session, "dynflags: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetUInt32Value(GAMEOBJECT_DYNAMIC));
    GreenSystemMessage(m_session, "faction: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetUInt32Value(GAMEOBJECT_FACTION));

    std::stringstream gottext;
    uint8 type = GObj->GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_TYPE_ID);
    switch(type)
    {
    case GAMEOBJECT_TYPE_DOOR:                      gottext << "Door";  break;
    case GAMEOBJECT_TYPE_BUTTON:                    gottext << "Button";    break;
    case GAMEOBJECT_TYPE_QUESTGIVER:                gottext << "Quest Giver";   break;
    case GAMEOBJECT_TYPE_CHEST:                     gottext << "Chest"; break;
    case GAMEOBJECT_TYPE_BINDER:                    gottext << "Binder";    break;
    case GAMEOBJECT_TYPE_GENERIC:                   gottext << "Generic";   break;
    case GAMEOBJECT_TYPE_TRAP:                      gottext << "Trap";  break;
    case GAMEOBJECT_TYPE_CHAIR:                     gottext << "Chair"; break;
    case GAMEOBJECT_TYPE_SPELL_FOCUS:               gottext << "Spell Focus";   break;
    case GAMEOBJECT_TYPE_TEXT:                      gottext << "Text";  break;
    case GAMEOBJECT_TYPE_GOOBER:                    gottext << "Goober";    break;
    case GAMEOBJECT_TYPE_TRANSPORT:                 gottext << "Transport"; break;
    case GAMEOBJECT_TYPE_AREADAMAGE:                gottext << "Area Damage";   break;
    case GAMEOBJECT_TYPE_CAMERA:                    gottext << "Camera";    break;
    case GAMEOBJECT_TYPE_MAP_OBJECT:                gottext << "Map WorldObject";    break;
    case GAMEOBJECT_TYPE_MO_TRANSPORT:              gottext << "Mo Transport";  break;
    case GAMEOBJECT_TYPE_DUEL_ARBITER:              gottext << "Duel Arbiter";  break;
    case GAMEOBJECT_TYPE_FISHINGNODE:               gottext << "Fishing Node";  break;
    case GAMEOBJECT_TYPE_RITUAL:                    gottext << "Ritual";    break;
    case GAMEOBJECT_TYPE_MAILBOX:                   gottext << "Mailbox";   break;
    case GAMEOBJECT_TYPE_AUCTIONHOUSE:              gottext << "Auction House"; break;
    case GAMEOBJECT_TYPE_GUARDPOST:                 gottext << "Guard Post";    break;
    case GAMEOBJECT_TYPE_SPELLCASTER:               gottext << "Spell Caster";  break;
    case GAMEOBJECT_TYPE_MEETINGSTONE:              gottext << "Meeting Stone"; break;
    case GAMEOBJECT_TYPE_FLAGSTAND:                 gottext << "Flag Stand";    break;
    case GAMEOBJECT_TYPE_FISHINGHOLE:               gottext << "Fishing Hole";  break;
    case GAMEOBJECT_TYPE_FLAGDROP:                  gottext << "Flag Drop"; break;
    case GAMEOBJECT_TYPE_MINI_GAME:                 gottext << "Mini Game"; break;
    case GAMEOBJECT_TYPE_LOTTERY_KIOSK:             gottext << "Lottery KIOSK"; break;
    case GAMEOBJECT_TYPE_CAPTURE_POINT:             gottext << "Capture Point"; break;
    case GAMEOBJECT_TYPE_AURA_GENERATOR:            gottext << "Aura Generator"; break;
    case GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY:        gottext << "Dungeon Difficulty"; break;
    case GAMEOBJECT_TYPE_BARBER_CHAIR:              gottext << "Barber Chair"; break;
    case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:     gottext << "Destructible Building"; break;
    case GAMEOBJECT_TYPE_GUILD_BANK:                gottext << "Guild Bank"; break;
    case GAMEOBJECT_TYPE_TRAPDOOR:                  gottext << "Trap Door"; break;
    default:                                        gottext << "Unknown.";  break;
    }
    GreenSystemMessage(m_session, "Type: %s%u|r -- %s", MSG_COLOR_LIGHTBLUE, type, gottext.str().c_str());
    GreenSystemMessage(m_session, "Distance: %s%f|r", MSG_COLOR_LIGHTBLUE, sqrtf(GObj->GetDistanceSq(m_session->GetPlayer())));
    GreenSystemMessage(m_session, "Size: %s%f|r", MSG_COLOR_LIGHTBLUE, GObj->GetFloatValue(OBJECT_FIELD_SCALE_X));
    if(GObj->GetInfo())
        GreenSystemMessage(m_session, "Name: %s%s|r", MSG_COLOR_LIGHTBLUE, GObj->GetInfo()->Name);
    GreenSystemMessage(m_session, "Phase: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetPhaseMask());
    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleGOEnable(const char *args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( GObj == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    if(GObj->GetUInt32Value(GAMEOBJECT_DYNAMIC) == 1)
    {
        // Deactivate
        GObj->SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
    } else {
        // /Activate
        GObj->SetUInt32Value(GAMEOBJECT_DYNAMIC, 1);
    }
    BlueSystemMessage(m_session, "Gameobject activate/deactivated.");
    return true;
}

bool ChatHandler::HandleGOActivate(const char* args, WorldSession *m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( GObj == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    if(GObj->GetState() == 1)
    {
        // Close/Deactivate
        GObj->SetState(0);
        GObj->SetFlags(GObj->GetFlags()-1);
    } 
    else 
    {
        // Open/Activate
        GObj->SetState(1);
        GObj->SetFlags(GObj->GetFlags()+1);
    }
    BlueSystemMessage(m_session, "Gameobject opened/closed.");
    return true;
}

bool ChatHandler::HandleGOScale(const char* args, WorldSession* m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *go = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( go == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    MapInstance* mgr = go->GetMapInstance();
    if(mgr == NULL)
    {
        RedSystemMessage(m_session, "Incorrectly selected Gameobject...");
        return true;
    }

    float scale = (float)atof(args);
    if(scale <= 0.f)
        scale = 1.f; // Scale defaults to 1 on GO's, so its basically a reset.
    if(scale > 255.f)
        scale = 255.f;
    BlueSystemMessage(m_session, "Set scale to %.3f", scale);

    go->WorldObject::RemoveFromWorld();
    go->SetFloatValue(OBJECT_FIELD_SCALE_X, scale);
    go->SaveToDB();
    sWorldMgr.PushToWorldQueue(go);
    sWorld.LogGM(m_session, "Scaled gameobject spawn id %u to %f", go->GetLowGUID(), scale);
    return true;
}

bool ChatHandler::HandleReviveStringcommand(const char* args, WorldSession* m_session)
{
    Player* plr = objmgr.GetPlayer(args, false);
    if(!plr)
    {
        RedSystemMessage(m_session, "Could not find player %s.", args);
        return true;
    }

    if(plr->isDead())
    {
        if(plr->GetInstanceID() == m_session->GetPlayer()->GetInstanceID())
            plr->RemoteRevive();

        GreenSystemMessage(m_session, "Revived player %s.", args);
    } 
    else 
    {
        GreenSystemMessage(m_session, "Player %s is not dead.", args);
    }
    sWorld.LogGM(m_session, "Remote revived %s", args);
    return true;
}

bool ChatHandler::HandleMountCommand(const char *args, WorldSession *m_session)
{
    if(!args)
    {
        RedSystemMessage(m_session, "No model specified");
        return true;
    }
    uint32 modelid = atol(args);
    if(!modelid)
    {
        RedSystemMessage(m_session, "No model specified");
        return true;
    }

    Unit* m_target = NULL;
    if(Player* m_plyr = getSelectedChar(m_session, false))
        m_target = m_plyr;
    else if(Creature* m_crt = getSelectedCreature(m_session, false))
        m_target = m_crt;

    if(m_target == NULL)
    {
        RedSystemMessage(m_session, "No target found.");
        return true;
    }

    if(m_target->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) != 0)
    {
        RedSystemMessage(m_session, "Target is already mounted.");
        return true;
    }

    m_target->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , modelid);
    //m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);

    BlueSystemMessage(m_session, "Now mounted with model %d.", modelid);
    return true;
}

bool ChatHandler::HandleGOAnimProgress(const char * args, WorldSession * m_session)
{
    Player *plr = m_session->GetPlayer();
    GameObject *GObj = plr->GetInRangeObject<GameObject>(plr->m_selectedGo);
    if( GObj == NULL )
    {
        RedSystemMessage(m_session, "%s GameObject selected...", plr->m_selectedGo.empty() ? "No" : "Invalid");
        return true;
    }

    GObj->SetAnimProgress(atol(args));
    BlueSystemMessage(m_session, "Set ANIMPROGRESS to %u", GObj->GetAnimProgress());
    return true;
}

bool ChatHandler::HandleNpcComeCommand(const char* args, WorldSession* m_session)
{
    Creature* unit = getSelectedCreature(m_session, false);
    if(!unit)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    unit->GetAIInterface()->m_pendingWaitTimer = 30000;
    unit->MoveTo(m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ(), m_session->GetPlayer()->GetOrientation());
    return true;
}

bool ChatHandler::HandleNpcJumpCommand(const char* args, WorldSession* m_session)
{

    return true;
}

bool ChatHandler::HandleVendorClearCommand(const char* args, WorldSession *m_session)
{
    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    if(guid.empty())
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapInstance()->GetCreature(guid);
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    if(!pCreature->IsSpawn())
    {
        SystemMessage(m_session, "You should select a saved creature.");
        return true;
    }

    bool first = true;
    QueryBuffer* qb = new QueryBuffer();
    std::vector<AvailableCreatureItem>::iterator itr, itr2, begin = pCreature->GetSellItemBegin(), end = pCreature->GetSellItemEnd();
    for(itr = begin; itr != end;)
    {
        itr2 = itr++;
        first = false;
        qb->AddQuery("DELETE FROM vendors WHERE entry = '%u' AND vendormask = '%i' AND item = '%u';", pCreature->GetEntry(), pCreature->GetVendorMask(), itr2->proto->ItemId);
    }

    if(!first)
    {
        WorldDatabase.AddQueryBuffer(qb);
        SystemMessage(m_session, "removed all items with vendor mask %u", pCreature->GetVendorMask());
        sWorld.LogGM(m_session, "removed all items with vendor mask %u from vendor %u", pCreature->GetVendorMask(), pCreature->GetEntry());
    }
    else
        RedSystemMessage(m_session, "No items found");
    return true;
}

bool ChatHandler::HandleItemSetCommand(const char* args, WorldSession *m_session)
{
    if(strlen(args) < 1)
        return false;

    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    if(guid.empty())
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapInstance()->GetCreature(guid);
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    if(!pCreature->IsSpawn())
    {
        SystemMessage(m_session, "You should select a saved creature.");
        return true;
    }

    int32 vendormask = 0, extendedcost = 0;
    uint32 setid, rank = 1, count = 0;
    if(sscanf(args, "%u %u %i %i", &setid, &rank, &extendedcost, &vendormask) < 1)
    {
        RedSystemMessage(m_session, "You must specify a setid.");
        return true;
    }

    std::list<ItemPrototype*>* l = objmgr.GetListForItemSet(setid);
    if(l == NULL)
    {
        RedSystemMessage(m_session, "Invalid item set.");
        return true;
    }

    if(vendormask == 0)
        vendormask = pCreature->GetVendorMask();

    std::stringstream sstext;
    for(std::list<ItemPrototype*>::iterator itr = l->begin(); itr != l->end(); itr++)
    {
        if((*itr)->ItemSetRank && (*itr)->ItemSetRank != rank)
            continue;

        std::stringstream ss;
        ss << "INSERT INTO vendors(entry, item, amount, extendedcost, vendormask) VALUES ('" << pCreature->GetEntry() << "', '" << (*itr)->ItemId << "', " << extendedcost << ", " << vendormask << " );";
        WorldDatabase.Execute( ss.str().c_str() );
        pCreature->AddVendorItem((*itr)->ItemId, vendormask, extendedcost);
        count++;
    }

    if(count)
    {
        sstext << "Item set '" << setid << "' rank '" << rank << "' Added to vendor." << '\0';
        sWorld.LogGM(m_session, "added item set %u rank %u to vendor %u", setid, rank, pCreature->GetEntry());
        SystemMessage(m_session, sstext.str().c_str());
    }
    else
        RedSystemMessage(m_session, "No items found");
    return true;
}

bool ChatHandler::HandleItemSetRemoveCommand(const char* args, WorldSession *m_session)
{
    if(strlen(args) < 1)
        return false;

    WoWGuid guid = m_session->GetPlayer()->GetSelection();
    if(guid.empty())
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapInstance()->GetCreature(guid);
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 setid, rank = 1;
    if(sscanf(args, "%u %u", &setid, &rank) < 1)
    {
        RedSystemMessage(m_session, "You must specify a setid.");
        return true;
    }

    std::list<ItemPrototype*>* l = objmgr.GetListForItemSet(setid);
    if(l == NULL)
    {
        RedSystemMessage(m_session, "Invalid item set.");
        return true;
    }

    QueryBuffer* qb = new QueryBuffer();
    bool first = true;
    for(std::list<ItemPrototype*>::iterator itr = l->begin(); itr != l->end(); itr++)
    {
        if((*itr)->ItemSetRank && (*itr)->ItemSetRank != rank)
            continue;

        first = false;
        qb->AddQuery("DELETE FROM vendors WHERE entry = '%u' AND vendormask = '%i' AND item = '%u';", pCreature->GetEntry(), pCreature->GetVendorMask(), (*itr)->ItemId);
    }

    if(!first)
    {
        WorldDatabase.AddQueryBuffer(qb);
        SystemMessage(m_session, "removed item set %u rank %u", setid, rank);
        sWorld.LogGM(m_session, "removed item set %u rank %u from vendor %u", setid, rank, pCreature->GetEntry());
    }
    else
        RedSystemMessage(m_session, "No items found");

    return true;
}
