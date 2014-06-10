/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

/////////////////////////////////////////////////////
////////////////ITEM COMMANDS////////////////////////
/////////////////////////////////////////////////////

// Item Gossip Functions
int luaItem_GossipCreateMenu(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    int text_id = luaL_checkint(L, 1);
    Unit* target = Lunar<Unit>::check(L, 2);
    if(target == NULL || !target->IsPlayer())
        RET_NIL(true);

    int autosend = luaL_checkint(L, 3);
    Player* plr = TO_PLAYER(target);
    objmgr.CreateGossipMenuForPlayer(&g_luaMgr.Menu, ptr->GetGUID(), text_id, plr);
    if(autosend)
        g_luaMgr.Menu->SendTo(plr);
    return 1;
}

int luaItem_GossipMenuAddItem(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    int icon = luaL_checkint(L, 1);
    const char * menu_text = luaL_checkstring(L, 2);
    int IntId = luaL_checkint(L, 3);
    if((menu_text == NULL) || g_luaMgr.Menu == NULL)
        RET_NIL(true);

    g_luaMgr.Menu->AddItem(icon, menu_text, IntId);
    return 1;
}

int luaItem_GossipSendMenu(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    Unit* target = Lunar<Unit>::check(L, 1);
    if(target == NULL)
        RET_NIL(true);

    Player * plr = TO_PLAYER(target);
    g_luaMgr.Menu->SendTo(plr);
    return 1;
}

int luaItem_GossipComplete(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    Unit* target = Lunar<Unit>::check(L, 1);
    if(target == NULL)
        RET_NIL(true);

    Player * plr = TO_PLAYER(target);
    plr->Gossip_Complete();
    return 1;
}

int luaItem_GossipSendPOI(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    Unit* target = Lunar<Unit>::check(L, 1);
    if(target == NULL)
        RET_NIL(true);

    Player* plr = TO_PLAYER(target);
    float x = float(luaL_checknumber(L, 2));
    float y = float(luaL_checknumber(L, 3));
    int icon = luaL_checkint(L, 4);
    int flags = luaL_checkint(L, 5);
    int data = luaL_checkint(L, 6);
    const char * name = luaL_checkstring(L, 7);
    if(!x || !y)
        RET_NIL(true);

    plr->Gossip_SendPOI(x, y, icon, flags, data, name);
    return 1;
}

int luaItem_GossipSendQuickMenu(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    uint32 text_id = luaL_checkint( L, 1 );
    Player *player = CHECK_PLAYER( L, 2 );
    if( player == NULL )
        RET_NIL(true);

    GossipMenuItem* Item = new GossipMenuItem();
    Item->IntId = luaL_checkint( L, 3 );
    Item->Icon = luaL_checkint( L, 4 );
    Item->Text = luaL_checkstring( L, 5 );
    Item->BoxMoney = luaL_checkint( L, 6 );
    Item->BoxMessage = luaL_checkstring( L, 7 );
    objmgr.CreateGossipMenuForPlayer(&g_luaMgr.Menu, ptr->GetGUID(), text_id, player);
    g_luaMgr.Menu->AddItem(Item);
    g_luaMgr.Menu->SendTo(player);
    return 1;
}

int luaItem_GetOwner(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    Player * owner = ptr->GetOwner();
    if(owner != NULL)
        Lunar<Unit>::push(L,owner,false);
    else
        lua_pushnil(L);
    return 1;
}

int luaItem_AddEnchantment(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    int entry = luaL_checkint(L,1);
    int duration = luaL_checkint(L,2);
    bool permanent = (duration == 0) ? true : false;
    bool temp = (luaL_checkint(L, 3) == 1) ? true : false;

    EnchantEntry *eentry = dbcEnchant.LookupEntry( entry );
    if(eentry == NULL)
        RET_NIL(true);

    lua_pushinteger(L, ptr->AddEnchantment(eentry, duration, permanent, true, temp));
    return 1;
}

int luaItem_RemoveEnchantment(lua_State * L, Item* ptr)
{
    TEST_ITEM();
    int slot = luaL_checkint(L,1);
    bool temp = (luaL_checkint(L,2) == 1 ? true : false);

    if (slot == -1)
        ptr->RemoveAllEnchantments(temp);
    else if (slot == -2)
        ptr->RemoveProfessionEnchant();
    else if (slot == -3)
        ptr->RemoveSocketBonusEnchant();
    else if (slot >= 0)
        ptr->RemoveEnchantment(slot);
    return 1;
}

int luaItem_GetEntryId(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetEntry());
    return 1;
}

int luaItem_GetName(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    ItemPrototype * proto = ptr->GetProto();
    if(proto == NULL)
        RET_NIL(true);

    lua_pushstring(L, proto->Name1);
    return 1;
}

int luaItem_GetSpellId(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    ItemPrototype * proto = ptr->GetProto();
    uint32 index = luaL_checkint(L, 1);
    if (index < 0 || index > 5)
        RET_NIL(true);

    lua_pushnumber(L, proto->Spells[index].Id);
    return 1;
}

int luaItem_GetSpellTrigger(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    ItemPrototype * proto = ptr->GetProto();
    uint32 index = luaL_checkint(L, 1);
    if (index < 0 || index > 5)
        RET_NIL(true);

    /*  
        USE             = 0,
        ON_EQUIP        = 1,
        CHANCE_ON_HIT   = 2,
        SOULSTONE       = 4,
        LEARNING        = 6,
    */
    lua_pushnumber(L, proto->Spells[index].Trigger);
    return 1;
}

int luaItem_GetGUID(lua_State * L, Item* ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushinteger(L,int(ptr->GetGUID()));
    return 1;
}

int luaItem_AddLoot(lua_State * L, Item* ptr)
{
    TEST_ITEM();
    uint32 itemid = luaL_checkint(L,1);
    uint32 mincount = luaL_checkint(L,2);
    uint32 maxcount = luaL_checkint(L,3);
    uint32 ffa_loot = luaL_checkint(L,4);
    bool perm = ((luaL_checkint(L,5) == 1) ? true : false);
    if(perm)
    {
        double chance = luaL_checknumber(L, 6);
        QueryResult* result = WorldDatabase.Query("SELECT * FROM itemloot WHERE entryid = %u, itemid = %u", ptr->GetEntry(), itemid);
        if(!result)
        {
            WorldDatabase.Execute("REPLACE INTO itemloot VALUES (%u, %u, %f, %u, %u, %u)", ptr->GetEntry(), itemid, float(chance), mincount, maxcount, ffa_loot);
            lootmgr.AddLoot(ptr->GetLoot(),itemid,mincount,maxcount,ffa_loot);
        }
    }
    else
        lootmgr.AddLoot(ptr->GetLoot(),itemid,mincount,maxcount,ffa_loot);

    return 1;
}

int luaItem_SetByteValue(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    uint32 index = luaL_checkint(L,1);
    uint32 index1 = luaL_checkint(L,2);
    uint8 value = luaL_checkint(L,3);
    ptr->SetByte(index,index1,value);
    return 1;
}

int luaItem_GetByteValue(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    uint32 index = luaL_checkint(L,1);
    uint32 index1 = luaL_checkint(L,2);
    lua_pushinteger(L,ptr->GetByte(index,index1));
    return 1;
}

int luaItem_GetItemLink(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
//  uint32 lang = luaL_optint(L, 1, LANG_UNIVERSAL);
    if(ptr->GetProto() == NULL)
        return 0;

    lua_pushstring(L, ptr->ConstructItemLink());
    return 1;
}

int luaItem_GetItemLevel(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetProto()->ItemLevel);
    return 1;
}

int luaItem_GetRequiredLevel(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetProto()->RequiredLevel);
    return 1;
}

int luaItem_GetBuyPrice(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetProto()->BuyPrice);
    return 1;
}

int luaItem_GetSellPrice(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetProto()->SellPrice);
    return 1;
}

int luaItem_RepairItem(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    ptr->SetDurabilityToMax();
    return 1;
}

int luaItem_GetMaxDurability(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetDurabilityMax());
    return 1;
}

int luaItem_GetDurability(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushnumber(L, ptr->GetDurability());
    return 1;
}

int luaItem_HasEnchantment(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET();
    if(ptr->HasEnchantments())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int luaItem_ModifyEnchantmentTime(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    uint32 slot = luaL_checkint(L, 1);
    uint32 duration = luaL_checkint(L, 2);
    ptr->ModifyEnchantmentTime(slot, duration);
    return 1;
}

int luaItem_SetStackCount(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    uint32 count = luaL_checkint(L, 1);
    if(!count || count > 1000)
        return 0;

    ptr->SetUInt32Value(ITEM_FIELD_STACK_COUNT, count);
    return 1;
}

int luaItem_HasFlag(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET();
    uint32 index = luaL_checkint(L,1);
    uint32 flag = luaL_checkint(L,2);
    lua_pushboolean(L, ptr->HasFlag(index,flag) ? 1 : 0);
    return 1;
}

int luaItem_IsSoulbound(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET();
    lua_pushboolean(L,ptr->IsSoulbound());
    return 1;
}

int luaItem_IsAccountbound(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET();
    lua_pushboolean(L, ptr->IsAccountbound());
    return 1;
}

int luaItem_IsContainer(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET();
    lua_pushboolean(L,ptr->IsContainer());
    return 1;
}

int luaItem_GetContainerItemCount(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET();
    uint32 itemid = (uint32)luaL_checknumber(L,1);
    if (!itemid)
        RET_NIL(true);

    Container * pCont = TO_CONTAINER(ptr);
    int16 TotalSlots = pCont->GetProto()->ContainerSlots;
    if(TotalSlots < 1)
        RET_NIL(true);

    int cnt = 0;
    for (int16 i = 0; i < TotalSlots; i++)
    {
        Item *item = pCont->GetItem(i);
        if (item)
        {
            if(item->GetEntry() == itemid && item->wrapped_item_id == 0)
            {
                cnt += item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) ? item->GetUInt32Value(ITEM_FIELD_STACK_COUNT) : 1; 
            }
        }
    }
    lua_pushinteger(L, cnt);
    return 1;
}

int luaItem_GetEquippedSlot(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushinteger(L, ptr->GetOwner()->GetItemInterface()->GetInventorySlotById(ptr->GetEntry()));
    return 1;
}

int luaItem_GetObjectType(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    lua_pushstring(L, "Item");
    return 1;
}

int luaItem_Remove(lua_State * L, Item * ptr)
{
    TEST_ITEM();
    ptr->RemoveFromWorld();
    return 1;
}

int luaItem_Create(lua_State * L, Item * ptr)
{
    TEST_ITEM_RET_NULL();
    uint32 id = (uint32)luaL_checknumber(L,1);
    uint32 stackcount = (uint32)luaL_checknumber(L,2);
    Item * pItem = objmgr.CreateItem(id, NULL);
    if (!pItem)
        RET_NIL(true);

    pItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, stackcount);
    pItem->SaveToDB(0, 0, true, NULL);
    Lunar<Item>::push(L,pItem);
    return 1;
}

int luaItem_ModUInt32Value(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    int value = luaL_checkint(L, 2);
    ptr->ModSignedInt32Value(field, value);
    return 1;
}

int luaItem_ModFloatValue(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    float value = (float)luaL_checknumber(L, 2);
    ptr->ModFloatValue(field, value);
    return 1;
}

int luaItem_SetUInt32Value(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    int value = luaL_checkint(L, 2);
    ptr->SetUInt32Value(field, value);
    return 1;
}

int luaItem_SetUInt64Value(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    int value = luaL_checkint(L, 2);
    ptr->SetUInt64Value(field, value);
    return 1;
}

int luaItem_SetFloatValue(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    float value = (float)luaL_checknumber(L, 2);
    ptr->SetFloatValue(field, value);
    return 1;
}

int luaItem_GetFloatValue(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    lua_pushnumber(L, ptr->GetFloatValue(field));
    return 1;
}

int luaItem_GetUInt32Value(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    lua_pushnumber(L, ptr->GetUInt32Value(field));
    return 1;
}

int luaItem_GetUInt64Value(lua_State * L, Item * ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    if(field >= ITEM_END)
        return 0;

    lua_pushinteger(L, (int)ptr->GetUInt64Value(field));
    return 1;
}

int luaItem_RemoveFlag(lua_State* L, Item* ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    int value = luaL_checkint(L, 2);
    if(ptr)
        ptr->RemoveFlag(field, value);
    return 0;
}

int luaItem_SetFlag(lua_State* L, Item* ptr)
{
    TEST_ITEM();

    int field = luaL_checkint(L, 1);
    int value = luaL_checkint(L, 2);
    if(ptr)
        ptr->SetFlag(field, value);
    return 0;
}
