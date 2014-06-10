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

#ifndef __TABLEFUNCTIONS_H
#define __TABLEFUNCTIONS_H

// Crow: Some of the following functions are based off of functions created by Hypersniper of LuaHypeArc.
// Areas where this applies, credit has been given in the form of a name declaration.

/*  Empty Functions: Both of these are used as placeholders for the actual code
    so we can have the functions, but they won't do anything. */
template<typename T> int EmptyLuaFunction(lua_State * L, T*)
{
    return 1;
}

int EmptyGlobalFunction(lua_State*L)
{
    return 1;
}

// Crow: Some of the following functions are based off of functions created by Hypersniper of LuaHypeArc.
// Areas where this applies, credit has been given in the form of a name declaration.
/************************************************************************/
/* SCRIPT FUNCTION TABLES                                               */
/************************************************************************/
RegType<Item> ItemMethods[];
RegType<Unit> UnitMethods[];
RegType<GameObject> GOMethods[];
RegType<TaxiPath> LuaTaxiMethods[];
RegType<WorldPacket> LuaPacketMethods[];
RegType<Spell> SpellMethods[];
RegType<QueryResult> QResultMethods[];
RegType<Field> SQLFieldMethods[];
RegType<Aura> AuraMethods[];

template<> RegType<Item>* GetMethodTable<Item>();
template<> RegType<Unit>* GetMethodTable<Unit>();
template<> RegType<GameObject>* GetMethodTable<GameObject>();
template<> RegType<TaxiPath>* GetMethodTable<TaxiPath>();
template<> RegType<WorldPacket>* GetMethodTable<WorldPacket>();
template<> RegType<Spell>* GetMethodTable<Spell>();
template<> RegType<QueryResult>* GetMethodTable<QueryResult>();
template<> RegType<Field> * GetMethodTable<Field>();
template<> RegType<Aura> * GetMethodTable<Aura>();

void RegisterGlobalFunctions(lua_State *L);

#endif // __TABLEFUNCTIONS_H
