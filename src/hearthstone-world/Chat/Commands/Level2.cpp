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

    plr->_InitialReputation();
    SystemMessage(m_session, "Done. Relog for changes to take effect.");
    sWorld.LogGM(m_session, "used reset reputation for %s", plr->GetName());
    return true;
}

bool ChatHandler::HandleInvincibleCommand(const char *args, WorldSession *m_session)
{
    Player* chr = m_session->GetPlayer();
    chr->bInvincible = !chr->bInvincible;
    SystemMessage(m_session, "Invincibility is now %s", chr->bInvincible ? "ON." : "OFF.");
    return true;
}

bool ChatHandler::HandleInvisibleCommand(const char *args, WorldSession *m_session)
{
    Player* pChar = m_session->GetPlayer();
    if(pChar->m_isGmInvisible || pChar->m_invisible)
    {
        pChar->m_isGmInvisible = false;
        pChar->m_invisible = false;
    } 
    else 
    {
        pChar->m_isGmInvisible = true;
        pChar->m_invisible = true;
    }

    GreenSystemMessage(m_session, "Invisibility is now %s", pChar->m_invisible ? "ON." : "OFF.");
    return true;
}

bool ChatHandler::CreateGuildCommand(const char* args, WorldSession *m_session)
{
    if(!*args)
        return false;

    Player* ptarget = getSelectedChar(m_session);
    if(ptarget == NULL)
        return true;

    if(strlen((char*)args)>75)
    {
        // send message to user
        char buf[256];
        snprintf((char*)buf,256,"The name was too long by %i", (unsigned int)strlen((char*)args)-75);
        SystemMessage(m_session, buf);
        return true;
    }

    for (uint32 i = 0; i < strlen(args); i++)
    {
        if(!isalpha(args[i]) && args[i]!=' ')
        {
            SystemMessage(m_session, "Error, name can only contain chars A-Z and a-z.");
            return true;
        }
    }

    GuildInfo* pGuild = guildmgr.GetGuildByGuildName(string(args));
    if(pGuild)
    {
        RedSystemMessage(m_session, "Guild name is already taken.");
        return true;
    }

    guildmgr.CreateGuildFromCommand(string(args), ptarget->GetLowGUID());
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
    else if(unit->IsPet() || unit->IsSummon())
    {
        SystemMessage(m_session, "You can't delete playerpets.");
        return true;
    }

    if( unit->m_spawn != NULL && !m_session->CanUseCommand('z') )
    {
        SystemMessage(m_session, "You do not have permission to do that. Please contact higher staff for removing of saved spawns.");
        return true;
    }

    if(unit->IsVehicle())
    {
        Vehicle* veh = TO_VEHICLE(unit);
        for(int i = 0; i < 8; i++)
        {
            if(!veh->GetPassenger(i))
                continue;

            // Remove any players
            if(veh->GetPassenger(i)->IsPlayer())
                veh->RemovePassenger(veh->GetPassenger(i));
            else // Remove any units.
                veh->GetPassenger(i)->RemoveFromWorld(true);
        }
    }

    sWorld.LogGM(m_session, "used npc delete, sqlid %u, creature %s, pos %f %f %f",
        unit->m_spawn ? unit->m_spawn->id : 0, unit->GetCreatureInfo() ? unit->GetCreatureInfo()->Name : "wtfbbqhax", unit->GetPositionX(), unit->GetPositionY(),
        unit->GetPositionZ());

    BlueSystemMessage(m_session, "Deleted creature ID %u", unit->spawnid);

    unit->DeleteFromDB();

    if(!unit->IsInWorld())
        return true;

    MapMgr* unitMgr = unit->GetMapMgr();
    if(unit->m_spawn)
    {
        uint32 cellx = unitMgr->GetPosX(unit->m_spawn->x);
        uint32 celly = unitMgr->GetPosX(unit->m_spawn->y);
        if(cellx <= _sizeX && celly <= _sizeY )
        {
            CellSpawns *c = unitMgr->GetBaseMap()->GetSpawnsList(cellx, celly);
            if( c != NULL )
            {
                CreatureSpawnList::iterator itr, itr2;
                for(itr = c->CreatureSpawns.begin(); itr != c->CreatureSpawns.end();)
                {
                    itr2 = itr++;
                    if((*itr2) == unit->m_spawn)
                    {
                        c->CreatureSpawns.erase(itr2);
                        delete unit->m_spawn;
                        break;
                    }
                }
            }
        }
    }
    unit->RemoveFromWorld(false, true);

    if(unit->IsVehicle())
        TO_VEHICLE(unit)->Destruct();
    else
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
    if(!pCreature || !pCreature->m_spawn || !(pCreature->HasNpcFlag(UNIT_NPC_FLAG_VENDOR) || pCreature->HasNpcFlag(UNIT_NPC_FLAG_ARMORER)))
    {
        SystemMessage(m_session, "You should select a vendor.");
        return true;
    }

    int amount = 1;
    uint32 item = 0, extendedcost = 0, vendormask = 0;
    if(sscanf(args, "%u %u %u %u", &item, &amount, &extendedcost, &vendormask) < 1)
    {
        // check for item link
        GetItemIDFromLink(args, &item);
        if(item == 0)
            return false;
    }
    if(item == 0)
        return false;

    if(vendormask == 0)
        vendormask = pCreature->m_spawn->vendormask;

    ItemPrototype* tmpItem = ItemPrototypeStorage.LookupEntry(item);
    std::stringstream sstext;
    if(tmpItem)
    {
        std::stringstream ss;
        ss << "INSERT INTO vendors(entry, item, amount, extendedcost, vendormask) VALUES ('" << pCreature->GetEntry() << "', '" << item << "', '" << amount << "', " << extendedcost << ", " << vendormask << " );";
        WorldDatabase.Execute( ss.str().c_str() );

        pCreature->AddVendorItem(item, amount, vendormask, extendedcost);

        sstext << "Item '" << item << "' '" << tmpItem->Name1 << "' Added to list" << '\0';
    }
    else
        sstext << "Item '" << item << "' Not Found in Database." << '\0';

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
    int slot = pCreature->GetSlotByItemId(itemguid);
    if(slot != -1)
    {
        std::stringstream ss;
        ss << "DELETE FROM vendors WHERE entry = '" << pCreature->GetEntry() << "' AND vendormask = '" << pCreature->VendorMask << "' AND item = " << itemguid << " LIMIT 1;";
        WorldDatabase.Execute( ss.str().c_str() );

        pCreature->RemoveVendorItem(itemguid);
        ItemPrototype* tmpItem = ItemPrototypeStorage.LookupEntry(itemguid);
        if(tmpItem)
            sstext << "Item '" << itemguid << "' '" << tmpItem->Name1 << "' Deleted from list" << '\0';
        else
            sstext << "Item '" << itemguid << "' Deleted from list" << '\0';
    }
    else
    {
        sstext << "Item '" << itemguid << "' Not Found in List." << '\0';
    }

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
    PlayerStorageMap::const_iterator itr;
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
        BlueSystemMessageToPlr(TO_PLAYER(unit), "%s killed you with a GM command.", m_session->GetPlayer()->GetName());
        TO_PLAYER(unit)->SetUInt32Value(UNIT_FIELD_HEALTH, 0); // Die, insect
        TO_PLAYER(unit)->KillPlayer();
        GreenSystemMessage(m_session, "Killed player %s.", unit->GetName());
    }
    else if(isTargetDummy(unit->GetEntry()))
        RedSystemMessage(m_session, "Target cannot be killed.");
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

    Spell* sp = new Spell(caster, spellentry, false, NULLAURA);
    if(!sp)
    {
        RedSystemMessage(m_session, "Spell failed creation!");
        return false;
    }
    BlueSystemMessage(m_session, "Casting spell %d on target.", spellid);
    SpellCastTargets targets;
    targets.m_unitTarget = target->GetGUID();
    sp->prepare(&targets);
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
    if(crt->m_spawn)
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

    if(crt->m_spawn == NULL)
    {
        RedSystemMessage(m_session, "Creature must be a valid spawn.");
        return true;
    }

    crt->m_spawn->CanMove |= LIMIT_ON_OBJ;
    crt->SaveToDB();

    BlueSystemMessage(m_session, "Setting creature on Object(%u)", crt->GetCanMove());
    sWorld.LogGM(m_session, "Set npc %s, spawn id %u on object", crt->GetName(), crt->m_spawn->id);
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
    if(crt == NULL)
    {
        RedSystemMessage(m_session, "Please select a creature before using this command.");
        return true;
    }

    crt->VendorMask = atol(args);
    crt->SaveToDB();
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

    WorldPacket data;

    data.Initialize( SMSG_SPELL_START );
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0);
    data << uint16(0);
    data << uint32(0);
    data << uint16(2);
    data << target->GetGUID();
    m_session->SendPacket( &data );
    data.clear();

    data.Initialize( SMSG_SPELL_GO );
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0) << uint8(1) << uint8(1);
    data << target->GetGUID();
    data << uint8(0);
    data << uint16(2);
    data << target->GetGUID();
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
        WorldPacket * data = FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, args, crt->GetGUID(), 0);
        crt->SendMessageToSet(data, true);
        delete data;
    }
    else
        crt->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, args);
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
        WorldPacket * data = FillMessageData(CHAT_MSG_YELL, LANG_UNIVERSAL, args, crt->GetGUID(), 0);
        crt->SendMessageToSet(data, true);
        delete data;
    }
    else
        crt->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, args);
    sWorld.LogGM(m_session, "Used npc yell command on %s %s", crt->IsPlayer() ? "Player" : "Creature", crt->GetName());
    return true;
}


bool ChatHandler::HandleGOSelect(const char *args, WorldSession *m_session)
{
    GameObject* GObj = NULLGOB;

    unordered_set<Object* >::iterator Itr = m_session->GetPlayer()->GetInRangeSetBegin();
    unordered_set<Object* >::iterator Itr2 = m_session->GetPlayer()->GetInRangeSetEnd();
    float cDist = 9999.0f;
    float nDist = 0.0f;
    bool bUseNext = false;

    if(args)
    {
        if(args[0] == '1')
        {
            if(m_session->GetPlayer()->m_GM_SelectedGO == NULL)
                bUseNext = true;

            for(;;Itr++)
            {
                if(Itr == Itr2 && GObj == NULL && bUseNext)
                    Itr = m_session->GetPlayer()->GetInRangeSetBegin();
                else if(Itr == Itr2)
                    break;

                if((*Itr)->GetTypeId() == TYPEID_GAMEOBJECT)
                {
                    // Find the current go, move to the next one
                    if(bUseNext)
                    {
                        // Select the first.
                        GObj = TO_GAMEOBJECT(*Itr);
                        break;
                    }
                    else
                    {
                        if(((*Itr) == m_session->GetPlayer()->m_GM_SelectedGO))
                        {
                            // Found him. Move to the next one, or beginning if we're at the end
                            bUseNext = true;
                        }
                    }
                }
            }
        }
    }

    if(!GObj)
    {
        for( ; Itr != Itr2; Itr++ )
        {
            if( (*Itr)->GetTypeId() == TYPEID_GAMEOBJECT )
            {
                if( (nDist = m_session->GetPlayer()->CalcDistance( *Itr )) < cDist )
                {
                    cDist = nDist;
                    nDist = 0.0f;
                    GObj = TO_GAMEOBJECT(*Itr);
                }
            }
        }
    }

    if( GObj == NULL )
    {
        m_session->GetPlayer()->m_GM_SelectedGO = NULL;
        RedSystemMessage(m_session, "No inrange GameObject found.");
        return true;
    }

    m_session->GetPlayer()->m_GM_SelectedGO = GObj;

    GreenSystemMessage(m_session, "Selected GameObject [ %s ] which is %.3f meters away from you.",
        GameObjectNameStorage.LookupEntry(GObj->GetEntry())->Name, m_session->GetPlayer()->CalcDistance(GObj));

    return true;
}

bool ChatHandler::HandleGODelete(const char *args, WorldSession *m_session)
{
    GameObject* GObj = m_session->GetPlayer()->m_GM_SelectedGO;
    if( !GObj )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    bool foundonmap = true;
    if(GObj->m_spawn && GObj->m_spawn->entry == GObj->GetEntry() && GObj->IsInWorld())
    {
        uint32 cellx = GObj->GetMapMgr()->GetPosX(GObj->m_spawn->x);
        uint32 celly = GObj->GetMapMgr()->GetPosY(GObj->m_spawn->y);

        if(cellx < _sizeX && celly < _sizeY)
        {
            foundonmap = false;
            GOSpawnList::iterator itr;
            ASSERT(GObj->GetMapMgr()->GetBaseMap() != NULL)
            CellSpawns *c = GObj->GetMapMgr()->GetBaseMap()->GetSpawnsList(cellx, celly);
            if(c != NULL)
            {
                for(itr = c->GOSpawns.begin(); itr != c->GOSpawns.end(); itr++)
                {
                    if((*itr) == GObj->m_spawn)
                    {
                        foundonmap = true;
                        c->GOSpawns.erase(itr);
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
    GObj->Despawn(0, 0); // Deleted through ExpireAndDelete
    GObj = NULLGOB;
    if(foundonmap)
        BlueSystemMessage(m_session, "Deleted selected object and erased it from spawn map.");
    else
        BlueSystemMessage(m_session, "Deleted selected object.");

    m_session->GetPlayer()->m_GM_SelectedGO = NULLGOB;
    return true;
}

bool ChatHandler::HandleGOSpawn(const char *args, WorldSession *m_session)
{
    if(!args)
        return false;

    char* pEntryID = strtok((char*)args, " ");
    if (!pEntryID)
        return false;

    uint32 EntryID = atoi(pEntryID);
    if((GameObjectNameStorage.LookupEntry(EntryID) == NULL) || (objmgr.SQLCheckExists("gameobject_names", "entry", EntryID) == NULL))
    {
        RedSystemMessage(m_session, "Invalid Gameobject ID(%u).", EntryID);
        return true;
    }

    bool Save = m_session->HasGMPermissions() ? true : false;
    char* pSave = strtok(NULL, " ");
    if(pSave)
        Save = (atoi(pSave) > 0 ? true : false);

    GameObject* go = m_session->GetPlayer()->GetMapMgr()->CreateGameObject(EntryID);
    if(go == NULL)
    {
        RedSystemMessage(m_session, "Spawn of Gameobject(%u) failed.", EntryID);
        return true;
    }
    go->Init();

    Player* chr = m_session->GetPlayer();
    uint32 mapid = chr->GetMapId();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();
    go->CreateFromProto(EntryID,mapid,x,y,z,o);
    BlueSystemMessage(m_session, "Spawning Gameobject(%u) at current position", EntryID);

    if(Save == true) // If we're saving, create template and add index
    {
        // Create spawn instance
        GOSpawn *gs = new GOSpawn;
        gs->entry = go->GetEntry();
        gs->facing = go->GetOrientation();
        gs->faction = go->GetUInt32Value(GAMEOBJECT_FACTION);
        gs->flags = go->GetUInt32Value(GAMEOBJECT_FLAGS);
        gs->id = objmgr.GenerateGameObjectSpawnID();
        gs->scale = go->GetFloatValue(OBJECT_FIELD_SCALE_X);
        gs->x = x;
        gs->y = y;
        gs->z = z;
        gs->state = go->GetByte(GAMEOBJECT_BYTES_1, GAMEOBJECT_BYTES_STATE);
        gs->phase = chr->GetPhaseMask();
        go->Load(gs);
        go->SaveToDB();
        uint32 cx = chr->GetMapMgr()->GetPosX(x);
        uint32 cy = chr->GetMapMgr()->GetPosY(y);
        chr->GetMapMgr()->AddGoSpawn(cx, cy, gs);
    }

    go->SetPhaseMask(chr->GetPhaseMask());
    go->SetInstanceID(chr->GetInstanceID());
    go->PushToWorld(m_session->GetPlayer()->GetMapMgr());

    sWorld.LogGM(m_session, "Spawned gameobject %u at %f %f %f (%s)", EntryID, x, y, z, Save ? "Saved" : "Not Saved");
    return true;
}

bool ChatHandler::HandleGOInfo(const char *args, WorldSession *m_session)
{
    std::stringstream sstext;
    GameObject *GObj = NULL;

    GObj = m_session->GetPlayer()->m_GM_SelectedGO;
    if( !GObj )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    WhiteSystemMessage(m_session, "Information:");
    if(GObj->m_spawn)
        GreenSystemMessage(m_session, "SpawnID: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->m_spawn->id);
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
    case GAMEOBJECT_TYPE_MAP_OBJECT:                gottext << "Map Object";    break;
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
    GreenSystemMessage(m_session, "Distance: %s%f|r", MSG_COLOR_LIGHTBLUE, GObj->CalcDistance((Object*)m_session->GetPlayer()));
    GreenSystemMessage(m_session, "Size: %s%f|r", MSG_COLOR_LIGHTBLUE, GObj->GetFloatValue(OBJECT_FIELD_SCALE_X));
    if(GObj->GetInfo())
        GreenSystemMessage(m_session, "Name: %s%s|r", MSG_COLOR_LIGHTBLUE, GObj->GetInfo()->Name);
    GreenSystemMessage(m_session, "Phase: %s%u|r", MSG_COLOR_LIGHTBLUE, GObj->GetPhaseMask());
    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleGOEnable(const char *args, WorldSession *m_session)
{
    GameObject* GObj = NULLGOB;

    GObj = m_session->GetPlayer()->m_GM_SelectedGO;
    if( !GObj )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
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
    GameObject* GObj = NULLGOB;

    GObj = m_session->GetPlayer()->m_GM_SelectedGO;
    if( !GObj )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
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
    GameObject* go = m_session->GetPlayer()->m_GM_SelectedGO;
    if( go == NULL )
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }
    MapMgr* mgr = go->GetMapMgr();
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

    go->Object::RemoveFromWorld(false);
    go->SetFloatValue(OBJECT_FIELD_SCALE_X, scale);
    go->SaveToDB();
    go->AddToWorld();
    sEventMgr.AddEvent(mgr, &MapMgr::EventPushObjectToSelf, (Object*)go, EVENT_MAPMGR_PUSH_TO_SELF, 3000, 1, EVENT_FLAG_NONE);
    sWorld.LogGM(m_session, "Scaled gameobject spawn id %u to %f", go->m_spawn ? go->m_spawn->id : 0, scale);
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
        else
            sEventMgr.AddEvent(plr, &Player::RemoteRevive, true, EVENT_PLAYER_REST, 1, 1,0);

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

    Unit* m_target = NULLUNIT;
    Player* m_plyr = getSelectedChar(m_session, false);
    if(m_plyr)
        m_target = m_plyr;
    else
    {
        Creature* m_crt = getSelectedCreature(m_session, false);
        if(m_crt)
            m_target = m_crt;
    }

    if(!m_target)
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
    if(!m_session->GetPlayer()->m_GM_SelectedGO)
        return false;

    uint32 ap = atol(args);
    m_session->GetPlayer()->m_GM_SelectedGO->SetAnimProgress(ap);
    BlueSystemMessage(m_session, "Set ANIMPROGRESS to %u", ap);
    return true;
}

bool ChatHandler::HandleNpcComeCommand(const char* args, WorldSession* m_session)
{
    // moves npc to players location
    Player* plr = m_session->GetPlayer();
    Creature* crt = getSelectedCreature(m_session, true);
    if(!crt)
        return true;

    crt->GetAIInterface()->MoveTo(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ());
    return true;
}

bool ChatHandler::HandleNpcJumpCommand(const char* args, WorldSession* m_session)
{
    // moves npc to players location
    Player* plr = m_session->GetPlayer();
    Creature* crt = getSelectedCreature(m_session, true);
    if(!crt)
        return false;

    crt->GetAIInterface()->JumpToTargetLocation();
    crt->GetAIInterface()->MoveTo(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ());
    return true;
}

bool ChatHandler::HandleVendorClearCommand(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if(guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    if(!pCreature->m_spawn)
    {
        SystemMessage(m_session, "You should select a saved creature.");
        return true;
    }

    bool first = true;
    QueryBuffer* qb = new QueryBuffer();
    std::map<uint32, CreatureItem>::iterator itr, itr2, begin = pCreature->GetSellItemBegin(), end = pCreature->GetSellItemEnd();
    for(itr = begin; itr != end;)
    {
        itr2 = itr++;
        if(itr2->second.vendormask < 0 || pCreature->VendorMask < 0 || itr2->second.vendormask == pCreature->VendorMask)
        {
            first = false;
            qb->AddQuery("DELETE FROM vendors WHERE entry = '%u' AND vendormask = '%i' AND item = '%u';", pCreature->GetEntry(), pCreature->VendorMask, itr2->second.itemid);
            pCreature->RemoveSellItem(itr2);
        }
    }

    if(!first)
    {
        WorldDatabase.AddQueryBuffer(qb);
        SystemMessage(m_session, "removed all items with vendor mask %u", pCreature->VendorMask);
        sWorld.LogGM(m_session, "removed all items with vendor mask %u from vendor %u", pCreature->VendorMask, pCreature->GetEntry());
    }
    else
        RedSystemMessage(m_session, "No items found");
    return true;
}

bool ChatHandler::HandleItemSetCommand(const char* args, WorldSession *m_session)
{
    if(strlen(args) < 1)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if(guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    if(!pCreature->m_spawn)
    {
        SystemMessage(m_session, "You should select a saved creature.");
        return true;
    }

    int32 vendormask = 0, extendedcost = 0;
    uint32 setid, rank = 1, amount = 1, count = 0;
    if(sscanf(args, "%u %u %u %i %i", &setid, &rank, &amount, &extendedcost, &vendormask) < 1)
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
        vendormask = pCreature->m_spawn->vendormask;

    std::stringstream sstext;
    for(std::list<ItemPrototype*>::iterator itr = l->begin(); itr != l->end(); itr++)
    {
        if((*itr)->ItemSetRank && (*itr)->ItemSetRank != rank)
            continue;

        std::stringstream ss;
        ss << "INSERT INTO vendors(entry, item, amount, extendedcost, vendormask) VALUES ('" << pCreature->GetEntry() << "', '" << (*itr)->ItemId << "', '" << amount << "', " << extendedcost << ", " << vendormask << " );";
        WorldDatabase.Execute( ss.str().c_str() );
        pCreature->AddVendorItem((*itr)->ItemId, amount, vendormask, extendedcost);
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

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if(guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
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
        qb->AddQuery("DELETE FROM vendors WHERE entry = '%u' AND vendormask = '%i' AND item = '%u';", pCreature->GetEntry(), pCreature->VendorMask, (*itr)->ItemId);
        pCreature->RemoveVendorItem((*itr)->ItemId);
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

bool ChatHandler::HandleTrainerAddLearnSpell(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if(guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    Trainer* trainer = pCreature->GetTrainer();
    if(!trainer)
    {
        SystemMessage(m_session, "You should select a trainer.");
        return true;
    }

    uint32 spellid = 0, spellcost = 0, requiredspell = 0, requiredskill = 0, requiredskillvalue = 0, requiredlevel = 0, deletespell = 0, isprofession = 0;
    if(sscanf(args, "%u %u %u %u %u %u %u %u", &spellid, &spellcost, &requiredspell, &requiredskill, &requiredskillvalue, &requiredlevel, &deletespell, &isprofession) == 0)
        return false;

    SpellEntry* spellinfo = dbcSpell.LookupEntry(spellid);
    if(spellinfo == NULL)
        return false;

    TrainerSpell ts;
    ts.pCastSpell = NULL;
    ts.pCastRealSpell = NULL;
    ts.pLearnSpell = spellinfo;
    ts.Cost = spellcost;
    ts.RequiredSpell = requiredspell;
    ts.RequiredSkillLine = requiredskill;
    ts.RequiredSkillLineValue = requiredskillvalue;
    ts.RequiredLevel = requiredlevel;
    ts.DeleteSpell = deletespell;
    ts.IsProfession = (isprofession > 0);
    trainer->Spells.push_back(ts);
    WorldDatabase.Execute("INSERT INTO trainer_spells VALUES('%u', '0', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')", pCreature->GetEntry(), spellid, spellcost, requiredspell, requiredskill, requiredskillvalue, requiredlevel, deletespell, isprofession);
    SystemMessage(m_session, "Learn spell added successfully.");
    return true;
}

bool ChatHandler::HandleTrainerAddCastSpell(const char* args, WorldSession *m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if(guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return true;
    }

    Creature* pCreature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    Trainer* trainer = pCreature->GetTrainer();
    if(!trainer)
    {
        SystemMessage(m_session, "You should select a trainer.");
        return true;
    }

    uint32 spellid = 0, spellcost = 0, requiredspell = 0, requiredskill = 0, requiredskillvalue = 0, requiredlevel = 0, deletespell = 0, isprofession = 0;
    if(sscanf(args, "%u %u %u %u %u %u %u %u", &spellid, &spellcost, &requiredspell, &requiredskill, &requiredskillvalue, &requiredlevel, &deletespell, &isprofession) == 0)
        return false;

    SpellEntry* spellinfo = dbcSpell.LookupEntry(spellid);
    if(spellinfo == NULL)
        return false;

    TrainerSpell ts;
    ts.pCastRealSpell = NULL;
    ts.pCastSpell = spellinfo;
    for( int k = 0; k < 3; ++k )
    {
        if( spellinfo->Effect[k] == SPELL_EFFECT_LEARN_SPELL )
        {
            ts.pCastRealSpell = dbcSpell.LookupEntry(spellinfo->EffectTriggerSpell[k]);
            break;
        }
    }

    ts.pLearnSpell = NULL;
    ts.Cost = spellcost;
    ts.RequiredSpell = requiredspell;
    ts.RequiredSkillLine = requiredskill;
    ts.RequiredSkillLineValue = requiredskillvalue;
    ts.RequiredLevel = requiredlevel;
    ts.DeleteSpell = deletespell;
    ts.IsProfession = (isprofession > 0);
    trainer->Spells.push_back(ts);
    WorldDatabase.Execute("INSERT INTO trainer_spells VALUES('%u', '%u', '0', '%u', '%u', '%u', '%u', '%u', '%u', '%u')", pCreature->GetEntry(), spellid, spellcost, requiredspell, requiredskill, requiredskillvalue, requiredlevel, deletespell, isprofession);
    SystemMessage(m_session, "Cast spell added successfully.");
    return true;
}
