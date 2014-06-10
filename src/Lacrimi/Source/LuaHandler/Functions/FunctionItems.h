/*
 * Sun++ Scripts for Sandshroud MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2007-2008 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2008-2009 Sun++ Team <http://www.sunscripting.com/>
 * Copyright (C) 2009-2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEM_FUNCTIONS_H
#define ITEM_FUNCTIONS_H

/////////////////////////////////////////////////////
////////////////ITEM COMMANDS////////////////////////
/////////////////////////////////////////////////////

// Item Gossip Functions
int luaItem_GossipCreateMenu(lua_State * L, Item * ptr);
int luaItem_GossipMenuAddItem(lua_State * L, Item * ptr);
int luaItem_GossipSendMenu(lua_State * L, Item * ptr);
int luaItem_GossipComplete(lua_State * L, Item * ptr);
int luaItem_GossipSendPOI(lua_State * L, Item * ptr);
int luaItem_GossipSendQuickMenu(lua_State * L, Item * ptr);
int luaItem_GetOwner(lua_State * L, Item* ptr);
int luaItem_AddEnchantment(lua_State * L, Item* ptr);
int luaItem_RemoveEnchantment(lua_State * L, Item* ptr);
int luaItem_GetEntryId(lua_State * L, Item* ptr);
int luaItem_GetName(lua_State * L, Item* ptr);
int luaItem_GetSpellId(lua_State * L, Item* ptr);
int luaItem_GetSpellTrigger(lua_State * L, Item* ptr);
int luaItem_GetGUID(lua_State * L, Item* ptr);
int luaItem_AddLoot(lua_State * L, Item* ptr);
int luaItem_SetByteValue(lua_State * L, Item * ptr);
int luaItem_GetByteValue(lua_State * L, Item * ptr);
int luaItem_GetItemLink(lua_State * L, Item * ptr);
int luaItem_GetItemLevel(lua_State * L, Item * ptr);
int luaItem_GetRequiredLevel(lua_State * L, Item * ptr);
int luaItem_GetBuyPrice(lua_State * L, Item * ptr);
int luaItem_GetSellPrice(lua_State * L, Item * ptr);
int luaItem_RepairItem(lua_State * L, Item * ptr);
int luaItem_GetMaxDurability(lua_State * L, Item * ptr);
int luaItem_GetDurability(lua_State * L, Item * ptr);
int luaItem_HasEnchantment(lua_State * L, Item * ptr);
int luaItem_ModifyEnchantmentTime(lua_State * L, Item * ptr);
int luaItem_SetStackCount(lua_State * L, Item * ptr);
int luaItem_HasFlag(lua_State * L, Item * ptr);
int luaItem_IsSoulbound(lua_State * L, Item * ptr);
int luaItem_IsAccountbound(lua_State * L, Item * ptr);
int luaItem_IsContainer(lua_State * L, Item * ptr);
int luaItem_GetContainerItemCount(lua_State * L, Item * ptr);
int luaItem_GetEquippedSlot(lua_State * L, Item * ptr);
int luaItem_GetObjectType(lua_State * L, Item * ptr);
int luaItem_Remove(lua_State * L, Item * ptr);
int luaItem_Create(lua_State * L, Item * ptr);
int luaItem_ModUInt32Value(lua_State * L, Item * ptr);
int luaItem_ModFloatValue(lua_State * L, Item * ptr);
int luaItem_SetUInt32Value(lua_State * L, Item * ptr);
int luaItem_SetUInt64Value(lua_State * L, Item * ptr);
int luaItem_SetFloatValue(lua_State * L, Item * ptr);
int luaItem_GetFloatValue(lua_State * L, Item * ptr);
int luaItem_GetUInt32Value(lua_State * L, Item * ptr);
int luaItem_GetUInt64Value(lua_State * L, Item * ptr);
int luaItem_RemoveFlag(lua_State * L, Item * ptr);
int luaItem_SetFlag(lua_State * L, Item * ptr);

#endif // ITEM_FUNCTIONS_H
