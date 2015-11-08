/***
 * Demonstrike Core
 */

#include "StdAfx.h"

trainertype trainer_types[TRAINER_TYPE_MAX] =
{
{   "Warrior",             0 },
{   "Paladin",             0 },
{   "Rogue"  ,             0 },
{   "Warlock",             0 },
{   "Mage",                0 },
{   "Shaman",              0 },
{   "Priest",              0 },
{   "Hunter",              0 },
{   "Druid",               0 },
{   "Leatherwork",         2 },
{   "Skinning",            2 },
{   "Fishing",             2 },
{   "First Aid",           2 },
{   "Physician",           2 },
{   "Engineer",            2 },
{   "Weapon Master",       0 },
};

bool CanTrainAt(Player* plr, Trainer * trn)
{
    if ( (trn->RequiredClass && plr->getClass() != trn->RequiredClass) ||
         (trn->RequiredSkill && !plr->_HasSkillLine(trn->RequiredSkill)) ||
         (trn->RequiredSkillLine && plr->_GetSkillLineCurrent(trn->RequiredSkill) < trn->RequiredSkillLine) )
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////
/// This function handles MSG_TABARDVENDOR_ACTIVATE:
//////////////////////////////////////////////////////////////
void WorldSession::HandleTabardVendorActivateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
    Creature* pCreature = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature) return;

    SendTabardHelp(pCreature);
}

void WorldSession::SendTabardHelp(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data(8);
    data.Initialize( MSG_TABARDVENDOR_ACTIVATE );
    data << pCreature->GetGUID();
    SendPacket( &data );
}


//////////////////////////////////////////////////////////////
/// This function handles CMSG_BANKER_ACTIVATE:
//////////////////////////////////////////////////////////////
void WorldSession::HandleBankerActivateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature) return;

    SendBankerList(pCreature);
}

void WorldSession::SendBankerList(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data(8);
    data.Initialize( SMSG_SHOW_BANK );
    data << pCreature->GetGUID();
    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_TRAINER_LIST
//////////////////////////////////////////////////////////////
//NOTE: we select prerequirements for spell that TEACHES you
//not by spell that you learn!
void WorldSession::HandleTrainerListOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    // Inits, grab creature, check.
    uint64 guid;
    recv_data >> guid;
    Creature* train = GetPlayer()->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(train == NULL)
        return;

    if(FactionEntry *faction = train->GetFaction())
        _player->Reputation_OnTalk(faction);
    SendTrainerList(train);
}

void WorldSession::SendTrainerList(Creature* pCreature)
{
    Trainer * pTrainer = pCreature->GetTrainer();
    if(pTrainer == 0)
        return;

    if(!CanTrainAt(_player,pTrainer))
    {
        GossipMenu * pMenu;
        objmgr.CreateGossipMenuForPlayer(&pMenu,pCreature->GetGUID(), pTrainer->Cannot_Train_GossipTextId, _player);
        pMenu->SendTo(_player);
        return;
    }

    TrainerSpell* pSpell;
    uint32 Spacer = 0;
    uint32 Count=0;
    uint8 Status;
    std::string Text;

    WorldPacket data(SMSG_TRAINER_LIST, 5000);
    data << pCreature->GetGUID();
    data << pTrainer->TrainerType;
    data << uint32(0);
    for(std::vector<TrainerSpell>::iterator itr = pTrainer->Spells.begin(); itr != pTrainer->Spells.end(); itr++)
    {
        pSpell = &(*itr);
        Status = TrainerGetSpellStatus(pSpell);
        if(Status == TRAINER_STATUS_NOT_AVAILABLE)
            continue; // Don't bother sending shit.
        else if( pSpell->pCastRealSpell )
            data << pSpell->pCastRealSpell->Id;
        else if( pSpell->pLearnSpell )
            data << pSpell->pLearnSpell->Id;
        else continue;

        data << Status;
        data << pSpell->Cost;
        data << uint32((pSpell->IsProfession && Status == TRAINER_STATUS_LEARNABLE) ? 1 : 0);
        data << uint32(pSpell->IsProfession);
        data << uint8(pSpell->RequiredLevel);
        data << pSpell->RequiredSkillLine;
        data << pSpell->RequiredSkillLineValue;
        data << pSpell->RequiredSpell;
        data << Spacer; //this is like a spell override or something, ex : (id=34568 or id=34547) or (id=36270 or id=34546) or (id=36271 or id=34548)
        data << Spacer;
        ++Count;
    }

    *(uint32*)&data.contents()[12] = Count;
    data << pTrainer->UIMessage;

    SendPacket(&data);
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN();
    uint64 Guid;
    uint32 TeachingSpellID;

    recvPacket >> Guid >> TeachingSpellID;
    Creature* pCreature = _player->GetMapMgr()->GetCreature(GUID_LOPART(Guid));
    if(pCreature == NULL)
        return;

    Trainer *pTrainer = pCreature->GetTrainer();
    if(pTrainer == NULL || !CanTrainAt(_player, pTrainer))
        return;

    TrainerSpell* pSpell = NULL;
    for(std::vector<TrainerSpell>::iterator itr = pTrainer->Spells.begin(); itr != pTrainer->Spells.end(); itr++)
    {
        if( ( itr->pCastRealSpell && itr->pCastRealSpell->Id == TeachingSpellID ) ||
            ( itr->pLearnSpell && itr->pLearnSpell->Id == TeachingSpellID ) )
        {
            pSpell = &(*itr);
        }
    }

    if(pSpell == NULL)
        return;

    if(TrainerGetSpellStatus(pSpell) > 0)
        return;

    _player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)pSpell->Cost);

    if( pSpell->pLearnSpell )
    {
        WoWGuid guid = pCreature->GetGUID();
        WorldPacket data(SMSG_PLAY_SPELL_VISUAL_KIT, 12);
        data << uint32(0) << uint32(179) << uint32(0);
        data.WriteBitString(8, guid[4], guid[7], guid[5], guid[3], guid[1], guid[2], guid[0], guid[6]);
        data.WriteSeqByteString(8, guid[0], guid[4], guid[1], guid[6], guid[7], guid[2], guid[3], guid[5]);
        pCreature->SendMessageToSet(&data, false);

        guid = _player->GetGUID();
        data.clear();
        data << uint32(0) << uint32(362) << uint32(0);
        data.WriteBitString(8, guid[4], guid[7], guid[5], guid[3], guid[1], guid[2], guid[0], guid[6]);
        data.WriteSeqByteString(8, guid[0], guid[4], guid[1], guid[6], guid[7], guid[2], guid[3], guid[5]);
        _player->SendMessageToSet(&data, true);

        _player->forget = pSpell->DeleteSpell;
        for( uint8 i = 0; i < 3; i++)
        {
            if(pSpell->pLearnSpell->Effect[i] == SPELL_EFFECT_PROFICIENCY || pSpell->pLearnSpell->Effect[i] == SPELL_EFFECT_LEARN_SPELL ||
                pSpell->pLearnSpell->Effect[i] == SPELL_EFFECT_WEAPON)
            {
                _player->CastSpell(_player, pSpell->pLearnSpell, true);
            }
        }

        _player->addSpell( pSpell->pLearnSpell->Id );
        _player->forget = 0;

        for( uint8 i = 0; i < 3; i++)
        {
            if( pSpell->pLearnSpell->Effect[i] == SPELL_EFFECT_SKILL )
            {
                uint32 skill = pSpell->pLearnSpell->EffectMiscValue[i];
                uint32 val = (pSpell->pLearnSpell->EffectBasePoints[i]+1) * 75;
                if( val > sWorld.GetMaxLevel(_player)*5 )
                    val = sWorld.GetMaxLevel(_player)*5;

                if( _player->_GetSkillLineMax(skill) >= val )
                    return;

                if( skill == SKILL_RIDING )
                    _player->_AddSkillLine( skill, val, val );
                else
                {
                    SkillLineEntry* sle = dbcSkillLine.LookupEntry(pSpell->RequiredSkillLine);
                    if( _player->_HasSkillLine(skill) )
                        _player->_ModifySkillMaximum(skill, val);
                    else if(sWorld.StartLevel > 1 && (sle == NULL || (sle->categoryId != SKILL_TYPE_PROFESSION && sle->categoryId != SKILL_TYPE_SECONDARY)))
                        _player->_AddSkillLine( skill, 5*sWorld.StartLevel, val);
                    else _player->_AddSkillLine( skill, 1, val);
                }
            }
        }
    }
    else if( pSpell->pCastSpell)
    {
        // Cast teaching spell on player
        pCreature->CastSpell(_player, pSpell->pCastSpell, true);
    }

    if(pSpell->DeleteSpell)
        _player->removeSpell(pSpell->DeleteSpell);
}

uint8 WorldSession::TrainerGetSpellStatus(TrainerSpell* pSpell)
{
    if(!pSpell->pCastRealSpell && !pSpell->pLearnSpell)
        return TRAINER_STATUS_NOT_AVAILABLE;

    if((pSpell->pCastRealSpell && objmgr.IsSpellDisabled(pSpell->pCastRealSpell->Id))
        || (pSpell->pLearnSpell && objmgr.IsSpellDisabled(pSpell->pLearnSpell->Id)))
        return TRAINER_STATUS_NOT_AVAILABLE;

    // Spells with the same names
    SpellEntry* namehashSP = NULL;
    if( pSpell->pCastRealSpell )
    {
        if( _player->HasSpell(pSpell->pCastRealSpell->Id)
            || ((namehashSP = _player->GetSpellWithNamehash(pSpell->pCastRealSpell->NameHash))
            && (namehashSP->RankNumber ? namehashSP->RankNumber > pSpell->pCastRealSpell->RankNumber : false)) )
            return TRAINER_STATUS_ALREADY_HAVE;
    }

    namehashSP = NULL;
    if( pSpell->pLearnSpell)
    {
        if( _player->HasSpell(pSpell->pLearnSpell->Id)
            || ((namehashSP = _player->GetSpellWithNamehash(pSpell->pLearnSpell->NameHash))
            && (namehashSP->RankNumber ? namehashSP->RankNumber > pSpell->pLearnSpell->RankNumber : false)) )
            return TRAINER_STATUS_ALREADY_HAVE;
    }

    // Spells with different names
    if((pSpell->pLearnSpell && _player->HasHigherSpellForSkillLine(pSpell->pLearnSpell))
        || (pSpell->pCastRealSpell && _player->HasHigherSpellForSkillLine(pSpell->pCastRealSpell)))
        return TRAINER_STATUS_ALREADY_HAVE;

    uint8 ssform = (pSpell->pLearnSpell ? pSpell->pLearnSpell->RequiredShapeShift : 0);
    uint32 ssspell = 0;
    if(ssform && ssform != FORM_MOONKIN) // We don't accept checks for this.
        ssspell = _player->GetSpellForShapeshiftForm(ssform, true);

    // Skill Checks
    bool hasskill = true;
    if(pSpell->RequiredSkillLine)
    {
        SkillLineEntry* sle = dbcSkillLine.LookupEntry(pSpell->RequiredSkillLine);
        if(sle != NULL) // It is a skill.
        {
            if(_player->_GetSkillLineCurrent(pSpell->RequiredSkillLine,true) < pSpell->RequiredSkillLineValue)
                hasskill = false;
        }
        else
        {
            SpellEntry* sp = dbcSpell.LookupEntry(pSpell->RequiredSkillLine);
            if(sp != NULL) // We accidentally put a spell here... -_-
            {
                // We purposely put a spell here, but we're still wrong... -_-
                if(SkillLineAbilityEntry* sla = dbcSkillLineSpell.LookupEntry(sp->Id))
                {
                    if(_player->_GetSkillLineCurrent(sla->skilline, true) < pSpell->RequiredSkillLineValue)
                        hasskill = false;
                }
            }
        }
    }

    if( (pSpell->RequiredLevel && _player->getLevel() < pSpell->RequiredLevel)
        || (pSpell->RequiredSpell && !_player->HasSpell(pSpell->RequiredSpell))
        || (pSpell->Cost && _player->GetUInt32Value(PLAYER_FIELD_COINAGE) < pSpell->Cost) || (!hasskill)
        || (pSpell->IsProfession && pSpell->RequiredSkillLine == 0 && _player->GetUInt32Value(PLAYER_CHARACTER_POINTS) == 0)//check level 1 professions if we can learn a new proffesion
        || (ssspell && !_player->HasSpell(ssspell)))
    {
        return TRAINER_STATUS_NOT_LEARNABLE;
    }

    return TRAINER_STATUS_LEARNABLE;
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_PETITION_SHOWLIST:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCharterShowListOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pCreature) return;

    SendCharterRequest(pCreature);
}

void WorldSession::SendCharterRequest(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    if( !pCreature->ArenaOrganizersFlags() )
    {
        WorldPacket data(SMSG_PETITION_SHOWLIST, 81);
        uint8 tdata[73];
        static const uint8 temp[73] = { 0x03, 0x01, 0x00, 0x00, 0x00, 0x08, 0x5C, 0x00, 0x00, 0x21, 0x3F, 0x00, 0x00, 0x00, 0x35, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x5C, 0x00, 0x00, 0x21, 0x3F, 0x00, 0x00, 0x80, 0x4F, 0x12, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0A, 0x5C, 0x00, 0x00, 0x21, 0x3F, 0x00, 0x00, 0x80, 0x84, 0x1E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 };
        memcpy(tdata, temp, sizeof(temp));
        data << pCreature->GetGUID();
        data.append(tdata,73);
        SendPacket(&data);
    }
    else
    {
        WorldPacket data(29);
        data.Initialize( SMSG_PETITION_SHOWLIST );
        data << pCreature->GetGUID();
        data << uint8(1);          // BOOL SHOW_COST = 1
        data << uint32(1);        // unknown
        if(pCreature && pCreature->GetEntry()==19861 ||
            pCreature->GetEntry()==18897 || pCreature->GetEntry()==19856)
        {
            data << uint16(ARENA_TEAM_CHARTER_2v2);  // ItemId of the guild charter
        } else data << uint16(0x16E7);  // ItemId of the guild charter

        data << float(0.62890625);  // strange floating point
        data << uint16(0);          // unknown
        data << uint32(1000);       // charter price
        data << uint32(0);          // unknown, maybe charter type
        data << uint32(9);          // amount of unique players needed to sign the charter
        SendPacket( &data );
    }
}

//////////////////////////////////////////////////////////////
/// This function handles MSG_AUCTION_HELLO:
//////////////////////////////////////////////////////////////
void WorldSession::HandleAuctionHelloOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;
    Creature* auctioneer = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!auctioneer)
        return;

    SendAuctionList(auctioneer);
}

void WorldSession::SendAuctionList(Creature* auctioneer)
{
    AuctionHouse* AH = sAuctionMgr.GetAuctionHouse(auctioneer->GetEntry());
    if(!AH)
    {
        sChatHandler.BlueSystemMessage(this, "Report to devs: Unbound auction house npc %u.", auctioneer->GetEntry());
        return;
    }

    WorldPacket data(MSG_AUCTION_HELLO, 12);
    data << auctioneer->GetGUID();
    data << uint32(AH->GetID());
    data << uint8(sWorld.AHEnabled ? 1 : 0);

    SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GOSSIP_HELLO:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Object* ent = NULL;
    switch(GUID_HIPART(guid)) // Crow: Could possibly do GetObject because I don't think we need items...
    {
    case HIGHGUID_TYPE_UNIT:
        ent = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
        break;
    case HIGHGUID_TYPE_GAMEOBJECT:
        ent = _player->GetMapMgr()->GetGameObject(GUID_LOPART(guid));
        break;
    case HIGHGUID_TYPE_ITEM:
        ent = _player->GetInventory()->GetInventoryItem(guid);
        break;
    }
    if(ent == NULL)
        return;
    SendGossipForObject(ent);
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GOSSIP_SELECT_OPTION:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    //WorldPacket data;
    uint32 option;
    uint32 unk24;
    uint64 guid;
    bool Coded = false;
    std::string BoxMessage;

    recv_data >> guid >> unk24 >> option;

}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_SPIRIT_HEALER_ACTIVATE:
//////////////////////////////////////////////////////////////
void WorldSession::HandleSpiritHealerActivateOpcode( WorldPacket & recv_data )
{
    if(!_player->IsInWorld() ||!_player->isDead())
        return;

    //No nonsense for GM's
    if(GetPermissionCount() == 0)
    {
        //25% duralbility loss
        _player->DeathDurabilityLoss(0.25f);

        //When revived by spirit healer, set health/mana at 50%
        _player->m_resurrectHealth = _player->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2;

        _player->ResurrectPlayer();

        if(_player->getLevel() > 10)
        {
            if(Aura* aur = _player->m_AuraInterface.FindAura(15007)) // If the player already have the aura, just extend it.
                aur->ResetExpirationTime();
            else
            {
                SpellCastTargets targets;
                targets.m_unitTarget = _player->GetGUID();
                if(Spell* sp = new Spell(_player, dbcSpell.LookupEntry(15007)))
                    sp->prepare(&targets, true);
            }
        }
    } else _player->ResurrectPlayer();
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_NPC_TEXT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNpcTextQueryOpcode( WorldPacket & recv_data )
{
    uint32 e = 0, i = 0;
    uint32 textID;
    uint64 targetGuid;
    GossipText *pGossip = NULL;

    recv_data >> textID;
    sLog.Debug("WORLD","CMSG_NPC_TEXT_QUERY ID '%u'", textID );

    recv_data >> targetGuid;
    GetPlayer()->SetUInt64Value(UNIT_FIELD_TARGET, targetGuid);

    if(textID != 68)
        pGossip = NpcTextStorage.LookupEntry(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 50000);
    data << textID;
    if(pGossip)
    {
        for(uint8 i = 0; i < 8; i++)
        {
            data << float(pGossip->Infos[i].Prob);
            if(strlen(pGossip->Texts[i].Text[0]) == 0)
                data << pGossip->Texts[i].Text[1];
            else
                data << pGossip->Texts[i].Text[0];

            if(strlen(pGossip->Texts[i].Text[1]) == 0)
                data << pGossip->Texts[i].Text[0];
            else
                data << pGossip->Texts[i].Text[1];
            data << pGossip->Infos[i].Lang;

            for(int e = 0; e < 3; e++)
            {
                data << uint32(pGossip->Infos[i].Delay);
                data << uint32(pGossip->Infos[i].Emote);
            }
        }
    }
    else
    {
        data << float(1.0f);        // Prob
        data << (textID != 68 ? "Hello, $N. What can I do for you?" : "Hi there, how can I help you $N"); // Team
        data << (textID != 68 ? "Hello, $N. What can I do for you?" : "Greetings, $N"); // Team
        data << uint32(0x00);       // Language

        for(int e = 0; e < 3; e++)
        {
            data << uint32(0x00);       // Emote delay
            data << uint32(0x00);       // Emote
        }

        for(uint8 i = 1; i < 8; i++)
        {
            data << float(1.0f);        // Prob
            data << uint8(0x00) << uint8(0x00); // Team
            data << uint32(0x00);       // Language

            for(int e = 0; e < 3; e++)
            {
                data << uint32(0x00);       // Emote delay
                data << uint32(0x00);       // Emote
            }
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleBinderActivateOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();
    uint64 guid;
    recv_data >> guid;

    Creature* pC = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if(!pC)
        return;

    SendInnkeeperBind(pC);
    _player->bHasBindDialogOpen = false;
}

void WorldSession::SendInnkeeperBind(Creature* pCreature)
{
    CHECK_INWORLD_RETURN();
    WorldPacket data(45);

    if(!_player->bHasBindDialogOpen)
    {
        OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);

        data.Initialize(SMSG_BINDER_CONFIRM);
        data << pCreature->GetGUID() << pCreature->GetZoneId();
        SendPacket(&data);

        _player->bHasBindDialogOpen = true;
        return;
    }

    pCreature->CastSpell(_player,3286,true);

    data.Initialize(SMSG_BINDPOINTUPDATE);
    data << _player->GetBindPositionX() << _player->GetBindPositionY() << _player->GetBindPositionZ() << _player->GetBindMapId() << _player->GetBindZoneId();
    SendPacket( &data );

    data.Initialize(SMSG_PLAYERBOUND);
    data << pCreature->GetGUID() << _player->GetBindZoneId();
    SendPacket(&data);

    OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);
}

void WorldSession::SendSpiritHealerRequest(Creature* pCreature)
{
    WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
    data << pCreature->GetGUID();
    SendPacket(&data);
}

void WorldSession::HandleListInventoryOpcode( WorldPacket & recv_data )
{
    CHECK_INWORLD_RETURN();

    sLog.Debug( "WORLD"," Recvd CMSG_LIST_INVENTORY" );
    uint64 guid;
    recv_data >> guid;

    Creature* unit = _player->GetMapMgr()->GetCreature(GUID_LOPART(guid));
    if (unit == NULL)
        return;

    if(unit->GetAIInterface())
        unit->GetAIInterface()->StopMovement(180000);

    if(FactionEntry *faction = unit->GetFaction())
        _player->Reputation_OnTalk(faction);
    SendInventoryList(unit);
}

void WorldSession::SendInventoryList(Creature* unit)
{
    if(!_player || !_player->IsInWorld())
        return;

    uint32 counter = 0;
    if(!unit->HasItems())
    {
        WorldPacket data(SMSG_LIST_INVENTORY, 10);
        data << uint64(unit->GetGUID());
        data << uint8(0) << uint8(0);
        SendPacket(&data);
        return;
    }

    ItemPrototype * curItem;
    WorldPacket data(SMSG_LIST_INVENTORY, ((unit->GetSellItemCount() * 28) + 9));      // allocate
    data << unit->GetGUID();
    data << uint8( 0 ); // placeholder for item count
    for(std::map<uint32, CreatureItem>::iterator itr = unit->GetSellItemBegin(); itr != unit->GetSellItemEnd(); itr++)
    {
        if(counter >= 150)
        {
            sLog.Error("VendorListing", "Creature %u contains too many items, Displaying (150/%u) items.",
                unit->GetEntry(), uint32(unit->GetSellItemCount()));
            break;
        }

        if(itr->second.itemid && (itr->second.max_amount == 0 || (itr->second.max_amount > 0 && itr->second.available_amount > 0)))
        {
            if((curItem = sItemMgr.LookupEntry(itr->second.itemid)))
            {
                if(!(itr->second.vendormask & unit->GetVendorMask()))
                    continue;

                if(!_player->ignoreitemreq_cheat)
                {
                    if(itr->second.IsDependent)
                    {
                        if(curItem->AllowableClass && !(_player->getClassMask() & curItem->AllowableClass))
                            continue;
                        if(curItem->AllowableRace && !(_player->getRaceMask() & curItem->AllowableRace))
                            continue;

                        if(curItem->Class == ITEM_CLASS_ARMOR && curItem->SubClass >= ITEM_SUBCLASS_ARMOR_LIBRAM && curItem->SubClass <= ITEM_SUBCLASS_ARMOR_SIGIL)
                            if(!(_player->GetArmorProficiency() & (uint32(1) << curItem->SubClass)))
                                continue; // Do not show relics to classes that can't use them.
                    }

                    if(itr->second.extended_cost == NULL && curItem->SellPrice > curItem->BuyPrice )
                        continue;
                }

                uint32 extendedCostId = itr->second.extended_cost != NULL ? itr->second.extended_cost->Id : 0;
                int32 av_am = (itr->second.max_amount > 0) ? itr->second.available_amount : -1;
                data << (++counter);
                data << uint32(1);
                data << curItem->ItemId;
                data << curItem->DisplayInfoID;
                data << av_am;
                data << sItemMgr.CalculateBuyPrice(curItem->ItemId, 1, _player, unit);
                data << curItem->MaxDurability;
                data << itr->second.amount;
                data << extendedCostId;
                data << uint8(0);
            }
        }
    }

    if(counter == 0) data << uint8(0);
    else data.put<uint8>(8, counter); // set count

    SendPacket( &data );
    sLog.Debug( "WORLD"," Sent SMSG_LIST_INVENTORY" );
}
